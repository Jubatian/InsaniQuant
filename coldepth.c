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


#include "coldepth.h"
#include "coldiff.h"


/* Internal depth reduction tables for each depth */
static uint8 coldepth_tb[256U * 8U];
static uint8 coldepth_se[256U * 8U];
static uint8 coldepth_le[256U * 8U];

/* Whether the table was initialized already */
static auint coldepth_ti = 0U;



/* Table initializer function */
static void coldepth_tb_init(void)
{
 auint i;
 auint j;
 auint k;

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

 for (j = 0U; j < 256U * 8U; j += 256U){
  for (i = 0U; i < 256U; i++){
   k = i;
   while (i < coldepth_tb[j + k]){ k--; }
   coldepth_se[j + i] = coldepth_tb[j + k];
   k = i;
   while (i > coldepth_tb[j + k]){ k++; }
   coldepth_le[j + i] = coldepth_tb[j + k];
  }
 }

 coldepth_ti = 1U;
}



/* Trims the passed input color down to the given depth. Note that it does not
** just trims the lowest bits, rather expands so the resulting color space
** still covers the entire 0 - 255 range. Depth can range from 0x111 - 0x888
** (R:G:B depth). */
auint coldepth(auint col, auint dep)
{
 auint rdep;
 auint gdep;
 auint bdep;

 if (dep <= 8U){ dep = (dep) | (dep << 4) | (dep << 8); }
 if (dep == 0x888U){ return col; }

 rdep = (dep >> 8) & 0xFU;
 gdep = (dep >> 4) & 0xFU;
 bdep = (dep     ) & 0xFU;

 if (rdep >= 8U){ rdep = 8U; }
 if (rdep == 0U){ rdep = 1U; }
 if (gdep >= 8U){ gdep = 8U; }
 if (gdep == 0U){ gdep = 1U; }
 if (bdep >= 8U){ bdep = 8U; }
 if (bdep == 0U){ bdep = 1U; }

 if (!coldepth_ti){ coldepth_tb_init(); }

 rdep = (rdep - 1U) << 8;
 gdep = (gdep - 1U) << 8;
 bdep = (bdep - 1U) << 8;

 return ((auint)(coldepth_tb[rdep + ((col >> 16) & 0xFFU)]) << 16) |
        ((auint)(coldepth_tb[gdep + ((col >>  8) & 0xFFU)]) <<  8) |
        ((auint)(coldepth_tb[bdep + ((col      ) & 0xFFU)])      );
}



/* Combines color from R G B */
static auint coldepth_ccom(auint r, auint g, auint b)
{
 return ((r & 0xFFU) << 16) |
        ((g & 0xFFU) <<  8) |
        ((b & 0xFFU)      );
}



/* Returns the nearest color of the given depth to the passed one by
** difference (as from coldiff). */
auint coldepth_d(auint col, auint dep)
{
 auint r;
 auint g;
 auint b;
 auint mv;
 auint mc;
 auint t;
 auint u;
 auint rdep;
 auint gdep;
 auint bdep;

 if (dep <= 8U){ dep = (dep) | (dep << 4) | (dep << 8); }
 if (dep == 0x888U){ return col; }

 rdep = (dep >> 8) & 0xFU;
 gdep = (dep >> 4) & 0xFU;
 bdep = (dep     ) & 0xFU;

 if (rdep >= 8U){ rdep = 8U; }
 if (rdep == 0U){ rdep = 1U; }
 if (gdep >= 8U){ gdep = 8U; }
 if (gdep == 0U){ gdep = 1U; }
 if (bdep >= 8U){ bdep = 8U; }
 if (bdep == 0U){ bdep = 1U; }

 if (!coldepth_ti){ coldepth_tb_init(); }

 rdep = (rdep - 1U) << 8;
 gdep = (gdep - 1U) << 8;
 bdep = (bdep - 1U) << 8;

 r = (col >> 16) & 0xFFU;
 g = (col >>  8) & 0xFFU;
 b = (col      ) & 0xFFU;

 /* Just selects the least different out of the 8 bounding colors by depth
 ** (trying bounds for each component individually) */

 t  = coldepth_ccom(coldepth_se[rdep + r], coldepth_se[gdep + g], coldepth_se[bdep + b]);
 u  = coldiff(col, t);
 mv = u;
 mc = t;
 t  = coldepth_ccom(coldepth_se[rdep + r], coldepth_se[gdep + g], coldepth_le[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }
 t  = coldepth_ccom(coldepth_se[rdep + r], coldepth_le[gdep + g], coldepth_se[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }
 t  = coldepth_ccom(coldepth_se[rdep + r], coldepth_le[gdep + g], coldepth_le[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }
 t  = coldepth_ccom(coldepth_le[rdep + r], coldepth_se[gdep + g], coldepth_se[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }
 t  = coldepth_ccom(coldepth_le[rdep + r], coldepth_se[gdep + g], coldepth_le[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }
 t  = coldepth_ccom(coldepth_le[rdep + r], coldepth_le[gdep + g], coldepth_se[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }
 t  = coldepth_ccom(coldepth_le[rdep + r], coldepth_le[gdep + g], coldepth_le[bdep + b]);
 u  = coldiff(col, t);
 if (mv > u){
  mv = u;
  mc = t;
 }

 return mc;
}
