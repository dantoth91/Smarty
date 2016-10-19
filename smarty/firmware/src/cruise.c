/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "cruise.h"
#include "speed.h"

#include "chprintf.h"
#include "eeprom.h"
#include "meas.h"

#include "can_items.h"
#include "can_comm.h"

#define START_CRUISE_KMPH       50
#define EEPROM_WRITE_PERIOD     250

#define CRUISE_DISABLE_PERIOD   150

#define ACCELERAT_RPM           200
#define DECELERATE_RPM          200
#define CRUISE_MAX_DIFERENT     500

/* Current limit */
#define ACCEL_LIMIT_CURR        400
#define CURR_MULTIPLIER         50       //30
#define MAX_CURR_SUBSTRACT      5000

#define CURRENT_MIN_SPEED       20
#define CURRENT_MAX_THROTTLE    7000
/* -------------------------- */

/* PID Controller values */
static int16_t K_P = 200;
static int16_t K_I = 4;
static int16_t K_D = 500;

static int32_t MAX_Result = 1000000;
static int32_t MIN_Result = 0;

static int32_t MAX_P = 1000000;
static int32_t MAX_I = 1000000;
static int32_t MAX_D = 1000000;

static double p_tag;
static double i_tag;
static double d_tag;
static int32_t result;
static double eelozo;
static double e_tag;
/* -------------------------- */

/* Regenerative brake values */
static int32_t brake_pwm;
static bool_t regen_on;
static bool_t CurrentLimitIsOn = false;
/* -------------------------- */

/* Cruise control values */
static int32_t save_pwm;
static int32_t pwm;
static uint16_t cruise_on;
static bool_t cruise_indicator;
static uint32_t cruise_indicator_index;

static uint16_t cruise_disable_period;
static bool_t period_null;

static int16_t set;
static int16_t old_set;
static uint32_t eeprom_write_period;
static bool_t eeprom_write;

static bool_t button_long_accel;
static bool_t button_long_decelerat;
/* -------------------------- */

/* Current Limit Variables */
static int32_t accel_limit;
static uint16_t substract;
static bool current_limit_status = 1;
/* -------------------------- */

static PWMConfig cruise_pwmcfg = {
  10000000,	/* 10MHz PWM clock frequency */
  10000,    /* PWM period 1 milliseconds (1 kHz) */
  NULL,
  {
   {PWM_OUTPUT_DISABLED , NULL},
   {PWM_OUTPUT_ACTIVE_HIGH , NULL},
   {PWM_OUTPUT_ACTIVE_HIGH , NULL},
   {PWM_OUTPUT_DISABLED , NULL}
  },
  0,
  0
};

void cruiseInit(void){
  pwm = 10000;
  brake_pwm = 0;
  save_pwm = 0;

  eeprom_write_period = 0;
  old_set = 0;
  eeprom_write = FALSE;

  cruise_on = FALSE;
  regen_on = FALSE;

  cruise_indicator = FALSE;
  cruise_indicator_index = 0;

  cruise_disable_period = 0;

  pwm = 10000;
  p_tag = 2985;
  i_tag = 384000;
  d_tag = 0;
	eelozo = 0;
	e_tag = 0;

  button_long_accel = FALSE;
  button_long_decelerat = FALSE;

  accel_limit = 0;

  /* Set cruise control values */
  if(eepromRead(CRUISE_CONTROLL, &set) != 0){
    set = speedKMPH_TO_RPM(START_CRUISE_KMPH);
  }
  if (set > 1300)
  {
    set = 1300;
  }
  else if (set < 0)
  {
    set = 0;
  }

  pwmStart(&PWMD5, &cruise_pwmcfg);
}

void cruiseCalc(void){

  eeprom_write_period ++;

  accel_limit =  bmsitems.pack_dcl;
  accel_limit *= 5;

/* Cruise control "set" value save */
  if(eeprom_write_period == EEPROM_WRITE_PERIOD)
  {
    if (old_set == set)
    {
      if (eeprom_write == TRUE)
      {
        eeprom_write = FALSE;
        if(eepromWrite(CRUISE_CONTROLL, set) != 0){
          old_set = -1;
        }
      }
    }
  }
  else if (old_set != set)
  {
    old_set = set;
    eeprom_write_period = 0;
    eeprom_write = TRUE;
  }
/* =========================== */

/* Cruise control minimum limiter */
  if (cruise_on && (speedGetSpeed() < 10))
  {
    cruise_on = FALSE;
  }
/* ============================== */

/* Cruise control activated */
  if (cruise_on)
  {
    if (button_long_accel)
    {
      set = speedGetRpm();
      pwm = (int32_t)(cruisePID((speedGetRpm() - ACCELERAT_RPM), set, MAX_Result, MIN_Result, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);
    }
    else if (button_long_decelerat)
    {
      set = speedGetRpm();
      pwm = (int32_t)(cruisePID((speedGetRpm() + DECELERATE_RPM), set, MAX_Result, MIN_Result, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);
    }
    else
    {
      pwm = (int32_t)(cruisePID(speedGetRpm(), set, MAX_Result, MIN_Result, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);
    }
    pwm = 10000 - pwm;

    /* Current limit */
    //pwm = bmsitems.pack_current > accel_limit ? pwm + (uint16_t)((bmsitems.pack_current - accel_limit) * CURR_MULTIPLIER) : \
                                                     pwm;

    if(bmsitems.pack_current > accel_limit && current_limit_status == 1)
    {
      if (speedGetSpeed() > CURRENT_MIN_SPEED)
      {
        substract = ((bmsitems.pack_current - accel_limit) * CURR_MULTIPLIER);
        if (substract > MAX_CURR_SUBSTRACT)
          substract = MAX_CURR_SUBSTRACT;
        pwm = pwm + substract;
        CurrentLimitIsOn = true;
      }
    }else
    {
      CurrentLimitIsOn = false;
    }
    if (speedGetSpeed() < CURRENT_MIN_SPEED && current_limit_status == 1)
    {
        pwm = pwm < CURRENT_MAX_THROTTLE ? CURRENT_MAX_THROTTLE : pwm;
    }


    /* =========================== */

    pwm = pwm > 10000 ? 10000 : pwm;
    pwm = pwm < 1000 ? 1000 : pwm;

    pwmEnableChannel(&PWMD5, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, pwm)); //10000 = 100%

    /* Throttle pedal push - Cruise controll off */
    if(pwm > (10000 - measGetValue_2(MEAS2_THROTTLE)))
    {
      cruise_disable_period ++;
      if(cruise_disable_period > CRUISE_DISABLE_PERIOD)
      {
        cruise_on = FALSE;
      }
    }
    /* =========================== */

    /* Brake pedal push - Cruise controll off */
    if(measGetValue_2(MEAS2_REGEN_BRAKE) > 500)
    {
      cruise_on = FALSE;
    }
    /* =========================== */
  }
/* =========================== */

/* Throttle pedal */
  else
  {

    cruise_disable_period = 0;
    cruise_indicator = FALSE;
    cruise_indicator_index = FALSE;
    pwm = 10000 - measGetValue_2(MEAS2_THROTTLE);
    p_tag = 15;
    i_tag = measGetValue_2(MEAS2_THROTTLE) * 100;
    d_tag = 0;
    eelozo = 0;
    e_tag = 0;
    

    /* Current limit */
    if(bmsitems.pack_current > accel_limit && current_limit_status == 1 && measGetValue(MEAS_IS_IN_DRIVE) == 1)
    {
      if (speedGetSpeed() > CURRENT_MIN_SPEED)
      {
        substract = ((bmsitems.pack_current - accel_limit) * CURR_MULTIPLIER);
        if (substract > MAX_CURR_SUBSTRACT)
          substract = MAX_CURR_SUBSTRACT;
        pwm = pwm + substract;
        CurrentLimitIsOn = true;
      }
    }else{
      CurrentLimitIsOn = false;
    }
    if (speedGetSpeed() < CURRENT_MIN_SPEED && current_limit_status == 1 && measGetValue(MEAS_IS_IN_DRIVE) == 1)
    {
       pwm = pwm < CURRENT_MAX_THROTTLE ? CURRENT_MAX_THROTTLE : pwm;
    }



    /* =========================== */
    pwm = pwm > 10000 ? 10000 : pwm;
    pwm = pwm < 1000 ? 1000 : pwm;

    pwmEnableChannel(&PWMD5, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, pwm)); //10000 = 100%
  }
  chSysLock();
  save_pwm = pwm;
/* =========================== */

/* Regenerative brake */
  brake_pwm = measGetValue_2(MEAS2_REGEN_BRAKE);
  chSysUnlock();

  if (regen_on)
  {
    pwmEnableChannel(&PWMD5, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, 10000 - brake_pwm)); //10000 = 0% - 0 = 100%
  }
}

void cruiseEnable(void){
  cruise_on = TRUE;
  cruise_disable_period = 0;
}
void cruiseDisable(void){
  cruise_on = FALSE;
}

void regen_brakeEnable(void){
  regen_on = TRUE;
}
void regen_brakeDisable(void){
  regen_on = FALSE;
}

bool_t cruiseStatus(void){
  return cruise_on;
}

bool_t cruiseIndicator(void){
  return cruise_indicator;
}

void cruiseAccel(){
  button_long_accel = TRUE;
}

void cruiseAccelOk(){
  if (cruise_on){
    set = speedGetRpm();
  }
  button_long_accel = FALSE;
}

void cruiseDecelerat(){
  button_long_decelerat = TRUE;
}

void cruiseDeceleratOk(){
  if (cruise_on){
    set = speedGetRpm();
  }
  button_long_decelerat = FALSE;
}

void cruiseIncrease(double rpm){
  set += rpm;
  set = set > speedKMPH_TO_RPM(130) ? speedKMPH_TO_RPM(130) : set;
}
void cruiseReduction(double rpm){
  set -= rpm;
  set = set < 0 ? 0 : set;
}

int32_t cruiseGetPWM(void){
  return save_pwm;
}

uint16_t cruiseGetCurrLimitSubstract()
{
  return substract;
}


uint8_t cruiseGet(void){
  return speedRPM_TO_KMPH(set);
}

bool_t GetCurrentLimitIsOn(){
  return CurrentLimitIsOn;
}

int32_t cruisePID (int16_t Input, int16_t Set, int32_t MaxResult, int32_t MinResult, int16_t k_p, int16_t k_i, int16_t k_d, int32_t MaxP, int32_t MaxI, int32_t MaxD)
{	 
	e_tag = Set - Input;

	/* Proportional */
	p_tag = k_p * e_tag;
	if (p_tag > MaxP){
		p_tag = MaxP;
	}
	else if (p_tag < (-1 * MaxP)){
		p_tag = -1 * MaxP;
	}
	/* ----------- */

	/* Integral */
	i_tag += (k_i * e_tag);
	if (i_tag > MaxI){
		i_tag = MaxI;
	}
	else if (i_tag < (-1 * MaxI)){
		i_tag = -1 * MaxI;
	}
	/* ----------- */

	/* Derivative */
	d_tag = k_d * ( e_tag - eelozo);
	if (d_tag > MaxD){
		d_tag = MaxD;
	}
	else if (d_tag < (-1 * MaxD)){
		d_tag = -1 * MaxD;
	}
	/* ----------- */

	/* Result */
	result = (int32_t)(p_tag + i_tag + d_tag);

	if (result > MaxResult){
		result = MaxResult;
	}
	if (result < MinResult){
		result = MinResult;
	}

	eelozo = e_tag;

	return result;
}

/*
 * Shell commands
 */

void cmd_cruisevalues(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);

      chprintf(chp, "K_P: %15d - P: %15d\r\n", K_P, (int32_t)p_tag);
      chprintf(chp, "K_I: %15d - I: %15d\r\n", K_I, (int32_t)i_tag);
	    chprintf(chp, "K_D: %15d - D: %15d\r\n", K_D, (int32_t)d_tag);
      chprintf(chp, "K_D: %15d - R: %15d\r\n", K_D, (int32_t)result);
      chprintf(chp, "pwm: %15d\r\n", pwm);
      chprintf(chp, "Current limit substract: %15d\r\n", substract);
      chprintf(chp, "set rpm: %15d\r\n", set);
      chprintf(chp, "input rpm: %15d\r\n", speedGetRpm());
      chprintf(chp, "set - rpm: %15d\r\n", set - speedGetRpm());
      chprintf(chp, "\r\n");
      chprintf(chp, "cruise_disable_period: %15d\r\n", cruise_disable_period);
      chprintf(chp, "period_null: %15d\r\n", period_null);
      chprintf(chp, "cruise_on: %15d\r\n", cruise_on);
      chprintf(chp, "regen_on: %15d\r\n", regen_on);
      chprintf(chp, "brake_pwm: %15d\r\n", 10000 - brake_pwm);
      chThdSleepMilliseconds(100);
  }
}

void cmd_setcruisevalues(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;

  int16_t szam;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);

      if ((argc == 2) && (strcmp(argv[0], "rpm") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < 3000){
            set = szam;
        }
        else{
            chprintf(chp, "This not good rpm! (0 - 3000)\r\n");
        }
      }

      else if ((argc == 2) && (strcmp(argv[0], "K_P") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < MAX_P){
            K_P = szam;
        }
        else{
            chprintf(chp, "This not good K_P!\r\n");
        }
      }

      else if ((argc == 2) && (strcmp(argv[0], "K_I") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < MAX_I){
            K_I = szam;
        }
        else{
            chprintf(chp, "This not good K_I!\r\n");
        }
      }

      else if ((argc == 2) && (strcmp(argv[0], "K_D") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < MAX_D){
            K_D = szam;
        }
        else{
            chprintf(chp, "This not good K_D!\r\n");
        }
      }

      else if ((argc == 1) && (strcmp(argv[0], "on") == 0)){
        cruiseEnable();
        chprintf(chp, "Cruise control switch ON!\r\n");
      }

      else if ((argc == 1) && (strcmp(argv[0], "off") == 0)){
        cruiseDisable();
        chprintf(chp, "Cruise control switch OFF!\r\n");
      }

      else{
        goto ERROR;
      }

      chprintf(chp, "K_P: %15d\r\n", K_P);
      chprintf(chp, "K_I: %15d\r\n", K_I);
      chprintf(chp, "K_D: %15d\r\n", K_D);
      chprintf(chp, "set rpm: %15d\r\n", set);
      return;

      ERROR:
        chprintf(chp, "This not good command!\r\n");
        return;

      chThdSleepMilliseconds(100);
  }
}

void cmd_cruise(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  if ((argc == 1) && (strcmp(argv[0], "on") == 0)){
    cruiseEnable();
    chprintf(chp, "Cruise control switch ON!\r\n");
  }

  else if ((argc == 1) && (strcmp(argv[0], "off") == 0)){
    cruiseDisable();
    chprintf(chp, "Cruise control switch OFF!\r\n");
  }

  else{
    goto ERROR;
  }
  return;

ERROR:
  chprintf(chp, "Usage: cruise on\r\n");
  chprintf(chp, "       cruise off\r\n");
  return;

  chThdSleepMilliseconds(100);
}

void cmd_current_limit_switch(BaseSequentialStream *chp, int argc, char *argv[])
{
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  if ((argc == 1) && (strcmp(argv[0], "on") == 0)){
    current_limit_status = 1;
    chprintf(chp, "Current limit switch ON!\r\n");
    chprintf(chp, "Discharge Current Limit: %d\r\n", bmsitems.pack_dcl);
  }

  else if ((argc == 1) && (strcmp(argv[0], "off") == 0)){
    current_limit_status = 0;
    chprintf(chp, "Current limit switch OFF!\r\n");
  }
  else{
    goto ERROR;
  }
  return;


ERROR:
  if(current_limit_status)
    chprintf(chp, "Current Limit Status: ON\r\n");
  else
    chprintf(chp, "Current Limit Status: OFF\r\n");

  chprintf(chp, "Usage: set_current_limit on\r\n");
  chprintf(chp, "       set_current_limit off\r\n");
  return;
  chThdSleepMilliseconds(100);
}

void cmd_regen_brake(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  if ((argc == 1) && (strcmp(argv[0], "on") == 0)){
    regen_brakeEnable();
    chprintf(chp, "Regenerative brake switch ON!\r\n");
  }

  else if ((argc == 1) && (strcmp(argv[0], "off") == 0)){
    regen_brakeDisable();
    chprintf(chp, "Regenerative brake switch OFF!\r\n");
  }

  else{
    goto ERROR;
  }
  return;

ERROR:
  chprintf(chp, "Usage: regen_brake on\r\n");
  chprintf(chp, "       regen_brake off\r\n");
  return;

  chThdSleepMilliseconds(100);
}
