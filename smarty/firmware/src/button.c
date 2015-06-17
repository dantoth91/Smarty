/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "button.h"

#include "chprintf.h"
#include "dsp.h"
#include "speed.h"
#include "cruise.h"

#define CRUISE_SPEED_STEP	1
#define CRUISE_FAST_STEP	10
#define PRESS_PERIOD		50

static bool_t cruise_ok;
static bool_t cruise_plusz;
static uint16_t cruise_long_plusz;
static bool_t cruise_minusz;
static uint16_t cruise_long_minusz;

static bool_t index_right;
static bool_t index_left;
static bool_t lamp_ok;

void buttonInit(void){
	cruise_ok = FALSE;
	cruise_plusz = FALSE;
	cruise_long_plusz = 0;
	cruise_minusz = FALSE;
	cruise_minusz = FALSE;
	/* Még nincs bekötve az index nyomógomb */
	/*index_right = FALSE;
	index_left = FALSE;*/
	lamp_ok = FALSE;
}

void buttonCalc(void){

/* Cruise controll activated */
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

/* Cruise controll increase */
	if((dspGetValue(4) == 0) && cruise_plusz){
		cruiseIncrease(speedKMPH_TO_RPM(CRUISE_SPEED_STEP));
		cruise_plusz = FALSE;
	}
	else if((dspGetValue(4) == 0) && cruise_plusz == FALSE){
		cruise_long_plusz++;
		if (cruise_long_plusz > PRESS_PERIOD)
		{
			if (cruise_long_plusz % 16 == 0)
			{
				cruiseIncrease(speedKMPH_TO_RPM(CRUISE_FAST_STEP));
			}
		}
	}
	else if(dspGetValue(4))
	{
		cruise_plusz = TRUE;
		cruise_long_plusz = 0;
	}

/* Cruise controll reduction */
	if((dspGetValue(3) == 0) && cruise_minusz){
		cruiseReduction(speedKMPH_TO_RPM(CRUISE_SPEED_STEP));
		cruise_minusz = FALSE;
	}
	else if((dspGetValue(3) == 0) && cruise_minusz == FALSE){
		cruise_long_minusz++;
		if (cruise_long_minusz > PRESS_PERIOD)
		{
			if (cruise_long_minusz % 16 == 0)
			{
				cruiseReduction(speedKMPH_TO_RPM(CRUISE_FAST_STEP));
			}
		}
	}
	else if(dspGetValue(3))
	{
		cruise_minusz = TRUE;
		cruise_long_minusz = 0;
	}

/* Még nincs bekötve az index nyomógomb */
/* Index right */
	/*if((dspGetValue() == 0) && index_right){
		if (getLightFlashing(2))
		{
			lightFlashing(1);
			index_right = FALSE;
		}
		else
		{
			lightFlashing(0);
		}
	}
	else if(dspGetValue(3))
	{
		index_right = TRUE;
	}*/

/* Még nincs bekötve az index nyomógomb */
/* Index left */
	/*if((dspGetValue() == 0) && index_left){
		if (getLightFlashing(3))
		{
			lightFlashing(2);
			index_left = FALSE;
		}
		else
		{
			lightFlashing(0);
		}
	}
	else if(dspGetValue())
	{
		index_left = TRUE;
	}*/

/* Pos. Lamp */
	if((dspGetValue(5) == 0) && lamp_ok){
		if (getLightFlashing(5))
		{
			lightPosLampOff();
			lamp_ok = FALSE;
		}
		else
		{
			lightPosLampOn();
			lamp_ok = FALSE;
		}
	}
	else if(dspGetValue(5))
	{
		lamp_ok = TRUE;
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