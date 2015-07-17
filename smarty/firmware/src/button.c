/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "ch.h"
#include "hal.h"

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
static bool_t button_accel;
static bool_t button_decelerat;

static bool_t index_right;
static bool_t index_left;

static bool_t lamp_ok;
static uint8_t seged;

void buttonInit(void){
	cruise_ok = FALSE;
	cruise_plusz = FALSE;
	cruise_long_plusz = 0;
	cruise_minusz = FALSE;
	cruise_minusz = FALSE;
	lamp_ok = FALSE;
	button_accel = FALSE;
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
		/*if (cruise_long_plusz > PRESS_PERIOD)
		{
			if (cruise_long_plusz % 16 == 0)
			{
				cruiseIncrease(speedKMPH_TO_RPM(CRUISE_FAST_STEP));
			}
		}*/

		if (cruise_long_plusz > PRESS_PERIOD)
		{
			cruiseAccel();
			button_accel = TRUE;
		}
	}
	else if(dspGetValue(4))
	{
		if (button_accel)
		{
			button_accel = FALSE;
			cruiseAccelOk();
		}
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
	  /*if (cruise_long_minusz > PRESS_PERIOD)
	  {
			if (cruise_long_minusz % 16 == 0)
			{
				cruiseReduction(speedKMPH_TO_RPM(CRUISE_FAST_STEP));
			}
		}*/
		if (cruise_long_minusz > PRESS_PERIOD)
		{
			cruiseDecelerat();
			button_decelerat = TRUE;
		}
	}
	else if(dspGetValue(3))
	{
		if (button_decelerat)
		{
			button_decelerat = FALSE;
			cruiseDeceleratOk();
		}
		cruise_minusz = TRUE;
		cruise_long_minusz = 0;
	}

/* Még nincs bekötve az index nyomógomb */
/* Index right */
	if((palReadPad(GPIOA, GPIOA_BUT5) == 0) && index_right){
		seged = 1;
		if (getLightFlashing(2))
		{
			seged = 2;
			lightFlashing(0);
			index_right = FALSE;
		}
		else
		{
			lightFlashing(1);
			index_right = FALSE;
			seged = 3;
		}
	}
	else if(palReadPad(GPIOA, GPIOA_BUT5))
	{
		index_right = TRUE;
		seged = 4;
	}

/* Még nincs bekötve az index nyomógomb */
/* Index left */
	if((palReadPad(GPIOA, GPIOA_BUT6) == 0) && index_left){
		if (getLightFlashing(3))
		{
			lightFlashing(0);
			index_left = FALSE;
		}
		else
		{
			lightFlashing(2);
			index_left = FALSE;
		}
	}
	else if(palReadPad(GPIOA, GPIOA_BUT6))
	{
		index_left = TRUE;
	}

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
      chprintf(chp, "right: %d left: %d \r\n", index_right, index_left);
      chprintf(chp, "seged: %d \r\n", seged);
      chThdSleepMilliseconds(100);
  }
}