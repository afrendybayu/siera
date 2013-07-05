/**********************************************************************************************************************/
/*  IP_PING.H   -  Copyright Sierra Wireless Inc. (c) 2010															  */
/*                                                                                                                    */
/*                                                                                                                    */
/* LICENSE                                                                                                            */
/* =======                                                                                                            */
/* If no specific agreement has been signed, this Source Code is subject to the following license terms:              */
/* http://www.sierrawireless.com/productsandservices/AirPrime/Sierra_Wireless_Software_Suite/License/Userlicense.aspx */
/* If you modify this file, you are subject to the contribution rules described into this license                     */
/**********************************************************************************************************************/


/***************************************************************************/
/* File    :   IP_PING.H                                                   */
/*                                                                         */
/* Scope   :   Open AT Test Example.                                       */
/*             Send a ping (ICMP echo request) to a IP host                */
/*             Analyse echo reply packet.                                  */
/*                                                                         */
/* Content :                                                               */
/*-------------------------------------------------------------------------*/
/* Date     | Author |        Modification                                 */
/*----------+--------+-----------------------------------------------------*/
/* 03/10/00 |  FCA   | Creation                                            */
/*----------+--------+-----------------------------------------------------*/
/* $LogWavecom: U:\projet\mmi\pvcsarch\archives\open-at\SAMPLES\Open-AT\Common\itf\ip_ping.h-arc $
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  08.06.05 | DPO    | 1.1            | *   Resolution for 28933: The Open-
 *           |        |                | AT Ping sample has to be more confi
 *           |        |                | gurable                            
 * ----------+--------+----------------+-------------------------------------
 *  01.12.03 | dpo    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  07.05.03 | dpo    | 1.1            | * Add new line at end of file      
 * ----------+--------+----------------+-------------------------------------
 *  02.10.02 | dpo    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
 */
/***************************************************************************/

#ifndef __IP_PING_H__
#define __IP_PING_H__

void ip_ping_init(void);

u8* ip_send_ping ( u32 host, u32 LocalAddr, u16 * size, u16 PayloadLength );

u8 ip_check_ping(u8 *packet);

u8 ip_get_header_size ( void );

#endif /*__IP_PING_H__*/
