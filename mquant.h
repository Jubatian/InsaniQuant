/**
**  \file
**  \brief     InsaniQuant main reduction pass
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2015.04.01
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
** This is the major pass of the quantizer, performing quantization by a
** Median Cut algorithm.
*/


#ifndef MQUANT_H
#define MQUANT_H

#include "types.h"


/* Maximal color count for the quantizer. A larger count increases memory and
** processing time requirements by a square factor. */

#define MQUANT_COLS 2048U


/* The main quantizer pass, reducing the occurrence weighted palette to the
** given count of colors. The pdep parameter can be used to force a bit depth
** on the palette it generates (1 - 8 bits). */
void mquant(iquant_pal_t* pal, auint cols, auint pdep);


#endif
