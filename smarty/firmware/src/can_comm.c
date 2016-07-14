/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "can_items.h"
#include "can_comm.h"
#include "speed.h"
#include "meas.h"
#include "calc.h"
#include "log.h"
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
#define CAN_SM_MESSAGES_4   0x04
#define CAN_SM_MESSAGES_5   0x05
#define CAN_SM_MESSAGES_6   0x06

#define CAN_ML_MIN              0x20
#define CAN_ML_MAX              0x2F
#define CAN_ML_ONEVIRE_MESSAGE  0x01
#define CAN_ML_CURRENT_MESSAGE  0x02

#define CAN_RPY_MIN         0x30
#define CAN_RPY_MAX         0x3F

#define CAN_LC_MIN          0x40
#define CAN_LC_MAX          0x5F
#define CAN_LC_MESSAGES_1   0x01
#define CAN_LC_MESSAGES_2   0x02
#define CAN_LC_MESSAGES_3   0x03

#define CAN_IOTC_MIN          0x60
#define CAN_IOTC_MAX          0x6F
#define CAN_IOTC_MESSAGES_1   0x01
#define CAN_IOTC_MESSAGES_2   0x02
#define CAN_IOTC_MESSAGES_3   0x03
#define CAN_IOTC_MESSAGES_4   0x04
#define CAN_IOTC_MESSAGES_5   0x05

#define CAN_MAX_ADR         0x1FFFFFF

enum canState
{
  CAN_BMS,
  CAN_SM,
  CAN_ML,
  CAN_RPY,
  CAN_LC,
  CAN_IOTC,
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
  CAN_MESSAGES_4,
  CAN_MESSAGES_5,
  CAN_MESSAGES_6,
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
/*  BRP[X]  9              |      13                */
/*  tq      0,333333333    |      0,333333333       */
/*--------------------------------------------------*/

static const CANConfig cancfg = {
  CAN_MCR_ABOM,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(13)
};

static uint16_t rx_id;
static uint16_t messages;

static int32_t message_status;
static bool can_newdata;
static bool can_transmit;

static uint32_t bms_asis;

static int8_t sender;

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
      else if(rx_id >= CAN_IOTC_MIN && rx_id <= CAN_IOTC_MAX){
        canstate = CAN_IOTC;
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
            for(i = 0; i < (sizeof(cellitems.id) / 4); i++){
              if (rxmsg.data8[0] == cellitems.id[i])
              {
                cellitems.cell_voltage[i]    = rxmsg.data8[2];
                cellitems.cell_voltage[i]   += rxmsg.data8[1] << 8;
                cellitems.cell_resistant[i]  = rxmsg.data8[4];
                cellitems.cell_resistant[i] += rxmsg.data8[3] << 8;
                cellitems.open_voltage[i]    = rxmsg.data8[6];
                cellitems.open_voltage[i]   += rxmsg.data8[5] << 8;
              }
            }
          }

          else if(messages == CAN_BMS_MESSAGES_2){
            bmsitems.id                  = rx_id - CAN_BMS_MIN;
            bmsitems.custom_flag_1       = rxmsg.data8[0];
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
            bmsitems.low_cell_volt       = rxmsg.data8[3];
            bmsitems.low_cell_volt      += rxmsg.data8[2] << 8;
            bmsitems.high_cell_volt      = rxmsg.data8[5];
            bmsitems.high_cell_volt     += rxmsg.data8[4] << 8;
            bmsitems.avg_cell_voltage    = rxmsg.data8[7];
            bmsitems.avg_cell_voltage   += rxmsg.data8[6] << 8;
          }

          else if(messages == CAN_BMS_MESSAGES_4){
            bmsitems.pack_current        = rxmsg.data8[1];
            bmsitems.pack_current       += rxmsg.data8[0] << 8;
            bmsitems.total_pack_cycle    = rxmsg.data8[2];
            bmsitems.pack_ccl            = rxmsg.data8[3];
            bmsitems.pack_dcl            = rxmsg.data8[4];
            bmsitems.maximum_cell        = rxmsg.data8[6];
            bmsitems.maximum_cell       += rxmsg.data8[5] << 8;
            bmsitems.custom_flag_2       = rxmsg.data8[7];
          }

          else if(messages == CAN_BMS_MESSAGES_5){
            bmsitems.minimum_cell        = rxmsg.data8[1];
            bmsitems.minimum_cell       += rxmsg.data8[0] << 8;
            bmsitems.high_cell_volt_num  = rxmsg.data8[2];
            bmsitems.low_cell_volt_num   = rxmsg.data8[3];
            bmsitems.supply_12v          = rxmsg.data8[4];
            bmsitems.fan_speed           = rxmsg.data8[5];
            bmsitems.pack_open_voltage   = rxmsg.data8[7];
            bmsitems.pack_open_voltage  += rxmsg.data8[6] << 8;
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
            mlitems.MODULE1_TEMP = rxmsg.data16[0];
            mlitems.MODULE8_TEMP = rxmsg.data16[1];
            mlitems.MODULE6_TEMP = rxmsg.data16[2];
            mlitems.MODULE12_TEMP = rxmsg.data16[3];
          }
          if(messages == CAN_ML_CURRENT_MESSAGE){
            mlitems.id        = (rx_id - CAN_ML_MIN) + 2;
            mlitems.sun_current = rxmsg.data16[0];
            mlitems.MODULE2_TEMP = rxmsg.data16[2];
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
            for(i = 0; i < (sizeof(lcitems.id) / 4); i++){
              if (rx_id == lcitems.id[i])
              {
                lcitems.temp[i]       = rxmsg.data8[0];
                lcitems.curr_in[i]    = rxmsg.data8[1];
                lcitems.curr_out[i]   = rxmsg.data8[2];
                lcitems.volt_in[i]    = rxmsg.data16[2];
                lcitems.volt_out[i]   = rxmsg.data16[3];
              }
            }
          }

          else if(messages == CAN_LC_MESSAGES_2){
            for(i = 0; i < (sizeof(lcitems.id) / 4); i++){
              if (rx_id == lcitems.id[i])
              {
                lcitems.status[i] = rxmsg.data16[0];
                lcitems.pwm[i]    = rxmsg.data16[1];
                lcitems.curr_in_from_pwm[i]    = rxmsg.data16[2];
              }
            }
          }

          else if(messages == CAN_LC_MESSAGES_3){            
          }
          canstate = CAN_WAIT;
          break;

        case CAN_IOTC:
          sender = 6;
          if(messages == CAN_IOTC_MESSAGES_1){
            /*for(i = 0; i < (sizeof(IOTCitems.id) / 4); i++){
              if (rx_id == IOTCitems.id[i])
              {
                IOTCitems.ain_1[i] = rxmsg.data16[0];
              }
            }*/
            IOTCitems.ain_1 = rxmsg.data16[0];
          }

          if(messages == CAN_IOTC_MESSAGES_2){
            /*for(i = 0; i < (sizeof(IOTCitems.id) / 4); i++){
              if (rx_id == IOTCitems.id[i])
              {
                IOTCitems.ain_2[i] = rxmsg.data16[0];
              }
            }*/
            IOTCitems.ain_2 = rxmsg.data16[0];
          }

          if(messages == CAN_IOTC_MESSAGES_3){
            /*for(i = 0; i < (sizeof(IOTCitems.id) / 4); i++){
              if (rx_id == IOTCitems.id[i])
              {
                IOTCitems.ain_3[i] = rxmsg.data16[0];
              }
            }*/
            IOTCitems.ain_3 = rxmsg.data16[0];
          }

          if(messages == CAN_IOTC_MESSAGES_4){
            /*for(i = 0; i < (sizeof(IOTCitems.id) / 4); i++){
              if (rx_id == IOTCitems.id[i])
              {
                IOTCitems.ain_4[i] = rxmsg.data16[0];
              }
            }*/
            IOTCitems.ain_4 = rxmsg.data16[0];
          }

          if(messages == CAN_IOTC_MESSAGES_5){
            /*for(i = 0; i < (sizeof(IOTCitems.id) / 4); i++){
              if (rx_id == IOTCitems.id[i])
              {
                IOTCitems.ain_5[i] = rxmsg.data16[0];
              }
            }*/
            IOTCitems.ain_5 = rxmsg.data16[0];
          }
          canstate = CAN_WAIT;
          break;

        case CAN_WAIT:
          break;

        default:
          break;
      }
      chSysUnlock();
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
      switch(message_status){
        case CAN_MESSAGES_1:
          /* Message 1 */
          /* 
          * 16bit - Speed
          * 16bit - Cruise control pwm
          * 8bit  - Cruise control set
          * 8bit  - Cruise control status
          * 8bit  - Engine OVT
          */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_1;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data16[0] = speedGetSpeed();
          txmsg.data16[1] = cruiseGetPWM();
          txmsg.data8[4] = cruiseGet();
          txmsg.data8[5] = cruiseStatus();
          txmsg.data8[6] = measGetValue(MEAS_IS_IN_DRIVE);
          
          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        case CAN_MESSAGES_2:
          /* Message 2 */
          /* 
          * 16bit - Throttle pedal
          * 16bit - Brake pedal
          * 16bit - Brake pressure 1
          * 16bit - Brake pressure 2
          */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_2;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data16[0] = measGetValue_2(MEAS2_THROTTLE);
          txmsg.data16[1] = measGetValue_2(MEAS2_REGEN_BRAKE);
          txmsg.data16[2] = measGetValue(MEAS_TEMP1);
          txmsg.data16[3] = measGetValue(MEAS_TEMP2);
          
          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        case CAN_MESSAGES_3:
          /* Message 3 */
          /* 
          * 16bit - Engine current
          * 16bit - 12V (UBAT)
          * 8bit - LOG state
          * 8bit - NULL
          * 16bit - Motor Current
          */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_3;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data16[0] = (int16_t)calcGetValue(CALC_MOTOR_POWER);
          txmsg.data16[1] = measGetValue(MEAS_UBAT);
          if(logGetState() == LOG_RUNNING)
          {
            txmsg.data8[4] = 1;
          }
          else
          {
            txmsg.data8[4] = 0;
          }

          txmsg.data16[3] = measGetValue_2(MEAS2_CURR1);

          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        case CAN_MESSAGES_4:
          /* Message 4 */
          /*
          * 32bit - Total KMeter
          * 32bit - Nullable Kmeter
          */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_4;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data32[0] = GetTotalKmeterDistance();
          txmsg.data32[1] = GetKmeterDistance();


          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        case CAN_MESSAGES_5:
          /* Message 4 */
          /*
          * 16bit - Average speed
          */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_5;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data16[0] = calcAvgSpeed();
          txmsg.data16[1] = measGetValue(MEAS_STEERING);

          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        case CAN_MESSAGES_6:
          /* Message 4 */
          /*
          * 16bit - CHP_J
          * 16bit - CHP_B
          */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_SM_MESSAGES_6;
          txmsg.EID += CAN_SM_EID << 8;

          txmsg.data16[0] = measGetValue(MEAS_CHP_B);
          txmsg.data16[1] = measGetValue(MEAS_CHP_J);

          can_transmit = TRUE;
          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        default:      
            break;
        
      }
      chThdSleepMilliseconds(5);
      }
    chThdSleepMilliseconds(100);
  }
  return 0;
}

void can_commInit(void){
  int i;
  
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

  for (i = 0; i < (sizeof(cellitems.id) / 4); ++i)
  {
    cellitems.id[i] = i;
  }

  for (i = 0; i < (sizeof(lcitems.id) / 4); ++i)
  {
    lcitems.id[i] = i + 64;
  }

  /*for (i = 0; i < (sizeof(IOTCitems.id) / 4); ++i)
  {
    IOTCitems.id[i] = i;
  }*/
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
  char c;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");

  db = atol(argv[0]);
  
  while (TRUE) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if (argc == 1){
      chnReadTimeout(&SD1, (uint8_t *)&c, 1, 100);

      if (c == 97){
        db--;
        c = 0;
      }

      else if (c == 100){
        db++;
        c = 0;
      }
      else if (c == 13){
        break;
      }

      db = db > (sizeof(lcitems.id) / 4) ? (sizeof(lcitems.id) / 4) : db;
      db = db < 1 ? 1 : db;

      chprintf(chp,"------------------------------------------------\r\n");
      chprintf(chp,"|                <-  (EXIT)  ->                |\r\n");
      chprintf(chp,"|               (a)  (ENTER) (d)               |\r\n");
      chprintf(chp,"--------------- LuxControl %d/%d ---------------\r\n", db, sizeof(lcitems.id) / 4);
      chprintf(chp,"lcitems.id[%d]         (ID)     : %15x \r\n", db, lcitems.id[db]);
      chprintf(chp,"lcitems.temp[%d]       (NTC)    : %15d \r\n", db, lcitems.temp[db]);
      chprintf(chp,"lcitems.curr_in[%d]    (IN_CUR) : %15d \r\n", db, lcitems.curr_in[db]);
      chprintf(chp,"lcitems.curr_out[%d]   (OUT_CUR): %15d \r\n", db, lcitems.curr_out[db]);
      chprintf(chp,"lcitems.status[%d]     (STATUS) : %15d \r\n", db, lcitems.status[db]);
      chprintf(chp,"lcitems.volt_in[%d]    (VIN)    : %15d \r\n", db, lcitems.volt_in[db]);
      chprintf(chp,"lcitems.volt_out[%d]   (VOUT)   : %15d \r\n", db, lcitems.volt_out[db]);
      chprintf(chp,"lcitems.pwm[%d]        (PWM)    : %15d \r\n", db, lcitems.pwm[db]);
      chprintf(chp,"lcitems.curr_in_from_pwm[%d]    : %15d \r\n", db, lcitems.curr_in_from_pwm[db]);
    }   

    else{
      chprintf(chp, "This not good parameters!\r\n");
      chprintf(chp, "(Luxcontrol 1-x) lc n\r\n");
      return;
    }

    chThdSleepMilliseconds(100);
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
      chprintf(chp,"Module 1 Temp: %15d \r\n", mlitems.MODULE1_TEMP);
      chprintf(chp,"Module 8 Temp: %15d \r\n", mlitems.MODULE8_TEMP);
      chprintf(chp,"Module 6 Temp: %15d \r\n", mlitems.MODULE6_TEMP);
      chprintf(chp,"Module 12 Temp: %15d \r\n", mlitems.MODULE12_TEMP);
      chprintf(chp,"Module 2 Temp: %15d \r\n", mlitems.MODULE2_TEMP);
      chprintf(chp,"SUNMODULES CURRENT: %15d \r\n", mlitems.sun_current);
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
      chprintf(chp,"custom_flag_1      : %15d \r\n", bmsitems.custom_flag_1);
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
      chprintf(chp,"custom_flag_2      : %15d \r\n", bmsitems.custom_flag_2);
      chprintf(chp,"\r\n----------- Messages 5 -------------\r\n");
      chprintf(chp,"minimum_cell       : %15d \r\n", bmsitems.minimum_cell);
      chprintf(chp,"high_cell_volt_num : %15d \r\n", bmsitems.high_cell_volt_num);
      chprintf(chp,"low_cell_volt_num  : %15d \r\n", bmsitems.low_cell_volt_num);
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

  char c;
  int16_t db;

  db = atol(argv[0]);

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  
  while (TRUE) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if (argc == 1){
      chnReadTimeout(&SD1, (uint8_t *)&c, 1, 100);

      if (c == 97){
        db--;
        c = 0;
      }

      else if (c == 100){
        db++;
        c = 0;
      }
      else if (c == 13){
        break;
      }
      
      db = db > (sizeof(cellitems.id) / 4) ? (sizeof(cellitems.id) / 4) : db;
      db = db < 1 ? 1 : db;

      chprintf(chp,"------------------------------------------------\r\n");
      chprintf(chp,"|                <-  (EXIT)  ->                |\r\n");
      chprintf(chp,"|               (a)  (ENTER) (d)               |\r\n");
      chprintf(chp,"-------------- Bettery Cell %d/%d --------------\r\n", db, 34);
      chprintf(chp,"cellitems.id[%d]              : %15x \r\n", db - 1, cellitems.id[db - 1]);
      chprintf(chp,"cellitems.cell_voltage[%d]    : %15d \r\n", db - 1, cellitems.cell_voltage[db - 1]);
      chprintf(chp,"cellitems.cell_resistant[%d]  : %15d \r\n", db - 1, cellitems.cell_resistant[db - 1]);
      chprintf(chp,"cellitems.open_voltage[%d]    : %15d \r\n", db - 1, cellitems.open_voltage[db - 1]);
    }   

    else{
      chprintf(chp, "This not good parameters!\r\n");
      chprintf(chp, "(Cell 1-x) lc n\r\n");
      return;
    }
    chThdSleepMilliseconds(100);
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
    /*if(can_transmit){
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
    }*/
    //chThdSleepMilliseconds(10);
  }
}
