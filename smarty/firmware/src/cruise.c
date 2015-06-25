/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/
#include <string.h>
#include <stdlib.h>

#include "cruise.h"
#include "speed.h"

#include "chprintf.h"
#include "eeprom.h"
#include "meas.h"

#define START_CRUISE_KMPH       50
#define EEPROM_WRITE_PERIOD     250

#define CRUISE_DISABLE_PERIOD   75

#define ACCELERAT_RPM           200
#define DECELERATE_RPM          200

static int16_t K_P = 5;
static int16_t K_I = 2;
static int16_t K_D = 500;

static double MAX_U = 1000000;
static double MIN_U = 0;

static double MAX_P = 1000000;
static double MAX_I = 1000000;
static double MAX_D = 1000000;

static int32_t pwm;
static double P;
static double I;
static double D;
static int32_t U;
static double eelozo;
static double e;
static bool_t cruise_on;
static bool_t cruise_indicator;
static uint32_t cruise_indicator_index;

static uint16_t cruise_disable_period;
static bool_t period_null;

static int16_t set;
static int16_t old_set;
static uint32_t eeprom_read_period;
static bool_t eeprom_write;

static bool_t button_long_accel;
static bool_t button_long_decelerat;

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

  eeprom_read_period = 0;
  old_set = 0;
  eeprom_write = FALSE;

  cruise_on = FALSE;

  cruise_indicator = FALSE;
  cruise_indicator_index = 0;

  cruise_disable_period = 0;

	pwm = 10000;
  P = 2985;
  I = 384000;
  D = 0;
	eelozo = 0;
	e = 0;

  button_long_accel = FALSE;
  button_long_decelerat = FALSE;

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
	eeprom_read_period ++;

/* Cruise control "set" value save */
  if(eeprom_read_period == EEPROM_WRITE_PERIOD)
  {
    if (old_set == set)
    {
      if (eeprom_write)
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
    eeprom_read_period = 0;
    eeprom_write = TRUE;
  }

/* Cruise control minimum limiter */
  if (cruise_on && (speedGetSpeed() < 10))
  {
    cruise_on = FALSE;
  }
/* ============================== */

/* Cruise control activated */
  if (cruise_on)
  {
    if (button_long_accel){
      set = speedGetRpm();
      pwm = (int32_t)(cruisePID((speedGetRpm() - ACCELERAT_RPM), set, MAX_U, MIN_U, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);
    }
    else if (button_long_decelerat){
      set = speedGetRpm();
      pwm = (int32_t)(cruisePID((speedGetRpm() + DECELERATE_RPM), set, MAX_U, MIN_U, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);
    }
    else
      pwm = (int32_t)(cruisePID(speedGetRpm(), set, MAX_U, MIN_U, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);
    
    pwm = 10000 - pwm;
    pwm = pwm < 1000 ? 1000 : pwm;

    pwmEnableChannel(&PWMD5, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, pwm)); //10000 = 100%

    if(pwm > (10000 - measGetValue_2(MEAS2_THROTTLE)))
    {
      cruise_disable_period ++;
      if(cruise_disable_period > CRUISE_DISABLE_PERIOD)
      {
        cruise_on = FALSE;
      }
    }
  }
/* =========================== */

/* Throttle pedal */
  else
  {
    cruise_disable_period = 0;
    cruise_indicator = FALSE;
    cruise_indicator_index = FALSE;
    pwm = 10000 - measGetValue_2(MEAS2_THROTTLE);
    P = 15;
    //P = pwm * 100;
    I = 606700;
    I = 0;
    D = 0;
    eelozo = 0;
    e = 0;

    pwmEnableChannel(&PWMD5, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, pwm)); //10000 = 100%
  }
}
/* ============== */

void cruiseEnable(void){
  cruise_on = TRUE;
  cruise_disable_period = 0;
}

void cruiseDisable(void){
  cruise_on = FALSE;
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

uint8_t cruiseGet(void){
  return speedRPM_TO_KMPH(set);
}

int32_t cruisePID (int16_t Input, int16_t Set, int32_t MaxU, int32_t MinU, double Kp, double Ki, double Kd, int32_t MaxP, int32_t MaxI, int32_t MaxD)
{	 
	e = Set - Input;

	/* Proportional */
	P = Kp * e;
	if (P > MaxP){
		P = MaxP;
	}
	else if (P < (-1 * MaxP)){
		P = -1 * MaxP;
	}
	/* ----------- */

	/* Integral */
	I += (Ki * e);
	if (I > MaxI){
		I = MaxI;
	}
	else if (I < (-1 * MaxI)){
		I = -1 * MaxI;
	}
	/* ----------- */

	/* Derivative */
	D = Kd * ( e - eelozo);
	if (D > MaxD){
		D = MaxD;
	}
	else if (D < (-1 * MaxD)){
		D = -1 * MaxD;
	}
	/* ----------- */

	/* Result */
	U = (int32_t)(P + I + D);

	if (U > MaxU){
		U = MaxU;
	}
	else if (U < MinU){
		U = MinU;
	}
	eelozo = e;

	return U;
}

void cmd_cruisevalues(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);

      chprintf(chp, "K_P: %15d - P: %15d\r\n", K_P, (int32_t)P);
      chprintf(chp, "K_I: %15d - I: %15d\r\n", K_I, (int32_t)I);
	    chprintf(chp, "K_D: %15d - D: %15d\r\n", K_D, (int32_t)D);
      chprintf(chp, "K_U: %15d - U: %15d\r\n", K_D, (int32_t)U);
      chprintf(chp, "pwm: %15d\r\n", pwm);
      chprintf(chp, "set rpm: %15d\r\n", set);
      chprintf(chp, "input rpm: %15d\r\n", speedGetRpm());
      chprintf(chp, "set - rpm: %15d\r\n", set - speedGetRpm());
      chprintf(chp, "\r\n");
      chprintf(chp, "cruise_disable_period: %15d\r\n", cruise_disable_period);
      chprintf(chp, "period_null: %15d\r\n", period_null);

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

      if ((argc == 2) && (strcmp(argv[0], "K_P") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < MAX_P){
            K_P = szam;
        }
        else{
            chprintf(chp, "This not good K_P!\r\n");
        }
      }

      if ((argc == 2) && (strcmp(argv[0], "K_I") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < MAX_I){
            K_I = szam;
        }
        else{
            chprintf(chp, "This not good K_I!\r\n");
        }
      }

      if ((argc == 2) && (strcmp(argv[0], "K_D") == 0)){
        szam = atol(argv[1]);
        if (szam > 0 || szam < MAX_D){
            K_D = szam;
        }
        else{
            chprintf(chp, "This not good K_D!\r\n");
        }
      }

      if ((argc == 1) && (strcmp(argv[0], "on") == 0)){
        cruiseEnable();
        chprintf(chp, "Cruise control switch ON!\r\n");
      }

      if ((argc == 1) && (strcmp(argv[0], "off") == 0)){
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

  if ((argc == 1) && (strcmp(argv[0], "off") == 0)){
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