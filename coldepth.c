/**
**  \file
**  \brief     InsaniQuant bit depth reduction pass
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


#include "coldepth.h"


/* Internal depth reduction tables for each depth */
static uint8 coldepth_tb[256U * 8U];

/* Whether the table was initialized already */
static auint coldepth_ti = 0U;



/* Table initializer function */
static void coldepth_tb_init(void)
{
 auint i;

 if (coldepth_ti){ return; }

 for (i =   0U; i < 128U; i++){ coldepth_tb[0x000U + i] = 0x00U; }
 for (i = 128U; i < 256U; i++){ coldepth_tb[0x000U + i] = 0xFFU; }

 for (i =   0U; i <  64U; i++){ coldepth_tb[0x100U + i] = 0x00U; }
 for (i =  64U; i < 128U; i++){ coldepth_tb[0x100U + i] = 0x55U; }
 for (i = 128U; i < 192U; i++){ coldepth_tb[0x100U + i] = 0xAAU; }
 for (i = 192U; i < 256U; i++){ coldepth_tb[0x100U + i] = 0xFFU; }

 for (i =   0U; i <  32U; i++){ coldepth_tb[0x200U + i] = 0x00U; }
 for (i =  32U; i <  64U; i++){ coldepth_tb[0x200U + i] = 0x24U; }
 for (i =  64U; i <  96U; i++){ coldepth_tb[0x200U + i] = 0x49U; }
 for (i =  96U; i < 128U; i++){ coldepth_tb[0x200U + i] = 0x6DU; }
 for (i = 128U; i < 160U; i++){ coldepth_tb[0x200U + i] = 0x92U; }
 for (i = 160U; i < 192U; i++){ coldepth_tb[0x200U + i] = 0xB6U; }
 for (i = 192U; i < 224U; i++){ coldepth_tb[0x200U + i] = 0xDBU; }
 for (i = 224U; i < 256U; i++){ coldepth_tb[0x200U + i] = 0xFFU; }

 for (i = 0U; i < 256U; i++){ coldepth_tb[0x300U + i] = (i & 0xF0U) | (i >> 4U); }

 for (i = 0U; i < 256U; i++){ coldepth_tb[0x400U + i] = (i & 0xF8U) | (i >> 5U); }

 for (i = 0U; i < 256U; i++){ coldepth_tb[0x500U + i] = (i & 0xFCU) | (i >> 6U); }

 for (i = 0U; i < 256U; i++){ coldepth_tb[0x600U + i] = (i & 0xFEU) | (i >> 7U); }

 for (i = 0U; i < 256U; i++){ coldepth_tb[0x700U + i] = i; }

 coldepth_ti = 1U;
}



/* Trims the passed input color down to the given depth. Note that it does not
** just trims the lowest bits, rather expands so the resulting color space
** still covers the entire 0 - 255 range. Depth can range from 1 - 8. */
auint coldepth(auint col, auint dep)
{
 if (dep >= 8U){ return col; }
 if (dep == 0U){ dep = 1U; }

 if (!coldepth_ti){ coldepth_tb_init(); }

 dep = (dep - 1U) << 8;

 return ((auint)(coldepth_tb[dep + ((col >> 16) & 0xFFU)]) << 16) |
        ((auint)(coldepth_tb[dep + ((col >>  8) & 0xFFU)]) <<  8) |
        ((auint)(coldepth_tb[dep + ((col      ) & 0xFFU)])      );
}
