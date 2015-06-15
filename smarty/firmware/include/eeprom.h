/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

#include "eepromItems.h"

void eepromInit(void);

uint8_t eepromWrite(enum eepromItemNames name, uint32_t value);
uint8_t eepromRead(enum eepromItemNames name, uint32_t *buff);

void cmd_eepromTest(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_eepromAllData(BaseSequentialStream *chp, int argc, char *argv[]);

#endif