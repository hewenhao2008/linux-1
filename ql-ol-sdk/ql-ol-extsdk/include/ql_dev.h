/**
 *@file     ql_dev.h
 *@date     2017-05-02
 *@author   
 *@brief    
 */  

#ifndef __QL_DEV_H__
#define __QL_DEV_H__

/*
    Get the IMEI string.
     -imei      : input buffer
     -imeiLen   : input buffer length
*/
extern int QL_DEV_GetImei(char* imei, size_t imeiLen);


/*
    Get the FirmwareVersion string.
     -version   : input buffer
     -versionLen: input buffer length
*/
extern int QL_DEV_GetFirmwareVersion(char* version, size_t versionLen);


/*
    Get the DeviceModel string.
     -model     : input buffer
     -modelLen  : input buffer length
*/
extern int QL_DEV_GetDeviceModel(char* model, size_t modelLen);

/*
    Get the MEID string.
     -meid      : input buffer
     -meidLen   : input buffer length
*/
extern int QL_DEV_GetMeid(char* meid, size_t meidLen);

/*
    Get the ESN string.
     -esn       : input buffer
     -esnLen    : input buffer length
*/
extern int QL_DEV_GetEsn(char* esn, size_t esnLen);


#endif // __QL_DEV_H__

