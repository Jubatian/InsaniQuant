/**
**  \file
**  \brief     InsaniQuant palette apply
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
**
**
** Applies palette to the image either with ditherizing or flat.
*/


#ifndef PALAPP_H
#define PALAPP_H

#include "types.h"



/* Ditherizes the image in buf, into wrk. */
void palapp_dither(uint8 const* buf, uint8* wrk, auint wd, auint hg, iquant_pal_t const* pal);


/* Applies the passed palette on the image flat */
void palapp_flat(uint8 const* buf, uint8* wrk, auint wd, auint hg, iquant_pal_t const* pal);


#endif
