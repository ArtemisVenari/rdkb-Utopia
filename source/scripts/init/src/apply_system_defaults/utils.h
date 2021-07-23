/*  */
#ifndef  _UTILS_H
#define  _UTILS_H

#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "wifi_hal.h"
#include <unistd.h>
#include "ccsp_base_api.h"

#define PARTNER_ID_LEN 64
#define PARTNERS_INFO_FILE_ETC  "/etc/partners_defaults.json"

#define WIFI_DBUS_PATH                    "/com/cisco/spvtg/ccsp/wifi"
#define WIFI_COMPONENT_NAME               "eRT.com.cisco.spvtg.ccsp.wifi"

#define DOMAIN_NAME "Device.WiFi.Radio.%d.RegulatoryDomain"
#define APPLY_SETTING_SSID "Device.WiFi.Radio.%d.X_CISCO_COM_ApplySettingSSID"
#define APPLY_SETTING "Device.WiFi.Radio.%d.X_CISCO_COM_ApplySetting"

#define DATAMODEL_PARAM_LENGTH 256
#define NO_RADIO 2

#if defined (_CBR_PRODUCT_REQ_) || defined (_XB6_PRODUCT_REQ_)
        #define LOG_FILE "/rdklogs/logs/Consolelog.txt.0"
#else
        #define LOG_FILE "/rdklogs/logs/ArmConsolelog.txt.0"
#endif

#define RETRY_COUNT 60
#define APPLY_PRINT(fmt ...)   {\
   FILE *logfp = fopen ( LOG_FILE , "a+");\
   if (logfp)\
   {\
        fprintf(logfp,fmt);\
        fclose(logfp);\
   }\
}\


/*Function prototype */

int getFactoryPartnerId(char* pValue);
char * json_file_parse( char *path );

#endif

