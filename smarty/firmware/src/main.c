/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/


#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"

#include "console.h"
#include "sdcard.h"
//#include "eeprom.h"
#include "can_comm.h"
#include "light.h"
#include "speed.h"
#include "meas.h"
#include "cruise.h"
#include "dsp.h"
#include "uart_comm.h"
#include "log.h"


/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    systime_t time;

    if(!palReadPad(GPIOD, GPIOD_SD_CARD_IN)){
      if(sdcardIsMounted()){
        time = 250;
      }
      else{
        time = 50;
      }
    }
    else {
      time = 1000;
    }

    palClearPad(GPIOA, GPIOA_TXD4);
    chThdSleepMilliseconds(time);
    palSetPad(GPIOA, GPIOA_TXD4);
    chThdSleepMilliseconds(time);
  }
  return 0; /* Never executed.*/
}

/*
 * 20ms Task
 */
static WORKING_AREA(watask20ms, 256);
static msg_t task20ms(void *arg) {
  systime_t time; 

  (void)arg;
  chRegSetThreadName("task20ms");
  time = chTimeNow();  
  while (TRUE) {
    time += MS2ST(20);

    lightCalc();
    can_commCalc();
    speedCalc();
    measCalc();
    cruiseCalc();
    dspCalc();
    logCalc();


    chThdSleepUntil(time);
  }
  return 0; /* Never executed.*/
}
/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* 
   * 5V Enable 
   */
  palSetPad(GPIOE, GPIOE_X5V_EN);

  /*
   * Initialize the SDcard module.
   */
  sdcardInit();

  /*
   * Shell manager initialization.
   */
  consoleInit();

  /*
   * EEPROM initialization.
   */
  //eepromInit();

  /*
   * CAN bus initialization.
   */
  can_commInit();
  /*
     * uart initialization.
     */
  uart_commInit();

  /*
   * Light control initialization.
   */
  lightInit();

  /*
   * Speed measurement initialization.
   */
  speedInit();

  /*
   * Analog measurement initialization.
   */
  measInit();

  /*
   * Cruise control initialization.
   */
  cruiseInit();

  /*
   * Initializes Display module.
   */
  chThdSleepMilliseconds(1000);
    dspInit();

  /*
   * Initializes Log module.
   */
  logInit();

  /*
   * Creates the 20ms Task.
   */
  chThdCreateStatic(watask20ms, sizeof(watask20ms), NORMALPRIO, task20ms, NULL);
  
  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    consoleStart();
    chThdSleepMilliseconds(1000);
  }
}
