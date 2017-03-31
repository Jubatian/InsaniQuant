/**
**  \file
**  \brief     InsaniQuant bit depth reduction pass
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


#ifndef COLDEPTH_H
#define COLDEPTH_H

#include "types.h"


/* Trims the passed input color down to the given depth. Note that it does not
** just trims the lowest bits, rather expands so the resulting color space
** still covers the entire 0 - 255 range. Depth can range from 0x111 - 0x888
** (R:G:B depth). */
auint coldepth(auint col, auint dep);


/* Returns the nearest color of the given depth to the passed one by
** difference (as from coldiff). */
auint coldepth_d(auint col, auint dep);


#endif
