/**
**  \file
**  \brief     InsaniQuant simple pleasant ditherizer
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


#include "dither.h"
#include "coldiff.h"



/* Ditherizer maximum color count. There is really no need to alter this, so
** it is not exported in the header. */
#define DITHER_COLS 256U



/* Retrieves an RGB color from the image */
static auint dither_iget(uint8 const* buf, auint px)
{
 return (buf[(px * 3U) + 0U] << 16) |
        (buf[(px * 3U) + 1U] <<  8) |
        (buf[(px * 3U) + 2U]      );
}



/* Writes an RGB color to the image */
static void dither_iset(uint8* buf, auint px, auint col)
{
 buf[(px * 3U) + 0U] = (col >> 16) & 0xFFU;
 buf[(px * 3U) + 1U] = (col >>  8) & 0xFFU;
 buf[(px * 3U) + 2U] = (col      ) & 0xFFU;
}



/* Calculates fourth color to complete a set, to average towards a target
** color. The fourth color is obtained from the passed palette, by index.
** 'c0' takes less weight than 'c1' or 'c2': it should be the corner
** neighbor color. */
static auint dither_avg(auint tg, auint c0, auint c1, auint c2, auint const* pal, auint ccnt)
{
 auint r;
 auint g;
 auint b;
 auint c;
 auint i;
 auint t;
 auint md;
 auint mi;

 r = ( (((c0 >> 16) & 0xFFU) *  0x9AU) +
       (((c1 >> 16) & 0xFFU) * 0x133U) +
       (((c2 >> 16) & 0xFFU) * 0x133U) ) >> 8;
 g = ( (((c0 >>  8) & 0xFFU) *  0x9AU) +
       (((c1 >>  8) & 0xFFU) * 0x133U) +
       (((c2 >>  8) & 0xFFU) * 0x133U) ) >> 8;
 b = ( (((c0      ) & 0xFFU) *  0x9AU) +
       (((c1      ) & 0xFFU) * 0x133U) +
       (((c2      ) & 0xFFU) * 0x133U) ) >> 8;

 md = 0xFFFFFFFFU;
 mi = 0U;
 for (i = 0U; i < ccnt; i++){
  c = (((r + ((pal[i] >> 16) & 0xFFU)) >> 2) << 16) |
      (((g + ((pal[i] >>  8) & 0xFFU)) >> 2) <<  8) |
      (((b + ((pal[i]      ) & 0xFFU)) >> 2)      );
  t = coldiff(tg, pal[i]);
  c = coldiff(tg, c) + (t >> 2) + ((t * t) >> 10);
  if (c < md){
   md = c;
   mi = i;
  }
 }

 return mi;
}



/* Ditherizes the image in buf, for a palette obtained from wrk. */
void dither(uint8 const* buf, uint8* wrk, auint wd, auint hg)
{
 auint ccnt = 0U;
 auint bsiz = wd * hg;
 auint i;
 auint j;
 auint c0;
 auint pal[DITHER_COLS];

 /* Collect the colors from the image to build palette */

 printf("Dither: Gathering color data\n");

 for (i = 0U; i < bsiz; i++){ /* Collect */
  c0 = dither_iget(wrk, i);
  for (j = 0U; j < ccnt; j++){
   if (c0 == pal[j]){ /* Already collected color */
    break;
   }
  }
  if ((j == ccnt) && (j != DITHER_COLS)){ /* One more color */
   pal[j] = c0;
   ccnt++;
  }else if (j == DITHER_COLS){
   printf("Dither: Color count exceed at position %u! Aborting.\n", i);
   return;
  }
 }

 /* Quantize the image with dithering applied */

 printf("Dither: Quantizing the image (%u colors)\n", ccnt);

 c0 = dither_iget(buf, 0U);
 c0 = pal[dither_avg(c0, c0, c0, c0, pal, ccnt)];
 dither_iset(wrk, 0U, c0);
 for (i = 1U; i < wd; i++){
  c0 = dither_iget(buf, i);
  c0 = pal[dither_avg(c0, c0, dither_iget(wrk, i - 1U), c0, pal, ccnt)];
  dither_iset(wrk, i, c0);
 }
 for (j = 1U; j < hg; j++){
  c0 = dither_iget(buf, j * wd);
  c0 = pal[dither_avg(c0, c0, dither_iget(wrk, (j - 1U) * wd), c0, pal, ccnt)];
  dither_iset(wrk, j * wd, c0);
  for (i = 1U; i < wd; i++){
   c0 = dither_iget(buf, (j * wd) + i);
   c0 = pal[dither_avg(c0, dither_iget(wrk, ((j - 1U) * wd) + (i - 1U)),
                           dither_iget(wrk, ((j     ) * wd) + (i - 1U)),
                           dither_iget(wrk, ((j - 1U) * wd) + (i     )), pal, ccnt)];
   dither_iset(wrk, (j * wd) + i, c0);
  }
 }
}
