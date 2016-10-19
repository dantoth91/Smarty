/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "brake.h"

#include "cruise.h"
#include "meas.h"

#include "chprintf.h"

static int32_t brake;

void brakeInit(void){
  brake = 0;
}

void brakeCalc(void){
  //brake = measGetValue_2(MEAS2_REGEN_BRAKE);
  //pwmEnableChannel(&PWMD5, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD5, 10000 - brake)); //10000 = 0% - 0 = 100%
}

/*
 * Shell command
 */

void cmd_brakevalues(BaseSequentialStream *chp, int argc, char *argv[]){
  
  (void)argc;
  (void)argv;
  int8_t i = 0;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "brake_pwm: %15d\r\n", brake);

    chThdSleepMilliseconds(1000);
  }
}