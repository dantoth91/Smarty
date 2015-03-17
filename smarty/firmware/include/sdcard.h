/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef SDCARD_H_INCLUDED
#define SDCARD_H_INCLUDED

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/**
 * @brief   Sdcard errors
 */
typedef enum {
  SDCARD_OK = 0,                   /**< OK.                                 */
  SDCARD_ERROR_CONNECT = -1,       /**< Connection error (card not present) */
  SDCARD_ERROR_FSMOUNT = -2,       /**< File system mount error             */
  SDCARD_ERROR_READ = -3,          /**< File system read error              */
} sdcarderror_t;

void sdcardInit(void);
sdcarderror_t sdcardMount(void);
sdcarderror_t sdcardUMount(void);
bool_t sdcardIsMounted(void);

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mountsd(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_umountsd(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_cat(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_sdiotest(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_debug(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
