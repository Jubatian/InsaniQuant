/**
**  \file
**  \brief     InsaniQuant main reduction pass
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2015.03.30
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


#include "iquant.h"
#include "coldiff.h"
#include "coldepth.h"
#include "fquant.h"



/* Average colors (going in the palette) for every bucket */
static auint iquant_bcl[IQUANT_COLS];

/* Occupation data for every bucket, for weighting */
static auint iquant_boc[IQUANT_COLS];

/* Large floating point number to start search at...
** Need to replace to something better. */
#define FLT_LARGE (1.0e30)



/* Calculates average colors (palette) for the buckets */
static void iquant_cpal(iquant_pal_t* pal, auint pdep)
{
 auint i;
 auint j;
 auint r;
 auint g;
 auint b;
 auint c;

 for (i = 0U; i < (pal->cct); i++){ /* For every bucket */

  if (iquant_boc[i] != 0U){ /* Only unless it was emptied earlier */

   r = 0U;
   g = 0U;
   b = 0U;
   c = 0U;
   for (j = 0U; j < (pal->cct); j++){ /* For every color in the bucket 'i' */
    if (pal->col[j].wrk == i){
     r += ((pal->col[j].col >> 16) & 0xFFU) * pal->col[j].occ;
     g += ((pal->col[j].col >>  8) & 0xFFU) * pal->col[j].occ;
     b += ((pal->col[j].col      ) & 0xFFU) * pal->col[j].occ;
     c += pal->col[j].occ;
    }
   }
   if (c != 0U){ /* (The mask with 0xFF shouldn't be necessary) */
    r = ((r + (c >> 1)) / c) & 0xFFU;
    g = ((g + (c >> 1)) / c) & 0xFFU;
    b = ((b + (c >> 1)) / c) & 0xFFU;
   }
   iquant_bcl[i] = coldepth((r << 16) | (g << 8) | (b), pdep);
   iquant_boc[i] = c;

  }

 }
}



/* The main quantizer pass, reducing the occurrence weighted palette to the
** given count of colors. The pdep parameter can be used to force a bit depth
** on the palette it generates (1 - 8 bits). */
void iquant(iquant_pal_t* pal, auint cols, auint pdep)
{
 auint bcnt;
 auint bmid;
 auint bmi2;
 float bmvl;
 auint bxid;
 float bxvl;
 float ft;
 auint otid;
 auint otvl;
 auint i;
 auint j;
 auint k;

 /* Check if palette can be used */

 if ((pal->cct) > IQUANT_COLS){
  printf("IQuant: Color count exceed (%u > %u)! Aborting.\n", pal->cct, IQUANT_COLS);
  return;
 }

 /* Initialize work data: bucket assingments, and initial bucket occurrences
 ** (the latter just to nonzero to indicate they are valid) */

 for (i = 0U; i < (pal->cct); i++){
  pal->col[i].wrk = i; /* Every color initially goes into own bucket */
  iquant_boc[i] = 1U;  /* Bucket valid */
 }

 /* Quantization pass: dismantle one color bucket in each iteration by the
 ** smallest of the largest principle (where the largest small difference to
 ** any out-of-bucket color is the smallest, that bucket will be dismantled).
 ** Do this until achieving the desired color count. */

 printf("IQuant: Reducing color count to %u colors\n", cols);

 bcnt = pal->cct;

 while (bcnt > cols){

  /* Calculate the bucket averages and occupation */

  iquant_cpal(pal, pdep);

  /* Search for a bucket to dismantle */

  bmid = 0U;
  bmi2 = 0U;
  bmvl = FLT_LARGE;

  for (i = 0U; i < (pal->cct); i++){ /* Go through all buckets */
   if (iquant_boc[i] != 0U){ /* Only if the bucket is still valid (contains colors) */

    /* Search the other buckets for smallest difference, weighted with
    ** occupation */

    bxid = 0U;
    bxvl = FLT_LARGE;

    for (j = i + 1U; j < (pal->cct); j++){
     if (iquant_boc[j] != 0U){ /* Valid */

      ft = coldiff_w(iquant_bcl[j], iquant_boc[j],
                     iquant_bcl[i], iquant_boc[j], pal->ocs);
      if (ft < bxvl){
       bxvl = ft;
       bxid = j;
      }

     }
    }

    if (bxvl < bmvl){
     bmvl = bxvl;
     bmid = i;
     bmi2 = bxid;
    }

   }
  }

  /* Now "smallest difference" relates to a pair of buckets. Select the one
  ** with the smaller occupation of those to dismantle */

  if (iquant_boc[bmi2] < iquant_boc[bmid]){ bmid = bmi2; }

  /* The bucket to dismantle is now in bmid. Spread out it's colors to the
  ** other buckets by the least difference to the current bucket averages. */

  iquant_boc[bmid] = 0U; /* Invalidate bucket to dismantle by zeroing occupation */

  for (i = 0U; i < (pal->cct); i++){
   if (pal->col[i].wrk == bmid){ /* A color needing a new bucket */

    otid = 0U;
    otvl = 0xFFFFFFFFU;
    for (j = 0U; j < (pal->cct); j++){
     if (iquant_boc[j] != 0U){ /* Bucket is non-empty (and neither the one under dismantling) */
      k = coldiff(iquant_bcl[j], pal->col[i].col);
      if (k < otvl){ /* Difference is smaller, more likely to go here */
       otid = j;
       otvl = k;
      }
     }
    }
    pal->col[i].wrk = otid; /* Reassign color to the other bucket */

   }
  }

  /* OK, one bucket less to go! */

  bcnt--;
  printf("."); /* Just an indicator of progress... */
  fflush(stdout);

 }

 printf("\n");

 /* Now the bucket count is reduced to the desired color count. Create a
 ** proper palette from it. */

 printf("IQuant: Assembling palette of %u colors\n", bcnt);

 iquant_cpal(pal, pdep); /* Just the final bucket averaging: the palette. */
 j = 0U;
 for (i = 0U; i < (pal->cct); i++){ /* Compact palette */

  /* Note that if depth reduction is present, the palette may contain
  ** identical colors. Also remove those for proper palette report and
  ** quantizing. Not entirely right since then pixel counts will still be
  ** wrong... */

  if (iquant_boc[i] != 0U){ /* Bucket valid */
   for (k = 0U; k < j; k++){
    if (pal->col[k].col == iquant_bcl[i]){ break; }
   }
   if (k == j){ /* No previous occurrence */
    pal->col[j].col = iquant_bcl[i];
    pal->col[j].occ = iquant_boc[i];
    printf("Color %3u: 0x%06X (pixels: %u)\n", j, iquant_bcl[i], iquant_boc[i]);
    j ++;
   }
  }
 }
 pal->cct = j; /* Update to true palette size */

}
