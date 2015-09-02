/*
    MegaControl - Copyright (C) 2012
    GAMF ECOMarathon Team              
*/

#ifndef SPEED_H_INCLUDED
#define SPEED_H_INCLUDED

#include "ch.h"
#include "hal.h"

void speedInit(void);
void speedCalc(void);

uint32_t speedGetLastPeriod(void);
uint32_t speedGetPeriodNumber(void);
uint32_t speedGetRpm(void);
uint32_t speedGetSpeed(void);
uint32_t speedRPM_TO_KMPH(double rpm);
uint32_t speedKMPH_TO_RPM(double kmph);
void DistanceMeas(void);
void cmd_distancevalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_set_total_kmeter(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_reset_kmeter(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_speedvalues(BaseSequentialStream *chp, int argc, char *argv[]);

static WORKING_AREA(wasave_kmeter_eeprom, 512);
static msg_t save_kmeter_eeprom(void *arg);
uint32_t GetTotalKmeterDistance();
uint32_t GetKmeterDistance();

#endif
