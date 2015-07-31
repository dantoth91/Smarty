/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/
    
#include "gfx.h"
#include "dsp.h"

#include "chprintf.h"

#include <string.h>
#include <stdlib.h>

#define GFX_CLS                   (0xFFCD)
#define GFX_BGCOLOUR              (0xFFA4)
#define GFX_CONTRAST              (0xFF9C)
#define GFX_TRANSPARENCY          (0xFFA0)
#define GFX_TRANSPARENTCOLOR      (0xFFA1)
#define FILE_MOUNT                (0xFF03)
#define FILE_LOADIMAGECONTROL     (0x0009)
#define MEDIA_IMAGE               (0xFF8B)
#define MEDIA_SETSECTOR           (0xFF92)
#define MEDIA_VIDEO               (0xFF95)
#define MEDIA_VIDEOFRAME          (0xFF94)
#define PUTSTR                    (0x0018)
#define DRAW_LINE                 (0xFFC8)
#define BUS_READ                  (0xFFCF)
#define BUS_IN                    (0xFFD3)

void gfx_Cls(void)
{
  sdPut(&SD2, GFX_CLS >> 8);
  sdPut(&SD2, GFX_CLS);
}

void file_Mount(void)
{
  sdPut(&SD2, FILE_MOUNT >> 8);
  sdPut(&SD2, FILE_MOUNT);
}

void media_Image(int  X, int  Y)
{
  sdPut(&SD2, MEDIA_IMAGE >> 8);
  sdPut(&SD2, MEDIA_IMAGE);
  sdPut(&SD2, X >> 8);
  sdPut(&SD2, X);
  sdPut(&SD2, Y >> 8);
  sdPut(&SD2, Y);
}

void media_SetSector(int  HiWord, int  LoWord)
{
  sdPut(&SD2, MEDIA_SETSECTOR >> 8);
  sdPut(&SD2, MEDIA_SETSECTOR);
  sdPut(&SD2, HiWord >> 8);
  sdPut(&SD2, HiWord);
  sdPut(&SD2, LoWord >> 8);
  sdPut(&SD2, LoWord);
}

void media_Video(int  X, int  Y)
{
  sdPut(&SD2, MEDIA_VIDEO >> 8);
  sdPut(&SD2, MEDIA_VIDEO);
  sdPut(&SD2, X >> 8);
  sdPut(&SD2, X);
  sdPut(&SD2, Y >> 8);
  sdPut(&SD2, Y);
}

void media_VideoFrame(int  X, int  Y, int  Framenumber)
{
  sdPut(&SD2, MEDIA_VIDEOFRAME >> 8);
  sdPut(&SD2, MEDIA_VIDEOFRAME);
  sdPut(&SD2, X >> 8);
  sdPut(&SD2, X);
  sdPut(&SD2, Y >> 8);
  sdPut(&SD2, Y);
  sdPut(&SD2, Framenumber >> 8);
  sdPut(&SD2, Framenumber);
}
void draw_line(int  X1, int  Y1,int  X2, int  Y2, int  Color)
{
  sdPut(&SD2, DRAW_LINE >> 8);
  sdPut(&SD2, DRAW_LINE);
  sdPut(&SD2, X1 >> 8);
  sdPut(&SD2, X1);
  sdPut(&SD2, Y1 >> 8);
  sdPut(&SD2, Y1);
  sdPut(&SD2, X2 >> 8);
  sdPut(&SD2, X2);
  sdPut(&SD2, Y2 >> 8);
  sdPut(&SD2, Y2);
  sdPut(&SD2, Color >> 8);
  sdPut(&SD2, Color);
}

void gfx_BGcolour(int Color)
{
  sdPut(&SD2, GFX_BGCOLOUR >> 8);
  sdPut(&SD2, GFX_BGCOLOUR);
  sdPut(&SD2, Color >> 8);
  sdPut(&SD2, Color);
}

void gfx_Contrast(int  Contrast)
{
  sdPut(&SD2, GFX_CONTRAST >> 8);
  sdPut(&SD2, GFX_CONTRAST);
  sdPut(&SD2, Contrast >> 8);
  sdPut(&SD2, Contrast);
}

void gfx_Transparency(int  OnOff)
{
  sdPut(&SD2, GFX_TRANSPARENCY >> 8);
  sdPut(&SD2, GFX_TRANSPARENCY);
  sdPut(&SD2, OnOff >> 8);
  sdPut(&SD2, OnOff);
}

void gfx_TransparentColour(int  Color)
{
  sdPut(&SD2, GFX_TRANSPARENTCOLOR >> 8);
  sdPut(&SD2, GFX_TRANSPARENTCOLOR);
  sdPut(&SD2, Color >> 8);
  sdPut(&SD2, Color);
}

void file_LoadImageControl(char *  Datname, char *  GCIName, int  Mode)
{
  sdPut(&SD2, FILE_LOADIMAGECONTROL >> 8);
  sdPut(&SD2, FILE_LOADIMAGECONTROL);
  sdPut(&SD2, Datname);
  sdPut(&SD2, GCIName);
  sdPut(&SD2, Mode >> 8);
  sdPut(&SD2, Mode);
}

void putstr(char *  InString)
{
  sdPut(&SD2, PUTSTR >> 8);
  sdPut(&SD2, PUTSTR);
  sdPut(&SD2, InString);
}

void WriteChars(char * charsout)
{
  unsigned char wk ;
  do
  {
    wk = *charsout++ ;
    sdPut(&SD2, wk) ;
  } while (wk) ;
}

uint16_t bus_Read(void)
{
  uint16_t bus;
  //palClearPad(GPIOA, GPIOA_TXD4);

  sdPut(&SD2, BUS_READ >> 8);
  sdPut(&SD2, BUS_READ);
  //chSequentialStreamRead(&SD2, &bus, 3);
  //sdReadTimeout(&SD2, &bus, 3, 10);
  sdReadTimeout(&SD2, &bus, 3, 10);
  //palSetPad(GPIOA, GPIOA_TXD4);
  return (uint8_t)bus;
}

void bus_In(void)
{
  sdPut(&SD2, BUS_IN >> 8);
  sdPut(&SD2, BUS_IN);
}