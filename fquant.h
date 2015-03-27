/**
**  \file
**  \brief     InsaniQuant fast reduction pass
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
** This is a simpler (lower quality) quantizer to work on larger amount of
** colors. It simply iteratively merges colors by occurrence weighted
** difference until meeting it's quantization goal.
*/


#ifndef FQUANT_H
#define FQUANT_H

#include "types.h"


/* Maximal color count for the quantizer. */

#define FQUANT_COLS 4096U


/* The fast quantizer pass, reducing the image to the given count of colors.
** It calls the depth reductor first as needed to trim down color count to an
** amount it is capable to work with. The cols parameter is it's output target
** (as quantized image). */
void fquant(uint8 const* buf, uint8* wrk, auint bsiz, auint cols);


#endif
