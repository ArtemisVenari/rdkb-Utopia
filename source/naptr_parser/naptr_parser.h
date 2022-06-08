#ifndef __NAPTR_PARSER_H__
#define __NAPTR_PARSER_H__

#define TRUE            1
#define FALSE           0
#define SYSEVENT_IP     "127.0.0.1"
#define MAX_DOMAIN_LEN  255
#define COND_CHECK(arg,str) ((arg != NULL) && (!strncmp (arg , str,strlen(str)))) ? TRUE : FALSE
#define NAPTR           "NAPTR"
#define SRV             "SRV"
#define A               "A"
#define AAAA            "AAAA"
#define SUPPORT_SIP     "SIP"
#define SYSEVENT_SIZE   512
#define LOG_FILE 	"/rdklogs/logs/ArmConsolelog.txt.0"
#define ARRAY_SIZE      16

#define APPLY_PRINT(fmt ...)   {\
   FILE *logfp = fopen ( LOG_FILE , "a+");\
   if (logfp)\
   {\
        fprintf(logfp,fmt);\
        fclose(logfp);\
   }\
}\

struct ares_naptr_reply *gpNaptrRecord = NULL; /* global naptr list  */
struct ares_srv_reply *gpSrvRecord = NULL; /* global srv list */
struct hostent *host = NULL;
ares_channel channel; /* ares channel */
char sysevent_str[SYSEVENT_SIZE]; /* sysevent str */

/******************************************************************
** Function	: sendNaptrQuery
** Description	: Function sends NAPTR query

** @param[in]	: domainName
** @param[out]  : None (invokes dnsCallback and parse reply)
** @retval	: void
*******************************************************************/

void sendNaptrQuery(char *pdomainName);

/******************************************************************
** Function     : sendSrvQuery
** Description  : Function sends SRV query

** @param[in]   : None
** @param[out]  : None (invokes dnsCallback and parse reply)
** @retval      : void
*******************************************************************/

void sendSrvQuery();

/******************************************************************
** Function     : sendA_AAAAQuery
** Description  : Function sends A, AAAA query

** @param[in]   : None
** @param[out]  : None (invokes dnsCallback and parse reply)
** @retval      : void
*******************************************************************/

void sendA_AAAAQuery();

#endif
