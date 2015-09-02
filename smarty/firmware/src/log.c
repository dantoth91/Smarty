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
#include "logItems.h"

#include "can_comm.h"
#include "can_items.h"

#include "meas.h"

struct moduluxItems mlitems;
struct bmsItems bmsitems;
struct bms_cellItem cellitems;
struct luxcontrolItem lcitems;

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

  logStart();
  
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
  uint16_t i, db;
  size = LOG_ITEMS_NUM;
  
  if (logGetState() == LOG_RUNNING){


    chSysLock();
    logitems[NUM].value = logPeriod;
    logitems[TIME].value = chTimeNow() - logStartTime;
    /* Analog meas */
    logitems[UBAT].value = measGetValue(MEAS_UBAT);
    logitems[TEMP1].value = measGetValue(MEAS_TEMP1);
    logitems[TEMP2].value = measGetValue(MEAS_TEMP2);
    logitems[STEERING_SEN].value = measGetValue(MEAS_STEERING);
    logitems[CURR1].value = measGetValue_2(MEAS2_CURR1);
    logitems[THROTTLE].value = measGetValue_2(MEAS2_THROTTLE);
    logitems[REGEN_BRAKE].value = measGetValue_2(MEAS2_REGEN_BRAKE);
    /* Cruise control */
    logitems[CRUISE_SWITCH].value = cruiseStatus();
    logitems[CRUISE_SET].value = cruiseGetPWM();
    logitems[CRUISE_SUBSTRACT].value = cruiseGetCurrLimitSubstract();
    logitems[SPEED_KMPH].value = speedGetSpeed();
    logitems[SPEED_RPM].value = speedGetRpm();
    /* Modulux */
    logitems[SOLAR_TEMP_MODULE_1].value = mlitems.MODULE1_TEMP;
    logitems[SOLAR_TEMP_MODULE_8].value = mlitems.MODULE8_TEMP;
    logitems[SOLAR_TEMP_MODULE_6].value = mlitems.MODULE6_TEMP;
    logitems[SOLAR_TEMP_MODULE_12].value = mlitems.MODULE12_TEMP;
    logitems[SOLAR_TEMP_MODULE_2].value = mlitems.MODULE2_TEMP;
    logitems[SOLARCELL_CURRENT].value = mlitems.sun_current;
    /* BMS */
    logitems[BMS_CUSTOM_FLAG_1].value = bmsitems.custom_flag_1;
    logitems[BMS_PACK_INST_VOLT].value = bmsitems.pack_inst_volt;
    logitems[BMS_PACK_SOC].value = bmsitems.pack_soc;
    logitems[BMS_PACK_HEALT].value = bmsitems.pack_healt;
    logitems[BMS_PACK_AMPS].value = bmsitems.pack_amps;
    logitems[BMS_PACK_RESIST].value = bmsitems.pack_resist;

    logitems[BMS_AVERAGE_TEMP].value = bmsitems.average_temp;
    logitems[BMS_INTERNAL_TEMP].value = bmsitems.internal_temp;
    logitems[BMS_LOW_CELL_VOLT].value = bmsitems.low_cell_volt;
    logitems[BMS_HIGH_CELL_VOLT].value = bmsitems.high_cell_volt;
    logitems[BMS_AVG_CELL_VOLT].value = bmsitems.avg_cell_voltage;

    logitems[BMS_PACK_CURRENT].value = bmsitems.pack_current;
    logitems[BMS_PACK_CYCLE].value = bmsitems.total_pack_cycle;
    logitems[BMS_PACK_CCL].value = bmsitems.pack_ccl;
    logitems[BMS_PACK_DCL].value = bmsitems.pack_dcl;
    logitems[BMS_MAXIMUM_CELL_VOLT].value = bmsitems.maximum_cell;
    logitems[BMS_CUSTOM_FLAG_2].value = bmsitems.custom_flag_2;

    logitems[BMS_MINIMUM_CELL_VOLT].value = bmsitems.minimum_cell;
    logitems[BMS_HIGH_CELL_VOLT_NUM].value = bmsitems.high_cell_volt_num;
    logitems[BMS_LOW_CELL_VOLT_NUM].value = bmsitems.low_cell_volt_num;
    logitems[BMS_SUPPLY_12V].value = bmsitems.supply_12v;
    logitems[BMS_FAN_SPEED].value = bmsitems.fan_speed;
    logitems[BMS_PACK_OPEN_VOLT].value = bmsitems.pack_open_voltage;
    
    for (i = 43, db = 0; db < 34; i+=3, db++)
    {
      logitems[i].value = cellitems.cell_voltage[db];
      logitems[i+1].value = cellitems.cell_resistant[db];
      logitems[i+2].value = cellitems.open_voltage[db];
    }

    for (i = 145, db = 0; db < 15; i+=8, db++)
    {
      logitems[i].value = lcitems.temp[db];
      logitems[i+1].value = lcitems.curr_in[db];
      logitems[i+2].value = lcitems.curr_out[db];
      logitems[i+3].value = lcitems.status[db];
      logitems[i+4].value = lcitems.volt_in[db];
      logitems[i+5].value = lcitems.volt_out[db];
      logitems[i+6].value = lcitems.pwm[db];
      logitems[i+7].value = lcitems.curr_in_from_pwm[db];
    }
    chSysUnlock();
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

void cmd_logvalues(BaseSequentialStream *chp, int argc, char *argv[]) {
  enum logStates state;
  uint32_t old_logPeriod = 0;
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

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
}
