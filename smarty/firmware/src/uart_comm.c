/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/
#include "ch.h"
#include "hal.h"
#include "ff.h"
#include "uart_comm.h"

/* Display answer message */


//* Serial configuration used for SD3 */
static SerialConfig diag_ser_SD2_cfg = {
    9600,
    0,
    0,
    0,
};


void uart_commInit(void){
  /* Second diag thread uses SD3 to communicate with dashboard (Dsp) */
  sdStart(&SD2, &diag_ser_SD2_cfg);
}

