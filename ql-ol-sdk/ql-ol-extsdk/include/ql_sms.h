/**
 *@file     ql_sms.h
 *@date     2017-05-02
 *@author   
 *@brief    
 */
 
#ifndef __QL_SMS_H__
#define __QL_SMS_H__

//#include <ql_oe.h>
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define QL_SMS_PHONE_NUM_MAX_LEN       20
#define QL_SMS_MSG_CENTER_ADDR_NUM_MAX_LEN  20

//-------------------------------------------------------------------------------------------------- 
/**  
 * Time stamp string length.      
 * The string format is "yy/MM/dd,hh:mm:ss+/-zz" (Year/Month/Day,Hour:Min:Seconds+/-TimeZone).  
 * One extra byte is added for the null character.   
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_TIMESTAMP_MAX_LEN 20    
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Time stamp string length (including the null-terminator).   
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_TIMESTAMP_MAX_BYTES 21  
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * The text message can be up to 160 characters long.     
 * One extra byte is added for the null character.   
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_TEXT_MAX_LEN 160   
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Test message string length (including the null-terminator). 
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_TEXT_MAX_BYTES 161      
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * The raw binary message can be up to 140 bytes long.    
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_BINARY_MAX_BYTES 140    
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * The UCS2 message can be up to 140 bytes long (70 characters).    
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_UCS2_MAX_BYTES 140      
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * The UCS2 message can be up to 70 characters (140 bytes long).    
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_UCS2_MAX_CHARS 70  
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * The PDU payload bytes long.    
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_PDU_MAX_PAYLOAD 140     
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * The PDU message can be up to 36 (header) + 140 (payload) bytes long.      
 */  
//-------------------------------------------------------------------------------------------------- 
#define QL_SMS_PDU_MAX_BYTES 176  
     

//-------------------------------------------------------------------------------------------------- 
/**  
 * Message Format.  
 */  
//-------------------------------------------------------------------------------------------------- 
typedef enum   
{    
    E_QL_SMS_FORMAT_PDU = 0,   ///< PDU message format.  
     
    E_QL_SMS_FORMAT_TEXT = 1,  ///< Text message format. 
     
    E_QL_SMS_FORMAT_BINARY = 2,///< Binary message format.    
     
    E_QL_SMS_FORMAT_UCS2 = 3,  ///< UCS2 message format. 
     
    E_QL_SMS_FORMAT_UNKNOWN = 4///< Unknown message format.   
}E_QL_SMS_FORMAT_T;    
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Message Type.    
 */  
//-------------------------------------------------------------------------------------------------- 
typedef enum   
{    
    E_QL_SMS_TYPE_RX = 0,      ///< SMS mobile terminated message.     
     
    E_QL_SMS_TYPE_TX = 1,      ///< SMS mobile originated message.     
     
    E_QL_SMS_TYPE_BROADCAST_RX = 2  ///< SMS Cell Broadcast message.   
}E_QL_SMS_TYPE_T; 
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Message Storage Type.    
 */  
//-------------------------------------------------------------------------------------------------- 
typedef enum   
{    
    E_QL_SMS_STORAGE_TYPE_SIM = 0,      ///< Sim SMS storage.    
     
    E_QL_SMS_STORAGE_TYPE_NV = 1,       ///< SNone volatile memory storage.  
     
    E_QL_SMS_STORAGE_TYPE_MAX = 2       ///< Undefined storage type.   
}E_QL_SMS_STORAGE_TYPE_T;    
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Message Mode.    
 */  
//-------------------------------------------------------------------------------------------------- 
typedef enum   
{
    E_QL_SMS_MODE_TYPE_CDMA = 0,      ///< SMS mode CDMA.    
    
    E_QL_SMS_MODE_TYPE_GW   = 1,      ///< SMS mode GW.  
     
    E_QL_SMS_MODE_TYPE_MAX  = 2       ///< Undefined SMS mode type.   
}E_QL_SMS_MODE_TYPE_T;  

//-------------------------------------------------------------------------------------------------- 
/**  
 * Message Status.  
 */  
//-------------------------------------------------------------------------------------------------- 
typedef enum   
{    
    E_QL_SMS_RX_READ = 0,      
   ///< Message present in the message storage has been read.  
     
    E_QL_SMS_RX_UNREAD = 1,    
   ///< Message present in the message storage has not been read.       
     
    E_QL_SMS_STORED_SENT = 2,  
   ///< Message saved in the message storage has been sent.    
     
    E_QL_SMS_STORED_UNSENT = 3,     
   ///< Message saved in the message storage has not been sent.    
     
    E_QL_SMS_SENT = 4,    
   ///< Message has been sent.    
     
    E_QL_SMS_SENDING = 5,      
   ///< Message has been in the sending pool.   
     
    E_QL_SMS_UNSENT = 6,       
   ///< Message has not been sent.    
     
    E_QL_SMS_SENDING_FAILED = 7,    
   ///< Message sending has Failed.   
     
    E_QL_SMS_SENDING_TIMEOUT = 8,   
   ///< Message sending has Failed due to timeout.   
     
    E_QL_SMS_STATUS_UNKNOWN = 9     
   ///< Unknown message status.   
}E_QL_SMS_STATUS_T;    
     

typedef struct
{
    E_QL_SMS_FORMAT_T       format;
    E_QL_SMS_TYPE_T         type;
    E_QL_SMS_STORAGE_TYPE_T e_storage;
    int                     msgPos;//the position stored in phone (+CMTI: "ME",9)
    uint16_t                brcstId;
    uint16_t                brcstSerialNum;
    uint32_t                timeout;
    char                    PhoneNum[QL_SMS_PHONE_NUM_MAX_LEN];      ///Telephone number string.  
    char                    smsCenterAddr[QL_SMS_MSG_CENTER_ADDR_NUM_MAX_LEN];  ///SMS center address number string.
    char                    timestamp[QL_SMS_TIMESTAMP_MAX_BYTES];    ///Message time stamp (in text mode). string format: "yy/MM/dd,hh:mm:ss+/-TimeZone" 
    size_t                  userdataLen;  ///Number of characters for text and UCS2 messages, or the length of the data in bytes for raw binary messages
    char                    textData[QL_SMS_TEXT_MAX_BYTES]; ///SMS text.
    size_t                  binLen;
    char                    binData[QL_SMS_BINARY_MAX_BYTES];    ///Binary message.
    size_t                  ucs2Len;
    wchar_t                 ucs2Data[QL_SMS_UCS2_MAX_CHARS];
    size_t                  pduLen;
    char                    pduData[QL_SMS_PDU_MAX_BYTES];
} sms_info_t;

typedef struct
{
    E_QL_SMS_STORAGE_TYPE_T e_storage;
    E_QL_SMS_MODE_TYPE_T    e_mode;
    uint32_t                storage_idx;
} sms_storage_info_t;

//-------------------------------------------------------------------------------------------------- 
/**  
 * Declare a reference type for referring to SMS Message objects.   
 */  
//-------------------------------------------------------------------------------------------------- 
//typedef struct QL_SMS_Msg* ST_SMS_MsgRef;
typedef sms_info_t* ST_SMS_MsgRef;   
     
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Reference type used by Add/Remove functions for EVENT 'QL_SMS_RxMessage'  
 */  
//-------------------------------------------------------------------------------------------------- 
//typedef struct QL_SMS_RxMessageHandler* QL_SMS_RxMessageHandlerRef_t;
typedef int QL_SMS_RxMessageHandlerRef_t;

     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Handler for Sending result.    
 *   
 * @param msgRef    
 *   Reference to the message object.      
 * @param status    
 *   Status result.     
 * @param contextPtr    
 */  
//-------------------------------------------------------------------------------------------------- 
typedef void (*QL_SMS_CallbackResultFunc_t)     
(    
    ST_SMS_MsgRef msgRef,  
    E_QL_SMS_STATUS_T status,  
    void* contextPtr    
);   
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Handler for New Message.  
 *   
 *   
 * @param msgRef    
 *   Message reference.      
 * @param contextPtr    
 */  
//-------------------------------------------------------------------------------------------------- 
typedef void (*QL_SMS_RxMessageHandlerFunc_t)   
(    
    ST_SMS_MsgRef msgRef,  
    void* contextPtr    
);   
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Add handler function for EVENT 'QL_SMS_RxMessage' 
 *   
 * This event provides information on new received messages.   
 */  
//-------------------------------------------------------------------------------------------------- 
QL_SMS_RxMessageHandlerRef_t QL_SMS_AddRxMessageHandler   
(    
    QL_SMS_RxMessageHandlerFunc_t handlerPtr,   
   ///< [IN]   
     
    void* contextPtr    
   ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Create an SMS Message data structure.   
 *   
 * @return Reference to the new Message object.      
 *   
 * @note       
 *      On failure, the process exits, so you don't have to worry about checking the returned   
 *      reference for validity.   
 */  
//-------------------------------------------------------------------------------------------------- 
ST_SMS_MsgRef QL_SMS_Create     
(    
    void       
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Set the timeout to send a SMS Message.       
 *   
 *   
 * @note       
 *      On failure, the process exits, so you don't have to worry about checking the returned   
 *      reference for validity.   
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetTimeout     
(    
    ST_SMS_MsgRef msgRef,   ///< [IN] Reference to the message object.   
    uint32_t timeout        ///< [IN] Timeout in seconds.  
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Set the Telephone destination number.   
 *   
 * @note If telephone destination number is too long (max QL_MDMDEFS_PHONE_NUM_MAX_LEN   
 *  digits), it is a fatal error, the function will not return.    
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetDestination      
(    
    ST_SMS_MsgRef msgRef,  ///< [IN] Reference to the message object.   
    const char* dest        ///< [IN] Telephone number string.      
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * This function must be called to set the Text Message content.    
 *   
 * @note If message is too long (max QL_SMS_TEXT_MAX_LEN digits), it is a fatal error, the      
 *  function will not return.     
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetText   
(    
    ST_SMS_MsgRef msgRef,   ///< [IN] Reference to the message object.   
    const char* text        ///< [IN] SMS text.  
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Set the binary message content.     
 *   
 * @note If length of the data is too long (max QL_SMS_BINARY_MAX_BYTES bytes), it is a fatal   
 *  error, the function will not return.   
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetBinary      
(    
    ST_SMS_MsgRef msgRef,  ///< [IN] Reference to the message object.   
     
    const char* binPtr,    ///< [IN] Binary data.    
     
    size_t binNumElements  ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Set the UCS2 message content (16-bit format).     
 *   
 *   
 * @note If length of the data is too long (max QL_SMS_UCS2_MAX_CHARS), it is a fatal  
 *  error, the function will not return.   
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetUCS2   
(    
    ST_SMS_MsgRef msgRef,  ///< [IN] Reference to the message object.   
     
    const uint16_t* ucs2Ptr,///< [IN] UCS2 message.   
     
    size_t ucs2NumElements  ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Set the PDU message content.   
 *   
 *   
 * @note If length of the data is too long (max QL_SMS_PDU_MAX_BYTES bytes), it is a fatal error,    
 *  the function will not return. 
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetPDU    
(    
    ST_SMS_MsgRef msgRef,   ///< [IN] Reference to the message object.   
     
    const char* pduPtr,     ///< [IN] PDU message.    
     
    size_t pduNumElements   ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Send an SMS message.      
 *   
 * Verifies first if the parameters are valid, then it checks the modem state can support       
 * message sending.     
 *   
 * @return QL_FORMAT_ERROR  Message content is invalid.   
 * @return E_QL_FAULT    Function failed to send the message.    
 * @return E_QL_OK       Function succeeded.      
 * @return QL_TIMEOUT       Timeout before the complete sending.    
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_Send      
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
  //-------------------------------------------------------------------------------------------------- 
/**  
 * Create and synchronously send a text message.    
 *   
 * @return     
 *  - QL_SMS_Msg Reference to the new Message object pooled.   
 *  - NULL Not possible to pool a new message.  
 *   
 * @note If telephone destination number is too long is too long (max QL_MDMDEFS_PHONE_NUM_MAX_LEN   
 *  digits), it is a fatal error, the function will not return.    
 * @note If message is too long (max QL_SMS_TEXT_MAX_LEN digits), it is a fatal error, the      
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
ST_SMS_MsgRef QL_SMS_SendText   
(    
    const char                  *destStr,   ///< [IN] Telephone number string.      
    const char                  *textStr,   ///< [IN] SMS text.  
    QL_SMS_CallbackResultFunc_t handlerPtr, ///< [IN]   
    void                        *contextPtr ///< [IN]   
);
  
//-------------------------------------------------------------------------------------------------- 
/**  
 * Delete a Message data structure.    
 *   
 * It deletes the Message data structure and all the allocated memory is freed. If several      
 * users own the Message object (e.g., several handler functions registered for   
 * SMS message reception), the Message object will only be deleted if one User    
 * owns the Message object.  
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
void QL_SMS_Delete      
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the message format.   
 *   
 * @return Message format.   
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 *   
 * @note For incoming SMS, format returned by QL_SMS_GetFormat is never QL_SMS_FORMAT_PDU.      
 */  
//-------------------------------------------------------------------------------------------------- 
E_QL_SMS_FORMAT_T QL_SMS_GetFormat  
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the message type.     
 *   
 * @return Message type.     
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
E_QL_SMS_TYPE_T QL_SMS_GetType      
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the Sender Telephone number.    
 *   
 * Output parameter is updated with the Telephone number. If the Telephone number string exceeds     
 * the value of 'len' parameter,  E_QL_OVERFLOW error code is returned and 'tel' is filled until       
 * 'len-1' characters and a null-character is implicitly appended at the end of 'tel'. 
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetSenderTel   
(    
    ST_SMS_MsgRef msgRef,   ///< [IN] Reference to the message object.   
     
    char* tel,              ///< [OUT] Telephone number string.     
     
    size_t telNumElements   ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the Service Center Time Stamp string.    
 *   
 * Output parameter is updated with the Time Stamp string. If the Time Stamp string exceeds the      
 * value of 'len' parameter, a E_QL_OVERFLOW error code is returned and 'timestamp' is filled until    
 * 'len-1' characters and a null-character is implicitly appended at the end of 'timestamp'.    
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetTimeStamp   
(    
    ST_SMS_MsgRef msgRef,  ///< [IN] Reference to the message object.   
     
    char* timestamp,    
   ///< [OUT] Message time stamp (in text mode).     
   ///<       string format: "yy/MM/dd,hh:mm:ss+/-zz"     
   ///<  (Year/Month/Day,Hour:Min:Seconds+/-TimeZone)    
     
    size_t timestampNumElements   ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the message Length value.  
 *   
 * @return Number of characters for text and UCS2 messages, or the length of the data in bytes for   
 *  raw binary messages.     
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
size_t QL_SMS_GetUserdataLen      
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the text Message.     
 *   
 * Output parameter is updated with the text string encoded in ASCII format. If the text string      
 * exceeds the value of 'len' parameter, E_QL_OVERFLOW error code is returned and 'text' is filled     
 * until 'len-1' characters and a null-character is implicitly appended at the end of 'text'.   
 *   
 * @return E_QL_OVERFLOW      Message length exceed the maximum length.   
 * @return E_QL_OK       Function succeeded.      
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetText   
(    
    ST_SMS_MsgRef msgRef,   ///< [IN] Reference to the message object.   
     
    char* text,             ///< [OUT] SMS text.      
     
    size_t textNumElements  ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the binary Message.   
 *   
 * Output parameters are updated with the binary message content and the length of the raw      
 * binary message in bytes. If the binary data exceed the value of 'len' input parameter, a     
 * E_QL_OVERFLOW error code is returned and 'raw' is filled until 'len' bytes.      
 *   
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetBinary      
(    
    ST_SMS_MsgRef msgRef,       ///< [IN] Reference to the message object.   
     
    char* binPtr,               ///< [OUT] Binary message.     
     
    size_t* binNumElementsPtr   ///< [INOUT]    
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the UCS2 Message (16-bit format).   
 *   
 * Output parameters are updated with the UCS2 message content and the number of characters. If      
 * the UCS2 data exceed the value of the length input parameter, a E_QL_OVERFLOW error   
 * code is returned and 'ucs2Ptr' is filled until of the number of chars specified.    
 *   
 *   
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetUCS2   
(    
    ST_SMS_MsgRef msgRef,       ///< [IN] Reference to the message object.   
     
    uint16_t* ucs2Ptr,          ///< [OUT] UCS2 message.  
     
    size_t* ucs2NumElementsPtr  ///< [INOUT]    
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the PDU message.      
 *   
 * Output parameters are updated with the PDU message content and the length of the PDU message      
 * in bytes. If the PDU data exceed the value of 'len' input parameter, a E_QL_OVERFLOW error code is  
 * returned and 'pdu' is filled until 'len' bytes.   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetPDU    
(    
    ST_SMS_MsgRef msgRef,       ///< [IN] Reference to the message object.   
     
    char* pduPtr,               ///< [OUT] PDU message.   
     
    size_t* pduNumElementsPtr   ///< [INOUT]    
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the message Length value.  
 *   
 * @return Length of the data in bytes of the PDU message.     
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
size_t QL_SMS_GetPDULen      
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Delete an SMS message from the storage area.      
 *   
 * Verifies first if the parameter is valid, then it checks the modem state can support    
 * message deleting.    
 *   
 *   
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the    
 *  function will not return.     
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_DeleteFromStorage   
(    
    ST_SMS_MsgRef msgRef   ///< [IN] Reference to the message object.   
);   
     
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Get the SMS center address.    
 *   
 * Output parameter is updated with the SMS Service center address. If the Telephone number string exceeds
 * the value of 'len' parameter,  E_QL_OVERFLOW error code is returned and 'tel' is filled until       
 * 'len-1' characters and a null-character is implicitly appended at the end of 'tel'. 
 *   
 *   
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_GetSmsCenterAddress     
(    
    ST_SMS_MsgRef     msgRef,         ///< [IN] Reference to the message object.  
    char* tel,                          ///< [OUT] SMS center address number string. 
    size_t telNumElements               ///< [IN]   
);   
     
//-------------------------------------------------------------------------------------------------- 
/**  
 * Set the SMS center address.    
 *   
 * SMS center address number is defined in ITU-T recommendations E.164/E.163.     
 * E.164 numbers can have a maximum of fifteen digits and are usually written with a '+' prefix.     
 *   
 *   
 * @note If the SMS center address number is too long (max QL_MDMDEFS_PHONE_NUM_MAX_LEN digits), it  
 *  is a fatal error, the function will not return.  
 */  
//-------------------------------------------------------------------------------------------------- 
int QL_SMS_SetSmsCenterAddress     
(    
    ST_SMS_MsgRef     msgRef,         ///< [IN] Reference to the message object.  
    const char* tel                     ///< [IN] SMS center address number string.  
);   

int QL_SMS_SetPreferredStorage     
(    
    ST_SMS_MsgRef           msgRef,         ///< [IN] Reference to the message object.  
    E_QL_SMS_STORAGE_TYPE_T e_storage_type  ///< [IN] SMS storage type.  
);
     
#endif // __QL_SMS_H__      
     
