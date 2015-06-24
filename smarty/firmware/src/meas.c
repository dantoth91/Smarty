/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "meas.h"
#include "eeprom.h"

#include "chprintf.h"

#define ADC_GRP1_BUF_DEPTH      8

#define MIN_PWM                 0
#define MAX_PWM                 9000

enum measStates
{
  MEAS_START,
  MEAS_RUNING
} measstate;

static int32_t measValue[MEAS_NUM_CH +2];
static adcsample_t samples[MEAS_NUM_CH * ADC_GRP1_BUF_DEPTH];

static int32_t measValue_2[MEAS2_NUM_CH +2];
static adcsample_t samples_2[MEAS2_NUM_CH * ADC_GRP1_BUF_DEPTH];

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err);
static void adcerrorcallback_2(ADCDriver *adcp, adcerror_t err);

static uint16_t max_throttle;
static uint16_t min_throttle;
static uint16_t throttle;

static bool_t set_min;
static bool_t set_max;

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 8 channel, SW triggered.
 * Channels:    IN14, IN10, IN11, IN12, IN13, IN15, IN8.
 * SMPR1 - CH 10...17, 			     |	SMPR2 - CH 0...9  |
 * SQR1 - 13...16 + sequence length, |	SQR2 - 7...12, 	  |	SQR3 - 1...6
 */
static const ADCConversionGroup adcgrpcfg = {
  FALSE,
  MEAS_NUM_CH,
  NULL,
  adcerrorcallback,
  0,                                                                       /* CR1 */
  ADC_CR2_SWSTART,                                                         /* CR2 */
  ADC_SMPR1_SMP_AN14(ADC_SAMPLE_15) | ADC_SMPR1_SMP_AN10(ADC_SAMPLE_15) |  /* SMPR1 | AN14-PC4-UBAT    | AN10-PC0-BRAKE1	 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_15) | ADC_SMPR1_SMP_AN12(ADC_SAMPLE_15) |  /* SMPR1 | AN11-PC1-BRAKE2  | AN12-PC2-SEN2 		 */
  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_15) | ADC_SMPR1_SMP_AN15(ADC_SAMPLE_15),   /* SMPR1 | AN13-PC3-SEN3    | AN15-PC5-SEN4  	 */
  ADC_SMPR2_SMP_AN8(ADC_SAMPLE_15),									                       /* SMPR2 | AN8-PB0-SEN5     |					         */
  ADC_SQR1_NUM_CH(MEAS_NUM_CH),                                            /* SQR1  -----------Number of sensors---------- */
  ADC_SQR2_SQ7_N(ADC_CHANNEL_IN8),    									                   /* SQR2  | 7. IN8-SEN5  	   |					         */
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN15) | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN13) |    /* SQR3  | 6. IN15-SEN4 	   | 5. IN13-SEN3   	 */
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN12) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN11) |    /* SQR3  | 4. IN12-SEN2 	   | 3. IN11-BRAKE2 	 */
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN10) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN14)      /* SQR3  | 2. IN10-BRAKE1   | 1. IN14-UBAT 		 */
};

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 8 channel, SW triggered.
 * Channels:    IN5, IN4.
 * SMPR1 -     CH 10...17,           |  SMPR2 - CH 0...9  |
 * SQR1 - 13...16 + sequence length, |  SQR2 - 7...12,    | SQR3 - 1...6
 */
static const ADCConversionGroup adcgrpcfg_2 = {
  FALSE,
  MEAS2_NUM_CH,
  NULL,
  adcerrorcallback,
  0,                                                                       /* CR1 */
  ADC_CR2_SWSTART,                                                         /* CR2 */
  0,                                                                       /* SMPR1 |                  |                   */
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_15) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_15),     /* SMPR2 | AN4-PF6-SEN1     | AN5-PF7-THROTTLE  */
  ADC_SQR1_NUM_CH(MEAS2_NUM_CH),                                           /* SQR1  -----------Number of sensors---------- */
  0,                                                                       /* SQR2  |                  |                   */
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN5) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN4)        /* SQR3  | 2. IN5-THROTTLE  | 1. IN4-SEN1       */
};

void measInit(void){

  /*
  * Activates the ADC3 driver.
  */
  adcStart(&ADCD3, NULL);
  adcStart(&ADCD2, NULL);
  measstate = MEAS_START;

  if(eepromRead(MAX_THROTTLE, &max_throttle) != 0){
    max_throttle = 2600;
  }
  if(eepromRead(MIN_THROTTLE, &min_throttle) != 0){
    min_throttle = 1111;
  }

  throttle = 0;

  set_min = FALSE;
  set_max = FALSE;
}

void measCalc(void){
  int ch;
  int avg, i; 

  /*
  * Starts the ADC conversion.
  */
  //Prociba menő fesz * 5,545 = tápfesz
  switch(measstate){

    case MEAS_START:
      measstate = MEAS_RUNING;
      break;
    case MEAS_RUNING:
      for(ch = 0; ch < MEAS_NUM_CH; ch++) {
        avg = 0;
        for(i = 0; i < ADC_GRP1_BUF_DEPTH; i ++) {
          avg += samples[ch + MEAS_NUM_CH * i];
        }
        avg /= ADC_GRP1_BUF_DEPTH;
        switch(ch){
          case MEAS_UBAT:
            avg *=  5545;
            avg /= 12412;
            break;
          case MEAS_BRAKE_PRESSURE1:
            break;
          case MEAS_BRAKE_PRESSURE2:
            break;
          case MEAS_SEN2:
            break;
          case MEAS_SEN3:
            break;
          case MEAS_SEN4:
            break;
          case MEAS_SEN5:
            break;
          default:
            break;
        }
        chSysLock();
        measValue[ch] = (int16_t)avg;
        chSysUnlock();
      }
      for(ch = 0; ch < MEAS2_NUM_CH; ch++) {
        avg = 0;
        for(i = 0; i < ADC_GRP1_BUF_DEPTH; i ++) {
          avg += samples_2[ch + MEAS2_NUM_CH * i];
        }
        avg /= ADC_GRP1_BUF_DEPTH;
        switch(ch){
          case MEAS2_CURR1:
            break;
          case MEAS2_THROTTLE:

            if(set_max){ 
              max_throttle = avg;
              if(eepromWrite(MAX_THROTTLE, max_throttle) != 0){
                max_throttle = avg;
              }
              set_max = FALSE;
            }

            if(set_min){ 
              min_throttle = avg;
              if(eepromWrite(MIN_THROTTLE, min_throttle) != 0){
                min_throttle = avg;
              }
              set_min = FALSE;
            }

            avg = avg > max_throttle ? max_throttle : avg;
            avg = avg < min_throttle ? min_throttle: avg;

            throttle = map(avg, min_throttle, max_throttle, MIN_PWM, MAX_PWM);
            avg = throttle;

            avg = avg > 10000 ? 10000 : avg;
            avg = avg < 0 ? 0: avg;
            break;
          default:
            break;
        }
        chSysLock();
        measValue_2[ch] = (int16_t)avg;
        chSysUnlock();
      }
      break;
    default:
      break;
  }
  adcConvert(&ADCD2, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
  adcConvert(&ADCD3, &adcgrpcfg_2, samples_2, ADC_GRP1_BUF_DEPTH);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int16_t measGetValue(enum measChannels ch){
	  return measValue[ch];
}

int16_t measGetValue_2(enum measChannels2 ch){
    return measValue_2[ch];
}

void meas_throttleSetMin(void){
  set_min = TRUE;
}

void meas_throttleSetMax(void){
  set_max = TRUE;
}

/*
 * ADC error callback. TODO: utilize
 */
static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

void cmd_measvalues(BaseSequentialStream *chp, int argc, char *argv[]){
  enum measChannels ch;

  static const char * const names[] = {

      "UBAT",
      "BRAKE_PRESSURE1",
      "BRAKE_PRESSURE2",
      "SEN2",
      "SEN3",
      "SEN4",
      "SEN5"};

  static const char * const names2[] = {

      "CURR1",
      "THROTTLE"};

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      chprintf(chp, "\x1B[%d;%dH", 0, 0);
      for(ch = 0; ch < MEAS_NUM_CH; ch++) {
          chprintf(chp, "%s: %15d\r\n", names[ch], measValue[ch]);
      }
      for(ch = 0; ch < MEAS2_NUM_CH; ch++) {
          chprintf(chp, "%s: %15d\r\n", names2[ch], measValue_2[ch]);
      }
      chThdSleepMilliseconds(1000);
  }
}

void cmd_getThrottle(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

  if ((argc == 1) && (strcmp(argv[0], "set_min") == 0)){
    meas_throttleSetMin();
    chprintf(chp, "Set throttle pedal min. value!\r\n");
  }

  else if ((argc == 1) && (strcmp(argv[0], "set_max") == 0)){
    meas_throttleSetMax();
    chprintf(chp, "Set throttle pedal max. value!\r\n");
  }

  else{
    chprintf(chp, "Usage: throttle set_min\r\n");
    chprintf(chp, "                set_max\r\n");  
  }
  return;
}