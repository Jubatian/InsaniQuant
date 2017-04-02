/**
**  \file
**  \brief     InsaniQuant Uzebox target formats
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2017, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2017.04.02
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


#ifndef UZEBOX_H
#define UZEBOX_H

#include "types.h"


/* Outputs for Uzebox from buf (rgb image data), by the given format, into
** the passed file */
void uzebox_gen(uint8 const* buf, auint wd, auint hg, iquant_pal_t const* pal, auint format, auint hex, FILE* f_out);


#endif
