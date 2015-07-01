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
  bool_t lights_disabled;
  bool_t right;
  bool_t left;
  bool_t warning;
  bool_t braking;
  bool_t pos_lamp;
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
static bool flashing;
static bool_t right_active;
static bool_t left_active;

void lightInit(void){
  period = 0;
  flashing = FALSE;
  right_active = FALSE;
  left_active = FALSE;
  pwmStart(&PWMD4, &pwmcfg);
}

void lightCalc(void){

  period++;
  
  if (period > (2 * LIGHTS_PERIOD)){
    period = 0;
    flashing = TRUE;
  }
  
  else if(period > LIGHTS_PERIOD)
  {
    pwmDisableChannel(&PWMD4, 1);
    pwmDisableChannel(&PWMD4, 2);

    right_active = FALSE;
    left_active = FALSE;
  }
  
  else{
    if(lightchanels.right && flashing)
    {
      can_lightRight();
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      flashing = FALSE;
      right_active = TRUE;
    }

    if(lightchanels.left && flashing)
    {
      can_lightLeft();
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      flashing = FALSE;
      left_active = TRUE;
    }

    if(lightchanels.warning && flashing)
    {
      can_lightWarning();
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      flashing = FALSE;
      right_active = TRUE;
      left_active = TRUE;
    }
  }
}

void lightFlashing (int chanel) {
  lightchanels.lights_disabled = (chanel == 0 ? TRUE : FALSE);
  lightchanels.right = (chanel == 1 ? TRUE : FALSE);
  lightchanels.left = (chanel == 2 ? TRUE : FALSE);
  lightchanels.warning = (chanel == 3 ? TRUE : FALSE);

  period = 0;
  flashing = TRUE;
}

void lightBrakeOn() {
  lightchanels.braking = TRUE;
  can_lightBreakOn();
}
void lightBrakeOff() {
  lightchanels.braking = FALSE;
  can_lightBreakOff();
}

void lightPosLampOn() {
  lightchanels.pos_lamp = TRUE;
  can_lightPosLampOn();
  pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
}
void lightPosLampOff() {

  lightchanels.pos_lamp = FALSE;
  can_lightPosLampOff();
  pwmDisableChannel(&PWMD4, 3);
}

bool_t getLightFlashing (uint8_t chanel) {
  if (chanel == 1){ return lightchanels.lights_disabled; }
  else if (chanel == 2 && right_active){ return lightchanels.right; }
  else if (chanel == 3 && left_active){ return lightchanels.left; }
  else if (chanel == 4 && right_active == 1 && left_active == 1){ return lightchanels.warning; }
  else if (chanel == 5){ return lightchanels.pos_lamp; }
  else if (chanel == 6){ return lightchanels.braking; }
  else
    return 0;
}

void cmd_lightvalues(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "Right light: %15d\r\n", lightchanels.right);
    chprintf(chp, "Left light: %15d\r\n", lightchanels.left);
    chprintf(chp, "Warning light: %15d\r\n", lightchanels.warning);
    chprintf(chp, "Brake light: %15d\r\n", lightchanels.braking);
    chprintf(chp, "Position light: %15d\r\n", lightchanels.pos_lamp);
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
    if ((argc == 1) && (strcmp(argv[0], "right") == 0)){

      chprintf(chp,"-------------- Blinking right enabled --------------\r\n");
      lightFlashing(1);
      return;
    }

    else if ((argc == 1) && (strcmp(argv[0], "left") == 0)){

      chprintf(chp,"-------------- Blinking left enabled --------------\r\n");
      lightFlashing(2);
      return;
    }

    else if ((argc == 1) && (strcmp(argv[0], "warning") == 0)){

      chprintf(chp,"-------------- Blinking warning enabled --------------\r\n");
      lightFlashing(3);
      return;
    }

    else if ((argc == 1) && (strcmp(argv[0], "brakeon") == 0)){

      chprintf(chp,"-------------- Brake light enabled --------------\r\n");
      lightBrakeOn();
      return;
    }

    else if ((argc == 1) && (strcmp(argv[0], "brakeoff") == 0)){

      chprintf(chp,"-------------- Brake light disabled --------------\r\n");
      lightBrakeOff();
      return;
    }

    else if ((argc == 1) && (strcmp(argv[0], "lampon") == 0)){

      chprintf(chp,"-------------- Position lamp enabled --------------\r\n");
      lightPosLampOn();
      return;
    }

    else if ((argc == 1) && (strcmp(argv[0], "lampoff") == 0)){

      chprintf(chp,"-------------- Position lamp disabled --------------\r\n");
      lightPosLampOff();
      return;
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
  chprintf(chp, "Usage: light right\r\n");
  chprintf(chp, "       light left\r\n");
  chprintf(chp, "       light warning\r\n");
  chprintf(chp, "       light brakeon\r\n");
  chprintf(chp, "       light brakeoff\r\n");
  chprintf(chp, "       light lampon\r\n");
  chprintf(chp, "       light lampoff\r\n");
  chprintf(chp, "       light stop\r\n");
  return;
}

void cmd_getLight(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "Right light: %15d\r\n", getLightFlashing(2));
    chprintf(chp, "Left light: %15d\r\n", getLightFlashing(3));
    chprintf(chp, "Warning light: %15d\r\n", getLightFlashing(4));
    chprintf(chp, "Brake light: %15d\r\n", getLightFlashing(5));
    chprintf(chp, "Position light: %15d\r\n", getLightFlashing(6));
    chThdSleepMilliseconds(100);
  }
}