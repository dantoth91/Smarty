/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

enum logItemNames
{
  NUM,
  TIME,
  UBAT,
  BRAKE_P1,
  BRAKE_P2,
  FUT_SEN1,
  FUT_SEN2,
  FUT_SEN3,
  FUT_SEN4,
  CURR1,
  THROTTLE,
  REGEN_BRAKE,
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
  { NUM,        "NUM;",           "1;",       0},
  { TIME,       "TIME;",          "1;",       0},
  { UBAT,       "UBAT;",          "100;",     0},
  { BRAKE_P1,   "BRAKE_P1;",      "1;",       0},
  { BRAKE_P2,   "BRAKE_P2;",      "1;",       0},
  { FUT_SEN1,   "FUT_SEN1;",      "1;",       0},
  { FUT_SEN2,   "FUT_SEN2;",      "1;",       0},
  { FUT_SEN3,   "FUT_SEN3;",      "1;",       0},
  { FUT_SEN4,   "FUT_SEN4;",      "1;",       0},
  { CURR1,      "MOTOR_CURR;",    "1;",       0},
  { THROTTLE,   "THROTTLE;",      "1;",       0},
  { REGEN_BRAKE,"REGEN_BRAKE;",   "1;",       0}
};