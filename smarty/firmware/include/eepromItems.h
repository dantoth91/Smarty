/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

enum eepromItemNames
{
  CRUISE_CONTROLL,
  MIN_THROTTLE,
  MAX_THROTTLE,
  MIN_REGEN_BRAKE,
  MAX_REGEN_BRAKE,
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
  /* Set cruise control km/h */
  { 0x0010, 0x0010, 4, CRUISE_CONTROLL},
  /* Throttle pedal min ADC */
  { 0x0011, 0x0014, 4, MIN_THROTTLE},
  /* Throttle pedal max ADC */
  { 0x0012, 0x0018, 4, MAX_THROTTLE},
  /* Regenerative brake pedal min ADC */
  { 0x0013, 0x001C, 4, MIN_REGEN_BRAKE},
  /* Regenerative brake pedal max ADC */
  { 0x0014, 0x0020, 4, MAX_REGEN_BRAKE},

  /* Last item, always should be 0xFFFF */
  { 0xFFFB, 0xFFFB, 4, LAST_ITEM}
};