/**********************************************************************************************************************/
/*  IP_PING.C   -  Copyright Sierra Wireless Inc. (c) 2010															  */
/*                                                                                                                    */
/*                                                                                                                    */
/* LICENSE                                                                                                            */
/* =======                                                                                                            */
/* If no specific agreement has been signed, this Source Code is subject to the following license terms:              */
/* http://www.sierrawireless.com/productsandservices/AirPrime/Sierra_Wireless_Software_Suite/License/Userlicense.aspx */
/* If you modify this file, you are subject to the contribution rules described into this license                     */
/**********************************************************************************************************************/

/****************************************************************************/
/* File :   IP_PING.C                                                       */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Closed source - not to be distributed                                    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************/
/*
    $LogWavecom: U:\projet\mmi\pvcsarch\archives\open-at\SAMPLES\Open-AT\_closed\src\ip_ping.c-arc $
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  13.09.05 | DPO    | 1.2            | *   Resolution for 30271: ulong typ
 *           |        |                | e redefinition in IP_Ping code not 
 *           |        |                | supported by new GCC compiler  Reso
 *           |        |                | lution for 30272: ulong type redefi
 *           |        |                | nition in IP_Ping code not supporte
 *           |        |                | d by new GCC compiler              
 * ----------+--------+----------------+-------------------------------------
 *  08.06.05 | DPO    | 1.1            | *   Resolution for 28933: The Open-
 *           |        |                | AT Ping sample has to be more confi
 *           |        |                | gurable                            
 * ----------+--------+----------------+-------------------------------------
 *  01.12.03 | dpo    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
*/
/****************************************************************************/

#include "adl_global.h"

#define DBG_TRACE_      TRACE
#define GET_MEM_(_X_)   adl_memGet (_X_)

#define ADDCARRY(x) { if((x) > 65535UL) (x) -= 65535UL; }
#define REDUCE { \
    l_util.l = sum; \
    sum = (u32) l_util.s[0] + l_util.s[1]; \
    ADDCARRY( sum); \
}

#define HTONS(a)            ( ((((u16) (a)) << 8)&0xff00) | \
                              ((((u16) (a)) >> 8)&0x00ff) )
#define HTONL(a)            ( ((((u32) (a)) << 24)&0xff000000U) | \
                              ((((u32) (a)) <<  8)&0x00ff0000U) | \
                              ((((u32) (a)) >>  8)&0x0000ff00U) | \
                              ((((u32) (a)) >> 24)&0x000000ffU) )


typedef u8 NGubyte;
typedef u16 NGushort;
typedef u32 NGuint;
typedef struct {
    NGubyte     ip_hl:4,    /* header lenght */
                ip_v:4;     /* version */
#define IPVERSION    4              /* current IP version */
    NGubyte     ip_tos;     /* type of service */
#define IPTOS_LOWDELAY      0x10    /* minimize delay */
#define IPTOS_THROUGHPUT    0x08    /* maximize throughput */
#define IPTOS_RELIABILITY   0x04    /* maximize reliability */
    NGushort    ip_len;     /* total length */
    NGushort    ip_id;      /* identification */
    NGushort    ip_off;     /* fragment offset field */
#define IPOFF_DF            0x4000  /* dont fragment flag */
#define IPOFF_MF            0x2000  /* more fragment flag */
#define IPOFF_MASK          0x1fff  /* mask for fragmenting bits */
    NGubyte     ip_ttl;     /* time to live */
    NGubyte     ip_p;       /* protocol */
    NGushort    ip_sum;     /* checksum */
    NGuint      ip_src;     /* source address */
    NGuint      ip_dst;     /* destination address */
} NGiphdr;


typedef struct {
	u8 type;
	u8 code;
	u16 cksum;
	u16 id;
	u16 seq;
} icmp_echo_hdr_t;

/***************************************************************************/
/*  Locale variables                                                       */
/*-------------------------------------------------------------------------*/
static u16 ip_id;
static u16 icmp_id;
static u16 icmp_seq;

/***************************************************************************/
/*  Locale functions                                                       */
/*-------------------------------------------------------------------------*/
static u16 ip_cksum( void *sptr, int len);



/***************************************************************************/
/*  Function   : ip_ping_init	                                           */
/*-------------------------------------------------------------------------*/
/*  Objet      : 				                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*                    |   |   |   |  				           */
/***************************************************************************/
void ip_ping_init(){
	ip_id=0xdead;
	icmp_id=0xA5A5;
	icmp_seq=0x1234;
}



/***************************************************************************/
/*  Function   : Checksum function	                                   */
/*-------------------------------------------------------------------------*/
/*  Objet      : 				                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*                    |   |   |   |  				           */
/***************************************************************************/
u16 ip_cksum( void *sptr, int len)
{
	u16 *w;
	u32 sum;
	int mlen;
	int bswapped;
	union {
		u8  c[2];
		u16 s;
	} s_util;
	union {
		u16 s[2];
		u32   l;
	} l_util;
	w = sptr;
	mlen = len;
	sum = 0;
	bswapped = 0;
	/* force to even boundary */
	if( (((int) w) & 1) && (mlen > 0)) {
		REDUCE;
		sum <<= 8;
		s_util.c[0] = *((u8 *) w);
		w = (u16 *) ((u8 *) w + 1);
		mlen--;
		bswapped = 1;
	}
	
	/* compute checksum */
	while( mlen >= 32) {
		sum += *w++; sum += *w++; sum += *w++; sum += *w++;
		sum += *w++; sum += *w++; sum += *w++; sum += *w++;
		sum += *w++; sum += *w++; sum += *w++; sum += *w++;
		sum += *w++; sum += *w++; sum += *w++; sum += *w++;
		mlen -= 32;
	}
	while( mlen >= 8) {
		sum += *w++; sum += *w++; sum += *w++; sum += *w++;
		mlen -= 8;
	}
	if( mlen || bswapped) {
		REDUCE;
		while( (mlen -= 2) >= 0) {
			sum += *w++;
		}
		if( bswapped) {
			REDUCE;
			sum <<= 8;
			bswapped = 0;
			if( mlen == -1) {
				s_util.c[1] = *((u8 *) w);
				sum += s_util.s;
				mlen = 0;
			}
			else mlen = -1;  /* 05/11/99 */
		}
		else if( mlen == -1) {
			s_util.c[0] = *((u8 *) w);
		}
	}
	if( mlen == -1) {
		/* one byte left */
		s_util.c[1] = 0;
		sum += s_util.s;
	}
	REDUCE;
	return( ~sum);
}


/***************************************************************************/
/*  Function   : ip_get_header_size                                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Objet      : Returns the PING packet header size                       */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
u8 ip_get_header_size ( void )
{
    return sizeof(NGiphdr)+sizeof(icmp_echo_hdr_t);
}


/***************************************************************************/
/*  Function   : Prepare ICMP Echo Reply Packet 			   */
/*									   */
/*-------------------------------------------------------------------------*/
/*  Objet      : 				                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  LocalAddr         |   |   |   |  	ME's Local IP Address		   */
/***************************************************************************/
u8* ip_send_ping ( u32 host, u32 LocalAddr, u16* size, u16 PayloadLength )
{
	NGiphdr *ip_h;
	icmp_echo_hdr_t *icmp;
	u8 *offset;
	u16 i;
	u16 checksum;
	u8* packet;

	*size = sizeof(NGiphdr)+sizeof(icmp_echo_hdr_t)+ PayloadLength ;
	packet=(u8 *)GET_MEM_(*size);

	wm_memset(packet,0,*size);
	/* 1) fill IP Header */
	ip_h=(NGiphdr*)packet;
	/* no fill IP Header */
	ip_h->ip_hl  = sizeof(NGiphdr)>>2;
	ip_h->ip_v   = 4; /* IP Version */
	ip_h->ip_tos = 0;
	ip_h->ip_len = HTONS(sizeof(NGiphdr) + sizeof(icmp_echo_hdr_t)+ PayloadLength );
	ip_h->ip_id  = HTONS(ip_id);
	ip_h->ip_off = 0;
#define IP_TTL 64
	ip_h->ip_ttl = IP_TTL;
	ip_h->ip_p   = 1; /* ICMP proto */
	ip_h->ip_src = HTONL(LocalAddr);
	ip_h->ip_dst = HTONL(host) ;
	ip_h->ip_sum = 0;
	ip_h->ip_sum = ip_cksum(ip_h, sizeof(NGiphdr));
	/* dump IP Header */
	/* fill ICMP header */
	icmp=(icmp_echo_hdr_t *)(&ip_h[1]);
	icmp->type=8; /* ICMP ECHO REQUEST */
	icmp->code=0;
	icmp->id=HTONS(icmp_id);
	icmp->seq=HTONS(icmp_seq);
	/* ICMP Payload */
	offset=(u8*)(&icmp[1]); /* store data here */
	/* fill data buffer */
	for (i=0x20; i<0x20+ PayloadLength; i++)
	{
		*offset++=i;
	}
	/* Calculate ICMP checksum */
	checksum=ip_cksum(icmp,sizeof(icmp_echo_hdr_t)+ PayloadLength );
	icmp->cksum=checksum; /* FGA: removed HTONS */

	DBG_TRACE_((9,"Dump du packet"));
	for(i=0;i<28;i++)
	{
	DBG_TRACE_((9,"%x",packet[i]));
	}
	/* send packet */
	return packet;
	
	//wm_osReleaseHeapMemory(packet);

}



/***************************************************************************/
/*  Function   : ip_check_ping	                                   	   */
/*-------------------------------------------------------------------------*/
/*  Objet      : 				                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*                    |   |   |   |  				           */
/***************************************************************************/
u8 ip_check_ping(u8 *packet){
	NGiphdr *ip;
	icmp_echo_hdr_t *icmp;
	u16 hlen;
	u8 i;
	ip=(NGiphdr *)packet;
	
	    for(i=0;i<28;i++)
	    {
		DBG_TRACE_((9,"%x",ip[i]));
	    }	

	if (ip->ip_v != 4){
		/* error : wrong version */
		DBG_TRACE_((8,"packet has wrong version "));
		return 1;
	}
	hlen=ip->ip_hl<<2;
	if (hlen<sizeof(NGiphdr)){
		/* error : wrong hlen */
		DBG_TRACE_((8,"bad header length"));
		return 2;
	}

	if ( ip_cksum(packet,hlen)!=0){
		/* error : wrong cksum */
		DBG_TRACE_((9,"bad cksum"));
		return 3;
	}
	DBG_TRACE_((9,"Ip_len"));
	DBG_TRACE_((9,"%x",ip->ip_len));


	if (HTONS(ip->ip_len)<hlen){
		/* error : wrong len */
		DBG_TRACE_((9,"bad packet length"));
		return 4;
	}
	if (HTONS(ip->ip_off) & (0x3FFF)){
		/* error : Fragment received */
		DBG_TRACE_((9,"Packet is a fragment"));
		return 5;
	}
	/* check proto */
	if (ip->ip_p != 1){
		/* error : Bad protocol */
		DBG_TRACE_((9,"Protocol is not ICMP "));
		return 6;
	}
			
	icmp=(icmp_echo_hdr_t *)(&ip[1]);
	if (icmp->type!=0){
		/* error : Bad icmp type */
		DBG_TRACE_((9,"Type is not Echo Reply "));
		return 7;
	}

	if (HTONS(icmp->id) != icmp_id && HTONS(icmp->seq) !=icmp_seq){
		/* error : Bad id/seq type */
		DBG_TRACE_((9,"Bad Id/Seq "));
		return 7;
	}
	/* check checksum ? */
	if (ip_cksum(icmp,HTONS(ip->ip_len)-hlen) != 0)
	{
		/* error : Bad id/seq type */
		DBG_TRACE_((9,"Bad cksum "));
		return 8;
	}
	/* packet is correct ! */
	DBG_TRACE_((9,"Receveived Ping Echo Reply."));
	/* */
	icmp_id++;
	icmp_seq++;
	return 0; /* OK */
}

/*****************************************************************************/

/* END OF FILE ip_task.c */
