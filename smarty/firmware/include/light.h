/*
    MegaControl - Copyright (C) 2012
    GAMF ECOMarathon Team              
*/

#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED

#include <math.h>
#include "ch.h"
#include "hal.h"

void lightInit(void);
void lightCalc(void);
void lightFlashing (int chanel);
void lightBrakeOn();
void lightBrakeOff();
void lightPosLampOn();
void lightPosLampOff();
bool_t getLightFlashing (uint8_t chanel);

void cmd_lightvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_lightblink(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_getLight(BaseSequentialStream *chp, int argc, char *argv[]);
#endif
