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


#include "colmerge.h"



/* Merges two colors in the palette, to the first, zeroing the occurrence
** of the second. */
void colmerge(iquant_pal_t* pal, auint id0, auint id1)
{
 auint r;
 auint g;
 auint b;
 auint c0;

 r   = ((pal->col[id0].col >> 16) & 0xFFU) * pal->col[id0].occ;
 g   = ((pal->col[id0].col >>  8) & 0xFFU) * pal->col[id0].occ;
 b   = ((pal->col[id0].col      ) & 0xFFU) * pal->col[id0].occ;
 c0  = pal->col[id0].occ;
 r  += ((pal->col[id1].col >> 16) & 0xFFU) * pal->col[id1].occ;
 g  += ((pal->col[id1].col >>  8) & 0xFFU) * pal->col[id1].occ;
 b  += ((pal->col[id1].col      ) & 0xFFU) * pal->col[id1].occ;
 c0 += pal->col[id1].occ;
 r   = ((r + (c0 >> 1)) / c0) & 0xFFU;
 g   = ((g + (c0 >> 1)) / c0) & 0xFFU;
 b   = ((b + (c0 >> 1)) / c0) & 0xFFU;
 pal->col[id0].col = (r << 16) | (g << 8) | (b);
 pal->col[id0].occ = c0;
 pal->col[id1].occ = 0U; /* Invalidate the other color */
}
