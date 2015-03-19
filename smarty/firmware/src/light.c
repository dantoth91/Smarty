/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "ch.h"
#include "hal.h"

#include "light.h"
#include "chprintf.h"
#include "can_comm.h"

static CANTxFrame txmsg;

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

/*
 * Blinking thread.
 */
static WORKING_AREA(light_blink_wa, 256);
static msg_t light_blink(void * p) {

  (void)p;
  chRegSetThreadName("light_blink");

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (!chThdShouldTerminate()) {
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
    pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000)); //100%
    chThdSleepMilliseconds(400);

    pwmDisableChannel(&PWMD4, 1); //1%
    chThdSleepMilliseconds(400);
  }
  return 0;
}

static uint32_t ido;

void lightInit(void){
    pwmStart(&PWMD4, &pwmcfg);

    /*
    * Creates the 20ms Task.
    */
    //chThdCreateStatic(light_blink_wa, sizeof(light_blink_wa), NORMALPRIO, light_blink, NULL);
}

void lightCalc(void){

  ido = RTT2US(chTimeNow());
  
  // Horn
	pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 10000)); //100%
	
  //Right front light
  //pwmEnableChannel(&PWMD4, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 7500));  //75%
	
  //Left front light
  pwmEnableChannel(&PWMD4, 2, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 5000));  //50%
	
  //Front light
  pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 1000));  //10%
}

void cmd_lightvalues(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "---- PWM START ----");
    chprintf(chp, "ido : %x \r\n", ido);
    chThdSleepMilliseconds(1000);
  }
}