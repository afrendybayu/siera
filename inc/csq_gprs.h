/*
 * csq_gprs.h
 *
 *  Created on: Jul 5, 2013
 *      Author: Afrendy Bayu
 */

#ifndef CSQ_GPRS_H_
#define CSQ_GPRS_H_

#define PG_GPRS_RSP_BUFF_SIZE       200
#define PG_GPRS_PARAM_SIZE           20

#define PG_DEFAULT_CTX_ID                  1
#define PG_DEFAULT_PING_INTERVAL          10
#define PG_DEFAULT_PINGPACKET_SIZE       100
#define PG_DEFAULT_DATA_SIZE             400

#define PG_IP_ADD_SIZE              	  33

#define CONTEXT_NB          4



void InitWdataParams ( void );
s8 HandlePingCmd ( adl_atCmdPreParser_t *paras );
#endif /* CSQ_GPRS_H_ */
