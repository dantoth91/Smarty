/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/

#include "uart_comm.h"
#include "meas.h"
#include "gfx.h"
#include "dsp.h"

#include "chprintf.h"

/* Display answer message */
#define DIAG_DSP_OK (0x06)


//Color
#define BLACK                     (0x0000)

enum dspState
{
  DSP_WAITING,
  DSP_RUNNING
}dspstate;

enum dspMessages
{
  DSP_SPEED,
  DSP_x10_SPEED,
  DSP_x1_SPEED,
  DSP_x10_AVG_SPEED,
  DSP_x1_AVG_SPEED,
  DSP_x01_AVG_SPEED,
  DSP_x100_CHT,
  DSP_x10_CHT,
  DSP_x1_CHT,
  DSP_x10_VOLTAGE,
  DSP_x1_VOLTAGE,
  DSP_x01_VOLTAGE,
  DSP_x10_ETM,
  DSP_x1_ETM,
  DSP_NUM_MSG
}dspmessages;


void dspInit(void){
  gfx_Cls();

}

void dspCalc(void){
  gfx_Cls();

}
