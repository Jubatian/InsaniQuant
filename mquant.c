/**
**  \file
**  \brief     InsaniQuant main reduction pass
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2015.04.07
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


#include "mquant.h"
#include "coldiff.h"
#include "coldepth.h"



/* Average colors (going in the palette) for every bucket */
static auint mquant_bcl[MQUANT_COLS];

/* Occupation data for every bucket, for weighting */
static auint mquant_boc[MQUANT_COLS];

/* Disabled buckets for splitting (nonzero: disabled) */
static uint8 mquant_dis[MQUANT_COLS];

/* Bucket split weights */
static float mquant_bxwg[MQUANT_COLS];

/* Color 0 endpoint if the bucket was to be split (palette index) */
static auint mquant_bxc0[MQUANT_COLS];

/* Color 1 endpoint if the bucket was to be split (palette index) */
static auint mquant_bxc1[MQUANT_COLS];

/* Current bucket count */
static auint mquant_bct;

/* All of the weighted differences between colors */
static uint16 mquant_dif[MQUANT_COLS * MQUANT_COLS];

/* Large floating point number to start search at...
** Need to replace to something better. */
#define FLT_LARGE (1.0e30)



/* Calculate occurrences for the buckets */
static void mquant_cocc(iquant_pal_t* pal)
{
 auint i;

 /* Clear all occurrence data */

 for (i = 0U; i < mquant_bct; i++){
  mquant_boc[i] = 0U;
 }

 /* Add up color occurrences */

 for (i = 0U; i < (pal->cct); i++){
  mquant_boc[pal->col[i].wrk] += pal->col[i].occ;
 }
}



/* Color rearrangement. This increases the quality of the median cut by that
** after the cut, the colors will converge towards the best group suiting
** them. */
static void mquant_rearrange(iquant_pal_t* pal, auint pdep)
{
 auint i;
 auint j;
 auint k;
 auint rei;
 auint bxid;
 auint bxvl;
 auint t;
 auint r;
 auint g;
 auint b;
 auint c;
 auint itr;

 if      (mquant_bct <= 16U){ itr = 4U; }
 else if (mquant_bct <= 32U){ itr = 3U; }
 else if (mquant_bct <= 64U){ itr = 2U; }
 else                       { itr = 1U; }

 /* Make sure occurrences are calculated */

 mquant_cocc(pal);

 /* Try some iterations of rearrangement */

 for (k = 0U; k < itr; k++){

  /* Re-arrange colors to nearest buckets */

  rei = 0U; /* Indicates whether anything was changed */

  for (i = 0U; i < (pal->cct); i++){

   bxid = 0U;
   bxvl = 0xFFFFFFFFU;

   for (j = 0U; j < mquant_bct; j++){
    t = coldiff(pal->col[i].col, mquant_bcl[j]);
    if (t < bxvl){
     bxvl = t;
     bxid = j;
    }
   }

   if (pal->col[i].wrk != bxid){
    pal->col[i].wrk = bxid;
    rei = 1U; /* Changed something, so worth iterating */
   }

  }

  /* Try to rebuild the palette by re-averaging buckets. However if a color
  ** would become equal to any other, avoid the change. If a bucket has no
  ** colors nearby, it is not affected (except updating it's occurrence to
  ** zero). */

  for (i = 0U; i < mquant_bct; i++){ /* For every bucket */

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
    t = coldepth_d((r << 16) | (g << 8) | (b), pdep);

    /* Only assign the new color if it is distinct and the bucket's occurrence
    ** didn't decrease too much. */

    if (c >= ((mquant_boc[i] + 1U) / 2U)){
     for (j = 0U; j < mquant_bct; j++){
      if (i != j){
       if (mquant_bcl[j] == t){ break; }
      }
     }
     if (j == mquant_bct){
      mquant_bcl[i] = t;
      mquant_boc[i] = c;
     }
    }

   }

  }

  /* If there is no need to iterate further, stop here */

  if (rei == 0U){ break; }

 }

}



/* Calculate split weight for a bucket. Factors for this are largest color
** difference (Median Cut), bucket's occurrence, and the expectable result
** after split (in terms of how large will be the resulting bucket halves).
** Bucket occurrences and the palette's difference matrix must be prepared */
static void mquant_calcsplitw(iquant_pal_t* pal, auint bid)
{
 auint bxc0 = 0U;
 auint bxc1 = 0U;
 auint bxp0;
 auint bxp1;
 auint bxi0;
 auint bxi1;
 auint i;
 auint j;
 auint b;
 float bxvl;
 float crvl;
 float f0;
 float f1;

 /* Find the split endpoints (bxc0, bxc1, bxvl) */

 bxvl = 0.0;

 for (i = 0U; i < (pal->cct); i++){

  if (pal->col[i].wrk == bid){

   b = i * MQUANT_COLS;
   for (j = i + 1U; j < (pal->cct); j++){

    f0 = (float)(mquant_dif[b + j]);
    if ( (pal->col[j].wrk == bid) && /* Same bucket */
         (f0 > bxvl) ){              /* Larger difference */
     bxvl = f0;
     bxc0 = i;
     bxc1 = j;
    }

   }

  }

 }

 /* Determine criticality of the bucket: if it has colors which differ a lot
 ** from other buckets, it is likely that this one is on the edge of the color
 ** space. This means it should be split, since otherwise parts of the image
 ** may become saturated. */

 crvl = 1.0; /* If there is only one bucket, prevent zero result */

 for (i = 0U; i < (pal->cct); i++){
  if (pal->col[i].wrk == bid){

   for (j = 0U; j < mquant_bct; j++){
    if (j != bid){

     f0 = (float)(coldiff(pal->col[i].col, mquant_bcl[j]));
     if (f0 > crvl){
      crvl = f0;
     }

    }
   }

  }
 }

 /* Get how balanced this split is: a balanced split (where both halves get
 ** similar occurrences) is preferrable. */

 bxp0 = 0U;
 bxp1 = 0U;

 for (i = 0U; i < (pal->cct); i++){

  if (pal->col[i].wrk == bid){

   if      (bxc0 < i){ f0 = mquant_dif[(bxc0 * MQUANT_COLS) + i]; }
   else if (bxc0 > i){ f0 = mquant_dif[(i * MQUANT_COLS) + bxc0]; }
   else              { f0 = 0.0; }
   if      (bxc1 < i){ f1 = mquant_dif[(bxc1 * MQUANT_COLS) + i]; }
   else if (bxc1 > i){ f1 = mquant_dif[(i * MQUANT_COLS) + bxc1]; }
   else              { f1 = 0.0; }

   if (f0 < f1){ bxp0 += pal->col[i].occ; }
   else        { bxp1 += pal->col[i].occ; }

  }

 }

 if      (bxp0 < bxp1){ f0 = (float)(bxp0) / (float)(bxp1); }
 else if (bxp1 < bxp0){ f0 = (float)(bxp1) / (float)(bxp0); }
 else                 { f0 = 1.0; }

 /* Assemble result */

 f1 =      (bxvl * bxvl) * (bxvl * bxvl) * bxvl;
 f1 = f1 * (crvl * crvl);
 f1 = f1 * (float)(mquant_boc[bid]) * (float)(mquant_boc[bid]);
 f1 = f1 * f0 * (1.0 + (f0 * 0.8));

 /* Adjust the weight by luminosity endpoints: if the bucket is such an
 ** endpoint (in any time there are two of these, if there are at least 2
 ** buckets), it is more likely to be split. It is quite critical to cover
 ** the luminosity range of the image, this part ensures that. */

 bxp0 = 0U;     /* Luma high end */
 bxi0 = 0U;
 bxp1 = 65535U; /* Luma low end */
 bxi1 = 0U;

 for (i = 0U; i < mquant_bct; i++){
  b = coldiff_getlum(mquant_bcl[i]);
  if (b <= bxp1){
   bxp1 = b;
   bxi1 = i;
  }
  if (b >= bxp0){
   bxp0 = b;
   bxi0 = i;
  }
 }

 if (bid == bxi0){ f1 = f1 * (bxvl * bxvl * bxvl * 0.0000001 + 1.0); }
 if (bid == bxi1){ f1 = f1 * (bxvl * bxvl * bxvl * 0.0000001 + 1.0); }

 /* Write out result */

 mquant_bxwg[bid] = f1;
 mquant_bxc0[bid] = bxc0;
 mquant_bxc1[bid] = bxc1;
}



/* The main quantizer pass, reducing the occurrence weighted palette to the
** given count of colors. The pdep parameter can be used to force a bit depth
** on the palette it generates (1 - 8 bits). */
void mquant(iquant_pal_t* pal, auint cols, auint pdep)
{
 auint bxid;
 float bxvl;
 auint bxc0;
 auint bxc1;
 auint i;
 auint j;
 auint k;

 /* Check if palette can be used */

 if ((pal->cct) > MQUANT_COLS){
  printf("MQuant: Color count exceed (%u > %u)! Aborting.\n", pal->cct, MQUANT_COLS);
  return;
 }

 /* Initialize work data: bucket assingments */

 for (i = 0U; i < (pal->cct); i++){
  pal->col[i].wrk = 0U; /* Every color initially goes into the same bucket */
 }
 mquant_bct = 1U; /* Start with one bucket */
 mquant_bcl[0] = 0U;

 /* Quantization pass: Median Cut with a twist: after every iteration, the
 ** colors are re-arranged to fit the new bucket layout better */

 printf("MQuant: Reducing color count to %u colors\n", cols);

 /* Pre-calculate the color difference matrix */

 for (i = 0U; i < (pal->cct); i++){
  k = i * MQUANT_COLS;
  for (j = i + 1U; j < (pal->cct); j++){
   mquant_dif[k + j] = coldiff(pal->col[j].col, pal->col[i].col);
  }
 }

 /* Quantization loop: Ideally produce the requested number of colors, however
 ** it is possible that the image just doesn't contain enough distinct colors
 ** to do it, then it will bail out sooner. */

 while (mquant_bct < cols){

  mquant_cocc(pal); /* Calculate occurrences */

  for (i = 0U; i < mquant_bct; i++){ /* Calculate split weights */
   mquant_calcsplitw(pal, i);
  }

  /* Try splitting until finding a split which produces new colors */

  for (i = 0U; i < mquant_bct; i++){
   mquant_dis[i] = 0U; /* All buckets enabled for splitting */
  }

  bxid = MQUANT_COLS; /* Indicates failed split at the end of the loop */

  while(1){

   /* Search for the bucket containing the biggest color difference: that will
   ** need splitting (Median Cut). */

   bxid = MQUANT_COLS; /* Not found mark */
   bxvl = 0.0;

   for (i = 0U; i < mquant_bct; i++){

    if (mquant_dis[i] == 0U){

     if (mquant_bxwg[i] > bxvl){
      bxvl = mquant_bxwg[i];
      bxid = i;
     }

    }

   }

   if (bxid == MQUANT_COLS){ break; } /* Can not split any more */

   bxc0 = mquant_bxc0[bxid];
   bxc1 = mquant_bxc1[bxid];
   bxc0 = coldepth_d(pal->col[bxc0].col, pdep);
   bxc1 = coldepth_d(pal->col[bxc1].col, pdep);

   /* Check if both of the colors are new. If so, the split is OK, otherwise
   ** something else has to be tried. */

   for (i = 0U; i < mquant_bct; i++){
    if (i != bxid){ /* Both must differ from all the other buckets */
     if ( (bxc0 == mquant_bcl[i]) ||
          (bxc1 == mquant_bcl[i]) ){ break; }
    }
   }

   if ( (i == mquant_bct) &&
        (bxc0 != bxc1) ){ /* OK, nothing identical. Add new bucket. */

    mquant_bcl[bxid] = bxc0;
    mquant_bcl[mquant_bct] = bxc1;
    mquant_bct ++; /* One bucket added */
    break;         /* All fine, done */

   }else{          /* Need to try again */

    mquant_dis[bxid] = 1U; /* Disable, so something else will be split */

   }

  }

  /* Now either a new bucket was created, or the thing is over. */

  if (bxid == MQUANT_COLS){ break; } /* Can not proceed any more */

  /* Calculate the bucket averages and occupation, rearranging entries as
  ** beneficial to fit with the new set of colors. Note that in this process
  ** no color equivalence may occur. */

  mquant_rearrange(pal, pdep);

  printf("."); /* Just an indicator of progress... */
  fflush(stdout);

 }

 printf("\n");

 /* Now the bucket count is reduced to the desired color count. Create a
 ** proper palette from it. */

 printf("MQuant: Assembling palette of %u colors\n", mquant_bct);

 mquant_rearrange(pal, pdep); /* Just the final bucket averaging: the palette. */

 for (i = 0U; i < mquant_bct; i++){ /* Compact palette */
  pal->col[i].col = mquant_bcl[i];
  pal->col[i].occ = mquant_boc[i];
  printf("Color %3u: 0x%06X (pixels: %u)\n", i, mquant_bcl[i], mquant_boc[i]);
 }
 pal->cct = mquant_bct; /* Update to true palette size */

}
