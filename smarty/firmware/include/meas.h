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
  MEAS_TEMP1,
  MEAS_TEMP2,
  MEAS_STEERING,
  MEAS_IS_IN_DRIVE,
  MEAS_CHP_B,
  MEAS_CHP_J,
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

void meas_str_angleSetMin(void);
void meas_str_angleSetMax(void);

void mainTime(systime_t maradek_time, uint8_t value);

int16_t measInterpolateNTC(adcsample_t rawvalue);

void cmd_measvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_getThrottle(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_getRegenBrake(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_getSteeringAngle(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_mainValues(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
