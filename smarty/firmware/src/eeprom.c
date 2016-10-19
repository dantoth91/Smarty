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

#define EEPROM_I2C_ADDR    0x50

#define EEPROM_MAX_ADDRESS	0xFFFF

static const I2CConfig i2ccfg =
{
OPMODE_I2C,
400000,
FAST_DUTY_CYCLE_2
};

struct eepromItem *eepromGetItembyName(enum eepromItemNames);

void eepromInit(void){
	i2cStart(&I2CD1, &i2ccfg);
}

struct eepromItem *eepromGetItembyName(enum eepromItemNames item_name){
  int i;
  
  i = 0;
  while(eepromitems[i].name != item_name) {
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

uint8_t eepromWrite(enum eepromItemNames name, uint32_t value){
  uint8_t tx[6];
  msg_t i2cmsg=0;

  int i;

  struct eepromItem *item;
  item = eepromGetItembyName(name);

  tx[0] = (uint8_t)(item->address >> 8);
  tx[1] = (uint8_t)(item->address);

  for(i = item->size; i > 0; i--){
    if(i > 1){
      tx[i + 1] = (uint8_t)(value >> ((i - 1) * 8));
    }
    else
      tx[i + 1] = (uint8_t)value;
  }
  
  i2cAcquireBus(&I2CD1);
  i2cmsg = i2cMasterTransmitTimeout(&I2CD1, EEPROM_I2C_ADDR, tx, item->size + 2, NULL, 0, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  if (i2cmsg != RDY_OK){
    return 1;
  }
  else
    return 0;
}

uint8_t eepromRead(enum eepromItemNames name, uint32_t *buff){
  uint8_t tx[2] = {0};
  msg_t i2cmsg=0;

  struct eepromItem *item;
  item = eepromGetItembyName(name);

  tx[0] = (uint8_t)(item->address >> 8);
  tx[1] = (uint8_t)(item->address);

  i2cAcquireBus(&I2CD1);
  i2cmsg = i2cMasterTransmitTimeout(&I2CD1, EEPROM_I2C_ADDR, tx, 2, buff, item->size, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  if (i2cmsg != RDY_OK){
    return 1;
  }
  else
    return 0;
}

void cmd_eepromTest(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  int addr = 0xFFFB;
  //uint32_t tx = rand() % 10000;
  int16_t tx = 900;
  uint32_t value = 0;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

  chprintf(chp, "EEPROM write test !\r\n"
  				"Address: 0x%02x, Value: %2d", addr, tx);

  if(eepromWrite(LAST_ITEM, tx) != 0){
  	chprintf(chp, "EEPROM write error! \r\n");
  }

  chprintf(chp, "\r\n----------------------- \r\n");
  chprintf(chp, "EEPROM read test ! \r\n");

  if(eepromRead(LAST_ITEM, &value) != 0){
    chprintf(chp, "EEPROM read error! \r\n");
  }
  chprintf(chp, "Address: 0x%02x, value: %2d\r\n",addr, value);
}

void cmd_eepromAllData(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t value = 0;

  enum eepromItemNames ch;

  static const char * const names[] = {

      "CRUISE_CONTROLL",
      "MIN_THROTTLE",
      "MAX_THROTTLE",
      "MIN_REGEN_BRAKE",
      "MAX_REGEN_BRAKE",
      "TOTAL_KMETER_COUNTER",
      "KMETER_COUNTER",
      "TOTAL_METER",
      "MIN_STR_ANGLE",
      "MAX_STR_ANGLE",
      "LAST_ITEM"};
      
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  
  chprintf(chp, "\x1B[%d;%dH", 0, 0);
  //chprintf(chp, "          NAME      DATA\r\n");
  for(ch = 0; ch < EEPROM_ITEMS_NUM; ch++) {
    
    if(eepromRead(ch, &value) != 0){
      chprintf(chp, "%s EEPROM read error!\r\n", names[ch]);
    }

    else{
      chprintf(chp, "%20s: %15d\r\n", names[ch], value);
      value = 0;
    }

    if (ch > 5)
    {
      chprintf(chp, "\r\n");
    }
  }
}