/**
 *@file     quectel-data-wwanAPI.h
 *@date     2017-02-17
 *@author   running.qian
 *@brief    High data operations interface Based on Qualcomm Data's Qcmap and Dsi Arch
 */
#ifndef _QUECTEL_DATA_WWANAPI_H_
#define _QUECTEL_DATA_WWANAPI_H_

/** Copying socket standard addr struct defined here for developer **/
/********************************************************************
//---------------------------IPV4------------------------------//
struct sockaddr_in {
     sa_family_t sin_family;             // AF_INET
     in_port_t sin_port;                 // Port number
     struct in_addr sin_addr;            // Internet address.

     // Pad to size of `struct sockaddr'.
     unsigned char sin_zero[sizeof (struct sockaddr) -
                            sizeof (sa_family_t) -
                            sizeof (in_port_t) -
                            sizeof (struct in_addr)];
};

typedef uint32_t in_addr_t;
struct in_addr  {
    in_addr_t s_addr;                    // IPv4 address
};

//---------------------------IPV6------------------------------//
struct sockaddr_in6 {
    sa_family_t sin6_family;    // AF_INET6
    in_port_t sin6_port;        // Transport layer port #
    uint32_t sin6_flowinfo;     // IPv6 flow information 
    struct in6_addr sin6_addr;  // IPv6 address 
    uint32_t sin6_scope_id;     // IPv6 scope-id
};
struct in6_addr {
    union {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    } in6_u;

    #define s6_addr                 in6_u.u6_addr8
    #define s6_addr16               in6_u.u6_addr16
    #define s6_addr32               in6_u.u6_addr32
};

//---------------------------SOCK ADDR(old)------------------------------//
struct sockaddr { 
    sa_family_t sa_family;
    char sa_data[14];            //protocol-specific address 
};

//---------------------------SOCK ADDR(new)------------------------------//
struct sockaddr_storage
{
     sa_family_t ss_family;
     __ss_aligntype __ss_align;  //Force desired alignment. 
     char __ss_padding[_SS_PADSIZE];
};
****************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/poll.h>
#include <linux/errno.h>
#include <arpa/inet.h>

/*sockaddr and sockaddr_storage helping macro */
#define QL_SA_FAMILY(addr)         (addr).sa_family
#define QL_SA_DATA(addr)           (addr).sa_data
#define QL_SASTORAGE_FAMILY(addr)  (addr).ss_family
#define QL_SASTORAGE_DATA(addr)    (addr).__ss_padding


typedef enum {
    QWWAN_IP_V4,
    QWWAN_IP_V6,
    QWWAN_IP_V4_V6
} Ql_wwan_ip_type_t;


typedef enum {
    QWWAN_UNKNOWN,  //-- Unknown or not applicable
    QWWAN_ATTACHED, //-- Attached
    QWWAN_DETACHED  //-- Detached
} Ql_wwan_attach_state_t;


typedef enum {
    QWWAN_NOT_REGISTERED,  /* Not registered; mobile is not currently 
                              searching for a new network to provide service 
                           */
    QWWAN_REGISTERED,   /* Registered with a network */
    QWWAN_NOT_REGISTERED_SEARCHING, /* Not registered, but mobile is currently  
                                       searching for a new network to provide service
                                    */
    QWWAN_REGISTRATION_DENIED, /* Registration denied by the visible network */
    QWWAN_REGISTRATION_UNKNOWN /* Registration state is unknown */
} Ql_wwan_registration_state_t;


typedef enum {
    QWWAN_UNKONWN,
    QWWAN_3GPP, //Connect the WAN on a UMTS network
    QWWAN_3GPP2 //Connect the WAN on a CDMA network
} Ql_wwan_network_t;


typedef enum {
    QWWAN_RAT_NULL_BEARER,
    QWWAN_RAT_3GPP_WCDMA,
    QWWAN_RAT_3GPP_GERAN,
    QWWAN_RAT_3GPP_LTE,
    QWWAN_RAT_3GPP_TDSCDMA,
    QWWAN_RAT_3GPP_WLAN,
    QWWAN_RAT_3GPP_MAX,

    QWWAN_RAT_3GPP2_1X,
    QWWAN_RAT_3GPP2_HRPD,
    QWWAN_RAT_3GPP2_EHRPD,
    QWWAN_RAT_3GPP2_WLAN,
    QWWAN_RAT_3GPP2_MAX,
    QWWAN_RAT_WLAN,
    QWWAN_RAT_WLAN_MAX
} Ql_wwan_bear_type_t;


typedef enum {
    QWWAN_CALL_IF_DSI,
    QWWAN_CALL_IF_QCMAP
} Ql_wwan_call_method_t;


typedef enum {
  QWWAN_CALL_IDLE,
  QWWAN_CALL_CONNECTING,
  QWWAN_CALL_CONNECTED, //get ip addr
  QWWAN_CALL_RECONNECT,
  QWWAN_CALL_DISCONNECTING,
  QWWAN_CALL_DISCONNECTED,
} Ql_wwan_call_state_t;

/** Data type for IPv4 configuration. */
typedef struct {
  struct in_addr public_ip;      /**< Public IP address. */
  struct in_addr gw_addr;        /**< Gateway IP address. */
  struct in_addr pri_dns_addr;   /**< Primary Domain Name Service (DNS) IP address. */
  struct in_addr sec_dns_addr;   /**< Secondary Domain Name Service (DNS) IP address. */
} Ql_wwan_v4_conf_t;

/** Data type for IPv6 configuration. */
typedef struct {
  struct in6_addr public_ip;     /**< Public IPv6 address. */
  struct in6_addr gw_addr;       /**< Gateway IP address. */
  struct in6_addr pri_dns_addr;  /**< Primary Domain Name Service (DNS) IPv6 address. */
  struct in6_addr sec_dns_addr;  /**< Secondary Domain Name Service (DNS) IPv6 address. */
} Ql_wwan_v6_conf_t;


/** Data type for network configuration. */
typedef struct {
     /** IPv4 configuration. */
    int v4_valid; //0:invalid, 1:valid
    Ql_wwan_v4_conf_t v4_conf;

    /** IPv6 configuration. */
    int v6_valid; //0:invalid, 1:valid
    Ql_wwan_v6_conf_t v6_conf;
} Ql_wwan_nw_params_t;

typedef struct {
    unsigned int profile_id;
    Ql_wwan_call_state_t state;
} Ql_wwan_evt_t;

typedef void (*Ql_wwan_call_evt_cbfun_t)(Ql_wwan_evt_t);


typedef struct {
    unsigned int profile_id;//0-4, 0 means APN1
    Ql_wwan_ip_type_t ip_type;
    bool autoconnect;
    Ql_wwan_call_evt_cbfun_t call_evt_cb;
} Ql_wwan_data_call_params_t;


typedef struct {
    bool enable;//false:down, true:up

    Ql_wwan_call_state_t sm;//state machine
    Ql_wwan_call_method_t call_if;
    Ql_wwan_ip_type_t call_ip_type;
    bool autoconnect;

    /* profile Params */
    char apn[16];
    unsigned int profile_id;

    /* Rm iface Params */
    char rm_name[16]; //iface name
    Ql_wwan_nw_params_t rm_conf;
} Ql_wwan_rm_info_t;//rmnet info


typedef struct {
    Ql_wwan_registration_state_t reg_state;
    Ql_wwan_attach_state_t attach_state;
    Ql_wwan_network_t network_type;
    Ql_wwan_bear_type_t bear_type;
} Ql_wwan_um_info_t;//rmnet info



bool Ql_wwan_init(void);

/*
 * Linux NetWork Interface(RmNet_Data) API
 */
bool Ql_wwan_rm_up(Ql_wwan_data_call_params_t *call_params);
bool Ql_wwan_rm_down(unsigned int profile_id);
bool Ql_wwan_rm_info_get(int profile_id, Ql_wwan_rm_info_t* rm_info);

/*
 * Modem Air interface API
 */
bool Ql_wwan_um_attach(void);
bool Ql_wwan_um_deattach(void);
bool Ql_wwan_um_info_get(Ql_wwan_um_info_t *um_info);

#ifdef __cplusplus
}
#endif
#endif