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
  DSP_x10_AVG_SPEED,
  DSP_x1_AVG_SPEED,
  DSP_x01_AVG_SPEED,
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
  DSP_INDEX_RIGHT,
  DSP_INDEX_LEFT,
  DSP_TEMPOMAT,
  DSP_LIGHT,
  DSP_OVER_CHARGE,
  DSP_DISCHARGE,
  DSP_MOTOR_TEMO,
  DSP_DANGER,
  DSP_NUM_MSG
} dspmessages;

static int32_t dspValue[DSP_NUM_MSG + 2] = {-1};
static int tasknumber;
static uint8_t bus_status;
bool_t dsp_activ;
uint8_t c;
static bool_t bus_bit[8];

int db = 127;
int db2 = 0;

void dsp_LoadTop(void) {

  media_SetSector(0, 39); // indexbal icon
  media_VideoFrame(433, 7, 0);

  media_SetSector(0, 6604); // tempomat icon
  media_VideoFrame(433, 61, 0);

  media_SetSector(0, 65); //lámpa icon
  media_VideoFrame(433, 154, 0);

  media_SetSector(0, 50); //indexjobb icon
  media_VideoFrame(433, 230, 0);

  media_SetSector(0, 6623); //túl fesz icon
  media_VideoFrame(375, 26, 0);

  media_SetSector(0, 11); //figy icon
  media_VideoFrame(375, 80, 0);

  media_SetSector(0, 76); //motor veszely icon
  media_VideoFrame(375, 170, 0);

  media_SetSector(0, 0); //alfesz icon
  media_VideoFrame(375, 219, 0);

  media_SetSector(0, 6404); //Átlag icon
  media_Image(340, 4);

  media_SetSector(0, 6615); //Tempomat szöveg
  media_Image(340, 166);

}
void dsp_LoadMidle(void) {

  gfx_Transparency(1);
  gfx_TransparentColour(BLUE);
  media_SetSector(0, 87); // müszer icon
  media_VideoFrame(148, 21, 0);

  media_SetSector(0, 61); // km/h szöveg
  media_Image(168, 204);

  media_SetSector(0, 6410); // szam kicsi icon
  media_VideoFrame(315, 244, 0);
  media_SetSector(0, 6410);
  media_VideoFrame(315, 40, 0);

  media_SetSector(0, 6449); // szam nagy icon
  media_VideoFrame(186, 155, 0);

}
void dsp_LoadBottom(void) {

  draw_line(360, 0, 360, 272, WHITE);
  draw_line(145, 0, 145, 272, WHITE);

  media_SetSector(0, 28); //  icon
  media_VideoFrame(108, 5, 0);

  media_SetSector(0, 28); //  icon
  media_VideoFrame(108, 135, 1);

  media_SetSector(0, 28); //  icon
  media_VideoFrame(66, 135, 2);

  media_SetSector(0, 6634);
  media_Image(65, 5);

  media_SetSector(0, 6423); // szam icon
  media_VideoFrame(108, 79, 0);
  media_SetSector(0, 6423);
  media_VideoFrame(108, 228, 0);
  media_SetSector(0, 6423);
  media_VideoFrame(66, 79, 0);
  media_SetSector(0, 6423);
  media_VideoFrame(66, 228, 0);

  media_SetSector(0, 22); //keplet
  media_VideoFrame(102, 105, 2);
  media_SetSector(0, 22);
  media_VideoFrame(65, 105, 1);
  media_SetSector(0, 22);
  media_VideoFrame(102, 249, 0);
  media_SetSector(0, 22);
  media_VideoFrame(65, 249, 0);

}

void dsp_RefreshMidle(int frame) {

  gfx_Transparency(1);
  gfx_TransparentColour(BLUE);
  media_SetSector(0, 87); // müszer icon
  media_VideoFrame(148, 21, frame / 5);

  media_SetSector(0, 61); // km/h szöveg
  media_Image(168, 204);

  media_SetSector(0, 6410); // szam kicsi icon
  media_VideoFrame(315, 244, 0);
  media_SetSector(0, 6410);
  media_VideoFrame(315, 33, 0);

  if (frame / 100 != 0) {
    media_SetSector(0, 6449); // szam nagy icon
    media_VideoFrame(186, 62, frame / 100);
  }
  else {
    media_SetSector(0, 6449); // szam nagy icon
    media_VideoFrame(186, 62, 10);
  }
  media_SetSector(0, 6449); // szam nagy icon
  media_VideoFrame(186, 112, (frame / 10) % 10);
  media_SetSector(0, 6449); // szam nagy icon
  media_VideoFrame(186, 162, frame % 10);
}

int szamlalo;
static uint32_t bus_read = 0;
bool_t cruise_assis = FALSE;

/*
 * Display Task
 */
static WORKING_AREA(wadspTask, 256);
static msg_t dspTask(void *arg) {
  systime_t time; 

  (void)arg;
  chRegSetThreadName("dspTask");
  time = chTimeNow();  
  while (TRUE) {
    int new_val;
    int i;

    bus_read++;

    if ((bus_read % 2) == 1)
    {
      //bus_read = 0;
      bus_status = 0;
      bus_status = bus_Read();
      if (bus_status > 0x06)
      {
        for (i = 0; i < 8; i++)
        {
          if ((bus_status >> i) & 0x01)
          {
            bus_bit[i] = 1;
          }
          else{
            bus_bit[i] = 0;
          }
        }
      }
    }

    else if((bus_read % 4) == 2)
    {
      szamlalo++;
      szamlalo = szamlalo > 999 ? 0 : szamlalo;

      tasknumber++;

      switch (dspmessages) {

        case DSP_x100_SPEED:
          if(speedGetSpeed() > 0 && speedGetSpeed() < 999){
            new_val = (int16_t)(speedGetSpeed()/100);
          }
          else
            new_val = 0;
          
          if (dspValue[dspmessages] != new_val) {
            if (new_val != 0) {
              media_SetSector(0, 6449); // szam nagy icon
              media_VideoFrame(186, 62, new_val);
            }
            else {
              media_SetSector(0, 6449); // szam nagy icon
              media_VideoFrame(186, 62, 10);
            }
            //dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            dspmessages++;
            break;
          }
          else
            dspmessages++;

        case DSP_x10_SPEED:
          if(speedGetSpeed() > 0 && speedGetSpeed() < 999){
            new_val = (int)(speedGetSpeed()/100);
          }
          else
            new_val = 0;

          new_val = (int)((speedGetSpeed()/10)%10);
          if (dspValue[dspmessages] != new_val) {

            media_SetSector(0, 6449); // szam nagy icon
            media_VideoFrame(186, 112, new_val);
            //dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            dspmessages++;
            break;
          }
          else
            dspmessages++;

        case DSP_x1_SPEED:
          if(speedGetSpeed() > 0 && speedGetSpeed() < 999){
              new_val = (int)(speedGetSpeed()/100);
          }
          else
            new_val = 0;

          new_val = (int)(speedGetSpeed()%10);
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6449); // szam nagy icon
            media_VideoFrame(186, 162, new_val);
            //dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            dspmessages++;
            break;
          }
          else
            dspmessages++;
          
        case DSP_GAUGE:
          new_val = speedGetSpeed();
          if (dspValue[dspmessages] != new_val) {
            gfx_Transparency(1);
            gfx_TransparentColour(BLUE);
            media_SetSector(0, 87); // müszer icon
            media_VideoFrame(148, 21, new_val / 5);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x10_AVG_SPEED:
          new_val = 995;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6410);
            media_VideoFrame(315, 0, new_val/100);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1_AVG_SPEED:
          new_val = 995;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6410);
            media_VideoFrame(315, 20, (new_val / 10) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x01_AVG_SPEED:
          new_val = 995;
            if (dspValue[dspmessages] != new_val) {
              media_SetSector(0, 6410);
              media_VideoFrame(315, 40, new_val%10);
            //  dspstate = DSP_WAITING;
              dspValue[dspmessages] = new_val;
              break;
            }
            else
              dspmessages++;

        case DSP_x100_TEMP_SPEED:
          new_val = cruiseGet();
            if (dspValue[dspmessages] != new_val) {
              media_SetSector(0, 6410);
              media_VideoFrame(315, 205, new_val/100);
            //  dspstate = DSP_WAITING;
              dspValue[dspmessages] = new_val;
              break;
            }
            else
              dspmessages++;

        case DSP_x10_TEMP_SPEED:
          new_val = cruiseGet();
            if (dspValue[dspmessages] != new_val) {
              media_SetSector(0, 6410);
              media_VideoFrame(315, 225, (new_val / 10) % 10);
            //  dspstate = DSP_WAITING;
              dspValue[dspmessages] = new_val;
              break;
            }
            else
              dspmessages++;

        case DSP_x1_TEMP_SPEED:
          new_val = cruiseGet();
            if (dspValue[dspmessages] != new_val) {
              media_SetSector(0, 6410);
              media_VideoFrame(315, 245, new_val  % 10);
            //  dspstate = DSP_WAITING;
              dspValue[dspmessages] = new_val;
              break;
            }
            else
              dspmessages++;

        case DSP_x100_BATTERY:
          new_val = 100;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 39, new_val / 100);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x10_BATTERY:
          new_val = 100;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 59, (new_val / 10) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1_BATTERY:
          new_val = 100;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 79, new_val  % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x10_BATTERY_TEMP:
          new_val = 47;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(66, 59, (new_val / 10) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1_BATTERY_TEMP:
          new_val = 47;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(66, 79, new_val  % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1000_SUN_POWER:
          new_val = 1234;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 168, new_val / 1000);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x100_SUN_POWER:
          new_val = 1234;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 188, (new_val / 100) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x10_SUN_POWER:
          new_val = 1234;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 208, (new_val / 10) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1_SUN_POWER:
          new_val = 1234;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(108, 228, new_val  % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1000_MOTOR_POWER:
          new_val = 2312;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(66, 168, new_val/1000);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x100_MOTOR_POWER:
          new_val = 2312;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(66, 188, (new_val / 100) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x10_MOTOR_POWER:
          new_val = 2312;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(66, 208, (new_val / 10) % 10);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_x1_MOTOR_POWER:
          new_val = 2312;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6423); // szam icon
            media_VideoFrame(66, 228, new_val  % 10);
          //  dspstate = DSP_WAITING;
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
            media_SetSector(0, 50); //indexjobb icon
            media_VideoFrame(433, 230, new_val);
          //  dspstate = DSP_WAITING;
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
            media_SetSector(0, 39); // indexbal icon
            media_VideoFrame(433, 7, new_val);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_TEMPOMAT:
          //if (cruiseIndicator()) // Tempomat villogás
          if (cruiseStatus())
          {
            new_val = 0;
          }
          else
            new_val = 1;

          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6604); // tempomat icon
            media_VideoFrame(433, 61, new_val);
          //  dspstate = DSP_WAITING;
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
            new_val = 1;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 65); //lámpa icon
            media_VideoFrame(433, 154, new_val);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_OVER_CHARGE:
          new_val = 1;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 6623); //túl fesz icon
            media_VideoFrame(375, 26, 1);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_DISCHARGE:
          new_val = 1;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 0); //alfesz icon
            media_VideoFrame(375, 219, 1);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_MOTOR_TEMO:
          new_val = 1;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 76); //motor veszely icon
            media_VideoFrame(375, 170, 1);
          //  dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        case DSP_DANGER:
          new_val = 1;
          if (dspValue[dspmessages] != new_val) {
            media_SetSector(0, 11); //figy icon
            media_VideoFrame(375, 80, 1);
            //dspstate = DSP_WAITING;
            dspValue[dspmessages] = new_val;
            break;
          }
          else
            dspmessages++;

        default:
          break;
      }
      dspmessages++;
      tasknumber = 0;
      if (dspmessages > DSP_NUM_MSG) {
        dspmessages = 0;
      }
    }

    chThdSleepMilliseconds(10);
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
  dspmessages = DSP_x100_SPEED;
  chThdCreateStatic(wadspTask, sizeof(wadspTask), NORMALPRIO + 7, dspTask, NULL);
}

void dspCalc(void) {

}

bool_t dspGetValue(uint8_t ch){
    ch = ch < 0 ? 0 : ch;
    ch = ch > 8 ? 8 : ch;
    return bus_bit[ch - 1];
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
    for (i = 0; i < DSP_NUM_MSG + 2; ++i)
    {
      chprintf(chp,"%d ", dspValue[i]);
    }
    chprintf(chp,"\r\n");
    chprintf(chp,"dspmessages: %d\r\n",dspmessages);
    chThdSleepMilliseconds(150);
  }
}

void cmdfrappans_dspmessages(BaseSequentialStream *chp, int argc, char *argv[]) {

enum dspMessages ch;

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

