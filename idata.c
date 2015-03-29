/**
**  \file
**  \brief     InsaniQuant image data tools
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


#include "idata.h"



/* Retrieves color value at a given pixel index (linear) from the image
** buffer. */
auint idata_get(uint8 const* buf, auint i)
{
 return (buf[(i * 3U) + 0U] << 16) |
        (buf[(i * 3U) + 1U] <<  8) |
        (buf[(i * 3U) + 2U]      );
}


/* Sets a color value at a given pixel index (linear) into the image
** buffer. */
void idata_set(uint8* buf, auint i, auint col)
{
 buf[(i * 3U) + 0U] = (col >> 16) & 0xFFU;
 buf[(i * 3U) + 1U] = (col >>  8) & 0xFFU;
 buf[(i * 3U) + 2U] = (col      ) & 0xFFU;
}
