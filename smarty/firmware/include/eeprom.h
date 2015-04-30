/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

void eepromInit(void);

//struct eepromItem *eepromGetItembyName(uint8_t id);
uint8_t eepromWrite(uint16_t addr, uint8_t size, uint32_t value);
uint8_t eepromRead(uint16_t addr,uint8_t size, uint32_t *buff);
//uint8_t eepromWriteItem(uint16_t addr, uint8_t size, uint32_t *buff);
//uint32_t eepromReadItem(enum eepromItemNames name, uint32_t *buff);


void cmd_eepromTest(BaseSequentialStream *chp, int argc, char *argv[]);

#endif