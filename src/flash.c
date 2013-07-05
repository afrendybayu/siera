/*
 * flash.c
 *
 *  Created on: Jul 4, 2013
 *      Author: Afrendy Bayu
 */

#include "at_cmd_service.h"
#include "flash.h"
#include "umum.h"
#include "csq_gprs.h"
#include "adl_gprs.h"

#include "adl_global.h"


/*IP Host Addr from the PING response */

//s8 FCMHandler = OK;
u32 DestAddressFlash;

void simpan_IP(u32 almt)	{
	u32 *Ptr32;

	Ptr32 = &almt;
	UpdateFlashParameter ( ( u16 ) PING_FLHID_HOSTADDR, PG_ZERO,
		                            ( ascii** ) &Ptr32, TRUE );
}


s8 ValidateIPAddress ( ascii *IP )		{
    s8 Ret = OK;
    u32 *Ptr32;
    //u32 DestAddress;

    if ( IP && IP [ PG_ZERO ] )
    {
        if ( adl_gprsIsAnIPAddress ( IP ) )
        {
            /* Convert IP address */
            TRACE ( ( 7, IP ) );

            DestAddress = ConvertIPatoi ( IP );

            /* Update flash */
            Ptr32 = &DestAddress;
            UpdateFlashParameter ( ( u16 ) PING_FLHID_HOSTADDR, PG_ZERO,
                            ( ascii** ) &Ptr32, TRUE );
        }
        else
        {
            /* IP error */
            TRACE ( ( 7, "Invalid IP Address" ) );
            Ret = ERROR;
        }
    }
    else
    {
        /* IP Address Not specified */
        TRACE ( ( 7, "IP Address Not Specified" ) );
    }
    return Ret;
}

s8 ValidateMode ( ascii *ModeParam )
{
    if ( ( ModeParam == NULL ) || ( wm_isnumstring ( ModeParam ) == FALSE )
                    || ( ( Mode = wm_atoi ( ModeParam ) )
                                    >= PING_LAST_PING_MODE ) || ( Mode
                    < PING_DISABLE_GPRS_SESSION ) )
    {
        /* Invalid ping mode */
        TRACE ( ( 7, "Invalid Mode" ) );
        return ERROR;
    }
    else
    {
        /* valid ping mode */
        return OK;
    }
}



void StartSession ( adl_gprsSetupParams_t *SetupParams, u8 Port )
{
    s32 Return = OK;

    PingSend = PG_ZERO;
    PingRec = PG_ZERO;
}





s16 init_flash() {

	s32 IdCount = PG_ZERO;
	adl_flhSubscribe ( ( ascii* ) PING_FLH_HANDLE, PING_FLHID_COUNT );

	IdCount = adl_flhGetIDCount ( ( ascii* ) PING_FLH_HANDLE );
	TRACE ( ( 2, "Subscribed IdCount: %d/%d", IdCount, PING_FLHID_COUNT ) );

	if ( IdCount != PING_FLHID_COUNT )	{
		adl_atSendResponse ( ADL_AT_UNS, "\r\nInit error: please reset "
	                    "flash objects (AT+WOPEN=3)\r\n" );
	    TRACE ( ( 33, "Embedded: Ping Appli flash init error" ) );
	}
	else {
		adl_atSendResponse ( ADL_AT_UNS, "\r\nTransfer Application started\r\n" );
		TRACE ( ( 15, "-------------Embedded : Flash subscribe" ) );
		subscribe_flash();
		TRACE ( ( 15, "-------------Embedded : Flash initWdataParam" ) );
		InitWdataParams ( );
		TRACE ( ( 15, "-------------Embedded : Flash baca init" ) );
		init_baca_flash();

	}


    TRACE ( ( 4, "----Reading Settings from Flash Done----" ) );
    return 0;
}

void init_baca_flash()	{
	u8  *Ptr8;
	u16 *Ptr16;
	u32 *Ptr32;


	TRACE ( ( 44, "----Read Settings from Flash----" ) );
	Ptr32 = &DestAddress;
	UpdateFlashParameter ( ( u16 ) PING_FLHID_HOSTADDR, PG_ZERO,
	                                ( ascii** ) &Ptr32, FALSE );
	Ptr32 = &DestDataSize;
	UpdateFlashParameter ( ( u16 ) PING_FLHID_TOTALSIZE, PG_ZERO,
	                                ( ascii** ) &Ptr32, FALSE );
	Ptr32 = &PingPacketSize;
	UpdateFlashParameter ( ( u16 ) PING_FLHID_PACKETSIZE, PG_ZERO,
	                                ( ascii** ) &Ptr32, FALSE );
	Ptr16 = &PingInterval;
	UpdateFlashParameter ( ( u16 ) PING_FLHID_TIMER, PG_ZERO,
	                                ( ascii** ) &Ptr16, FALSE );
	Ptr8 = &ContextID;
	UpdateFlashParameter ( ( u16 ) PING_FLHID_CID, PG_ZERO,
	                                ( ascii** ) &Ptr8, FALSE );
}

void subscribe_flash()	{
	TRACE ( ( 4, "----subscribe cmd Ping ****" ) );

	adl_atCmdSubscribe ( ( ascii* ) PingCmd, cbPingCmdHandler,
	                                ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_PARA
	                                | ADL_CMD_TYPE_READ | WDATA_CMD_PARAM_CONFIG );

	adl_atCmdSubscribe ( ( ascii* ) PingConfigCmd,
	                                cbPingConfigCmdHandler, ADL_CMD_TYPE_TEST |
	                                	ADL_CMD_TYPE_PARA | ADL_CMD_TYPE_READ | WSET_CMD_PARAM_CONFIG );
}



void RefreshSetupParams ( u8 CID, adl_gprsSetupParams_t * SetupParams )
{
    TRACE ( ( 15, "fnRefreshSetupParams" ) );
    //UpdateFlashParameter ( ( u16 ) PING_FLHID_NAME, CID, &SetupParams->NAME, FALSE );
    UpdateFlashParameter ( ( u16 ) PING_FLHID_APN, CID, &SetupParams->APN, FALSE );
    UpdateFlashParameter ( ( u16 ) PING_FLHID_LOGIN, CID, &SetupParams->Login, FALSE );
    UpdateFlashParameter ( ( u16 ) PING_FLHID_PASSWORD, CID, &SetupParams->Password, FALSE );

    UpdateFlashParameter ( ( u16 ) PING_FLHID_IP, CID, &SetupParams->FixedIP, FALSE );

    UpdateFlashParameter ( ( u16 ) PING_FLHID_DATA_COMPRESSION, CID,
                    ( ascii** ) &SetupParams->DataCompression, FALSE );

    UpdateFlashParameter ( ( u16 ) PING_FLHID_HEADER_COMPRESSION, CID,
                    ( ascii** ) &SetupParams->HeaderCompression, FALSE );
}

void ReleaseSetupParams ( adl_gprsSetupParams_t * SetupParams )
{
    if ( SetupParams->APN )    {
        adl_memRelease ( SetupParams->APN );
    }
    if ( SetupParams->Login )    {
        adl_memRelease ( SetupParams->Login );
    }
    if ( SetupParams->Password )    {
        adl_memRelease ( SetupParams->Password );
    }
    if ( SetupParams->FixedIP )    {
        adl_memRelease ( SetupParams->FixedIP );
    }
}

s8 HandlePingConfigCmdParams ( adl_atCmdPreParser_t * paras )
{
    adl_gprsSetupParams_t SetupParams;
    ascii * Para = NULL;
    u8 CID = PG_ZERO;
    ascii provider [30] = { PG_ZERO };

    wm_memset ( &SetupParams, PG_ZERO, sizeof(adl_gprsSetupParams_t) );

    /* Initialize the members of GPRS settings */
    SetupParams.APN = NULL;
    SetupParams.FixedIP = NULL;
    SetupParams.Login = NULL;
    SetupParams.Password = NULL;

    /* Check parameters */
    /* First Parameter : CID */
    Para = ADL_GET_PARAM ( paras, PG_FIRST_PARAM );

    if ( ( Para == NULL ) || ( wm_isnumstring ( Para ) == FALSE ) || ( PG_ZERO
                    == ( CID = wm_atoi ( Para ) ) ) || ( CID > CONTEXT_NB ) )
    {
        TRACE ( ( 15, "Bad context NB" ) );
        adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                        ADL_AT_RSP ), ADL_STR_CME_ERROR, PG_INVALID_ERR_PARAM );
        return ERROR;
    }

    TRACE ( ( 15, " Context Id : %d", CID ) );

    /* Update Setup Params from required context */
    RefreshSetupParams ( CID, &SetupParams );

    /* Second Parameter : privider */

    Para = ADL_GET_PARAM ( paras, PG_SECOND_PARAM );
    if ( Para == NULL )  {
    	wm_sprintf ( provider, "%s", "-" );
    }	    	/* Update flash */
    TRACE ( ( 15, " >>>>> Provider : %s", provider ) );
    UpdateFlashParameter ( ( u16 ) PING_FLHID_NAME, CID, provider, TRUE );
    TRACE ( ( 15, provider ) );



    /* Third Parameter : apn */
    Para = ADL_GET_PARAM ( paras, PG_THIRD_PARAM );
    if ( Para != NULL )    {
        if ( !Para [ PG_ZERO ] )        {
            TRACE ( ( 15, "Invalid Apn parameter value" ) );
            adl_atSendStdResponseExt ( ADL_AT_PORT_TYPE ( paras->Port,
                            ADL_AT_RSP ), ADL_STR_CME_ERROR, PG_INVALID_ERR_PARAM );

            /* Release memory */
            ReleaseSetupParams ( &SetupParams );
            return ERROR;
        }

        /* Set APN param */
        if ( SetupParams.APN != NULL )
        {
            /* Release old parameter */
            adl_memRelease ( SetupParams.APN );
        }

        /* Copy param */
        SetupParams.APN = ( ascii* ) adl_memGet ( wm_strlen ( Para ) + PG_ONE );
        wm_memset ( SetupParams.APN, PG_ZERO, sizeof ( SetupParams.APN ) );
        wm_strcpy ( SetupParams.APN, Para );

        /* Update flash */
        UpdateFlashParameter ( ( u16 ) PING_FLHID_APN, CID, &SetupParams.APN, TRUE );
        TRACE ( ( 15, SetupParams.APN ) );
    }

    /* Forth Parameter : login */ // EMPAT : 4
    Para = ADL_GET_PARAM ( paras, PG_FOURTH_PARAM );
    if ( Para != NULL )    {
        /* Set Login param */
        if ( SetupParams.Login != NULL )
        {
            /* Release old parameter */
            adl_memRelease ( SetupParams.Login );
        }

        /* Copy param */
        SetupParams.Login = ( ascii* ) adl_memGet ( wm_strlen ( Para ) + PG_ONE );
        wm_memset ( SetupParams.Login, PG_ZERO, sizeof ( SetupParams.Login ) );
        wm_strcpy ( SetupParams.Login, Para );

        /* Update flash */
        UpdateFlashParameter ( ( u16 ) PING_FLHID_LOGIN, CID, &SetupParams.Login, TRUE );
        TRACE ( ( 15, SetupParams.Login ) );
    }

    /* Fifth Parameter : password */
    Para = ADL_GET_PARAM ( paras, PG_FIFTH_PARAM );
    if ( Para != NULL )
    {
        /* Set Password param */
        if ( SetupParams.Password != NULL )  {
            /* Release old parameter */
            adl_memRelease ( SetupParams.Password );
        }

        /* Copy param */
        SetupParams.Password = ( ascii* ) adl_memGet ( ( wm_strlen ( Para )
                        + PG_ONE ) );
        wm_memset ( SetupParams.Password, PG_ZERO, sizeof ( SetupParams.Password ) );
        wm_strcpy ( SetupParams.Password, Para );

        /* Update flash */
        UpdateFlashParameter ( ( u16 ) PING_FLHID_PASSWORD, CID,
                        &SetupParams.Password, TRUE );
        TRACE ( ( 15, SetupParams.Password ) );
    }

    /* Sixth Parameter : fixed IP */
    Para = ADL_GET_PARAM ( paras, PG_SIXTH_PARAM );

    if ( Para != NULL )
    {
        /* Set FixedIP param */
        if ( SetupParams.FixedIP != NULL )
        {
            /* Release old parameter */
            adl_memRelease ( SetupParams.FixedIP );
        }

        /* Copy param */
        SetupParams.FixedIP = ( ascii* ) adl_memGet ( ( wm_strlen ( Para )
                        + PG_ONE ) );

        wm_memset ( SetupParams.FixedIP, PG_ZERO, sizeof ( SetupParams.FixedIP ) );
        wm_strcpy ( SetupParams.FixedIP, Para );

        /* Check IP address (for info) */
        TRACE ( ( 5, "Is an IP address : %d",
                                        adl_gprsIsAnIPAddress ( SetupParams.FixedIP ) ) );
    }

    /* Update flash */
    UpdateFlashParameter ( ( u16 ) PING_FLHID_IP, CID, &SetupParams.FixedIP, TRUE );

    /* Seventh Parameter : Data compression */
    Para = ADL_GET_PARAM ( paras, PG_SEVENTH_PARAM );
    if ( Para != NULL )
    {
        /* Copy param */
        SetupParams.DataCompression = wm_atoi ( Para );

        /* Update flash */
        UpdateFlashParameter ( ( u16 ) PING_FLHID_DATA_COMPRESSION, CID,
                        ( ascii* ) &SetupParams.DataCompression, TRUE );
        TRACE ( ( 5, "Data Compression : %d", SetupParams.DataCompression ) );
    }

    /* Seventh Parameter : Header compression */
    Para = ADL_GET_PARAM ( paras, PG_SEVENTH_PARAM );
    if ( Para )
    {
        /* Copy param */
        SetupParams.HeaderCompression = wm_atoi ( Para );

        /* Update flash */
        UpdateFlashParameter ( ( u16 ) PING_FLHID_HEADER_COMPRESSION, CID,
                        ( ascii* ) &SetupParams.HeaderCompression, TRUE );
        TRACE ( ( 5, "Header Compression : %d", SetupParams.HeaderCompression ) );
    }

    /* Release memory */
    ReleaseSetupParams ( &SetupParams );

    adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port,
                    ADL_AT_RSP ), ADL_STR_OK );

    return OK;

}

void cbPingConfigCmdHandler ( adl_atCmdPreParser_t * paras )
{
    ascii Rsp [ PG_RSP_BUFF_SIZE ] = { PG_ZERO };
    ascii provider [30] = { PG_ZERO };
    u8 Counter = PG_ZERO;
    adl_gprsSetupParams_t SetupParams;
    TRACE( ( 1, "++++++ AT+GSET cbPingConfigCmdHandler: ") );
    TRACE( ( 2, paras->StrData ) );

    /* Initialize the members of GPRS settings */
    SetupParams.APN = NULL;
    SetupParams.FixedIP = NULL;
    SetupParams.Login = NULL;
    SetupParams.Password = NULL;
    SetupParams.DataCompression = 0;
    SetupParams.HeaderCompression = 0;

    switch ( paras->Type )    {
        case ADL_CMD_TYPE_TEST :
        {
            /* Send Intermediate responses */
            adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ),
                            "+GSET: [No],[Provider],[APN],[user],[password],[FixIP],[DataCompres],[HeadrComp]\r\n" );
            adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ),
                            "+GSET: [1-4],str(30),str(30),str(30),str(30),str(15),[0-1],[0-1]\r\n" );

            adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_RSP ), ADL_STR_OK );
        }
        break;

        case ADL_CMD_TYPE_READ :
        {
            /* Loop on contexts */
            for ( Counter = PG_ONE; Counter <= CONTEXT_NB; Counter++ )
            {
                /* Refresh Setup parameters */
                RefreshSetupParams ( Counter, &SetupParams );
                UpdateFlashParameter ( ( u16 ) PING_FLHID_NAME, Counter, provider, FALSE );

                /* Ready to display */

                wm_sprintf ( Rsp, "\r\n+GSET: %d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\r\n",
                				Counter, provider, SetupParams.APN ? SetupParams.APN : "",
                                SetupParams.Login ? SetupParams.Login : "",
                                SetupParams.Password ? SetupParams.Password : "",
                                SetupParams.FixedIP ? SetupParams.FixedIP : "",
                                SetupParams.DataCompression, SetupParams.HeaderCompression );

                /* Send Intermediate responses */
                adl_atSendResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_INT ), Rsp );

                /* Release memory */
                ReleaseSetupParams ( &SetupParams );
            }
            adl_atSendStdResponse ( ADL_AT_PORT_TYPE ( paras->Port, ADL_AT_RSP ), ADL_STR_OK );
        }
        break;

        case ADL_CMD_TYPE_PARA :
        {
        	TRACE( ( 1, "############### HandlePingConfigCmdParams ADL_CMD_TYPE_PARA ##### ") );
            HandlePingConfigCmdParams ( paras );
        }
        break;
    }
}


void UpdateFlashParameter ( u16 id, u8 CID, ascii ** ParamStr, bool bWrite )
{
    /* Get format */
    PingFlashFormat_e Format = Ping_FormatsTable [ id ];
    s32 Length = PG_ZERO;
    if (id == ( u16 ) PING_FLHID_NAME)	{
    	TRACE ( ( 5, "********* provider : %s/%s ***" , ParamStr, *ParamStr ) );
    }

    /* Compute Real flash ID */
    //TRACE ( ( 5, "[fnUpdateFlashParameter] id : %d, CtxId : %d" ,id, CID ) );

    /* Check for non-context flash parameters */
    if ( !CID || ( CID > CONTEXT_NB ) )    {
        /* Offset is CONTEXT_NB - 1 (id should already be >
         * PING_FLHID_LAST_CONTEXT_PARAM ) */
        CID = ( CONTEXT_NB - PG_ONE );
    }
    else    {
        /* Reduce offset by one */
        CID--;
    }

    /* Apply offset */
    id += ( CID * PING_FLHID_LAST_CONTEXT_PARAM );

    /* Check for mode */
    if ( bWrite == TRUE )
    {
        /*  Check for provided string */
        if ( *ParamStr != NULL )
        {
            /* Switch on format */
            TRACE ( ( 15, "Write into Flash" ) );
            switch ( Format )
            {
                case PING_FORMAT_STR :
                {
                    Length = ( wm_strlen ( *ParamStr ) + PG_ONE );
                    adl_flhWrite ( ( ascii* ) PING_FLH_HANDLE, id, Length,
                                    ( u8* ) *ParamStr );
                }
                break;
                case PING_FORMAT_U8 :
                {
                    Length = sizeof(u8);
                    adl_flhWrite ( ( ascii* ) PING_FLH_HANDLE, id, Length,
                                    ( u8 * ) ParamStr );
                }
                break;
                case PING_FORMAT_U16 :
                {
                    Length = sizeof(u16);
                    adl_flhWrite ( ( ascii* ) PING_FLH_HANDLE, id, Length,
                                    ( u8 * ) ParamStr );
                }
                break;
                case PING_FORMAT_U32 :
                {
                    Length = sizeof(u32);
                    adl_flhWrite ( ( ascii* ) PING_FLH_HANDLE, id, Length,
                                    ( u8 * ) ParamStr );
                }
                break;
                default :
                {
                    TRACE ( ( 44, "default format" ) );
                }
                break;
            }
        }
        else
        {
            /* Erase parameter (if any) */
            TRACE ( ( 44, "Erase from Flash" ) );
            adl_flhErase ( ( ascii* ) PING_FLH_HANDLE, id );
        }
    }
    else
    {
        /* Read parameter */
        Length = adl_flhExist ( ( ascii* ) PING_FLH_HANDLE, id );

        /* Check if the object exists */
        if ( Length > PG_ZERO )
        {
            /* Switch on format */
            TRACE ( ( 44, "Read from Flash, Length : %d", Length ) );

            switch ( Format )
            {
                case PING_FORMAT_STR :
                {
                    /* Check if the provided string was allocated */
                    if ( *ParamStr != NULL )   {
                        /* Release the previous string */
                        adl_memRelease ( *ParamStr );
                    }

                    /*  Allocate memory for the parameter */
                    *ParamStr = ( ascii* ) adl_memGet ( Length );

                    /* Read from flash */
                    adl_flhRead ( ( ascii* ) PING_FLH_HANDLE, id, Length,
                                    ( u8* ) *ParamStr );
                    TRACE ( ( 44, *ParamStr ) );
                }
                break;

                case PING_FORMAT_U8 :
                {
                    adl_flhRead ( ( ascii* ) PING_FLH_HANDLE, id, sizeof(u8),
                                    ( u8 * ) ParamStr );
                }
                break;
                case PING_FORMAT_U16 :
                {
                    adl_flhRead ( ( ascii* ) PING_FLH_HANDLE, id, sizeof(u16),
                                    ( u8 * ) ParamStr );
                }
                break;
                case PING_FORMAT_U32 :
                {
                    adl_flhRead ( ( ascii* ) PING_FLH_HANDLE, id, sizeof(u32),
                                    ( u8 * ) ParamStr );
                }
                break;
                default :
                {
                    TRACE ( ( 44, "default format" ) );
                }
                break;
            }
        }
    }
}
