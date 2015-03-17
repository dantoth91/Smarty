/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

static enum eepromItemNames
{
  FIRST_TEST,
  EEPROM_ITEMS_NUM
};

/* int32_t    -   size = 4 
 * uint32_t   -   size = 4
 */
static struct eepromItem 
{
  uint8_t id;
  uint16_t address;
  uint8_t size;
  enum eepromItemNames name;
  uint32_t value;
};

/* EEPROM layout definition */

static struct eepromItem eepromitems[] = { 
  /* Engine Control Settings*/
  /* First test */
  { 0x0010, 0x0010, 4, FIRST_TEST, first_test},

  /* Engine Control Factor Tables*/
  /* Last item, always should be 0xFF */
  { 0xFFFF, 0, 0}
};