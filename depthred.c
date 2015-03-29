/**
**  \file
**  \brief     InsaniQuant bit depth reduction pass
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2015.03.29
**
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "depthred.h"
#include "coldepth.h"
#include "palgen.h"
#include "idata.h"



/* Color count buffer. Placed in the global (file local) namespace since
** putting 2 megs on the stack might not work out too well. */
static uint8 depthred_ccb[(256U * 256U * 256U) >> 3];

/* Count of set bits table in a byte */
static const uint8 depthred_bct[256] = {
 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

/* Internal reference palette size, from which colors are gathered in
** selective depth increment */
#define DR_RPSIZ 16384U

/* Color buffer for the reference palette (avoid stack) */
static iquant_col_t depthred_rcb[8][DR_RPSIZ];



/* Counts colors using a given target depth in the passed RGB image buffer.
** The size is specified as pixel count. */
static auint depthred_cc(uint8 const* buf, auint bsiz, auint depth)
{
 auint i;
 auint rgb;
 auint r = 0U;

 /* Clear color count bitmap */

 memset(depthred_ccb, 0U, (256U * 256U * 256U) >> 3);

 /* Collect used colors as a bitmap */

 for (i = 0U; i < bsiz; i++){
  rgb = coldepth(idata_get(buf, i), depth);
  depthred_ccb[rgb >> 3] |= 1U << (rgb & 0x7U);
 }

 /* Count the set bits */

 for (i = 0U; i < ((256U * 256U * 256U) >> 3); i++){
  r += depthred_bct[depthred_ccb[i]];
 }

 return r;
}



/* Reduces bit depth of the passed image by trimming low bits. The bsiz
** parameter is the RGB image buffer's size in pixels. The clipped low bits
** are not populated with zero, rather scaled to let them cover the entire
** original color range (if clipped, the image would slightly darken as a
** result of the quantization which is not desirable). */
void depthred(uint8 const* buf, auint bsiz, iquant_pal_t* pal, auint cols)
{
 auint dep = 8U;
 auint rdep = 0U;
 auint cc;
 auint i;
 auint ocm;
 auint ocv;
 auint oci;
 auint ds;
 auint c0;
 iquant_pal_t rpal[8];

 /* This stage should be used for coarse reducting. So cols must be at least
 ** 1024. */

 if (cols < 1280U){
  cols = 1280U;
  printf("Depth reduction: Asked for too few colors, targeting 1280 instead\n");
 }
 if (cols > (pal->mct)){
  printf("Depth reduction: Asked for too many colors, aborting\n");
  pal->cct = 0U;
  return;
 }

 cc = depthred_cc(buf, bsiz, dep);
 printf("Depth reduction: Initial color count: %u (target: %u)\n", cc, cols);

 /* Reduce to fit in 1024 colors, meanwhile generating reference palettes
 ** for each depth where it is possible */

 while (dep > 1U){
  if (cc < DR_RPSIZ){
   rpal[dep - 1U].mct = DR_RPSIZ;
   rpal[dep - 1U].col = &(depthred_rcb[dep - 1U][0]);
   palgen(buf, bsiz, &(rpal[dep - 1U]), dep);
   if (rdep == 0U){ rdep = dep; }
  }else{
   rpal[dep - 1U].cct = 0U;
  }
  if (cc <= 1024U){ break; } /* Done */
  dep--;
  cc = depthred_cc(buf, bsiz, dep);
  printf("Depth reduction: Depth: %u, Color count: %u (target: %u)\n", dep, cc, cols);
 }

 /* Prepare initial output palette */

 palgen(buf, bsiz, pal, dep);

 /* Now iteratively increment color count by selectively increasing the depth
 ** of the most occuring color until hitting the color limit, or the
 ** possibilities for doing this are exhausted, using the reference palette.
 ** To know how to compare, follow the depths assigned to the colors. */

 for (i = 0U; i < (pal->cct); i++){
  pal->col[i].wrk = dep; /* Initial depths */
 }

 ocm = pal->ocs; /* Maximum occurence to consider (to exclude anything already done) */

 /* Note: one depth increment should swap out one color to six others since
 ** there are 3 components, with a depth increment each splits in two. However
 ** be safe, and assume propable 9 colors by the split (since the depth
 ** reduction is not simple bit trimming, oddities may happen). */

 printf("Depth reduction: Selectively increasing depth\n");

 while ((pal->cct) < (cols - 8U)){

  /* Look for maximum occurrence */

  oci = 0U;
  ocv = 0U;
  for (i = 0U; i < (pal->cct); i++){
   if ( (pal->col[i].occ > ocv) &&  /* Occurs more */
        (pal->col[i].wrk < rdep) && /* Not at peak depth by reference palettes */
        (pal->col[i].occ <= ocm) ){ /* Neither tops the occurrence limit */
    oci = i;
    ocv = pal->col[i].occ;
   }
  }
  ocm = ocv;               /* Next time don't go this high in occupation search */

  if (ocv == 0U){ break; } /* No more increment possibilites, palette completed */

  /* Now hunt down colors in the reference palette which match this one one
  ** depth level added, and combine those in the palette */

  ds = pal->col[oci].wrk;
  c0 = coldepth(pal->col[oci].col, ds);

  for (i = 0U; i < rpal[ds].cct; i++){
   if (coldepth(rpal[ds].col[i].col, ds) == c0){ /* Matching color */
    pal->col[oci].col = rpal[ds].col[i].col;
    pal->col[oci].occ = rpal[ds].col[i].occ;
    pal->col[oci].wrk = ds + 1U;
    break;
   }
  }
  for (      ; i < rpal[ds].cct; i++){
   if (coldepth(rpal[ds].col[i].col, ds) == c0){ /* Matching color */
    if ((pal->cct) != (pal->mct)){
     pal->col[pal->cct].col = rpal[ds].col[i].col;
     pal->col[pal->cct].occ = rpal[ds].col[i].occ;
     pal->col[pal->cct].wrk = ds + 1U;
     pal->cct ++;
    }else{
     printf("Depth reduction: Palette maximum (%u) exceed, aborting\n", pal->mct);
     return;
    }
   }
  }

 }

 printf("Depth reduction: Final color count %u\n", pal->cct);
}
