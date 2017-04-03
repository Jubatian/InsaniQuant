/**
**  \file
**  \brief     InsaniQuant Uzebox target formats
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2017, GNU General Public License version 2 or any later
**             version, see LICENSE
**  \date      2017.04.03
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


#include "uzebox.h"
#include "idata.h"



/* Counter for hexa output, 16 hex values per line */
static auint uzebox_hct;



/* Retrieves palette index for image pixel */
static auint uzebox_getcol(uint8 const* buf, auint i, iquant_pal_t const* pal)
{
 auint col = idata_get(buf, i);
 auint j;

 for (j = 0U; j < pal->cct; j++){
  if (col == (pal->col[j].col)){ break; }
 }

 return j;
}



/* Converts color to Uzebox color */
static auint uzebox_toucol(auint col)
{
 auint r = ((col >> 16) & 0xFFU) >> 5;
 auint g = ((col >>  8) & 0xFFU) >> 5;
 auint b = ((col      ) & 0xFFU) >> 6;

 return (r) + (g << 3) + (b << 6);
}



/* Outputs byte to file (hex / binary) */
static void uzebox_out(auint val, auint hex, FILE* f_out)
{
 uint8 c;

 if (hex != 0U){
  fprintf(f_out, " 0x%02X,", val & 0xFFU);
  uzebox_hct ++;
  if (uzebox_hct == 16U){
   uzebox_hct = 0U;
   fprintf(f_out, "\n");
  }
 }else{
  c = val;
  fwrite(&c, 1U, 1U, f_out);
 }
}



/* Outputs a line break to file (hex only) */
static void uzebox_outnl(auint hex, FILE* f_out)
{
 if (hex != 0U){
  if (uzebox_hct != 0U){
   uzebox_hct = 0U;
   fprintf(f_out, "\n");
  }
 }
}



/* Outputs a fixed amount of palette colors (padded with 0xFF) */
static void uzebox_gen_pal(iquant_pal_t const* pal, auint ncol, auint hex, FILE* f_out)
{
 auint i;
 auint col;

 for (i = 0U; i < ncol; i++){

  if (i < (pal->cct)){
   col = uzebox_toucol(pal->col[i].col);
  }else{
   col = 0xFFU;
  }
  uzebox_out(col, hex, f_out);

 }
}



/* Outputs width:height */
static void uzebox_gen_wh(auint wd, auint hg, auint hex, FILE* f_out)
{
 uzebox_out((wd     ) & 0xFFU, hex, f_out);
 uzebox_out((wd >> 8) & 0xFFU, hex, f_out);
 uzebox_out((hg     ) & 0xFFU, hex, f_out);
 uzebox_out((hg >> 8) & 0xFFU, hex, f_out);
}



/* Format 0 output:
** 16 colors, 2b width, 2b height, 16 palette bytes, image data */
static void uzebox_gen_0(uint8 const* buf, auint wd, auint hg, iquant_pal_t const* pal, auint hex, FILE* f_out)
{
 auint i;
 auint c0;
 auint c1;

 uzebox_gen_wh(wd, hg, hex, f_out);
 uzebox_outnl(hex, f_out);
 uzebox_gen_pal(pal, 16U, hex, f_out);
 uzebox_outnl(hex, f_out);

 for (i = 0U; i < (wd * hg); i += 2U){
  c0 = uzebox_getcol(buf, i + 0U, pal);
  c1 = uzebox_getcol(buf, i + 1U, pal);
  if (c0 > 15U){ c0 = 15U; }
  if (c1 > 15U){ c1 = 15U; }
  uzebox_out((c0 << 4) + c1, hex, f_out);
 }
}



/* Format 1 output:
** 8 colors, 2b width, 2b height, 8 palette bytes, image data
** Image data comes in byte triplets: 60006111 62227333 74447555 */
static void uzebox_gen_1(uint8 const* buf, auint wd, auint hg, iquant_pal_t const* pal, auint hex, FILE* f_out)
{
 auint i;
 auint c0;
 auint c1;
 auint c2;
 auint c3;
 auint c4;
 auint c5;
 auint c6;
 auint c7;

 uzebox_gen_wh(wd, hg, hex, f_out);
 uzebox_outnl(hex, f_out);
 uzebox_gen_pal(pal, 8U, hex, f_out);
 uzebox_outnl(hex, f_out);

 for (i = 0U; i < (wd * hg); i += 8U){
  c0 = uzebox_getcol(buf, i + 0U, pal);
  c1 = uzebox_getcol(buf, i + 1U, pal);
  c2 = uzebox_getcol(buf, i + 2U, pal);
  c3 = uzebox_getcol(buf, i + 3U, pal);
  c4 = uzebox_getcol(buf, i + 4U, pal);
  c5 = uzebox_getcol(buf, i + 5U, pal);
  c6 = uzebox_getcol(buf, i + 6U, pal);
  c7 = uzebox_getcol(buf, i + 7U, pal);
  if (c0 > 7U){ c0 = 7U; }
  if (c1 > 7U){ c1 = 7U; }
  if (c2 > 7U){ c2 = 7U; }
  if (c3 > 7U){ c3 = 7U; }
  if (c4 > 7U){ c4 = 7U; }
  if (c5 > 7U){ c5 = 7U; }
  if (c6 > 7U){ c6 = 7U; }
  if (c7 > 7U){ c7 = 7U; }
  uzebox_out( (((c6 >> 2) & 1U) << 7) + (c0 << 4) +
              (((c6 >> 1) & 1U) << 3) + (c1     ), hex, f_out );
  uzebox_out( (((c6     ) & 1U) << 7) + (c2 << 4) +
              (((c7 >> 2) & 1U) << 3) + (c3     ), hex, f_out );
  uzebox_out( (((c7 >> 1) & 1U) << 7) + (c4 << 4) +
              (((c7     ) & 1U) << 3) + (c5     ), hex, f_out );
 }
}



/* Outputs for Uzebox from buf (rgb image data), by the given format, into
** the passed file */
void uzebox_gen(uint8 const* buf, auint wd, auint hg, iquant_pal_t const* pal, auint format, auint hex, FILE* f_out)
{
 uzebox_hct = 0U;

 switch (format){
  case 0U: uzebox_gen_0(buf, wd, hg, pal, hex, f_out); break;
  case 1U: uzebox_gen_1(buf, wd, hg, pal, hex, f_out); break;
  default: break;
 }
}
