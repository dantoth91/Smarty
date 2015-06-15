/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef CRUISE_H_INCLUDED
#define CRUISE_H_INCLUDED

#include "ch.h"
#include "hal.h"

void cruiseInit(void);
void cruiseCalc(void);

void cruiseEnable(void);
void cruiseDisable(void);
bool_t cruiseStatus(void);
void cruiseIncrease(double rpm);
void cruiseReduction(double rpm);
uint8_t cruiseGet(void);

int32_t cruisePID (int16_t Input, int16_t Set, int32_t MaxU, int32_t MinU, double Kp, double Ki, double Kd, int32_t MaxP, int32_t MaxI, int32_t MaxD);


void cmd_cruisevalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_setcruisevalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_cruise(BaseSequentialStream *chp, int argc, char *argv[]);

#endif