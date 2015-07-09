/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

/* Modulux data */
static struct moduluxItems
{
  uint32_t id;
  uint16_t onevire_1;
  uint16_t onevire_2;
  uint16_t onevire_3;
  uint16_t onevire_4;
};

/* BMS data */
static struct bmsItems
{
  uint32_t id;
  uint8_t  relay;
  uint16_t pack_inst_volt;
  uint16_t pack_soc;
  uint8_t  pack_healt;
  uint16_t pack_amps;
  uint8_t  pack_resist;
  uint8_t  average_temp;
  uint8_t  internal_temp;
  uint16_t low_cell_volt;
  uint16_t high_cell_volt;
  uint16_t avg_cell_voltage;
  uint16_t pack_current;
  uint8_t  total_pack_cycle;
  uint8_t  pack_ccl;
  uint8_t  pack_dcl;
  uint16_t maximum_cell;
  uint8_t  custom_flag;
  uint16_t minimum_cell;
  uint8_t  high_cell_volt2;
  uint8_t  low_cell_volt2;
  uint8_t  supply_12v;
  uint8_t  fan_speed;
  uint16_t pack_open_voltage;
};

 /* Battery cell data */
static struct bms_celllItem 
{
  uint32_t id;
  uint16_t cell_voltage;
  uint16_t open_voltage;
  uint16_t cell_resistant;
};

static struct bms_celllItem cellitems[] = { 
  { 0x00, 0, 0, 0},
  { 0x01, 0, 0, 0},
  { 0x02, 0, 0, 0},
  { 0x03, 0, 0, 0},
  { 0x04, 0, 0, 0},
  { 0x05, 0, 0, 0},
  { 0x06, 0, 0, 0},
  { 0x07, 0, 0, 0},
  { 0x08, 0, 0, 0},
  { 0x09, 0, 0, 0},
  { 0x0A, 0, 0, 0},
  { 0x0B, 0, 0, 0},
  { 0x0C, 0, 0, 0},
  { 0x0D, 0, 0, 0},
  { 0x0E, 0, 0, 0},
  { 0x0F, 0, 0, 0},
  { 0x10, 0, 0, 0},
  { 0x11, 0, 0, 0},
  { 0x12, 0, 0, 0},
  { 0x13, 0, 0, 0},
  { 0x14, 0, 0, 0},
  { 0x15, 0, 0, 0},
  { 0x16, 0, 0, 0},
  { 0x17, 0, 0, 0},
  { 0x18, 0, 0, 0},
  { 0x19, 0, 0, 0},
  { 0x1A, 0, 0, 0},
  { 0x1B, 0, 0, 0},
  { 0x1C, 0, 0, 0},
  { 0x1C, 0, 0, 0},
  { 0x1E, 0, 0, 0},
  { 0x1F, 0, 0, 0},
  { 0x20, 0, 0, 0},
  { 0x21, 0, 0, 0}
};

/* LuxControl data */
static struct luxcontrolItem 
{
  uint32_t id;
  uint8_t temp;
  uint8_t curr_in;
  uint8_t curr_out;
  uint8_t efficiency;
  uint16_t status;
  uint32_t volt_in;
  uint16_t volt_out;
  uint16_t pwm;
};

static struct luxcontrolItem lcitems[] = { 
  { 0x40, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x41, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x42, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x43, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x44, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x45, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x46, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x47, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x48, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x49, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x4A, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x4B, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x4C, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x4D, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x4E, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x4F, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x50, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x51, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x52, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x53, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x54, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x55, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x56, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x57, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x58, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x59, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x5A, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x5B, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x5C, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x5D, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x5E, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0x5F, 0, 0, 0, 0, 0, 0, 0, 0}
};