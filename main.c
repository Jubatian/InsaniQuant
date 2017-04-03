/**
**  \file
**  \brief     InsaniQuant main file
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
** Short usage summary:
** insaniquant infile.rgb width heigh colors outfile.rgb [depth] [dither]
*/



#include "types.h"
#include "version.h"
#include "depthred.h"
#include "mquant.h"
#include "palapp.h"
#include "uzebox.h"



/* Application name string */
static char const* main_appname = "InsaniQuant for Uzebox, Version: " IQUANT_VERSION;

/* Other elements */
static char const* main_appauth = "By: Sandor Zsuga (Jubatian)\n";
static char const* main_copyrig = "Copyright: 2013 - 2017, GNU General Public License version 2\n"
                                  "           or any later version, see LICENSE\n";



/* Scan a decimal value (process parameters) */

auint main_sdec(char const* str)
{
 auint r = 0U;
 auint i = 0U;
 while ((str[i] >= '0') && (str[i] <= '9')){
  r = (r * 10U) + (auint)(str[i] - '0');
  i ++;
 }
 return r;
}



/* Main */

int main(int argc, char** argv)
{
 FILE* f_inp;
 FILE* f_out;
 auint par_w;
 auint par_h;
 auint par_c;
 auint par_b;
 auint par_f;
 auint par_d;
 auint par_x;
 void* tptr;
 uint8* img_buf;
 uint8* img_wrk;
 iquant_pal_t pal;
 size_t s_tmp;

 /* Welcome message */

 printf("\n");
 printf("%s", main_appname);
 printf("\n\n");
 printf("%s", main_appauth);
 printf("%s", main_copyrig);
 printf("\n");

 /* Load parameters, trying to open the files as well (note: argv[0] is the
 ** InsaniQuant executable's path) */

 if (argc <= 6){
  printf("Needs at least 6 parameters:\n\n");
  printf("- Input file name (.rgb file, as from ImageMagick)\n");
  printf("- Width of the image in pixels\n");
  printf("- Height of the image in pixels\n");
  printf("- Target color count (2 - 256)\n");
  printf("- Output file name\n");
  printf("- Output format\n");
  printf("- (Optional) Request dithering ('d'), defaults to disabled\n");
  printf("- (Optional) Request text (hexa) output ('h'), defaults to binary\n\n");
  printf("The output formats:\n\n");
  printf("- 0: 16 color: 2 byte Width, 2 byte Height, 16 byte Palette, Data\n\n");
  printf("- 1: 8 color: 2 byte Width, 2 byte Height, 8 byte Palette, Data (M748/5)\n\n");
  printf("In all formats, multi byte values are Little Endian,\n");
  printf("pixel order is high bits corresponding to left pixels.\n\n");
  exit(1);
 }

 par_w = main_sdec(argv[2]);
 par_h = main_sdec(argv[3]);
 par_c = main_sdec(argv[4]);
 par_f = main_sdec(argv[6]);
 par_b = 0x332U;
 par_d = 0U;
 par_x = 0U;
 if (argc > 7){
  if (argv[7][0] == 'd'){ par_d = 1U; }
  if (argv[7][0] == 'h'){ par_x = 1U; }
  if (argc > 8){
   if (argv[8][0] == 'd'){ par_d = 1U; }
   if (argv[8][0] == 'h'){ par_x = 1U; }
  }
 }

 if ((par_w == 0U) || (par_w > 16384U)){
  fprintf(stderr, "Invalid width (%u)\n", par_w);
  exit(1);
 }
 if ((par_h == 0U) || (par_h > 16384U)){
  fprintf(stderr, "Invalid height (%u)\n", par_h);
  exit(1);
 }
 if ((par_c  < 2U) || (par_c > 256U)){
  fprintf(stderr, "Invalid color count (%u)\n", par_c);
  exit(1);
 }

 switch (par_f){ /* Format constraints */

  case 0U: /* 16 color, 2b width, 2b height, palette, data */
   if (par_c > 16U){ par_c = 16U; }
   break;

  case 1U: /* 8 color, 2b width, 2b height, palette, data */
   if (par_c >  8U){ par_c =  8U; }
   break;

  default: /* Unknown format */
   fprintf(stderr, "Unknown format (%u)\n", par_f);
   exit(1);

 }

 f_inp = fopen(argv[1], "rb");
 if (f_inp == NULL){
  perror("Could not open input file");
  exit(1);
 }

 f_out = fopen(argv[5], "wb");
 if (f_out == NULL){
  perror("Could not open output file");
  fclose(f_inp);
  exit(1);
 }

 /* Attempt to allocate buffers, and load the input file in it. */

 tptr = malloc( (par_w * par_h * 3U) +
                (par_w * par_h * 3U) +
                (sizeof(iquant_col_t) * MQUANT_COLS) );
 if (tptr == NULL){
  fprintf(stderr, "Couldn't allocate memory for image (%u bytes)\n", par_w * par_h * 3U);
  fclose(f_inp);
  fclose(f_out);
  exit(1);
 }
 img_buf = (void*)(((uint8*)(tptr)));
 img_wrk = (void*)(((uint8*)(tptr)) + (par_w * par_h * 3U));
 pal.col = (void*)(((uint8*)(tptr)) + (par_w * par_h * 3U) + (par_w * par_h * 3U));
 pal.mct = MQUANT_COLS;

 s_tmp = fread(img_buf, 1, par_w * par_h * 3U, f_inp); /* Note: fits in 32 bit unsigned int due to size limits */
 if ((par_w * par_h * 3U) != (auint)(s_tmp)){
  fprintf(stderr, "Warning: input file size didn't match dimensions! (%u <=> %u size)\n", par_w * par_h * 3U, (auint)(s_tmp));
 }

 fclose(f_inp); /* Don't care about close error on the input... Not my damn problem */

 /* Quantize */

 printf("Starting quantization:\n");
 printf("- Input file ..........: %s\n", argv[1]);
 printf("- Width ...............: %u px\n", par_w);
 printf("- Height ..............: %u px\n", par_h);
 printf("- Target color count ..: %u\n", par_c);
 printf("- Output file .........: %s\n", argv[5]);
 printf("- Target palette depth : %x R:G:B bits\n", par_b);
 printf("- Dithering request ...: %u\n", par_d);
 printf("- Uzebox output format : %u\n", par_f);
 printf("- Hexadecimal output ..: %u\n", par_x);
 printf("\n");

 depthred(img_buf, par_w * par_h, &pal, MQUANT_COLS);
 mquant(&pal, par_c, par_b);
 if (par_d){
  palapp_dither(img_buf, img_wrk, par_w, par_h, &pal);
 }else{
  palapp_flat  (img_buf, img_wrk, par_w, par_h, &pal);
 }

 /* Write back, clean up and exit */

 uzebox_gen(img_wrk, par_w, par_h, &pal, par_f, par_h, f_out);

 free(img_buf);

 if (fclose(f_out)){
  perror("Could not close output file");
  exit(1);
 }

 printf("Quantization complete\n");

 return 0;      /* Proper exit */
}
