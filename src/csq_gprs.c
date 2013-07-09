/*
 * csq_gprs.c
 *
 *  Created on: Jul 4, 2013
 *      Author: Afrendy Bayu
 */

#include "adl_global.h"
#include "at_cmd_service.h"
#include "csq_gprs.h"
#include "flash.h"



s8 iRssiSIM;
s8 iBerSIM;
s8 signalOK;
s8 flagGPRS;

adl_gprsInfosCid_t InfosCid;

static adl_tmr_t* PingTimer;
static adl_port_e PingResponsePort;
static u32 PingNbPackets;

void ATCmdCSQ_Handler ( adl_atCmdPreParser_t *paras );

void cbPingCmdHandler ( adl_atCmdPreParser_t *paras )
{
    ascii RspBuffer [ PG_RSP_BUFF_SIZE ] = { PG_ZERO };
    ascii IPAddress [ PG_IP_ADD_SIZE ] = { PG_ZERO };
    s8 Ret = OK;

    switch ( paras->Type )    {
        case ADL_CMD_TYPE_TEST :
        {
            wm_sprintf ( RspBuffer, "+WDATA: (0-3),(15),(1-4),(%d-u32),"
                "(%d-1500),(1-255)\r\n", ip_get_header_size ( ), ip_get_header_size ( ) );

            /* Send intermediate response */
            adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ), RspBuffer );
            /* Send terminal OK  response */
            adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port,
                            ADL_AT_RSP ), ADL_STR_OK );
        }
        break;

        case ADL_CMD_TYPE_READ :
        {
	    wm_sprintf ( RspBuffer, "+WDATA: %d,\"%s\",%d,%d,%d,%d\r\n",
                            PingSend, ConvertIPitoa ( DestAddress, IPAddress ),
                            ContextID, DestDataSize, PingPacketSize, PingInterval );

            /* Send intermediate response */
            adl_atSendResponsePort ( ADL_AT_INT, paras->Port, RspBuffer );
            /* Send terminal OK  response */
            adl_atSendStdResponsePort ( ADL_AT_RSP, paras->Port, ADL_STR_OK );
        }
        break;

        case ADL_CMD_TYPE_PARA :
        {
            Ret = HandlePingCmd ( paras );
            TRACE ( ( 2, "fnHandlePingCmd Returns : %d", Ret ) );
        }
        break;

        default :
        {
            TRACE ( ( 2, "[cbPingCmdHandler] Default cmd type" ) );
        }
        break;
    }
}

void init_csq_gprs()	{
	signalOK = ERROR;
	flagGPRS = ERROR;
}

s8 ValidateContextID ( ascii *Cid )
{
    /* Try to update CID */
    if ( ( Cid == NULL ) || ( wm_isnumstring ( Cid ) == FALSE )
                    || ( ( ContextID = wm_atoi ( Cid ) ) == PG_ZERO )
                    || ( CONTEXT_NB < ContextID ) )
    {
        /* Invalid gprs context id */
        return ERROR;
    }
    else
    {
        /* valid gprs context id */
        return OK;
    }
}

void InitWdataParams ( void )	{
    /* Initialize the AT+WADATA command parameters */
    ContextID = PG_DEFAULT_CTX_ID;
    PingInterval = PG_DEFAULT_PING_INTERVAL;
    PingPacketSize = PG_DEFAULT_PINGPACKET_SIZE;
    DestDataSize = PG_DEFAULT_DATA_SIZE;
}



s8 HandlePingCmd ( adl_atCmdPreParser_t *paras )
{
    adl_gprsSetupParams_t SetupParams;
    ascii *IP = NULL, *DataSize = NULL, *PacketSize = NULL, *TimerInterval =
                    NULL;
    u32 PingTotalSizeold = DestDataSize; /* temporarily store */
    u32 PingPacketSizeold = PingPacketSize;

    u8 *Ptr8 = NULL;
    u16 *Ptr16 = NULL;
    u32 *Ptr32 = NULL;

    ascii *Param0 = NULL;
    ascii *Param2 = NULL;

    TRACE ( ( 6, "fnHandlePingCmd" ) );

    /* Initialize the members of GPRS settings */
    SetupParams.APN = NULL;
    SetupParams.FixedIP = NULL;
    SetupParams.Login = NULL;
    SetupParams.Password = NULL;

    /* Retrieve and validate parameters */
    Param0 = ADL_GET_PARAM ( paras, PG_FIRST_PARAM );

    if ( ValidateMode ( Param0 ) == ERROR )
    {
        /* Invalid mode - Return Error */
        adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                        ADL_AT_RSP ), ADL_STR_CME_ERROR, PG_INVALID_ERR_PARAM );
        return ERROR;
    }

    TRACE ( ( 6, "New Mode: %d", Mode ) );

    /* In case of Disable mode no need of rest of the parameters */
    Param2 = ADL_GET_PARAM ( paras, PG_THIRD_PARAM );

    if ( ( paras->NbPara > PG_ONE ) && ( ValidateContextID ( Param2 ) == ERROR ) )
    {
        /* Invalid Context ID - Return Error */
        TRACE ( ( 6, "Invalid Context ID" ) );
        adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                        ADL_AT_RSP ), ADL_STR_CME_ERROR, PG_INVALID_ERR_PARAM );
        return ERROR;
    }
    /* Context parameter is valid or not specified (default) */
    TRACE ( ( 6, "New CID: %d", ContextID ) );

    /* Update flash */
    Ptr8 = &ContextID;
    UpdateFlashParameter ( ( u16 ) PING_FLHID_CID, PG_ZERO, ( ascii** ) &Ptr8,
                    TRUE );

    switch ( Mode )
    {
        case PING_DISABLE_GPRS_SESSION :
        {
            /* Stop the application */
            TRACE ( ( 6, "Disable GPRS Session" ) );
            PingSend = PG_ZERO;
            PingRec = PG_ZERO;
            adl_gprsDeactExt ( ContextID, paras->Port );
            adl_fcmUnsubscribe ( FCMHandler );
        }
        break;

        case PING_PROCESS_GPRS_SESSION_ACTIVATION :
        case PING_START_GPRS_SESSION :
        case PING_RESTART_GPRS_SESSION :
        {
            /* Refresh setup parameter for selected CID */
            RefreshSetupParams ( ContextID, &SetupParams );

            /* Check parameters */
            if ( SetupParams.APN == NULL )
            {
                TRACE ( ( 6, "APN parameter not set" ) );
                adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                                ADL_AT_RSP ), ADL_STR_CME_ERROR, PG_INVALID_ERR_OPERATION );

                /* Release setup params */
                ReleaseSetupParams ( &SetupParams );
                return ERROR;
            }

            TRACE ( ( 6, SetupParams.APN ) );

            /* Get IP address */
            IP = ADL_GET_PARAM ( paras, PG_SECOND_PARAM );
            if ( ValidateIPAddress ( IP ) == ERROR )	{
                TRACE ( ( 6, "Invalid IP Address" ) );

                /* Release setup params */
                ReleaseSetupParams ( &SetupParams );

                adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                                ADL_AT_RSP ), ADL_STR_CME_ERROR,
                                PG_INVALID_ERR_PARAM );
                return ERROR;
            }

            /* Get Total size */
            DataSize = ADL_GET_PARAM ( paras, PG_FOURTH_PARAM );
            DestDataSize = ( DataSize ? wm_atoi ( DataSize ) : DestDataSize );

            if ( ip_get_header_size ( ) > DestDataSize )	{
                DestDataSize = PingTotalSizeold;
                TRACE ( ( 6, "Invalid Data Size" ) );
                adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                                ADL_AT_RSP ), ADL_STR_CME_ERROR,
                                PG_INVALID_ERR_PARAM );
                return ERROR;
            }
            TRACE ( ( 6, "Dest data Size : %d", DestDataSize ) );

            /* Update flash */
            Ptr32 = &DestDataSize;
            UpdateFlashParameter ( ( u16 ) PING_FLHID_TOTALSIZE, PG_ZERO,
                            ( ascii** ) &Ptr32, TRUE );

            /* Get Packet size */
            PacketSize = ADL_GET_PARAM ( paras, PG_FIFTH_PARAM );
            PingPacketSize = ( PacketSize ? wm_atoi ( PacketSize ) : PingPacketSize );

            if ( PingPacketSize < ip_get_header_size ( ) )    {
                PingPacketSize = PingPacketSizeold;
                TRACE ( ( 6, "Invalid Ping Packet Size" ) );
                adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                                ADL_AT_RSP ), ADL_STR_CME_ERROR,
                                PG_INVALID_ERR_PARAM );
                return ERROR;
            }
            TRACE ( ( 6, "Ping packet Size : %d", PingPacketSize ) );
            /* Update flash */
            Ptr32 = &PingPacketSize;
            UpdateFlashParameter ( ( u16 ) PING_FLHID_PACKETSIZE, PG_ZERO,
                            ( ascii** ) &Ptr32, TRUE );

            /* Get "inter-packet" timer */
            TimerInterval = ADL_GET_PARAM ( paras, PG_SIXTH_PARAM );
            PingInterval = ( TimerInterval ? wm_atoi ( TimerInterval )
                            : PingInterval );

            /* Update flash */
            Ptr16 = &PingInterval;
            UpdateFlashParameter ( ( u16 ) PING_FLHID_TIMER, PG_ZERO,
                            ( ascii** ) &Ptr16, TRUE );

            /* Start the application */
            StartSession ( &SetupParams, paras->Port );
        }
        break;

        default :
        {
            TRACE ( ( 6, "Default GPRS mode" ) );
        }
        break;
    }

    return OK;
}

s16 subscribe_csq()	{
	s16 s8Return = adl_atCmdSubscribe ( "AT+CSQ", ( adl_atCmdHandler_t ) ATCmdCSQ_Handler,
	                      ADL_CMD_TYPE_ACT );
	return s8Return;
}

s16 cek_signal()
{
	s8 s8Return;
	s8Return = adl_atCmdSend( "AT+CSQ", ( adl_atRspHandler_t ) ATCmdCSQ_Handler, "*", NULL );
	return s8Return;
}

void cbPingTimerHandler ( u8 ID, void * Context )
{
    ascii NewText [ PG_RSP_BUFF_SIZE ] = { PG_ZERO };

    TRACE ( ( 2, "Embedded : Send next packet" ) );

    PingSend++;

    if ( PingSend < PingNbPackets )
    {
        /* One more Packet */
        SendPingPacket ( );
    }
    else
    {
        /* The End */
        wm_sprintf ( NewText, "\r\nResult : Ping Sent %d , "
            "Resp Received %d\r\n", PingNbPackets, PingRec );
        adl_atSendResponse ( ADL_AT_UNS, NewText );
    }
}

bool cbFCMControlHandler ( adl_fcmEvent_e Event )
{
    switch ( Event )
    {
        case ADL_FCM_EVENT_FLOW_OPENNED :
        {
            TRACE ( ( 2, "ADL_FCM_EVENT_FLOW_OPENNED" ) );
            /* Check the request mode */
            if ( Mode == PING_START_GPRS_SESSION )
            {
                /* Sending the First Ping */
                //SendPingPacket ( );
            }
        }
        break;

        case ADL_FCM_EVENT_FLOW_CLOSED :
        {
            TRACE ( ( 2, "ADL_FCM_EVENT_FLOW_CLOSED" ) );
        }
        break;

        case ADL_FCM_EVENT_RESUME :
        {
            TRACE ( ( 2, "ADL_FCM_EVENT_RESUME" ) );
            /* Start Timer */
            PingTimer = adl_tmrSubscribe ( FALSE, PingInterval,
                            ADL_TMR_TYPE_100MS, cbPingTimerHandler );
        }
        break;

        case ADL_FCM_EVENT_MEM_RELEASE :
        {
            TRACE ( ( 2, "ADL_FCM_EVENT_MEM_RELEASE" ) );
        }
        break;

        default :
        {
            TRACE ( ( 2, "Embedded : cbFCMControlHandler "
                                            "Event not processed" ) );
        }
        break;
    }
    return TRUE;
}

bool cbFCMDataHandler ( u16 DataLen, u8 * Data )
{
    u8 Result = PG_ZERO;

    TRACE ( ( 2, "\r\n<- Receive FCM Data Blocks \r\n" ) );
    DUMP ( 2, Data, DataLen );

    /* Check the Ping response */
    Result = ip_check_ping ( Data );

    if ( Result == PG_ZERO )
    {
        adl_atSendResponse ( ADL_AT_UNS, "\r\nPing Rsp OK\r\n" );
    }
    else if ( Result == PG_EIGHT )
    {
        adl_atSendResponse ( ADL_AT_UNS, "\r\nPing Rsp KO : bad chksum\r\n" );
    }
    else if ( Result == PG_SEVEN )
    {
        adl_atSendResponse ( ADL_AT_UNS, "\r\nEcho Reply OK\r\n" );
    }

    /* If we have really received an IP Packet */
    if ( ( Result == PG_ZERO ) || ( Result == PG_EIGHT ) || ( Result
                    == PG_SEVEN ) )
    {
        /* Count one more received packet */
        PingRec++;
    }
    return TRUE;
}

s8 cbGPRSHandler ( u16 Event, u8 Cid )
{
    s8 Return = ADL_GPRS_NO_FORWARD; /* no forward events to ext app */
    ascii RspBuffer [ PG_GPRS_RSP_BUFF_SIZE ] = { PG_ZERO };

    TRACE ( ( 2, "GPRS : Event received %d", Event ) );

    switch ( Event )
    {
        case ADL_GPRS_EVENT_NO_CARRIER :
        case ADL_GPRS_EVENT_DEACTIVATE_OK :
        case ADL_GPRS_EVENT_ANSWER_OK_FROM_EXT :
        case ADL_GPRS_EVENT_DEACTIVATE_OK_FROM_EXT :
        case ADL_GPRS_EVENT_GPRS_DIAL_OK_FROM_EXT :
        case ADL_GPRS_EVENT_HANGUP_OK_FROM_EXT :
        case ADL_GPRS_EVENT_DEACTIVATE_KO_FROM_EXT :
        case ADL_GPRS_EVENT_ACTIVATE_KO_FROM_EXT :
        case ADL_GPRS_EVENT_ME_ATTACH :
        case ADL_GPRS_EVENT_ME_UNREG :
        case ADL_GPRS_EVENT_ME_UNREG_SEARCHING :
        {
            Return = ADL_GPRS_FORWARD;
        }
        break;

        case ADL_GPRS_EVENT_ACTIVATE_OK :
        {
            ascii localIP [ PG_GPRS_PARAM_SIZE ] = { PG_ZERO },
                            DNS1 [ PG_GPRS_PARAM_SIZE ] = { PG_ZERO },
                            DNS2 [ PG_GPRS_PARAM_SIZE ] = { PG_ZERO },
                            Gateway [ PG_GPRS_PARAM_SIZE ] = { PG_ZERO };

            TRACE ( ( 2,
                                            "Embedded : PDP Ctxt %d Activation OK",Cid ) );

            /* OK to AT+WDATA command */
            Return = ADL_GPRS_FORWARD;

            /* Get CID info */
            adl_gprsGetCidInformations ( ContextID, &InfosCid );

            TRACE ( ( 2,
                                            "Embedded  : IPCP Information DNS1 %u, DNS2 %u, "
                                            "Gateway %u", InfosCid.DNS1,
                                            InfosCid.DNS2, InfosCid.Gateway ) );
            wm_sprintf ( RspBuffer,
                            "\r\nIPCP Information :\r\nLocal IP %s\r\nDNS1 "
                                "%s\r\nDNS2 %s\r\nGateway %s\r\n",
                            ConvertIPitoa ( InfosCid.LocalIP, localIP ),
                            ConvertIPitoa ( InfosCid.DNS1, DNS1 ),
                            ConvertIPitoa ( InfosCid.DNS2, DNS2 ),
                            ConvertIPitoa ( InfosCid.Gateway, Gateway ) );
            adl_atSendResponse ( ADL_AT_UNS, RspBuffer );

            /* Open FCM Flows */
            FCMHandler = adl_fcmSubscribe ( ADL_FCM_FLOW_GPRS,
                            cbFCMControlHandler, cbFCMDataHandler );
        }
        break;

        case ADL_GPRS_EVENT_ACTIVATE_OK_FROM_EXT :
        {
            TRACE ( ( 2, "ADL_GPRS_EVENT_ACTIVATE_OK_FROM_EXT" ) );
            adl_atSendResponse ( ADL_AT_UNS,
                            "\r\nADL_GPRS_EVENT_ACTIVATE_OK_FROM_EXT\r\n" );
            Return = ADL_GPRS_FORWARD;
        }
        break;

        case ADL_GPRS_EVENT_ACTIVATE_KO :
        {
            TRACE ( ( 2, "ADL_GPRS_EVENT_ACTIVATE_KO" ) );
            /* ERROR to AT+WDATA command */
            adl_atSendResponsePort ( ADL_AT_INT, PingResponsePort,
                            "\r\nContext activation error\r\n" );
            Return = ADL_GPRS_FORWARD;
        }
        break;

        case ADL_GPRS_EVENT_SETUP_OK :
        {
            TRACE ( ( 2, "PDP Ctxt Cid %d Setup is OK", Cid ) );
            /* Activate context */
            adl_gprsActExt ( ContextID, PingResponsePort );
        }
        break;

        case ADL_GPRS_EVENT_SETUP_KO :
        {
            TRACE ( ( 2, "ADL_GPRS_EVENT_SETUP_KO" ) );
            /* ERROR to AT+WDATA command */
            adl_atSendResponsePort ( ADL_AT_UNS, PingResponsePort,
                            "\r\nContext setup error\r\n" );
            Return = ADL_GPRS_FORWARD;
        }
        break;

        default :
        {
            TRACE ( ( 2, "Default GPRS Event : %d", Event ) );
        }
        break;
    }

    return Return;
}

s16 konek_GPRS()	{
	s16 konek=OK;
	TRACE ( ( 2, "KOnek GPRS----" ) );
	if ( adl_gprsSubscribe ( cbGPRSHandler ) == OK )	    {

	}
	return konek;
}

void aksi_GPRS(){
	if ( (signalOK==OK) && (flagGPRS==ERROR) )	{		// konek GPRS
		konek_GPRS();
	}
	if ( (signalOK==ERROR) && (flagGPRS==OK) )	{		// matikan GPRS

	}
}

void ATCmdCSQ_Handler ( adl_atCmdPreParser_t *paras )
{
    //u8 CID = PG_ZERO;
    ascii * Para = NULL;
    ascii RspBuffer [ PG_RSP_BUFF_SIZE ] = { PG_ZERO };
    ascii rssiSIM[5], berSIM[5];

    TRACE( ( 22, paras->StrData ) );

    //wm_strRemoveCRLF (RspBuffer, paras->StrData, wm_strlen(paras->StrData) );
    if (wm_strlen(paras->StrData)>5)	{
		wm_sprintf ( RspBuffer, "%s" , paras->StrData );
		//adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ), RspBuffer );

		wm_strGetParameterString (rssiSIM, RspBuffer, 1);
		wm_strGetParameterString (berSIM, RspBuffer, 2);
		iRssiSIM = wm_atoi(rssiSIM);
		iBerSIM  = wm_atoi(berSIM);
		if ( (iBerSIM<3) && (iRssiSIM>15) )	{
			signalOK = OK;
		} else {
			signalOK = ERROR;
		}

		wm_sprintf ( RspBuffer, "Hasil CSQ RSSI: %d, BER: %d, signal: %d\r\n" ,
				iRssiSIM, iBerSIM, signalOK );
		adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ), RspBuffer );
		//adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_RSP ), ADL_STR_OK );

		aksi_GPRS();

    }
}
