/*
 Smarty - Copyright (C) 2015
 GAMF ECOMarathon Team
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

#include "chprintf.h"

/* Display answer message */
#define DIAG_DSP_OK (0x06)

//Color
#define RED                       (0xF800)
#define BLUE                      (0x001F)
#define BLACK                     (0x0000)
#define WHITE                     (0xFFFF)

enum dspState {
  DSP_WAITING,
  DSP_RUNNING
} dspstate;

enum dspMessages {

  DSP_x100_SPEED,
  DSP_x10_SPEED,
  DSP_x1_SPEED,
  DSP_GAUGE,
  DSP_x100_AVG_SPEED,
  DSP_x10_AVG_SPEED,
  DSP_x1_AVG_SPEED,
  DSP_x100_TEMP_SPEED,
  DSP_x10_TEMP_SPEED,
  DSP_x1_TEMP_SPEED,
  DSP_x100_BATTERY,
  DSP_x10_BATTERY,
  DSP_x1_BATTERY,
  DSP_x10_BATTERY_TEMP,
  DSP_x1_BATTERY_TEMP,
  DSP_x1000_SUN_POWER,
  DSP_x100_SUN_POWER,
  DSP_x10_SUN_POWER,
  DSP_x1_SUN_POWER,
  DSP_x1000_MOTOR_POWER,
  DSP_x100_MOTOR_POWER,
  DSP_x10_MOTOR_POWER,
  DSP_x1_MOTOR_POWER,
  DSP_REGEN_BRAKE,
  DSP_INDEX_RIGHT,
  DSP_INDEX_LEFT,
  DSP_TEMPOMAT,
  DSP_LIGHT,
  DSP_OVER_CHARGE,
  DSP_DISCHARGE,
  DSP_MOTOR_TEMO,
  DSP_DANGER,
  DSP_INDICATOR,
  DSP_NUM_MSG
} dspmessages;

static int32_t dspValue[DSP_NUM_MSG + 2] = {-1};
static int tasknumber;
static uint8_t bus_status;
bool_t dsp_activ;
uint8_t c;
static bool_t bus_bit[8];
static uint16_t danger, motor_temp, discharge, over_charge;

int db = 127;
int db2 = 0;

void dsp_LoadTop(void) {

  media_SetSector(0, 57); // indexbal icon
  media_VideoFrame(434, 8, 0);

  media_SetSector(0, 17742); // tempomat icon
  media_VideoFrame(434, 58, 0);

  media_SetSector(0, 110); //lĂˇmpa icon
  media_VideoFrame(434, 183, 0);

  media_SetSector(0, 68); //indexjobb icon
  media_VideoFrame(434, 231, 0);

  media_SetSector(0, 17761); //tĂşl fesz icon
  media_VideoFrame(375, 26, 0);

  media_SetSector(0, 23); //figy icon
  media_VideoFrame(375, 80, 0);

  media_SetSector(0, 121); //motor veszely icon
  media_VideoFrame(375, 170, 0);

  media_SetSector(0, 0); //alfesz icon
  media_VideoFrame(375, 219, 0);

  media_SetSector(0, 17); //Ă�tlag icon
  media_Image(340, 4);

  media_SetSector(0, 17753); //Tempomat szĂ¶veg
  media_Image(340, 166);

}
void dsp_LoadMidle(void) {

  gfx_Transparency(1);
  gfx_TransparentColour(BLUE);
  media_SetSector(0, 138); // mĂĽszer icon
  media_VideoFrame(148, 21, 0);

  media_SetSector(0, 106); // km/h szĂ¶veg
  media_Image(168, 204);

  media_SetSector(0, 17548); // szam kicsi icon
  media_VideoFrame(315, 244, 0);
  media_SetSector(0, 17548);
  media_VideoFrame(315, 40, 0);

  media_SetSector(0, 17587); // szam nagy icon
  media_VideoFrame(186, 167, 0);

}
void dsp_LoadBottom(void) {

  draw_line(360, 0, 360, 272, WHITE);
  draw_line(145, 0, 145, 272, WHITE);

  media_SetSector(0, 46); //  ikonalsĂł icon
  media_VideoFrame(108, 5, 0);

  media_SetSector(0, 46); //  ikonalsĂł icon
  media_VideoFrame(108, 135, 1);

  media_SetSector(0, 46); //  ikonalsĂł icon
  media_VideoFrame(66, 135, 2);

  media_SetSector(0, 17778); //zHom icon
  media_Image(65, 5);

  media_SetSector(0, 17561); // szamkozep icon
  media_VideoFrame(108, 79, 0);
  media_SetSector(0, 17561); // szamkozep icon
  media_VideoFrame(108, 228, 0);
  media_SetSector(0, 17561); // szamkozep icon
  media_VideoFrame(66, 79, 0);
  media_SetSector(0, 17561); // szamkozep icon
  media_VideoFrame(66, 228, 0);

  media_SetSector(0, 40); //keplet (C.gif)
  media_VideoFrame(102, 105, 2);
  media_SetSector(0, 40); //keplet (C.gif)
  media_VideoFrame(65, 105, 1);
  media_SetSector(0, 40); //keplet (C.gif)
  media_VideoFrame(102, 249, 0);
  media_SetSector(0, 40); //keplet (C.gif)
  media_VideoFrame(65, 249, 0);

  media_SetSector(0, 6455); // rotation.gif
  media_VideoFrame(3, 0, 0);

}

void dsp_RefreshMidle(int frame) {

  gfx_Transparency(1);
  gfx_TransparentColour(BLUE);
  media_SetSector(0, 138); // mĂĽszer icon
  media_VideoFrame(148, 21, frame / 5);

  media_SetSector(0, 106); // km/h szĂ¶veg
  media_Image(168, 204);

  media_SetSector(0, 17548); // szam kicsi icon
  media_VideoFrame(315, 244, 0);
  media_SetSector(0, 17548);
  media_VideoFrame(315, 33, 0);

  if (frame / 100 != 0) {
    media_SetSector(0, 17587); // szam nagy icon
    media_VideoFrame(186, 67, frame / 100);
  }
  else {
    media_SetSector(0, 17587); // szam nagy icon
    media_VideoFrame(186, 67, 10);
  }
  media_SetSector(0, 17587); // szam nagy icon
  media_VideoFrame(186, 117, (frame / 10) % 10);
  media_SetSector(0, 17587); // szam nagy icon
  media_VideoFrame(186, 167, frame % 10);
}

static uint32_t bus_num;
static uint8_t valasz;
bool_t cruise_assis = FALSE;

/*
 * Display Task
 */
static WORKING_AREA(wadspTask, 1024);
static msg_t dspTask(void *arg) {
  systime_t time; 

  (void)arg;
  chRegSetThreadName("dspTask");
  time = chTimeNow();  
  while (TRUE) {
    int32_t new_val;
    uint32_t regen_assis;

    bus_num++;

    if ((bus_num % 2) == 1)
    {
      bus_status = bus_Read();
    }

    else if((bus_num % 2) == 0)
    {

      tasknumber++;
      switch (dspstate) {
        case DSP_WAITING:
        valasz = gfxGetDspAns();
          if(valasz == DIAG_DSP_OK || tasknumber > 7){
            dspstate = DSP_RUNNING;
          }
          if(gfxGetDspAns() == DIAG_DSP_OK){
              dsp_activ = TRUE;
          }
          break;
        case DSP_RUNNING:
          switch (dspmessages) {

            case DSP_x100_SPEED:
              new_val = (int16_t)(speedGetSpeed() / 100);
              new_val = new_val == 0 ? 10 : new_val;
              
              //new_val = 0;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17587); // szam nagy icon
                gfxGetDspAns();
                media_VideoFrame(186, 67, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                dspmessages++;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_SPEED:
              new_val = (uint16_t)(speedGetSpeed() / 10);
              new_val = new_val % 10;
              

              if (dspValue[DSP_x100_SPEED] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              //new_val = 0;

              if (dspValue[dspmessages] != new_val) {

                media_SetSector(0, 17587); // szam nagy icon
                gfxGetDspAns();
                media_VideoFrame(186, 117, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                dspmessages++;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_SPEED:
              new_val = (uint16_t)(speedGetSpeed() % 10);
              
              //new_val = 0;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17587); // szam nagy icon
                gfxGetDspAns();
                media_VideoFrame(186, 167, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                dspmessages++;
                break;
              }
              else
                dspmessages++;
              
            case DSP_GAUGE:
              new_val = speedGetSpeed() / 5;
              
              //new_val = 20;
              
              if (dspValue[dspmessages] != new_val) {
                gfx_Transparency(1);
                gfxGetDspAns();
                gfx_TransparentColour(BLUE);
                gfxGetDspAns();
                media_SetSector(0, 138); // mĂĽszer icon
                gfxGetDspAns();
                media_VideoFrame(148, 21, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x100_AVG_SPEED:
              new_val = calcAvgSpeed();
              new_val /= 1000;

              new_val = new_val == 0 ? 10 : new_val;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17548);
                gfxGetDspAns();
                media_VideoFrame(315, 0, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_AVG_SPEED:
              new_val = calcAvgSpeed();
              new_val /= 100;
              new_val = new_val % 10;

              if (dspValue[DSP_x100_AVG_SPEED] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17548);
                gfxGetDspAns();
                media_VideoFrame(315, 20, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_AVG_SPEED:
              new_val = calcAvgSpeed();
              new_val /= 10;
              new_val = new_val % 10;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17548);
                gfxGetDspAns();
                media_VideoFrame(315, 40, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x100_TEMP_SPEED:
              new_val = (uint16_t)(cruiseGet() / 100);
              new_val = new_val == 0 ? 10 : new_val;
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17548);
                gfxGetDspAns();
                media_VideoFrame(315, 205, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_TEMP_SPEED:
              new_val = (uint16_t)(cruiseGet() / 10);
              new_val = new_val % 10;

              if (dspValue[DSP_x100_TEMP_SPEED] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17548);
                gfxGetDspAns();
                media_VideoFrame(315, 225, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_TEMP_SPEED:
              new_val = cruiseGet() % 10;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17548);
                gfxGetDspAns();
                media_VideoFrame(315, 245, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x100_BATTERY:
              new_val = bmsitems.pack_soc;
              if( (new_val / 1000) == 0){
                new_val = 10;
              }
              else{
                new_val = new_val / 1000;
              }
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 39, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_BATTERY:
              new_val = bmsitems.pack_soc / 100;
              new_val = new_val % 10;

              /*if (dspValue[DSP_x100_BATTERY] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }*/

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 59, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_BATTERY:
              new_val = bmsitems.pack_soc / 10;
              new_val = new_val % 10;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 79, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_BATTERY_TEMP:
              new_val = (uint16_t)(bmsitems.average_temp / 10);
              new_val = new_val % 10;

              new_val = new_val == 0 ? 10 : new_val;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(66, 59, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_BATTERY_TEMP:
              new_val = bmsitems.average_temp % 10;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(66, 79, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1000_SUN_POWER:
              new_val = calcGetValue(CALC_SUN_POWER) / 1000;
              
              new_val = new_val == 0 ? 10 : new_val;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 168, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x100_SUN_POWER:
              new_val = calcGetValue(CALC_SUN_POWER) / 100;
              new_val = new_val % 10;

              if (dspValue[DSP_x1000_SUN_POWER] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 188, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_SUN_POWER:
              new_val = calcGetValue(CALC_SUN_POWER) / 10;
              new_val = new_val % 10;

              if (dspValue[DSP_x100_SUN_POWER] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 208, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_SUN_POWER:
              new_val = (calcGetValue(CALC_SUN_POWER) % 10);

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(108, 228, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1000_MOTOR_POWER:
              new_val = calcGetValue(CALC_MOTOR_POWER);
              if (new_val < 0)
                new_val = 0;

              new_val = (uint32_t)(new_val / 1000);
              new_val = new_val == 0 ? 10 : new_val;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(66, 168, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x100_MOTOR_POWER:
              new_val = calcGetValue(CALC_MOTOR_POWER);
              if (new_val < 0)
                new_val = 0;
              new_val /= 100;
              new_val = new_val % 10;

              if (dspValue[DSP_x1000_MOTOR_POWER] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(66, 188, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x10_MOTOR_POWER:
              new_val = calcGetValue(CALC_MOTOR_POWER);
              if (new_val < 0)
                new_val = 0;
              new_val /= 10;
              new_val = new_val % 10;

              if (dspValue[DSP_x100_MOTOR_POWER] == 10)
              {
                new_val = new_val == 0 ? 10 : new_val;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(66, 208, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_x1_MOTOR_POWER:
              new_val = calcGetValue(CALC_MOTOR_POWER) % 10;
              if (new_val < 0)
                new_val = 0;
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17561); // szam icon
                gfxGetDspAns();
                media_VideoFrame(66, 228, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_REGEN_BRAKE:
              regen_assis = measGetValue_2(MEAS2_REGEN_BRAKE);
              regen_assis *= 29;
              regen_assis = (int32_t)(regen_assis /1000);
              regen_assis = regen_assis > 260 ? 260 : regen_assis;
              new_val = regen_assis;

              //new_val = map(measGetValue(MEAS2_REGEN_BRAKE), 0, 9000, 0, 260);
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 6455); // rotation.gif
                gfxGetDspAns();
                media_VideoFrame(3, 0, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_INDEX_RIGHT:
              if (getLightFlashing(2) || getLightFlashing(4)) { new_val = 0; }
              else
                new_val = 1;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 68); //indexjobb icon
                gfxGetDspAns();
                media_VideoFrame(434, 231, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_INDEX_LEFT:
              if (getLightFlashing(3) || getLightFlashing(4)){ new_val = 0; }
              else
                new_val = 1;
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 57); // indexbal icon
                gfxGetDspAns();
                media_VideoFrame(434, 8, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_TEMPOMAT:
              if (cruiseStatus())
              {
                new_val = 0;
              }
              else
                new_val = 1;
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17742); // tempomat icon
                gfxGetDspAns();
                media_VideoFrame(434, 58, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_LIGHT:
              if (getLightFlashing(5))
              {
                new_val = 0;
              }
              else
              {
                new_val = 1;
              }
              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 110); //lĂˇmpa icon
                gfxGetDspAns();
                media_VideoFrame(434, 183, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_OVER_CHARGE:
              if((bmsitems.custom_flag_1 >> 1) & 0x01)
              {
                new_val = 0;
              }
              else
              {
                over_charge ++;
                new_val = over_charge < 10 ? 1 : 2;
                over_charge = over_charge > 20 ? 0 : over_charge;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 17761); //tĂşl fesz icon
                gfxGetDspAns();
                media_VideoFrame(375, 26, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_DISCHARGE:
              if(bmsitems.pack_soc > 10)
              {
                new_val = 0;
              }
              else
              {
                discharge ++;
                new_val = discharge < 10 ? 1 : 2;
                discharge = discharge > 20 ? 0 : discharge;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 0); //alfesz icon
                gfxGetDspAns();
                media_VideoFrame(375, 219, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_MOTOR_TEMO:
              if(measGetValue(MEAS_IS_IN_DRIVE))
              {
                motor_temp ++;
                new_val = motor_temp < 10 ? 1 : 2;
                motor_temp = motor_temp > 20 ? 0 : motor_temp;
              }
              else
              {
                new_val = 0;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 121); //motor veszely icon
                gfxGetDspAns();
                media_VideoFrame(375, 170, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_DANGER:
              if(bmsitems.custom_flag_1 & 0x01)
              {
                //DEACTIVE
                new_val = 0;
                danger = 0;
              }
              else
              {
                //ACTIVE
                danger ++;
                new_val = danger < 10 ? 1 : 2;
                danger = danger > 20 ? 0 : danger;
              }

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 23); //figy icon
                gfxGetDspAns();
                media_VideoFrame(375, 80, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
                break;
              }
              else
                dspmessages++;

            case DSP_INDICATOR:
              new_val = 0;

              if (dspValue[dspmessages] != new_val) {
                media_SetSector(0, 79); //figy icon
                gfxGetDspAns();
                media_VideoFrame(435, 118, new_val);
                dspstate = DSP_WAITING;
                dspValue[dspmessages] = new_val;
              }
              break;

            default:
              break;
          }
          dspmessages++;
          tasknumber = 0;
          gfxGetDspAnsZero();
          if (dspmessages > DSP_NUM_MSG) {
            dspmessages = 0;
          }
          break;
        default:
          dspstate = DSP_RUNNING;
          break;
      }
    }

    //chThdSleepMilliseconds(15);
  }
  return 0;
}


void dspInit(void) {

  gfx_Cls();
  file_Mount();
  dsp_LoadTop();
  dsp_LoadMidle();
  dsp_LoadBottom();
  bus_In();
  dsp_activ = FALSE;
  dspmessages = DSP_x100_SPEED;
  dspstate = DSP_WAITING;
  bus_num = 0;
  chThdCreateStatic(wadspTask, sizeof(wadspTask), NORMALPRIO + 7, dspTask, NULL);
}

bool_t dspGetItem(uint8_t ch){
    ch = ch < 1 ? 1 : ch;
    ch = ch > 8 ? 8 : ch;
    return bus_bit[ch - 1];
}

uint16_t dspGetValue(void){
    return bus_status;
}

bool_t dsp_active(void){
    return dsp_activ;
}

void cmd_dspvalues(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  int i;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);
    chprintf(chp,"bus_status: %x\r\n",bus_status);
    chprintf(chp,"bus_bit[]: ");
    for (i = 0; i < 8; ++i)
    {
      chprintf(chp,"%d ", bus_bit[i]);
    }
    chprintf(chp,"\r\n");
    chprintf(chp,"dspValues[]: ");
    for (i = 0; i < DSP_NUM_MSG; ++i)
    {
      chprintf(chp,"%d ", dspValue[i]);
    }
    chprintf(chp,"\r\n");
    chprintf(chp,"dspmessages: %d\r\n",dspmessages);
    chprintf(chp,"bus_num: %d\r\n",bus_num);
    chprintf(chp,"BUT7: %d\r\n", palReadPad(GPIOB, GPIOB_BUT7));
    chprintf(chp,"valasz: %d\r\n", valasz);

    chThdSleepMilliseconds(150);
  }
}

void cmdfrappans_dspmessages(BaseSequentialStream *chp, int argc, char *argv[]) {

(void)argc;
(void)argv;
chprintf(chp, "\x1B\x63");
chprintf(chp, "\x1B[2J");
while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
  chprintf(chp, "\x1B[%d;%dH", 0, 0);

  if ((argc == 1) && (strcmp(argv[0], "Clear") == 0)) {

    chprintf(chp, "-------------- dps clear --------------\r\n");
    gfx_Cls();
    return;
  }

  if ((argc == 1) && (strcmp(argv[0], "Sector") == 0)) {

    chprintf(chp, "--------------Set sector --------------\r\n");
    media_SetSector(0, 6593);

    return;
  }

  if ((argc == 1) && (strcmp(argv[0], "Image") == 0)) {

    chprintf(chp, "-------------- media Image --------------\r\n");

    media_SetSector(0, 6593);
    media_Image(0, 0);
    return;
  }

  if ((argc == 1) && (strcmp(argv[0], "Background") == 0)) {

    chprintf(chp, "-------------- Background --------------\r\n");
    gfx_BGcolour(RED);
    return;
  }
  if ((argc == 2) && (strcmp(argv[0], "Speed") == 0)) {

    chprintf(chp, "-------------- Background --------------\r\n");
    db=atol(argv[1]);
    return;
  }

  else {
    goto ERROR;
  }
}

ERROR: chprintf(chp, "Usage: Clear\r\n");
chprintf(chp, "       Sector\r\n");
chprintf(chp, "       Image\r\n");
chprintf(chp, "       Background\r\n");
return;
chThdSleepMilliseconds(1000);

}
