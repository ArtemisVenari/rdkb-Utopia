#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ares.h"
#include "syscfg/syscfg.h"
#include "sysevent/sysevent.h"
#include "naptr_parser.h"

/******************************************************************
** Function     : mainLoop
** Description  : Function monitors for multiple fds become "ready"
                  for some class of I/O operation and process query

** @param[in]   : Name service channel
** @param[out]  : None
** @retval      : void
*******************************************************************/

static void mainLoop(ares_channel *channel)
{
     int nfds, count;
     fd_set readers, writers;
     struct timeval tv, *tvp;
     while (1)
     {
          FD_ZERO (&readers);
          FD_ZERO (&writers);
          nfds = ares_fds (*channel, &readers, &writers);
          if (nfds == 0)
               break;
          tvp = ares_timeout (*channel, NULL, &tv);
          count = select (nfds, &readers, &writers, NULL, tvp);
          ares_process (*channel, &readers, &writers);
     }
}

/******************************************************************
** Function     : dnsCallback
** Description  : Callback parses replies using below ares APIs
                  NAPTR - ares_parse_naptr_reply
                  SRV   - ares_parse_srv_reply
                  A     - ares_parse_a_reply
                  AAAA  - ares_parse_aaaa_reply

** @param[in]   : *arg (Type of record)
                  status - ares_query status
** @param[out]  : List of records
** @retval      : void
*******************************************************************/

static void dnsCallback(void *arg,
                         int status,
                         int timeouts,
                         unsigned char *abuf,
                         int alen)
{
     int err;
     APPLY_PRINT(" %s Status = %d \n",__func__,status);
     struct ares_srv_reply *psrvLocal, *ptempSrv;

     if(status == ARES_SUCCESS)
     {
          if(COND_CHECK(arg,NAPTR))
          {
               err = ares_parse_naptr_reply (abuf, alen, &gpNaptrRecord);
               if (err == ARES_SUCCESS)
               {
                    APPLY_PRINT("NAPTR records found\n");
               }
               else
               {
                    APPLY_PRINT("err=%i\n", err);
               }
               return;
          }

          if(COND_CHECK(arg,SRV))
          {
                err = ares_parse_srv_reply(abuf, alen, &psrvLocal);
                if (err == ARES_SUCCESS)
                {
                     APPLY_PRINT("SRV records found\n");
                     if(psrvLocal != NULL)
                     {
                          if(gpSrvRecord == NULL)
                          {
                               gpSrvRecord = psrvLocal;
                          }
                          else
                          {
                               ptempSrv = gpSrvRecord;
                               while( ptempSrv->next != NULL)
                               {
                                    ptempSrv = ptempSrv->next;
                               }
                               ptempSrv->next = psrvLocal;
                          }
                     }
                }
                else
                {
                     APPLY_PRINT("err=%i\n", err);
                }
          }

          if (COND_CHECK(arg,A))
          {
               err = ares_parse_a_reply(abuf, alen, &host, NULL, NULL);
               char ip[INET6_ADDRSTRLEN];
               int j = 0;

               if (err == ARES_SUCCESS)
               {
                    APPLY_PRINT(" IP address type is %d \n", host->h_addrtype);
                    for (j = 0; host->h_addr_list[j]; ++j)
                    {
                         inet_ntop(host->h_addrtype, host->h_addr_list[j], ip, sizeof(ip));
                         strncat(sysevent_str, ip, sizeof(ip));
                         APPLY_PRINT("IP adress is %s\n", ip);
                    }
                    strncat(sysevent_str, ",", 1);
                    ares_free_hostent(host);
               }
               else
               {
                    APPLY_PRINT("err=%i\n", err);
                    return;
               }
          }

          if (COND_CHECK(arg,AAAA))
          {
               err = ares_parse_aaaa_reply(abuf, alen, &host, NULL, NULL);
               char ip[INET6_ADDRSTRLEN];
               int j = 0;

               if (err == ARES_SUCCESS)
               {
                    APPLY_PRINT("IP address type is %d \n", host->h_addrtype);
                    for (j = 0; host->h_addr_list[j]; ++j)
                    {
                         inet_ntop(host->h_addrtype, host->h_addr_list[j], ip, sizeof(ip));
                         strncat(sysevent_str, ip, sizeof(ip));
                         APPLY_PRINT("IPv6 Address is %s\n", ip);
                    }
                    strncat(sysevent_str, ",", 1);
                    ares_free_hostent(host);
               }
               else
               {
                    APPLY_PRINT("err=%i\n", err);
                    return;
               }
          }
     }
     else if( (status == ARES_ENODATA) || (status == ARES_EFORMERR) || (status == ARES_EBADRESP))
     {
          APPLY_PRINT(" No %s records found \n",arg);
     }
     else
     {
          APPLY_PRINT("Error unknown\n");
     }
}

/******************************************************************
** Function     : sendNaptrQuery
** Description  : Function sends NAPTR query

** @param[in]   : domainName
** @param[out]  : None (invokes dnsCallback and parse reply)
** @retval      : void
*******************************************************************/

void sendNaptrQuery(char *pdomainName)
{
     ares_query (channel, pdomainName,
                          ns_c_in,   /* ns_c_in */
                          ns_t_naptr, /* NAPTR */
                          dnsCallback, NAPTR);

     mainLoop (&channel);
}

/******************************************************************
** Function     : sendSrvQuery
** Description  : Function sends SRV query

** @param[in]   : None
** @param[out]  : None (invokes dnsCallback and parse reply)
** @retval      : void
*******************************************************************/

void sendSrvQuery()
{
     struct ares_naptr_reply *pnaptrCurr = NULL, *temp = NULL;
     if(gpNaptrRecord != NULL)
     {
         pnaptrCurr = (gpNaptrRecord !=NULL)  ? gpNaptrRecord : NULL;

         while (pnaptrCurr != NULL)
         {
              if(pnaptrCurr->replacement !=  NULL)
              {
                  if ( strstr(pnaptrCurr->service, SUPPORT_SIP))
                  {
                        ares_query (channel, pnaptrCurr->replacement,
                                             ns_c_in,   /* ns_c_in */
                                             ns_t_srv, /* SRV */
                                             dnsCallback, SRV);
                        mainLoop (&channel);
                  }
              }
              temp = pnaptrCurr;
              pnaptrCurr = pnaptrCurr->next;
         }
     }

}

/******************************************************************
** Function     : sendA_AAAAQuery
** Description  : Function sends A, AAAA query

** @param[in]   : None
** @param[out]  : None (invokes dnsCallback and parse reply)
** @retval      : void
*******************************************************************/

void sendA_AAAAQuery()
{
     struct ares_srv_reply *ptempSrv = NULL;
     ptempSrv = gpSrvRecord;
     while(ptempSrv != NULL)
     {
          if(ptempSrv->host !=  NULL)
          {
               ares_query (channel, ptempSrv->host,
                                    ns_c_in,   /* ns_c_in */
                                    ns_t_a, /* a-record */
                                    dnsCallback, A);
               mainLoop (&channel);

               ares_query (channel, ptempSrv->host,
                                    ns_c_in,   /* ns_c_in */
                                    ns_t_aaaa, /* aaaa-record */
                                    dnsCallback, AAAA);
               mainLoop (&channel);
          }
          ptempSrv = ptempSrv->next;
    }
}

/******************************************************************
** Funtion	: main
*******************************************************************/
int main(int argc, char **argv)
{
    int            sysevent_fd = -1;
    token_t        sysevent_token;
    char           sysevent_ip[ARRAY_SIZE];
    char          *sysevent_name = "qos_ip_address";
    unsigned short sysevent_port;
    char domainName[MAX_DOMAIN_LEN];
    char status[16];
    int err;

    /* Initialize syscfg db */
    if (0 != syscfg_init())
    {
         APPLY_PRINT("Failed to initialize syscfg\n");
         exit (EXIT_FAILURE);
    }

    snprintf(sysevent_ip, sizeof(sysevent_ip), SYSEVENT_IP);
    sysevent_port = SE_SERVER_WELL_KNOWN_PORT;

    /* Register for sysevent */
    sysevent_fd =  sysevent_open(sysevent_ip, sysevent_port, SE_VERSION, sysevent_name, &sysevent_token);
    if (0 > sysevent_fd)
    {
         APPLY_PRINT("Unable to register with sysevent daemon at %s %u.\n", sysevent_ip, sysevent_port);
         exit (EXIT_FAILURE);
    }

    /* Fetch wan-status */
    sysevent_get(sysevent_fd, sysevent_token, "wan-status", status, sizeof(status));

    if(!(strncmp(status,"started", sizeof(status))))
    {
         syscfg_get(NULL, "naptr_domain_name", domainName, sizeof(domainName));
         if(domainName[0] == '\0')
         {
              APPLY_PRINT("\n Unable to fetch domain name \n");
              exit (EXIT_FAILURE);
         }

         if((err = ares_init (&channel)) != ARES_SUCCESS)
         {
              APPLY_PRINT("ares_init() failed (%i)\n", err);
              exit (EXIT_FAILURE);
         }

         /* send query to fetch naptr record */
         sendNaptrQuery(domainName);

         /*send query to fetch srv record */
         sendSrvQuery();

         /*send query to fetch a and aaaa record */
         sendA_AAAAQuery();

         if ( 0 != (sysevent_set(sysevent_fd, sysevent_token, "qos_ip_address", sysevent_str, 0)))
         {
               APPLY_PRINT("Unable to set sysevent for qos_ip_address \n");
               exit (EXIT_FAILURE);
         }
    }
    else if (!(strncmp(status,"stopped", sizeof(status))))
    {
         if ( 0 != (sysevent_set(sysevent_fd, sysevent_token, "qos_ip_address", NULL, 0)))
         {
               APPLY_PRINT("Unable to set sysevent NULL for qos_ip_address \n");
               exit (EXIT_FAILURE);
         }
    }
    /* Restart firewall */
    sysevent_set(sysevent_fd, sysevent_token, "firewall-restart", NULL, 0);

    if(gpNaptrRecord != NULL)
         ares_free_data (gpNaptrRecord);
    if(gpSrvRecord != NULL)
         ares_free_data (gpSrvRecord);

    ares_destroy (channel);
    exit (EXIT_SUCCESS);
}
