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

#define AVG_TMB      200

enum calcStates
{
  CALC_START,
  CALC_RUNING
} calcstate;

static int32_t calcValue[CALC_NUM_CH +2];

static int motor_index;
static uint16_t avg_motor[AVG_TMB];

void calcCalc(void){
  int ch, i;
  int calc;
  double motorasis;

  switch(calcstate){
    case CALC_START:
      calcstate = CALC_RUNING;
      break;
    case CALC_RUNING:
      for(ch = 0; ch < CALC_NUM_CH; ch++) {
        switch(ch){
          case CALC_MOTOR_POWER:

            //calc = measGetValue_2(MEAS2_CURR1);
            //calc -= Modulux CURRENT
            //calc *= bmsitems.pack_inst_volt;
            //calc /= 100;

            calc = bmsitems.pack_current;

            calc = calc < 0 ? 0 : calc;
            //calc -= Modulux CURRENT
            calc *= bmsitems.pack_inst_volt;
            calc /= 100;

            motor_index++;
            if(motor_index > (AVG_TMB - 1)){
              motor_index = 0;
            }
            avg_motor[motor_index] = calc;
            motorasis = 0;
            for (i = 0; i < AVG_TMB; i++){
              motorasis += avg_motor[i];
            }
            calc = motorasis / AVG_TMB;
            if(calc < 0){
              calc = 0;
            }
            break;
          default:
            break;
        }
        chSysLock();
        calcValue[ch] = (int16_t)calc;
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

void cmd_calcvalues(BaseSequentialStream *chp, int argc, char *argv[]){
  enum calcChannels ch;

  static const char * const names[] = {

      "CALC_MOTOR_POWER"};

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
      chThdSleepMilliseconds(500);
  }
}