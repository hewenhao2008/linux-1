/**
 *@file     ql_sim.h
 *@date     2017-05-02
 *@author   
 *@brief    
 */   

#ifndef __QL_SIM_H__
#define __QL_SIM_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include "ql_type.h"


//--------------------------------------------------------------------------------------------------
/**
 * SIM states.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    E_QL_SIM_STAT_NOT_INSERTED,
    E_QL_SIM_STAT_READY,
    E_QL_SIM_STAT_PIN_REQ,
    E_QL_SIM_STAT_PUK_REQ,
    E_QL_SIM_STAT_BUSY,
    E_QL_SIM_STAT_BLOCKED,
    E_QL_SIM_STAT_UNKNOWN
}E_QL_SIM_STATES_T;


//--------------------------------------------------------------------------------------------------
/**
 * SIM identifiers.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    E_QL_SIM_EXTERNAL_SLOT_1,
    E_QL_SIM_EXTERNAL_SLOT_2,
    E_QL_SIM_EMBEDDED,
    E_QL_SIM_REMOTE,
    E_QL_SIM_ID_MAX
}E_QL_SIM_ID_T;


/*
    Get the sim card ICCID string.
     -simId     : sim ID
     -iccid     : input buffer
     -iccidLen  : input buffer length
*/
extern int  QL_SIM_GetICCID
(
    E_QL_SIM_ID_T   simId,      ///< [IN] The SIM identifier.
    char*           iccid,      ///< [OUT] ICCID
    size_t          iccidLen    ///< [IN]
);

/*
    Get the sim card IMSI string.
     -simId     : sim ID
     -imsi      : input buffer
     -imsiLen   : input buffer length
*/
extern int  QL_SIM_GetIMSI
(
    E_QL_SIM_ID_T   simId,      ///< [IN] The SIM identifier.
    char*           imsi,       ///< [OUT] IMSI
    size_t          imsiLen     ///< [IN]
);


//--------------------------------------------------------------------------------------------------
/**
 * This function been called to verify the PIN code.
 *
 * @return E_QL_BAD_PARAMETER The parameters are invalid.
 * @return E_QL_NOT_FOUND     The function failed to select the SIM card for this operation.
 * @return QL_UNDERFLOW     The PIN code is not long enough (min 4 digits).
 * @return E_QL_FAULT    The function failed to enter the PIN code.
 * @return E_QL_OK       The function succeeded.
 *
 * @note If PIN code is too long (max 8 digits), it is a fatal error, the
 *  function will not return.
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *  function will not return.
 */
//--------------------------------------------------------------------------------------------------
int  QL_SIM_VerifyPIN
(
    E_QL_SIM_ID_T   simId,  ///< [IN] The SIM identifier.
    const char      *pin    ///< [IN] The PIN code.
);


//--------------------------------------------------------------------------------------------------
/**
 * This function been called to change the PIN code.
 *
 * @note If PIN code is too long (max 8 digits), it is a fatal error, the
 *  function will not return.
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *  function will not return.
 */
//--------------------------------------------------------------------------------------------------
int  QL_SIM_ChangePIN
(
    E_QL_SIM_ID_T   simId,      ///< [IN] The SIM identifier.
    const char      *old_pin,   ///< [IN] The old PIN code.
    const char      *new_pin    ///< [IN] The new PIN code.
);


//--------------------------------------------------------------------------------------------------
/**
 * Unlock the SIM card: it will disables the request of PIN code.
 *
 * @note If PIN code is too long (max 8 digits), it is a fatal error, the
 *  function will not return.
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *  function will not return.
*/
//--------------------------------------------------------------------------------------------------
int  QL_SIM_Unlock
(
    E_QL_SIM_ID_T   simId,      ///< [IN] The SIM identifier.
    const char      *pin        ///< [IN] The PIN code.
);


//--------------------------------------------------------------------------------------------------
/**
 * Lock the SIM card: it will enables the request of PIN code.
 *
 * @note If PIN code is too long (max 8 digits), it is a fatal error, the
 *  function will not return.
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *  function will not return.
 */
//--------------------------------------------------------------------------------------------------
int  QL_SIM_Lock
(
    E_QL_SIM_ID_T   simId,  ///< [IN] The SIM identifier.
    const char      *pin    ///< [IN] The PIN code.
);


//--------------------------------------------------------------------------------------------------
/**
 * Unblock the SIM card: it will use puk to unblock the SIM card and set new pin.
 *
 * @note If PIN code is too long (max 8 digits), it is a fatal error, the
 *  function will not return.
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *  function will not return.
 */
//--------------------------------------------------------------------------------------------------
int  QL_SIM_Unblock
(
    E_QL_SIM_ID_T   simId,      ///< [IN] The SIM identifier.
    const char      *puk,       ///< [IN] The PIN code.
    const char      *newpin     ///< [IN] The PIN code.
);

/*
    Get the sim card PIN retry times.
     -simId     : sim ID
*/
extern int  QL_SIM_GetPINTriesCnt
(
     E_QL_SIM_ID_T simId    ///< [IN] The SIM identifier.
);


/*
    Get the sim card state.
     -simId     : sim ID
*/
extern E_QL_SIM_STATES_T  QL_SIM_GetState
(
    E_QL_SIM_ID_T simId    ///< [IN] The SIM identifier.
);



#endif // __QL_SIM_H__

