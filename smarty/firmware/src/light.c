/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "ch.h"
#include "hal.h"

#include "light.h"
#include "chprintf.h"
#include "can_comm.h"

#define LIGHTS_PERIOD   30

static struct lightChanels 
{
  bool lights_disabled;
  bool right;
  bool left;
  bool warning;
} lightchanels;

static PWMConfig pwmcfg = {
  1000000,	/* 1MHz PWM clock frequency */
  1000,    /* PWM period 1 milliseconds */
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0
};

static int period;

void lightInit(void){
  period = 0;
  pwmStart(&PWMD4, &pwmcfg);
}

void lightCalc(void){
  

  /* Horn */
	//pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000)); //100%

  if(lightchanels.right)
  {
    period++;

    if (period > LIGHTS_PERIOD)
    {
      can_lightRight();
      period = 0;
    }
  }

  if (lightchanels.left)
  {
    period++;

    if (period > LIGHTS_PERIOD)
    {
      can_lightLeft();
      period = 0;
    }
  }
    
  if (lightchanels.warning)
  {
    period++;

    if (period > LIGHTS_PERIOD)
    {
      can_lightWarning();
      period = 0;
    }
  }
}

void lightFlashing (int chanel) {
  lightchanels.lights_disabled = (chanel == 0 ? TRUE : FALSE);
  lightchanels.right = (chanel == 1 ? TRUE : FALSE);
  lightchanels.left = (chanel == 2 ? TRUE : FALSE);
  lightchanels.warning = (chanel == 3 ? TRUE : FALSE);

  period = 0;
}

void cmd_lightvalues(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "---- PWM START ----");
    chThdSleepMilliseconds(1000);
  }
}

void cmd_lightblink(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);
    if ((argc == 2) && (strcmp(argv[0], "start") == 0)){
      if ((argc == 2) && (strcmp(argv[1], "right") == 0)){

        chprintf(chp,"-------------- Blinking right enabled --------------\r\n");
        lightFlashing(1);
        return;
      }

      if ((argc == 2) && (strcmp(argv[1], "left") == 0)){

        chprintf(chp,"-------------- Blinking left enabled --------------\r\n");
        lightFlashing(2);
        return;
      }

      if ((argc == 2) && (strcmp(argv[1], "warning") == 0)){

        chprintf(chp,"-------------- Blinking warning enabled --------------\r\n");
        lightFlashing(3);
        return;
      }
    }

    else if ((argc == 1) && (strcmp(argv[0], "stop") == 0)){
      chprintf(chp,"-------------- All blinking disabled --------------\r\n");
      lightFlashing(0);
      return;
    }

    else{
      goto ERROR;
    }
  }

ERROR:
  chprintf(chp, "Usage: start right\r\n");
  chprintf(chp, "       start left\r\n");
  chprintf(chp, "       start warning\r\n");
  chprintf(chp, "       stop\r\n");
  return;
}