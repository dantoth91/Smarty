/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "can_comm.h"
#include "can_items.h"
#include "speed.h"
#include "meas.h"
/*
 * #define CAN_MIN_EID         0x10
 * #define CAN_MAX_EID         0x1FFFFFF
 */

#define CAN_BMS_MIN         0x00
#define CAN_BMS_MAX         0x0F
#define CAN_BMS_MESSAGES_1  0x01
#define CAN_BMS_MESSAGES_2  0x02
#define CAN_BMS_MESSAGES_3  0x03
#define CAN_BMS_MESSAGES_4  0x04
#define CAN_BMS_MESSAGES_5  0x05

#define CAN_SM_MIN          0x10
#define CAN_SM_MAX          0x1F
#define CAN_SM_EID          0x10
#define CAN_SM_MESSAGES_1   0x01
#define CAN_SM_MESSAGES_2   0x02
#define CAN_SM_MESSAGES_3   0x03

#define CAN_ML_MIN              0x20
#define CAN_ML_MAX              0x2F
#define CAN_ML_ONEVIRE_MESSAGE  0x01

#define CAN_RPY_MIN         0x30
#define CAN_RPY_MAX         0x3F

#define CAN_LC_MIN          0x40
#define CAN_LC_MAX          0x5F
#define CAN_LC_MESSAGES_1   0x01
#define CAN_LC_MESSAGES_2   0x02
#define CAN_LC_MESSAGES_3   0x03

#define CAN_MAX_ADR         0x1FFFFFF

enum canState
{
  CAN_BMS,
  CAN_SM,
  CAN_ML,
  CAN_RPY,
  CAN_LC,
  CAN_WAIT,
  CAN_NUM_CH
}canstate;

enum canLight
{
  CAN_LIGHT_RIGHT,
  CAN_LIGHT_LEFT,
  CAN_LIGHT_WARNING,
  CAN_LIGHT_BRAKE_ON,
  CAN_LIGHT_BRAKE_OFF,
  CAN_LIGHT_LAMP_ON,
  CAN_LIGHT_LAMP_OFF,
  CAN_LIGHT_MESS
}canlight;

enum canMessages
{
  CAN_MESSAGES_1,
  CAN_MESSAGES_2,
  CAN_MESSAGES_3,
  CAN_MESSAGES_LAST,
  CAN_NUM_MESS
}canmessages;

static CANTxFrame txmsg;
static CANRxFrame rxmsg;

/*--------------------------------------------------*/
/* CAN_MCR_ABOM   ->  Automatic Bus-Off Management  */
/* CAN_MCR_AWUM   ->  Automatic Wakeup Mode         */
/* CAN_MCR_TXFP   ->  Transmit FIFO Priority        */
/* CAN_MSR_TXM    ->  Transmit Mode                 */
/* CAN_MSR_RXM    ->  Receive Mode                  */
/* CAN_BTR_LBKM   ->  Loop Back Mode (Debug)        */
/* CAN_BTR_SJW()  ->  Resynchronization Jump Width  */
/* CAN_BTR_TS2()  ->  Time Segment 2                */
/* CAN_BTR_TS1()  ->  Time Segment 1                */
/* CAN_BTR_BRP()  ->  Baud Rate Prescaler           */
/* ________________________________________________ */
/*                  CAN_BTR_BRP Baud                */
/*          F2XX           |      F4XX              */
/*  fPCLK   30             |      42                */
/*  tPCLK   0,033333333    |      0,023809524       */
/*  BRP[X]  4              |      6                 */
/*  tq      0,166666667    |      0,166666667       */
/*--------------------------------------------------*/

static const CANConfig cancfg = {
  CAN_MCR_ABOM,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

static uint16_t rx_id;
static uint16_t messages;

static int32_t message_status;
static bool can_newdata;
static bool can_transmit;

static int8_t sender;

static struct moduluxItems mlitems;
static struct bmsItems bmsitems;
/*
 * Receiver thread.
 */
static WORKING_AREA(can_rx_wa, 256);
static msg_t can_rx(void *p) {
  EventListener el;
  int i;

  (void)p;
  chRegSetThreadName("receiver");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);
  while(!chThdShouldTerminate()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
      continue;
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == RDY_OK) {
      palClearPad(GPIOA, GPIOA_TXD4);
      chSysLock();
      rx_id = 0;
      rx_id = rxmsg.EID >> 8;
      messages = (uint8_t)rxmsg.EID;
      can_newdata = TRUE;

      if(rx_id >= CAN_BMS_MIN && rx_id <= CAN_BMS_MAX){
        canstate = CAN_BMS;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_SM_MIN && rx_id <= CAN_SM_MAX){
        canstate = CAN_SM;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_ML_MIN && rx_id <= CAN_ML_MAX){
        canstate = CAN_ML;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_RPY_MIN && rx_id <= CAN_RPY_MAX){
        canstate = CAN_RPY;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_LC_MIN && rx_id <= CAN_LC_MAX){
        canstate = CAN_LC;
        rxmsg.EID = 0;
      }
      else{
        canstate = CAN_WAIT;
        rxmsg.EID = 0;
      }

      switch(canstate){

        case CAN_BMS:
          sender = 1;
          if(messages == CAN_BMS_MESSAGES_1){
            for(i = 0; i < 34; i++){
              if (rxmsg.data8[0] == cellitems[i].id)
              {
                cellitems[i].cell_voltage    = rxmsg.data8[2];
                cellitems[i].cell_voltage   += rxmsg.data8[1] << 8;
                cellitems[i].cell_resistant  = rxmsg.data8[4];
                cellitems[i].cell_resistant += rxmsg.data8[3] << 8;
                cellitems[i].open_voltage    = rxmsg.data8[6];
                cellitems[i].open_voltage   += rxmsg.data8[5] << 8;
              }
            }
          }

          else if(messages == CAN_BMS_MESSAGES_2){
            bmsitems.id                  = rx_id - CAN_BMS_MIN;
            bmsitems.relay               = rxmsg.data8[0];
            bmsitems.pack_inst_volt      = rxmsg.data8[2];
            bmsitems.pack_inst_volt     += rxmsg.data8[1] << 8;
            bmsitems.pack_soc            = (rxmsg.data8[3] * 10) / 2;
            bmsitems.pack_healt          = rxmsg.data8[4];
            bmsitems.pack_amps           = rxmsg.data8[6];
            bmsitems.pack_amps          += rxmsg.data8[5] << 8;
            bmsitems.pack_resist         = rxmsg.data8[7];
          }

          else if(messages == CAN_BMS_MESSAGES_3){
            bmsitems.average_temp        = rxmsg.data8[0];
            bmsitems.internal_temp       = rxmsg.data8[1];
            bmsitems.low_cell_volt       = rxmsg.data16[1];
            bmsitems.high_cell_volt      = rxmsg.data16[2];
            bmsitems.avg_cell_voltage    = rxmsg.data16[3];
          }

          else if(messages == CAN_BMS_MESSAGES_4){
            bmsitems.pack_current        = rxmsg.data16[0];
            bmsitems.total_pack_cycle    = rxmsg.data8[2];
            bmsitems.pack_ccl            = rxmsg.data8[3];
            bmsitems.pack_dcl            = rxmsg.data8[4];
            bmsitems.maximum_cell        = rxmsg.data8[6];
            bmsitems.maximum_cell       += rxmsg.data8[5] << 8;
            bmsitems.custom_flag         = rxmsg.data8[7];
          }

          else if(messages == CAN_BMS_MESSAGES_5){
            bmsitems.minimum_cell        = rxmsg.data16[0];
            bmsitems.high_cell_volt2     = rxmsg.data8[2];
            bmsitems.low_cell_volt2      = rxmsg.data8[3];
            bmsitems.supply_12v          = rxmsg.data8[4];
            bmsitems.fan_speed           = rxmsg.data8[5];
            bmsitems.pack_open_voltage   = rxmsg.data16[4];
          }

          canstate = CAN_WAIT;
          break;

        case CAN_SM:
          sender = 2;
          canstate = CAN_WAIT;
          break;

        case CAN_ML:
          sender = 3;
          if(messages == CAN_ML_ONEVIRE_MESSAGE){
            mlitems.id        = (rx_id - CAN_ML_MIN) + 1;
            mlitems.onevire_1 = rxmsg.data16[0];
            mlitems.onevire_2 = rxmsg.data16[1];
            mlitems.onevire_3 = rxmsg.data16[2];
            mlitems.onevire_4 = rxmsg.data16[3];
          }
          canstate = CAN_WAIT;
          break;

        case CAN_RPY:
          sender = 4;
          canstate = CAN_WAIT;
          break;

        case CAN_LC:
          sender = 5;
          if(messages == CAN_LC_MESSAGES_1){
            for(i = 0; i < sizeof(lcitems); i++){
              if (rx_id == lcitems[i].id)
              {
                lcitems[i].temp       = rxmsg.data8[0];
                lcitems[i].curr_in    = rxmsg.data8[1];
                lcitems[i].curr_out   = rxmsg.data8[2];
                lcitems[i].efficiency = rxmsg.data8[3];
                lcitems[i].volt_in    = rxmsg.data16[2];
                lcitems[i].volt_out   = rxmsg.data16[3];
              }
            }
          }

          else if(messages == CAN_LC_MESSAGES_2){
            for(i = 0; i < sizeof(lcitems); i++){
              if (rx_id == lcitems[i].id)
              {
                lcitems[i].status = rxmsg.data16[0];
                lcitems[i].pwm    = rxmsg.data16[1];
              }
            }
          }

          else if(messages == CAN_LC_MESSAGES_3){            
          }
          canstate = CAN_WAIT;
          break;

        case CAN_WAIT:
          break;

        default:
          break;
      }
      chSysUnlock();
      palSetPad(GPIOA, GPIOA_TXD4);
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return 0;
}

/*
 * Transmitter thread.
 */
static WORKING_AREA(can_tx_wa, 256);
static msg_t can_tx(void * p) {

  (void)p;
  uint8_t period = 0;
  chRegSetThreadName("transmitter");

  while (!chThdShouldTerminate()) {
    
    for(message_status = 0; message_status < CAN_NUM_MESS; message_status ++){
      palClearPad(GPIOD, GPIOD_TXD3);
      switch(message_status){
        case CAN_MESSAGES_1:
          /* Message 3 */
          /* 
          * 16bit - sebesség
          * 16bit - gáz állás
          * 8bit  - tempomat beállított értéke
          * 8bit  - tempomat státusz 
          */

          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_1;
          txmsg.EID += CAN_SM_EID << 8;

          
          //txmsg.data8[0] = 0x0A;
          //txmsg.data8[1] = 0x0D;
          txmsg.data16[0] = speedGetSpeed();
          txmsg.data16[1] = cruiseGetPWM();
          txmsg.data8[4] = cruiseGet();
          txmsg.data8[5] = cruiseStatus();
          
          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          break;

        case CAN_MESSAGES_2:
          /* Message 3 */
          /* 
          * 16bit - gázpedál állás
          * 16bit - fákpedál állás
          * 16bit - féknyomás1
          * 16bit - féknyomás2
          */

          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_2;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data16[0] = measGetValue_2(MEAS2_THROTTLE);
          txmsg.data16[1] = measGetValue_2(MEAS2_REGEN_BRAKE);
          txmsg.data16[2] = measGetValue(MEAS_BRAKE_PRESSURE1);
          txmsg.data16[3] = measGetValue(MEAS_BRAKE_PRESSURE2);
          
          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          break;

        case CAN_MESSAGES_3:
          /* Message 3 */
          /* 
          * 8bit  - motortúlmelegedés mérés
          * 16bit - motor áram
          * 16bit - 12V feszültséget (UBAT)
          */
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_3;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data8[0] = 0;
          txmsg.data16[1] = measGetValue_2(MEAS2_CURR1);
          txmsg.data16[2] = measGetValue(MEAS_UBAT);
          
          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          break;

        default:      
            break;
        
      }
      palSetPad(GPIOD, GPIOD_TXD3);
      chThdSleepMilliseconds(5);
      }
    chThdSleepMilliseconds(200);
  }
  return 0;
}

void can_commInit(void){
  canStart(&CAND1, &cancfg);
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;

  rxmsg.IDE = CAN_IDE_EXT;
  rxmsg.RTR = CAN_RTR_DATA;
  rxmsg.DLC = 8;

  can_newdata = FALSE;
}

void cmd_can_commvalues(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    //chprintf(chp, "\x1B[%d;%dH", 0, 0);
    chprintf(chp,"RX.EID   : %x \r\n",rxmsg.EID);
    chprintf(chp,"RX.DATA1 : %d \r\n",rxmsg.data16[0]);
    chprintf(chp,"RX.DATA2 : %d \r\n",rxmsg.data16[1]);
    chprintf(chp,"RX.DATA3 : %d \r\n",rxmsg.data16[2]);
    chprintf(chp,"RX.DATA4 : %d \r\n",rxmsg.data16[3]);
    chprintf(chp,"\r\n");

    chThdSleepMilliseconds(50);
  }
}

void cmd_canmppttest(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    chprintf(chp,"----------- CAN Transmit ------------\r\n");
    chprintf(chp,"TX.EID              : %15x \r\n",txmsg.EID);
    chprintf(chp,"TX.DATA32_1         : %15x \r\n",txmsg.data32[0]);
    chprintf(chp,"TX.DATA32_2         : %15x \r\n",txmsg.data32[1]);
    chprintf(chp,"\r\n");
    //canTransmitData(txmessages);

    //canReceiveData(rxmessages);
    chprintf(chp,"----------- CAN Receive -------------\r\n");
    chprintf(chp,"RX.EID              : %15x \r\n",rxmsg.EID);
    chprintf(chp,"RX.DATA8_1 (NTC)    : %15d \r\n",rxmsg.data8[0]);
    chprintf(chp,"RX.DATA8_2 (IN_CUR) : %15d \r\n",rxmsg.data8[1]);
    chprintf(chp,"RX.DATA8_3 (OUT_CUR): %15d \r\n",rxmsg.data8[2]);
    chprintf(chp,"RX.DATA8_4 (EFF)    : %15d \r\n",rxmsg.data8[3]);
    chprintf(chp,"RX.DATA8_5 (VIN)    : %15d \r\n",rxmsg.data16[2]);
    chprintf(chp,"RX.DATA8_5 (VOUT)   : %15d \r\n",rxmsg.data16[3]);
    chprintf(chp,"\r\n");
    chprintf(chp,"id                  : %15x \r\n",rx_id);
    chprintf(chp,"messages            : %15x \r\n",messages);

    chThdSleepMilliseconds(250);
  }
}

void cmd_canall(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if ((argc == 1) && (strcmp(argv[0], "8") == 0)){

      chprintf(chp,"-------------- CAN all data --------------\r\n");
      chprintf(chp,"rxmsg.EID     : %15d \r\n",rxmsg.EID);
      chprintf(chp,"rxmsg.data8[0]: %15d \r\n",rxmsg.data8[0]);
      chprintf(chp,"rxmsg.data8[1]: %15d \r\n",rxmsg.data8[1]);
      chprintf(chp,"rxmsg.data8[2]: %15d \r\n",rxmsg.data8[2]);
      chprintf(chp,"rxmsg.data8[3]: %15d \r\n",rxmsg.data8[3]);
      chprintf(chp,"rxmsg.data8[4]: %15d \r\n",rxmsg.data8[4]);
      chprintf(chp,"rxmsg.data8[5]: %15d \r\n",rxmsg.data8[5]);
      chprintf(chp,"rxmsg.data8[6]: %15d \r\n",rxmsg.data8[6]);
      chprintf(chp,"rxmsg.data8[7]: %15d \r\n",rxmsg.data8[7]);
    }

    else if ((argc == 1) && (strcmp(argv[0], "16") == 0)){

      chprintf(chp,"-------------- CAN all data --------------\r\n");
      chprintf(chp,"rxmsg.EID      : %15d \r\n",rxmsg.EID);
      chprintf(chp,"rxmsg.data16[0]: %15d \r\n",rxmsg.data16[0]);
      chprintf(chp,"rxmsg.data16[1]: %15d \r\n",rxmsg.data16[1]);
      chprintf(chp,"rxmsg.data16[2]: %15d \r\n",rxmsg.data16[2]);
      chprintf(chp,"rxmsg.data16[3]: %15d \r\n",rxmsg.data16[3]);
    }

    else if ((argc == 1) && (strcmp(argv[0], "32") == 0)){

      chprintf(chp,"-------------- CAN all data --------------\r\n");
      chprintf(chp,"rxmsg.EID      : %15d \r\n",rxmsg.EID);
      chprintf(chp,"rxmsg.data32[0]: %15d \r\n",rxmsg.data32[0]);
      chprintf(chp,"rxmsg.data32[1]: %15d \r\n",rxmsg.data32[1]);
    }

    else{
      chprintf(chp, "This not good parameters! (canall 8/16/32)\r\n");
      return;
    }
  }
}

void cmd_candata_lc(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;

  int16_t db;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if (argc == 1){
      
      db = atol(argv[0]);
      db = db > (sizeof(lcitems) / 16) ? (sizeof(lcitems) / 16) : db;

      chprintf(chp,"-------------- LuxControl %d/%d --------------\r\n", db, sizeof(lcitems) / 16);
      chprintf(chp,"lcitems[%d].id         (ID)     : %15x \r\n", db, lcitems[db].id);
      chprintf(chp,"lcitems[%d].temp       (NTC)    : %15d \r\n", db, lcitems[db].temp);
      chprintf(chp,"lcitems[%d].curr_in    (IN_CUR) : %15d \r\n", db, lcitems[db].curr_in);
      chprintf(chp,"lcitems[%d].curr_out   (OUT_CUR): %15d \r\n", db, lcitems[db].curr_out);
      chprintf(chp,"lcitems[%d].efficiency (EFF)    : %15d \r\n", db, lcitems[db].efficiency);
      chprintf(chp,"lcitems[%d].status     (STATUS) : %15d \r\n", db, lcitems[db].status);
      chprintf(chp,"lcitems[%d].volt_in    (VIN)    : %15d \r\n", db, lcitems[db].volt_in);
      chprintf(chp,"lcitems[%d].volt_out   (VOUT)   : %15d \r\n", db, lcitems[db].volt_out);
      chprintf(chp,"lcitems[%d].pwm        (PWM)    : %15d \r\n", db, lcitems[db].pwm);
    }   

    else{
      chprintf(chp, "This not good parameters!\r\n");
      chprintf(chp, "(Luxcontrol 1-x) lc n\r\n");
      return;
    }
  }
}

void cmd_candata_ml(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;

  int16_t db;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if (argc == 1){
      chprintf(chp,"---------- Modulux 1/1 ------------\r\n");
      chprintf(chp,"id        (ID)     : %15x \r\n", mlitems.id);
      chprintf(chp,"onevire_1 (DS18B20): %15d \r\n", mlitems.onevire_1);
      chprintf(chp,"onevire_2 (DS18B20): %15d \r\n", mlitems.onevire_2);
      chprintf(chp,"onevire_3 (DS18B20): %15d \r\n", mlitems.onevire_3);
      chprintf(chp,"onevire_4 (DS18B20): %15d \r\n", mlitems.onevire_4);
    } 
    else{
      chprintf(chp, "This not good parameters!\r\n");
      chprintf(chp, "(Modulux 1-x) candata_ml n\r\n");
      return;
    }
  }
}

void cmd_candata_bms(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;

  int16_t db;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if (argc == 1){
      chprintf(chp,"------------- BMS 1/1 --------------\r\n");
      chprintf(chp,"----------- Messages 2 -------------\r\n");
      chprintf(chp,"id                 : %15x \r\n", bmsitems.id);
      chprintf(chp,"relay              : %15d \r\n", bmsitems.relay);
      chprintf(chp,"pack_inst_volt     : %15d \r\n", bmsitems.pack_inst_volt);
      chprintf(chp,"pack_soc           : %15d \r\n", bmsitems.pack_soc);
      chprintf(chp,"pack_healt         : %15d \r\n", bmsitems.pack_healt);
      chprintf(chp,"pack_amps          : %15d \r\n", bmsitems.pack_amps);
      chprintf(chp,"pack_resist        : %15d \r\n", bmsitems.pack_resist);
      chprintf(chp,"\r\n----------- Messages 3 -------------\r\n");
      chprintf(chp,"average_temp       : %15d \r\n", bmsitems.average_temp);
      chprintf(chp,"internal_temp      : %15d \r\n", bmsitems.internal_temp);
      chprintf(chp,"low_cell_volt      : %15d \r\n", bmsitems.low_cell_volt);
      chprintf(chp,"high_cell_volt     : %15d \r\n", bmsitems.high_cell_volt);
      chprintf(chp,"avg_cell_voltage   : %15d \r\n", bmsitems.avg_cell_voltage);
      chprintf(chp,"\r\n----------- Messages 4 -------------\r\n");
      chprintf(chp,"pack_current       : %15d \r\n", bmsitems.pack_current);
      chprintf(chp,"total_pack_cycle   : %15d \r\n", bmsitems.total_pack_cycle);
      chprintf(chp,"pack_ccl           : %15d \r\n", bmsitems.pack_ccl);
      chprintf(chp,"pack_dcl           : %15d \r\n", bmsitems.pack_dcl);
      chprintf(chp,"maximum_cell       : %15d \r\n", bmsitems.maximum_cell);
      chprintf(chp,"custom_flag        : %15d \r\n", bmsitems.custom_flag);
      chprintf(chp,"\r\n----------- Messages 5 -------------\r\n");
      chprintf(chp,"minimum_cell       : %15d \r\n", bmsitems.minimum_cell);
      chprintf(chp,"high_cell_volt2    : %15d \r\n", bmsitems.high_cell_volt2);
      chprintf(chp,"low_cell_volt2     : %15d \r\n", bmsitems.low_cell_volt2);
      chprintf(chp,"supply_12v         : %15d \r\n", bmsitems.supply_12v);
      chprintf(chp,"fan_speed          : %15d \r\n", bmsitems.fan_speed);
      chprintf(chp,"pack_open_voltage  : %15d \r\n", bmsitems.pack_open_voltage);
    } 
    
    else{
      chprintf(chp, "This not good parameters!\r\n");
      chprintf(chp, "(BMS 1-x)   candata_bms n\r\n");
      return;
    }
  }
}

void cmd_candata_cell(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;

  int16_t db;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if (argc == 1){
      
      db = atol(argv[0]);
      db = db > 34 ? 34 : db;

      chprintf(chp,"-------------- Bettery Cell %d/%d --------------\r\n", db, 34);
      chprintf(chp,"cellitems[%d].id              : %15x \r\n", db, cellitems[db].id);
      chprintf(chp,"cellitems[%d].cell_voltage    : %15d \r\n", db, cellitems[db].cell_voltage);
      chprintf(chp,"cellitems[%d].cell_resistant  : %15d \r\n", db, cellitems[db].cell_resistant);
      chprintf(chp,"cellitems[%d].open_voltage    : %15d \r\n", db, cellitems[db].open_voltage);
    }   

    else{
      chprintf(chp, "This not good parameters!\r\n");
      chprintf(chp, "(Cell 1-x) lc n\r\n");
      return;
    }
  }
}

void cmd_lcSleep(BaseSequentialStream *chp, int argc, char *argv[]){
  //message_status = CAN_MESSAGES_1;
  chprintf(chp,"Sleep 1. LC!\r\n");
}

void cmd_canmonitor(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    //chprintf(chp, "\x1B[%d;%dH", 0, 0);
    if(can_newdata){
      chprintf(chp,"%-6s", 
              sender == 1 ? "BMS" :     \
              sender == 2 ? "SM" :  \
              sender == 3 ? "ML" :      \
              sender == 4 ? "RBY" : "LC");

      chprintf(chp,"%3x %3x - %4x %4x %4x %4x %4x %4x %4x %4x\r\n", 
        rx_id,
        messages,
        rxmsg.data8[0],
        rxmsg.data8[1],
        rxmsg.data8[2],
        rxmsg.data8[3],
        rxmsg.data8[4],
        rxmsg.data8[5],
        rxmsg.data8[6],
        rxmsg.data8[7]
      );

      can_newdata = FALSE;      
    }
    if(can_transmit){
      chprintf(chp,"%-6s", "SM");
      chprintf(chp,"%3x %3x - %4x %4x %4x %4x %4x %4x %4x %4x\r\n", 
        txmsg.EID  >> 8,
        (uint8_t)txmsg.EID,
        txmsg.data8[0],
        txmsg.data8[1],
        txmsg.data8[2],
        txmsg.data8[3],
        txmsg.data8[4],
        txmsg.data8[5],
        txmsg.data8[6],
        txmsg.data8[7]
      );

      can_transmit = FALSE;      
    }
    //chThdSleepMilliseconds(10);
  }
}