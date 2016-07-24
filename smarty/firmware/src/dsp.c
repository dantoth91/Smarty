/**
 * Smarty - Copyright (C) 2015
 *
 * @file    dsp.x
 * @brief   Display Application Layer .
 *
 * @details This file contains those functions which
 *          show the content in the display.
 *          (Numbers, gifs, pngs etc.)
 */

#include "uart_comm.h"
#include "meas.h"
#include "gfx.h"
#include "dsp.h"
#include "cruise.h"
#include "light.h"
#include "can_items.h"
#include "can_comm.h"
#include "calc.h"
#include "nmea.h"

#include "chprintf.h"

/*===========================================================================*/
/* Global variables.                                                         */
/*===========================================================================*/

/*
 * OldValues and NewValues contain the actual value of the graphical elements.
 */
static int32_t OldValues[DSP_NUM_MSG + 2] = {-1};
static int32_t NewValues[DSP_NUM_MSG + 2] = {-1};

nmeaPosition_t pos;
static int32_t MerlegPower = 0;
static uint8_t blinker = 0;
static uint8_t TPBlinker = 2;

static uint8_t bus_status;

/*
 * Show a 'png' image with X and Y point.
 */
void gfx_ShowPngXY(int sector, int x, int y){
  media_SetSector(0, sector);
  media_Image(x, y);
}

/*
 * Show a 'gif' image with X and Y point.
 */
void gfx_ShowGifXY(int sector, int x, int y, int FrameNumber){
  media_SetSector(0, sector);
  media_VideoFrame(x, y, FrameNumber);
}

int ConvertToBcd(int value, char *bcd){
  int x1000 = 0;
  int x100 = 0;
  int x10 = 0;
  int x1 = 0;

  x1000 = value / 1000;
  value -= (x1000 * 1000);
  x100 = value / 100;
  value -= (x100 * 100);
  x10 = value / 10;
  value -= (x10 * 10);
  x1 = value;

  bcd[3] = x1000;
  bcd[2] = x100;
  bcd[1] = x10;
  bcd[0] = x1;

  int i = 0;
  for(i = 3; bcd[i]==0; i--);
  return (i+1) > 0 ? (i+1) : 0;
}

void gfx_PlaceNumber(int sector, int x, int y, int distance, uint16_t value, char maxdigit, char fill){
  int orig_y = y;
  char bcd[4];
  int digits = 0;
  digits = ConvertToBcd(value, &bcd);
  int i;
  for(i = (digits-1); i >= 0; i--)
  {
    gfx_ShowGifXY(sector, x, y, bcd[(digits-1)-i]);
    y -= distance;
  }
  y = orig_y;

  if (value == 0){
    gfx_ShowGifXY(sector, x, y, 0);
  }
  y -= distance;
  for(i = 1; i < maxdigit; i++)
  {
    if((i+1) > digits)
        gfx_ShowGifXY(sector, x, y, fill);

    y -= distance;
  }
}

/*
 * Load the images to the display with static values.
 */
void LoadImages(){

  if(NewValues[DSP_TOLTES] == 0){
    gfx_Transparency(1);
    gfx_TransparentColour(BLUE);
    gfx_ShowGifXY(SECTOR_MERLEGGRAF, X(MERLEGGRAF), Y(MERLEGGRAF), 0);//merleggraf

    gfx_PlaceNumber(SECTOR_KMORA, X(KMORA), Y(KMORA), DISTANCE_KMORA, 0, THREE_DIGIT, FILL_WITH_BLANK); //kmora

    gfx_ShowPngXY(SECTOR_TEMPOMAT_KMH, X(KMORA_MERTEKEGYSEG), Y(KMORA_MERTEKEGYSEG)); //kmora mertekegyseg
  }

  gfx_ShowGifXY(SECTOR_BALINDEX, X(BALINDEX), Y(BALINDEX), 0);//balindex

  gfx_ShowGifXY(SECTOR_JOBBINDEX, X(JOBBINDEX), Y(JOBBINDEX), 0);//jobbindex

  gfx_ShowGifXY(SECTOR_TIREPRESSURE, X(TIREPRESSURE), Y(TIREPRESSURE), 1);//tire pressure

  gfx_ShowGifXY(SECTOR_LIGHT, X(LIGHT), Y(LIGHT), 0);//lampa

  gfx_ShowGifXY(SECTOR_HIBA, X(HIBA), Y(HIBA), 0);//hiba

  gfx_ShowGifXY(SECTOR_AKKUHIBA, X(AKKUHIBA), Y(AKKUHIBA), 0);//akkuhiba

  gfx_ShowGifXY(SECTOR_FEK, X(FEK), Y(FEK), 0);//regenerativ fek

  gfx_PlaceNumber(SECTOR_IDO, X(TIME_H), Y(TIME_H), DISTANCE_IDO, 0, TWO_DIGIT, FILL_WITH_ZEROS); //time hour
  gfx_ShowPngXY(SECTOR_IDO_KPONT, X(KPONT1), Y(KPONT1));//kettõspont

  gfx_PlaceNumber(SECTOR_IDO, X(TIME_M), Y(TIME_M), DISTANCE_IDO, 0, TWO_DIGIT, FILL_WITH_ZEROS); //time minute
  gfx_ShowPngXY(SECTOR_IDO_KPONT, X(KPONT2), Y(KPONT2));//kettõspont

  gfx_PlaceNumber(SECTOR_IDO, X(TIME_S), Y(TIME_S), DISTANCE_IDO, 0, TWO_DIGIT, FILL_WITH_ZEROS); //time seconds

  gfx_ShowPngXY(SECTOR_MOTORJEL, X(MOTORJEL), Y(MOTORJEL)); //motorjel

  gfx_ShowGifXY(SECTOR_MERTERTEK, X(MOTORWATTJEL), Y(MOTORWATTJEL), 12);//motor power W

  gfx_PlaceNumber(SECTOR_MERTERTEK, X(MOTORWATT), Y(MOTORWATT), DISTANCE_MERTERTEK, 0, FOUR_DIGIT, FILL_WITH_BLANK); //motor power

  gfx_ShowGifXY(SECTOR_AKKU_PICT, X(AKKUPICT), Y(AKKUPICT), 0); //akku piktogram

  gfx_PlaceNumber(SECTOR_MERTERTEK, X(AKKUSOC), Y(AKKUSOC), DISTANCE_MERTERTEK, 0, THREE_DIGIT, FILL_WITH_BLANK); //akku SOC

  gfx_ShowGifXY(SECTOR_MERTERTEK, X(AKKUSZAZALEKJEL), Y(AKKUSZAZALEKJEL), 11);// akku % jel

  gfx_PlaceNumber(SECTOR_DIST_NUMBER, X(TEMPOMATSEBESSEG), Y(TEMPOMATSEBESSEG), DISTANCE_DIST_NUMBER, 0, THREE_DIGIT, FILL_WITH_BLANK);    //tempomat sebesség
  gfx_ShowPngXY(SECTOR_TEMPOMAT_KMH, X(TEMPOMATKMH), Y(TEMPOMATKMH));    //tempomat km/h

  gfx_ShowGifXY(SECTOR_TEMPOMAT_PICT, X(TEMPOMATPICT), Y(TEMPOMATPICT), 0); //tempomat  piktogram

  gfx_ShowPngXY(SECTOR_BESUGARZAS_PICT, X(BESUGARZASPICT), Y(BESUGARZASPICT));  //besugárzás piktogram

  gfx_ShowGifXY(SECTOR_MERTERTEK, X(BESUGARZASWATTJEL), Y(BESUGARZASWATTJEL), 12);// besugarzas w jel

  gfx_PlaceNumber(SECTOR_MERTERTEK, X(BESUGARZAS), Y(BESUGARZAS), DISTANCE_MERTERTEK, 0, FOUR_DIGIT, FILL_WITH_BLANK); //besugarzas

  gfx_ShowGifXY(SECTOR_MERLEGJEL, X(MERLEGJEL), Y(MERLEGJEL), 0);// merleg jel

  gfx_ShowGifXY(SECTOR_MERTERTEK, X(MERLEGWATTJEL), Y(MERLEGWATTJEL), 12);// merleg w jel

  gfx_PlaceNumber(SECTOR_MERTERTEK, X(MERLEGSZAM), Y(MERLEGSZAM), DISTANCE_MERTERTEK, 0, FOUR_DIGIT, FILL_WITH_BLANK); //merleg szam

  gfx_PlaceNumber(SECTOR_DIST_NUMBER, X(DISTANCETIZED), Y(DISTANCETIZED), DISTANCE_DIST_NUMBER, 0, ONE_DIGIT, FILL_WITH_BLANK); //distance .0
  gfx_ShowPngXY(SECTOR_DISTANCE_PONT, X(DISTANCEPONT), Y(DISTANCEPONT)); //.
  gfx_PlaceNumber(SECTOR_DIST_NUMBER, X(DISTANCENUM), Y(DISTANCENUM), DISTANCE_DIST_NUMBER, 0, FOUR_DIGIT, FILL_WITH_BLANK); //distance 0.

  gfx_ShowPngXY(SECTOR_DISTANCE, X(DISTANCETEXT), Y(DISTANCETEXT)); //distance text

  gfx_ShowGifXY(SECTOR_GPS, X(GPS), Y(GPS), 1);// GPS

  draw_line(365, 0, 365, 272, WHITE);
  draw_line(150, 0, 150, 272, WHITE);
}

/*
 * Collect the values for the pictograms.
 */
void PictogramDataBinding(void){
  int new_val = 0;

  //BALINDEX
  if (getLightFlashing(3) || getLightFlashing(4)) { new_val = 0; }
  else
    new_val = 1;
  NewValues[DSP_BALINDEX] = new_val;

  //JOBBINDEX
  if (getLightFlashing(2) || getLightFlashing(4)) { new_val = 0; }
  else
    new_val = 1;
  NewValues[DSP_JOBBINDEX] = new_val;

  //LÁMPA
  if (!getLightFlashing(5))
    new_val = 0;
  else
    new_val = 1;
  NewValues[DSP_LIGHT] = new_val;

  //akkuhiba
  if(bmsitems.pack_soc < AKKUHIBA_ON_SOC)
    new_val = 0;
  else
    new_val = 1;
  NewValues[DSP_AKKUHIBA] = new_val;

  //TEMPOMAT
  if (!cruiseStatus())
    new_val = 0;
  else
    new_val = 1;

  NewValues[DSP_TEMPOMATJEL] = new_val;

  //hiba
  if(GetCurrentLimitIsOn())
    new_val = 0;
  else
    new_val = 1;

  NewValues[DSP_HIBA] = new_val;

  //gps
  nmeaGetCurrentPosition(&pos);
  int satellites = pos.Svs;

  if(satellites < 3){
    //blinking
    switch(blinker){
    case 0:
      blinker = 1;
      new_val = blinker;
      break;
    case 1:
      blinker = 0;
      new_val = blinker;
      break;
    }

  }else if(satellites >=3){
    new_val = 1;
  }
  NewValues[DSP_GPS] = new_val;

  //charge
  uint8_t IsInCharge = bmsitems.custom_flag_2 & 0x08;   //IsInCharge mode?
  NewValues[DSP_TOLTES] = IsInCharge > 0 ? 1 : 0;

  //tirepressure
  int i;
  new_val = 2;      //default is blank
  //yellow alarm
  for(i = 0; i < NUM_OF_SENSORS; i++){
    uint8_t IsSensorConnected = TirePressures.Flags[i] & TIRE_SENSOR_CONNECTED;
    uint8_t PressFault = TirePressures.TirePressureThresholdDetection[i] & TIRE_UNDER_PRESSURE;
    if(PressFault != 0x40 && IsSensorConnected == 1){
      new_val = 0;
      break;
    }
  }

  //blinkink red alarm
  int TireLeakDetected = 0x00;

  for(i = 0; i < NUM_OF_SENSORS; i++){
    uint8_t IsSensorConnected = TirePressures.Flags[i] & TIRE_SENSOR_CONNECTED;
    TireLeakDetected = TirePressures.Flags[i] & TIRE_LEAK_DETECTED_MASK;
    if(TireLeakDetected != 0x00 && IsSensorConnected == 1){
      switch(TPBlinker){
      case 2:
        TPBlinker = 1;
        new_val = TPBlinker;
        break;
      case 1:
        TPBlinker = 2;
        new_val = TPBlinker;
        break;
      }
      break;
    }
  }

  NewValues[DSP_TIREPRESSURE] = new_val;
}

/*
 * Collect the values for the numbers in the display.
 */
void NumberDataBinding(void){
  int16_t new_val;

  //KM ora
  NewValues[DSP_KMORA] = speedGetSpeed();

  //IDÕ
  nmeaGetCurrentPosition(&pos);
  int dayseconds = pos.Tof % 86400;
  NewValues[DSP_TIME_H] = (dayseconds / 3600) + 2;
  NewValues[DSP_TIME_M] = (dayseconds % 3600) / 60;
  NewValues[DSP_TIME_S] = (dayseconds % 3600) % 60;

  //Motor power
  new_val = calcGetValue(CALC_MOTOR_POWER);
  if (new_val < 0)
    new_val = 0;

  NewValues[DSP_MOTORPOWER] = new_val;

  //AKKU SOC
  NewValues[DSP_AKKUSOC] = (bmsitems.pack_soc/10);

  //Besugarzas
  NewValues[DSP_SUNPOWER] = calcGetValue(CALC_SUN_POWER);

  //merlegwatt
  new_val = MerlegPower;
  if(new_val < 0)
  {
    NewValues[DSP_MERLEGJEL] = 1;
    new_val = -new_val;
  }else if(new_val > 0){
    NewValues[DSP_MERLEGJEL] = 2;
  }else if(new_val == 0){
    NewValues[DSP_MERLEGJEL] = 0;
  }

  NewValues[DSP_MERLEGWATT] = new_val;

  //Meter
  int16_t meter = GetMeterCounter();
  NewValues[DSP_DISTANCETIZED] = (meter / 100);

  //kmer
  NewValues[DSP_DISTANCE] = GetKmeterDistance();
}

/*
 * Collect the values for the gauges.
 */
void GaugeDataBinding(void){
  //Regenerative brake
  int new_val = (int32_t)(measGetValue_2(MEAS2_REGEN_BRAKE) * REGENBRAKE_RESOLUTION);
  new_val = new_val> 320 ? 320 : new_val;
  NewValues[DSP_FEK] = new_val;

  //Energybalance
  double temp = ((double)bmsitems.pack_current / 10);
  temp *= (bmsitems.pack_inst_volt / 10);
  MerlegPower = (int32_t)(temp);
  if(MerlegPower < 0)
  {
    new_val = -MerlegPower;
    new_val = new_val * MERLEGGRAF_ZOLD_DIVIDE;
    new_val = new_val > 31 ? 31 : new_val;
    new_val += 1;
  }else if(MerlegPower > 0){
    new_val = MerlegPower * MERLEGGRAF_PIROS_DIVIDE;
    new_val = new_val > 31 ? 31 : new_val;
    new_val += 33;
  }else if(MerlegPower == 0){
    new_val = 0;
  }
  NewValues[DSP_MERLEGGRAF] = new_val;
}

/*
 * Refresh the pictogram state in the display from NewValues array.
 */
void RefreshPiktograms(void){
  if(OldValues[DSP_BALINDEX] != NewValues[DSP_BALINDEX]){
    gfx_ShowGifXY(SECTOR_BALINDEX, X(BALINDEX), Y(BALINDEX), NewValues[DSP_BALINDEX]);//balindex
    OldValues[DSP_BALINDEX] = NewValues[DSP_BALINDEX];
  }

  if(OldValues[DSP_JOBBINDEX] != NewValues[DSP_JOBBINDEX]){
    gfx_ShowGifXY(SECTOR_JOBBINDEX, X(JOBBINDEX), Y(JOBBINDEX), NewValues[DSP_JOBBINDEX]);//jobbindex
    OldValues[DSP_JOBBINDEX] = NewValues[DSP_JOBBINDEX];
  }

  if(OldValues[DSP_TIREPRESSURE] != NewValues[DSP_TIREPRESSURE]){
    gfx_ShowGifXY(SECTOR_TIREPRESSURE, X(TIREPRESSURE), Y(TIREPRESSURE), NewValues[DSP_TIREPRESSURE]);//tire pressure
    OldValues[DSP_TIREPRESSURE] = NewValues[DSP_TIREPRESSURE];
  }

  if(OldValues[DSP_LIGHT] != NewValues[DSP_LIGHT]){
    gfx_ShowGifXY(SECTOR_LIGHT, X(LIGHT), Y(LIGHT), NewValues[DSP_LIGHT]);//lampa
    OldValues[DSP_LIGHT] = NewValues[DSP_LIGHT];
  }

  if(OldValues[DSP_HIBA] != NewValues[DSP_HIBA]){
    gfx_ShowGifXY(SECTOR_HIBA, X(HIBA), Y(HIBA), NewValues[DSP_HIBA]);//hiba
    OldValues[DSP_HIBA] = NewValues[DSP_HIBA];
  }

  if(OldValues[DSP_AKKUHIBA] != NewValues[DSP_AKKUHIBA]){
    gfx_ShowGifXY(SECTOR_AKKUHIBA, X(AKKUHIBA), Y(AKKUHIBA), NewValues[DSP_AKKUHIBA]);//akkuhiba
    OldValues[DSP_AKKUHIBA] = NewValues[DSP_AKKUHIBA];
  }

  if(OldValues[DSP_TEMPOMATJEL] != NewValues[DSP_TEMPOMATJEL]){
    gfx_ShowGifXY(SECTOR_TEMPOMAT_PICT, X(TEMPOMATPICT), Y(TEMPOMATPICT), NewValues[DSP_TEMPOMATJEL]); //tempomat piktogram
    OldValues[DSP_TEMPOMATJEL] = NewValues[DSP_TEMPOMATJEL];
  }


  if(OldValues[DSP_MERLEGJEL] != NewValues[DSP_MERLEGJEL]){
    gfx_ShowGifXY(SECTOR_MERLEGJEL, X(MERLEGJEL), Y(MERLEGJEL), NewValues[DSP_MERLEGJEL]);// merleg jel
    OldValues[DSP_MERLEGJEL] = NewValues[DSP_MERLEGJEL];
  }

  if(OldValues[DSP_GPS] != NewValues[DSP_GPS]){
    gfx_ShowGifXY(SECTOR_GPS, X(GPS), Y(GPS), NewValues[DSP_GPS]);// GPS
    OldValues[DSP_GPS] = NewValues[DSP_GPS];
  }

  if(OldValues[DSP_TOLTES] != NewValues[DSP_TOLTES]){
    if(NewValues[DSP_TOLTES] == 1){

      DisplayOnOff(LCD_OFF);
      ReDrawLcd();
      gfx_ShowPngXY(SECTOR_TOLTES, X(TOLTES), Y(TOLTES)); // toltes
      DisplayOnOff(LCD_ON);

    }
    else if(NewValues[DSP_TOLTES] == 0){

      DisplayOnOff(LCD_OFF);
      ReDrawLcd();
      DisplayOnOff(LCD_ON);

    }
    OldValues[DSP_TOLTES] = NewValues[DSP_TOLTES];
  }

}

/*
 * Refresh the number value in the display from NewValues array.
 */
void RefreshNumbers(void){

  if((OldValues[DSP_KMORA] != NewValues[DSP_KMORA]) && NewValues[DSP_TOLTES] == 0){
    gfx_PlaceNumber(SECTOR_KMORA, X(KMORA), Y(KMORA), DISTANCE_KMORA, NewValues[DSP_KMORA], THREE_DIGIT, FILL_WITH_BLANK); //kmora
    OldValues[DSP_KMORA] = NewValues[DSP_KMORA];
  }

  if(OldValues[DSP_TIME_H] != NewValues[DSP_TIME_H]){
    gfx_PlaceNumber(SECTOR_IDO, X(TIME_H), Y(TIME_H), DISTANCE_IDO, NewValues[DSP_TIME_H], TWO_DIGIT, FILL_WITH_ZEROS); //time hour
    OldValues[DSP_TIME_H] = NewValues[DSP_TIME_H];
  }

  if(OldValues[DSP_TIME_M] != NewValues[DSP_TIME_M]){
    gfx_PlaceNumber(SECTOR_IDO, X(TIME_M), Y(TIME_M), DISTANCE_IDO, NewValues[DSP_TIME_M], TWO_DIGIT, FILL_WITH_ZEROS); //time minute
    OldValues[DSP_TIME_M] = NewValues[DSP_TIME_M];
  }

  if(OldValues[DSP_TIME_S] != NewValues[DSP_TIME_S]){
    gfx_PlaceNumber(SECTOR_IDO, X(TIME_S), Y(TIME_S), DISTANCE_IDO, NewValues[DSP_TIME_S], TWO_DIGIT, FILL_WITH_ZEROS); //time seconds
    OldValues[DSP_TIME_S] = NewValues[DSP_TIME_S];
  }

  if(OldValues[DSP_MOTORPOWER] != NewValues[DSP_MOTORPOWER]){
    gfx_PlaceNumber(SECTOR_MERTERTEK, X(MOTORWATT), Y(MOTORWATT), DISTANCE_MERTERTEK, NewValues[DSP_MOTORPOWER], FOUR_DIGIT, FILL_WITH_BLANK); //motor power
    OldValues[DSP_MOTORPOWER] = NewValues[DSP_MOTORPOWER];
  }

  if(OldValues[DSP_AKKUSOC] != NewValues[DSP_AKKUSOC]){
    gfx_PlaceNumber(SECTOR_MERTERTEK, X(AKKUSOC), Y(AKKUSOC), DISTANCE_MERTERTEK, NewValues[DSP_AKKUSOC], THREE_DIGIT, FILL_WITH_BLANK); //akku SOC
    OldValues[DSP_AKKUSOC] = NewValues[DSP_AKKUSOC];
  }

  if(OldValues[DSP_SUNPOWER] != NewValues[DSP_SUNPOWER]){
    gfx_PlaceNumber(SECTOR_MERTERTEK, X(BESUGARZAS), Y(BESUGARZAS), DISTANCE_MERTERTEK, NewValues[DSP_SUNPOWER], FOUR_DIGIT, FILL_WITH_BLANK); //besugarzas
    OldValues[DSP_SUNPOWER] = NewValues[DSP_SUNPOWER];
  }

  if(OldValues[DSP_MERLEGWATT] != NewValues[DSP_MERLEGWATT]){
    gfx_PlaceNumber(SECTOR_MERTERTEK, X(MERLEGSZAM), Y(MERLEGSZAM), DISTANCE_MERTERTEK, NewValues[DSP_MERLEGWATT], FOUR_DIGIT, FILL_WITH_BLANK); //merleg szam
    OldValues[DSP_MERLEGWATT] = NewValues[DSP_MERLEGWATT];
  }

  if(OldValues[DSP_DISTANCETIZED] != NewValues[DSP_DISTANCETIZED]){
    gfx_PlaceNumber(SECTOR_DIST_NUMBER, X(DISTANCETIZED), Y(DISTANCETIZED), DISTANCE_DIST_NUMBER, NewValues[DSP_DISTANCETIZED], ONE_DIGIT, FILL_WITH_BLANK); //distance .0
    OldValues[DSP_DISTANCETIZED] = NewValues[DSP_DISTANCETIZED];
  }

  if(OldValues[DSP_DISTANCE] != NewValues[DSP_DISTANCE]){
    gfx_PlaceNumber(SECTOR_DIST_NUMBER, X(DISTANCENUM), Y(DISTANCENUM), DISTANCE_DIST_NUMBER, NewValues[DSP_DISTANCE], FOUR_DIGIT, FILL_WITH_BLANK); //distance 0.
    OldValues[DSP_DISTANCE] = NewValues[DSP_DISTANCE];
  }
}

/*
 * Refresh the tempomat speed number in the display from NewValues array.
 */
void RefreshTempomatNumber(void){
  //Tempomat sebesseg
  NewValues[DSP_TEMPOMAT] = cruiseGet();

  if(OldValues[DSP_TEMPOMAT] != NewValues[DSP_TEMPOMAT]){
    gfx_PlaceNumber(SECTOR_DIST_NUMBER, X(TEMPOMATSEBESSEG), Y(TEMPOMATSEBESSEG), DISTANCE_DIST_NUMBER, NewValues[DSP_TEMPOMAT], THREE_DIGIT, FILL_WITH_BLANK);    //tempomat sebesség
    OldValues[DSP_TEMPOMAT] = NewValues[DSP_TEMPOMAT];
  }
}

/*
 * Refresh the gauge state in the display from NewValues array.
 */
void RefreshGauge(void){

  if(OldValues[DSP_FEK] != NewValues[DSP_FEK]){
    gfx_ShowGifXY(SECTOR_FEK, X(FEK), Y(FEK), NewValues[DSP_FEK]);//regenerativ fek
    OldValues[DSP_FEK] = NewValues[DSP_FEK];
  }
  if((OldValues[DSP_MERLEGGRAF] != NewValues[DSP_MERLEGGRAF]) && NewValues[DSP_TOLTES] == 0){
    gfx_Transparency(1);
    gfx_TransparentColour(BLUE);
    gfx_ShowGifXY(SECTOR_MERLEGGRAF, X(MERLEGGRAF), Y(MERLEGGRAF), NewValues[DSP_MERLEGGRAF]);//merleggraf
    OldValues[DSP_MERLEGGRAF] = NewValues[DSP_MERLEGGRAF];
  }
}


/*
 * The DSPThread responsible for the continous display drawing.
 */
static WORKING_AREA(waDSPThread, 1024);
static msg_t DSPThread(void *arg) {
  systime_t time;

  (void)arg;
  chRegSetThreadName("dspthread");

  ReDrawLcd();
  //DisplayOnOff(LCD_ON);


  int refresh = 0;
  time = chTimeNow();
  while (true) {

    if(refresh % 5 == 0){
      PictogramDataBinding();
      RefreshPiktograms();
    }

    if(refresh % 7 == 0){
      NumberDataBinding();
      RefreshNumbers();
    }

    RefreshTempomatNumber();
    GaugeDataBinding();
    RefreshGauge();

    refresh++;
    chThdSleepMilliseconds(50);
  }
}


/*
 * The DSPReader reads the button values connected into the LCD.
 */
static WORKING_AREA(waDSPReader, 1024);
static msg_t DSPReader(void *arg) {
  systime_t time;

  (void)arg;
  chRegSetThreadName("dspreader");
  time = chTimeNow();
  while (TRUE) {
    time += MS2ST(50);
    uint8_t temp = bus_Read();
    if(temp != 0x00)
      bus_status = temp;

    chThdSleepUntil(time);
  }
}

uint8_t dspGetValue(void){
    return bus_status;
}

/*
 * Switch on/off the display with animation.
 */
void DisplayOnOff(bool_t OnOrOff){
  if(OnOrOff){
    int i;
    for(i=0; i<=15; i++){
      gfx_Contrast(i);
      chThdSleepMilliseconds(ANIMATION_TIME);
    }
  }else{
    gfx_Contrast(0);
  }
}

/*
 * Redraw the elements in the lcd.
 */
void ReDrawLcd(){
  gfx_Cls();
  LoadImages();
  DeleteOldValues();
}

/*
 * Clean the OldValues array elements.
 */
void DeleteOldValues(){
  int i;
  for(i = 0; i < DSP_NUM_MSG; i++)
    OldValues[i] = -1;
}

/*
 * Inits the display.
 */
void dspInit(){
  SerialInit();

  DisplayOnOff(LCD_OFF);
  file_Mount();


  /*
   * Creates the DSPthread.
   */
  chThdCreateStatic(waDSPReader, sizeof(waDSPReader), NORMALPRIO + 7, DSPReader, NULL);

  chThdCreateStatic(waDSPThread, sizeof(waDSPThread), NORMALPRIO + 7, DSPThread, NULL);
}

void cmd_dspvalues(BaseSequentialStream *chp, int argc, char *argv[]) {
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  uint16_t values = 0x00;
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B\x63");
    chprintf(chp, "\x1B[2J");
    chprintf(chp, "bus_read 0x%02x\r\n", bus_status);
    chprintf(chp, "meter %d\r\n", NewValues[DSP_DISTANCETIZED]);
    chprintf(chp, "Merlegwatt %d\r\n", NewValues[DSP_MERLEGWATT]);
    chprintf(chp, "Motorwatt %d\r\n", NewValues[DSP_MOTORPOWER]);
    chprintf(chp, "Merleggraf %d\r\n", NewValues[DSP_MERLEGGRAF]);
    //chprintf(chp, "Pressfault %d\r\n", PressFault);
    chprintf(chp, "TirePressure %d\r\n", NewValues[DSP_TIREPRESSURE]);




    chThdSleepMilliseconds(50);
  }
}
