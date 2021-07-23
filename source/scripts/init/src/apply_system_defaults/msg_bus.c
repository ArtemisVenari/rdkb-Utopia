 /*
===================================================================
   This programs handles dbus initialization , PSM set record API &
   SetDataModelParam

Date : 02-07-2021

===================================================================
*/

#include "ccsp_custom.h"
#include "ccsp_psm_helper.h"
#include <ccsp_base_api.h>
#include "ccsp_memory.h"
#include "utils.h"

#define CCSP_SUBSYS "eRT."


char* const g_cComponent_id  = "ccsp.apply";
void *g_vBus_handle = NULL;

/*
 * Procedure     : dbusInit
 * Purpose       : Dbus connection initialization
 * Parameters    :
 * Return Value  :
 */

int
dbusInit (void)
{
  int ret = -1;
  char *pCfg = CCSP_MSG_BUS_CFG;
  if (g_vBus_handle == NULL)
    {
// Dbus connection init
#ifdef DBUS_INIT_SYNC_MODE
      ret = CCSP_Message_Bus_Init_Synced (g_cComponent_id,
                                          pCfg,
                                          &g_vBus_handle,
                                          Ansc_AllocateMemory_Callback,
                                          Ansc_FreeMemory_Callback);
#else
      ret = CCSP_Message_Bus_Init (g_cComponent_id,
                                   pCfg,
                                   &g_vBus_handle,
                                   Ansc_AllocateMemory_Callback,
                                   Ansc_FreeMemory_Callback);
#endif /* DBUS_INIT_SYNC_MODE */
    }


  return ret;
}

/*
 * Procedure     : set_psm_record
 * Purpose       : PSM SET API to set psm entries
 * Parameters    :
 *    name       : name of the record
 *    str        : value of the record
 * Return Value  :
 */

int set_psm_record(char *name,char *str)
{
    int ret = 0;
    int retry = 10;

    /* Since we are setting PSM value on early stage of bootup sometime we found error CCSP_ERR_NOT_EXIST
       because PSM is not initialized. So we added the retry mechanism to recover.
    */
    do {
        ret = PSM_Set_Record_Value2(g_vBus_handle, CCSP_SUBSYS, name, ccsp_string, str);
        if(ret != CCSP_SUCCESS)
            sleep(1);
    } while ( (ret != CCSP_SUCCESS) && (retry--) );

}

/*
 * Procedure     : SetDataModelParam
 * Purpose       : To set datamodel with a value
 * Parameters    :
 *    pComponent : name of the component
 *    pBus       : Dbus path
 *    pParamName : Datamodel name
 *    pParamVal  : Datamodel value
 *    dataType_e : Datamodel datatype
 * Return Value  :
 */

int SetDataModelParam(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag)
{
    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)g_vBus_handle;
    parameterValStruct_t param_val[1] = {0};
    char *faultParam = NULL;
    int ret = 0;

    param_val[0].parameterName = pParamName;
    param_val[0].parameterValue = pParamVal;
    param_val[0].type = type;

    ret = CcspBaseIf_setParameterValues(
        g_vBus_handle,
        pComponent,
        pBus,
        0,
        0,
        param_val,
        1,
        TRUE,
        &faultParam);

    if ((ret != CCSP_SUCCESS) && (faultParam != NULL)){
        printf("%s-%d Failed to set %s\n", __FUNCTION__, __LINE__, pParamName);
        bus_info->freefunc(faultParam);
        return -1;
    }

    return 0;
}
