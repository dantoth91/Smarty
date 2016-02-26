#include "nmea.h"

#include "chprintf.h"

#include <string.h>
#include <stdlib.h>
/*
#include <math.h>
*/

/* Nmea thread memory */
#define NMEA_WA_SIZE    THD_WA_SIZE(1024)

static nmeaPosition_t nmeaCurPos;
static uint32_t nmeaGpggaCount;

static uint8_t longitude_direction;
static uint8_t latitude_direction;

static struct nmeaSentence_t{
  uint8_t Sentence[83];
  uint8_t CharCount;
  uint8_t CheckSum;
  uint8_t ReceivedCheckSum;
} nmeaSentence;

static enum nmeaStates_t{
  NMEA_SOS,
  NMEA_SENTENCE,
  NMEA_CHECKSUM1,
  NMEA_CHECKSUM2,
  NMEA_EOS1,
  NMEA_EOS2
} nmeaState;

static Mutex nmea_mutex;

/* Serial configuration used for SD3 */
static SerialConfig nmea_ser_SD3_cfg = {
    19200,
    0,
    0,
    0,
};

void nmeaReadByte(uint8_t b);
static msg_t nmeaThread(void *arg);
void nmeaInitState(void);

void nmeaInit(void){
  int i;
  nmeaInitState;
  nmeaCurPos.Lon = 0.0;
  nmeaCurPos.Lat = 0.0;
  nmeaCurPos.Alt = 0.0;
  nmeaCurPos.Vel = 0.0;
  nmeaCurPos.Hdg = 0.0;
  nmeaCurPos.Svs = 0;
  nmeaCurPos.Temp = 0;
  nmeaCurPos.Tof = 0;
  nmeaCurPos.Upd = 0;
  for(i=0;i<10;i++) nmeaCurPos.Tof2[i] = 0;
  nmeaGpggaCount = 0;

  chMtxInit(&nmea_mutex);
  sdStart(&SD3, &nmea_ser_SD3_cfg);
  static WORKING_AREA(waNmea, NMEA_WA_SIZE);
  chThdCreateStatic(waNmea, sizeof(waNmea), NORMALPRIO, nmeaThread, NULL);
}

static msg_t nmeaThread(void *arg) {

  (void)arg;
  chRegSetThreadName("nmeaThread");
  while (TRUE) {
    uint8_t c;
  
    chSequentialStreamRead(&SD3, &c, 1);
    nmeaReadByte(c);
  }
  return 0;
}

void nmeaInitState(void){
  nmeaState = NMEA_SOS;
  nmeaSentence.CharCount = 0;
  nmeaSentence.CheckSum = 0;
  nmeaSentence.ReceivedCheckSum = 0;
}

void nmeaProcessSentence(){
  uint8_t s[64];
  uint8_t *t, *u;
  double deg, min;
  uint8_t i;
  struct tm tt;
  uint8_t str[10];

//  esciATransmitString(nmeaSentence.Sentence);
//  esciATransmitString("\r\n");
  t = nmeaSentence.Sentence;
  u = t;
  while(*u != ',' && *u != 0)u++;
  if(*u == ',')*u++ = 0;
  
  // GPRMC
  if(!strcmp(t, "GPRMC")){
    t = u;                                          // Time of fix
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    if(*t == 0)return;
    strncpy(nmeaCurPos.Tof2, t, 6);

    strncpy(str, (uint8_t *)nmeaCurPos.Tof2, 2);
    nmeaCurPos.Tof = atoi(str) * 3600;
    strncpy(str, (uint8_t *)nmeaCurPos.Tof2 + 2, 2);
    nmeaCurPos.Tof += atoi(str) * 60;
    strncpy(str, (uint8_t *)nmeaCurPos.Tof2 + 4, 2);
    nmeaCurPos.Tof += atoi(str);
    
    tt.tm_hour = atoi(strncpy(s, t, 2));
    t += 2;                
    tt.tm_min = atoi(strncpy(s, t, 2));
    t += 2;                
    tt.tm_sec = atoi(strncpy(s, t, 2));
    
    t = u;                                          // Status
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;

    t = u;                                          // Latitude
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    nmeaCurPos.Lat = atof(t);
    deg = (double)((uint16_t)nmeaCurPos.Lat / 100);
    min = nmeaCurPos.Lat - deg * 100.0;
    nmeaCurPos.Lat = (deg + min / 60) * NMEA_RAD;

    t = u;                                          // Latitude direction
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    if(*t == 'S'){
      nmeaCurPos.Lat = -nmeaCurPos.Lat;
      latitude_direction = 1;
    }

    t = u;                                          // Longitude
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    nmeaCurPos.Lon = atof(t);
    deg = (double)((uint16_t)nmeaCurPos.Lon / 100);
    min = nmeaCurPos.Lon - deg * 100.0;
    nmeaCurPos.Lon = (deg + min / 60) * NMEA_RAD;
    
    t = u;                                          // Longitude direction
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    if(*t == 'W'){
      nmeaCurPos.Lon = -nmeaCurPos.Lon;
      longitude_direction = 1;
    }
    
    t = u;                                          // Speed over ground (knots)
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    nmeaCurPos.Vel = 1.852 * atof(t); // in kmph
    
    t = u;                                          // Course over ground
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    nmeaCurPos.Hdg = atof(t);
    
    t = u;                                          // Date of fix
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    tt.tm_mday = atoi(strncpy(s, t, 2));
    t += 2;                
    tt.tm_mon = atoi(strncpy(s, t, 2)) - 1;
    t += 2;                
    tt.tm_year = 100 + atoi(strncpy(s, t, 2));
    nmeaCurPos.Tof = mktime(&tt);
    
    t = u;                                          // Magnetic variation
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Magnetic variation direction
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Mode indicator
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;

//    nmeaCurPos.Upd = timerGetRTC();
    return;
  }
  
  // GPGGA
  if(!strcmp(t, "GPGGA")){
    nmeaGpggaCount++;
    t = u;                                          // Time of fix
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    if(*t == 0)return; 
    //nmeaCurPos.Tof = atof(t);                
    t = u;                                          // Latitude
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    //nmeaCurPos.Lat = atof(t);
    //deg = (double)((uint16_t)nmeaCurPos.Lat / 100);
    //min = nmeaCurPos.Lat - deg * 100.0;
    //nmeaCurPos.Lat = (deg + min / 60) * NMEA_RAD;
    t = u;                                          // Latitude direction
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    //if(*t == 'S')nmeaCurPos.Lat = -nmeaCurPos.Lat;
    t = u;                                          // Longitude
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    //nmeaCurPos.Lon = atof(t);
    //deg = (double)((uint16_t)nmeaCurPos.Lon / 100);
    //min = nmeaCurPos.Lon - deg * 100.0;
    //nmeaCurPos.Lon = (deg + min / 60) * NMEA_RAD;
    t = u;                                          // Longitude direction
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    //if(*t == 'W')nmeaCurPos.Lon = -nmeaCurPos.Lat;
    t = u;                                          // Quality
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
//    esciATransmitString(t);
//    esciATransmitString(" <--quality \r\n");
    t = u;                                          // Svs
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
//    esciATransmitString(t);
//    esciATransmitString(" <--svs \r\n");
    nmeaCurPos.Svs = atof(t);
    t = u;                                          // Horizontal dilutation of precision
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Altitude
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    nmeaCurPos.Alt = atof(t);
    return;
  }
  
  // GPVTG
  if(!strcmp(t, "GPVTG")){
    t = u;                                          // True Heading
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    if(*t == 0)return; 
    //nmeaCurPos.Hdg = atof(t);                
    t = u;                                          // "T"
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Magnetic Heading
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // "M"
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Speed over ground Knots
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // "N"
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Speed over ground Km/h
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    //nmeaCurPos.Vel = atof(t);
    return;
  }

  // PGRMT
  if(!strcmp(t, "PGRMT")){
    t = u;                                          // Product Model #
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    if(*t == 0)return; 
    t = u;                                          // ROM Checksum
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Rec failure
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Stored data lost
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // RTC lost
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Oscillator drift discrete
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Data collection discrete
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    t = u;                                          // Sensor temperature
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    nmeaCurPos.Temp = (int8_t)atof(t);
    t = u;                                          // sensor configuration
    while(*u != ',' && *u != 0)u++;
    if(*u == ',')*u++ = 0;
    return;
  }
}

void nmeaReadByte(uint8_t b){

  switch(nmeaState){
    case NMEA_SOS :
      if(b == '$'){
        nmeaState = NMEA_SENTENCE;
      }
      break;
    case NMEA_SENTENCE :
      if(b == '*')
        nmeaState = NMEA_CHECKSUM1;
      else if(b == 0x0D)
        nmeaState = NMEA_EOS2;
      else{
        nmeaSentence.Sentence[nmeaSentence.CharCount++] = b;
        nmeaSentence.CheckSum ^= b;
      }
      break;
    case NMEA_CHECKSUM1 :
      if(b >= '0' && b<= '9'){ 
        nmeaSentence.ReceivedCheckSum = 16 * (b - '0');
        nmeaState = NMEA_CHECKSUM2;
      }
      else if(b >= 'A' && b <= 'F'){
        nmeaSentence.ReceivedCheckSum = 16 * ((b - 'A') + 10);
        nmeaState = NMEA_CHECKSUM2;
      }
      else
        nmeaInitState();
      break;  
    case NMEA_CHECKSUM2 :
      if(b >= '0' && b<= '9'){ 
        nmeaSentence.ReceivedCheckSum += (b - '0');
        if(nmeaSentence.ReceivedCheckSum == nmeaSentence.CheckSum)
          nmeaState = NMEA_EOS1;
        else 
          nmeaInitState();
      }
      else if(b >= 'A' && b <= 'F'){
        nmeaSentence.ReceivedCheckSum += (b - 'A') + 10;
        if(nmeaSentence.ReceivedCheckSum == nmeaSentence.CheckSum)
          nmeaState = NMEA_EOS1;
        else
          nmeaInitState();
      }
      else
        nmeaInitState();
      break;  
    case NMEA_EOS1 :
      if(b == 0x0D)
        nmeaState = NMEA_EOS2;
      else
        nmeaInitState();
      break;
    case NMEA_EOS2 :
      if(b == 0x0A){
        nmeaSentence.Sentence[nmeaSentence.CharCount] = 0;
        chMtxLock(&nmea_mutex);
        nmeaProcessSentence();
        chMtxUnlock();
        nmeaInitState();
      }
      else
        nmeaInitState();
      break;
  }
}

void nmeaGetCurrentPosition(nmeaPosition_t *d){
  int i;
  chMtxLock(&nmea_mutex);
  d->Lon = nmeaCurPos.Lon;
  d->Lat = nmeaCurPos.Lat;
  d->Alt = nmeaCurPos.Alt;
  d->Vel = nmeaCurPos.Vel;
  d->Hdg = nmeaCurPos.Hdg;
  d->Svs = nmeaCurPos.Svs;
  d->Temp = nmeaCurPos.Temp;
  d->Tof = nmeaCurPos.Tof;
  for(i=0;i<10;i++) d->Tof2[i] = nmeaCurPos.Tof2[i];
  d->Upd = nmeaCurPos.Upd;
  chMtxUnlock();
}

/*
double nmeaCalculateDistance(nmeaPosition_t *p1, nmeaPosition_t *p2){
  double R = 6367.0;
  double lat1, lon1, lat2, lon2, dlon, dlat, a, c;

  lon1 = p1->Lon;
  lat1 = p1->Lat;
  lon2 = p2->Lon;
  lat2 = p2->Lat;
  dlon = lon2 - lon1 ; // difference in longitude 
  dlat = lat2 - lat1; // difference in latitude 
  
  a = (sin(dlat/2.0)* sin(dlat/2.0)) + (cos(lat1) * cos(lat2) * sin(dlon/2.0) * sin(dlon/2.0)); 
  
  c = 2.0 * asin(sqrt(a)); 
  
  return (double)(R * c); // Final distance is in km. 
}
*/

uint32_t nmeaGetGPGGACount(void){
  uint32_t temp;
  
  chMtxLock(&nmea_mutex);
  temp = nmeaGpggaCount;
  chMtxUnlock();
  return temp;
}

void cmd_nmeatest(BaseSequentialStream *chp, int argc, char *argv[]) {
  nmeaPosition_t pos;
  int i;

  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    nmeaGetCurrentPosition(&pos);
    chprintf(chp, "\x1B[%d;%dH", 0, 0);
    chprintf(chp, "Lat: %12D\r\n", (int32_t)(pos.Lat * 100000000.0));
    chprintf(chp, "direct: %12s\r\n", latitude_direction == 1 ? "W" : "E");
    chprintf(chp, "Lon: %12D\r\n", (int32_t)(pos.Lon * 100000000.0));
    chprintf(chp, "direct: %12s\r\n", longitude_direction == 1 ? "S" : "N");
    chprintf(chp, "Alt: %12D\r\n", (int32_t)(pos.Alt));
    chprintf(chp, "Vel: %12d\r\n", (int16_t)(pos.Vel * 100.0));
    chprintf(chp, "Hdg: %12d\r\n", (int8_t)(pos.Hdg * 0.5));
    chprintf(chp, "Svs: %12d\r\n", pos.Svs);
    chprintf(chp, "Temp: %12d\r\n", pos.Temp);
    chprintf(chp, "Tof: %12d\r\n", pos.Tof);
    chprintf(chp, "GPGGA #: %12d\r\n", nmeaGetGPGGACount());
    
    chThdSleepMilliseconds(1000);
  }
}
