/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/
#ifndef GFX_H_INCLUDED
#define GFX_H_INCLUDED

#include "ch.h"
#include "hal.h"

void gfx_Cls(void);
void file_Mount(void);
void media_Image(int X, int Y);
void media_SetSector(int HiWord, int LoWord);
void media_Video(int X, int Y);
void media_VideoFrame(int X, int Y, int Framenumber);
void gfx_BGcolour(int Color);
void gfx_Contrast(int Contrast);
void gfx_Transparency(int OnOff);
void gfx_TransparentColour(int Color);
void file_LoadImageControl(char * Datname, char * GCIName, int Mode);
void media_VideoFrame(int X, int Y, int Framenumber);
void putstr(char * InString);
void WriteChars(char * charsout);

#endif

