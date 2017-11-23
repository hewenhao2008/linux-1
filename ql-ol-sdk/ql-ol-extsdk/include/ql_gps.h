#ifndef __QL_GPS_H__
#define __QL_GPS_H__

#if 0
//--------------------------------------------------------------------------------------------------
/**
 * Type for handler called when a server disconnects.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*QL_GPS_DisconnectHandler_t)(void *);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Connect the current client thread to the service providing this API. Block until the service is
 * available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void QL_GPS_ConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Try to connect the current client thread to the service providing this API. Return with an error
 * if the service is not available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 *
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_TryConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Set handler called when server disconnection is detected.
 *
 * When a server connection is lost, call this handler then exit with- QL_FATAL.  If a program wants
 * to continue without exiting, it should call longjmp() from inside the handler.
 */
//--------------------------------------------------------------------------------------------------
void QL_GPS_SetServerDisconnectHandler
(
    QL_GPS_DisconnectHandler_t  disconnectHandler,
    void                        *contextPtr
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Disconnect the current client thread from the service providing this API.
 *
 * Normally, this function doesn't need to be called. After this function is called, there's no
 * longer a connection to the service, and the functions in this API can't be used. For details, see
 * @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void QL_GPS_DisconnectService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 *  Fix position states.
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    QL_GPS_STATE_NO_FIX = 0,        ///< The position is not fixed.
    QL_GPS_STATE_FIX_2D = 1,        ///< 2-Dimensional position fix.
    QL_GPS_STATE_FIX_3D = 2,        ///< 3-Dimensional position fix.
    QL_GPS_STATE_FIX_ESTIMATED = 3, ///< Estimated (i.e. forward predicted) position fix.
    QL_GPS_STATE_UNKNOWN = 4        ///< Unknow state.
}
QL_GPS_FixState_t;


//--------------------------------------------------------------------------------------------------
/**
 *  Reference type for dealing with Position samples.
 */
//--------------------------------------------------------------------------------------------------
typedef struct QL_GPS_Sample* QL_GPS_SampleRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'QL_GPS_Movement'
 */
//--------------------------------------------------------------------------------------------------
typedef struct QL_GPS_MovementHandler* QL_GPS_MovementHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Handler for Movement changes.
 *
 */
//--------------------------------------------------------------------------------------------------
typedef void (*QL_GPS_MovementHandlerFunc_t)
(
    QL_GPS_SampleRef_t  positionSampleRef,  ///< Position's sample reference
    void                *contextPtr         ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'QL_GPS_Movement'
 *
 * This event provides information on movement changes.
 *
 */
//--------------------------------------------------------------------------------------------------
QL_GPS_MovementHandlerRef_t QL_GPS_AddMovementHandler
(
    uint32_t                        horizontalMagnitude,    ///< [IN] Horizontal magnitude in meters. 0 means that I don't care about changes in the latitude and longitude.
    uint32_t                        verticalMagnitude,      ///< [IN] Vertical magnitude in meters. 0 means that I don't care about changes in the altitude.
    QL_GPS_MovementHandlerFunc_t    handlerPtr,             ///< [IN]
    void                            *contextPtr             ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'QL_GPS_Movement'
 */
//--------------------------------------------------------------------------------------------------
void QL_GPS_RemoveMovementHandler
(
    QL_GPS_MovementHandlerRef_t handlerRef
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the 2D location's data (Latitude, Longitude, Horizontal
 * accuracy).
 *
 * @note latitudePtr, longitudePtr, hAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Get2DLocation
(
    int32_t* latitudePtr,           ///< [OUT] WGS84 Latitude in degrees, positive North [resolution 1e-6].
    int32_t* longitudePtr,          ///< [OUT] WGS84 Longitude in degrees, positive East [resolution 1e-6].
    int32_t* hAccuracyPtr           ///< [OUT] Horizontal position's accuracy in meters.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the 3D location's data (Latitude, Longitude, Altitude,
 * Horizontal accuracy, Vertical accuracy).
 *
 * @note latitudePtr, longitudePtr,hAccuracyPtr, altitudePtr, vAccuracyPtr can be set to NULL
 *       if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Get3DLocation
(
    int32_t* latitudePtr,           ///< [OUT] WGS84 Latitude in degrees, positive North [resolution 1e-6].
    int32_t* longitudePtr,          ///< [OUT] WGS84 Longitude in degrees, positive East [resolution 1e-6].
    int32_t* hAccuracyPtr,          ///< [OUT] Horizontal position's accuracy in meters.
    int32_t* altitudePtr,           ///< [OUT] Altitude in meters, above Mean Sea Level.
    int32_t* vAccuracyPtr           ///< [OUT] Vertical position's accuracy in meters.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the time of the last updated location
 *
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_GetTime
(
    uint16_t* hoursPtr,             ///< [OUT] UTC Hours into the day [range 0..23].
    uint16_t* minutesPtr,           ///< [OUT] UTC Minutes into the hour [range 0..59].
    uint16_t* secondsPtr,           ///< [OUT] UTC Seconds into the minute [range 0..59].
    uint16_t* millisecondsPtr       ///< [OUT] UTC Milliseconds into the second [range 0..999].
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the date of the last updated location
 *
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_GetDate
(
    uint16_t* yearPtr,              ///< [OUT] UTC Year A.D. [e.g. 2014].
    uint16_t* monthPtr,             ///< [OUT] UTC Month into the year [range 1...12].
    uint16_t* dayPtr                ///< [OUT] UTC Days into the month [range 1...31].
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the motion's data (Horizontal Speed, Horizontal Speed's
 * accuracy, Vertical Speed, Vertical Speed's accuracy).
 *
 * @note hSpeedPtr, hSpeedAccuracyPtr, vSpeedPtr, vSpeedAccuracyPtr can be set to NULL if not
 *       needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_GetMotion
(
    uint32_t* hSpeedPtr,            ///< [OUT] Horizontal Speed in m/sec.
    uint32_t* hSpeedAccuracyPtr,    ///< [OUT] Horizontal Speed's accuracy in m/sec.
    int32_t*  vSpeedPtr,            ///< [OUT] Vertical Speed in m/sec, positive up.
    int32_t*  vSpeedAccuracyPtr     ///< [OUT] Vertical Speed's accuracy in m/sec.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the heading indication.
 *
 * @note Heading is given in degrees.
 *       Heading ranges from 0 to 359 degrees, where 0 is True North.
 *
 * @note headingPtr, headingAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_GetHeading
(
    uint32_t* headingPtr,           ///< [OUT] Heading in degrees. Range: 0 to 359, where 0 is True North.
    uint32_t* headingAccuracyPtr    ///< [OUT] Heading's accuracy in degrees.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the direction indication. Direction of movement is the direction that the vehicle or person
 * is actually moving.
 *
 * @note Direction is given in degrees.
 *       Direction ranges from 0 to 359 degrees, where 0 is True North.
 *
 * @note directionPtr, directionAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_GetDirection
(
    uint32_t* directionPtr,         ///< [OUT] Direction indication in degrees. Range: 0 to 359, where 0 is True North.
    uint32_t* directionAccuracyPtr  ///< [OUT] Direction's accuracy estimate in degrees.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position fix state
 *
 * @note In case the function fails to get the position fix state, a fatal error occurs,
 *       the function will not return.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_GetFixState
(
    QL_GPS_FixState_t* statePtr     ///< [OUT] Position fix state.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's 2D location (latitude, longitude,
 * horizontal accuracy).
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 *
 * @note latitudePtr, longitudePtr, horizontalAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_Get2DLocation
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    int32_t*            latitudePtr,            ///< [OUT] WGS84 Latitude in degrees, positive North [resolution 1e-6].
    int32_t*            longitudePtr,           ///< [OUT] WGS84 Longitude in degrees, positive East [resolution 1e-6].
    int32_t*            horizontalAccuracyPtr   ///< [OUT] Horizontal position's accuracy in meters.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's time.
 *
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetTime
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    uint16_t*           hoursPtr,               ///< [OUT] UTC Hours into the day [range 0..23].
    uint16_t*           minutesPtr,             ///< [OUT] UTC Minutes into the hour [range 0..59].
    uint16_t*           secondsPtr,             ///< [OUT] UTC Seconds into the minute [range 0..59].
    uint16_t*           millisecondsPtr         ///< [OUT] UTC Milliseconds into the second [range 0..999].
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's date.
 *
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetDate
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    uint16_t*           yearPtr,                ///< [OUT] UTC Year A.D. [e.g. 2014].
    uint16_t*           monthPtr,               ///< [OUT] UTC Month into the year [range 1...12].
    uint16_t*           dayPtr                  ///< [OUT] UTC Days into the month [range 1...31].
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's altitude.
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 *
 * @note altitudePtr, altitudeAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetAltitude
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    int32_t*            altitudePtr,            ///< [OUT] Altitude in meters, above Mean Sea Level.
    int32_t*            altitudeAccuracyPtr     ///< [OUT] Vertical position's accuracy in meters.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's horizontal speed.
 * 
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 *
 * @note hSpeedPtr, hSpeedAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetHorizontalSpeed
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    uint32_t*           hSpeedPtr,              ///< [OUT] The Horizontal Speed in m/sec.
    uint32_t*           hSpeedAccuracyPtr       ///< [OUT] The Horizontal Speed's accuracy in m/sec.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's vertical speed.
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 *
 * @note vSpeedPtr, vSpeedAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetVerticalSpeed
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    int32_t*            vspeedPtr,              ///< [OUT] The Vertical Speed in m/sec, positive up.
    int32_t*            vspeedAccuracyPtr       ///< [OUT] The Vertical Speed's accuracy in m/sec.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's heading. Heading is the direction that the vehicle or person is facing.
 *
 * @note Heading is given in degrees.
 *       Heading ranges from 0 to 359 degrees, where 0 is True North.
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 *
 * @note headingPtr, headingAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetHeading
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    uint32_t*           headingPtr,             ///< [OUT] Heading in degrees. Range: 0 to 359, where 0 is True North.
    uint32_t*           headingAccuracyPtr      ///< [OUT] Heading's accuracy estimate in degrees.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's direction. Direction of movement is the direction that the vehicle or
 * person is actually moving.
 *
 * @note Direction is given in degrees.
 *       Direction ranges from 0 to 359 degrees, where 0 is True North.
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 *
 * @note directionPtr, directionAccuracyPtr can be set to NULL if not needed.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetDirection
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    uint32_t*           directionPtr,           ///< [OUT] Direction indication in degrees. Range: 0 to 359, where 0 is True North.
    uint32_t*           directionAccuracyPtr    ///< [OUT] Direction's accuracy estimate in degrees.
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the position sample's fix state.
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_Sample_GetFixState
(
    QL_GPS_SampleRef_t  positionSampleRef,      ///< [IN] Position sample's reference.
    QL_GPS_FixState_t*  statePtr                ///< [OUT] Position fix state.
);

//--------------------------------------------------------------------------------------------------
/**
 * Release the position sample.
 *
 * @note If the caller is passing an invalid Position reference into this function,
 *       it is a fatal error, the function will not return.
 */
//--------------------------------------------------------------------------------------------------
void QL_GPS_Sample_Release
(
    QL_GPS_SampleRef_t  positionSampleRef       ///< [IN] Position sample's reference.
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the acquisition rate.
 *
 */
//--------------------------------------------------------------------------------------------------
int QL_GPS_SetAcquisitionRate
(
    uint32_t acquisitionRate                    ///< [IN] Acquisition rate in milliseconds.
);

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the acquisition rate.
 *
 * @return
 *    Acquisition rate in milliseconds.
 */
//--------------------------------------------------------------------------------------------------
uint32_t QL_GPS_GetAcquisitionRate
(
    void
);

#else

#define     LOC_IND_LOCATION_INFO_ON            (1 << 0)
#define     LOC_IND_STATUS_INFO_ON              (1 << 1)
#define     LOC_IND_SV_INFO_ON                  (1 << 2)
#define     LOC_IND_NMEA_INFO_ON                (1 << 3)
#define     LOC_IND_CAP_INFO_ON                 (1 << 4)
#define     LOC_IND_UTC_TIME_REQ_ON             (1 << 5)
#define     LOC_IND_XTRA_DARA_REQ_ON            (1 << 6)
#define     LOC_IND_AGPS_DATA_CONN_CMD_REQ_ON   (1 << 7)
#define     LOC_IND_NI_NFY_USER_RESP_REQ_ON     (1 << 8)

typedef uint32 loc_client_handle_type;

typedef enum 
{
    E_QL_LOC_POS_MODE_STANDALONE        = 0, /**<  Mode for running GPS standalone (no assistance).  */
    E_QL_LOC_POS_MODE_MS_BASED          = 1, /**<  AGPS MS-Based mode.  */
    E_QL_LOC_POS_MODE_MS_ASSISTED       = 2, /**<  AGPS MS-Assisted mode.  */
}E_QL_LOC_POS_MODE_T;

typedef enum 
{
    E_QL_LOC_POS_RECURRENCE_PERIODIC    = 0, /**<  Receive GPS fixes on a recurring basis at a specified period.  */
    E_QL_LOC_POS_RECURRENCE_SINGLE      = 1, /**<  Request a single-shot GPS fix.  */
}E_QL_LOC_POS_RECURRENCE_T;

typedef struct 
{
  E_QL_LOC_POS_MODE_T       mode;
  E_QL_LOC_POS_RECURRENCE_T recurrence;
  uint32_t                  min_interval;
  uint32_t                  preferred_accuracy;
  uint32_t                  preferred_time;
}QL_LOC_POS_MODE_INFO_T; 

typedef enum 
{
    E_QL_LOC_LOCATION_LAT_LONG_VALID   = 0x0001, /**<  GPS location has valid latitude and longitude.  */
    E_QL_LOC_LOCATION_ALTITUDE_VALID   = 0x0002, /**<  GPS location has a valid altitude.  */
    E_QL_LOC_LOCATION_SPEED_VALID      = 0x0004, /**<  GPS location has a valid speed.  */
    E_QL_LOC_LOCATION_BEARING_VALID    = 0x0008, /**<  GPS location has a valid bearing.  */
    E_QL_LOC_LOCATION_ACCURACY_VALID   = 0x0010, /**<  GPS location has valid accuracy.  */
    E_QL_LOC_LOCATION_SOURCE_INFO_VALID= 0x0020, /**<  GPS location has valid source information.  */
    E_QL_LOC_LOCATION_IS_INDOOR_VALID  = 0x0040, /**<  GPS location has a valid "is indoor?" flag.  */
    E_QL_LOC_LOCATION_FLOOR_NUMBE_VALID= 0x0080, /**<  GPS location has a valid floor number.  */
    E_QL_LOC_LOCATION_MAP_URL_VALID    = 0x0100, /**<  GPS location has a valid map URL.  */
    E_QL_LOC_LOCATION_MAP_INDEX_VALID  = 0x0200, /**<  GPS location has a valid map index.  */
}E_QL_LOC_LOCATION_VALID_FLAG;

typedef enum 
{
    E_QL_LOC_ULP_LOCATION_SOURCE_HYBRID= 0x0001, /**<  Position source is ULP.  */
    E_QL_LOC_ULP_LOCATION_SOURCE_GNSS  = 0x0002, /**<  Position source is GNSS only.  */
}E_QL_LOC_ULP_LOCATION_SOURCE;

typedef struct 
{
    uint32_t    size;                   /**<   Set to the size of mcm_gps_location_t. */
    E_QL_LOC_LOCATION_VALID_FLAG flags; /**<   Contains GPS location flags bits. */
    E_QL_LOC_ULP_LOCATION_SOURCE position_source;  /**<   Provider indicator for HYBRID or GPS. */
    double      latitude;               /**<   Latitude in degrees. */
    double      longitude;              /**<   Longitude in degrees. */
    double      altitude;               /**<   Altitude in meters above the WGS 84 reference ellipsoid. */
    float       speed;                  /**<   Speed in meters per second. */
    float       bearing;                /**<   Heading in degrees. */
    float       accuracy;               /**<   Expected accuracy in meters. */
    int64_t     timestamp;              /**<   Timestamp for the location fix. */    
    int32_t     is_indoor;              /**<   Location is indoors. */
    float       floor_number;           /**<   Indicates the floor number. */
}QL_LOC_LOCATION_INFO_T;  /* Type */


/* Set what we want callbacks for */
int QL_LOC_Set_Indications(loc_client_handle_type h_loc, int bit_mask);

int QL_LOC_Set_Position_Mode(loc_client_handle_type h_loc, QL_LOC_POS_MODE_INFO_T *pt_mode);

int QL_LOC_Start_Navigation(loc_client_handle_type h_loc);

int QL_LOC_Stop_Navigation(loc_client_handle_type h_loc);

/* Set up GPS connection and start getting coordinates */
int QL_LOC_Get_Current_Location(loc_client_handle_type h_loc, QL_LOC_LOCATION_INFO_T *pt_loc_info);

int QL_LOC_Client_Init(loc_client_handle_type  *ph_loc);

/* Stop getting coordinates */
int QL_LOC_Client_Deinit(loc_client_handle_type h_loc);

#endif

#endif // __QL_GPS_H__

