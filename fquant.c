/**
**  \file
**  \brief     InsaniQuant fast reduction pass
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2015.03.27
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


#include "fquant.h"
#include "coldiff.h"
#include "depthred.h"



/* Structure for one color */
typedef struct{
 auint col;       /* RGB color value */
 auint occ;       /* Occurrence */
}fquant_col_t;

/* All of the colors on the image */
static fquant_col_t fquant_col[FQUANT_COLS];

/* All of the weighted differences between colors */
static auint fquant_dif[FQUANT_COLS * FQUANT_COLS];



/* Retrieves an RGB color from the image */
static auint fquant_iget(uint8 const* buf, auint px)
{
 return (buf[(px * 3U) + 0U] << 16) |
        (buf[(px * 3U) + 1U] <<  8) |
        (buf[(px * 3U) + 2U]      );
}



/* The fast quantizer pass, reducing the image to the given count of colors.
** It calls the depth reductor first as needed to trim down color count to an
** amount it is capable to work with. The cols parameter is it's output target
** (as quantized image). */
void fquant(uint8 const* buf, uint8* wrk, auint bsiz, auint cols)
{
 auint ccnt = 0U;
 auint bcnt;
 auint clid;
 auint cli2;
 auint clvl;
 auint cxid;
 auint cxvl;
 auint i;
 auint j;
 auint k;
 auint r;
 auint g;
 auint b;
 auint c0;

 /* Depth reduction to get a suitable amount of colors */

 depthred(buf, wrk, bsiz, FQUANT_COLS);

 /* Collect the colors from the image with their occurrences */

 printf("FQuant: Gathering color data\n");

 for (i = 0U; i < bsiz; i++){ /* Collect */
  c0 = fquant_iget(wrk, i);
  for (j = 0U; j < ccnt; j++){
   if (c0 == fquant_col[j].col){ /* Already collected color */
    fquant_col[j].occ++;
    break;
   }
  }
  if ((j == ccnt) && (j != FQUANT_COLS)){ /* One more color */
   fquant_col[j].col = c0;
   fquant_col[j].occ = 1U;
   ccnt++;
  }else if (j == FQUANT_COLS){
   printf("FQuant: Color count exceed at position %u! Aborting.\n", i);
   return;
  }
 }

 /* Quantization pass: Merge the two least occuring colors in every pass.
 ** Don't care about depth target to keep things clean: the simple
 ** quantization style won't work out well with that reduction. */

 printf("FQuant: Reducing color count to %u colors\n", cols);

 /* Pre-calculate the color difference matrix */

 for (i = 0U; i < ccnt; i++){
  k = i * FQUANT_COLS;
  for (j = i + 1U; j < ccnt; j++){
   fquant_dif[k + j] = coldiff_w(fquant_col[j].col, fquant_col[j].occ,
                                 fquant_col[i].col, fquant_col[i].occ, bsiz);
  }
 }

 bcnt = ccnt;

 while (bcnt > cols){

  /* Search for a color pair to merge */

  clid = 0U;
  cli2 = 0U;
  clvl = 0xFFFFFFFFU;

  for (i = 0U; i < ccnt; i++){ /* Go through all colors */
   if (fquant_col[i].occ != 0U){ /* Only if the color is still valid (exists) */

    /* Search the other colors for smallest difference, weighted with
    ** occupation */

    k    = i * FQUANT_COLS;
    cxid = 0U;
    cxvl = 0xFFFFFFFFU;

    for (j = i + 1U; j < ccnt; j++){

     c0 = fquant_dif[k + j];
     if (c0 < cxvl){
      cxvl = c0;
      cxid = j;
     }

    }

    if (cxvl < clvl){
     clvl = cxvl;
     clid = i;
     cli2 = cxid;
    }

   }
  }

  /* The colors to merge are now identified in clid and cli2. Average them
  ** weighted with their occupation. */

  r   = ((fquant_col[clid].col >> 16) & 0xFFU) * fquant_col[clid].occ;
  g   = ((fquant_col[clid].col >>  8) & 0xFFU) * fquant_col[clid].occ;
  b   = ((fquant_col[clid].col      ) & 0xFFU) * fquant_col[clid].occ;
  c0  = fquant_col[clid].occ;
  r  += ((fquant_col[cli2].col >> 16) & 0xFFU) * fquant_col[cli2].occ;
  g  += ((fquant_col[cli2].col >>  8) & 0xFFU) * fquant_col[cli2].occ;
  b  += ((fquant_col[cli2].col      ) & 0xFFU) * fquant_col[cli2].occ;
  c0 += fquant_col[cli2].occ;
  r   = ((r + (c0 >> 1)) / c0) & 0xFFU;
  g   = ((g + (c0 >> 1)) / c0) & 0xFFU;
  b   = ((b + (c0 >> 1)) / c0) & 0xFFU;
  fquant_col[clid].col = (r << 16) | (g << 8) | (b);
  fquant_col[clid].occ = c0;
  fquant_col[cli2].occ = 0U; /* Invalidate the other color */

  /* Update difference matrix for the new color */

  k = clid * FQUANT_COLS;
  for (j = clid + 1U; j < ccnt; j++){
   if (fquant_col[j].occ != 0U){ /* Valid color */
    fquant_dif[k + j] = coldiff_w(fquant_col[   j].col, fquant_col[   j].occ,
                                  fquant_col[clid].col, fquant_col[clid].occ, bsiz);
   }else{                        /* Invalid color: disable selecting it */
    fquant_dif[k + j] = 0xFFFFFFFFU;
   }
  }
  for (i = 0U; i < clid; i++){
   if (fquant_col[i].occ != 0U){ /* Valid color */
    fquant_dif[(i * FQUANT_COLS) + clid] =
                        coldiff_w(fquant_col[clid].col, fquant_col[clid].occ,
                                  fquant_col[   i].col, fquant_col[   i].occ, bsiz);
   }else{                        /* Invalid color: disable selecting it */
    fquant_dif[(i * FQUANT_COLS) + clid] = 0xFFFFFFFFU;
   }
  }

  /* Disable the discarded color in the difference matrix */

  k = cli2 * FQUANT_COLS;
  for (j = clid + 1U; j < ccnt; j++){
   fquant_dif[k + j] = 0xFFFFFFFFU;
  }
  for (i = 0U; i < cli2; i++){
   fquant_dif[(i * FQUANT_COLS) + cli2] = 0xFFFFFFFFU;
  }

  /* OK, one color less to go! */

  bcnt--;
  printf("."); /* Just an indicator of progress... */
  fflush(stdout);

 }

 printf("\n");

 /* Now the bucket count is reduced to the desired color count. Create a
 ** palette from it, so the image may be processed. */

 printf("FQuant: Assembling palette of %u colors\n", bcnt);

 j = 0U;
 for (i = 0U; i < ccnt; i++){ /* Compact palette */
  if (fquant_col[i].occ != 0U){
   fquant_col[j].col = fquant_col[i].col;
   j ++;
  }
 }

 /* Palette OK, quantize the image with it */

 printf("FQuant: Quantizing the image (%u colors)\n", bcnt);

 c0 = 0x80000000U;
 clid = 0U;
 for (i = 0U; i < bsiz; i++){
  k = fquant_iget(buf, i);
  if (c0 != k){ /* Be faster for identical colors */
   c0   = k;
   clid = 0U;
   clvl = 0xFFFFFFFFU;
   for (j = 0U; j < bcnt; j++){ /* Get least differing color from palette */
    k = coldiff(fquant_col[j].col, c0);
    if (k < clvl){
     clvl = k;
     clid = j;
    }
   }
  }
  k = fquant_col[clid].col;
  wrk[(i * 3U) + 0U] = (k >> 16) & 0xFFU;
  wrk[(i * 3U) + 1U] = (k >>  8) & 0xFFU;
  wrk[(i * 3U) + 2U] = (k      ) & 0xFFU;
 }

}
