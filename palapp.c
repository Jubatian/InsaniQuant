/**
**  \file
**  \brief     InsaniQuant palette apply
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2017, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2017.03.31
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


#include "palapp.h"
#include "coldiff.h"
#include "idata.h"



/* Calculates fourth color to complete a set, to average towards a target
** color. The fourth color is obtained from the passed palette, by index.
** 'c0' takes less weight than 'c1' or 'c2': it should be the corner
** neighbor color. */
static auint palapp_d_avg(auint tg, auint c0, auint c1, auint c2, iquant_pal_t const* pal, auint dst)
{
 auint r;
 auint g;
 auint b;
 auint c;
 auint i;
 auint t;
 auint md;
 auint mi;

 r = (( ((c0 >> 16) & 0xFFU) +
        ((c1 >> 16) & 0xFFU) +
        ((c2 >> 16) & 0xFFU) ) * 2U) / 3U;
 g = (( ((c0 >>  8) & 0xFFU) +
        ((c1 >>  8) & 0xFFU) +
        ((c2 >>  8) & 0xFFU) ) * 2U) / 3U;
 b = (( ((c0      ) & 0xFFU) +
        ((c1      ) & 0xFFU) +
        ((c2      ) & 0xFFU) ) * 2U) / 3U;

 md = 0xFFFFFFFFU;
 mi = 0U;
 for (i = 0U; i < (pal->cct); i++){
  c = (((r + ((pal->col[i].col >> 16) & 0xFFU)) / 3U) << 16) |
      (((g + ((pal->col[i].col >>  8) & 0xFFU)) / 3U) <<  8) |
      (((b + ((pal->col[i].col      ) & 0xFFU)) / 3U)      );
  t = coldiff(tg, pal->col[i].col);
  c = coldiff(tg, c) + (t >> dst) + ((t * t) >> (dst + 6U));
  if (c < md){
   md = c;
   mi = i;
  }
 }

 return mi;
}



/* Calculates overall difference between 4 colors, used to detect how "flat"
** is the region which is dithered. If the region is not flat, then dithering
** is reduced. */
static auint palapp_d_flr(auint c0, auint c1, auint c2, auint c3)
{
 auint ret;

 ret = coldiff(c0, c1) +
       coldiff(c0, c2) +
       coldiff(c0, c3) +
       coldiff(c1, c2) +
       coldiff(c1, c3) +
       coldiff(c2, c3);

 if (ret < 1024U){ return 0U; }
 if (ret < 4096U){ return 1U; }
 return 2U;
}



/* Ditherizes the image in buf, into wrk. */
void palapp_dither(uint8 const* buf, uint8* wrk, auint wd, auint hg, iquant_pal_t const* pal)
{
 auint i;
 auint j;
 auint c0;
 auint dst;
 auint ddf;

 /* Set dithering strength by palette size */

 if       (pal->cct <=  8U){
  dst = 6U;
 }else if (pal->cct <= 16U){
  dst = 5U;
 }else if (pal->cct <= 32U){
  dst = 4U;
 }else if (pal->cct <= 64U){
  dst = 3U;
 }else{
  dst = 2U;
 }

 /* Quantize the image with dithering applied */

 printf("Dither: Quantizing the image (%u colors)\n", pal->cct);

 c0 = idata_get(buf, 0U);
 c0 = pal->col[palapp_d_avg(c0, c0, c0, c0, pal, dst)].col;
 idata_set(wrk, 0U, c0);
 for (i = 1U; i < wd; i++){
  c0  = idata_get(buf, i);
  ddf = dst - palapp_d_flr(c0, c0, idata_get(buf, i - 1U), idata_get(buf, i - 1U));
  c0  = pal->col[palapp_d_avg(c0, c0, idata_get(wrk, i - 1U), c0, pal, ddf)].col;
  idata_set(wrk, i, c0);
 }
 for (j = 1U; j < hg; j++){
  c0  = idata_get(buf, j * wd);
  ddf = dst - palapp_d_flr(c0, c0, idata_get(buf, (j - 1U) * wd), idata_get(buf, (j - 1U) * wd));
  c0  = pal->col[palapp_d_avg(c0, c0, idata_get(wrk, (j - 1U) * wd), c0, pal, ddf)].col;
  idata_set(wrk, j * wd, c0);
  for (i = 1U; i < wd; i++){
   c0  = idata_get(buf, (j * wd) + i);
   ddf = dst -    palapp_d_flr(c0, idata_get(buf, ((j - 1U) * wd) + (i - 1U)),
                                   idata_get(buf, ((j     ) * wd) + (i - 1U)),
                                   idata_get(buf, ((j - 1U) * wd) + (i     )));
   c0  = pal->col[palapp_d_avg(c0, idata_get(wrk, ((j - 1U) * wd) + (i - 1U)),
                                   idata_get(wrk, ((j     ) * wd) + (i - 1U)),
                                   idata_get(wrk, ((j - 1U) * wd) + (i     )), pal, ddf)].col;
   idata_set(wrk, (j * wd) + i, c0);
  }
 }
}



/* Applies the passed palette on the image flat */
void palapp_flat(uint8 const* buf, uint8* wrk, auint wd, auint hg, iquant_pal_t const* pal)
{
 auint bsiz = wd * hg;
 auint i;
 auint j;
 auint k;
 auint mi;
 auint mv;
 auint c0;

 printf("Flat: Quantizing the image (%u colors)\n", pal->cct);

 c0 = 0x80000000U;
 mi = 0U;
 for (i = 0U; i < bsiz; i++){
  k = idata_get(buf, i);
  if (c0 != k){ /* Be faster for identical colors */
   c0 = k;
   mi = 0U;
   mv = 0xFFFFFFFFU;
   for (j = 0U; j < (pal->cct); j++){ /* Get least differing color from palette */
    k = coldiff(pal->col[j].col, c0);
    if (k < mv){
     mv = k;
     mi = j;
    }
   }
  }
  k = pal->col[mi].col;
  idata_set(wrk, i, k);
 }
}
