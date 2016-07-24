/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "speed.h"
#include "chprintf.h"
#include "eepromitems.h"

/* 10kHz ICU clock frequency.  (1MHz - 1000000L) */
#define SPEED_ICU_CLOCK (1000000L)

#define SEN_POINTS          48
#define WHEEL               1733     /* Kerék kerület mm-ben */
#define MSTOS               60000000
#define MAX_RPM_STEP        150
#define SPEED_ZERO_PERIOD   50

static double rpm_buff[SEN_POINTS];

static uint16_t rpm_index;

/*
 * Variables for the distance measuring
 */
static uint8_t tick_counter;
static double meter_counter;
static double total_meter;
static uint32_t kmeter_counter;
static uint32_t total_kmeter_counter;


static void speedWheelPeriodCb(ICUDriver *icup);
static void speedWheelPeriodNumber(ICUDriver *icup);

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  SPEED_ICU_CLOCK,
  speedWheelPeriodNumber,
  speedWheelPeriodCb,
  NULL,
  ICU_CHANNEL_1,
  0
};

static uint32_t speed_last_period;
static uint32_t speed_period_num;
static uint32_t speed_zero_period;

static double rotation;
static double old_rotation;
static double speed;
static double rpmasis;


static void speedWheelPeriodCb(ICUDriver *icup) {
  int i;
  
  
  speed_last_period = icuGetPeriod(icup); //0.1 * ms

  rotation = MSTOS / (speed_last_period * SEN_POINTS);

  rpm_index++;

  if(rpm_index > (SEN_POINTS - 1)){
    rpm_index = 0;
  }

  rpm_buff[rpm_index] = rotation;
  rpmasis = 0;
  
  for (i = 0; i < SEN_POINTS; i++){
    rpmasis += rpm_buff[i];
  }

  rotation = rpmasis / SEN_POINTS;

  if(rotation < 0){
    rotation = 0;
  }

  if(rotation > 3000){
    rotation = 3000;
  }

  if ((rotation - old_rotation) > MAX_RPM_STEP)
  {
    rotation = old_rotation;
  }

  old_rotation = rotation;

  speed = speedRPM_TO_KMPH(rotation);
}

static void speedWheelPeriodNumber(ICUDriver *icup) {
    (void)icup;
    speed_period_num ++;
    speed_zero_period = 0;
    DistanceMeas();
}

void speedInit(void){

  /*
   * Activates and starts the icu driver 1
   */
  icuStart(&ICUD1, &icucfg);
  icuEnable(&ICUD1);

  speed_period_num = 0;
  old_rotation = 0;

  kmeter_counter = 0;
  total_kmeter_counter= 0;

  eepromRead(KMETER_COUNTER, &kmeter_counter);
  eepromRead(TOTAL_KMETER_COUNTER, &total_kmeter_counter);
}

void speedCalc(void){
  speed_zero_period ++;
  if(speed_zero_period > SPEED_ZERO_PERIOD){
    rotation = 0;
    speed = 0;
  }
}

uint32_t speedGetLastPeriod(void){
  uint32_t tmp;

  chSysLock();
  tmp = speed_last_period;
  chSysUnlock();
  return tmp;
}

uint32_t speedGetPeriodNumber(void){
  uint32_t tmp2;

  chSysLock();
  tmp2 = speed_period_num;
  chSysUnlock();
  return tmp2;
}

uint32_t speedGetRpm(void){
  uint32_t tmp;

  chSysLock();
  tmp = (uint32_t)rotation;
  chSysUnlock();
  return tmp;
}

uint32_t speedGetSpeed(void){
  uint32_t tmp;

  chSysLock();
  tmp = (uint32_t)speed;
  chSysUnlock();
  return tmp;
}

uint32_t speedRPM_TO_KMPH(double rpm){
  uint32_t tmp;

  chSysLock();
  tmp = rpm * WHEEL;
  tmp *= 60;
  tmp /= 100000;

  if((tmp % 10) > 5){
    tmp /= 10;
    tmp += 1;
  }
  else
    tmp /= 10;
  chSysUnlock();
  
  return tmp;
}

uint32_t speedKMPH_TO_RPM(double kmph){
  uint32_t tmp;

  chSysLock();
  tmp = kmph * 1000000;
  tmp /= 60;
  tmp /= (WHEEL / 10);

  if((tmp % 10) > 5){
    tmp /= 10;
    tmp += 1;
  }
  else
    tmp /= 10;
  chSysUnlock();

  return tmp;
}

/*
 * Measure the Distance in KM
 * Two counters: - Total Kmeter
 *               - Nullable Kmeter
 */
void DistanceMeas(void)
{
  double wheel_meter = WHEEL;
  
  tick_counter++;
  if((tick_counter % SEN_POINTS) == 0)
  {
    meter_counter += (double)(wheel_meter / 1000);
    total_meter += (double)(wheel_meter / 1000);
    tick_counter = 0;
  }

  if (meter_counter >= 1000)
  {
    kmeter_counter++;
    total_kmeter_counter++;
    meter_counter -= 1000;
    //Until one plus KM save the values to the eeprom
    chThdCreateStatic(wasave_kmeter_eeprom, sizeof(wasave_kmeter_eeprom), NORMALPRIO, save_kmeter_eeprom, NULL);
  }
}

/*
 * This task save the "kmeter_counter" and "total_kmeter_counter" to the eeprom
 */
static WORKING_AREA(wasave_kmeter_eeprom, 512);
static msg_t save_kmeter_eeprom(void *arg) {
  chSysLock();
  eepromWrite(TOTAL_KMETER_COUNTER, total_kmeter_counter);
  chThdSleepMilliseconds(10);
  eepromWrite(KMETER_COUNTER, kmeter_counter);
  chSysUnlock();
  return 0;
}

uint32_t GetTotalKmeterDistance()
{
  return total_kmeter_counter;
}
uint32_t GetKmeterDistance()
{
  return kmeter_counter;
}

double GetTotalMeter()
{
  double tmp;

  chSysLock();
  tmp = total_meter;
  chSysUnlock();

  return total_meter;
}

uint16_t GetMeterCounter(){
  return (uint16_t)meter_counter;
}

void TotalMeterZero()
{
  return total_meter = 0;
}

void cmd_speedvalues(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "speed_last_period: %5d\r\n", speed_last_period); //2500000L / speed_last_period
    chprintf(chp, "speed_period_num: %5d\r\n", speed_period_num);
    chprintf(chp, "rpm: %5d\r\n", rotation);
    chprintf(chp, "speed: %5d\r\n", speed);
    chprintf(chp, "rpmasis: %5d\r\n", rpmasis);
    chThdSleepMilliseconds(1000);
  }
}

void cmd_distancevalues(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp, "tick_counter: %5d\r\n", tick_counter);
    chprintf(chp, "meter_count: %5d\r\n", (uint16_t)meter_counter);
    chprintf(chp, "total_meter: %5d\r\n", (uint16_t)total_meter);
    chprintf(chp, "kmeter_count: %5d\r\n", kmeter_counter);
    chprintf(chp, "total_kmeter_count: %5d\r\n", total_kmeter_counter);

    chThdSleepMilliseconds(200);
  }
}

void cmd_set_total_kmeter(BaseSequentialStream *chp, int argc, char *argv[])
{
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  total_kmeter_counter = atol(argv[0]);

  if(eepromWrite(TOTAL_KMETER_COUNTER, total_kmeter_counter) != 0)
  {
    chprintf(chp, "EEPROM write error!\r\n");
  }else
  {
    chprintf(chp, "Total KMeter has stored!\r\n");
  }

  chprintf(chp, "Total KMeter: %d\r\n", total_kmeter_counter);

  chThdSleepMilliseconds(100);
}

void cmd_reset_kmeter(BaseSequentialStream *chp, int argc, char *argv[])
{
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  kmeter_counter = 0;
  if(eepromWrite(KMETER_COUNTER, kmeter_counter) != 0)
  {
    chprintf(chp, "EEPROM write error!\r\n");
  }else
  {
    chprintf(chp, "KMeter has cleared!\r\n");
  }
  chprintf(chp, "KMeter: %d\r\n", kmeter_counter);

  chThdSleepMilliseconds(100);
}

void cmd_reset_meter(BaseSequentialStream *chp, int argc, char *argv[])
{
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  TotalMeterZero();
  if(eepromWrite(TOTAL_METER, total_meter) != 0)
  {
    chprintf(chp, "EEPROM write error!\r\n");
  }else
  {
    chprintf(chp, "Total meter has cleared!\r\n");
  }
  chprintf(chp, "Total Meter: %d\r\n", (uint16_t)total_meter);
}
