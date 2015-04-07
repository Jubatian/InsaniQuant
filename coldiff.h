/**
**  \file
**  \brief     InsaniQuant color difference calculation
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2015.04.07
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


#ifndef COLDIFF_H
#define COLDIFF_H

#include "types.h"


/* Returns a luminosity value for the color, between 0 and 65535 */
auint coldiff_getlum(auint c0);

/* Normal color difference calculation between two RGB colors. Returns a
** difference value between 0 and 4096. */
auint coldiff(auint c0, auint c1);


#endif
