/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdio.h>

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
  uint8_t  custom_flag_1;
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
  int16_t pack_current;
  uint8_t  total_pack_cycle;
  uint8_t  pack_ccl;
  uint8_t  pack_dcl;
  uint16_t maximum_cell;
  uint8_t  custom_flag_2;
  uint16_t minimum_cell;
  uint8_t  high_cell_volt_num;
  uint8_t  low_cell_volt_num;
  uint8_t  supply_12v;
  uint8_t  fan_speed;
  uint16_t pack_open_voltage;
};

 /* Battery cell data */
static struct bms_cellItem 
{
  uint32_t id[34];
  uint16_t cell_voltage[34];
  uint16_t open_voltage[34];
  uint16_t cell_resistant[34];
};

/* LuxControl data */
static struct luxcontrolItem 
{
  uint32_t id[32];
  uint8_t temp[32];
  uint8_t curr_in[32];
  uint8_t curr_out[32];
  uint8_t efficiency[32];
  uint16_t status[32];
  uint32_t volt_in[32];
  uint16_t volt_out[32];
  uint16_t pwm[32];
};