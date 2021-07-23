/* Fetch PartnerID using getFactoryPartnerId() & json_file_parse() */

#include "utils.h"

// getFactoryPartnerId from HAL:
int getFactoryPartnerId
        (
                char*                       pValue
        )
{
    FILE *fp = NULL;

    if(pValue == NULL)
        return -1;

    fp = fopen("/nvram/PartnerID","r");
    if(fp != NULL){
        fscanf(fp, "%s", pValue);
        fclose(fp);
        return 0;
    }
    else
    {
#if defined (_XB6_PRODUCT_REQ_) || defined(_HUB4_PRODUCT_REQ_) || defined(_DT_WAN_Manager_Enable_)
        if(0 == platform_hal_getFactoryPartnerId(pValue))
        {
                APPLY_PRINT("%s - %s\n",__FUNCTION__,pValue);
                return 0;
        }
        else
        {
                int count = 0 ;
                while ( count < 3 )
                {
                        APPLY_PRINT(" Retrying for getting partnerID from HAL, Retry Count:%d\n", count + 1);
                        if(0 == platform_hal_getFactoryPartnerId(pValue))
                        {
                                APPLY_PRINT("%s - %s\n",__FUNCTION__,pValue);
                                return 0;
                        }
                        sleep(3);
                        count++;
                }
                //TCCBR-4426 getFactoryPartnerId is implemented for XB6/HUB4 Products as of now
                APPLY_PRINT("%s - Failed Get factoryPartnerId \n", __FUNCTION__);
        }
#endif
    }

    if(!strcmp(pValue,""))
        strncpy(pValue,"telekom-dev",20);

    return 0;
}



/*jSON file parser function */
char * json_file_parse( char *path )
{
        FILE            *fileRead       = NULL;
        char            *data           = NULL;
        int              len            = 0;

        //File read
        fileRead = fopen( path, "r" );

        //Null Check
        if( fileRead == NULL )
        {
                APPLY_PRINT( "%s-%d : Error in opening JSON file\n" , __FUNCTION__, __LINE__ );
                return NULL;
        }

        //Calculate length for memory allocation
        fseek( fileRead, 0, SEEK_END );
        len = ftell( fileRead );
        fseek( fileRead, 0, SEEK_SET );

        APPLY_PRINT("%s-%d : Total File Length :%d \n", __FUNCTION__, __LINE__, len );

        if( len > 0 )
        {
                 data = ( char* )malloc( sizeof(char) * (len + 1) );
                 //Check memory availability
                 if ( data != NULL )
                 {
                        memset( data, 0, ( sizeof(char) * (len + 1) ));
                        fread( data, 1, len, fileRead );

                 }
                 else
                 {
                         APPLY_PRINT("%s-%d : Memory allocation failed Length :%d\n", __FUNCTION__, __LINE__, len );
                 }
        }

        if( fileRead )
        fclose( fileRead );

        return data;
}

