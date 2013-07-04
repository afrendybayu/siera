/*
 * csq_gprs.c
 *
 *  Created on: Jul 4, 2013
 *      Author: Afrendy Bayu
 */

#include "at_cmd_service.h"

static const ascii* PingConfigCmd = "AT+GSET";

void ATCmdCSQ_Handler ( adl_atCmdPreParser_t *paras );

s16 subscribe_csq()	{
	s16 s8Return = adl_atCmdSubscribe ( "AT+CSQ", ( adl_atCmdHandler_t ) ATCmdCSQ_Handler,
	                      ADL_CMD_TYPE_ACT );
	return s8Return;
}

s16 cek_signal()
{
	s8 s8Return;
	s8Return = adl_atCmdSend( "AT+CSQ", ( adl_atRspHandler_t ) ATCmdCSQ_Handler,
                                  "*", NULL );
	return s8Return;
}

void ATCmdCSQ_Handler ( adl_atCmdPreParser_t *paras )
{
    s32 s32Return = 0;
    s8 s8Return = 0;
    ascii RspBuffer [ PG_RSP_BUFF_SIZE ] = { PG_ZERO };

    //TRACE( ( 1, "ATCmdCSQ_Handler: Command Type %d, Received command:", paras->Type ) );
    TRACE( ( 22, paras->StrData ) );
    TRACE( ( 3, "Cmd Type %d", paras->Type ) );
    //u8 FirstParam = wm_atoi ( ADL_GET_PARAM( paras, 0 ) );
    //TRACE( ( 5, "+CSQ parameter value: %d", FirstParam ) );

    if (strlen(paras->StrData)>5)	{
		wm_sprintf ( RspBuffer, "Hasil CSQ: %s" , paras->StrData );
		//adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ), RspBuffer );
		//adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_RSP ), ADL_STR_OK );
    }

    if ( ( 0 > s8Return ) || ( 0 > s32Return ) )
    {
        TRACE( ( APPLI_AT_CMD_TRACE_LEVEL, "ATCmdCSQ_Handler: error in sending the command" ) );
    }
}
