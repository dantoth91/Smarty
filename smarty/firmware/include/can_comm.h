/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef CAN_COMM_H_INCLUDED
#define CAN_COMM_H_INCLUDED

extern struct moduluxItems mlitems;
extern struct bmsItems bmsitems;
extern struct bms_cellItem cellitems;
extern struct luxcontrolItem lcitems;

void can_commInit(void);

void cmd_can_commvalues(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_canall(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_canmppttest(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_candata_lc(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_candata_ml(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_candata_bms(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_candata_cell(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_lcSleep(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_canmonitor(BaseSequentialStream *chp, int argc, char *argv[]);

#endif