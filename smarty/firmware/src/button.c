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
static bool_t index_right_active;
static bool_t index_left;
static bool_t index_left_active;

static bool_t warning;
static bool_t warning_active;

static bool_t lamp_ok;
static uint8_t lamp_long;

static uint16_t bus;
static bool_t bus_bit[8];

void buttonInit(void){
	cruise_ok = FALSE;
	cruise_plusz = FALSE;
	cruise_long_plusz = 0;
	cruise_minusz = FALSE;
	cruise_minusz = FALSE;
	lamp_ok = FALSE;
	lamp_long = 0;
	button_accel = FALSE;

	bus = 0;
}

void buttonCalc(void){
	uint8_t i;

	bus = dspGetValue();

	if (bus > 0x6 && bus != 0x1F)
    {
      for (i = 0; i < 8; i++)
      {
        if ((bus >> i) & 0x01)
        {
          bus_bit[i] = 1;
        }
        else{
          bus_bit[i] = 0;
        }
      }
    }

/* Cruise controll activated */
	if((bus_bit[7] == 0) && cruise_ok){
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
	else if(bus_bit[7])
	{
		cruise_ok = TRUE;
	}

/* Cruise controll increase */
	if((bus_bit[3] == 0) && cruise_plusz){
		cruiseIncrease(speedKMPH_TO_RPM(CRUISE_SPEED_STEP));
		cruise_plusz = FALSE;
	}
	else if((bus_bit[3] == 0) && cruise_plusz == FALSE){
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
	else if(bus_bit[3])
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
	if((bus_bit[2] == 0) && cruise_minusz){
	  cruiseReduction(speedKMPH_TO_RPM(CRUISE_SPEED_STEP));
	  cruise_minusz = FALSE;
	}
	else if((bus_bit[2] == 0) && cruise_minusz == FALSE){
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
	else if(bus_bit[2])
	{
		if (button_decelerat)
		{
			button_decelerat = FALSE;
			cruiseDeceleratOk();
		}
		cruise_minusz = TRUE;
		cruise_long_minusz = 0;
	}

/* Index right */
	if((palReadPad(GPIOA, GPIOA_BUT5) == 0) && index_right){
		if (index_right_active || index_left_active || warning_active)
		{
			lightFlashing(0);
			index_right = FALSE;
			
			index_right_active = FALSE;
			index_left_active = FALSE;
			warning_active = FALSE;
		}
		else
		{
			lightFlashing(1);
			index_right = FALSE;
			index_right_active = TRUE;
		}
	}
	else if(palReadPad(GPIOA, GPIOA_BUT5))
	{
		index_right = TRUE;
	}

/* Index left */
	if((palReadPad(GPIOA, GPIOA_BUT6) == 0) && index_left){
		if (index_left_active || index_right_active || warning_active)
		{
			lightFlashing(0);
			index_left = FALSE;

			index_left_active = FALSE;
			index_right_active = FALSE;
			warning_active = FALSE;
		}
		else
		{
			lightFlashing(2);
			index_left = FALSE;
			index_left_active = TRUE;
		}
	}
	else if(palReadPad(GPIOA, GPIOA_BUT6))
	{
		index_left = TRUE;
	}

/* Warning light */
	if((palReadPad(GPIOA, GPIOA_BUT6) == 0) && warning){
		if((palReadPad(GPIOA, GPIOA_BUT5) == 0) && warning){
			if (warning_active)
			{
				lightFlashing(0);
				warning = FALSE;
				warning_active = FALSE;
			}
			else
			{
				lightFlashing(3);
				warning = FALSE;
				warning_active = TRUE;
			}
		}
	}
	else if((palReadPad(GPIOA, GPIOA_BUT6)) && (palReadPad(GPIOA, GPIOA_BUT5)))
	{
		warning = TRUE;
	}

/* Pos. Lamp */
	if(bus_bit[4] == 0)
	{
		lamp_long++;
		if (lamp_ok)
		{
			if (getLightFlashing(5) || getLightFlashing(7))
			{
				lightPosLampOff();
				lightLampDemoOff();
				lamp_ok = FALSE;
			}
			else
			{
				lightPosLampOn();
				lamp_ok = FALSE;
			}
		}
		if (lamp_long > 100)
		{
			lamp_long = 0;
		    lightLampDemoOn();
		}

	}
	else if(bus_bit[4])
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
      chprintf(chp, "bus: %x\r\n", bus);
      chprintf(chp, "bus_status: ");
      for(i = 0; i < 8; i++) {
          chprintf(chp, "%d ", bus_bit[i]);
      }
      chprintf(chp, "\r\n");
      chprintf(chp, "right: %d left: %d \r\n", index_right, index_left);
      chprintf(chp, "\r\ncruise_ok: %d \r\n", cruise_ok);
	  chprintf(chp, "cruise_plusz: %d \r\n", cruise_plusz);
	  chprintf(chp, "cruise_minusz: %d \r\n", cruise_minusz);
	  chprintf(chp, "button_accel: %d \r\n", button_accel);
	  chprintf(chp, "button_decelerat: %d \r\n", button_decelerat);
	  chprintf(chp, "index_right: %d \r\n", index_right);
	  chprintf(chp, "index_left: %d \r\n", index_left);
	  chprintf(chp, "warning: %d \r\n", warning);
	  chprintf(chp, "lamp_ok: %d \r\n", lamp_ok);
      chThdSleepMilliseconds(100);
  }
}