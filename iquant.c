/**
**  \file
**  \brief     InsaniQuant main reduction pass
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


#include "iquant.h"
#include "coldiff.h"
#include "depthred.h"
#include "fquant.h"



/* Structure for one color */
typedef struct{
 auint col;       /* RGB color value */
 auint occ;       /* Occurrence */
 auint bid;       /* Bucket ID the color is sitting in */
}iquant_col_t;

/* All of the colors on the image */
static iquant_col_t iquant_col[IQUANT_COLS];

/* Average colors (going in the palette) for every bucket */
static auint iquant_bcl[IQUANT_COLS];

/* Occupation data for every bucket, for weighting */
static auint iquant_boc[IQUANT_COLS];



/* Retrieves an RGB color from the image */
static auint iquant_iget(uint8 const* buf, auint px)
{
 return (buf[(px * 3U) + 0U] << 16) |
        (buf[(px * 3U) + 1U] <<  8) |
        (buf[(px * 3U) + 2U]      );
}



/* Calculates average colors (palette) for the buckets */
static void iquant_cpal(auint ccnt, auint pdep)
{
 auint i;
 auint j;
 auint r;
 auint g;
 auint b;
 auint c;

 for (i = 0U; i < ccnt; i++){ /* For every bucket */

  if (iquant_boc[i] != 0U){ /* Only unless it was emptied earlier */

   r = 0U;
   g = 0U;
   b = 0U;
   c = 0U;
   for (j = 0U; j < ccnt; j++){ /* For every color in the bucket 'i' */
    if (iquant_col[j].bid == i){
     r += ((iquant_col[j].col >> 16) & 0xFFU) * iquant_col[j].occ;
     g += ((iquant_col[j].col >>  8) & 0xFFU) * iquant_col[j].occ;
     b += ((iquant_col[j].col      ) & 0xFFU) * iquant_col[j].occ;
     c += iquant_col[j].occ;
    }
   }
   if (c != 0U){ /* (The mask with 0xFF shouldn't be necessary) */
    r = ((r + (c >> 1)) / c) & 0xFFU;
    g = ((g + (c >> 1)) / c) & 0xFFU;
    b = ((b + (c >> 1)) / c) & 0xFFU;
   }
   iquant_bcl[i] = depthred_col((r << 16) | (g << 8) | (b), pdep);
   iquant_boc[i] = c;

  }

 }
}



/* The main quantizer pass, reducing the image to the given count of colors.
** It calls the fast quantizer first as needed to trim down color count to an
** amount it is capable to work with. The cols parameter is it's output target
** (as quantized image). The pdep parameter can be used to force a bit depth
** on the palette it generates (1 - 8 bits). */
void iquant(uint8 const* buf, uint8* wrk, auint bsiz, auint cols, auint pdep)
{
 auint ccnt = 0U;
 auint bcnt;
 auint bmid;
 auint bmi2;
 auint bmvl;
 auint bxid;
 auint bxvl;
 auint otid;
 auint otvl;
 auint i;
 auint j;
 auint k;
 auint c0;

 /* Reduction to get a suitable amount of colors */

 fquant(buf, wrk, bsiz, IQUANT_COLS);

 /* Collect the colors from the image with their occurrences */

 printf("IQuant: Gathering color data\n");

 for (i = 0U; i < bsiz; i++){ /* Collect */
  c0 = iquant_iget(wrk, i);
  for (j = 0U; j < ccnt; j++){
   if (c0 == iquant_col[j].col){ /* Already collected color */
    iquant_col[j].occ++;
    break;
   }
  }
  if ((j == ccnt) && (j != IQUANT_COLS)){ /* One more color */
   iquant_col[j].col = c0;
   iquant_col[j].occ = 1U;
   iquant_col[j].bid = j; /* Every color initially drops into own bucket */
   iquant_boc[j] = 1U;    /* Also set bucket occupation, so empty buckets can be skipped later */
   ccnt++;
  }else if (j == IQUANT_COLS){
   printf("IQuant: Color count exceed at position %u! Aborting.\n", i);
   return;
  }
 }

 /* Quantization pass: dismantle one color bucket in each iteration by the
 ** smallest of the largest principle (where the largest small difference to
 ** any out-of-bucket color is the smallest, that bucket will be dismantled).
 ** Do this until achieving the desired color count. */

 printf("IQuant: Reducing color count to %u colors\n", cols);

 bcnt = ccnt;

 while (bcnt > cols){

  /* Calculate the bucket averages and occupation */

  iquant_cpal(ccnt, pdep);

  /* Search for a bucket to dismantle */

  bmid = 0U;
  bmi2 = 0U;
  bmvl = 0xFFFFFFFFU;

  for (i = 0U; i < ccnt; i++){ /* Go through all buckets */
   if (iquant_boc[i] != 0U){ /* Only if the bucket is still valid (contains colors) */

    /* Search the other buckets for smallest difference, weighted with
    ** occupation */

    bxid = 0U;
    bxvl = 0xFFFFFFFFU;

    for (j = i + 1U; j < ccnt; j++){
     if (iquant_boc[j] != 0U){ /* Valid */

      c0 = coldiff_w(iquant_bcl[j], iquant_boc[j],
                     iquant_bcl[i], iquant_boc[j], bsiz);
      if (c0 < bxvl){
       bxvl = c0;
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

  for (i = 0U; i < ccnt; i++){
   if (iquant_col[i].bid == bmid){ /* A color needing a new bucket */

    otid = 0U;
    otvl = 0xFFFFFFFFU;
    for (j = 0U; j < ccnt; j++){
     if (iquant_boc[j] != 0U){ /* Bucket is non-empty (and neither the one under dismantling) */
      k = coldiff(iquant_bcl[j], iquant_col[i].col);
      if (k < otvl){ /* Difference is smaller, more likely to go here */
       otid = j;
       otvl = k;
      }
     }
    }
    iquant_col[i].bid = otid; /* Reassign color to the other bucket */

   }
  }

  /* OK, one bucket less to go! */

  bcnt--;
  printf("."); /* Just an indicator of progress... */
  fflush(stdout);

 }

 printf("\n");

 /* Now the bucket count is reduced to the desired color count. Create a
 ** palette from it, so the image may be processed. */

 printf("IQuant: Assembling palette of %u colors\n", bcnt);

 iquant_cpal(ccnt, pdep); /* Just the final bucket averaging: the palette. */
 j = 0U;
 for (i = 0U; i < ccnt; i++){ /* Compact palette */

  /* Note that if depth reduction is present, the palette may contain
  ** identical colors. Also remove those for proper palette report and
  ** quantizing. Not entirely right since then pixel counts will still be
  ** wrong... */

  if (iquant_boc[i] != 0U){ /* Bucket valid */
   for (k = 0U; k < j; k++){
    if (iquant_bcl[i] == iquant_bcl[k]){ break; }
   }
   if (k == j){ /* No previous occurrence */
    iquant_bcl[j] = iquant_bcl[i];
    printf("Color %3u: 0x%06X (pixels: %u)\n", j, iquant_bcl[i], iquant_boc[i]);
    j ++;
   }
  }
 }
 bcnt = j; /* Update to true palette size */

 /* Palette OK, quantize the image with it */

 printf("IQuant: Quantizing the image (%u colors)\n", bcnt);

 c0 = 0x80000000U;
 bmid = 0U;
 for (i = 0U; i < bsiz; i++){
  k = iquant_iget(buf, i);
  if (c0 != k){ /* Be faster for identical colors */
   c0   = k;
   bmid = 0U;
   bmvl = 0xFFFFFFFFU;
   for (j = 0U; j < bcnt; j++){ /* Get least differing color from palette */
    k = coldiff(iquant_bcl[j], c0);
    if (k < bmvl){
     bmvl = k;
     bmid = j;
    }
   }
  }
  k = iquant_bcl[bmid];
  wrk[(i * 3U) + 0U] = (k >> 16) & 0xFFU;
  wrk[(i * 3U) + 1U] = (k >>  8) & 0xFFU;
  wrk[(i * 3U) + 2U] = (k      ) & 0xFFU;
 }

}
