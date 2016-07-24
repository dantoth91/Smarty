/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/
#ifndef GFX_H_INCLUDED
#define GFX_H_INCLUDED

#include "ch.h"
#include "hal.h"

void read(void);
uint8_t gfx_Cls(void);
uint8_t file_Mount(void);
void media_Image(int X, int Y);
uint8_t media_SetSector(int  HiWord, int  LoWord);
void media_Video(int X, int Y);
uint8_t media_VideoFrame(int  X, int  Y, int  Framenumber);
uint8_t gfx_BGcolour(int Color);
void gfx_Contrast(int Contrast);
uint8_t gfx_Transparency(int  OnOff);
uint8_t gfx_TransparentColour(int  Color);
void file_LoadImageControl(char * Datname, char * GCIName, int Mode);
void putstr(char * InString);
void WriteChars(char * charsout);
uint8_t bus_Read(void);
void bus_In(void);
uint8_t gfxGetDspAns(void);
void gfxGetDspAnsZero(void);
uint8_t draw_line(int  X1, int  Y1,int  X2, int  Y2, int  Color);
void SetBaud115200();

void SerialInit();

#endif

