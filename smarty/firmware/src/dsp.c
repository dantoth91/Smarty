/*
   Smarty - Copyright (C) 2015
    GAMF ECOMarathon Team
*/

#include "uart_comm.h"
#include "meas.h"
#include "gfx.h"
#include "dsp.h"

#include "chprintf.h"

/* Display answer message */
#define DIAG_DSP_OK (0x06)


//Color
#define RED                       (0xF800)
#define BLACK                     (0x0000)

enum dspState
{
  DSP_WAITING,
  DSP_RUNNING
}dspstate;

enum dspMessages
{
  DSP_SPEED,
  DSP_x10_SPEED,
  DSP_x1_SPEED,
  DSP_x10_AVG_SPEED,
  DSP_x1_AVG_SPEED,
  DSP_x01_AVG_SPEED,
  DSP_x100_CHT,
  DSP_x10_CHT,
  DSP_x1_CHT,
  DSP_x10_VOLTAGE,
  DSP_x1_VOLTAGE,
  DSP_x01_VOLTAGE,
  DSP_x10_ETM,
  DSP_x1_ETM,
  DSP_NUM_MSG
}dspmessages;

static int32_t dspValue[DSP_NUM_MSG +2] = {-1};
static int tasknumber;
bool_t dsp_activ;
uint8_t c;

int db=0;
int db2=10;

void dsp_LoadTop(void){
  media_SetSector(0, 35); // indexbal icon
   media_VideoFrame(433,7,1);

   media_SetSector(0, 6582); // tempomat icon
   media_VideoFrame(433,61,1);

   media_SetSector(0, 61); //lámpa icon
   media_VideoFrame(433,154,1);

   media_SetSector(0, 46); //indexjobb icon
   media_VideoFrame(433,230,1);

   media_SetSector(0, 6601); //túl fesz icon
   media_VideoFrame(375,26,1);

   media_SetSector(0, 11); //figy icon
   media_VideoFrame(375,80,1);

   media_SetSector(0, 72); //motor veszely icon
   media_VideoFrame(375,170,1);

   media_SetSector(0, 0); //alfesz icon
   media_VideoFrame(375,219,1);

   media_SetSector(0, 6400); //Átlag icon
   media_Image(340,4);

   media_SetSector(0, 6593); //Tempomat szöveg
   media_Image(340,166);


}
void dsp_LoadMidle(void){

  media_SetSector(0, 83); // müszer icon
  media_VideoFrame(148,21,1);

  media_SetSector(0, 57); // km/h szöveg
  media_Image(168,204);

  media_SetSector(0, 6406); // szam kicsi icon
  media_VideoFrame(315,244,1);
  media_SetSector(0, 6406);
  media_VideoFrame(315,33,1);

  media_SetSector(0, 6441); // szam nagy icon
  media_VideoFrame(186,155,1);

}
void dspInit(void){

  gfx_Cls();
  file_Mount();
  dsp_LoadTop();
  dsp_LoadMidle();




}

void dspCalc(void){
  /*
  db++;
  db2++;
  if(db2==20)
  {
    media_SetSector(0, 6593);
    //chSequentialStreamRead(&SD2, &c, 1);


  db2=0;
  }
  if(db==20)
    {
      media_Image(0, 0);
      //chSequentialStreamRead(&SD2, &c, 1);

    db=0;
    }

*/
}

void cmdfrappans_dspmessages(BaseSequentialStream *chp, int argc, char *argv[]) {

  enum dspMessages ch;


  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);


          if ((argc == 1) && (strcmp(argv[0], "Clear") == 0)){

            chprintf(chp,"-------------- dps clear --------------\r\n");
            gfx_Cls();
            return;
          }

          if ((argc == 1) && (strcmp(argv[0], "Sector") == 0)){

            chprintf(chp,"--------------Set sector --------------\r\n");
            media_SetSector(0, 6593);

            return;
          }

          if ((argc == 1) && (strcmp(argv[0], "Image") == 0)){

            chprintf(chp,"-------------- media Image --------------\r\n");

            media_SetSector(0, 6593);
            media_Image(0, 0);
            return;
          }

          if ((argc == 1) && (strcmp(argv[0], "Bg") == 0)){

                      chprintf(chp,"-------------- Background --------------\r\n");
                      gfx_BGcolour(RED);
                      return;
                    }

        else{
              goto ERROR;
            }
      }

    ERROR:
      chprintf(chp, "Usage: start right\r\n");
      chprintf(chp, "       start left\r\n");
      chprintf(chp, "       start warning\r\n");
      chprintf(chp, "       stop\r\n");
      return;
    chThdSleepMilliseconds(1000);

}

