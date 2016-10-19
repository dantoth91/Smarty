/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef SAFETY_H_INCLUDED
#define SAFETY_H_INCLUDED

#include "ch.h"
#include "hal.h"

void safetyInit(void);

void cmd_safetyvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fan_speed(BaseSequentialStream *chp, int argc, char *argv[]);

#endif