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

#include "log.h"

#define LOG_WA_SIZE (2048)
#define LOG_BUFFER_SIZE (100)
#define LOG_MAX_IDLE_CNT (200 / 20)
#define LOG_MAX_PERIOD (25000)

struct logEntry
{
  uint32_t time;
  int32_t igncnt;
  int32_t injcnt;
  int16_t injtime;
  int16_t ignangle;
  uint16_t rpm;
  uint16_t ubat;
  uint16_t cht;
  uint16_t mat;
  uint16_t cyt;
  uint16_t trq;
  uint16_t eff;
  uint16_t fup;
  uint16_t cls;
  uint16_t bws;
  uint16_t avg_bws;
  uint16_t fuelcon;
  uint16_t afr;
  int32_t lon;
  int32_t lat;
  int16_t alt;
  int16_t vel;
  int8_t hdg;
  uint8_t svs;
  int16_t sta;
  uint16_t pwm;
  uint16_t bws_rpm;
} __attribute__((packed));

struct logEntry logBuffer[LOG_BUFFER_SIZE];
int logReadPointer;
int logWritePointer;

enum logStates logState;
FIL logFileObject;
bool_t logStartRequest;
bool_t logStopRequest;
systime_t logStartTime;
uint32_t logIdleCount;

char logFileName[] = "test.log";

static msg_t logThread(void *arg);
static allapot;
static allapot2;

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
  logIdleCount = 0;
  allapot = 0;
  allapot2 = 0;

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
        allapot = 1;
/*     if((   actualIgnCount != logLastIgnCount \
        && actualLastPeriod < LOG_MAX_PERIOD)          \
        || logIdleCount > LOG_MAX_IDLE_CNT){
                                                                       // igncount = -2
        if(actualIgnCount > logLastIgnCount){               // actualIgnCount = ecGetIgnCount() > logLastIgnCount = -2;
            ecGetLastIgnStartTime() - logStartTime;         // ecGetLastIgnStartTime() = gyujtásonként a jelenlegi idő; logStartTime = logolás után a jelenlegi idő
        }
        else{
            chTimeNow() - logStartTime;                     // jelenlegi idő - logolások utáni jelenlegi idő
        }

      logBuffer[logWritePointer].time = (actualIgnCount > logLastIgnCount ? ecGetLastIgnStartTime() : chTimeNow()); //- logStartTime;
      logBuffer[logWritePointer].igncnt = actualIgnCount;*/
      
      chSysLock();
      logWritePointer = logWritePointer < LOG_BUFFER_SIZE - 1 ? logWritePointer + 1 : 0;
      chSysUnlock();
      /*logLastIgnCount = actualIgnCount;*/
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
  char teststring[10];
  teststring[0] = "H";
  teststring[1] = "e";
  teststring[2] = "l";
  
  FRESULT err;
  UINT bytesWritten;
  
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
        if(isStartRequest){
          sdcardMount();
          if (sdcardIsMounted()) {
            err = f_open(&logFileObject, logFileName, FA_WRITE | FA_OPEN_ALWAYS);
            if (err == FR_OK) {
              allapot = 50;
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
          allapot = 256;
          err = f_write(&logFileObject, teststring, sizeof(teststring), &bytesWritten);
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
    chprintf(chp, "allapot: %15d", allapot);
    chprintf(chp, "allapot2: %15d", allapot2);
    chThdSleepMilliseconds(200);
  }
  logStop();
  chprintf(chp, "\r\nLogging stopped\r\n");
}
