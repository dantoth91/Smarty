/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef CALC_H_INCLUDED
#define CALC_H_INCLUDED

#include "ch.h"
#include "hal.h"

enum calcChannels
{
  CALC_MOTOR_POWER,
  CALC_SUN_POWER,
  CALC_AVG_SPEED,
  CALC_NUM_CH
};

void calcInit(void);
void calcCalc(void);

int32_t calcGetValue(enum calcChannels ch);

uint32_t calcAvgSpeed(void);

void cmd_calcvalues(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
