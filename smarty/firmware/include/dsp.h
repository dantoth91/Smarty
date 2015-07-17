/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/

#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED

#include "ch.h"
#include "hal.h"

void dspInit(void);
bool_t dsp_active(void);
void cmdfrappans_dspmessages(BaseSequentialStream *chp, int argc, char *argv[]) ;
bool_t dspGetValue(uint8_t ch);

void cmd_dspvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_dspmessages(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_dspbites(BaseSequentialStream *chp, int argc, char *argv[]);
#endif
