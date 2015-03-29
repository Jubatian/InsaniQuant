/**
**  \file
**  \brief     InsaniQuant weighted color merge
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


#ifndef CMERGE_H
#define CMERGE_H

#include "types.h"



/* Merges two colors in the palette, to the first, zeroing the occurrence
** of the second. */
void colmerge(iquant_pal_t* pal, auint id0, auint id1);


#endif
