/*
  bmp2nes.c
  Windows .bmp to NES .chr converter
*/

/*

Copyright 2001 Damian Yerrick

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ALLEGRO_DJGPP
/* leave most of Allegro out of the statically linked exe */

BEGIN_JOYSTICK_DRIVER_LIST
END_JOYSTICK_DRIVER_LIST
BEGIN_DIGI_DRIVER_LIST
END_DIGI_DRIVER_LIST
BEGIN_MIDI_DRIVER_LIST
END_MIDI_DRIVER_LIST

BEGIN_GFX_DRIVER_LIST
END_GFX_DRIVER_LIST
BEGIN_COLOR_DEPTH_LIST
  COLOR_DEPTH_8
END_COLOR_DEPTH_LIST

#endif

/**************************************\
* BMP to CHR conversion                *
*                                      *
\**************************************/

void dither_8to2(BITMAP *dst)
{
  static const char dither_matrix[4][4] =
  {
    { 0, 8, 3,11},
    {12, 4,15, 7},
    { 2,10, 1, 9},
    {14, 6,13, 5}
  };

  int x, y;
  int wid, ht;
  unsigned int c, cbase;

  if(!dst)
    return;
  wid = dst->w;
  ht = dst->h;
  for(y = 0; y < ht; y++)
    for(x = 0; x < wid; x++)
    {
      c = (getpixel(dst, x, y) + 3) * 48 / 255; /* rescale pixel */
      cbase = c / 16;
      c %= 16;
      if(c > dither_matrix[y & 3][x & 3])
        cbase++;
      putpixel(dst, x, y, cbase);
    }
}


int convert_BMP2CHR(const char *path, int dither)
{
  BITMAP *bmp = load_bitmap(path, NULL);
  char newpath[1024];

  replace_extension(newpath, path, "chr", 1024);

  if(bmp == 0)
  {
    fprintf(stderr, "could not load bitmap %s\n", path);
    return 1;
  }
  else
  {
    unsigned int height = (bmp->h + 7) / 8 * 8;
    unsigned int width = (bmp->w + 7) / 8 * 8;
    unsigned x, y, cx, cy, cz, curByte;
    FILE *fp = fopen(newpath, "wb");

    if(fp == 0)
    {
      fprintf(stderr, "could not open destination file\n");
      destroy_bitmap(bmp);
      return 1;
    }
    else
    {
      if(dither)
	dither_8to2(bmp);

      for(y = 0; y < height; y += 8) /* for each row of tiles */
      {
        for(x = 0; x < width; x += 8) /* for each tile */
        {
          /* for the Game Boy version, interchange the cz and cy loops */

          for(cz = 0; cz < 2; cz++) /* for each plane */
          {
            for(cy = y; cy < y + 8; cy++) /* for each byte */
            {
              curByte = 0;
              for(cx = x; cx < x + 8; cx++) /* for each bit */
              {
                curByte <<= 1;
                curByte |= (getpixel(bmp, cx, cy) >> cz) & 1;
              }
              fputc(curByte, fp);
            }
          }
        }
      }
      fclose(fp);
    }
    destroy_bitmap(bmp);
  }
  return 0;
}


int main(int argc, const char **argv)
{
  int dither = 0;
  int processed = 0;
  int arg;

  /* using allegro only for bmp file reading
     so don't load most of Allegro
   */
  install_allegro(SYSTEM_NONE, &errno, (int (*)(void (*)(void)))atexit);

  for(arg = 1; arg < argc; arg++)
  {
    if(argv[arg][0] == '-')
      switch(argv[arg][1])
	{
	case 'd':
	  dither = 1;
	  break;
	}
    else if(convert_BMP2CHR(argv[arg++], dither))
      return EXIT_FAILURE;
    else
      processed++;
  }
  if(processed < 0)
  {
    fputs("bmp2nes by Damian Yerrick: convert .bmp or .pcx files to NES .chr\n"
          "syntax: bmp2nes file.bmp file2.bmp (convert from 4-color indexed)\n"
	  "        bmp2nes -d file3.bmp file4.bmp (dither from GIMP grayscale)", stderr);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
END_OF_MAIN();
