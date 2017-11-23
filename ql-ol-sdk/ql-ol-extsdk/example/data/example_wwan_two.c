#include <stdio.h>
#include <ql_data_wwan.h>

#define QL_USER_LOG(format, ...) fprintf(stderr, "[Ql_test][%d]:"format, __LINE__, ##__VA_ARGS__)

void Ql_wwan_call_evt_cb (Ql_wwan_evt_t Ql_wwan_evt)
{
    char ip_str[64];
    Ql_wwan_rm_info_t rmnetdata_info;

    switch (Ql_wwan_evt.state) {
    case QWWAN_CALL_IDLE:
        QL_USER_LOG("data call enter idle state\n\r");
        QL_USER_LOG("If you want to data call, please call Ql_wwan_rm_up() again\n\r");
        //\TODO::maybe user need call "Ql_wwan_rm_up" again
        //\TODO::report user app
        break;

    case QWWAN_CALL_CONNECTING:
        QL_USER_LOG("User Got APN%d connecting evt:\n\r", Ql_wwan_evt.profile_id);
        //\TODO:report user app
        break;

    case QWWAN_CALL_CONNECTED:
        //\TODO::maybe user refresh nw params,just next below:
        Ql_wwan_rm_info_get(Ql_wwan_evt.profile_id, &rmnetdata_info);
        QL_USER_LOG("User Got APN%d connected evt:\n\r", Ql_wwan_evt.profile_id);
        QL_USER_LOG("iface: %s\n", rmnetdata_info.rm_name);
        if (rmnetdata_info.rm_conf.v4_valid == 1) {
            QL_USER_LOG("ipv4addr: %s\n", inet_ntop(AF_INET,
                &rmnetdata_info.rm_conf.v4_conf.public_ip.s_addr, ip_str, sizeof(ip_str)));
        }
        //\TODO::report user app
        
        break;

    case QWWAN_CALL_DISCONNECTING:
        QL_USER_LOG("User Got APN%d disconnecting evt:\n\r", Ql_wwan_evt.profile_id);
        //\TODO::report user app
        break;

    case QWWAN_CALL_DISCONNECTED:
        QL_USER_LOG("User Got APN%d disconnected evt:\n\r", Ql_wwan_evt.profile_id);
        //\TODO::report user app
        break;

    case QWWAN_CALL_RECONNECT:
        QL_USER_LOG("User Got APN%d reconnect evt :\n\r", Ql_wwan_evt.profile_id);
        //\TODO::report user app
        break;

    default:
        QL_USER_LOG("unkonwn evt, don't care!!!\n\r");
        break;
    }
}


int main(int argc, char *argv[])
{
    bool call_ret1, call_ret2;
    int profile_id1, profile_id2;
    Ql_wwan_um_info_t um_info;
    Ql_wwan_data_call_params_t call_params;

    profile_id1 = 0;
    profile_id2 = -1;
    if (argc == 2) {
        profile_id1 = argv[1][0] - '0';
    }
    else if (argc == 3) {
        profile_id1 = argv[1][0] - '0';
        profile_id2 = argv[2][0] - '0';
    }
    else {
        QL_USER_LOG("./quectel_test_wwan <profile_id>\n\r");
        return 0;
    }

    if (profile_id1 < 0 || profile_id1 > 4
        || profile_id1 == profile_id2 || profile_id2 > 4) {
        QL_USER_LOG("invalid proflile id");
        return 0;
    }

    QL_USER_LOG("<---Quectel Data wwanAPI Test--->\n\r");
    Ql_wwan_init();

    Ql_wwan_um_info_get(&um_info);

    call_ret1 = false;
    call_ret2 = false;

    /* active first APN */
    call_params.ip_type = QWWAN_IP_V4;
    call_params.profile_id = profile_id1;
    call_params.autoconnect = true;
    call_params.call_evt_cb = Ql_wwan_call_evt_cb;    
    call_ret1 = Ql_wwan_rm_up(&call_params);

    /* active second APN */
    if (profile_id2 >= 0) {
        call_params.ip_type = QWWAN_IP_V4;
        call_params.profile_id = profile_id2;
        call_params.call_evt_cb = Ql_wwan_call_evt_cb;    
        call_ret2 = Ql_wwan_rm_up(&call_params);
    }

    while (call_ret1 || call_ret2) {
        sleep(10);
    }
    
    return 0;
}