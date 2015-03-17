/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/
#include <string.h>
#include <stdlib.h>

#include "cruise.h"
#include "speed.h"

#include "chprintf.h"

static int16_t K_P = 5;
static int16_t K_I = 2;
static int16_t K_D = 500;

static double MAX_U = 1000000;
static double MIN_U = 650000;

static double MAX_P = 1000000;
static double MAX_I = 1000000;
static double MAX_D = 1000000;

static int32_t pwm;
static double P;
static double I;
static double D;
static double eelozo;
static double e;

static int set;

static PWMConfig cruise_pwmcfg = {
  10000000,	/* 10MHz PWM clock frequency */
  10000,    /* PWM period 1 milliseconds (1 kHz) */
  NULL,
  {
   {PWM_OUTPUT_DISABLED , NULL},
   {PWM_OUTPUT_DISABLED , NULL},
   {PWM_OUTPUT_ACTIVE_HIGH , NULL},
   {PWM_OUTPUT_DISABLED , NULL}
  },
  0,
};

void cruiseInit(void){
	pwm = 0;

	P = 0;
	I = 0;
	D = 0;
	eelozo = 0;
	e = 0;

	set = 500;

	pwmStart(&PWMD5, &cruise_pwmcfg);
}

void cruiseCalc(void){
	pwm = (int32_t)(cruisePID(speedGetRpm(), set, MAX_U, MIN_U, K_P, K_I, K_D, MAX_P, MAX_I, MAX_D) / 100);

	pwmEnableChannel(&PWMD5, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, pwm)); //10000 = 100%
}

int32_t cruisePID (int16_t Input, int16_t Set, int32_t MaxU, int32_t MinU, double Kp, double Ki, double Kd, int32_t MaxP, int32_t MaxI, int32_t MaxD)
{	 
  int32_t U;

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
      chprintf(chp, "pwm: %15d\r\n", pwm);
      chprintf(chp, "set rpm: %15d\r\n", set);
      chprintf(chp, "input rpm: %15d\r\n", speedGetRpm());

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