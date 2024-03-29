/**********************************************************************************************************************/
/*  Hello_World.c   -  Copyright Sierra Wireless Inc. (c) 2010														  */
/*                                                                                                                    */
/*                                                                                                                    */
/* LICENSE                                                                                                            */
/* =======                                                                                                            */
/* If no specific agreement has been signed, this Source Code is subject to the following license terms:              */
/* http://www.sierrawireless.com/productsandservices/AirPrime/Sierra_Wireless_Software_Suite/License/Userlicense.aspx */
/* If you modify this file, you are subject to the contribution rules described into this license                     */
/**********************************************************************************************************************/


/***************************************************************************/
/*  File       : Hello_World.c                                             */
/*-------------------------------------------------------------------------*/
/*  Object     : Customer application                                      */
/*                                                                         */
/*  contents   : Customer main procedures                                  */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*  Include files                                                          */
/***************************************************************************/
#include "at_cmd_service.h"

#include "adl_global.h"
#include "wip.h"

#include "wm_types.h"
#include "wm_stdio.h"
#include "wm_list.h"
#include "adl_str.h"
#include "adl_port.h"
#include "adl_at.h"
#include "adl_traces.h"
#include "adl_memory.h"
#include "adl_flash.h"
#include "adl_fcm.h"
#include "adl_AppliInit.h"
#include "adl_UnSoHandler.h"
#include "adl_RspHandler.h"
#include "adl_CmdHandler.h"
#include "adl_TimerHandler.h"
#include "adl_gprs.h"
#include "ip_ping.h"



#ifndef __GNU_GCC__
/*****************************************************************************/
/*  Macro   : DECLARE_CALL_STACK                                             */
/*---------------------------------------------------------------------------*/
/*  Object  : The GCC compiler and GNU Newlib (standard C library)           */
/*            implementation require more stack size than ARM compilers. If  */
/*            the GCC compiler is used, the Open AT� application has to be   */
/*            declared with greater stack sizes.                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilization                             */
/*--------------------+---+---+---+------------------------------------------*/
/*  X                 | X |   |   |  required stack size                     */
/*--------------------+---+---+---+------------------------------------------*/
#define DECLARE_CALL_STACK(X)   (X)
#else /* #ifndef __GNU_GCC__ */
#define DECLARE_CALL_STACK(X)   (X*3)
#endif /* #ifndef __GNU_GCC__ */

u8  ContextID;
s8  Mode;
u16 PingInterval;
u32 DestAddress;
u32 DestDataSize;
u32 PingPacketSize;
s8 FCMHandler;

// Application tasks prototypes
extern void main_task ( void );

// Application tasks declaration table
const adl_InitTasks_t adl_InitTasks [] =
{
    { main_task,  DECLARE_CALL_STACK ( 8192 ), "main", 1 },
    { 0, 0, 0, 0 }
};



/***************************************************************************/
/*  Function   : HelloWorld_TimerHandler                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : Timer handler                                             */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  ID                |   |   |   | Timer ID                               */
/*--------------------+---+---+---+----------------------------------------*/
/*  Context           |   |   |   | Context                                */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
//void HelloWorld_TimerHandler ( u8 ID, void * Context )
void HelloWorld_TimerHandler ( adl_atCmdPreParser_t *paras )
{
    /* Send a trace: Hello World */
    //TRACE (( 1, "Embedded : Hello World" ));
    /* Send a response: Hello World */
    //adl_atSendResponse ( ADL_AT_UNS, "Cek Signal dulu ....Hello World \r\n" );

    s8 s8Return = 0;
    s8Return = cek_signal();

    //TRACE (( APPLI_AT_CMD_TRACE_LEVEL, "atCmdCreate AT+CSQ : %d", s8Return ));
}



/***************************************************************************/
/*  Function   : main_task                                                 */
/*-------------------------------------------------------------------------*/
/*  Object     : Customer application initialisation                       */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************/

//extern void CfgGprs ( void ( *EntryPoint ) ( void ) ) ;
extern void CfgGprs ( void ) ;

void main_task ( void )		{
	adl_InitType_e adl_InitType = adl_InitGetType ();
    //TRACE (( 1, "Embedded : Appli Init" ));

    s16 s16Return = 0;
    init_csq_gprs();
    //s16Return = subscribe_csq();
    subscribe_flash();
    init_flash();

    TRACE( ( APPLI_AT_CMD_TRACE_LEVEL, "ATCmdCSQ_Subscription: error in subscription: %d", s16Return ) );

    if ( 0 > s16Return )    {
    	adl_atSendResponse ( ADL_AT_UNS, "masuk csq subscribe > 0\r\n\r\n" );
    }
    else {
    	adl_atSendResponse ( ADL_AT_UNS, "masuk csq subscribe NOL\r\n\r\n" );
    }

    /* Set 1s cyclic timer */
    //adl_tmrSubscribe ( TRUE, 50, ADL_TMR_TYPE_100MS, HelloWorld_TimerHandler );

    s8 ret;
    ret = wip_netInitOpts ( WIP_NET_OPT_END );

    if ( OK == ret )    {
    	CfgGprs () ;
    }
}
