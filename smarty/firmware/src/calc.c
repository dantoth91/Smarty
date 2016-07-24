/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "calc.h"
#include "meas.h"

#include "can_items.h"
#include "can_comm.h"

#include "chprintf.h"
#include "speed.h"
//#include "console.h"

#define AVG_TMB      200

enum calcStates
{
  CALC_START,
  CALC_RUNING
} calcstate;

static int32_t calcValue[CALC_NUM_CH +2];

static bool_t avg_speed_enable;
static uint32_t avg_speed_period;
static double avg_speed;
static uint32_t time_values;

static time_t time_unix;

static int32_t calc;

void calcInit(void){
  avg_speed_enable = FALSE;
  avg_speed_period = 0;
  avg_speed = 0;
}

void calcCalc(void){
  int ch, i;
  double motorasis;
  double temp = 0;
  avg_speed_period++;

  switch(calcstate){
    case CALC_START:
      calcstate = CALC_RUNING;
      break;
    case CALC_RUNING:
      for(ch = 0; ch < CALC_NUM_CH; ch++) {
        switch(ch){
          case CALC_MOTOR_POWER:

            /*
             * Calculating Motor Power
             * Power = bmsitems.pack_inst_volt * MEAS2_CURR1;
             */
            temp = (double)measGetValue_2(MEAS2_CURR1) / 100;
            temp *= (bmsitems.pack_inst_volt / 10);
            calc = (int32_t)(temp);
            calc -= 20;     //kompenzáció
            /*
            calc = measGetValue_2(MEAS2_CURR1);
            calc *= bmsitems.pack_inst_volt;
            calc /= 1000;*/

            break;
          case CALC_SUN_POWER:
            calc = bmsitems.pack_inst_volt * mlitems.sun_current;
            calc /= 10000;

            break;

          case CALC_AVG_SPEED:

            if((speedGetSpeed() > 10) && (avg_speed_enable == FALSE)){
              avg_speed_enable = TRUE;
              avg_speed_period = 0;
              time_unix = rtcGetTimeUnixSec(&RTCD1);
              TotalMeterZero();
            }

            if (avg_speed_enable){
              if((avg_speed_period % 50) == 0)
              {
                avg_speed = GetTotalMeter();
                time_values = (uint32_t)(rtcGetTimeUnixSec(&RTCD1) - time_unix);
                avg_speed /= (double)time_values;
                avg_speed *= 36;
                //calc = (int32_t)(avg_speed * 36);
                //calc = (int32_t)(avg_speed * 100);
                //calc = 50;
                calc = (int32_t)avg_speed;
              }
            }

            if (avg_speed_period > 60000)
            {
              avg_speed = 0;
              avg_speed_enable = FALSE;
            }

            break;

          default:
            break;
        }
        chSysLock();
        calcValue[ch] = (int32_t)calc;
        chSysUnlock();
      }
      break;

    default:
      break;
  }
}

int32_t calcGetValue(enum calcChannels ch){
  return calcValue[ch];
}

uint32_t calcAvgSpeed(void){
  uint32_t tmp;

  chSysLock();
  tmp = (uint32_t)avg_speed;
  chSysUnlock();
  return tmp;
}

void cmd_calcvalues(BaseSequentialStream *chp, int argc, char *argv[]){
  enum calcChannels ch;

  static const char * const names[] = {

      "CALC_MOTOR_POWER", "CALC_SUN_POWER", "AVG_SPEED"};

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);
      for(ch = 0; ch < CALC_NUM_CH; ch++) {
          chprintf(chp, "%s: %15d\r\n", names[ch], calcValue[ch]);
      }
      chprintf(chp, "rpm: %15d\r\n", speedGetRpm());
      chprintf(chp, "avg_speed: %15d\r\n", (uint16_t)avg_speed);
      chprintf(chp, "time_values: %15d\r\n", time_values);
      chprintf(chp, "calcAvgSpeed(): %15d\r\n", calcAvgSpeed());
      chThdSleepMilliseconds(500);
  }
}
