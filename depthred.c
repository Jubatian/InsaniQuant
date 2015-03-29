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
 auint cc;

 cc = depthred_cc(buf, bsiz, dep);
 printf("Depth reduction: Initial color count: %u (target: %u)\n", cc, cols);

 while ((cols < cc) && (dep > 1U)){
  dep--;
  cc = depthred_cc(buf, bsiz, dep);
  printf("Depth reduction: Depth: %u, Color count: %u (target: %u)\n", dep, cc, cols);
 }

 palgen(buf, bsiz, pal, dep);
}
