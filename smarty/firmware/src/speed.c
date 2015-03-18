/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "speed.h"
#include "chprintf.h"

/* 10kHz ICU clock frequency.  (1MHz - 1000000L) */
#define SPEED_ICU_CLOCK (1000000L)

#define SEN_POINTS       48
#define WHEEL_DIAMETER   550
#define MSTOS            60000000

static double rpm_buff[SEN_POINTS];

static int rpm_index;

static void speedWheelPeriodCb(ICUDriver *icup);
static void speedWheelPeriodNumber(ICUDriver *icup);

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  SPEED_ICU_CLOCK,
  speedWheelPeriodNumber,
  speedWheelPeriodCb,
  NULL,
  ICU_CHANNEL_1,
  0
};

static uint32_t speed_last_period;
static uint32_t speed_period_num;

static double rotation;

static void speedWheelPeriodCb(ICUDriver *icup) {
    speed_last_period = icuGetPeriod(icup); //0.1 * ms
}

static void speedWheelPeriodNumber(ICUDriver *icup) {
    (void)icup;
    speed_period_num ++;
}

void speedInit(void){

  /*
   * Activates and starts the icu driver 1
   */
  icuStart(&ICUD1, &icucfg);
  icuEnable(&ICUD1);

  speed_period_num = 0;
}

void speedCalc(void){
  int i;
  double rpmasis;

  rotation = MSTOS / (speed_last_period * SEN_POINTS);

  rpm_index++;
  if(rpm_index > (SEN_POINTS - 1)){
    rpm_index = 0;
  }
  rpm_buff[rpm_index] = rotation;
  rpmasis = 0;
  for (i = 0; i < SEN_POINTS; i++){
    rpmasis += rpm_buff[i];
  }
  rotation = rpmasis / SEN_POINTS;
  if(rotation < 0){
    rotation = 0;
  }

  /*if(rotation > 3000){
    rotation = 3000;
  }
  else if(rotation < 0){
    rotation = 0;
  }*/
}

uint32_t speedGetLastPeriod(void){
  uint32_t tmp;

  chSysLock();
  tmp = speed_last_period;
  chSysUnlock();
  return tmp;
}

uint32_t speedGetPeriodNumber(void){
  uint32_t tmp2;

  chSysLock();
  tmp2 = speed_period_num;
  chSysUnlock();
  return tmp2;
}

uint32_t speedGetRpm(void){
  uint32_t tmp;

  chSysLock();
  tmp = (uint32_t)rotation;
  chSysUnlock();
  return tmp;
}

void cmd_speedvalues(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "speed_last_period: %5d\r\n", speed_last_period); //2500000L / speed_last_period
    chprintf(chp, "speed_period_num: %5d\r\n", speed_period_num);
    chprintf(chp, "rpm: %5d\r\n", rotation);
    chThdSleepMilliseconds(1000);
  }
}