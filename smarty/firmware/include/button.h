/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include "ch.h"
#include "hal.h"

void buttonInit(void);
void buttonCalc(void);

void cmd_buttonvalues(BaseSequentialStream *chp, int argc, char *argv[]);

#endif