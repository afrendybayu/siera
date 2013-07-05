/**********************************************************************************************************************/
/*  at_cmd_service.h -  Copyright Sierra Wireless Inc. (c) 2010                                                       */
/*                                                                                                                    */
/*                                                                                                                    */
/* LICENSE                                                                                                            */
/* =======                                                                                                            */
/* If no specific agreement has been signed, this Source Code is subject to the following license terms:              */
/* http://www.sierrawireless.com/productsandservices/AirPrime/Sierra_Wireless_Software_Suite/License/Userlicense.aspx */
/* If you modify this file, you are subject to the contribution rules described into this license                     */
/**********************************************************************************************************************/

/*****************************************************************************/
/*  Includes                                                                 */
/*****************************************************************************/

#ifndef HELLO_H_
#define HELLO_H_

#include "wm_types.h"       /* Sierra Wireless types definitions             */
#include "wm_stdio.h"       /* Sierra Wireless standard input/output         */
                            /* definitions                                   */
#include "wm_list.h"        /* Sierra Wireless List and Stack API definitions*/
#include "adl_str.h"        /* String service                                */
#include "adl_port.h"       /* Port service                                  */
#include "adl_at.h"         /* AT commands related structures definition     */
#include "adl_TimerHandler.h" /* Timer service                               */
#include "adl_flash.h"

#include "adl_traces.h"     /* Traces macros definition                      */
#include "adl_memory.h"     /* Memory management functions                   */
#include "adl_error.h"      /* Error functions and definitions               */

#include "adl_AppliInit.h"   /* Application initialization definitions       */
#include "adl_UnSoHandler.h" /* AT command unsolicited response definitions  */
#include "adl_RspHandler.h"  /* AT command response definitions              */
#include "adl_CmdHandler.h"  /* AT command service definitions               */
#include "adl_CmdStackHandler.h"  /* AT command service definitions          */

#include "adl_gprs.h"


#define PG_ZERO                     0
#define PG_ONE                      1
#define PG_TWO                      2
#define PG_THREE                    3
#define PG_FOUR                     4
#define PG_SEVEN                    7
#define PG_EIGHT                    8
#define PG_RSP_BUFF_SIZE            100

#define PG_MASK                           0xFF
#define PG_IP_FIRST_BYTE_COMPARATOR       0x2E
#define PG_SHIFT_ONE_BYTES                 8
#define PG_SHIFT_TWO_BYTES                16
#define PG_SHIFT_THREE_BYTES              24

/* Traces level */
#define APPLI_AT_CMD_TRACE_LEVEL 2

/* Parameters sequence numbers */
#define PG_FIRST_PARAM                     0
#define PG_SECOND_PARAM                    1
#define PG_THIRD_PARAM                     2
#define PG_FOURTH_PARAM                    3
#define PG_FIFTH_PARAM                     4
#define PG_SIXTH_PARAM                     5
#define PG_SEVENTH_PARAM                   6
#define PG_EIGTH_PARAM                     7

#define PG_INVALID_ERR_PARAM               3

u32 ConvertIPatoi ( ascii * aIP );

s16 cek_signal();
s16 subscribe_csq();
s16 init_flash();
void subscribe_flash();

#endif

