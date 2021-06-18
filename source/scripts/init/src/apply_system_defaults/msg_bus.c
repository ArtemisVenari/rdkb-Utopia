 /*
===================================================================
   This programs handles dbus initialization & PSM set record API


===================================================================
*/

#include "ccsp_custom.h"
#include "ccsp_psm_helper.h"
#include <ccsp_base_api.h>
#include "ccsp_memory.h"

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

