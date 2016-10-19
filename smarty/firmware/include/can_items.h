/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdio.h>

/* Modulux data */
static struct moduluxItems
{
  uint32_t id;
  uint16_t MODULE1_TEMP;
  uint16_t MODULE8_TEMP;
  uint16_t MODULE6_TEMP;
  uint16_t MODULE12_TEMP;
  uint16_t MODULE2_TEMP;
  uint16_t sun_current;
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


#define NULL_AMPER_ADC   2040
#define AMP_PER_ADC      43.3

/* CCL Items  */
static struct CCLItem{
  uint8_t id[6];
  uint16_t current_adc1[6];
  uint16_t current_adc2[6];
  uint16_t current_adc3[6];
  uint8_t temp1[6];
  uint8_t temp2[6];
  uint8_t temp3[6];
};

/* IOTC data */
/*static struct IOTCItem 
{
  uint32_t id[32];
  uint16_t ain_1[32];
  uint16_t ain_2[32];
  uint16_t ain_3[32];
  uint16_t ain_4[32];
  uint16_t ain_5[32];
};*/
static struct IOTCItem 
{
  uint32_t id;
  uint16_t ain_1;
  uint16_t ain_2;
  uint16_t ain_3;
  uint16_t ain_4;
  uint16_t ain_5;
};
/* Tire Pressure Items */
static struct TirePressureStructure{
  uint8_t id[6];
  uint8_t pressure[6];
  uint16_t temperature[6];
  uint8_t Flags[6];
  uint8_t TirePressureThresholdDetection[6];
};
#define NUM_OF_SENSORS 6

//extern struct moduluxItems mlitems;
//extern struct bmsItems bmsitems;
//extern struct bms_cellItem cellitems;
//extern struct luxcontrolItem lcitems;
//extern struct IOTClItem IOTCitems;
