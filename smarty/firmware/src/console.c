/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ch.h"
#include "hal.h"

#include "test.h"
#include "shell.h"
#include "chprintf.h"
#include "chrtclib.h"

#include "console.h"
#include "sdcard.h"
#include "log.h"
#include "eeprom.h"
#include "can_comm.h"
#include "light.h"
#include "speed.h"
#include "meas.h"
#include "cruise.h"
#include "dsp.h"
#include "button.h"
#include "brake.h"
#include "safety.h"
#include "calc.h"
#include "nmea.h"

/* libc stub */
int _getpid(void) {return 1;}
/* libc stub */
void _exit(int i) {(void)i;}
/* libc stub */
#include <errno.h>
#undef errno
extern int errno;
int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

#define SHELL_WA_SIZE   THD_WA_SIZE(2304)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static time_t unix_time;

static SerialConfig ser_cfg = {
  230400,
  0,
  0,
  0,
};

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]);
void func_sleep(void);
void cmd_sleep(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_alarm(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_clear(BaseSequentialStream *chp, int argc, char *argv[]);

Thread *consoleThread;

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"test", cmd_test},
  {"date", cmd_date},
  {"alarm", cmd_alarm},
  {"sleep", cmd_sleep},
  {"clear", cmd_clear},
  {"mountsd", cmd_mountsd},
  {"umountsd", cmd_umountsd},
  {"tree", cmd_tree},
  {"cat", cmd_cat},
  {"sdiotest", cmd_sdiotest},
  {"testlog", cmd_testlog},
  {"logvalues", cmd_logvalues},
  {"logstop", cmd_logstop},
  {"eepromtest", cmd_eepromTest},
  {"eepromall", cmd_eepromAllData},
  {"canvalues", cmd_can_commvalues},
  {"canall", cmd_canall},
  {"canmppttest", cmd_canmppttest},
  {"candata_lc", cmd_candata_lc},
  {"candata_ml", cmd_candata_ml},
  {"candata_bms", cmd_candata_bms},
  {"candata_cell", cmd_candata_cell},
  {"canmonitor", cmd_canmonitor},
  {"lightvalues", cmd_lightvalues},
  {"light", cmd_lightblink},
  {"getlight", cmd_getLight},
  {"speedvalues", cmd_speedvalues},
  {"measvalues", cmd_measvalues},
  {"throttle", cmd_getThrottle},
  {"brake", cmd_getRegenBrake},
  {"steering", cmd_getSteeringAngle},
  {"brakevalues", cmd_brakevalues},
  {"cruisevalues", cmd_cruisevalues},
  {"setcruisevalues", cmd_setcruisevalues},
  {"cruise", cmd_cruise},
  {"regen_brake", cmd_regen_brake},
  {"dbshow", cmdfrappans_dspmessages},
  {"lcsleep", cmd_lcSleep},
  {"dspvalues", cmd_dspvalues},
  {"buttonvalues", cmd_buttonvalues},
  {"mainvalues", cmd_mainValues},
  {"safetyvalues", cmd_safetyvalues},
  {"fan_speed", cmd_fan_speed},
  {"calcvalues", cmd_calcvalues},
  {"set_current_limit", cmd_current_limit_switch},
  {"distancevalues", cmd_distancevalues},
  {"set_total_kmeter", cmd_set_total_kmeter},
  {"reset_kmeter", cmd_reset_kmeter},
  {"reset_meter", cmd_reset_meter},
  {"gpsvalues", cmd_nmeatest},
  {NULL, NULL}
};

static RTCAlarm alarmspec;

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream  *)&SD1,
  commands
};

void consoleInit(void){
  /* Shell initialization.*/
  sdStart(&SD1, &ser_cfg);
  shellInit();
  consoleThread = NULL;
}

void consoleStart(void){
  if (!consoleThread) {
    consoleThread = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
  }
  else if (chThdTerminated(consoleThread)) {
    chThdRelease(consoleThread);    /* Recovers memory of the previous shell.   */
    consoleThread = NULL;           /* Triggers spawning of a new shell.        */
  }
}

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriority(),
                           TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

/*
*
*/
void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argv;
  struct tm timp;

  if (argc == 0) {
    goto ERROR;
  }

  if ((argc == 1) && (strcmp(argv[0], "get") == 0)){
    unix_time = rtcGetTimeUnixSec(&RTCD1);

    if (unix_time == -1){
      chprintf(chp, "incorrect time in RTC cell\r\n");
    }
    else{
      chprintf(chp, "%D%s",unix_time," - unix time\r\n");
      rtcGetTimeTm(&RTCD1, &timp);
      chprintf(chp, "%s%s",asctime(&timp)," - formatted time string\r\n");
    }
    return;
  }

  if ((argc == 2) && (strcmp(argv[0], "set") == 0)){
    unix_time = atol(argv[1]);
    if (unix_time > 0){
      rtcSetTimeUnixSec(&RTCD1, unix_time);
      return;
    }
    else{
      goto ERROR;
    }
  }
  else{
    goto ERROR;
  }

ERROR:
  chprintf(chp, "Usage: date get\r\n");
  chprintf(chp, "       date set N\r\n");
  chprintf(chp, "where N is time in seconds sins Unix epoch\r\n");
  chprintf(chp, "you can get current N value from unix console by the command\r\n");
  chprintf(chp, "%s", "date +\%s\r\n");
  return;
}

/*
*
*/
void func_sleep(void){
  chSysLock();
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  PWR->CR |= (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_CSBF | PWR_CR_CWUF);
  RTC->ISR &= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF | RTC_ISR_WUTF | RTC_ISR_TAMP1F |
                RTC_ISR_TSOVF | RTC_ISR_TSF);
  __WFI();
}

void cmd_sleep(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: sleep\r\n");
    return;
  }
  chprintf(chp, "Going to sleep.\r\n");

  chThdSleepMilliseconds(200);

  /* going to anabiosis */
  func_sleep();
}

/*
 *
 */
void cmd_alarm(BaseSequentialStream *chp, int argc, char *argv[]){
  int i = 0;

  (void)argv;
  if (argc < 1) {
    goto ERROR;
  }

  if ((argc == 1) && (strcmp(argv[0], "get") == 0)){
    rtcGetAlarm(&RTCD1, 0, &alarmspec);
    chprintf(chp, "%D%s",alarmspec," - alarm in STM internal format\r\n");
    return;
  }

  if ((argc == 2) && (strcmp(argv[0], "set") == 0)){
    i = atol(argv[1]);
    alarmspec.tv_datetime = ((i / 10) & 7 << 4) | (i % 10) | RTC_ALRMAR_MSK4 |
                            RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK2;
    rtcSetAlarm(&RTCD1, 0, &alarmspec);
    return;
  }
  else{
    goto ERROR;
  }

ERROR:
  chprintf(chp, "Usage: alarm get\r\n");
  chprintf(chp, "       alarm set N\r\n");
  chprintf(chp, "where N is alarm time in seconds\r\n");
}

void cmd_clear(BaseSequentialStream *chp, int argc, char *argv[]){

  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);
}
