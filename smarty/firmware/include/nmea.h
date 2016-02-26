#ifndef NMEA_H_INCLUDED
#define NMEA_H_INCLUDED

#include "ch.h"
#include "hal.h"

#include <time.h>

#define NMEA_RAD 1.745329e-2          // pi/180

typedef struct {
  time_t Tof;
  double Lon;
  double Lat;
  double Alt;
  double Vel;
  double Hdg;
  uint8_t Svs;
  int8_t Temp;
  time_t Upd;
  uint8_t Tof2[10];
} nmeaPosition_t;

void nmeaInit(void);
void nmeaGetCurrentPosition(nmeaPosition_t *d);
/*
double nmeaCalculateDistance(nmeaPosition_t *p1, nmeaPosition_t *p2);
*/
uint32_t nmeaGetGPGGACount(void);

void cmd_nmeatest(BaseSequentialStream *chp, int argc, char *argv[]);

#endif

