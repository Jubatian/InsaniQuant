/**
**  \file
**  \brief     InsaniQuant fast reduction pass
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


#include "fquant.h"
#include "coldiff.h"
#include "colmerge.h"
#include "depthred.h"



/* All of the weighted differences between colors */
static float fquant_dif[FQUANT_COLS * FQUANT_COLS];

/* Large floating point number to start search at...
** Need to replace to something better. */
#define FLT_LARGE (1.0e30)



/* The fast quantizer pass, reducing the occurrence weighted palette to the
** given count of colors. The cols parameter is it's output target. */
void fquant(iquant_pal_t* pal, auint cols)
{
 auint bcnt;
 auint clid;
 auint cli2;
 float clvl;
 auint cxid;
 float cxvl;
 float ft;
 auint i;
 auint j;
 auint k;

 /* Check if palette can be used */

 if ((pal->cct) > FQUANT_COLS){
  printf("FQuant: Color count exceed (%u > %u)! Aborting.\n", pal->cct, FQUANT_COLS);
  return;
 }

 /* Quantization pass: Merge the two least occuring colors in every pass.
 ** Don't care about depth target to keep things clean: the simple
 ** quantization style won't work out well with that reduction. */

 printf("FQuant: Reducing color count to %u colors\n", cols);

 /* Pre-calculate the color difference matrix */

 for (i = 0U; i < (pal->cct); i++){
  k = i * FQUANT_COLS;
  for (j = i + 1U; j < (pal->cct); j++){
   fquant_dif[k + j] = coldiff_w(pal->col[j].col, pal->col[j].occ,
                                 pal->col[i].col, pal->col[i].occ, pal->ocs);
  }
 }

 bcnt = pal->cct;

 while (bcnt > cols){

  /* Search for a color pair to merge */

  clid = 0U;
  cli2 = 0U;
  clvl = FLT_LARGE;

  for (i = 0U; i < (pal->cct); i++){ /* Go through all colors */
   if ((pal->col[i].occ) != 0U){ /* Only if the color is still valid (exists) */

    /* Search the other colors for smallest difference, weighted with
    ** occupation */

    k    = i * FQUANT_COLS;
    cxid = 0U;
    cxvl = FLT_LARGE;

    for (j = i + 1U; j < (pal->cct); j++){

     ft = fquant_dif[k + j];
     if (ft < cxvl){
      cxvl = ft;
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
  ** weighted with their occupation (into clid, cli2 invalidated). */

  colmerge(pal, clid, cli2);

  /* Update difference matrix for the new color */

  k = clid * FQUANT_COLS;
  for (j = clid + 1U; j < (pal->cct); j++){
   if (pal->col[j].occ != 0U){ /* Valid color */
    fquant_dif[k + j] = coldiff_w(pal->col[   j].col, pal->col[   j].occ,
                                  pal->col[clid].col, pal->col[clid].occ, pal->ocs);
   }else{                      /* Invalid color: disable selecting it */
    fquant_dif[k + j] = FLT_LARGE;
   }
  }
  for (i = 0U; i < clid; i++){
   if (pal->col[i].occ != 0U){ /* Valid color */
    fquant_dif[(i * FQUANT_COLS) + clid] =
                        coldiff_w(pal->col[clid].col, pal->col[clid].occ,
                                  pal->col[   i].col, pal->col[   i].occ, pal->ocs);
   }else{                      /* Invalid color: disable selecting it */
    fquant_dif[(i * FQUANT_COLS) + clid] = FLT_LARGE;
   }
  }

  /* Disable the discarded color in the difference matrix */

  k = cli2 * FQUANT_COLS;
  for (j = clid + 1U; j < (pal->cct); j++){
   fquant_dif[k + j] = FLT_LARGE;
  }
  for (i = 0U; i < cli2; i++){
   fquant_dif[(i * FQUANT_COLS) + cli2] = FLT_LARGE;
  }

  /* OK, one color less to go! */

  bcnt--;
  printf("."); /* Just an indicator of progress... */
  fflush(stdout);

 }

 printf("\n");

 /* Now the color count is reduced to the desired color count. Create a proper
 ** palette from it. */

 printf("FQuant: Assembling palette of %u colors\n", bcnt);

 j = 0U;
 for (i = 0U; i < (pal->cct); i++){ /* Compact palette */
  if (pal->col[i].occ != 0U){
   pal->col[j].col = pal->col[i].col;
   pal->col[j].occ = pal->col[i].occ;
   j ++;
  }
 }
 pal->cct = j;
}
