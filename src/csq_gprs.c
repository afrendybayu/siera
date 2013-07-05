/*
 * csq_gprs.c
 *
 *  Created on: Jul 4, 2013
 *      Author: Afrendy Bayu
 */

#include "at_cmd_service.h"



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
    //u8 CID = PG_ZERO;
    ascii * Para = NULL;
    ascii RspBuffer [ PG_RSP_BUFF_SIZE ] = { PG_ZERO };

    //TRACE( ( 1, "ATCmdCSQ_Handler: Command Type %d, Received command:", paras->Type ) );
    TRACE( ( 22, paras->StrData ) );
    //TRACE( ( 3, "Cmd Type %d", paras->Type ) );
    //u8 FirstParam = wm_atoi ( ADL_GET_PARAM( paras, 0 ) );
    //TRACE( ( 5, "+CSQ parameter value: %d", FirstParam ) );

    if (strlen(paras->StrData)>5)	{
		wm_sprintf ( RspBuffer, "Hasil CSQ: %s" , paras->StrData );
		//Para = wm_lstGetItem ( paras, PG_FIRST_PARAM );
		//TRACE( ( 2, Para ) );
		///strGetIDExt
		adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ), RspBuffer );
		//adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_RSP ), ADL_STR_OK );
    }

    switch ( paras->Type )
        {
            case ADL_CMD_TYPE_TEST :
            {
            	TRACE( ( 1, "+++++++++++ ATCmdCSQ_Handler ADL_CMD_TYPE_TEST" ) );
            }
            break;

            case ADL_CMD_TYPE_READ :
            {
            	TRACE( ( 1, "+++++++++++ ATCmdCSQ_Handler ADL_CMD_TYPE_READ" ) );
            }
            break;
            case ADL_CMD_TYPE_PARA :
            {
            	TRACE( ( 1, "+++++++++++ ATCmdCSQ_Handler ADL_CMD_TYPE_PARA" ) );
            }
            break;

            case ADL_CMD_TYPE_ACT :
            {
            	TRACE( ( 1, "+++++++++++ ATCmdCSQ_Handler ADL_CMD_TYPE_ACT" ) );
            }
            break;
            case ADL_CMD_TYPE_ROOT :
            {
            	TRACE( ( 1, "+++++++++++ ATCmdCSQ_Handler ADL_CMD_TYPE_ROOT" ) );
            }
            break;
        }
}
