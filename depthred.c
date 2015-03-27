/**
**  \file
**  \brief     InsaniQuant bit depth reduction pass
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


#include "depthred.h"
#include "colcount.h"



/* Internal depth reduction tables for each depth */
static uint8 depthred_tb[256U * 8U];

/* Whether the table was initialized already */
static auint depthred_ti = 0U;



/* Table initializer function */
static void depthred_tb_init(void)
{
 auint i;

 if (depthred_ti){ return; }

 for (i =   0U; i < 128U; i++){ depthred_tb[0x000U + i] = 0x00U; }
 for (i = 128U; i < 256U; i++){ depthred_tb[0x000U + i] = 0xFFU; }

 for (i =   0U; i <  64U; i++){ depthred_tb[0x100U + i] = 0x00U; }
 for (i =  64U; i < 128U; i++){ depthred_tb[0x100U + i] = 0x55U; }
 for (i = 128U; i < 192U; i++){ depthred_tb[0x100U + i] = 0xAAU; }
 for (i = 192U; i < 256U; i++){ depthred_tb[0x100U + i] = 0xFFU; }

 for (i =   0U; i <  32U; i++){ depthred_tb[0x200U + i] = 0x00U; }
 for (i =  32U; i <  64U; i++){ depthred_tb[0x200U + i] = 0x24U; }
 for (i =  64U; i <  96U; i++){ depthred_tb[0x200U + i] = 0x49U; }
 for (i =  96U; i < 128U; i++){ depthred_tb[0x200U + i] = 0x6DU; }
 for (i = 128U; i < 160U; i++){ depthred_tb[0x200U + i] = 0x92U; }
 for (i = 160U; i < 192U; i++){ depthred_tb[0x200U + i] = 0xB6U; }
 for (i = 192U; i < 224U; i++){ depthred_tb[0x200U + i] = 0xDBU; }
 for (i = 224U; i < 256U; i++){ depthred_tb[0x200U + i] = 0xFFU; }

 for (i = 0U; i < 256U; i++){ depthred_tb[0x300U + i] = (i & 0xF0U) | (i >> 4U); }

 for (i = 0U; i < 256U; i++){ depthred_tb[0x400U + i] = (i & 0xF8U) | (i >> 5U); }

 for (i = 0U; i < 256U; i++){ depthred_tb[0x500U + i] = (i & 0xFCU) | (i >> 6U); }

 for (i = 0U; i < 256U; i++){ depthred_tb[0x600U + i] = (i & 0xFEU) | (i >> 7U); }

 for (i = 0U; i < 256U; i++){ depthred_tb[0x700U + i] = i; }

 depthred_ti = 1U;
}



/* Internal function for reducing depth. */
static void depthred_dr(uint8* buf, auint bsiz, auint depth)
{
 auint i;

 if (depth >  8U){ depth = 8U; }
 if (depth == 0U){ depth = 1U; }

 depth = (depth - 1U) << 8;

 if (!depthred_ti){ depthred_tb_init(); }

 for (i = 0U; i < bsiz; i++){
  buf[(i * 3U) + 0U] = depthred_tb[depth + buf[(i * 3U) + 0U]];
  buf[(i * 3U) + 1U] = depthred_tb[depth + buf[(i * 3U) + 1U]];
  buf[(i * 3U) + 2U] = depthred_tb[depth + buf[(i * 3U) + 2U]];
 }
}



/* Reduces bit depth of the passed image by trimming low bits. The bsiz
** parameter is the RGB image buffer's size in pixels, wrk is the work image
** buffer (to which the reduction takes place). The clipped low bits are not
** populated with zero, rather scaled to let them cover the entire original
** color range (if clipped, the image would slightly darken as a result of the
** quantization which is not desirable). */
void depthred(uint8 const* buf, uint8* wrk, auint bsiz, auint cmax)
{
 auint cc;
 auint dstart = 8U;

 cc = colcount(buf, bsiz);
 printf("Depth reduction: Initial color count: %u (target: %u)\n", cc, cmax);

 if (cc <= cmax){ /* Color count is already OK */
  memcpy(wrk, buf, bsiz * 3U);
  return;
 }

 while ((cmax < cc) && (dstart > 1U)){
  dstart--;
  memcpy(wrk, buf, bsiz * 3U);
  depthred_dr(wrk, bsiz, dstart);
  cc = colcount(wrk, bsiz);
  printf("Depth reduction: Depth: %u, Color count: %u (target: %u)\n", dstart, cc, cmax);
 }
}



/* Trims the passed input color down to the given depth. Note that it does not
** just trims the lowest bits, rather expands so the resulting color space
** still covers the entire 0 - 255 range. Depth can range from 1 - 8. */
auint depthred_col(auint col, auint dep)
{
 if (dep >= 8U){ return col; }
 if (dep == 0U){ dep = 1U; }

 dep = (dep - 1U) << 8;

 return ((auint)(depthred_tb[dep + ((col >> 16) & 0xFFU)]) << 16) |
        ((auint)(depthred_tb[dep + ((col >>  8) & 0xFFU)]) <<  8) |
        ((auint)(depthred_tb[dep + ((col      ) & 0xFFU)])      );
}
