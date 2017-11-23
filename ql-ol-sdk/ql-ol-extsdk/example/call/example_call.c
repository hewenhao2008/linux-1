#include "ql_oe.h"
#include <aio.h>
#include <signal.h>
#include <unistd.h>
 #include <fcntl.h>


#define QUEC_AT_PORT    "/dev/smd8"

static int smd_fd = -1;
int Ql_SendAT(char* atCmd, char* finalRsp, long timeout_ms);


static void SIGIO_Handler_Register(void);
static void SIGIO_Handler_Unregister(void);
static struct sigaction old_saio;
static int old_F_GETFL;
static int old_F_GETOWN;

/*
备注:本例子中，使用select系统调用介绍at 命令的响应.
              使用SIGIO处理函数来接收模块主动上报的URC(比如ring，新信息提醒等)
*/

int main(int argc, char* argv[])
{
	static int count = 0;
	printf("< QuecOpen: call example >\n");
	
	smd_fd = open(QUEC_AT_PORT, O_RDWR | O_NONBLOCK | O_NOCTTY);
	printf("< open(\"%s\")=%d >\n", QUEC_AT_PORT, smd_fd);

	old_F_GETFL = fcntl(smd_fd,F_GETFL); /*保存原始值*/
	old_F_GETOWN = fcntl(smd_fd,F_GETOWN);
	printf("fcntl(smd_fd,F_GETFL)= %2x\n",fcntl(smd_fd,F_GETFL));
	printf("fcntl(smd_fd,F_GETOWN)= %2x\n",fcntl(smd_fd,F_GETOWN));
	
	
	SIGIO_Handler_Register();

	// start to send AT...
	Ql_SendAT("ATE0", "OK", 1000);
	Ql_SendAT("AT", "OK", 1000);
	Ql_SendAT("ATI", "OK", 1000);
	Ql_SendAT("AT+CFUN?", "OK", 1000);
	Ql_SendAT("AT+CPIN?", "OK", 1000);
	Ql_SendAT("AT+CSQ",  "OK", 1000);
	Ql_SendAT("AT+CREG?", "OK", 1000);
	Ql_SendAT("AT+COPS?", "OK", 1000);
	
   
	//Ql_SendAT("ATD15013828037;", "OK", 10*1000);

	while (1)
	{
		//iRet = Ql_UART_Write(fd_uart, (const void*)strTmp, sizeof(strTmp));
		//printf("< write(fd=%d)=%d\n", fd_uart, iRet);

		sleep(1);
		printf("count = %d\n",count++);
		if(count == 5)
		{
			Ql_SendAT("AT+QSCLK=1", "OK", 1000);/*10秒后让模块进入sleep模式, 测试能否接收到来电urc*/
		}
	}

	close(smd_fd);
	printf("< QuecOpen: call example end >\n");
	return 0;
}

int Ql_SendAT(char* atCmd, char* finalRsp, long timeout_ms)
{
    int iRet;
    int iLen;
    fd_set fds;
    int rdLen;
#define lenToRead 100
    char strAT[100];
    char strFinalRsp[100];
    char strResponse[100];
    struct timeval timeout = {0, 0};
    boolean bRcvFinalRsp = FALSE;

    SIGIO_Handler_Unregister();

    memset(strAT, 0x0, sizeof(strAT));
    iLen = sizeof(atCmd);
    strncpy(strAT, atCmd, iLen);

    sprintf(strFinalRsp, "\r\n%s", finalRsp);
    
		timeout.tv_sec  = timeout_ms / 1000;
		timeout.tv_usec = timeout_ms % 1000;
    
    
    // Add <cr><lf> if needed
    iLen = strlen(atCmd);
    if ((atCmd[iLen-1] != '\r') && (atCmd[iLen-1] != '\n'))
    {
        iLen = sprintf(strAT, "%s\r\n", atCmd); 
        strAT[iLen] = 0;
    }

    // Send AT
    iRet = write(smd_fd, strAT, iLen);
    printf(">>Send AT: \"%s\", iRet=%d\n", atCmd, iRet);
    
    // Wait for the response
	while (1)
	{
		FD_ZERO(&fds); 
		FD_SET(smd_fd, &fds); 

		printf("timeout.tv_sec=%d, timeout.tv_usec: %d \n", (int)timeout.tv_sec, (int)timeout.tv_usec);
		switch (select(smd_fd + 1, &fds, NULL, NULL, &timeout))
		//switch (select(smd_fd + 1, &fds, NULL, NULL, NULL))	// block mode
		{
		case -1: 
			printf("< select error >\n");
			return -1;

		case 0:
			printf("< time out >\n");
			return 1; 

		default: 
			if (FD_ISSET(smd_fd, &fds)) 
			{
				do {
					memset(strResponse, 0x0, sizeof(strResponse));
					rdLen = read(smd_fd, strResponse, lenToRead);
					printf(">>Read response/urc, len=%d, content:\n%s\n", rdLen, strResponse);
					//printf("rcv:%s", strResponse);
					//printf("final rsp:%s", strFinalRsp);
					if ((rdLen > 0) && strstr(strResponse, strFinalRsp))
					{
					    if (strstr(strResponse, strFinalRsp)     // final OK response
					       || strstr(strResponse, "+CME ERROR:") // +CME ERROR
					       || strstr(strResponse, "+CMS ERROR:") // +CMS ERROR
					       || strstr(strResponse, "ERROR"))      // Unknown ERROR
					    {
					        //printf("\n< match >\n");
					        bRcvFinalRsp = TRUE;
					    }else{
					        printf("\n< not final rsp >\n");
					    }
					}
				} while ((rdLen > 0) && (lenToRead == rdLen));
			}else{
				printf("FD is missed\n");
			}
			break;
		}

		// Found the final response , return back
		if (bRcvFinalRsp)
		{
		    break;
		}	
   	}
   	SIGIO_Handler_Register();
   	return 0;
}

static void SIGIO_Handler(int status)
{
    int rdLen;

    #define lenToRead 100
    char strResponse[100];

    printf("receive a SIGIO signal\n");

    memset(strResponse, 0x0, sizeof(strResponse));
    rdLen = read(smd_fd, strResponse, lenToRead);
    printf("%s>>Read response/urc, len=%d, content:\n%s\n", __FUNCTION__,rdLen, strResponse);


    if(strstr(strResponse,"RING"))
    {
        Ql_SendAT("AT+QSCLK=0", "OK", 1000);
    }
}

/**
  * @brief  为SIGIO信号注册一个处理函数
  * @retval none.
  */

static void SIGIO_Handler_Register(void)
{
    struct sigaction saio; /*definition of signal axtion */

    //printf("SIGIO_Handler_Register\n");
    saio.sa_handler = SIGIO_Handler;
    sigemptyset(&saio.sa_mask);
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;

    sigaction(SIGIO,&saio,&old_saio);/*change SIGIO' process  actionl*/

    fcntl(smd_fd,F_SETOWN,getpid());/* allow the process to recevie SIGIO*/
    fcntl(smd_fd,F_SETFL,FASYNC);/* Make the file descriptor asynchronous*/
    //printf("fcntl(smd_fd,F_GETOWN)= %2x\n",fcntl(smd_fd,F_GETOWN));
    //printf("fcntl(smd_fd,F_GETFL)= %2x\n",fcntl(smd_fd,F_GETFL));
}


/**
  * @brief  : 取消SIGIO信号注册的处理函数函数
  * @retval:  none.
  */

static void SIGIO_Handler_Unregister(void)
{
    //printf("SIGIO_Handler_Unregister\n");
    sigaction(SIGIO,&old_saio,NULL);
    fcntl(smd_fd,F_SETFL,old_F_GETFL);
    fcntl(smd_fd,F_SETOWN,old_F_GETOWN);
    //printf("fcntl(smd_fd,F_GETFL)= %2x\n",fcntl(smd_fd,F_GETFL));
    //printf("fcntl(smd_fd,F_GETOWN)= %2x\n",fcntl(smd_fd,F_GETOWN));
}
