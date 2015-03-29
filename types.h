/**
**  \file
**  \brief     Basic type defs & includes
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


#ifndef TYPES_H
#define TYPES_H


#include <stdio.h>              /* printf */
#include <stdlib.h>             /* NULL, malloc, free, etc. */
#include <string.h>             /* Basic block memory routines */
#include <stdint.h>             /* Fixed width integer types */
#include <errno.h>              /* For errors */


typedef   signed  int   asint;  /* Architecture signed integer (At least 2^31) */
typedef unsigned  int   auint;  /* Architecture unsigned integer (At least 2^31) */
typedef  int16_t        sint16;
typedef uint16_t        uint16;
typedef  int32_t        sint32;
typedef uint32_t        uint32;
typedef   int8_t        sint8;
typedef  uint8_t        uint8;



/* Structure for one color with occupation data, and a work field to be used
** by quantizing algorithms */
typedef struct{
 auint col;         /* RGB color value */
 auint occ;         /* Occurrence */
 auint wrk;         /* Work field (for algorithm-related temp. data) */
}iquant_col_t;


/* Structure for a palette of colors */
typedef struct{
 iquant_col_t* col; /* Colors in palette */
 auint cct;         /* Current color count */
 auint mct;         /* Maximal color count (size of the buffer under col) */
 auint ocs;         /* Occurrence sum: total number of pixels */
}iquant_pal_t;


#endif
