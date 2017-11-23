#include <ql_oe.h>

#define QUEC_AT_PORT    "/dev/smd8"
#define BUF_SIZE        128
#define SUPPORTED_APN_MAX           5

#define TEST_PORT                   5678    // the port users will be connecting to
#define SUPPORTED_CONNECTION_MAX    1       // how many pending connections queue will hold
 

typedef struct
{
    int  cmdIdx;
    char *funcName;
} st_api_test_case;

static sem_t binSem;
static int g_cur_apn_idx = 0;
static dsi_hndl_t g_h_handle[SUPPORTED_APN_MAX] = {0};
static int g_is_network_ready[SUPPORTED_APN_MAX]= {0};

//for server test
static int g_accept_fd[SUPPORTED_CONNECTION_MAX];         // accepted connection fd
static int g_cur_conn_cnt; 
static int g_is_server_daemon_running = 0; 

st_api_test_case at_api_testcases[] = 
{
    {0,    "QL_DEV_GetImei"},
    {1,    "QL_DEV_GetFirmwareVersion"},
    {2,    "QL_DEV_GetDeviceModel"},
    {3,    "QL_DEV_GetMeid"},
    {4,    "QL_DEV_GetEsn"},
    
    {5,    "QL_WWAN_Initialize"},
    {6,    "QL_WWAN_GetDataSrvHandle"},
    {7,    "QL_WWAN_StartDataCall"},
    {8,    "QL_WWAN_StopDataCall"},
    {9,    "test download using wget"},
    {10,    "QL_WWAN_SetProfileId, QL_WWAN_SetIPVersion"},
    {11,    "QL_WWAN_GetDeviceName"},
    {12,    "QL_WWAN_GetIPAddr"},
    {13,    "QL_WWAN_GetDataBearerTech"},
    {14,    "QL_WWAN_GetDataEndReason"},    
    {15,    "QL_WWAN_ReleaseDataSrvHandle"},
    {16,    "QL_WWAN_SetAPN"},
    {17,    "QL_WWAN_GetAPN"},
    {18,    "QL_WWAN_AddConnStateEventHandler"},
    
    {20,    "QL_NW_GetRegState"},
    {21,    "QL_NW_GetNetworkNameMccMnc"},
    {22,    "QL_NW_GetServingCell"},
    {23,    "QL_NW_AddRegStateEventHandler"},
    
    {30,    "QL_SIM_GetICCID"},
    {31,    "QL_SIM_GetIMSI"},
    {32,    "QL_SIM_VerifyPIN"},
    {33,    "QL_SIM_ChangePIN"},    
    {34,    "QL_SIM_Unlock"},
    {35,    "QL_SIM_Lock"},
    {36,    "QL_SIM_Unblock"},    
    {37,    "QL_SIM_GetPINTriesCnt"},
    {38,    "QL_SIM_GetState"},
    
    {40,    "QL_SMS_AddRxMessageHandler"},
    {41,    "QL_SMS_Create"},
    {42,    "QL_SMS_SetDestination"},
    {43,    "QL_SMS_SetTimeout"},
    {44,    "QL_SMS_SetText"},
    {45,    "QL_SMS_SetBinary"},
    {46,    "QL_SMS_SetUCS2"},
    {47,    "QL_SMS_SetPDU"},
    {48,    "QL_SMS_SendText"},    
    {49,    "QL_SMS_Encode"},
    {50,    "QL_SMS_Send"},
    {51,    "QL_SMS_Delete"},
    {52,    "QL_SMS_Decode"},    
    {53,    "QL_SMS_GetFormat"},
    {54,    "QL_SMS_GetType "},
    {55,    "QL_SMS_SetPreferredStorage"},
    
    {56,    "QL_SMS_GetSenderTel"},
    {57,    "QL_SMS_GetTimeStamp"},
    {58,    "QL_SMS_GetUserdataLen"},
    {59,    "QL_SMS_GetText"},
    {60,    "QL_SMS_GetBinary"},
    {61,    "QL_SMS_GetUCS2"},
    {62,    "QL_SMS_GetPDU"},
    {63,    "QL_SMS_GetPDULen"},
    {64,    "QL_SMS_DeleteFromStorage"},
    {65,    "QL_SMS_GetSmsCenterAddress"},
    {66,    "QL_SMS_SetSmsCenterAddress"},
    
    {70,    "QL_VCALL_AddStateHandler"},
    {71,    "QL_VCALL_RemoveStateHandler"},
    {72,    "QL_VCALL_Start"},
    {73,    "QL_VCALL_End"},
    {74,    "QL_VCALL_Answer"},
    {75,    "QL_VCALL_GetTerminationReason"},

    {80,    "set_selected_apn_idx"},
    {90,    "test_sleep_wakeup"},
    
    {97,    "testapi_client(work as client, send msg to server)"},
    {98,    "stop testapi_client testing"},
    
    {99,    "testapi_server(work as server, receive msg and ack msg+ack)"},
    {100,   "stop testapi_server testing"},

    
    {101,   "test_udp_server"},
    {102,   "stop test_udp_server testing"},
    
    {103,   "test_udp_client"},
    
    {104,   "testapi_client_send_ack"},
    
    {110,   "start_qmi_timer to send heartbeat"},
    {111,   "auto sleep onoff"},
    
    
    {112,   "start_qmi_timer to send sms"},

    {120,   "resolve private domain name"},
    
    {-1,    NULL}
};

void print_help(void)
{
    int i;

    printf("Supported test cases:\n");
    for(i = 0; ; i++)
    {
        if(at_api_testcases[i].cmdIdx == -1)
        {
            break;
        }
        printf("%d:\t%s\n", at_api_testcases[i].cmdIdx, at_api_testcases[i].funcName);
    }
}

int start_qmi_timer(ql_timer_exp_cb exp_cb_fcn, int timeout_sec, int repeat, void* param)
{
    int timerID = 0;  
    
    while(timerID < 10)
    {//get available timer id.
        if(QL_Timer_Register(timerID, exp_cb_fcn, param) == 0)
        {
            break;
        }
        timerID++;
    }
    if(timerID == 10)
    {
        printf("No available timer ID found\n");
        return -1;
    }
    QL_Timer_Start(timerID, timeout_sec*1000, repeat!=0);
    
    return timerID;
}

static void ql_sms_rxMsg_cb_func(ST_SMS_MsgRef msgRef, void* contextPtr)
{
    int             ret     = E_QL_OK;
    ST_SMS_MsgRef   h_msg; 
    char            SMSContent[QL_SMS_TEXT_MAX_LEN] = {0};
    
    if(msgRef->userdataLen != 0)
    {
        printf("#####  You've got one new message from %s, Content:%s #######\n", 
                msgRef->PhoneNum,
                msgRef->textData);
        snprintf(SMSContent, sizeof(SMSContent), "%s\n", msgRef->textData); 
    }
    else if(msgRef->pduLen != 0)
    {
        printf("#####  You've got one new message from %s, Content:%s #######\n", 
                msgRef->PhoneNum,
                msgRef->pduData);
        snprintf(SMSContent, sizeof(SMSContent), "%s\n", msgRef->pduData); 
    }
    
    printf("SMSContent=%s\n", SMSContent);
    h_msg = (ST_SMS_MsgRef)QL_SMS_SendText(msgRef->PhoneNum, 
                                            SMSContent,
                                            NULL,
                                            NULL);
    QL_SMS_Delete(h_msg);
}
    
static void ql_vcall_cb_func(ST_VCALL_CallRef    reference,
    const char* identifier,
                             E_QL_VCALL_STATE_T  event,
                             void*               contextPtr)
{
    ST_VCALL_CallRef *ph_call = (ST_VCALL_CallRef *)contextPtr;
    
    *ph_call = reference;
    printf("######### Call id=%d, Phone:%s, event=%d!  ######\n", reference, identifier, event);
}

static void ql_nw_regstate_cb_func( E_QL_WWAN_NET_REG_STATE_T   state,
                                    void                        *contextPtr)
{
    printf("######### Current network regstate changed to %d  ######\n", state);
}


static void ql_wwan_connstate_cb_func(int     profileIdx,
                                      bool    isConnected,
                                      void    *contextPtr)
{
    printf("######### APN%d %s !  ######\n", profileIdx, isConnected ? "Connected" : "DisConnected");
}


void nw_init_cb(void* cb_data)
{
    printf("\nnw_init_cb!\n");    
}

void dsi_net_evt_cb( dsi_hndl_t         hndl,         /**< Handle for which this event is associated. */
                    void              *user_data,    /**< Application-provided user data. */
                    dsi_net_evt_t      evt,          /**< Event identifier. */
                    dsi_evt_payload_t *payload_ptr   /**< Associated event information. */
                  )
{
    int apn_idx = (int)user_data;
    
    printf("\nAPN[%d] dsi_net_evt_cb, evt=%d!\n", apn_idx, evt);  
    switch(evt)
    {
        case DSI_EVT_NET_IS_CONN:
            printf("Call is connected!!\n"); 
            break;
        case DSI_EVT_NET_NO_NET: /**< Call is disconnected. */
            printf("Call is Disconnected!!\n"); 
            g_is_network_ready[apn_idx] = 0;
            break;
        case DSI_EVT_NET_NEWADDR:
            printf("New address is generated!!\n"); 
            break;
        case DSI_EVT_NET_DELADDR:
            printf("Old address deleted !!\n"); 
            break;
        case DSI_EVT_WDS_CONNECTED:
            printf("Network Ready!!\n"); 
            g_is_network_ready[apn_idx] = 1;
            break;
        default:
            break;
    }
}

#if 1 // test TCP long-connection and heartbeat.
static int    g_testclient_sockfd    = 0;
static int g_heartbeat_thread_run = 0;
static int g_client_send_ack_thread_run   = 0;
static pthread_t thrd_keep_alive= 0;
static pthread_t thrd_client_send_ack= 0;
static pthread_t thrd_server_daemon = 0;
static int  g_timer_id = -1; //use fixed timerID = 1, you can change it( 1 < timer_id < 10 ) 


int sms_timer_expire_cb(ql_timer_id timer_id, void *params)
{
    int             ret     = E_QL_OK;
    ST_SMS_MsgRef   h_msg; 
    char            SMSContent[128] = {0};
    struct timespec time1   = {0, 0};  

    clock_gettime(CLOCK_REALTIME, &time1);        
    snprintf(SMSContent, sizeof(SMSContent), "HI, Guys! Msg sent at tick=%d.%d\n", (int)time1.tv_sec, (int)time1.tv_nsec); 
    
    h_msg = (ST_SMS_MsgRef)QL_SMS_SendText((char*)params, 
                                            SMSContent,
                                            NULL,
                                            NULL);
    QL_SMS_Delete(h_msg);
}

int heart_beat_timer_expire_cb(ql_timer_id timer_id, void *params)
{
    printf("time_id = %d expired, send heartbeat\n", timer_id);     
    sem_post(&binSem);
    
    return 0;
}


int send_heartbeat_process(void *params)
{
    int     sockfd          = (int)params;
    int     len             = 0;
    char    heartbeat[64]   = {0};
    //char    rcv_buf[128]    = {0};
    struct timespec time1   = {0, 0};  
    
    g_heartbeat_thread_run = 1;
    
    printf("sockfd = %d !!!\n", sockfd);
    
    while(g_heartbeat_thread_run)
    {
        sem_wait(&binSem);

        clock_gettime(CLOCK_REALTIME, &time1);        
        snprintf(heartbeat, sizeof(heartbeat), "heart beat to server, tick=%d.%d\n", (int)time1.tv_sec, (int)time1.tv_nsec); 

        len = send(sockfd, heartbeat, strlen(heartbeat), 0);
        if (len < 0)
        {
            printf("Message '%s' failed to send ! "
                    "The error code is %d, error message '%s'\n",
                    heartbeat, errno, strerror(errno));
        }
        else
        {
            printf("News: %s\b >>> Send Done!!!\n", heartbeat);
        }
        // removed as this will cause SIGNAL 13
        //recv(sockfd, rcv_buf, sizeof(rcv_buf), 0);
        //printf("###News from server: %s\n", rcv_buf);
    }
    close(g_testclient_sockfd);
    return 0;
}

static void keep_alive_thread_init(int sockfd)
{
    sem_init(&binSem, 0, 0);
    if (pthread_create(&thrd_keep_alive, NULL, (void*)send_heartbeat_process, (void*)sockfd) != 0)
    {
        printf("Fail to create thread!\n");
        return;
    }
}

int client_send_ack_process(void *params)
{
    int     sockfd          = (int)params;
    int     len             = 0;
    char    ack_msg[64]     = {0};
    char    rcv_buf[128]    = {0};
    struct  timespec time = {0, 0};
    
    g_client_send_ack_thread_run = 1;
    
    printf("sockfd = %d !!!\n", sockfd);
    
    while(g_client_send_ack_thread_run)
    {
        len = recv(sockfd, rcv_buf, sizeof(rcv_buf), 0);
        if(len > 0)
        {
	    clock_gettime(CLOCK_REALTIME, &time);
	    printf("###News from server: %s\n", rcv_buf);
	    printf("####Client Got msg time: tick=%d.%d\n", (int)time.tv_sec, (int)time.tv_nsec);

	    snprintf(ack_msg, sizeof(ack_msg), "%s", rcv_buf);

            len = send(sockfd, ack_msg, strlen(ack_msg), 0);
            if (len < 0)
            {
                printf("Message '%s' failed to send ! "
                        "The error code is %d, error message '%s'\n",
                        ack_msg, errno, strerror(errno));
            }
            else
            {
                printf("News: %s\b >>> Send Done!!!\n", ack_msg);
            }
        }
        sleep(1);
    }
    close(g_testclient_sockfd);
    g_testclient_sockfd = 0;
    return 0;
}

static void client_send_ack_thread_init(int sockfd)
{
    if (pthread_create(&thrd_client_send_ack, NULL, (void*)client_send_ack_process, (void*)sockfd) != 0)
    {
        printf("Fail to create thread!\n");
        return;
    }
}

#define MAXBUF 1024

int testapi_client_send_ack(void)
{
    struct sockaddr_in dest;
    char ip_addr[32] = {0};
    int  port = 0;

    if ((g_testclient_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket");
        return -1;
    }    
    
    while(1)
    {
        printf("please input valid ip address: \n");
        scanf("%30s", ip_addr); // won't accept empty string
        
        if (inet_aton(ip_addr, (struct in_addr*)&dest.sin_addr.s_addr) == 0)
        {
            perror(ip_addr);
        }
        else
        {
            break;
        }
    }
        
    printf("please input port number: ");
    scanf("%d", &port);

    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    if (connect(g_testclient_sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0)
    {
        perror("Connect");
        return -1;
    }
    printf("Connect OK, sockfd=%d\n", g_testclient_sockfd);
    
    client_send_ack_thread_init(g_testclient_sockfd);
    return 0;
}

int testapi_client(void)
{
    struct sockaddr_in dest;
    char ip_addr[64] = {0};
    int  port = 0;

    bzero(&dest, sizeof(dest));
    if ((g_testclient_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket");
        return -1;
    }    
    
    while(1)
    {
        printf("please input valid ip address: \n");
        scanf("%40s", ip_addr); // won't accept empty string
        
        if (inet_aton(ip_addr, (struct in_addr*)&dest.sin_addr.s_addr) == 0)
        {
            perror(ip_addr);
        }
        else
        {
            break;
        }
    }

    printf("please input port number: ");
    scanf("%d", &port);

    
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    //memset(&(dest.sin_zero), 0, 8);

    if (connect(g_testclient_sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0)
    {
        perror("Connect");
        return -1;
    }
    printf("Connect OK, sockfd=%d\n", g_testclient_sockfd);
    
#if 1 //use qmi_timer to send heartbeat

    /* support user to input heartbeat time interval by gale 2017/08/16 */
    int time_val;
    printf("please input heartbeat time interval(s): ");
    scanf("%d", &time_val);

    if(-1 == start_qmi_timer(heart_beat_timer_expire_cb, time_val, 1, (void *)g_testclient_sockfd))
    {
        return -1;
    }
    keep_alive_thread_init(g_testclient_sockfd);
#else //use system select
    struct timeval tv;
    int retval, maxfd = -1, len;
    char buffer[MAXBUF];
    fd_set rfds;
    char heartbeat[64] = "hello server";
    
    printf("\nReady to start chatting.\n"
            "Direct input messages and \n"
            "enter to send messages to the server\n");

    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        maxfd = 0;

        FD_SET(sockfd, &rfds);
        if (sockfd > maxfd)
        maxfd = sockfd;

        tv.tv_sec = 10;
        tv.tv_usec = 0;

        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
        {
            printf("Will exit and the select is error! %s", strerror(errno));
            break;
        }
        else if (retval == 0) //"No message comes, no buttons, continue to wait ...\n"
        {
            struct timespec time1 = {0, 0};   
              
            clock_gettime(CLOCK_REALTIME, &time1);        
            snprintf(heartbeat, sizeof(heartbeat), "heart beat to server, tick=%d.%d\n", (int)time1.tv_sec, (int)time1.tv_nsec); 
    
            len = send(sockfd, heartbeat, strlen(heartbeat), 0);
            if (len < 0)
            {
                printf("Message '%s' failed to send ! "
                "The error code is %d, error message '%s'\n",
                heartbeat, errno, strerror(errno));
                break;
            }
            else
            {
                printf("News: %s\b >>> Send Done!!!\n", heartbeat);
            }
            continue;
        }
        else
        {
            if (FD_ISSET(sockfd, &rfds))
            {
                bzero(buffer, MAXBUF+1);
                len = recv(sockfd, buffer, MAXBUF, 0);

                if (len > 0)
                {
                    printf("Successfully received the message: '%s',%d bytes of data\n",
                    buffer, len);
                }
                else
                {
                    if (len < 0)
                    {
                        printf("Failed to receive the message! "
                                "The error code is %d, error message is '%s'\n",
                                errno, strerror(errno));
                    }
                    else
                    {
                        printf("Chat to terminate!\n");
                    }
                    break;
                }
            }
            
            if (FD_ISSET(0, &rfds))
            {
                bzero(buffer, MAXBUF+1);
                fgets(buffer, MAXBUF, stdin);

                if (!strncasecmp(buffer, "quit", 4))
                {
                    printf("Own request to terminate the chat!\n");
                    break;
                }

                len = send(sockfd, buffer, strlen(buffer)-1, 0);
                if (len < 0)
                {
                    printf( "Message '%s' failed to send ! "
                            "The error code is %d, error message '%s'\n",
                            buffer, errno, strerror(errno));
                    break;
                }
                else
                {
                    printf("News: %s\b >>> Send Done! \n", buffer);
                }
            }
        }
    }
    close(sockfd);
#endif
    return 0;
}


void showclient()
{
    int i;
    printf("client amount: %d\n", g_cur_conn_cnt);
    for (i = 0; i < SUPPORTED_CONNECTION_MAX; i++) 
    {
        printf("[%d]:%d  ", i, g_accept_fd[i]);
    }
    printf("\n\n");
}

void server_send_ack(int fd)
{
    char    ack_msg[128]   = {0};
    struct timespec time1   = {0, 0};  
    
    clock_gettime(CLOCK_REALTIME, &time1);  
    
    snprintf(ack_msg, sizeof(ack_msg), "ack from server, tick=%d.%d\n", (int)time1.tv_sec, (int)time1.tv_nsec); 

    send(fd, ack_msg, strlen(ack_msg), 0);
}

static void test_server_daemon(int sockfd)
{
    int             i;
    int             ret;
    int             yes = 1;
    char            buf[MAXBUF] = {0};
    socklen_t       sin_size;
    int             sock_fd, new_fd;    // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;     // server address information
    struct sockaddr_in client_addr;     // connector's address information
    struct timespec time = {0, 0};      // record time get msg  by gale 2017/08/16

    fd_set          fdsr;
    int             maxsock;
    struct timeval  tv;
    
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("socket");
        return ;
    }
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
    {
        perror("setsockopt");
        return ;
    }
        
    server_addr.sin_family = AF_INET;           // host byte order
    server_addr.sin_port = htons(TEST_PORT);    // short, network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY;   // automatically fill with my IP
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
    {
        perror("bind");
        return ;
    }
    if (listen(sock_fd, SUPPORTED_CONNECTION_MAX) == -1) 
    {
        perror("listen");
        return ;
    }
    printf("\nlisten port %d start!\n", TEST_PORT);
    
    g_cur_conn_cnt = 0;
    sin_size = sizeof(client_addr);
    maxsock = sock_fd;
    while (g_is_server_daemon_running) 
    {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(sock_fd, &fdsr);  // add fd
        // timeout setting
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        // add active connection to fd set
        for (i = 0; i < SUPPORTED_CONNECTION_MAX; i++) 
        {
            if (g_accept_fd[i] != 0) 
            {
                FD_SET(g_accept_fd[i], &fdsr);
            }
        }
        ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
        if (ret < 0) 
        {          // error
            perror("select");
            //break;
        } 
        else if (ret == 0) 
        {  // time out
            printf("timeout\n");
            continue;
        }
        // check every fd in the set
        for (i = 0; i < g_cur_conn_cnt; i++) 
        {
            if (FD_ISSET(g_accept_fd[i], &fdsr)) // check which fd is ready
            {
                ret = recv(g_accept_fd[i], buf, sizeof(buf), 0);
                if (ret <= 0) 
                {        // client close
                    printf("ret : %d and client[%d] close\n", ret, i);
                    close(g_accept_fd[i]);
                    FD_CLR(g_accept_fd[i], &fdsr);  // delete fd 
                    g_accept_fd[i] = 0;
                    g_cur_conn_cnt--;
                }
                else 
                {   // receive data
                    if (ret < MAXBUF)
                    {
                        memset(&buf[ret], '\0', 1); // add NULL('\0')
                    }
		    //record the time when server get msg by gale 2017/08/16
		    clock_gettime(CLOCK_REALTIME, &time);
		    printf("####Server Got msg time: tick=%d.%d\n", (int)time.tv_sec, (int)time.tv_nsec);

                    printf("####Server Got: Client[%d] Msg:%s\n", i, buf);
                    server_send_ack(g_accept_fd[i]);
                }
            }
        }
        // check whether a new connection comes
        if (FD_ISSET(sock_fd, &fdsr))  // accept new connection 
        {
            new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
            if (new_fd <= 0) 
            {
                perror("accept");
                continue;
            }
            // add to fd queue
            if (g_cur_conn_cnt < SUPPORTED_CONNECTION_MAX) 
            {
                char *welcome_msg = "Hello, I'm server!";
                g_accept_fd[g_cur_conn_cnt++] = new_fd;
                printf("new connection client[%d] %s:%d\n", g_cur_conn_cnt,
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                if (new_fd > maxsock)  // update the maxsock fd for select function
                {
                    maxsock = new_fd;
                }
                send(new_fd, welcome_msg, strlen(welcome_msg)+1, 0);
            }
            else 
            {
                printf("max connections arrive, exit\n");
                send(new_fd, "bye", 4, 0);
                close(new_fd);
                //break;   
            }
        }
        showclient();
    }
    // close other connections
    for (i = 0; i < SUPPORTED_CONNECTION_MAX; i++) 
    {
        if (g_accept_fd[i] != 0) 
        {
            close(g_accept_fd[i]);
        }
    }
    return ;
}


static void test_server_daemon_thread_init(int sockfd)
{
    g_is_server_daemon_running = 1;
    if (pthread_create(&thrd_server_daemon, NULL, (void*)test_server_daemon, (void*)sockfd) != 0)
    {
        printf("Fail to create thread!\n");
        return;
    }
}
#endif

#if 1 //UDP test client/server

static int g_is_udp_server_running = 0; 
void *thread_udp_server_recv(void *arg)
{    
    int recvLen = 0; 
    int fd = (int)arg;
    struct sockaddr_in from;
    uint32_t fromlen = sizeof(from);
    unsigned char recvDataBuff[1024] = {0}; 

    printf("udp server thread begin!\r\n");

    while(g_is_udp_server_running)    
    {
        recvLen = recvfrom(fd, recvDataBuff, 1024, 0, (struct sockaddr *)&from, &fromlen);
        if(-1 == recvLen)        
        {
            perror("recvfrom");
            break;
        }

        if(recvLen > 0)
        {
            char    ack_msg[128]   = {0};
            struct timespec time1   = {0, 0};  
            
            printf("###UDP server recv %d byte from %s:%d, Content:%s\n", 
                recvLen, inet_ntoa(from.sin_addr), ntohs(from.sin_port), recvDataBuff);            
            
            clock_gettime(CLOCK_REALTIME, &time1);  
            
            snprintf(ack_msg, sizeof(ack_msg), "this is ack from udp server, tick=%d.%d\n", (int)time1.tv_sec, (int)time1.tv_nsec); 

            sendto(fd, ack_msg, sizeof(ack_msg), 0, (struct sockaddr *)&from, sizeof(from)); 
        }
        sleep(1);
    }
    close(fd);

    return NULL;
}

int test_udp_server(void)
{
    int sockfd = 0;    
    struct sockaddr_in local_addr;
    pthread_t tid_udp_recv;

    //create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == sockfd)
    {
        perror("socket");
        return -1;
    }

    //bind server port
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(6789);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&local_addr.sin_zero, 8);
    if(-1 == bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)))
    {
        perror("bind");
    close(sockfd);
        return -1;
    }
    g_is_udp_server_running = 1;
    //start receive thread
    pthread_create(&tid_udp_recv, NULL, thread_udp_server_recv, (void*)sockfd);

    return 0;
}

int test_udp_client(void)  
{  
    int udp_sock_fd         = 0;   
    char buff[128]          = {0};  
    char ip_addr[32]        = {0};
    struct sockaddr_in addr;  
    struct timespec time1   = {0, 0};  
  
    printf("This is a UDP client\n"); 
  
    if ( (udp_sock_fd=socket(AF_INET, SOCK_DGRAM, 0)) <0)  
    {  
        perror("socket");  
        return -1;  
    }  
    
    
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(6789);  
    while(1)
    {
        printf("please input server ip address: \n");
        scanf("%30s", ip_addr); // won't accept empty string
        
        addr.sin_addr.s_addr = inet_addr(ip_addr);  
        if (addr.sin_addr.s_addr == INADDR_NONE)  
        {  
            continue;  
        }
        break;
    }
  
    while (1)  
    {  
        clock_gettime(CLOCK_REALTIME, &time1);  
        
        snprintf(buff, sizeof(buff), "This is udp data from client, tick=%d.%d\n", (int)time1.tv_sec, (int)time1.tv_nsec); 
    
        if( sendto(udp_sock_fd, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {  
            perror("sendto");  
            break;  
        }
        else
        {  
            printf("client send <<<%s>>> success!\n", buff);  
        }
        break;//only send once to avoid block main
    }  
    close(udp_sock_fd);  
      
    return 0;  
}  

#endif


void force_sleep(void)
{
    /* add by gale 2017/08/16 */
    printf("\t######### Module enter sleep #######\n");
    system("echo 0 > /sys/devices/78b1000.uart/clock");
    system("echo -e 'at+qsclk=1\r\n' > /dev/smd8");
}

static void handle_sig(int sig)
{
    printf("######## got signal %d #######\n", sig);
}

void nslookup()
{
    
    char hostname[64];
    char dns_server_ip[20];
    char type;
    
    QUERY_IP_TYPE ip_type;
    hostaddr_info_u resolved_addr;
    int i;
    char ip_str[128];

    //get hostname
    printf("input private domain: ");
    scanf("%s", hostname);
    //Get the DNS servers
    printf("input private network dns ip: ");
    scanf("%s", dns_server_ip);
    //expect ip type
    printf("input ip type(4 or 6): ");
    getchar();
    scanf("%c", &type);

    if (type == '6') {
        ip_type = QUERY_IPV6_E;
    }
    else {
        ip_type = QUERY_IPV4_E;
    }
    
    //resolving...
    QL_nslookup(hostname, dns_server_ip, ip_type, &resolved_addr);
    
    //printf resolved addr
    for (i = 0; i < resolved_addr.addr_cnt; i++) {
         inet_ntop(AF_INET, &resolved_addr.addr[i].s_addr, ip_str, sizeof(ip_str));
         printf("%s has IPv4 address : %s\n\r", hostname, ip_str);
    }
    
    for (i = 0; i < resolved_addr.addr6_cnt; i++) {
         inet_ntop(AF_INET6, &resolved_addr.addr6[i].s6_addr, ip_str, sizeof(ip_str));
         printf("%s has IPv6 address : %s\n\r", hostname, ip_str);
    }

}

int main(int argc, char *argv[])
{
    int    cmdIdx  = 0;
    int    value   = 0;
    int    value1  = 0;
    int    ret     = E_QL_OK;
    char   buf[BUF_SIZE] = {0};
    char   buf1[BUF_SIZE]= {0};
    char   phoneNum[32] = {0};
    QL_SMS_RxMessageHandlerRef_t    h_rxMsgHandle;
    QL_VCALL_StateHandlerRef_t  h_voicecall;    
    ST_VCALL_CallRef            h_call;
    ST_SMS_MsgRef                   h_msg;    
    
    signal(SIGPIPE, handle_sig);
    
    print_help();

    while(1)
    {
        printf("please input cmd index(-1 exit): ");
        scanf("%d", &cmdIdx);
        if(cmdIdx == -1)
        {
            int i = 0;
            for(i=0; i<SUPPORTED_APN_MAX; i++)
            {
                if(g_h_handle[i] != NULL)
                {
                    QL_WWAN_StopDataCall(g_h_handle[i]);
                    while(g_is_network_ready[g_cur_apn_idx])
                    {
                        printf("Wait for network disconnect!\n");
                        sleep(1);
                    }
                    QL_WWAN_ReleaseDataSrvHandle(g_h_handle[i]);
                    //QL_WWAN_Deinitialize();
                    g_h_handle[i] = NULL;
                }
            }
            if(g_heartbeat_thread_run)
            {
                g_heartbeat_thread_run = 0;
                if(g_testclient_sockfd != 0)
                {
                    close(g_testclient_sockfd);
                }
            }
            if(g_timer_id != -1)
            {
                ret = QL_Timer_Stop(g_timer_id);
                printf("QL_Timer_Stop(%d) ret %d\n", g_timer_id, ret);
            }
            break;
        }
        memset(buf, 0, sizeof(buf));
        switch(cmdIdx)
        {
        case 0://"QL_DEV_GetImei"
            ret = QL_DEV_GetImei(buf, BUF_SIZE);
            printf("ret = %d, IMEI: %s\n", ret, buf);
            break;
        case 1://"QL_DEV_GetFirmwareVersion"
            ret = QL_DEV_GetFirmwareVersion(buf, BUF_SIZE);
            printf("ret = %d, FW Version: %s\n", ret, buf);
            break;
        case 2://"QL_DEV_GetDeviceModel"
            ret = QL_DEV_GetDeviceModel(buf, BUF_SIZE);
            printf("ret = %d, Dev Model: %s\n", ret, buf);
            break;
        case 3://"QL_DEV_GetMeid"
            ret = QL_DEV_GetMeid(buf, BUF_SIZE);
            printf("ret = %d, MEID: %s\n", ret, buf);
            break;
        case 4://"QL_DEV_GetEsn"
            ret = QL_DEV_GetEsn(buf, BUF_SIZE);
            printf("ret = %d, ESN: %s\n", ret, buf);
            break;
            
        case 5://"QL_WWAN_Initialize"
        {
            ret = QL_WWAN_Initialize(nw_init_cb, NULL);
            printf("QL_WWAN_Initialize ret=%d!\n", ret);
            break;
        }
        case 6://"QL_WWAN_GetDataSrvHandle"
        {
            g_h_handle[g_cur_apn_idx] = QL_WWAN_GetDataSrvHandle(dsi_net_evt_cb, (void*)g_cur_apn_idx);
            printf("QL_WWAN_GetDataSrvHandle g_h_handle=%d!\n", (int)g_h_handle[g_cur_apn_idx]);
            break;
        }
        case 7://"QL_WWAN_StartDataCall"
        {
            char    command[BUF_SIZE]= {0};
            char    ip_str[64]  = {0};
            int     valid_ip_cnt= 0;
            dsi_addr_info_t info;
	    static unsigned short route_flag = 0;	// default route flag
            
            ret = QL_WWAN_StartDataCall(g_h_handle[g_cur_apn_idx]);
            printf("QL_WWAN_StartDataCall ret=%d!\n", ret);
            while(g_is_network_ready[g_cur_apn_idx] == 0)
            {
                printf("Wait for network ready!\n");
                sleep(1);
            }
            
            ret = QL_WWAN_GetIPAddr(g_h_handle[g_cur_apn_idx], &info, &valid_ip_cnt);
            if(info.iface_addr_s.valid_addr == 0)
            {
                printf("QL_WWAN_GetIPAddr ret = %d, NO valid addr!!\n", ret);
                break;
            }            
            
            inet_ntop(AF_INET, info.iface_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            printf("QL_WWAN_GetIPAddr ret = %d, valid_ip_cnt=%d \nInterface IP address =%s\n",  ret, valid_ip_cnt, ip_str);
            
            ret = QL_WWAN_GetDeviceName(g_h_handle[g_cur_apn_idx], buf, sizeof(buf));
            printf("QL_WWAN_GetDeviceName ret = %d, name=%s\n", ret, buf);

            snprintf(command, sizeof(command), "ifconfig %s %s netmask 255.0.0.0",buf, ip_str);
            system(command);
           
	    /* remove the default route rule, otherwise we cannot add the new one, by gale */
	    if(route_flag)
	    {
		memset(command, 0, sizeof(command));
		snprintf(command, sizeof(command), "ip route del default");
		system(command);
	    }
            inet_ntop(AF_INET, &info.gtwy_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            snprintf(command, sizeof(command), "ip route add default via %s dev %s", ip_str, buf);
            system(command);
	    route_flag = 1;	//Mark: there is already a defaule route rule
            printf("Gateway IP address   =%s\n", ip_str);

            inet_ntop(AF_INET, &info.dnsp_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            snprintf(command, sizeof(command), "echo 'nameserver %s' > /etc/resolv.conf", ip_str);
            system(command);
            printf("Primary DNS address  =%s\n", ip_str);
            
            inet_ntop(AF_INET, &info.dnss_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf", ip_str);
            system(command);
            printf("Secondary DNS address=%s\n\n", ip_str);            
            break;  
        }
        case 8://"QL_WWAN_StopDataCall"
            ret = QL_WWAN_StopDataCall(g_h_handle[g_cur_apn_idx]);
            printf("QL_WWAN_StopDataCall ret=%d!\n", ret);
            break;
#if 0
        case 9://"test download using wget"
        {
            printf("Start download \"http://img.bimg.126.net/photo/U4wG4x8qAaK0Qwc_aSWE2Q==/2002131509343880029.jpg\" !!!\n");
            system("/bin/wget --no-check-certificate -c -O download.jpg \"http://img.bimg.126.net/photo/U4wG4x8qAaK0Qwc_aSWE2Q==/2002131509343880029.jpg\" 2>&1 | tee wget.log");
            //system("/bin/wget --no-check-certificate -c -O download.jpg \"http://220.180.239.212:8399/hfs.ini\"");
            if(access("download.jpg", 0) == 0)
            {
                printf("Download finished successfully!!!\n");
            }
            else
            {
                printf("Download Failed !!!\n");
            }
            break;
        }
#else
        case 9://"test network available"
        {
            char    command[BUF_SIZE]= {0};
            char    ip_str[64]  = {0};
            int     valid_ip_cnt= 0;
            dsi_addr_info_t info;
            
            ret = QL_WWAN_GetIPAddr(g_h_handle[g_cur_apn_idx], &info, &valid_ip_cnt);
            if(info.iface_addr_s.valid_addr == 0)
            {
                printf("QL_WWAN_GetIPAddr ret = %d, NO valid addr!!\n", ret);
                break;
            }            
            
            inet_ntop(AF_INET, info.iface_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            
            snprintf(command, BUF_SIZE, "ping -c 10 -I %s www.baidu.com", ip_str);
            system(command);
            break;
        }
#endif
        case 10://"QL_WWAN_SetIPVersion"
            ret = QL_WWAN_SetProfileId(g_h_handle[g_cur_apn_idx], g_cur_apn_idx+1); //Use path 1, you can change it to others
            printf("QL_WWAN_SetProfileId ret = %d\n", ret);
            ret = QL_WWAN_SetIPVersion(g_h_handle[g_cur_apn_idx], 4);
            printf("QL_WWAN_SetIPVersion ret = %d\n", ret);
            break;
        case 11://"QL_WWAN_GetDeviceName"
            ret = QL_WWAN_GetDeviceName(g_h_handle[g_cur_apn_idx], buf, BUF_LEN);
            printf("QL_WWAN_GetDeviceName ret = %d, name=%s\n", ret, buf);
            break;
        case 12://"QL_WWAN_GetIPAddr"
        {
            char ip_str[64] = {0};
            dsi_addr_info_t info;
            int valid_ip_cnt = 0;
            
            ret = QL_WWAN_GetIPAddr(g_h_handle[g_cur_apn_idx], &info, &valid_ip_cnt);
            if(info.iface_addr_s.valid_addr == 0)
            {
                printf("QL_WWAN_GetIPAddr ret = %d, NO valid addr!!\n", ret);
                break;
            }            
            
            inet_ntop(AF_INET, info.iface_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            printf("QL_WWAN_GetIPAddr ret = %d, valid_ip_cnt=%d \nInterface IP address =%s\n", 
                            ret, valid_ip_cnt, ip_str);
            
            inet_ntop(AF_INET, &info.gtwy_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            printf("Gateway IP address   =%s\n", ip_str);

            inet_ntop(AF_INET, &info.dnsp_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            printf("Primary DNS address  =%s\n", ip_str);
            
            inet_ntop(AF_INET, &info.dnss_addr_s.addr.__ss_padding, ip_str, sizeof(ip_str));
            printf("Secondary DNS address=%s\n\n", ip_str);            
            break;  
        }
        case 13://"QL_WWAN_GetDataBearerTech"
            ret = (int)QL_WWAN_GetDataBearerTech(g_h_handle[g_cur_apn_idx]);
            printf("QL_WWAN_GetDataBearerTech ret = %d\n", ret);
            break;
        case 14://"QL_WWAN_GetDataEndReason"
        {
            dsi_ce_reason_t ce_reason;
            ret = (int)QL_WWAN_GetDataEndReason(g_h_handle[g_cur_apn_idx], &ce_reason, DSI_IP_FAMILY_V4);
            printf("QL_WWAN_GetDataEndReason ret = %d, reason.type=%d, reason.code=%d\n", 
                    ret, ce_reason.reason_type, ce_reason.reason_code);
            break;
        }
        case 15://"QL_WWAN_ReleaseDataSrvHandle"
        {
            QL_WWAN_ReleaseDataSrvHandle(g_h_handle[g_cur_apn_idx]);
            g_h_handle[g_cur_apn_idx] = NULL;
            printf("QL_WWAN_ReleaseDataSrvHandle return!\n");
            break;
        }
        case 16://"QL_WWAN_SetAPN"
        {
            ret = QL_WWAN_SetAPN(g_h_handle[g_cur_apn_idx], 1, "qldsiCfgAPN", "testuser", "testpwd");
            printf("QL_WWAN_SetAPN ret = %d\n", ret);
            break;
        }
        case 17://"QL_WWAN_GetAPN"
        {
            char   pwd[BUF_SIZE]= {0};
            ret = QL_WWAN_GetAPN(g_h_handle[g_cur_apn_idx], 1, buf, BUF_SIZE, buf1, BUF_SIZE, pwd, BUF_SIZE);
            printf("ret = %d, APN: %s, user:%s, pwd:%s\n", ret, buf, buf1, pwd);
            break;
        }
        case 18://"QL_WWAN_AddConnStateEventHandler"
        {
            ret = QL_WWAN_AddConnStateEventHandler(ql_wwan_connstate_cb_func, NULL);
            printf("QL_WWAN_AddConnStateEventHandler ret = %d\n", ret);
            break;
        }
        
        
        case 20://"QL_NW_GetRegState"
        {
            E_QL_NW_RADIO_ACCESS_TYPE_T rat;
            E_QL_WWAN_NET_REG_STATE_T state;
            ret = QL_NW_GetRegState(&rat, &state, &value, &value1);
            printf("ret = %d, rat=%d, NetRegState= %d, rssi= %d, ber=%d\n", ret, rat, state, value, value1);
            break;
        }
        case 21://QL_NW_GetNetworkNameMccMnc
        {
            char   mncStr[BUF_SIZE]= {0};
            ret = QL_NW_GetNetworkNameMccMnc(buf, BUF_SIZE, buf1, BUF_SIZE, mncStr, BUF_SIZE);
            printf("ret = %d, name: %s, mcc:%s, mnc:%s\n", ret, buf, buf1, mncStr);
            break;            
        }
        case 22://"QL_NW_GetServingCell"
        {
            ST_CellInfo  info;
            ret = QL_NW_GetServingCell(&info);
            printf("ServingCell rat=%s, mcc=%d, mnc=%d, lac = 0x%X, ci=0x%x\n", info.cellInfo[0].rat, info.cellInfo[0].mcc, 
                info.cellInfo[0].mnc, info.cellInfo[0].lac, info.cellInfo[0].cid);
            break;
        }
        case 23://"QL_NW_AddRegStateEventHandler"
            ret = QL_NW_AddRegStateEventHandler(ql_nw_regstate_cb_func, (void*)NULL);
            printf("QL_NW_AddRegStateEventHandler ret %d\n", ret);
            break;
        
        
        case 30://"QL_SIM_GetICCID"
            ret = QL_SIM_GetICCID(E_QL_SIM_EXTERNAL_SLOT_1, buf, BUF_SIZE);
            printf("ret = %d, ICCID: %s\n", ret, buf);
            break;
        case 31://"QL_SIM_GetIMSI"
            ret = QL_SIM_GetIMSI(E_QL_SIM_EXTERNAL_SLOT_1, buf, BUF_SIZE);
            printf("ret = %d, IMSI: %s\n", ret, buf);
            break;
        case 32://"QL_SIM_VerifyPIN"
        {
            char pin[8] = {0};
            printf("please input pin: \n");
            scanf("%s", pin);
            ret = QL_SIM_VerifyPIN(E_QL_SIM_EXTERNAL_SLOT_1, pin);
            printf("QL_SIM_VerifyPIN ret = %d\n", ret);
            break;
        }
        case 33://"QL_SIM_ChangePIN"
        {
            char old_pin[8] = {0};
            char new_pin[8] = {0};
            
            printf("please input OLD pin: \n");
            scanf("%s", old_pin);
            
            printf("please input NEW pin: \n");
            scanf("%s", new_pin);
            
            ret = QL_SIM_ChangePIN(E_QL_SIM_EXTERNAL_SLOT_1, old_pin, new_pin);
            printf("QL_SIM_ChangePIN ret = %d\n", ret);
            break;
        }
        case 34://"QL_SIM_Unlock"
        {
            char pin[8] = {0};
            
            printf("please input pin: \n");
            scanf("%s", pin);
            
            ret = QL_SIM_Unlock(E_QL_SIM_EXTERNAL_SLOT_1, pin);
            printf("QL_SIM_Unlock ret = %d\n", ret);
            break;
        }
        case 35://"QL_SIM_Lock"
        {
            char pin[8] = {0};
            
            printf("please input pin: \n");
            scanf("%s", pin);
            
            ret = QL_SIM_Lock(E_QL_SIM_EXTERNAL_SLOT_1, pin);
            printf("QL_SIM_Lock ret = %d\n", ret);
            break;
        }
        case 36://"QL_SIM_Unblock"
        {
            char puk[8] = {0};
            char new_pin[8] = {0};
            
            printf("please input puk: \n");
            scanf("%s", puk);
            
            printf("please input NEW pin: \n");
            scanf("%s", new_pin);
            
            ret = QL_SIM_Unblock(E_QL_SIM_EXTERNAL_SLOT_1, puk, new_pin);
            printf("QL_SIM_Unblock ret = %d\n", ret);
            break;
        }        
        case 37://"QL_SIM_GetPINTriesCnt"
            ret = QL_SIM_GetPINTriesCnt(E_QL_SIM_EXTERNAL_SLOT_1);
            printf("QL_SIM_GetPINTriesCnt = %d\n", ret);
            break;
        case 38://"QL_SIM_GetState"
            ret = QL_SIM_GetState(E_QL_SIM_EXTERNAL_SLOT_1);
            printf("QL_SIM_GetState = %d\n", ret);
            break;
            
        case 40://"QL_SMS_AddRxMessageHandler"
            h_rxMsgHandle = QL_SMS_AddRxMessageHandler(ql_sms_rxMsg_cb_func, (void*)h_msg);
            printf("h_rxMsgHandle = %d\n", h_rxMsgHandle);
            break;
        case 41://"QL_SMS_Create"
            h_msg = QL_SMS_Create();
            printf("QL_SMS_Create  h_msg= %ld\n", (int)h_msg);
            break;
        case 42://"QL_SMS_SetDestination"
        {
            char DestPhone[QL_SMS_PHONE_NUM_MAX_LEN] = {0};
            printf("please input dest phone number: \n");
            scanf("%s", DestPhone);
            ret = QL_SMS_SetDestination(h_msg, DestPhone);
            printf("QL_SMS_SetDestination ret= %d\n", ret);
        }
            break;
        case 43://"QL_SMS_SetTimeout"
            ret = QL_SMS_SetTimeout(h_msg, 3600);
            printf("QL_SMS_SetTimeout ret= %d\n", ret);
            break;
        case 44://"QL_SMS_SetText"
        {
            char SMSContent[200] = {0};
            printf("please input sms content: \n");
            //scanf("%s", SMSContent);
            getchar();//Add this on purpose or the following fgets will be skipped.
            fgets(SMSContent, QL_SMS_TEXT_MAX_LEN, stdin);
            printf("SMSContent:%s\n", SMSContent);

            ret = QL_SMS_SetText(h_msg, SMSContent);
            printf("QL_SMS_SetText ret= %d\n", ret);
        }
            break;
        case 45://"QL_SMS_SetBinary"
        {
            char binData[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            ret = QL_SMS_SetBinary(h_msg, binData, 16);
            printf("QL_SMS_SetBinary ret= %d\n", ret);
            break;
        }
        case 46://"QL_SMS_SetUCS2"
        {
            uint16_t binData[16] = {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
            ret = QL_SMS_SetUCS2(h_msg, binData, 16);
            printf("QL_SMS_SetUCS2 ret= %d\n", ret);
            break;
        }
        case 47://"QL_SMS_SetPDU"
        {
            char pdu_raw[QL_SMS_PDU_MAX_BYTES*2] = {0};
            printf("please input PDU data: \n");
            scanf("%s", pdu_raw);
            ret = QL_SMS_SetPDU(h_msg, pdu_raw, strlen(pdu_raw));
            printf("QL_SMS_SetPDU ret= %d\n", ret);
            break;
        }
        case 48://"QL_SMS_SendText"
        {
            ST_SMS_MsgRef msgRef;
            char    PhoneNum[QL_SMS_PHONE_NUM_MAX_LEN]                  = {0}; 
            char    textData[QL_SMS_TEXT_MAX_BYTES]                     = {0};

            printf("please input dest phone number: \n");
            scanf("%s", PhoneNum);

            printf("please input message content: \n");
            getchar();//Add this on purpose or the following fgets will be skipped.
            fgets(textData, QL_SMS_TEXT_MAX_LEN, stdin);
            printf("textData:%s\n", textData);
            
            msgRef = (ST_SMS_MsgRef)QL_SMS_SendText(PhoneNum, 
                                                      textData,
                                                      NULL,
                                                      NULL);
            printf("QL_SMS_SendText msgRef= %d\n", msgRef);

            QL_SMS_Delete(msgRef);
            break;
        }
        case 49://"QL_SMS_Encode"    
            ret = QL_SMS_Encode(h_msg);
            printf("QL_SMS_Encode ret= %d\n", ret);
            break;
        case 50://"QL_SMS_Send"
            ret = QL_SMS_Send(h_msg);
            printf("QL_SMS_Send ret= %d\n", ret);
            break;
        case 51://"QL_SMS_Delete"
            QL_SMS_Delete(h_msg);
            h_msg = NULL;
            printf("QL_SMS_Delete OK\n");
            break;
        case 52://"QL_SMS_Decode"
        {
            char pdu_data[512] = {0};
            if(h_msg == NULL)
            {
                printf("please run QL_SMS_Create first! \n");
                break;
            }
            printf("please input pdu content= \n");
            scanf("%s", pdu_data);
            ret = QL_SMS_Decode(pdu_data, h_msg);
            break;
        }
        case 53://"QL_SMS_GetFormat"
        {
            E_QL_SMS_FORMAT_T format;
            format = QL_SMS_GetFormat(h_msg);
            printf("smsFormat = %d\n", format);
            break;
        }
        case 54://"QL_SMS_GetType"
        {
            E_QL_SMS_TYPE_T type;
            type = QL_SMS_GetType(h_msg);
            printf("smsType = %d\n", type);
            break;
        }
        case 55://"QL_SMS_SetPreferredStorage"
        {
            int type;
            E_QL_SMS_STORAGE_TYPE_T e_storage_type;
            
            printf("Please input preferred storage type(0:NV, 1:SIM):\n");
            scanf("%d", &type);
            if(type == 0)
            {
                e_storage_type = E_QL_SMS_STORAGE_TYPE_NV;
            }
            else
            {
                e_storage_type = E_QL_SMS_STORAGE_TYPE_SIM;
            }
            
            ret = QL_SMS_SetPreferredStorage(h_msg, e_storage_type);
            printf("QL_SMS_SetPreferredStorage ret = %d\n", ret);
            break;
        }
        case 56://"QL_SMS_GetSenderTel"
            ret = QL_SMS_GetSenderTel(h_msg, buf, BUF_SIZE);
            printf("QL_SMS_GetSenderTel ret = %d, SenderTel=%s\n", ret, buf);
            break;
        case 57://"QL_SMS_GetTimeStamp"
            ret = QL_SMS_GetTimeStamp(h_msg, buf, BUF_SIZE);
            printf("QL_SMS_GetTimeStamp ret = %d, TimeStamp=%s\n", ret, buf);
            break;
        case 58://"QL_SMS_GetUserdataLen"
            value = QL_SMS_GetUserdataLen(h_msg);
            printf("UserdataLen = %d\n", value);
            break;
        case 59://"QL_SMS_GetText"
            ret = QL_SMS_GetText(h_msg, buf, BUF_SIZE);
            printf("QL_SMS_GetText ret = %d, Text=%s\n", ret, buf);
            break;
        case 60://"QL_SMS_GetBinary"
        {
            int i;
            size_t dataLen = BUF_SIZE;
            ret = QL_SMS_GetBinary(h_msg, (char*)buf, &dataLen);
            printf("QL_SMS_GetBinary ret = %d, dataLen=%d, Data:\n", ret, (int)dataLen);
            for(i=0; i<dataLen; i++)
            {
                printf("%02X", buf[i]);
            }
            break;
        }
        case 61://"QL_SMS_GetUCS2"
        {
            int i;
            size_t dataLen = BUF_SIZE;
            ret = QL_SMS_GetUCS2(h_msg, (uint16_t*)buf, &dataLen);
            printf("QL_SMS_GetUCS2 ret = %d, dataLen=%d, Data:\n", ret, (int)dataLen);
            for(i=0; i<dataLen; i++)
            {
                printf("%02X", buf[i]);
            }
            break;
        }
        case 62://"QL_SMS_GetPDU"
        {
            int i;
            size_t dataLen = BUF_SIZE;
            ret = QL_SMS_GetPDU(h_msg, (char*)buf, &dataLen);
            printf("QL_SMS_GetPDU ret = %d, dataLen=%d, Data:\n", ret, (int)dataLen);
            for(i=0; i<dataLen; i++)
            {
                printf("%02X", buf[i]);
            }
            break;
        }
        case 63://"QL_SMS_GetPDULen"
            value = QL_SMS_GetPDULen(h_msg);
            printf("UserdataLen = %d\n", value);
            break;
        case 64://"QL_SMS_DeleteFromStorage"
            ret = QL_SMS_DeleteFromStorage(h_msg);
            printf("QL_SMS_DeleteFromStorage ret= %d\n", ret);
            break;
        case 65://"QL_SMS_GetSmsCenterAddress"
            ret = QL_SMS_GetSmsCenterAddress(h_msg, buf, BUF_SIZE);
            printf("QL_SMS_GetSmsCenterAddress ret = %d, SmsCenterAddress=%s\n", ret, buf);
            break;
        case 66://"QL_SMS_SetSmsCenterAddress"
            ret = QL_SMS_SetSmsCenterAddress(h_msg, "13010305500");
            printf("QL_SMS_SetSmsCenterAddress ret = %d\n", ret);
            break;
            
        case 70://"QL_VCALL_AddStateHandler"
        {
            h_voicecall = QL_VCALL_AddStateHandler(ql_vcall_cb_func, (void*)&h_call);
            printf("QL_VCALL_AddStateHandler h_voicecall = %d\n", h_voicecall);
            break;
        }
        case 71://"QL_VCALL_RemoveStateHandler"
            QL_VCALL_RemoveStateHandler(h_voicecall);
            printf("QL_VCALL_RemoveStateHandler finished\n");
            break;
        case 72://"QL_VCALL_Start"
        {
            char    PhoneNum[QL_SMS_PHONE_NUM_MAX_LEN]                  = {0}; 

            printf("please input dest phone number: \n");
            scanf("%s", PhoneNum);
            
            h_call = QL_VCALL_Start(E_QL_VCALL_EXTERNAL_SLOT_1, PhoneNum);
            printf("QL_VCALL_Start h_call=%d\n", h_call);
        }
            break;
        case 73://"QL_VCALL_End"
            ret = QL_VCALL_End(h_call);
            printf("QL_VCALL_End ret=%d \n", ret);
            break;
        case 74://"QL_VCALL_Answer"
            ret = QL_VCALL_Answer(h_call);
            printf("QL_VCALL_Answer ret=%d \n", ret);
            break;

        case 80://"set_selected_apn_idx"
        {
            do
            {
                printf("please input valid apn index(1~5): ");
                scanf("%d", &g_cur_apn_idx);
                if(g_cur_apn_idx > SUPPORTED_APN_MAX)
                {
                    continue;
                }
                g_cur_apn_idx -= 1;//used as array index
            }while(0);
            break;
        }            
        case 90://"test_sleep_wakeup"
            break;
            
        case 97://"testapi_client(work as client, send msg to server)"
            testapi_client();
            break;
            
        case 98://"stop test client"
        {
            g_heartbeat_thread_run = 0;
            break;
        }
            
        case 99://"testapi_server(work as server, receive msg and ack msg+ack)"
        {
            test_server_daemon_thread_init((int)g_h_handle[g_cur_apn_idx]);
            break;
        }
        case 100://"stop test server"
        {
            g_is_server_daemon_running = 0;
            break;
        }
        
        case 101://"test_udp_server"
        {
            test_udp_server();
            break;
        }
        case 102://"stop test_udp_server"
        {
            g_is_udp_server_running = 0;
            break;
        }

        
        case 103://"test_udp_server"
        {
            test_udp_client();
            break;
        }
    
        
        case 104://"testapi_client_send_ack"
        {
            testapi_client_send_ack();
            break;
        }

        case 110://"start_qmi_timer to send heartbeat"
        {
            int timeout = 0;
            int repeat_flag = 0;  
            printf("please input timeout(s) and repeat flag, i.e 10 0/1: ");
            scanf("%d%d", &timeout, &repeat_flag);
            start_qmi_timer(heart_beat_timer_expire_cb, timeout, repeat_flag, (void *)g_testclient_sockfd);
            break;
        }

        case 111://"auto sleep onoff"
        {
            //force_sleep();
            int onoff = 0;  
            printf("please input autosleep onoff value(on:1, off:0): ");
            scanf("%d", &onoff);
            Ql_Autosleep_Enable(onoff!=0);
            break;
        }
        
        case 112://"start_qmi_timer to send sms"
        {
            int timeout = 0;
            int repeat_flag = 0;  
            
            printf("please input dest phone number: ");
            scanf("%s", phoneNum);
            
            printf("please input timeout(s) and repeat flag, i.e 10 0/1: ");
            scanf("%d%d", &timeout, &repeat_flag);
            g_timer_id = start_qmi_timer(sms_timer_expire_cb, timeout, repeat_flag, (void*)phoneNum);
            break;
        }
        
	case 120:	/*resolve private network domain*/
	{
	    nslookup();
	    break;
	}

        default:
            print_help();
            break;   
        }
    }
    return 0;
}
