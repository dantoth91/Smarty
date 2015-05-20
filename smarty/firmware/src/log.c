/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdlib.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "chrtclib.h"

#include "ff.h"
#include "sdcard.h"
#include "can_items.h"

#include "log.h"

#define LOG_WA_SIZE (2048)
#define LOG_BUFFER_SIZE (100)
#define LOG_MAX_IDLE_CNT (200 / 20)
#define LOG_MAX_PERIOD (25000)

struct logEntry
{
  uint32_t time;
  int8_t lc1_temp;
  int8_t lc1_curr_in;
  int8_t lc1_curr_out;
  int8_t lc1_eff;
  uint16_t lc1_v_in;
  uint16_t lc1_v_out;
  uint16_t lc1_status;
  uint16_t lc1_pwm;
} __attribute__((packed));

struct logEntry logBuffer[LOG_BUFFER_SIZE];
int logReadPointer;
int logWritePointer;

enum logStates logState;
FIL logFileObject;
bool_t logStartRequest;
bool_t logStopRequest;
systime_t logStartTime;
int32_t logLastIgnCount;
uint32_t logIdleCount;

//char logFileName[] = "0:mm12_yymmdd_hhmmss.mega";
char logFileName[] = "test.txt";

static msg_t logThread(void *arg);

int logCount = 0;

/*
 * Initializes log 
 */
void logInit(void){
  logReadPointer = 0;
  logWritePointer = 0;
  logState = LOG_STOPPED;
  logStartRequest = FALSE;
  logStopRequest = FALSE;
  logStartTime = 0;
  logLastIgnCount = -3;
  logIdleCount = 0;

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
  int actualReadPointer;
  chSysLock();
  actualReadPointer = logReadPointer;
  chSysUnlock();
  systime_t logTime;

  if (logGetState() == LOG_RUNNING){
    chTimeNow() - logStartTime;                     // jelenlegi idő - logolások utáni jelenlegi idő
    if (logCount == 0)
    {
      logTime = chTimeNow();
    }
    logCount++;

    logBuffer[logWritePointer].time = chTimeNow() - logTime; //- logStartTime;
    logBuffer[logWritePointer].lc1_temp = (uint8_t)lcitems[0].temp;
    logBuffer[logWritePointer].lc1_curr_in = (uint8_t)lcitems[0].curr_in;
    logBuffer[logWritePointer].lc1_curr_out = (uint8_t)lcitems[0].curr_out;
    logBuffer[logWritePointer].lc1_eff = (uint8_t)lcitems[0].efficiency;
    logBuffer[logWritePointer].lc1_v_in = (uint16_t)lcitems[0].volt_in;
    logBuffer[logWritePointer].lc1_v_out = (uint16_t)lcitems[0].volt_out;
    logBuffer[logWritePointer].lc1_status = (uint16_t)lcitems[0].status;
    logBuffer[logWritePointer].lc1_pwm = (uint16_t)lcitems[0].pwm;
    chSysLock();
    logWritePointer = logWritePointer < LOG_BUFFER_SIZE - 1 ? logWritePointer + 1 : 0;
    chSysUnlock();
    logIdleCount = 0;
  }
  else {
    logIdleCount++;
  }
}

/*
 * Log task writes data to file 
 */
static msg_t logThread(void *arg) {
  bool_t isStartRequest;
  bool_t isStopRequest;
  int actualWritePointer;
  
  FRESULT err;
  UINT bytesWritten;
  struct tm timp;
  
  (void)arg;
  chRegSetThreadName("logTask");
  
  while(TRUE){
    chSysLock();
    isStartRequest = logStartRequest;
    isStopRequest = logStopRequest;
    logStartRequest = FALSE;
    logStopRequest = FALSE;
    actualWritePointer = logWritePointer;
    chSysUnlock();

    switch(logState){
      case LOG_STOPPED:
        logCount = 0;
        if(isStartRequest){
          sdcardMount();
          if (sdcardIsMounted()) {
            rtcGetTimeTm(&RTCD1, &timp);
            /* Adjust file name */
            /*twodigit(timp.tm_year - 100, &logFileName[7]);
            twodigit(timp.tm_mon + 1, &logFileName[9]);
            twodigit(timp.tm_mday, &logFileName[11]);
            twodigit(timp.tm_hour, &logFileName[14]);
            twodigit(timp.tm_min, &logFileName[16]);
            twodigit(timp.tm_sec, &logFileName[18]);*/
            
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
        while(actualWritePointer != logReadPointer)
        {
          err = f_write(&logFileObject, (void *)&logBuffer[logReadPointer], sizeof(struct logEntry), &bytesWritten);
          if (err != FR_OK || bytesWritten != sizeof(struct logEntry)){
            isStopRequest = TRUE;
            break;
          }
          err = f_sync(&logFileObject);
          if (err != FR_OK){
            isStopRequest = TRUE;
          }
          chSysLock();
          logReadPointer = logReadPointer < LOG_BUFFER_SIZE - 1 ? logReadPointer + 1 : 0;
          chSysUnlock();
        }
        if(isStopRequest){
          logReadPointer = 0;
          logWritePointer = 0;
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
    chThdSleepMilliseconds(500);
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
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  logStart();
  chprintf(chp, "Logging starting...\r\n");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    state = logGetState();
    chprintf(chp, "\x1B[%d;%dH", 0, 0);
    chprintf(chp, "Filename = %s Logtime = %D    \r\n", logFileName, chTimeNow() - logStartTime);
    chprintf(chp, "logState = %s \r\n", 
             state == LOG_RUNNING ? "LOG_RUNNING" : \
             state == LOG_STOPPED ? "LOG_STOPPED" : \
             state == LOG_STOPPING ? "LOG_STOPPING" : "LOG_DOWNLOADING");
    if (state == LOG_RUNNING){
      chprintf(chp, "ReadPointer = %D WritePointer = %D    \r\n", logReadPointer, logWritePointer);
    }
    chThdSleepMilliseconds(200);
  }
  logStop();
  chprintf(chp, "\r\nLogging stopped\r\n");
}
