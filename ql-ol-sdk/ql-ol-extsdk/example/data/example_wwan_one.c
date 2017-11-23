#include <stdio.h>
#include <ql_data_wwan.h>

#define QL_USER_LOG(format, ...) fprintf(stderr, "[Ql_test][%d]:"format, __LINE__, ##__VA_ARGS__)

void Ql_wwan_call_update_netconfig(int profile_id)
{
    char ip_str[64];
    char command[200];
    Ql_wwan_rm_info_t rminfo;

    Ql_wwan_rm_info_get(profile_id, &rminfo);

    if (rminfo.rm_conf.v4_valid == 1) {
        QL_USER_LOG("ipv4addr: %s\n", inet_ntop(AF_INET,
            &rminfo.rm_conf.v4_conf.public_ip.s_addr, ip_str, sizeof(ip_str)));
    }

    inet_ntop(AF_INET, &rminfo.rm_conf.v4_conf.gw_addr, ip_str, sizeof(ip_str));
    printf("gw_addr: %s\n", ip_str);
    snprintf(command, sizeof(command), "ip route add default via %s dev %s", ip_str, rminfo.rm_name);
    system(command);

    inet_ntop(AF_INET, &rminfo.rm_conf.v4_conf.pri_dns_addr, ip_str, sizeof(ip_str));
    printf("primary_dnsv4: %s\n", ip_str);
    snprintf(command, sizeof(command), "echo 'nameserver %s' > /etc/resolv.conf", ip_str);
    system(command);

    inet_ntop(AF_INET, &rminfo.rm_conf.v4_conf.sec_dns_addr, ip_str, sizeof(ip_str));
    printf("secondary_dnsv4: %s\n", ip_str);
    snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf", ip_str);
    system(command);
}

void Ql_wwan_call_evt_cb (Ql_wwan_evt_t Ql_wwan_evt)
{
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
        QL_USER_LOG("User Got APN%d connected evt:\n\r", Ql_wwan_evt.profile_id);

        Ql_wwan_call_update_netconfig(Ql_wwan_evt.profile_id);
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
    bool call_ret1;
    int profile_id1;
    Ql_wwan_um_info_t um_info;
    Ql_wwan_data_call_params_t call_params;

    profile_id1 = 0;
    if (argc == 2) {
        profile_id1 = argv[1][0] - '0';
    }
    else {
        QL_USER_LOG("./quectel_test_wwan <profile_id>\n\r");
        return 0;
    }

    if (profile_id1 < 0 || profile_id1 > 4) {
        QL_USER_LOG("invalid proflile id");
        return 0;
    }

    QL_USER_LOG("<---Quectel Data wwanAPI Test--->\n\r");
    Ql_wwan_init();

    Ql_wwan_um_info_get(&um_info);

    call_ret1 = false;

    /* active first APN */
    call_params.ip_type = QWWAN_IP_V4;
    call_params.profile_id = profile_id1;
    call_params.autoconnect = true;
    call_params.call_evt_cb = Ql_wwan_call_evt_cb;    
    call_ret1 = Ql_wwan_rm_up(&call_params);

    while (call_ret1) {
        sleep(10);
    }
    
    return 0;
}