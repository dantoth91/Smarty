/*
    Smarty - Copyright (C) 2015
    GAMF MegaLux Team              
*/

#include "ch.h"
#include "hal.h"

#include "light.h"
#include "chprintf.h"
#include "can_comm.h"

/* Index flashing period */
#define LIGHTS_PERIOD   16

static struct lightChanels 
{
  bool_t lights_disabled;
  bool_t right;
  bool_t left;
  bool_t warning;
  bool_t braking;
  bool_t front_pos_lamp;
  bool_t rear_pos_lamp;
  bool_t demo;
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
static uint16_t demo_period;
static uint16_t demo_time;

void lightInit(void){
  period = 0;
  demo_period = 0;
  demo_time = 10;
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
    palClearPad(GPIOG, GPIOG_PO4);

    right_active = FALSE;
    left_active = FALSE;
  }
  
  else{
    /* Right index */
    if(lightchanels.right && flashing)
    {
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      palSetPad(GPIOG, GPIOG_PO4);
      flashing = FALSE;
      right_active = TRUE;
    }

    /* Left index */
    if(lightchanels.left && flashing)
    {
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      palSetPad(GPIOG, GPIOG_PO4);
      flashing = FALSE;
      left_active = TRUE;
    }

    /* Warning lamp */
    if(lightchanels.warning && flashing)
    {
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      flashing = FALSE;
      right_active = TRUE;
      left_active = TRUE;
    }
  }

  /* Light */
  if (lightchanels.lights_disabled)
  {
    pwmDisableChannel(&PWMD4, 1);
    pwmDisableChannel(&PWMD4, 2);
  }

  /* Show effect */
  if (lightchanels.demo)
  {
    if (demo_period == 0)
    {
      pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      lightchanels.front_pos_lamp = TRUE;
    }
    else if (demo_period == 1)
    {
      pwmDisableChannel(&PWMD4, 3);
      lightchanels.front_pos_lamp = FALSE;
    }
    else if (demo_period == 4)
    {
      pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      lightchanels.front_pos_lamp = TRUE;
    }
    else if (demo_period == 5)
    {
      pwmDisableChannel(&PWMD4, 3);
      lightchanels.front_pos_lamp = FALSE;
    }
    else if (demo_period == 8)
    {
      pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      lightchanels.front_pos_lamp = TRUE;
    }
    else if (demo_period == 9)
    {
      pwmDisableChannel(&PWMD4, 3);
      lightchanels.front_pos_lamp = FALSE;
    }



    if (demo_period == 14)
    {
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      lightchanels.warning = TRUE;
    }
    else if (demo_period == 15)
    {
      pwmDisableChannel(&PWMD4, 1);
      pwmDisableChannel(&PWMD4, 2);
      lightchanels.warning = FALSE;
    }
    else if (demo_period == 18)
    {
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      lightchanels.warning = TRUE;
    }
    else if (demo_period == 19)
    {
      pwmDisableChannel(&PWMD4, 1);
      pwmDisableChannel(&PWMD4, 2);
      lightchanels.warning = FALSE;
    }
    else if (demo_period == 22)
    {
      pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      lightchanels.warning = TRUE;
    }
    else if (demo_period == 23)
    {
      pwmDisableChannel(&PWMD4, 1);
      pwmDisableChannel(&PWMD4, 2);
      lightchanels.warning = FALSE;
    }
    demo_period++;
    demo_period = demo_period > 48 ? 0 : demo_period;
  }
  if (!lightchanels.demo && !lightchanels.front_pos_lamp)
  {
    pwmDisableChannel(&PWMD4, 3);
  }
  if (!lightchanels.demo && !lightchanels.warning && !lightchanels.right && !lightchanels.left)
  {
    pwmDisableChannel(&PWMD4, 1);
    pwmDisableChannel(&PWMD4, 2);
  }

  /*if (lightchanels.demo)
  {
    demo_period++;
    if ((demo_period > demo_time) && (lightchanels.front_pos_lamp == FALSE))
    {
      pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
      demo_time++;
      lightchanels.front_pos_lamp = TRUE;
    }
    else if ((demo_period > demo_time) && lightchanels.front_pos_lamp)
    {
      pwmDisableChannel(&PWMD4, 3);
      lightchanels.front_pos_lamp = FALSE;
      demo_period = 0;
      demo_time = 10
    }
  }
  if (!lightchanels.demo && !lightchanels.front_pos_lamp)
  {
    pwmDisableChannel(&PWMD4, 3);
  }*/
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
  pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000));
}
void lightBrakeOff() {
  lightchanels.braking = FALSE;
  if (lightchanels.rear_pos_lamp)
  {
    pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 5000));
  }
  else
    pwmDisableChannel(&PWMD4, 0);
}

void lightPosLampOn() {
  lightchanels.front_pos_lamp = TRUE;
  lightchanels.rear_pos_lamp = TRUE;
  pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 7000));
  pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 2500));
}
void lightPosLampOff() {

  lightchanels.front_pos_lamp = FALSE;
  lightchanels.rear_pos_lamp = FALSE;
  pwmDisableChannel(&PWMD4, 3);
  pwmDisableChannel(&PWMD4, 0);
}

void lightLampDemoOn() {
  lightchanels.demo = TRUE;
}

void lightLampDemoOff() {
  lightchanels.demo = FALSE;
}

bool_t getLightFlashing (uint8_t chanel) {
  if (chanel == 1){ return lightchanels.lights_disabled; }
  else if (chanel == 2 && right_active){ return lightchanels.right; }
  else if (chanel == 3 && left_active){ return lightchanels.left; }
  else if (chanel == 4 && right_active == 1 && left_active == 1){ return lightchanels.warning; }
  else if (chanel == 5){ return lightchanels.front_pos_lamp; }
  else if (chanel == 6){ return lightchanels.braking; }
  else if (chanel == 7){ return lightchanels.demo; }
  else
    return 0;
}

/*
 * Shell commands
 */

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
    chprintf(chp, "Position light: %15d\r\n", lightchanels.front_pos_lamp);
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
