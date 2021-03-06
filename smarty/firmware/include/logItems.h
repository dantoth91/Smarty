/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

enum logItemNames
{
  NUM,
  TIME,
  /* Analog meas */
  UBAT,
  TEMP1,
  TEMP2,
  STEERING_SEN,
  CURR1,
  THROTTLE,
  REGEN_BRAKE,
  CHP_E_B,
  CHP_E_J,
  CHP_H_B,
  CHP_H_J,
  /* Cruise control */
  CRUISE_SWITCH,
  CRUISE_SET,
  CRUISE_SUBSTRACT,
  SPEED_KMPH,
  SPEED_RPM,
  AVG_SPEED,
  /* GPS */
  GPS_LON,
  GPS_LAT,
  GPS_ALT,
  GPS_VEL,
  GPS_SVS,
  GPS_TIME,
  /* Modulux */
  SOLAR_TEMP_MODULE_1,
  SOLAR_TEMP_MODULE_8,
  SOLAR_TEMP_MODULE_6,
  SOLAR_TEMP_MODULE_12,
  SOLAR_TEMP_MODULE_2,
  SOLARCELL_CURRENT,
  /* BMS */
  BMS_CUSTOM_FLAG_1,
  BMS_PACK_INST_VOLT,
  BMS_PACK_SOC,
  BMS_PACK_HEALT,
  BMS_PACK_AMPS,
  BMS_PACK_RESIST,
  BMS_AVERAGE_TEMP,
  BMS_INTERNAL_TEMP,
  BMS_LOW_CELL_VOLT,
  BMS_HIGH_CELL_VOLT,
  BMS_AVG_CELL_VOLT,
  BMS_PACK_CURRENT,
  BMS_PACK_CYCLE,
  BMS_PACK_CCL,
  BMS_PACK_DCL,
  BMS_MAXIMUM_CELL_VOLT,
  BMS_CUSTOM_FLAG_2,
  BMS_MINIMUM_CELL_VOLT,
  BMS_HIGH_CELL_VOLT_NUM,
  BMS_LOW_CELL_VOLT_NUM,
  BMS_SUPPLY_12V,
  BMS_FAN_SPEED,
  BMS_PACK_OPEN_VOLT,
  /* Battery cell data */
  CELL_1_VOLT,
  CELL_1_RESIST,
  CELL_1_OPEN_VOLT,
  CELL_2_VOLT,
  CELL_2_RESIST,
  CELL_2_OPEN_VOLT,
  CELL_3_VOLT,
  CELL_3_RESIST,
  CELL_3_OPEN_VOLT,
  CELL_4_VOLT,
  CELL_4_RESIST,
  CELL_4_OPEN_VOLT,
  CELL_5_VOLT,
  CELL_5_RESIST,
  CELL_5_OPEN_VOLT,
  CELL_6_VOLT,
  CELL_6_RESIST,
  CELL_6_OPEN_VOLT,
  CELL_7_VOLT,
  CELL_7_RESIST,
  CELL_7_OPEN_VOLT,
  CELL_8_VOLT,
  CELL_8_RESIST,
  CELL_8_OPEN_VOLT,
  CELL_9_VOLT,
  CELL_9_RESIST,
  CELL_9_OPEN_VOLT,
  CELL_10_VOLT,
  CELL_10_RESIST,
  CELL_10_OPEN_VOLT,
  CELL_11_VOLT,
  CELL_11_RESIST,
  CELL_11_OPEN_VOLT,
  CELL_12_VOLT,
  CELL_12_RESIST,
  CELL_12_OPEN_VOLT,
  CELL_13_VOLT,
  CELL_13_RESIST,
  CELL_13_OPEN_VOLT,
  CELL_14_VOLT,
  CELL_14_RESIST,
  CELL_14_OPEN_VOLT,
  CELL_15_VOLT,
  CELL_15_RESIST,
  CELL_15_OPEN_VOLT,
  CELL_16_VOLT,
  CELL_16_RESIST,
  CELL_16_OPEN_VOLT,
  CELL_17_VOLT,
  CELL_17_RESIST,
  CELL_17_OPEN_VOLT,
  CELL_18_VOLT,
  CELL_18_RESIST,
  CELL_18_OPEN_VOLT,
  CELL_19_VOLT,
  CELL_19_RESIST,
  CELL_19_OPEN_VOLT,
  CELL_20_VOLT,
  CELL_20_RESIST,
  CELL_20_OPEN_VOLT,
  CELL_21_VOLT,
  CELL_21_RESIST,
  CELL_21_OPEN_VOLT,
  CELL_22_VOLT,
  CELL_22_RESIST,
  CELL_22_OPEN_VOLT,
  CELL_23_VOLT,
  CELL_23_RESIST,
  CELL_23_OPEN_VOLT,
  CELL_24_VOLT,
  CELL_24_RESIST,
  CELL_24_OPEN_VOLT,
  CELL_25_VOLT,
  CELL_25_RESIST,
  CELL_25_OPEN_VOLT,
  CELL_26_VOLT,
  CELL_26_RESIST,
  CELL_26_OPEN_VOLT,
  CELL_27_VOLT,
  CELL_27_RESIST,
  CELL_27_OPEN_VOLT,
  CELL_28_VOLT,
  CELL_28_RESIST,
  CELL_28_OPEN_VOLT,
  CELL_29_VOLT,
  CELL_29_RESIST,
  CELL_29_OPEN_VOLT,
  CELL_30_VOLT,
  CELL_30_RESIST,
  CELL_30_OPEN_VOLT,
  CELL_31_VOLT,
  CELL_31_RESIST,
  CELL_31_OPEN_VOLT,
  CELL_32_VOLT,
  CELL_32_RESIST,
  CELL_32_OPEN_VOLT,
  CELL_33_VOLT,
  CELL_33_RESIST,
  CELL_33_OPEN_VOLT,  
  CELL_34_VOLT,
  CELL_34_RESIST,
  CELL_34_OPEN_VOLT,
  /* LuxControl */
  CCL_1_CURRENT1,
  CCL_1_CURRENT2,
  CCL_1_CURRENT3,
  CCL_1_TEMP1,
  CCL_1_TEMP2,
  CCL_1_TEMP3,
  CCL_2_CURRENT1,
  CCL_2_CURRENT2,
  CCL_2_CURRENT3,
  CCL_2_TEMP1,
  CCL_2_TEMP2,
  CCL_2_TEMP3,
  CCL_3_CURRENT1,
  CCL_3_CURRENT2,
  CCL_3_CURRENT3,
  CCL_3_TEMP1,
  CCL_3_TEMP2,
  CCL_3_TEMP3,
  CCL_4_CURRENT1,
  CCL_4_CURRENT2,
  CCL_4_CURRENT3,
  CCL_4_TEMP1,
  CCL_4_TEMP2,
  CCL_4_TEMP3,
  CCL_5_CURRENT1,
  CCL_5_CURRENT2,
  CCL_5_CURRENT3,
  CCL_5_TEMP1,
  CCL_5_TEMP2,
  CCL_5_TEMP3,
  CCL_6_CURRENT1,
  CCL_6_CURRENT2,
  CCL_6_CURRENT3,
  CCL_6_TEMP1,
  CCL_6_TEMP2,
  CCL_6_TEMP3,
  TP_1_ID,
  TP_1_PRESSURE,
  TP_1_TEMP,
  TP_1_FLAGS,
  TP_1_TPTD,
  TP_2_ID,
  TP_2_PRESSURE,
  TP_2_TEMP,
  TP_2_FLAGS,
  TP_2_TPTD,
  TP_3_ID,
  TP_3_PRESSURE,
  TP_3_TEMP,
  TP_3_FLAGS,
  TP_3_TPTD,
  TP_4_ID,
  TP_4_PRESSURE,
  TP_4_TEMP,
  TP_4_FLAGS,
  TP_4_TPTD,
  TP_5_ID,
  TP_5_PRESSURE,
  TP_5_TEMP,
  TP_5_FLAGS,
  TP_5_TPTD,
  TP_6_ID,
  TP_6_PRESSURE,
  TP_6_TEMP,
  TP_6_FLAGS,
  TP_6_TPTD,

  END,
  LOG_ITEMS_NUM
};

static struct logItem 
{
  enum logItemNames item;
  char *name;
  char *convert;
  uint32_t value;
};

static struct logItem logitems[] = {
  { NUM,                    "NUM;",                     "1;",       0},
  { TIME,                   "TIME;",                    "1;",       0},
  /* Analog meas */
  { UBAT,                   "UBAT;",                    "100;",     0},
  { TEMP1,                  "TEMP1;",                   "1;",       0},
  { TEMP2,                  "TEMP2;",                   "1;",       0},
  { STEERING_SEN,           "STEERING_SEN;",            "1;",       0},
  { CURR1,                  "MOTOR_CURR;",              "1;",       0},
  { THROTTLE,               "THROTTLE;",                "1;",       0},
  { REGEN_BRAKE,            "REGEN_BRAKE;",             "1;",       0},
  { CHP_E_B,                "CHP_E_B;",                 "1;",       0},
  { CHP_E_J,                "CHP_E_J;",                 "1;",       0},
  { CHP_H_B,                "CHP_H_B;",                 "1;",       0},
  { CHP_H_J,                "CHP_H_J;",                 "1;",       0},
  /* Cruise control */
  { CRUISE_SWITCH,          "CRUISE_SWITCH;",           "1;",       0},
  { CRUISE_SET,             "CRUISE_SET;",              "10;",      0},
  { CRUISE_SUBSTRACT,       "CRUISE_SUBSTRACT;",        "1;",       0},
  { SPEED_KMPH,             "SPEED_KMPH;",              "1;",       0},
  { SPEED_RPM,              "SPEED_RPM;",               "1;",       0},
  { AVG_SPEED,              "AVG_SPEED;",               "10;",      0},
  /* GPS */
  { GPS_LON,                "GPS_LON;",                 "1;",       0},
  { GPS_LAT,                "GPS_LAT;",                 "1;",       0},
  { GPS_ALT,                "GPS_ALT;",                 "1;",       0},
  { GPS_VEL,                "GPS_VEL;",                 "1;",       0},
  { GPS_SVS,                "GPS_SVS;",                 "1;",       0},
  { GPS_TIME,               "GPS_UNIX_TIME;",           "1;",       0},
  /* Modulux */
  { SOLAR_TEMP_MODULE_1,    "SOLAR_TEMP_MODULE_1;",     "10;",      0},
  { SOLAR_TEMP_MODULE_8,    "SOLAR_TEMP_MODULE_8;",     "10;",      0},
  { SOLAR_TEMP_MODULE_6,    "SOLAR_TEMP_MODULE_6;",     "10;",      0},
  { SOLAR_TEMP_MODULE_12,   "SOLAR_TEMP_MODULE_12;",    "10;",      0},
  { SOLAR_TEMP_MODULE_2,    "SOLAR_TEMP_MODULE_2;",     "10;",      0},
  { SOLARCELL_CURRENT,      "SOLARCELL_CURRENT;",       "1000;",    0},
  /* BMS */
  { BMS_CUSTOM_FLAG_1,      "BMS_CUSTOM_FLAG_1;",       "1;",       0},
  { BMS_PACK_INST_VOLT,     "BMS_PACK_INST_VOLT;",      "10;",      0},
  { BMS_PACK_SOC,           "BMS_PACK_SOC;",            "10;",      0},
  { BMS_PACK_HEALT,         "BMS_PACK_HEALT;",          "1;",       0},
  { BMS_PACK_AMPS,          "BMS_PACK_AMPS;",           "10;",      0},
  { BMS_PACK_RESIST,        "BMS_PACK_RESIST;",         "10;",      0},
  { BMS_AVERAGE_TEMP,       "BMS_AVERAGE_TEMP;",        "1;",       0},
  { BMS_INTERNAL_TEMP,      "BMS_INTERNAL_TEMP;",       "1;",       0},
  { BMS_LOW_CELL_VOLT,      "BMS_LOW_CELL_VOLT;",       "10000;",   0},
  { BMS_HIGH_CELL_VOLT,     "BMS_HIGH_CELL_VOLT;",      "10000;",   0},
  { BMS_AVG_CELL_VOLT,      "BMS_AVG_CELL_VOLT;",       "10000;",   0},
  { BMS_PACK_CURRENT,       "BMS_PACK_CURRENT;",        "1;",       0},
  { BMS_PACK_CYCLE,         "BMS_PACK_CYCLE;",          "1;",       0},
  { BMS_PACK_CCL,           "BMS_PACK_CCL;",            "1;",       0},
  { BMS_PACK_DCL,           "BMS_PACK_DCL;",            "1;",       0},
  { BMS_MAXIMUM_CELL_VOLT,  "BMS_MAXIMUM_CELL_VOLT;",   "10000;",   0},
  { BMS_CUSTOM_FLAG_2,      "BMS_CUSTOM_FLAG_2;",       "1;",       0},
  { BMS_MINIMUM_CELL_VOLT,  "BMS_MINIMUM_CELL_VOLT;",   "10000;",   0},
  { BMS_HIGH_CELL_VOLT_NUM, "BMS_HIGH_CELL_VOLT_NUM;",  "1;",       0},
  { BMS_LOW_CELL_VOLT_NUM,  "BMS_LOW_CELL_VOLT_NUM;",   "1;",       0},
  { BMS_SUPPLY_12V,         "BMS_SUPPLY_12V;",          "10;",      0},
  { BMS_FAN_SPEED,          "BMS_FAN_SPEED;",           "1;",       0},
  { BMS_PACK_OPEN_VOLT,     "BMS_PACK_OPEN_VOLT;",      "10;",      0},
  /* Battery cell */
  { CELL_1_VOLT,            "CELL_1_VOLT;",             "10000;",   0},
  { CELL_1_RESIST,          "CELL_1_RESIST;",           "1;",       0},
  { CELL_1_OPEN_VOLT,       "CELL_1_OPEN_VOLT;",        "10000;",   0},
  { CELL_2_VOLT,            "CELL_2_VOLT;",             "10000;",   0},
  { CELL_2_RESIST,          "CELL_2_RESIST;",           "1;",       0},
  { CELL_2_OPEN_VOLT,       "CELL_2_OPEN_VOLT;",        "10000;",   0},
  { CELL_3_VOLT,            "CELL_3_VOLT;",             "10000;",   0},
  { CELL_3_RESIST,          "CELL_3_RESIST;",           "1;",       0},
  { CELL_3_OPEN_VOLT,       "CELL_3_OPEN_VOLT;",        "10000;",   0},
  { CELL_4_VOLT,            "CELL_4_VOLT;",             "10000;",   0},
  { CELL_4_RESIST,          "CELL_4_RESIST;",           "1;",       0},
  { CELL_4_OPEN_VOLT,       "CELL_4_OPEN_VOLT;",        "10000;",   0},
  { CELL_5_VOLT,            "CELL_5_VOLT;",             "10000;",   0},
  { CELL_5_RESIST,          "CELL_5_RESIST;",           "1;",       0},
  { CELL_5_OPEN_VOLT,       "CELL_5_OPEN_VOLT;",        "10000;",   0},
  { CELL_6_VOLT,            "CELL_6_VOLT;",             "10000;",   0},
  { CELL_6_RESIST,          "CELL_6_RESIST;",           "1;",       0},
  { CELL_6_OPEN_VOLT,       "CELL_6_OPEN_VOLT;",        "10000;",   0},
  { CELL_7_VOLT,            "CELL_7_VOLT;",             "10000;",   0},
  { CELL_7_RESIST,          "CELL_7_RESIST;",           "1;",       0},
  { CELL_7_OPEN_VOLT,       "CELL_7_OPEN_VOLT;",        "10000;",   0},
  { CELL_8_VOLT,            "CELL_8_VOLT;",             "10000;",   0},
  { CELL_8_RESIST,          "CELL_8_RESIST;",           "1;",       0},
  { CELL_8_OPEN_VOLT,       "CELL_8_OPEN_VOLT;",        "10000;",   0},
  { CELL_9_VOLT,            "CELL_9_VOLT;",             "10000;",   0},
  { CELL_9_RESIST,          "CELL_9_RESIST;",           "1;",       0},
  { CELL_9_OPEN_VOLT,       "CELL_9_OPEN_VOLT;",        "10000;",   0},
  { CELL_10_VOLT,           "CELL_10_VOLT;",            "10000;",   0},
  { CELL_10_RESIST,         "CELL_10_RESIST;",          "1;",       0},
  { CELL_10_OPEN_VOLT,      "CELL_10_OPEN_VOLT;",       "10000;",   0},
  { CELL_11_VOLT,           "CELL_11_VOLT;",            "10000;",   0},
  { CELL_11_RESIST,         "CELL_11_RESIST;",          "1;",       0},
  { CELL_11_OPEN_VOLT,      "CELL_11_OPEN_VOLT;",       "10000;",   0},
  { CELL_12_VOLT,           "CELL_12_VOLT;",            "10000;",   0},
  { CELL_12_RESIST,         "CELL_12_RESIST;",          "1;",       0},
  { CELL_12_OPEN_VOLT,      "CELL_12_OPEN_VOLT;",       "10000;",   0},
  { CELL_13_VOLT,           "CELL_13_VOLT;",            "10000;",   0},
  { CELL_13_RESIST,         "CELL_13_RESIST;",          "1;",       0},
  { CELL_13_OPEN_VOLT,      "CELL_13_OPEN_VOLT;",       "10000;",   0},
  { CELL_14_VOLT,           "CELL_14_VOLT;",            "10000;",   0},
  { CELL_14_RESIST,         "CELL_14_RESIST;",          "1;",       0},
  { CELL_14_OPEN_VOLT,      "CELL_14_OPEN_VOLT;",       "10000;",   0},
  { CELL_15_VOLT,           "CELL_15_VOLT;",            "10000;",   0},
  { CELL_15_RESIST,         "CELL_15_RESIST;",          "1;",       0},
  { CELL_15_OPEN_VOLT,      "CELL_15_OPEN_VOLT;",       "10000;",   0},
  { CELL_16_VOLT,           "CELL_16_VOLT;",            "10000;",   0},
  { CELL_16_RESIST,         "CELL_16_RESIST;",          "1;",       0},
  { CELL_16_OPEN_VOLT,      "CELL_16_OPEN_VOLT;",       "10000;",   0},
  { CELL_17_VOLT,           "CELL_17_VOLT;",            "10000;",   0},
  { CELL_17_RESIST,         "CELL_17_RESIST;",          "1;",       0},
  { CELL_17_OPEN_VOLT,      "CELL_17_OPEN_VOLT;",       "10000;",   0},
  { CELL_18_VOLT,           "CELL_18_VOLT;",            "10000;",   0},
  { CELL_18_RESIST,         "CELL_18_RESIST;",          "1;",       0},
  { CELL_18_OPEN_VOLT,      "CELL_18_OPEN_VOLT;",       "10000;",   0},
  { CELL_19_VOLT,           "CELL_19_VOLT;",            "10000;",   0},
  { CELL_19_RESIST,         "CELL_19_RESIST;",          "1;",       0},
  { CELL_19_OPEN_VOLT,      "CELL_19_OPEN_VOLT;",       "10000;",   0},
  { CELL_20_VOLT,           "CELL_20_VOLT;",            "10000;",   0},
  { CELL_20_RESIST,         "CELL_20_RESIST;",          "1;",       0},
  { CELL_20_OPEN_VOLT,      "CELL_20_OPEN_VOLT;",       "10000;",   0},
  { CELL_21_VOLT,           "CELL_21_VOLT;",            "10000;",   0},
  { CELL_21_RESIST,         "CELL_21_RESIST;",          "1;",       0},
  { CELL_21_OPEN_VOLT,      "CELL_21_OPEN_VOLT;",       "10000;",   0},
  { CELL_22_VOLT,           "CELL_22_VOLT;",            "10000;",   0},
  { CELL_22_RESIST,         "CELL_22_RESIST;",          "1;",       0},
  { CELL_22_OPEN_VOLT,      "CELL_22_OPEN_VOLT;",       "10000;",   0},
  { CELL_23_VOLT,           "CELL_23_VOLT;",            "10000;",   0},
  { CELL_23_RESIST,         "CELL_23_RESIST;",          "1;",       0},
  { CELL_23_OPEN_VOLT,      "CELL_23_OPEN_VOLT;",       "10000;",   0},
  { CELL_24_VOLT,           "CELL_24_VOLT;",            "10000;",   0},
  { CELL_24_RESIST,         "CELL_24_RESIST;",          "1;",       0},
  { CELL_24_OPEN_VOLT,      "CELL_24_OPEN_VOLT;",       "10000;",   0},
  { CELL_25_VOLT,           "CELL_25_VOLT;",            "10000;",   0},
  { CELL_25_RESIST,         "CELL_25_RESIST;",          "1;",       0},
  { CELL_25_OPEN_VOLT,      "CELL_25_OPEN_VOLT;",       "10000;",   0},
  { CELL_26_VOLT,           "CELL_26_VOLT;",            "10000;",   0},
  { CELL_26_RESIST,         "CELL_26_RESIST;",          "1;",       0},
  { CELL_26_OPEN_VOLT,      "CELL_26_OPEN_VOLT;",       "10000;",   0},
  { CELL_27_VOLT,           "CELL_27_VOLT;",            "10000;",   0},
  { CELL_27_RESIST,         "CELL_27_RESIST;",          "1;",       0},
  { CELL_27_OPEN_VOLT,      "CELL_27_OPEN_VOLT;",       "10000;",   0},
  { CELL_28_VOLT,           "CELL_28_VOLT;",            "10000;",   0},
  { CELL_28_RESIST,         "CELL_28_RESIST;",          "1;",       0},
  { CELL_28_OPEN_VOLT,      "CELL_28_OPEN_VOLT;",       "10000;",   0},
  { CELL_29_VOLT,           "CELL_29_VOLT;",            "10000;",   0},
  { CELL_29_RESIST,         "CELL_29_RESIST;",          "1;",       0},
  { CELL_29_OPEN_VOLT,      "CELL_29_OPEN_VOLT;",       "10000;",   0},
  { CELL_30_VOLT,           "CELL_30_VOLT;",            "10000;",   0},
  { CELL_30_RESIST,         "CELL_30_RESIST;",          "1;",       0},
  { CELL_30_OPEN_VOLT,      "CELL_30_OPEN_VOLT;",       "10000;",   0},
  { CELL_31_VOLT,           "CELL_31_VOLT;",            "10000;",   0},
  { CELL_31_RESIST,         "CELL_31_RESIST;",          "1;",       0},
  { CELL_31_OPEN_VOLT,      "CELL_31_OPEN_VOLT;",       "10000;",   0},
  { CELL_32_VOLT,           "CELL_32_VOLT;",            "10000;",   0},
  { CELL_32_RESIST,         "CELL_32_RESIST;",          "1;",       0},
  { CELL_32_OPEN_VOLT,      "CELL_32_OPEN_VOLT;",       "10000;",   0},
  { CELL_33_VOLT,           "CELL_33_VOLT;",            "10000;",   0},
  { CELL_33_RESIST,         "CELL_33_RESIST;",          "1;",       0},
  { CELL_33_OPEN_VOLT,      "CELL_33_OPEN_VOLT;",       "10000;",   0},
  { CELL_34_VOLT,           "CELL_34_VOLT;",            "10000;",   0},
  { CELL_34_RESIST,         "CELL_34_RESIST;",          "1;",       0},
  { CELL_34_OPEN_VOLT,      "CELL_34_OPEN_VOLT;",       "10000;",   0},
  /* CLL Items */
  { CCL_1_CURRENT1,         "CCL_1_CURRENT1;",          "1;",       0},
  { CCL_1_CURRENT2,         "CCL_1_CURRENT2;",          "1;",       0},
  { CCL_1_CURRENT3,         "CCL_1_CURRENT3;",          "1;",       0},
  { CCL_1_TEMP1,            "CCL_1_TEMP1;",             "1;",       0},
  { CCL_1_TEMP2,            "CCL_1_TEMP2;",             "1;",       0},
  { CCL_1_TEMP3,            "CCL_1_TEMP3;",             "1;",       0},
  { CCL_2_CURRENT1,         "CCL_2_CURRENT1;",          "1;",       0},
  { CCL_2_CURRENT2,         "CCL_2_CURRENT2;",          "1;",       0},
  { CCL_2_CURRENT3,         "CCL_2_CURRENT3;",          "1;",       0},
  { CCL_2_TEMP1,            "CCL_2_TEMP1;",             "1;",       0},
  { CCL_2_TEMP2,            "CCL_2_TEMP2;",             "1;",       0},
  { CCL_2_TEMP3,            "CCL_2_TEMP3;",             "1;",       0},
  { CCL_3_CURRENT1,         "CCL_3_CURRENT1;",          "1;",       0},
  { CCL_3_CURRENT2,         "CCL_3_CURRENT2;",          "1;",       0},
  { CCL_3_CURRENT3,         "CCL_3_CURRENT3;",          "1;",       0},
  { CCL_3_TEMP1,            "CCL_3_TEMP1;",             "1;",       0},
  { CCL_3_TEMP2,            "CCL_3_TEMP2;",             "1;",       0},
  { CCL_3_TEMP3,            "CCL_3_TEMP3;",             "1;",       0},
  { CCL_4_CURRENT1,         "CCL_4_CURRENT1;",          "1;",       0},
  { CCL_4_CURRENT2,         "CCL_4_CURRENT2;",          "1;",       0},
  { CCL_4_CURRENT3,         "CCL_4_CURRENT3;",          "1;",       0},
  { CCL_4_TEMP1,            "CCL_4_TEMP1;",             "1;",       0},
  { CCL_4_TEMP2,            "CCL_4_TEMP2;",             "1;",       0},
  { CCL_4_TEMP3,            "CCL_4_TEMP3;",             "1;",       0},
  { CCL_5_CURRENT1,         "CCL_5_CURRENT1;",          "1;",       0},
  { CCL_5_CURRENT2,         "CCL_5_CURRENT2;",          "1;",       0},
  { CCL_5_CURRENT3,         "CCL_5_CURRENT3;",          "1;",       0},
  { CCL_5_TEMP1,            "CCL_5_TEMP1;",             "1;",       0},
  { CCL_5_TEMP2,            "CCL_5_TEMP2;",             "1;",       0},
  { CCL_5_TEMP3,            "CCL_5_TEMP3;",             "1;",       0},
  { CCL_6_CURRENT1,         "CCL_6_CURRENT1;",          "1;",       0},
  { CCL_6_CURRENT2,         "CCL_6_CURRENT2;",          "1;",       0},
  { CCL_6_CURRENT3,         "CCL_6_CURRENT3;",          "1;",       0},
  { CCL_6_TEMP1,            "CCL_6_TEMP1;",             "1;",       0},
  { CCL_6_TEMP2,            "CCL_6_TEMP2;",             "1;",       0},
  { CCL_6_TEMP3,            "CCL_6_TEMP3;",             "1;",       0},
  /* Tire Pressure */
  { TP_1_ID,                "TP_1_ID;",                 "1;",       0},
  { TP_1_PRESSURE,          "TP_1_PRESSURE;",           "1;",       0},
  { TP_1_TEMP,              "TP_1_TEMP;",               "1;",       0},
  { TP_1_FLAGS,             "TP_1_FLAGS;",              "1;",       0},
  { TP_1_TPTD,              "TP_1_TPTD;",               "1;",       0},
  { TP_2_ID,                "TP_2_ID;",                 "1;",       0},
  { TP_2_PRESSURE,          "TP_2_PRESSURE;",           "1;",       0},
  { TP_2_TEMP,              "TP_2_TEMP;",               "1;",       0},
  { TP_2_FLAGS,             "TP_2_FLAGS;",              "1;",       0},
  { TP_2_TPTD,              "TP_2_TPTD;",               "1;",       0},
  { TP_3_ID,                "TP_3_ID;",                 "1;",       0},
  { TP_3_PRESSURE,          "TP_3_PRESSURE;",           "1;",       0},
  { TP_3_TEMP,              "TP_3_TEMP;",               "1;",       0},
  { TP_3_FLAGS,             "TP_3_FLAGS;",              "1;",       0},
  { TP_3_TPTD,              "TP_3_TPTD;",               "1;",       0},
  { TP_4_ID,                "TP_4_ID;",                 "1;",       0},
  { TP_4_PRESSURE,          "TP_4_PRESSURE;",           "1;",       0},
  { TP_4_TEMP,              "TP_4_TEMP;",               "1;",       0},
  { TP_4_FLAGS,             "TP_4_FLAGS;",              "1;",       0},
  { TP_4_TPTD,              "TP_4_TPTD;",               "1;",       0},
  { TP_5_ID,                "TP_5_ID;",                 "1;",       0},
  { TP_5_PRESSURE,          "TP_5_PRESSURE;",           "1;",       0},
  { TP_5_TEMP,              "TP_5_TEMP;",               "1;",       0},
  { TP_5_FLAGS,             "TP_5_FLAGS;",              "1;",       0},
  { TP_5_TPTD,              "TP_5_TPTD;",               "1;",       0},
  { TP_6_ID,                "TP_6_ID;",                 "1;",       0},
  { TP_6_PRESSURE,          "TP_6_PRESSURE;",           "1;",       0},
  { TP_6_TEMP,              "TP_6_TEMP;",               "1;",       0},
  { TP_6_FLAGS,             "TP_6_FLAGS;",              "1;",       0},
  { TP_6_TPTD,              "TP_6_TPTD;",               "1;",       0},

  { END,                    "END;",                     "END;",     1}
};
