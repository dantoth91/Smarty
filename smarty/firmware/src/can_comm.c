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

#define CAN_MIN_EID         0x10
#define CAN_MAX_EID         0x1FFFFFF

#define CAN_SM_MIN          0x10
#define CAN_SM_MAX          0x1F

#define CAN_ML_MIN          0x20
#define CAN_ML_MAX          0x2F

#define CAN_RPY_MIN         0x30
#define CAN_RPY_MAX         0x3F

#define CAN_LC_MIN          0x40
#define CAN_LC_MAX          0x5F
#define CAN_LC_MESSAGES_1   0x01
#define CAN_LC_MESSAGES_2   0x02

#define CAN_MAX_ADR         0x1FFFFFF

enum canState
{
  CAN_SM,
  CAN_ML,
  CAN_RPY,
  CAN_LC,
  CAN_WAIT,
  CAN_NUM_CH
}canstate;

static CANTxFrame txmsg;
static CANRxFrame rxmsg;

//uint32_t canRxEID;

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
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6),
  0,
  NULL
};

static int seged;
static uint16_t id;
static uint16_t messages;

/*
 * Receiver thread.
 */
static WORKING_AREA(can_rx_wa, 256);
static msg_t can_rx(void *p) {
  EventListener el;
  seged = 0;
  int i;

  (void)p;
  chRegSetThreadName("receiver");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);
  while(!chThdShouldTerminate()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
      continue;
    while (canReceive(&CAND1, &rxmsg, TIME_IMMEDIATE) == RDY_OK) {

      id = rxmsg.EID >> 8;
      messages = (uint8_t)rxmsg.EID;
      
      if(id >= CAN_SM_MIN && id <= CAN_SM_MAX){
        canstate = CAN_SM;
      }
      if(id >= CAN_ML_MIN && id <= CAN_ML_MAX){
        canstate = CAN_ML;
      }
      if(id >= CAN_RPY_MIN && id <= CAN_RPY_MAX){
        canstate = CAN_RPY;
      }
      if(id >= CAN_LC_MIN && id <= CAN_LC_MAX){
        canstate = CAN_LC;
      }
      else
        canstate = CAN_WAIT;

      switch(canstate){
          case CAN_SM:
            seged = 1;
            canstate = CAN_WAIT;
            break;

          case CAN_ML:
            seged = 5;
            canstate = CAN_WAIT;
            break;

          case CAN_RPY:
            seged = 3;
            canstate = CAN_WAIT;
            break;

          case CAN_LC:
            if(messages == CAN_LC_MESSAGES_1){
              for(i = 0; i < sizeof(lcitems); i++){
                if (id == lcitems[i].id)
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
                if (id == lcitems[i].id)
                {
                  lcitems[i].status = rxmsg.data16[0];
                  lcitems[i].pwm    = rxmsg.data16[1];
                }
              }
            }
            seged = 8;
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
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  //canTransmit(&CAND1, &txmsg, MS2ST(100));

  while (!chThdShouldTerminate()) {
    canTransmit(&CAND1, &txmsg, MS2ST(100));
    chThdSleepMilliseconds(100);
  }
  return 0;
}

/*uint8_t canTransmitData(CANTxFrame txmsg){

  canTransmit(&CAND1, &txmsg, MS2ST(100));
  return 0;
}

uint8_t canReceiveData(CANRxFrame *rxmsg){
  
  canReceive(&CAND1, &rxmsg, TIME_IMMEDIATE)
  return 0;

}*/

void can_commInit(void){
  canStart(&CAND1, &cancfg);
	chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
}

void cmd_can_commvalues(BaseSequentialStream *chp, int argc, char *argv[]) {
  
  (void)argc;
  (void)argv;
  chprintf(chp, "\x1B\x63");
  chprintf(chp, "\x1B[2J");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B[%d;%dH", 0, 0);
    chprintf(chp,"RX.EID : %x \r\n",txmsg.EID);
    chprintf(chp,"RX.DATA1 : %x \r\n",txmsg.data32[0]);
    chprintf(chp,"RX.DATA2 : %x \r\n",txmsg.data32[1]);

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
    chprintf(chp,"seged               : %15d \r\n",seged);
    chprintf(chp,"id                  : %15x \r\n",id);
    chprintf(chp,"messages            : %15x \r\n",messages);

    chThdSleepMilliseconds(250);
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

      chprintf(chp,"-------------- luxCControl %d/%d --------------\r\n", db, sizeof(lcitems) / 16);
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