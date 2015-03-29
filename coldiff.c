/**
**  \file
**  \brief     InsaniQuant color difference calculation
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


#include "types.h"
#include "coldiff.h"



/* Factors for counting in Hue, Saturation and Luma. Keep around 0x100. */
#define HUE_DIFF 0x120U
#define SAT_DIFF 0x050U
#define LUM_DIFF 0x180U



/* Saturation rescaling table. Adjusts saturation by that visually with very
** saturated colors, the same saturation difference is less apparent than with
** low saturations (for example there is barely any visual difference between
** a 90% saturated color and a 100% saturated one) */
static uint8 coldiff_sattb[256];

/* Hue rescaling table. Adjusts the hue circle so it is evenly spaced out by
** human perception. The key input points for the hue circle are as follows:
** 0x00: Red
** 0x35: Yellow
** 0x6A: Green
** 0x80: Cyan
** 0xB5: Blue
** 0xEA: Purple */
static uint8 coldiff_huetb[256];

/* Table initialization marker */
static auint coldiff_tbi = 0U;



/* Prepare the tables */
static void coldiff_tb_init(void)
{
 auint i;

 if (coldiff_tbi){ return; }

 /* Saturation rescaling table */

 for (i = 0x00U; i <  0x55U; i++){
  coldiff_sattb[i] = 0x00U + (( i          * 3U) >> 1);
 }
 for (i = 0x55U; i <  0x96U; i++){
  coldiff_sattb[i] = 0x7FU + (((i - 0x55U) * 5U) >> 2);
 }
 for (i = 0x96U; i <  0xB6U; i++){
  coldiff_sattb[i] = 0xD0U +   (i - 0x96U);
 }
 for (i = 0xB6U; i <  0xC6U; i++){
  coldiff_sattb[i] = 0xF0U + ( (i - 0xB6U)       >> 1);
 }
 for (i = 0xC6U; i <  0xF6U; i++){
  coldiff_sattb[i] = 0xF8U + ( (i - 0xC6U)       >> 3);
 }
 for (i = 0xF6U; i < 0x100U; i++){
  coldiff_sattb[i] = 0xFFU;
 }

 /* Hue rescaling table */

 for (i = 0x00U; i <  0x15U; i++){ /* Red -> Yellow begin */
  coldiff_huetb[i] = ((( 0x10U - 0x00U) * (i - 0x00U)) / 0x10U) + 0x00U;
 }
 for (i = 0x15U; i <  0x35U; i++){ /* Red -> Yellow end */
  coldiff_huetb[i] = ((( 0x35U - 0x10U) * (i - 0x15U)) / 0x25U) + 0x10U;
 }
 for (i = 0x35U; i <  0x55U; i++){ /* Yellow -> Green begin */
  coldiff_huetb[i] = ((( 0x5AU - 0x35U) * (i - 0x35U)) / 0x25U) + 0x35U;
 }
 for (i = 0x55U; i <  0x6AU; i++){ /* Yellow -> Green end */
  coldiff_huetb[i] = ((( 0x6AU - 0x5AU) * (i - 0x55U)) / 0x10U) + 0x5AU;
 }
 for (i = 0x6AU; i <  0x75U; i++){ /* Green -> Cyan begin */
  coldiff_huetb[i] = ((( 0x72U - 0x6AU) * (i - 0x6AU)) / 0x08U) + 0x6AU;
 }
 for (i = 0x75U; i <  0x80U; i++){ /* Green -> Cyan end */
  coldiff_huetb[i] = ((( 0x80U - 0x72U) * (i - 0x75U)) / 0x0EU) + 0x72U;
 }
 for (i = 0x80U; i <  0xA5U; i++){ /* Cyan -> Blue begin */
  coldiff_huetb[i] = ((( 0xAAU - 0x80U) * (i - 0x80U)) / 0x2AU) + 0x80U;
 }
 for (i = 0xA5U; i <  0xB5U; i++){ /* Cyan -> Blue end */
  coldiff_huetb[i] = ((( 0xB5U - 0xAAU) * (i - 0xA5U)) / 0x0BU) + 0xAAU;
 }
 for (i = 0xB5U; i <  0xC5U; i++){ /* Blue -> Purple begin */
  coldiff_huetb[i] = ((( 0xC0U - 0xB5U) * (i - 0xB5U)) / 0x0BU) + 0xB5U;
 }
 for (i = 0xC5U; i <  0xEAU; i++){ /* Blue -> Purple end */
  coldiff_huetb[i] = ((( 0xEAU - 0xC0U) * (i - 0xC5U)) / 0x2AU) + 0xC0U;
 }
 for (i = 0xEAU; i <  0xF5U; i++){ /* Purple -> Red begin */
  coldiff_huetb[i] = ((( 0xF8U - 0xEAU) * (i - 0xEAU)) / 0x0EU) + 0xEAU;
 }
 for (i = 0xF5U; i < 0x100U; i++){ /* Purple -> Red end */
  coldiff_huetb[i] = (((0x100U - 0xF8U) * (i - 0xF5U)) / 0x08U) + 0xF8U;
 }

 coldiff_tbi = 1U;
}



/* Calculate balanced hue, and the saturation of the color. This balancing
** means a weighting so that colors differing more to human eye get a greater
** arch of the hue circle.
** The ranges:
** 0x00 - 0x35: Red <-> Yellow
** 0x35 - 0x6A: Yellow <-> Green
** 0x6A - 0x80: Green <-> Cyan (Not too visible difference)
** 0x80 - 0xB5: Cyan <-> Blue
** 0xB5 - 0xEA: Blue <-> Purple
** 0xEA - 0x00: Purple <-> Red (Not too visible difference)
** The saturation comes from the greatest difference along components, which
** is used for the Hue calculation as well. */
static void coldiff_huesat(auint c, auint* h, auint* s)
{
 auint r, g, b;
 auint d;

 if (!coldiff_tbi){ coldiff_tb_init(); }

 r = (c >> 16) & 0xFFU;
 g = (c >>  8) & 0xFFU;
 b = (c      ) & 0xFFU;

 if (r > g){                   /* Red <-> Yellow or
                               ** Blue <-> Purple or
                               ** Purple <-> Red */
  if (g > b){                  /* (r >  g >  b) Red <-> Yellow */
   d  = r - b;
   *h = (((g - b) * 0x35U) / d) + 0x00U;
  }else{                       /* Blue <-> Purple or
                               ** Purple <-> Red */
   if (b > r){                 /* (b >  r >  g) Blue <-> Purple */
    d  = b - g;
    *h = (((r - g) * 0x35U) / d) + 0xB5U;
   }else{                      /* (r >  b >= g) Purple <-> Red */
    d  = r - g;
    *h = ((((r - b) * 0x16U) / d) + 0xEAU) & 0xFFU;
   }
  }
 }else{                        /* Yellow <-> Green or
                               ** Green <-> Cyan or
                               ** Cyan <-> Blue */
  if (b > g){                  /* (b >  g >= r) Cyan <-> Blue */
   d  = b - r;
   *h = (((b - g) * 0x35U) / d) + 0x80U;
  }else{                       /* Yellow <-> Green or
                               ** Green <-> Cyan */
   if (r > b){                 /* (g >= r >  b) Yellow <-> Green */
    d  = g - b;
    *h = (((g - r) * 0x35U) / d) + 0x35U;
   }else{                      /* (g >= b >= r) Green <-> Cyan */
    d  = g - r;
    if (d){ *h = (((b - r) * 0x16U) / d) + 0x6AU; }
    else  { *h = 0; }
   }
  }
 }

 *h = coldiff_huetb[*h];
 *s = coldiff_sattb[ d];
}




/* Normal color difference calculation between two RGB colors. Returns a
** difference value between 0 and 4096. */
auint coldiff(auint c0, auint c1)
{
 auint h0;
 auint s0;
 auint h1;
 auint s1;
 auint g0;
 auint g1;
 auint r;
 auint t;

 /* Shortcut when comparing identical colors */

 if (((c0 ^ c1) & 0xFFFFFFU) == 0U){ return 0U; }

 /* Hue / Saturation difference */

 coldiff_huesat(c0, &h0, &s0);
 coldiff_huesat(c1, &h1, &s1);

 g0 = (h0 - h1) & 0xFFU;       /* Hues - circular difference is needed */
 g1 = (h1 - h0) & 0xFFU;       /* (Maximal diff. is 128 for this) */
 if (g0 < g1){ r = g0; }
 else        { r = g1; }
 if (s0 < s1){
  t = s1 - s0;                 /* Saturation difference (0 - 255) */
  r = r * s0;
 }else{
  t = s0 - s1;                 /* Saturation difference (0 - 255) */
  r = r * s1;
 }
 r  = (r * HUE_DIFF) >> 7;     /* Hue difference (0 - 65536) */
 r += (t * SAT_DIFF);          /* Saturation difference (0 - 65536) */

 /* Greyscale difference */

 g0 = (((c0 >> 16) & 0xFFU) *  98U) +
      (((c0 >>  8) & 0xFFU) * 128U) +
      (((c0      ) & 0xFFU) *  30U);
 g1 = (((c1 >> 16) & 0xFFU) *  98U) +
      (((c1 >>  8) & 0xFFU) * 128U) +
      (((c1      ) & 0xFFU) *  30U);
 t = abs((asint)(g0) - (asint)(g1));
 r += (t * LUM_DIFF) >> 8;     /* Luminosity difference (0 - 65536) */

 return (r >> 6);
}



/* Weighted color difference calculation: uses the occurrence data relative
** to the image total size to weight difference (covering larger area makes
** differences larger). */
float coldiff_w(auint c0, auint p0, auint c1, auint p1, auint bsiz)
{
 auint cdf = coldiff(c0, c1);
 auint wgt = p0 + p1;

 /* Some explanations on this part:
 ** Two important characteristics of the quantized image battle here: the
 ** preservation of smaller distinctly colored areas, and the faithful
 ** reproduction of relatively large areas, especially gradients. For the
 ** former, the color difference raised to a power works. The latter is
 ** simple weighting. Note that the result is only used in smaller / larger
 ** comparisons, so scaling is not important. */

 return (float)(cdf) * (float)(cdf) * (float)(cdf) * (float)(cdf) *
        (1.0 + ((float)(cdf) * 0.5)) *
        (float)(wgt);
}
