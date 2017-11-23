//---------------------------------------------------------------------------
#ifndef QL_SMS_PARSER_H
#define QL_SMS_PARSER_H

//--Include file
#include <stdlib.h>
//#include "ql_type.h"
#include "ql_sms.h"



#define MAX_UD 70
#define LONG_SMS_HEAD_6BIT ("050003") // 6-bit codec header
#define LONG_SMS_HEAD_7BIT ("060804") // 7-bit codec header

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

extern int QL_SMS_Decode(const char *s_in_pdu,  sms_info_t  *pt_sms_info);
extern int QL_SMS_Encode(sms_info_t  *pt_sms_info);



#endif //QL_SMS_PARSER_H
