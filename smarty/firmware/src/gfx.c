/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/
    
#include "gfx.h"
#include "dsp.h"
#include "hal.h"
#include "ch.h"

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
#define SETBAUD                   (0x0026)

Mutex SerialMutex;

/* Display answer message */
#define DIAG_DSP_OK (0x06)

static uint8_t dspAns;

uint8_t gfx_Cls(void)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, GFX_CLS >> 8);
  sdPut(&SD2, GFX_CLS);
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

uint8_t file_Mount(void)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, FILE_MOUNT >> 8);
  sdPut(&SD2, FILE_MOUNT);
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  gfxGetDspAns();
  gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

void media_Image(int  X, int  Y)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, MEDIA_IMAGE >> 8);
  sdPut(&SD2, MEDIA_IMAGE);
  sdPut(&SD2, X >> 8);
  sdPut(&SD2, X);
  sdPut(&SD2, Y >> 8);
  sdPut(&SD2, Y);
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  //gfxGetDspAns();
  chMtxUnlock();
}

uint8_t media_SetSector(int  HiWord, int  LoWord)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, MEDIA_SETSECTOR >> 8);
  sdPut(&SD2, MEDIA_SETSECTOR);
  sdPut(&SD2, HiWord >> 8);
  sdPut(&SD2, HiWord);
  sdPut(&SD2, LoWord >> 8);
  sdPut(&SD2, LoWord);
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

void media_Video(int  X, int  Y)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, MEDIA_VIDEO >> 8);
  sdPut(&SD2, MEDIA_VIDEO);
  sdPut(&SD2, X >> 8);
  sdPut(&SD2, X);
  sdPut(&SD2, Y >> 8);
  sdPut(&SD2, Y);
  gfxGetDspAns();
  chMtxUnlock();
}

uint8_t media_VideoFrame(int  X, int  Y, int  Framenumber)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, MEDIA_VIDEOFRAME >> 8);
  sdPut(&SD2, MEDIA_VIDEOFRAME);
  sdPut(&SD2, X >> 8);
  sdPut(&SD2, X);
  sdPut(&SD2, Y >> 8);
  sdPut(&SD2, Y);
  sdPut(&SD2, Framenumber >> 8);
  sdPut(&SD2, Framenumber);
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  chMtxUnlock();
  return ans;
}
uint8_t draw_line(int  X1, int  Y1,int  X2, int  Y2, int  Color)
{
  chMtxLock(&SerialMutex);
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
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

uint8_t gfx_BGcolour(int Color)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, GFX_BGCOLOUR >> 8);
  sdPut(&SD2, GFX_BGCOLOUR);
  sdPut(&SD2, Color >> 8);
  sdPut(&SD2, Color);
  uint8_t ans = gfxGetDspAns();
  gfxGetDspAns();
  gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

void gfx_Contrast(int  Contrast)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, GFX_CONTRAST >> 8);
  sdPut(&SD2, GFX_CONTRAST);
  sdPut(&SD2, Contrast >> 8);
  sdPut(&SD2, Contrast);
  uint8_t ans = gfxGetDspAns();
  while(ans != DIAG_DSP_OK)
    ans = gfxGetDspAns();
  gfxGetDspAns();
  gfxGetDspAns();
  chMtxUnlock();
}

uint8_t gfx_Transparency(int  OnOff)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, GFX_TRANSPARENCY >> 8);
  sdPut(&SD2, GFX_TRANSPARENCY);
  sdPut(&SD2, OnOff >> 8);
  sdPut(&SD2, OnOff);
  uint8_t ans = gfxGetDspAns();
  gfxGetDspAns();
  gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

uint8_t gfx_TransparentColour(int  Color)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, GFX_TRANSPARENTCOLOR >> 8);
  sdPut(&SD2, GFX_TRANSPARENTCOLOR);
  sdPut(&SD2, Color >> 8);
  sdPut(&SD2, Color);
  uint8_t ans = gfxGetDspAns();
  gfxGetDspAns();
  gfxGetDspAns();
  chMtxUnlock();
  return ans;
}

void file_LoadImageControl(char *  Datname, char *  GCIName, int  Mode)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, FILE_LOADIMAGECONTROL >> 8);
  sdPut(&SD2, FILE_LOADIMAGECONTROL);
  sdPut(&SD2, Datname);
  sdPut(&SD2, GCIName);
  sdPut(&SD2, Mode >> 8);
  sdPut(&SD2, Mode);
  gfxGetDspAns();
  chMtxUnlock();
}

void putstr(char *  InString)
{
  chMtxLock(&SerialMutex);
  sdPut(&SD2, PUTSTR >> 8);
  sdPut(&SD2, PUTSTR);
  sdPut(&SD2, InString);
  gfxGetDspAns();
  chMtxUnlock();
}

void WriteChars(char * charsout)
{
  chMtxLock(&SerialMutex);
  unsigned char wk ;
  do
  {
    wk = *charsout++ ;
    sdPut(&SD2, wk) ;
  } while (wk) ;

  chMtxUnlock();
}

uint8_t bus_Read(void)
{
  uint8_t bus[2] = {0, 0};
  if(chMtxTryLock(&SerialMutex)){
    uint8_t ack;
    sdPut(&SD2, BUS_READ >> 8);
    sdPut(&SD2, BUS_READ);
    ack = gfxGetDspAns();
    if (ack == DIAG_DSP_OK)
    {
      sdReadTimeout(&SD2, &bus, 2, 10);
    }
    //else
    //  bus = 0xFF;
    chMtxUnlock();
  }

  return bus[1];
}

void bus_In(void)
{
  sdPut(&SD2, BUS_IN >> 8);
  sdPut(&SD2, BUS_IN);
}

uint8_t gfxGetDspAns(void)
{
  uint8_t ans = 0x00;
  sdReadTimeout(&SD2, &ans, 1, 10);

  return (uint8_t)ans;
}

void gfxGetDspAnsZero(void){
    dspAns = 0;
}

void SetBaud115200(){
  chMtxLock(&SerialMutex);
  sdPut(&SD2, SETBAUD >> 8);
  sdPut(&SD2, SETBAUD);
  sdPut(&SD2, 0x00);
  sdPut(&SD2, 0x09);
  uint8_t ans = gfxGetDspAns();
  chMtxUnlock();
}

void SerialInit(){
  chMtxInit(&SerialMutex);
}
