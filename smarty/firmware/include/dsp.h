/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/

#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED

#include "ch.h"
#include "hal.h"

#define ONE_DIGIT     1
#define TWO_DIGIT     2
#define THREE_DIGIT   3
#define FOUR_DIGIT    4


//Color
#define RED                       (0xF800)
#define BLUE                      (0x001F)
#define BLACK                     (0x0000)
#define WHITE                     (0xFFFF)

//Display SDCard sectors
#define SECTOR_MERTERTEK          0x484B
#define SECTOR_DIST_NUMBER        0x0022
#define SECTOR_DISTANCE_PONT      0x001C
#define SECTOR_DISTANCE           0x001D
#define SECTOR_TEMPOMAT_KMH       0x488D
#define SECTOR_TEMPOMAT_PICT      0x4891
#define SECTOR_AKKU_PICT          0x000E
#define SECTOR_BESUGARZAS_PICT    0x0016
#define SECTOR_BALINDEX           0x1418
#define SECTOR_JOBBINDEX          0x1423
#define SECTOR_TIREPRESSURE       0x13F1
#define SECTOR_LIGHT              0x152F
#define SECTOR_HIBA               0x140A
#define SECTOR_AKKUHIBA           0x0000
#define SECTOR_MERLEGGRAF         0x1541
#define SECTOR_FEK                0x0037
#define SECTOR_KMORA              0x1434
#define SECTOR_IDO                0x4865
#define SECTOR_IDO_KPONT          0x488A
#define SECTOR_MOTORJEL           0x485F
#define SECTOR_KMORA_MERTEKEGYSEG 0x142E
#define SECTOR_MERLEGJEL          0x483E
#define SECTOR_TOLTES             0x48AA
#define SECTOR_GPS                0x13E8


#define DISTANCE_MERTERTEK      15
#define DISTANCE_DIST_NUMBER    17
#define DISTANCE_KMORA          64
#define DISTANCE_IDO            23

#define FILL_WITH_BLANK         10
#define FILL_WITH_ZEROS         0

//COORDINATES

#define BALINDEX                ( 434, 4   )
#define JOBBINDEX               ( 434, 235 )
#define TIREPRESSURE            ( 380, 10  )
#define LIGHT                   ( 384, 215 )
#define HIBA                    ( 383, 152 )
#define AKKUHIBA                ( 384, 78  )
#define MERLEGGRAF              ( 162, 0   )
#define KMORA                   ( 210, 162 )
#define KMORA_MERTEKEGYSEG      ( 207, 226 )
#define FEK                     ( 5, 0     )
#define TIME_H                  ( 436, 71  )
#define TIME_M                  ( 436, 136 )
#define TIME_S                  ( 436, 201 )
#define KPONT1                  ( 436, 98  )
#define KPONT2                  ( 436, 163 )
#define MOTORJEL                ( 114, 2   )
#define MOTORWATTJEL            ( 114, 109 )
#define AKKUSOC                 ( 79, 75   )
#define AKKUPICT                ( 79, 2   )
#define MOTORWATT               ( 114, 84  )
#define AKKUSZAZALEKJEL         ( 80, 90   )
#define TEMPOMATSEBESSEG        ( 24, 35   )
#define TEMPOMATKMH             ( 49, 6    )
#define TEMPOMATPICT            ( 24, 60   )
#define BESUGARZASPICT          ( 110, 234 )
#define BESUGARZASWATTJEL       ( 114, 210 )
#define BESUGARZAS              ( 114, 188 )
#define MERLEGJEL               ( 79, 230  )
#define MERLEGWATTJEL           ( 79, 210  )
#define MERLEGSZAM              ( 79, 188  )
#define DISTANCETIZED           ( 23, 249  )
#define DISTANCEPONT            ( 23, 239  )
#define DISTANCENUM             ( 21, 221  )
#define DISTANCETEXT            ( 47, 208  )
#define GPS                     ( 24, 123  )
#define TOLTES                  ( 164, 90  )

//COORDINATE ADDRESS SYSTEM
#define CONCAT(A,B)              A ## B
#define EXPAND_CONCAT(A,B)       CONCAT(A, B)

#define ARGN(N, LIST)            EXPAND_CONCAT(ARG_, N) LIST
#define ARG_0(A0, ...)           A0
#define ARG_1(A0, A1, ...)       A1

#define X(NAME)                  ARGN(0, NAME)
#define Y(NAME)                  ARGN(1, NAME)

/*
 * Function declarations.
 */
void dspInit();
void LoadImages();
uint8_t dspGetValue(void);
void cmd_dspvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void DeleteOldValues();
void DisplayOnOff(bool_t OnOrOff);
void ReDrawLcd();


#define LCD_ON              TRUE
#define LCD_OFF             FALSE
#define ANIMATION_TIME      37      //*15ms


enum dspMessages {
  DSP_BALINDEX,
  DSP_JOBBINDEX,
  DSP_TIME_H,
  DSP_TIME_M,
  DSP_TIME_S,
  DSP_TIREPRESSURE,
  DSP_AKKUHIBA,
  DSP_HIBA,
  DSP_LIGHT,
  DSP_GPS,
  DSP_MERLEGGRAF,
  DSP_KMORA,
  DSP_MOTORPOWER,
  DSP_AKKUSOC,
  DSP_SUNPOWER,
  DSP_MERLEGWATT,
  DSP_MERLEGJEL,
  DSP_TEMPOMAT,
  DSP_TEMPOMATJEL,
  DSP_FEK,
  DSP_DISTANCETIZED,
  DSP_DISTANCE,
  DSP_TOLTES,
  DSP_NUM_MSG
} ;

#define AKKUHIBA_ON_SOC             20
#define REGENBRAKE_RESOLUTION       0.036       // (321 / 9000)
#define MERLEGGRAF_ZOLD_DIVIDE      0.0095      // watt  31 / 3000
#define MERLEGGRAF_PIROS_DIVIDE     0.008       // watt  31 / 4000

#define TIRE_LEAK_DETECTED_MASK     0x0C
#define TIRE_UNDER_PRESSURE         0xE0
#define TIRE_SENSOR_CONNECTED       0x03

//TEMPORARLY APIs for developing
/*
#define getLightFlashing(n) 0
#define pack_soc 20
#define cruiseStatus() 1
#define speedGetSpeed() 43
#define TIMESTAMP 1468361262
#define calcGetValue(CALC_MOTOR_POWER) 876
#define calcGetValue(CALC_SUN_POWER) 467
#define cruiseGet() 89
#define GetMeterCounter() 365
#define GetKmeterDistance() 1435
#define measGetValue_2(MEAS2_REGEN_BRAKE) 1000*/


#endif
