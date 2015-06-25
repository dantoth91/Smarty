/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef BRAKE_H_INCLUDED
#define BRAKE_H_INCLUDED

#include "ch.h"
#include "hal.h"

void brakeInit(void);
void brakeCalc(void);

void cmd_brakevalues(BaseSequentialStream *chp, int argc, char *argv[]);

#endif