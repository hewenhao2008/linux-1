#ifndef __QL_NW_H__
#define __QL_NW_H__


//#include "ql_type.h"
//#include "inc/dsi_netctrl.h"

typedef void (* nw_init_cb_func)(void* cb_data);



#define QL_NW_APN_NAME_MAX_LEN 100    
     
 
#define QL_NW_USER_NAME_MAX_LEN 64    
     
 
#define QL_NW_PWD_NAME_MAX_LEN 100    
    
 


//--------------------------------------------------------------------------------------------------  
/**   
 * Network Registration states.    
 */   
//--------------------------------------------------------------------------------------------------  
typedef enum    
{     
    E_QL_WWAN_REG_NOT_REGISTERED                 = 0,    ///< Not registered and not currently searching for new operator.     
 
    E_QL_WWAN_REG_REGISTERED_HOME_NETWORK        = 1,    ///< Registered, home network.      
 
    E_QL_WWAN_REG_NOT_REGISTERED_SEARCHING_NOW   = 2,    ///< Not registered but currently searching for a new operator.  
 
    E_QL_WWAN_REG_DENIED                         = 3,    ///< Registration was denied, usually because of invalid access credentials.   
 
    E_QL_WWAN_REG_UNKNOWN                        = 4,    ///< Unknown state.     

    E_QL_WWAN_REG_ROAMING                        = 5     ///< Registered to a roaming network.    
}E_QL_WWAN_NET_REG_STATE_T;    


//--------------------------------------------------------------------------------------------------  
/**   
 * Radio Access Technology enum    
 */   
//--------------------------------------------------------------------------------------------------  
typedef enum    
{     
    E_QL_NW_RAT_GSM                     = 0,
    E_QL_NW_RAT_UTRAN                   = 2,
    E_QL_NW_RAT_GSMW_EGPRS              = 3,
    E_QL_NW_RAT_UTRANW_HSDPA            = 4,
    E_QL_NW_RAT_UTRANW_HSUPA            = 5,
    E_QL_NW_RAT_UTRANW_HSDPA_AND_HSUPA  = 6,
    E_QL_NW_RAT_E_UTRAN                 = 7 
}E_QL_NW_RADIO_ACCESS_TYPE_T;   


#define MAX_NUM_NEIGHBOR_CELLS 10
typedef struct
{
    char    rat[16];
    int     mcc;
    int     mnc;
    int     lac;
    int     cid;
}ST_SingleCellInfo;  

typedef struct
{
    int                 validCnt;
    ST_SingleCellInfo   cellInfo[MAX_NUM_NEIGHBOR_CELLS];
}ST_CellInfo;  

//--------------------------------------------------------------------------------------------------  
/**                                                                                                   
 * Handler of network registration state changes.                                                    
 *                                                                                                    
 *                                                                                                    
 * @param state                                                                                       
 *        Parameter ready to receive the Network Registration state.                                  
 * @param contextPtr                                                                                  
 */                                                                                                   
//--------------------------------------------------------------------------------------------------  
typedef void (*QL_NW_RegStateHandlerFunc_t)                                                       
(
    E_QL_WWAN_NET_REG_STATE_T   state,                                                                       
    void                        *contextPtr                                                                                  
);   

//--------------------------------------------------------------------------------------------------  
/**                                                                                                   
 * Add handler function for EVENT 'le_mrc_NetRegStateEvent'                                           
 *                                                                                                    
 * This event provides information on network registration state changes.                             
 *                                                                                                    
 * @note <b>multi-app safe</b>                                                                        
 */                                                                                                   
//--------------------------------------------------------------------------------------------------  
extern int QL_NW_AddRegStateEventHandler                                 
(
    QL_NW_RegStateHandlerFunc_t handlerPtr, ///< [IN]                                                                                                      
    void                        *contextPtr ///< [IN]                                                                                     
);   

/*
    Get current network information: network name/country code/network code
*/
extern int QL_NW_GetNetworkNameMccMnc    
(     
    char        *nameStr,   ///< [OUT] the home network Name    
    size_t      nameLen,    ///< [IN]   
    char        *mccStr,    ///< [OUT] the mobile country code       
    size_t      mccLen,     ///< [IN]   
    char        *mncStr,    ///< [OUT] the mobile network code       
    size_t      mncLen      ///< [IN]   
);



/*
    Get current network register signal information: rat/state/rssi/ber
*/
extern int QL_NW_GetRegState  
(    
    E_QL_NW_RADIO_ACCESS_TYPE_T *rat,       ///< [OUT] The Radio Access Technology.  
    E_QL_WWAN_NET_REG_STATE_T   *state,     ///< [OUT] Network Registration state.   
    int                         *rssi,      ///< [OUT] [OUT] Received signal strength quality 
    int                         *ber        ///< [OUT] [OUT] Received signal bit error rate
);


/*
    Get current network location information: rat/mcc/mnc/lac/cid
*/
int QL_NW_GetServingCell(ST_CellInfo *pt_info);



#endif //__QL_NW_H__

