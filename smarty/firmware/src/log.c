/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chrtclib.h"

#include "ff.h"
#include "sdcard.h"

#include "log.h"
#include "meas.h"
#include "logItems.h"

/*
CAN-en küldeni
sebesség
tempomat beállított értéke
gáz állás
gázpedád állás
fákpedál állás
2db féknyomás
motortúlmelegedés mérés
motor áram
tempomat státusz
lámpa fel/le
féklámpa fel/le
index jobb/bal
12V feszültséget (UBAT)

LuxControlok adatai
BMS adatai
*/

#define LOG_WA_SIZE (2048)

enum logStates logState;
FIL logFileObject;
bool_t logStartRequest;
bool_t logStopRequest;
systime_t logStartTime;

char logFileName[] = "0:ml_yymmdd_hhmmss.mega";

static msg_t logThread(void *arg);
static uint16_t size = 0;
static uint32_t logPeriod = 0;
static uint32_t szam = 0;

/*
 * Initializes log 
 */
void logInit(void){
  logState = LOG_STOPPED;
  logStartRequest = FALSE;
  logStopRequest = FALSE;
  logStartTime = 0;
  logPeriod = 0;
  size = 0;

  static WORKING_AREA(waLog, LOG_WA_SIZE);
  chThdCreateStatic(waLog, sizeof(waLog), LOWPRIO, logThread, NULL);
}
 
void twodigit(int n, char s[]){
  if(n >= 0 && n < 10){
    s[0] = '0';
    s[1] = n + '0';
  }
  else if(n < 100){
    s[0] = n / 10 + '0';
    s[1] = n % 10 + '0';
  }
  else {
    s[0] = 'X';
    s[1] = 'X';
  }
}

/*
 * Starts a logging session: opens new file with timestamp name and writes header
 */
void logStart(void){
  chSysLock();
  logStartTime = chTimeNow();
  logPeriod = 0;
  logStartRequest = TRUE;
  chSysUnlock();
}

/*
 * Stops a logging session: flushes and closes the file
 */
void logStop(void){
  chSysLock();
  if(logState == LOG_RUNNING){
    logStopRequest = TRUE;
  }
  chSysUnlock();
}

/*
 * Main modue to be called from periodic task, handles buffering and flushing of logged data 
 */
void logCalc(void){
  size = LOG_ITEMS_NUM;
  
  if (logGetState() == LOG_RUNNING){

    logitems[NUM].value = logPeriod;
    logitems[TIME].value = chTimeNow() - logStartTime;
    logitems[UBAT].value = measGetValue(MEAS_UBAT);
    logitems[BRAKE_P1].value = measGetValue(MEAS_BRAKE_PRESSURE1);
    logitems[BRAKE_P2].value = measGetValue(MEAS_BRAKE_PRESSURE2);
    logitems[FUT_SEN1].value = measGetValue(MEAS_SEN2);
    logitems[FUT_SEN2].value = measGetValue(MEAS_SEN3);
    logitems[FUT_SEN3].value = measGetValue(MEAS_SEN4);
    logitems[FUT_SEN4].value = measGetValue(MEAS_SEN5);
    logitems[CURR1].value = measGetValue_2(MEAS2_CURR1);
    logitems[THROTTLE].value = measGetValue_2(MEAS2_THROTTLE);
    logitems[REGEN_BRAKE].value = measGetValue_2(MEAS2_REGEN_BRAKE);
  }
}

/*
 * Log task writes data to file 
 */
static msg_t logThread(void *arg) {
  bool_t isStartRequest;
  bool_t isStopRequest;
  int i;
  
  FRESULT err;

  struct tm timp;
  
  (void)arg;
  chRegSetThreadName("logTask");
  
  while(TRUE){
    chSysLock();
    isStartRequest = logStartRequest;
    isStopRequest = logStopRequest;
    logStartRequest = FALSE;
    logStopRequest = FALSE;
    chSysUnlock();

    switch(logState){
      case LOG_STOPPED:
        if(isStartRequest){
          sdcardMount();
          if (sdcardIsMounted()) {
            rtcGetTimeTm(&RTCD1, &timp);
            /* Adjust file name */
            twodigit(timp.tm_year - 100, &logFileName[5]);
            twodigit(timp.tm_mon + 1, &logFileName[7]);
            twodigit(timp.tm_mday, &logFileName[9]);
            twodigit(timp.tm_hour, &logFileName[12]);
            twodigit(timp.tm_min, &logFileName[14]);
            twodigit(timp.tm_sec, &logFileName[16]);

            err = f_open(&logFileObject, logFileName, FA_WRITE | FA_OPEN_ALWAYS);
            if (err == FR_OK) {
              chSysLock();
              logStartTime = chTimeNow();
              logState = LOG_RUNNING;
              chSysUnlock();
            }
          }
        }
        break;
      case LOG_RUNNING:
        if(isStopRequest){
          chSysLock();
          logState = LOG_STOPPING;
          chSysUnlock();
        }

        else if (logPeriod == 0)
        {
          for (i = 0; i < LOG_ITEMS_NUM; ++i)
          {
            err = f_printf (&logFileObject, "%s", logitems[i].name);
            if (err == EOF){
              isStopRequest = TRUE;
            }
          }
          err = f_puts ("\r\n", &logFileObject);
          if (err == EOF){
              isStopRequest = TRUE;
            }

          for (i = 0; i < LOG_ITEMS_NUM; ++i)
          {
            err = f_printf (&logFileObject, "%s", logitems[i].convert);
            if (err == EOF){
              isStopRequest = TRUE;
            }
          }
          err = f_puts ("\r\n", &logFileObject);
          if (err == EOF){
            isStopRequest = TRUE;
          }
          szam = EOF;
          logPeriod += 1;
        }         
        else if (logPeriod > 0)
        {
          for (i = 0; i < LOG_ITEMS_NUM; ++i)
          {
            err = f_printf (&logFileObject, "%d;", logitems[i].value);
            if (err == EOF){
              isStopRequest = TRUE;
            }
          }
          err = f_puts ("\r\n", &logFileObject);
          if (err == EOF){
            isStopRequest = TRUE;
          }
          logPeriod += 1;
        }
          
        err = f_sync(&logFileObject);
        if (err != FR_OK){
          isStopRequest = TRUE;
        }

        if(isStopRequest){
          f_close(&logFileObject);
          sdcardUMount();
          chSysLock();
          logState = LOG_STOPPED;
          chSysUnlock();
        }
        break;
      case LOG_STOPPING:
        chSysLock();
        logState = LOG_STOPPED;
        chSysUnlock();
        break;
      case LOG_DOWNLOADING:
        break;
      default:
        break;
    }
    chThdSleepMilliseconds(50);

  }
  return 0; /* Never executed.*/
}

bool_t logStartDownload(char **filename){
  bool_t ret;
  chSysLock();
  if(logState == LOG_STOPPED){
    logState = LOG_DOWNLOADING;
    ret = TRUE;
  }
  chSysUnlock();
  *filename = ret ? logFileName : NULL;
  return ret;
}

void logStopDownload(void){
  chSysLock();
  if(logState == LOG_DOWNLOADING){
    logState = LOG_STOPPED;
  }
  chSysUnlock();
}

enum logStates logGetState(void){
  enum logStates ret;
  
  chSysLock();
  ret = logState;
  chSysUnlock();
  
  return ret;
}

systime_t logGetTime(void){
  systime_t ret;

  chSysLock();
  ret = chTimeNow() - logStartTime;
  chSysUnlock();

  return ret;  
}

void cmd_testlog(BaseSequentialStream *chp, int argc, char *argv[]) {
  enum logStates state;
  uint32_t old_logPeriod = 0;
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  logStart();
  chprintf(chp, "Logging starting...\r\n");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    if(old_logPeriod != logPeriod){
      chprintf(chp, "\x1B[%d;%dH", 0, 0);
      state = logGetState();
      chprintf(chp, "\x1B[%d;%dH", 0, 0);
      chprintf(chp, "Filename = %s Logtime = %D    \r\n", logFileName, chTimeNow() - logStartTime);
      chprintf(chp, "logState = %s \r\n", 
               state == LOG_RUNNING ? "LOG_RUNNING" : \
               state == LOG_STOPPED ? "LOG_STOPPED" : \
               state == LOG_STOPPING ? "LOG_STOPPING" : "LOG_DOWNLOADING");
      chprintf(chp, "size: %d\r\n", size);
      chprintf(chp, "logPeriod: %d\r\n", logPeriod);
      chprintf(chp, "szam: %d\r\n", szam);
      old_logPeriod = logPeriod;
    }
    chThdSleepMilliseconds(10);
  }
  logStop();
  chprintf(chp, "\r\nLogging stopped\r\n");
}