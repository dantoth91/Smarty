/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "eeprom.h"
#include "eepromItems.h"

#define EEPROM_I2C_ADDR    0x50

#define EEPROM_MAX_ADDRESS	0xFFFF

static const I2CConfig i2ccfg =
{
OPMODE_I2C,
400000,
FAST_DUTY_CYCLE_2
};

uint8_t eepromWrite(uint16_t addr, uint8_t size, uint32_t value);
uint8_t eepromRead(uint16_t addr, uint8_t size, uint32_t *buff);
//uint8_t eepromWriteItem(uint16_t addr, uint8_t size, uint32_t *buff);
uint32_t eepromReadItem(enum eepromItemNames name, uint32_t *buff);

void eepromInit(void){
	i2cStart(&I2CD1, &i2ccfg);
}

struct eepromItem *eepromGetItembyName(uint8_t id) {
  int i;
  
  i = 0;
  while(eepromitems[i].name != name) {
    if(eepromitems[i].name == EEPROM_ITEMS_NUM || i == EEPROM_ITEMS_NUM) {
      i = -1;
      break;
    }
    i++;
  }
  if(i < 0) {
    return NULL;
  }
  else {
    return &eepromitems[i];
  }
}

uint8_t eepromWrite(uint16_t addr, uint8_t size, uint32_t value){
  uint8_t tx[6];
  msg_t i2cmsg=0;

  int i;

  tx[0] = (uint8_t)(addr >> 8);
  tx[1] = (uint8_t)(addr);

  for(i = size; i > 0; i--){
    tx[i + 1] = (uint8_t)(value >> ((i - 1) * 8));
  }
  
  i2cAcquireBus(&I2CD1);
  i2cmsg = i2cMasterTransmitTimeout(&I2CD1, EEPROM_I2C_ADDR, tx, size + 2, NULL, 0, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  if (i2cmsg != RDY_OK){
    return 1;
  }
  else
    return 0;
}

uint8_t eepromRead(uint16_t addr, uint8_t size, uint32_t *buff){
  uint8_t tx[2] = {0};
  msg_t i2cmsg=0;

  tx[0] = (uint8_t)(addr >> 8);
  tx[1] = (uint8_t)(addr);

  i2cAcquireBus(&I2CD1);
  i2cmsg = i2cMasterTransmitTimeout(&I2CD1, EEPROM_I2C_ADDR, tx, 2, buff, size, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  if (i2cmsg != RDY_OK){
    return 1;
  }
  else
    return 0;
}

/*uint8_t eepromWriteItem(uint16_t addr, uint8_t size, uint32_t *buff){
  
}*/

uint32_t eepromReadItem(enum eepromItemNames name, uint32_t *buff){
  uint8_t size;
  struct eepromItem *item;
  item = eepromGetItembyName(name);

  if(eepromRead(uint16_t item->addr, item->size, (uint32_t *)(item->value)) != 0){
    buff = item.value;
    return 0;
  }

  else {
    buff = 0;
    return 1;
  }
}

void cmd_eepromTest(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  int addr = 0x0010;
  uint32_t tx = rand() % 1000000000;
  uint32_t value = 0;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

  chprintf(chp, "EEPROM write test !\r\n"
  				"Address: 0x%02x, Value: %2d", addr, tx);

  if(eepromWrite(addr, 4, tx) != 0){
  	chprintf(chp, "EEPROM write error! \r\n");
  }

  chprintf(chp, "\r\n----------------------- \r\n");
  chprintf(chp, "EEPROM read test ! \r\n");

  if(eepromReadItem(FIRST_TEST, &value) != 0){
    chprintf(chp, "EEPROM read error! \r\n");
  }
  chprintf(chp, "Address: 0x%02x, value: %2d\r\n",addr, value);
}