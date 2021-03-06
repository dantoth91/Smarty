/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "meas.h"
#include "eeprom.h"
#include "calc.h"

#include "can_items.h"
#include "can_comm.h"

#include "chprintf.h"

#define ADC_GRP1_BUF_DEPTH      16

#define MIN_PWM                 0
#define MAX_PWM                 9000

#define MIN_PERCENT             0
#define MAX_PERCENT             9000

#define MIN_STR                 0
#define MAX_STR                 200

/*
 * Motor Current Measuring defines
 * 196 adc - 8.5A
 *
 * 2904
 * 23 adc - 1 A
 *
 * 2880
 * 47 adc - 2 A
 *
 * 2856
 * 71 adc - 3 A
 *
 * 2833
 * 94 adc - 4 A
 * -191
 * 2927
 */
#define NULL_AMPER_ADC          2880
#define CURRENT_PER_ADC         23.5
//#define CURRENT_PER_ADC         43.36734
#define MOTOR_CURRENT_LENGTH    100

/*
 * NTC Measuring stuffs
 */
#define MEAS_NTCCAL_NUM 43
#define MEAS_NTCCAL_START -55
#define MEAS_NTCCAL_STEP 5
const adcsample_t measNTCcalib[] = {
  4054, 4036, 4011, 3978, 3934, 3877, 3804, 3713, 3602, 3469,
  3313, 3136, 2939, 2726, 2503, 2275, 2048, 1828, 1618, 1424,
  1245, 1085,  942,  816,  706,  611,  528,  458,  397,  344,
   300,  261,  228,  199,  175,  153,  135,  120,  106,   94,
    83,   74,   66
} ;



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

/* Throttle pedal values */
static uint16_t max_throttle;
static uint16_t min_throttle;
static uint16_t throttle;
/* -------------------------- */

/* Brake pedal values */
static uint16_t max_regen_brake;
static uint16_t min_regen_brake;
static uint16_t regen_brake;
/* -------------------------- */

/* Steering angle values */
static uint16_t max_str_angle;
static uint16_t min_str_angle;
static uint16_t str_angle;
/* -------------------------- */


static bool_t set_min_throttle;
static bool_t set_max_throttle;

static bool_t set_min_regen;
static bool_t set_max_regen;

static bool_t set_min_str_angle;
static bool_t set_max_str_angle;

static systime_t ido;
static systime_t valtozo;

static int curr_avg;

/*
 * Motor Current Meas Buffer
 */
static uint16_t Motor_current_index;
static uint16_t Motor_current_avg[MOTOR_CURRENT_LENGTH];
static uint32_t motor_curr;

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 7 channel, SW triggered.
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
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_15) | ADC_SMPR1_SMP_AN12(ADC_SAMPLE_15) |  /* SMPR1 | AN11-PC1-BRAKE2  | AN12-PC2-STEERING */
  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_15) | ADC_SMPR1_SMP_AN15(ADC_SAMPLE_15),   /* SMPR1 | AN13-PC3-SEN1    | AN15-PC5-SEN2  	 */
  ADC_SMPR2_SMP_AN8(ADC_SAMPLE_15),									                       /* SMPR2 | AN8-PB0-SEN3     |					         */
  ADC_SQR1_NUM_CH(MEAS_NUM_CH),                                            /* SQR1  -----------Number of sensors---------- */
  ADC_SQR2_SQ7_N(ADC_CHANNEL_IN8),    									                   /* SQR2  | 7. IN8-SEN3  	   |					         */
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN15) | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN13) |    /* SQR3  | 6. IN15-SEN2 	   | 5. IN13-SEN1   	 */
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN12) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN11) |    /* SQR3  | 4. IN12-STEERING | 3. IN11-BRAKE2 	 */
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN10) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN14)      /* SQR3  | 2. IN10-BRAKE1   | 1. IN14-UBAT 		 */
};

/*
 * ADC conversion group2.
 * Mode:        Linear buffer, 8 samples of 3 channel, SW triggered.
 * Channels:    IN5, IN4, IN6.
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
  0,                                                                       /* SMPR1 |                       |                   */
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_15) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_15) |    /* SMPR2 | AN4-PF6-CURR          | AN5-PF7-THROTTLE  */
  ADC_SMPR2_SMP_AN6(ADC_SAMPLE_15),                                        /* SMPR2 | AN6-PF8-REGEN_BRAKE   |                   */
  ADC_SQR1_NUM_CH(MEAS2_NUM_CH),                                           /* SQR1  -----------Number of sensors----------      */
  0,                                                                       /* SQR2  |                       |                   */
  ADC_SQR3_SQ3_N(ADC_CHANNEL_IN6) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN5) |      /* SQR3  | 3. IN6-REGEN_BREAK    |2. IN5-THROTTLE    */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN4)                                          /* SQR3  | 1. IN4-CURR           |                   */
};

void measInit(void){

  /*
  * Activates the ADC3 driver.
  */
  adcStart(&ADCD3, NULL);
  adcStart(&ADCD2, NULL);
  measstate = MEAS_START;

  if(eepromRead(MAX_REGEN_BRAKE, &max_regen_brake) != 0){
    max_regen_brake = 100;
  }
  if(eepromRead(MIN_REGEN_BRAKE, &min_regen_brake) != 0){
    min_regen_brake = 0;
  }

  if(eepromRead(MAX_THROTTLE, &max_throttle) != 0){
    max_throttle = 2600;
  }
  if(eepromRead(MIN_THROTTLE, &min_throttle) != 0){
    min_throttle = 1111;
  }

  if(eepromRead(MAX_STR_ANGLE, &max_str_angle) != 0){
    max_str_angle = 0;
  }
  if(eepromRead(MIN_STR_ANGLE, &min_str_angle) != 0){
    min_str_angle = 0;
  }

  throttle = 0;
  regen_brake = 0;
  str_angle = 0;

  set_min_throttle = FALSE;
  set_max_throttle = FALSE;

  set_min_regen = FALSE;
  set_max_regen = FALSE;

  set_min_str_angle = FALSE;
  set_max_str_angle = FALSE;
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
            /*
             * Power supply meas
             */
            avg *=  5545;
            avg /= 12412;
            break;
          case MEAS_TEMP1:
            /*
             * NTC1 meas
             */
            avg = measInterpolateNTC(avg);
            break;
          case MEAS_TEMP2:
            /*
             * NTC2 meas
             */
            avg = measInterpolateNTC(avg);
            break;
          case MEAS_STEERING:
            /*
             * Steering angle meas
             */
            if(set_max_str_angle){
              max_str_angle = avg;
              if(eepromWrite(MAX_STR_ANGLE, max_str_angle) != 0){
                max_str_angle = avg;
              }
              set_max_str_angle = FALSE;
            }

            if(set_min_str_angle){
              min_str_angle = avg;
              if(eepromWrite(MIN_STR_ANGLE, min_str_angle) != 0){
                min_str_angle = avg;
              }
              set_min_str_angle = FALSE;
            }

            avg = avg > max_str_angle ? max_str_angle : avg;
            avg = avg < min_str_angle ? min_str_angle : avg;

            str_angle = map(avg, min_str_angle, max_str_angle, MIN_STR, MAX_STR);
            avg = str_angle;

            avg = avg > MAX_STR ? MAX_STR : avg;
            avg = avg < MIN_STR ? MIN_STR: avg;
            break;
          case MEAS_IS_IN_DRIVE:
            avg = avg > 500 ? 1 : 0;
            break;
          case MEAS_CHP_B:
            /*
             * Suspension left
             */
            break;
          case MEAS_CHP_J:
            /*
             * Suspension right
             */
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
            curr_avg = avg;
            /*
             * Motor Current Meas Buffer
             */
/*
            Motor_current_index++;
            if(Motor_current_index > (MOTOR_CURRENT_LENGTH - 1)){
              Motor_current_index = 0;
            }
            Motor_current_avg[Motor_current_index] = avg;
            motor_curr = 0;
            for (i = 0; i < MOTOR_CURRENT_LENGTH; i++){
              motor_curr += Motor_current_avg[i];
            }
            avg = motor_curr / MOTOR_CURRENT_LENGTH;

            /*
             * Motor Current Calculate
             */

            double temp = 0;
            avg -= NULL_AMPER_ADC;
            temp = (double)avg / CURRENT_PER_ADC;
            avg =  (int)(temp * 100);

            /*
             * Motor current calculate with SUN CURRENT and PACK CURRENT
             */

/*
            int temp = bmsitems.pack_current - (mlitems.sun_current < 0 ? 0 : (mlitems.sun_current / 10));
            avg = bmsitems.pack_inst_volt * temp;
            avg /= 100;*/



            //avg = avg < 0 ? 0 : avg;
            break;

          case MEAS2_THROTTLE:
            /*
             * Throttle pedal position
             */
            if(set_max_throttle){
              max_throttle = avg;
              if(eepromWrite(MAX_THROTTLE, max_throttle) != 0){
                max_throttle = avg;
              }
              set_max_throttle = FALSE;
            }

            if(set_min_throttle){
              min_throttle = avg;
              if(eepromWrite(MIN_THROTTLE, min_throttle) != 0){
                min_throttle = avg;
              }
              set_min_throttle = FALSE;
            }

            avg = avg > max_throttle ? max_throttle : avg;
            avg = avg < min_throttle ? min_throttle : avg;

            throttle = map(avg, min_throttle, max_throttle, MIN_PWM, MAX_PWM);
            avg = throttle;

            avg = avg > MAX_PWM ? MAX_PWM : avg;
            avg = avg < MIN_PWM ? MIN_PWM: avg;
            break;

          case  MEAS2_REGEN_BRAKE:
            /*
             * Brake pedal position
             */
            if(set_max_regen){ 
              max_regen_brake = avg;
              if(eepromWrite(MAX_REGEN_BRAKE, max_regen_brake) != 0){
                max_regen_brake = avg;
              }
              set_max_regen = FALSE;
            }

            if(set_min_regen){ 
              min_regen_brake = avg;
              if(eepromWrite(MIN_REGEN_BRAKE, min_regen_brake) != 0){
                min_regen_brake = avg;
              }
              set_min_regen = FALSE;
            }

            avg = avg > max_regen_brake ? max_regen_brake : avg;
            avg = avg < min_regen_brake ? min_regen_brake: avg;

            regen_brake = map(avg, min_regen_brake, max_regen_brake, MIN_PERCENT, MAX_PERCENT);
            avg = regen_brake;

            avg = avg > MAX_PERCENT ? MAX_PERCENT : avg;
            avg = avg < MIN_PERCENT ? MIN_PERCENT : avg;
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
  set_min_throttle = TRUE;
}
void meas_throttleSetMax(void){
  set_max_throttle = TRUE;
}

void meas_regen_brakeSetMin(void){
  set_min_regen = TRUE;
}
void meas_regen_brakeSetMax(void){
  set_max_regen = TRUE;
}

void meas_str_angleSetMin(void){
  set_min_str_angle = TRUE;
}
void meas_str_angleSetMax(void){
  set_max_str_angle = TRUE;
}

void mainTime(systime_t maradek_time, uint8_t value){
  ido = maradek_time;
  valtozo = value;
}


/*
 * Calculate the temperature from the ADC value
 */
int16_t measInterpolateNTC(adcsample_t rawvalue){
  int16_t value = 0;
  adcsample_t left, right;
  int i;

  if(rawvalue >= measNTCcalib[0]){
    value = MEAS_NTCCAL_START;
  }
  else if(rawvalue < measNTCcalib[MEAS_NTCCAL_NUM - 1]){
    value = MEAS_NTCCAL_START + (MEAS_NTCCAL_NUM - 1) * MEAS_NTCCAL_STEP;
  }
  else {
    i = 0;
    left = measNTCcalib[i];
    right = measNTCcalib[i + 1];
    while(rawvalue < right && i < MEAS_NTCCAL_NUM - 1){
      i++;
      left = right;
      right = measNTCcalib[i + 1];
    }
    value = MEAS_NTCCAL_START +
            i * MEAS_NTCCAL_STEP +
            (MEAS_NTCCAL_STEP * (left - rawvalue)) / (left - right);
  }
  return value;
}


/*
 * ADC error callback. TODO: utilize
 */
static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

/*
 * Shell commands
 */

void cmd_measvalues(BaseSequentialStream *chp, int argc, char *argv[]){
  enum measChannels ch;

  static const char * const names[] = {

      "UBAT",
      "BRAKE_PRESSURE1",
      "BRAKE_PRESSURE2",
      "MEAS_STEERING",
      "MEAS_IS_IN_DRIVE",
      "SEN2",
      "SEN3"};

  static const char * const names2[] = {

      "CURR1",
      "THROTTLE",
      "REGEN_BRAKE"};

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
      chprintf(chp, "curr_avg: %15d\r\n", curr_avg);
      chprintf(chp, "CALC_AVG_SPEED: %15d\r\n", calcGetValue(CALC_AVG_SPEED));
      chprintf(chp, "curr_avg: %15d\r\n", curr_avg);
      chprintf(chp, "\r\n");
      chprintf(chp, "min_throttle: %15d\r\n", min_throttle);
      chprintf(chp, "max_throttle: %15d\r\n", max_throttle);
      chprintf(chp, "min_regen_brake: %15d\r\n", min_regen_brake);
      chprintf(chp, "max_regen_brake: %15d\r\n", max_regen_brake);
      chThdSleepMilliseconds(500);
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

void cmd_getRegenBrake(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

  if ((argc == 1) && (strcmp(argv[0], "set_min") == 0)){
    meas_regen_brakeSetMin();
    chprintf(chp, "Set regen. brake pedal min. value!\r\n");
  }

  else if ((argc == 1) && (strcmp(argv[0], "set_max") == 0)){
    meas_regen_brakeSetMax();
    chprintf(chp, "Set regen. brake pedal max. value!\r\n");
  }

  else{
    chprintf(chp, "Usage: brake set_min\r\n");
    chprintf(chp, "                set_max\r\n");  
  }
  return;
}

void cmd_getSteeringAngle(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

  if ((argc == 1) && (strcmp(argv[0], "set_min") == 0)){
    meas_str_angleSetMin();
    chprintf(chp, "Set regen. brake pedal min. value!\r\n");
  }

  else if ((argc == 1) && (strcmp(argv[0], "set_max") == 0)){
    meas_str_angleSetMax();
    chprintf(chp, "Set regen. brake pedal max. value!\r\n");
  }

  else{
    chprintf(chp, "Usage: brake set_min\r\n");
    chprintf(chp, "                set_max\r\n");  
  }
  return;
}

void cmd_mainValues(BaseSequentialStream *chp, int argc, char *argv[]){
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      //chprintf(chp, "chTimeNov(), beallitott, maradek: %15d, %15d, %15d\r\n", chTimeNow(), ido, ido - chTimeNow());
      chprintf(chp,
       "valtozo, chTimeNov(): %15d, %15d\r\n", valtozo, chTimeNow());
      chThdSleepMilliseconds(50);
  }
}
