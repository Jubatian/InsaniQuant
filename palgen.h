/**
**  \file
**  \brief     InsaniQuant palette from image routine
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


#ifndef PALGEN_H
#define PALGEN_H

#include "types.h"



/* Generates palette from the passed image data with occurrence data,
** targeting a given depth. Uses the mct member to limit color count.
** Returns nonzero if successful, zero otherwise (image has more colors than
** fitting in the palette). */
auint palgen(uint8 const* buf, auint bsiz, iquant_pal_t* pal, auint depth);


#endif
