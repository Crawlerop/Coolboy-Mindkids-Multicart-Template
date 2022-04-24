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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char the_chr[257][16];

int main(int argc, char **argv)
{
  int n_chars;
  int i;
  int skipped = 0;
  int n_opt = 0;
  int checkback = 1;
 
  FILE *inchr, *outchr, *outnam;

  if(argc != 2)
  {
    fputs("syntax: chropt foo.chr\n", stderr);
    return 1;
  }

  inchr = fopen(argv[1], "rb");
  if(inchr == NULL)
  {
    fputs("chropt could not open ", stderr);
    perror(argv[1]);
    return 1;
  }

  fseek(inchr, 0, SEEK_END);
  n_chars = ftell(inchr) / 16;
  rewind(inchr);

  outchr = fopen("out.chr", "wb");
  if(!outchr)
  {
    fclose(inchr);
    perror("chropt could not open chropt.chr");
    return 1;
  }

  outnam = fopen("out.nam", "wb");
  if(!outnam)
  {
    fclose(inchr);
    fclose(outchr);
    perror("chropt could not open chropt.nam");
    return 1;
  }

  if(n_chars > 0)
  {
    for(i = 0; i < 16; i++)
    {
      the_chr[n_opt][i] = fgetc(inchr);
      fputc(the_chr[n_opt][i], outchr);
    }
    n_opt = 1;
    n_chars--;
    fputc(0, outnam);
  }


  while(n_chars > 0)
  {
    int matched = 0;

    /* read character */
    for(i = 0; i < 16; i++)
      the_chr[n_opt][i] = fgetc(inchr);

    for(checkback = 0; checkback < n_opt; checkback++)
    {
      if(!memcmp(the_chr[checkback], the_chr[n_opt], 16))
      {
        fputc(checkback, outnam);
        matched = 1;
        break;
      }
    }
    if(!matched)
    {
      fputc(n_opt, outnam);
      if(n_opt < 256)
      {
        for(i = 0; i < 16; i++)
          fputc(the_chr[n_opt][i], outchr);
        n_opt++;
      }
      else
        skipped++;
    }

    n_chars--;

  }

  fclose(inchr);
  fclose(outnam);

  if(skipped)
  {
    fprintf(stderr, "warning: %d tiles in %s\n"
            "do not match the first 256 unique tiles",
            skipped, argv[1]);
    n_chars = 0;
  }
  /* pad out.chr to 4 KB to please NSA */
  while(n_opt < 256)
  {
    static const unsigned char x_tile[16] =
    {0x44,0xee,0x00,0x00,0x00,0xee,0x44,0x00,
     0x00,0x00,0x7c,0x38,0x7c,0xee,0x44,0x00
    };
    fwrite(x_tile, 1, 16, outchr);
    n_opt++;
  }
  fclose(outchr);
  return 0;
}
