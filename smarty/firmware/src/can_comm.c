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


//#define CAN_MIN_EID         0x10
//#define CAN_MAX_EID         0x1FFFFFF

#define CAN_BMS_MIN         0x00
#define CAN_BMS_MAX         0x0F

#define CAN_SM_MIN          0x10
#define CAN_SM_MAX          0x1F
#define CAN_SM_EID          0x10
#define CAN_SM_MESSAGES_1   0x41
#define CAN_SM_MESSAGES_2   0x42
#define CAN_SM_MESSAGES_3   0x43
#define CAN_SM_MESSAGES_4   0x44
#define CAN_SM_MESSAGES_5   0x45
#define CAN_SM_MESSAGES_6   0x46
#define CAN_SM_MESSAGES_7   0x47
#define CAN_SM_MESSAGES_8   0x48
#define CAN_SM_MESSAGES_9   0x49
#define CAN_SM_MESSAGES_10  0x4A
#define CAN_SM_MESSAGES_11  0x4B
#define CAN_SM_MESSAGES_12  0x4C
#define CAN_SM_MESSAGES_13  0x4D
#define CAN_SM_MESSAGES_14  0x4E
#define CAN_SM_MESSAGES_15  0x4F

#define CAN_ML_MIN          0x20
#define CAN_ML_MAX          0x2F
#define CAN_ANS_BRAKE_ON    0x02
#define CAN_ANS_BRAKE_OFF   0x03
#define CAN_ANS_LIGHT_ON    0x04
#define CAN_ANS_LIGHT_OFF   0x05
#define CAN_ANS_TIME        25

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

enum canMessages
{
  CAN_MESSAGES_1,
  CAN_MESSAGES_WAIT,
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
static uint32_t brakeon_ans_time;
static uint32_t brakeoff_ans_time;
static uint32_t lampon_ans_time;
static uint32_t lampoff_ans_time;
static int tx_status;
static bool can_newdata;
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

      rx_id = rxmsg.EID >> 8;
      messages = (uint8_t)rxmsg.EID;
      can_newdata = TRUE;

      if(rx_id >= CAN_BMS_MIN && rx_id <= CAN_BMS_MAX){
        canstate = CAN_BMS;
      }
      else if(rx_id >= CAN_SM_MIN && rx_id <= CAN_SM_MAX){
        canstate = CAN_SM;
      }
      else if(rx_id >= CAN_ML_MIN && rx_id <= CAN_ML_MAX){
        canstate = CAN_ML;
      }
      else if(rx_id >= CAN_RPY_MIN && rx_id <= CAN_RPY_MAX){
        canstate = CAN_RPY;
      }
      else if(rx_id >= CAN_LC_MIN && rx_id <= CAN_LC_MAX){
        canstate = CAN_LC;
      }
      else
        canstate = CAN_WAIT;

      switch(canstate){

          case CAN_BMS:
            canstate = CAN_WAIT;
            break;

          case CAN_SM:
            canstate = CAN_WAIT;
            break;

          case CAN_ML:
            if(messages == CAN_ANS_BRAKE_ON){
              brakeon_ans_time = CAN_ANS_TIME + 10;
            }

            if(messages == CAN_ANS_BRAKE_OFF){
              brakeoff_ans_time = CAN_ANS_TIME + 10;
            }

            if(messages == CAN_ANS_LIGHT_ON){
              lampon_ans_time = CAN_ANS_TIME + 10;
            }

            if(messages == CAN_ANS_LIGHT_OFF){
              lampoff_ans_time = CAN_ANS_TIME + 10;
            }

            canstate = CAN_WAIT;
            break;

          case CAN_RPY:
            canstate = CAN_WAIT;
            break;

          case CAN_LC:
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

            if(messages == CAN_LC_MESSAGES_2){
              for(i = 0; i < sizeof(lcitems); i++){
                if (rx_id == lcitems[i].id)
                {
                  lcitems[i].status = rxmsg.data16[0];
                  lcitems[i].pwm    = rxmsg.data16[1];
                }
              }
            }
            if(messages == CAN_LC_MESSAGES_3){
              
            }
            canstate = CAN_WAIT;
            break;

          case CAN_WAIT:
            break;

          default:
            break;
        }
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
  chRegSetThreadName("transmitter");

  while (!chThdShouldTerminate()) {
    
    switch(tx_status){
      case CAN_MESSAGES_1:
        /* Message 1 */
        txmsg.EID = 0;
        txmsg.EID = CAN_SM_MESSAGES_1;
        txmsg.EID += CAN_SM_EID << 8;

        txmsg.data8[0] = 0xAA;
        txmsg.data8[1] = 0;
        txmsg.data16[1] = 0;
        txmsg.data32[1] = 0;
         
        canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
        tx_status = CAN_MESSAGES_WAIT;
        break;
      case CAN_MESSAGES_WAIT:
        /* Message wait */
        break;

      default:      
        break;
    }
    chThdSleepMilliseconds(100);
  }
  return 0;
}

void can_commInit(void){
  canStart(&CAND1, &cancfg);
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
  brakeon_ans_time = CAN_ANS_TIME + 10;
  brakeoff_ans_time = CAN_ANS_TIME + 10;
  lampon_ans_time = CAN_ANS_TIME + 10;
  lampoff_ans_time = CAN_ANS_TIME + 10;
  tx_status = CAN_MESSAGES_WAIT;

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;

  rxmsg.IDE = CAN_IDE_EXT;
  rxmsg.RTR = CAN_RTR_DATA;
  rxmsg.DLC = 8;

  can_newdata = FALSE;
}

void can_commCalc(void){
  brakeon_ans_time++;
  brakeoff_ans_time++;
  lampon_ans_time++;
  lampoff_ans_time++;

  if (brakeon_ans_time == CAN_ANS_TIME)
  {
    can_lightBreakOn();
  }

  if (brakeoff_ans_time == CAN_ANS_TIME)
  {
    can_lightBreakOff();
  }

  if (lampon_ans_time == CAN_ANS_TIME)
  {
    can_lightPosLampOn();
  }

  if (lampoff_ans_time == CAN_ANS_TIME)
  {
    can_lightPosLampOff();
  }
}

void can_lightRight(void){
  txmsg.EID = 0x00001001;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
}

void can_lightLeft(void){
  txmsg.EID = 0x00001002;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
}

void can_lightWarning(void){
  txmsg.EID = 0x00001003;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
}

void can_lightBreakOn(void){
  txmsg.EID = 0x00001004;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
  brakeon_ans_time = 0;
  brakeoff_ans_time = CAN_ANS_TIME + 10;
}
void can_lightBreakOff(void){
  txmsg.EID = 0x00001005;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
  brakeoff_ans_time = 0;
  brakeon_ans_time = CAN_ANS_TIME + 10;
}

void can_lightPosLampOn(void){
  txmsg.EID = 0x00001006;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
  lampon_ans_time = 0;
  lampoff_ans_time = CAN_ANS_TIME + 10;
}
void can_lightPosLampOff(void){
  txmsg.EID = 0x00001007;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x55AA55AA;
  canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
  lampoff_ans_time = 0;
  lampon_ans_time = CAN_ANS_TIME + 10;
}

void cmd_can_commvalues(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);
    chprintf(chp,"RX.EID : %x \r\n",rxmsg.EID);
    chprintf(chp,"RX.DATA1 : %d \r\n",rxmsg.data16[0]);
    chprintf(chp,"RX.DATA2 : %d \r\n",rxmsg.data16[1]);
    chprintf(chp,"RX.DATA3 : %d \r\n",rxmsg.data16[2]);
    chprintf(chp,"RX.DATA4 : %d \r\n",rxmsg.data16[3]);

    chThdSleepMilliseconds(1000);
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
      chprintf(chp,"rxmsg.EID     : %15d \r\n",rxmsg.EID);
      chprintf(chp,"rxmsg.data16[0]: %15d \r\n",rxmsg.data16[0]);
      chprintf(chp,"rxmsg.data16[1]: %15d \r\n",rxmsg.data16[1]);
      chprintf(chp,"rxmsg.data16[2]: %15d \r\n",rxmsg.data16[2]);
      chprintf(chp,"rxmsg.data16[3]: %15d \r\n",rxmsg.data16[3]);
    }

    else if ((argc == 1) && (strcmp(argv[0], "32") == 0)){

      chprintf(chp,"-------------- CAN all data --------------\r\n");
      chprintf(chp,"rxmsg.EID     : %15d \r\n",rxmsg.EID);
      chprintf(chp,"rxmsg.data32[0]: %15d \r\n",rxmsg.data32[0]);
      chprintf(chp,"rxmsg.data32[1]: %15d \r\n",rxmsg.data32[1]);
    }

    else{
      chprintf(chp, "This not good parameters! (canall 8/16/32)\r\n");
      return;
    }
  }
}

void cmd_candata(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;

  int16_t db;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);

    if ((argc == 2) && (strcmp(argv[0], "lc") == 0)){
      
      db = atol(argv[1]);

      chprintf(chp,"-------------- luxControl %d/%d --------------\r\n", db, sizeof(lcitems) / 16);
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
      return;
    }
  }
}

void cmd_lcSleep(BaseSequentialStream *chp, int argc, char *argv[]){
  tx_status = CAN_MESSAGES_1;
  chprintf(chp,"Sleep 1. LC!\r\n");
}

void cmd_canmonitor(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;

  int16_t db;

  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    //chprintf(chp, "\x1B[%d;%dH", 0, 0);
    if(can_newdata){
      chprintf(chp,"%5x - %4x %4x %4x %4x %4x %4x %4x %4x\r\n", 
        rxmsg.EID,
        rxmsg.data8[0],
        rxmsg.data8[1],
        rxmsg.data8[2],
        rxmsg.data8[3],
        rxmsg.data8[4],
        rxmsg.data8[5],
        rxmsg.data8[6],
        rxmsg.data8[7]);
      can_newdata = FALSE;
    }
  }
}