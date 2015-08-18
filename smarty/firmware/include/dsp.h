/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/

#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED

#include "ch.h"
#include "hal.h"

void dspInit(void);

bool_t dspGetItem(uint8_t ch);
uint16_t dspGetValue(void);

bool_t dsp_active(void);

void cmd_dspvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmdfrappans_dspmessages(BaseSequentialStream *chp, int argc, char *argv[]);
#endif
