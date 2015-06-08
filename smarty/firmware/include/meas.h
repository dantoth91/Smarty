/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef MEAS_H_INCLUDED
#define MEAS_H_INCLUDED

#include "ch.h"
#include "hal.h"

enum measChannels
{
  MEAS_UBAT,
  MEAS_BRAKE_PRESSURE1,
  MEAS_BRAKE_PRESSURE2,
  MEAS_SEN2,
  MEAS_SEN3,
  MEAS_SEN4,
  MEAS_SEN5,
  MEAS_NUM_CH
};

enum measChannels2
{
  MEAS2_CURR1,
  MEAS2_THROTTLE,
  MEAS2_NUM_CH
};

void measInit(void);
void measCalc(void);
int16_t measGetValue(enum measChannels ch);
int16_t measGetValue2(enum measChannels2 ch);

void cmd_measvalues(BaseSequentialStream *chp, int argc, char *argv[]);

#endif