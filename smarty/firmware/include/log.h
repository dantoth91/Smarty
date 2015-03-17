/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef LOG_H
#define LOG_H

enum logStates
{
  LOG_STOPPED,
  LOG_RUNNING,
  LOG_STOPPING,
  LOG_DOWNLOADING
};

void logInit(void);
void logCalc(void);
void logStart(void);
void logStop(void);
bool_t logStartDownload(char **filename);
void logStopDownload(void);

enum logStates logGetState(void);
systime_t logGetTime(void);

void cmd_testlog(BaseSequentialStream *chp, int argc, char *argv[]);

#endif