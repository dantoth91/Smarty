/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "ch.h"
#include "hal.h"

#include "safety.h"

#include "chprintf.h"

#include "can_items.h"
#include "can_comm.h"
#include "cruise.h"

/* 1MHz timer clock frequency. */
#define FAN_SPEED_CLOCK 1000000
/* 1 second pulse */
#define FAN_PERIOD_PULSE  10000

enum fanStates
{
  FAN_DISABLED,
  FAN_ACTIVE,
  FAN_DEACTIVE
} fanstate;

static void safetyFanTimerCb(GPTDriver *gptp);

static bool_t relay[16];
static bool_t fan_start;
static uint8_t fan_start_period;
static uint32_t fan_percent;

/*
 * GPT2 configuration.
 */
static const GPTConfig gpt2cfg = {
  FAN_SPEED_CLOCK,    
  safetyFanTimerCb    /* Timer callback.*/
};

/*
 * GPT callback for Ignition interrupt 
 */
static void safetyFanTimerCb(GPTDriver *gptp) {

  (void)gptp;
  switch(fanstate){
    case FAN_ACTIVE:
      palClearPad(GPIOG, GPIOG_PO4);
      palClearPad(GPIOD, GPIOD_TXD3);
      fanstate = FAN_DEACTIVE;
      chSysLockFromIsr();
      gptStartOneShotI(&GPTD2, FAN_PERIOD_PULSE - fan_percent);
      chSysUnlockFromIsr();
      break;

    case FAN_DEACTIVE:
      palSetPad(GPIOG, GPIOG_PO4);
      palSetPad(GPIOD, GPIOD_TXD3);
      break;

    default:
      break;
  }
}

/*
 * Transmitter thread.
 */
static WORKING_AREA(safety_wa, 256);
static msg_t safety(void * arg) {

  (void)arg;
  chRegSetThreadName("safety");
  int i;


  while(TRUE){

  	for (i = 0; i < 8; i++)
  	{
  		if ((bmsitems.custom_flag_1 >> i) & 0x01)
      {
	      	relay[i] = 1;
	    }
	    else{
	        relay[i] = 0;
	    }
    }

    for (i = 8; i < 16; i++)
  	{
  		if ((bmsitems.custom_flag_2 >> (i - 8)) & 0x01)
      {
	      	relay[i] = 1;
	    }
	    else{
	        relay[i] = 0;
	    }
    }

    if((bmsitems.custom_flag_1 >> 1) & 0x01)
    {
      regen_brakeDisable();
    }
    else
    {
      regen_brakeEnable();
    }

    if(bmsitems.fan_speed != 0)
    {
      if (fan_start)
      {

        fan_start_period ++;
        fan_start = fan_start_period > 10 ? FALSE : TRUE;

        palSetPad(GPIOG, GPIOG_PO4);
        palClearPad(GPIOD, GPIOD_TXD3);
      }
      
      else if ((bmsitems.fan_speed != 6) && (fan_start == FALSE))
      {
        fan_percent = bmsitems.fan_speed == 1 ? 500  : \
                      bmsitems.fan_speed == 2 ? 2500 : \
                      bmsitems.fan_speed == 3 ? 4500 : \
                      bmsitems.fan_speed == 4 ? 6500 : 8500;
        
        fanstate = FAN_ACTIVE;
        gptStartOneShot(&GPTD2, fan_percent);
      }
      else
      {
        palSetPad(GPIOG, GPIOG_PO4);
        palClearPad(GPIOD, GPIOD_TXD3);
      }

    }
    else{
      fan_start = TRUE;
      fan_start_period = 0;
      palClearPad(GPIOG, GPIOG_PO4);
      palSetPad(GPIOD, GPIOD_TXD3);
    }

    chThdSleepMilliseconds(20);
  }
  return 0;
}

void safetyInit(void){
	int i;
	for (i = 0; i < 8; ++i)
	{
		relay[i] = 0;
	}

  fanstate = FAN_DEACTIVE;
  fan_percent = 1;
  fan_start = FALSE;
  /*
   * Activates and starts the gpt driver 1 and 2
   */
  gptStart(&GPTD2, &gpt2cfg);
  gptStartOneShot(&GPTD2, FAN_PERIOD_PULSE);   /* 0.1 second pulse.*/

  chThdCreateStatic(safety_wa, sizeof(safety_wa), NORMALPRIO + 7, safety, NULL);
}

void cmd_safetyvalues(BaseSequentialStream *chp, int argc, char *argv[]){
  
  (void)argc;
  (void)argv;

  int i;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);
      
      chprintf(chp, "custom_flag_1: %5d\r\n", bmsitems.custom_flag_1);
      chprintf(chp, "custom_flag_2: %5d\r\n", bmsitems.custom_flag_2);
      chprintf(chp, "%5d\r\n", sizeof(relay));
      chprintf(chp, "Relay: ");
      for (i = sizeof(relay); i > 0; --i)
      {
        chprintf(chp, "%d", relay[i - 1]);
      }
      chprintf(chp, "\r\n");
      chprintf(chp, "fan_percent: %5d\r\n",fan_percent);
      
      chprintf(chp, "\r\n");
      chThdSleepMilliseconds(100);
  }
}