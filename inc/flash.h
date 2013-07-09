/*
 * flash.h
 *
 *  Created on: Jul 4, 2013
 *      Author: Afrendy Bayu
 */

#ifndef FLASH_H_
#define FLASH_H_

//#include "csq_gprs.h"
#include "adl_global.h"
#include "wm_types.h"
#include "wm_stdio.h"
#include "wm_list.h"
#include "adl_str.h"

#define WDATA_CMD_PARAM_CONFIG  0x0061
#define WSET_CMD_PARAM_CONFIG   0x0081		// 8 jml argumennya !!

/* Parameters of AT+WDATA command */

/* Flash objects handle */
static const ascii* PING_FLH_HANDLE = "WmPING";
static const ascii* PingCmd = "AT+WDATA";
static const ascii* PingConfigCmd = "AT+GSET";

typedef enum	{
    /* Contexts parameters */
	PING_FLHID_NAME,
	PING_FLHID_APN,
    PING_FLHID_LOGIN,
    PING_FLHID_PASSWORD,
    PING_FLHID_IP,
    PING_FLHID_DATA_COMPRESSION,
    PING_FLHID_HEADER_COMPRESSION,
    PING_FLHID_LAST_CONTEXT_PARAM,

    /* Ping parameters */
    PING_FLHID_HOSTADDR = PING_FLHID_LAST_CONTEXT_PARAM,
    PING_FLHID_TOTALSIZE,
    PING_FLHID_PACKETSIZE,
    PING_FLHID_TIMER,
    PING_FLHID_CID,

    PING_FLHID_LAST
} ping_FlashIds_e;

typedef enum	{
    PING_FORMAT_STR,
    PING_FORMAT_U8,
    PING_FORMAT_U16,
    PING_FORMAT_U32
} PingFlashFormat_e;


static const PingFlashFormat_e Ping_FormatsTable [ PING_FLHID_LAST ] = {
/* PING_FLHID_NAME                */PING_FORMAT_STR,
/* PING_FLHID_APN                 */PING_FORMAT_STR,
/* PING_FLHID_LOGIN               */PING_FORMAT_STR,
/* PING_FLHID_PASSWORD            */PING_FORMAT_STR,
/* PING_FLHID_IP                  */PING_FORMAT_STR,
/* PING_FLHID_DATA_COMPRESSION    */PING_FORMAT_U8,
/* PING_FLHID_HEADER_COMPRESSION  */PING_FORMAT_U8,
/* PING_FLHID_HOSTADDR            */PING_FORMAT_U32,
/* PING_FLHID_TOTALSIZE           */PING_FORMAT_U32,
/* PING_FLHID_PACKETSIZE          */PING_FORMAT_U16,
/* PING_FLHID_TIMER               */PING_FORMAT_U8,
/* PING_FLHID_CID                 */PING_FORMAT_U8 };


#define PING_FLHID_COUNT    ( ( PING_FLHID_LAST_CONTEXT_PARAM * CONTEXT_NB ) \
                                + ( PING_FLHID_LAST - \
                                    PING_FLHID_LAST_CONTEXT_PARAM ) )
//6*4 + 11-6 = 24 + 5 = 29

void cbPingCmdHandler ( adl_atCmdPreParser_t *paras );
void cbPingConfigCmdHandler ( adl_atCmdPreParser_t * paras );

void init_baca_flash();
void ReleaseSetupParams ( adl_gprsSetupParams_t * SetupParams );
void UpdateFlashParameter ( u16 id, u8 CID, ascii ** ParamStr, bool bWrite );


s8 ValidateContextID ( ascii *Cid );
void RefreshSetupParams ( u8 CID, adl_gprsSetupParams_t * SetupParams );

#endif /* FLASH_H_ */
