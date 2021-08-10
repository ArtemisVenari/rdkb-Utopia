/* WiFi Country code setting based on PartnerID */

#include "utils.h"

/* Function prototypes */
extern int dbusInit (void);
extern void *g_vBus_handle ;
extern int SetDataModelParam(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);

int main()
{
    char  PartnerID[ PARTNER_ID_LEN ]  = { 0 };
    char country_code_ret[20];
    cJSON *json  = NULL;
    char* data = NULL;
    char acSetParamName[256];

    /* dbus initialization */
    dbusInit();

    /* Fetching PartnerID from HAL */
    getFactoryPartnerId(PartnerID);
    APPLY_PRINT("%s - PartnerID is %s\n",__FUNCTION__,PartnerID);

    if( !PartnerID || PartnerID[0] == '\0' ){
           APPLY_PRINT("%s - PartnerID not found\n",__FUNCTION__);
           return -1;
        }

    /* Parsing JSON data */
    data = json_file_parse(PARTNERS_INFO_FILE_ETC);

    if (strlen(data) == 0) {
        APPLY_PRINT("%s - data is null\n",__FUNCTION__);
        return -1;
    }

    json = cJSON_Parse(data);
    if(!json){
        APPLY_PRINT("%s - json parse data is null\n",__FUNCTION__);
        return -1;
    }

    cJSON *partnerObj = cJSON_GetObjectItem( json, PartnerID);
    if ( NULL == partnerObj){
         APPLY_PRINT("%s - partnerObj is null\n",__FUNCTION__);
         return -1;
    }
    cJSON *CountrycodeObj = cJSON_GetObjectItem(partnerObj, "country_code");
    if ( NULL == CountrycodeObj){
          APPLY_PRINT("%s - CountrycodeObj is null\n",__FUNCTION__);
          return -1;
    }
    strcpy(country_code_ret, CountrycodeObj->valuestring);

    /* Setting RegulatoryDomain,ApplySettingSSID & ApplySetting DM */
    for(int index=1; index<=NO_RADIO; index++){
        memset(acSetParamName,0,DATAMODEL_PARAM_LENGTH);
        snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, DOMAIN_NAME, index);
        if(SetDataModelParam(WIFI_COMPONENT_NAME, WIFI_DBUS_PATH, acSetParamName, country_code_ret, ccsp_string,FALSE) !=0){
            APPLY_PRINT("%s - Regulatory Domain SET operation failed\n",__FUNCTION__);
            return -1;
        }

        memset(acSetParamName,0,DATAMODEL_PARAM_LENGTH);
        snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, APPLY_SETTING_SSID, index);
        if(SetDataModelParam(WIFI_COMPONENT_NAME, WIFI_DBUS_PATH, acSetParamName ,"0", ccsp_int,FALSE) !=0){
            APPLY_PRINT("%s - ApplySettingSSID SET operation failed\n",__FUNCTION__);
            return -1;
        }

        memset(acSetParamName,0,DATAMODEL_PARAM_LENGTH);
        snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, APPLY_SETTING, index);
        if(SetDataModelParam(WIFI_COMPONENT_NAME, WIFI_DBUS_PATH, acSetParamName ,"true", ccsp_boolean,FALSE) !=0){
            APPLY_PRINT("%s - ApplySetting SET operation failed\n",__FUNCTION__);
            return -1;
        }
   }
    return 0;
}
