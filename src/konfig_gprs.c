/*
 * konfig_gprs.c
 *
 *  Created on: Jul 9, 2013
 *      Author: Afrendy Bayu
 */

#include "adl_global.h"
#include "wip.h"

#define ERROR_TRACE_LEVEL       1
#define NORMAL_TRACE_LEVEL		11
#define CREG_POLLING_PERIOD     20
#define REG_STATE_REG   1
#define REG_STATE_ROAM  5

#define ASSERT( pred )  if( !(pred)) TRACE(( ERROR_TRACE_LEVEL, "ASSERTION FAILURE line %i: " #pred "\n", __LINE__))
#define ASSERT_OK( v )  ASSERT( OK == (v))

#define GPRS_APN        "internet"
#define GPRS_USER       "wap"
#define GPRS_PASSWORD   "wap123"
#define GPRS_PINCODE    "1234"
#define GPRS_BEARER     "GPRS"
const ascii * HTTP_STR_URL = "http://google.com";

static void PollCreg ( u8 Id );

typedef struct
{
    wip_channel_t CnxChannel;   /* session channel      */
    wip_channel_t DataChannel;  /* data channel         */
    u32 dataLength;             /* response data length */
} http_ClientTestCtx_t;

http_ClientTestCtx_t http_ClientTestCtx;



static void http_ClientTestDataHandler ( wip_event_t *ev, void *ctx )
{
    ascii tmpbuf [ 256 ];
    ascii Ptr_OnTrace [ 240 ];
    int len, tmplen;
    s32 status;
    http_ClientTestCtx_t *pHttpClientCtx = ( http_ClientTestCtx_t * ) &http_ClientTestCtx;

    switch ( ev->kind )
    {
        case WIP_CEV_OPEN:
        {
            TRACE ( ( NORMAL_TRACE_LEVEL, "http_ClientTestDataHandler: WIP_CEV_OPEN" ) );
            adl_atSendResponse ( ADL_AT_UNS, "http_ClientTestDataHandler: Start\r\n" );
            /* ready for getting response data  */
            pHttpClientCtx->dataLength = 0;
            break;
        }

        case WIP_CEV_READ:
        {
            TRACE ( ( NORMAL_TRACE_LEVEL, "http_ClientTestDataHandler: WIP_CEV_READ" ) );
            /* we must read all available data to trigger WIP_CEV_READ again    */
            tmplen = 0;
            while ( ( len = wip_read ( ev->channel, tmpbuf, sizeof ( tmpbuf ) - 1 ) ) > 0 )
            {
                tmpbuf [ len ] = 0;
                //adl_atSendResponse ( ADL_AT_UNS, tmpbuf );
                tmplen += len;
            }
            TRACE(( NORMAL_TRACE_LEVEL, "http_ClientTestDataHandler: read %d bytes", tmplen ));
            /* compute total length of response */
            pHttpClientCtx->dataLength += tmplen;
            break;
        }

        case WIP_CEV_PEER_CLOSE:
        {
            TRACE ( ( NORMAL_TRACE_LEVEL, "http_ClientTestDataHandler: WIP_CEV_PEER_CLOSE" ) );
            adl_atSendResponse ( ADL_AT_UNS, "\r\nhttp_ClientTestDataHandler: Done\r\n" );

            /* end of data  */
            /* show response information    */
            if ( wip_getOpts ( ev->channel,
                               WIP_COPT_HTTP_STATUS_CODE, &status,
                               WIP_COPT_HTTP_STATUS_REASON, tmpbuf, sizeof ( tmpbuf ),
                               WIP_COPT_END ) == OK )
            {
                ascii sbuf [ 16 ];
                adl_atSendResponse ( ADL_AT_UNS, "http_ClientTestDataHandler: Status=" );
                wm_sprintf ( sbuf, "%d", ( s16 ) status );
                adl_atSendResponse ( ADL_AT_UNS, sbuf );
                adl_atSendResponse ( ADL_AT_UNS, "\r\nhttp_ClientTestDataHandler: Reason=\"" );
                adl_atSendResponse ( ADL_AT_UNS, tmpbuf );
                adl_atSendResponse ( ADL_AT_UNS, "\"\r\n" );
                TRACE ( ( NORMAL_TRACE_LEVEL, "http_ClientTestDataHandler: Status=%d", status ) );
                wm_sprintf ( Ptr_OnTrace, "http_ClientTestDataHandler: Reason=\"%s\"", tmpbuf );
                TRACE ( ( NORMAL_TRACE_LEVEL, Ptr_OnTrace ) );
            }

            if ( wip_getOpts ( ev->channel,
                               WIP_COPT_HTTP_HEADER, "content-type", tmpbuf, sizeof ( tmpbuf ),
                               WIP_COPT_END ) == OK )
            {
                wm_sprintf ( Ptr_OnTrace, "http_ClientTestDataHandler: Content Type=\"%s\"\n", tmpbuf );
                TRACE ( ( NORMAL_TRACE_LEVEL, Ptr_OnTrace ) );
            }
            TRACE ( ( NORMAL_TRACE_LEVEL, "http_ClientTestDataHandler: Response Length=%d bytes", pHttpClientCtx->dataLength ) );

            /* data channel must be closed*/
            wip_close( ev->channel );

            break;
        }

        case WIP_CEV_ERROR:
        {
            TRACE ( ( ERROR_TRACE_LEVEL, "http_ClientTestDataHandler: WIP_CEV_ERROR %d", ev->content.error.errnum ) );
            adl_atSendResponse ( ADL_AT_UNS, "http_ClientTestDataHandler: ERROR\r\n" );
            /* connection to server broken  */
            wip_close( ev->channel);
            break;
        }

        default:
        {
            TRACE ( ( ERROR_TRACE_LEVEL, "http_ClientTestDataHandler: unexpected event: %d", ev->kind ) );
            break;
        }
    }
}

static s32 http_ClientTestCreate ( void )
{
    s32 ret = OK;

    /* HTTP Session creation    */
    http_ClientTestCtx.CnxChannel = wip_HTTPClientCreateOpts ( NULL, NULL,
                                                               WIP_COPT_HTTP_HEADER, "User-Agent", "WIPHTTP/1.0",
                                                               WIP_COPT_END );
    if ( NULL == http_ClientTestCtx.CnxChannel )
    {
        TRACE ( ( ERROR_TRACE_LEVEL, "cannot create http session channel" ) );
        adl_atSendResponse ( ADL_AT_UNS, "cannot create http session channel\r\n" );
        ret = ERROR;
    }
    else
    {
        /* HTTP GET command */
        http_ClientTestCtx.DataChannel = wip_getFileOpts ( http_ClientTestCtx.CnxChannel,   /* session channel  */
                                                           (ascii *) HTTP_STR_URL,          /* requested URL    */
                                                           http_ClientTestDataHandler,      /* data handler     */
                                                           &http_ClientTestCtx,             /* context          */
                                                           /* request headers   */
                                                           WIP_COPT_HTTP_HEADER, "Accept", "text/html",
                                                           WIP_COPT_HTTP_HEADER, "Accept-Language", "fr, en",
                                                           WIP_COPT_END );

        if ( http_ClientTestCtx.DataChannel == NULL )
        {
            TRACE ( ( ERROR_TRACE_LEVEL, "cannot create http data channel" ) );
            adl_atSendResponse ( ADL_AT_UNS, "cannot create http data channel\r\n" );
            wip_close ( http_ClientTestCtx.CnxChannel);
            ret = ERROR;
        }
    }
    return ret;
}

void AppliEntryPoint ( void )	{
    TRACE ( ( NORMAL_TRACE_LEVEL, "HTTP Client Service test application : Init" ) );
    adl_atSendResponse ( ADL_AT_UNS, "\r\nHTTP Client Service test application : Init\r\n" );
    //adl_tmrSubscribe ( TRUE, 50, ADL_TMR_TYPE_100MS, HelloWorld_TimerHandler );
    http_ClientTestCreate ( );
}

static void cbEvhBearer ( wip_bearer_t b, s8 event, void *ctx )		{
    if ( WIP_BEV_IP_CONNECTED == event )    {
        AppliEntryPoint ( ) ;
    }
}

static void OpenAndStartBearer ( void )
{
    s8 bearerRet;
    wip_bearer_t br;

    /* Open the GPRS bearer */
    bearerRet = wip_bearerOpen ( &br, GPRS_BEARER, cbEvhBearer, NULL ) ;
    ASSERT_OK( bearerRet);

    /* Set the bearer configuration */
    bearerRet = wip_bearerSetOpts ( br,
                            WIP_BOPT_GPRS_APN, GPRS_APN,
                            WIP_BOPT_LOGIN, GPRS_USER,
                            WIP_BOPT_PASSWORD, GPRS_PASSWORD,
                            WIP_BOPT_END );
    ASSERT_OK( bearerRet );

    /* Start the bearer */
    bearerRet = wip_bearerStart ( br );
    ASSERT( OK == bearerRet || WIP_BERR_OK_INPROGRESS == bearerRet);
}

static bool cbPollCreg ( adl_atResponse_t *Rsp )	{
    ascii *rsp;
    ascii regStateString [ 3 ];
    s32 regStateInt;

    TRACE ( ( NORMAL_TRACE_LEVEL, "(cbPollCreg) Enter." ) ) ;

    rsp = ( ascii * ) adl_memGet ( Rsp->StrLength );
    wm_strRemoveCRLF ( rsp, Rsp->StrData, Rsp->StrLength );


    TRACE ( ( NORMAL_TRACE_LEVEL, rsp ) ) ;

    wm_strGetParameterString ( regStateString, Rsp->StrData, 2 );
    regStateInt = wm_atoi ( regStateString );

    TRACE ( ( NORMAL_TRACE_LEVEL, regStateString ) ) ;

    if ( REG_STATE_REG == regStateInt || REG_STATE_ROAM == regStateInt )    {
        TRACE( ( NORMAL_TRACE_LEVEL, "(cbPollCreg) Registered on GPRS network." ) ) ;
        /* Registration is complete so open and start bearer */
        OpenAndStartBearer ( ) ;
    }
    else    {
        /* Not ready yet, we'll check again later. Set a one-off timer. */
        adl_tmrSubscribe ( FALSE, CREG_POLLING_PERIOD, ADL_TMR_TYPE_100MS, (adl_tmrHandler_t ) PollCreg );
    }
    adl_memRelease(rsp);
    return FALSE;
}

static void PollCreg ( u8 Id )	{
    adl_atCmdCreate ( "AT+CREG?", FALSE, cbPollCreg, ADL_STR_CREG, NULL );
}

static void cbEvhSim ( u8 event )	{
    TRACE( ( NORMAL_TRACE_LEVEL, "(cbEvhSim) Enter." ) ) ;
    if ( ADL_SIM_EVENT_FULL_INIT == event )    {
        /* argument 0 is dummy, see PollCreg() "Object" comment */
        PollCreg ( 0 );
    }
}

//void CfgGprs ( void ( * EntryPoint ) ( void ) )		{
void CfgGprs ()		{
    TRACE ( ( NORMAL_TRACE_LEVEL, "(CfgGprs) Enter." ) ) ;
    //AppliEntryPoint = EntryPoint;
    adl_simSubscribe ( cbEvhSim, GPRS_PINCODE ) ;
}
