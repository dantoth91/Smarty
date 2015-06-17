/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

enum eepromItemNames
{
  CRUISE_CONTROLL,
  CENTER_ITEM,
  LAST_ITEM,
  EEPROM_ITEMS_NUM
};

static struct eepromItem 
{
  uint8_t id;
  uint16_t address;
  uint8_t size;
  enum eepromItemNames name;
};

/* 
 * EEPROM layout definition
 * int32_t    -   size = 4
 * uint32_t   -   size = 4
 */
static struct eepromItem eepromitems[] = {
  /* Engine Control Settings*/
  /* First item */
  { 0x0010, 0x0010, 4, CRUISE_CONTROLL},

  /* Center item */
  { 0x0011, 0x0014, 4, CENTER_ITEM},

  /* Last item, always should be 0xFFFF */
  { 0xFFFB, 0xFFFB, 4, LAST_ITEM}
};