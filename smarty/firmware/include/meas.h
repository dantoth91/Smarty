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
  MEAS2_REGEN_BRAKE,
  MEAS2_NUM_CH
};

void measInit(void);
void measCalc(void);

long map(long x, long in_min, long in_max, long out_min, long out_max);

int16_t measGetValue(enum measChannels ch);
int16_t measGetValue2(enum measChannels2 ch);

void meas_throttleSetMin(void);
void meas_throttleSetMax(void);

void meas_regen_brakeSetMin(void);
void meas_regen_brakeSetMax(void);

void cmd_measvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_getThrottle(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_getRegenBrake(BaseSequentialStream *chp, int argc, char *argv[]);

#endif