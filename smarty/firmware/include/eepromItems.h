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
  TOTAL_KMETER_COUNTER,
  KMETER_COUNTER,
  TOTAL_METER,
  MIN_STR_ANGLE,
  MAX_STR_ANGLE,
  LAST_ITEM,
  EEPROM_ITEMS_NUM
};

static struct eepromItem 
{
  uint16_t id;
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
  /* Total KMeter Counter */
  { 0x0015, 0x0024, 4, TOTAL_KMETER_COUNTER},
  /* KMeter counter */
  { 0x0016, 0x0028, 4, KMETER_COUNTER},
  /* Total meter counter */
  { 0x0017, 0x003C, 4, TOTAL_METER},
  /* Steering angle min ADC */
  { 0x0018, 0x0040, 4, MIN_STR_ANGLE},
  /* Steering angle max ADC */
  { 0x0019, 0x0044, 4, MAX_STR_ANGLE},

  /* Last item, always should be 0xFFFF */
  { 0xFFFB, 0xFFFB, 4, LAST_ITEM}
};