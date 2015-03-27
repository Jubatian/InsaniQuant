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
**
**
** This pass iteratively reduces the bit depth of an image by trimming low
** bits until meeting a specific color count. It is used to bring down color
** count to a manageable level from where the more complicated quantizer can
** start off.
*/


#ifndef DEPTHRED_H
#define DEPTHRED_H

#include "types.h"



/* Reduces bit depth of the passed image by trimming low bits. The bsiz
** parameter is the RGB image buffer's size in pixels, wrk is the work image
** buffer (to which the reduction takes place). The clipped low bits are not
** populated with zero, rather scaled to let them cover the entire original
** color range (if clipped, the image would slightly darken as a result of the
** quantization which is not desirable). */
void depthred(uint8 const* buf, uint8* wrk, auint bsiz, auint cmax);

/* Trims the passed input color down to the given depth. Note that it does not
** just trims the lowest bits, rather expands so the resulting color space
** still covers the entire 0 - 255 range. Depth can range from 1 - 8. */
auint depthred_col(auint col, auint dep);


#endif
