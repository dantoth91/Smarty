/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "button.h"

#include "chprintf.h"
#include "dsp.h"

static bool_t cruise_ok;

void buttonInit(void){
	cruise_ok = FALSE;
}

void buttonCalc(void){
	if((dspGetValue(8) == 0) && cruise_ok){
		if (cruiseStatus())
		{
			cruiseDisable();
			cruise_ok = FALSE;
		}
		else
		{
			cruiseEnable();
			cruise_ok = FALSE;
		}
	}
	else if(dspGetValue(8))
	{
		cruise_ok = TRUE;
	}

}

void cmd_buttonvalues(BaseSequentialStream *chp, int argc, char *argv[]){
  
  (void)argc;
  (void)argv;
  int8_t i = 0;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);
      chprintf(chp, "bus_status: ");
      for(i = 1; i < 9; i++) {
          chprintf(chp, "%d ", dspGetValue(i));
      }
      chprintf(chp, "\r\n");
      chThdSleepMilliseconds(100);
  }
}