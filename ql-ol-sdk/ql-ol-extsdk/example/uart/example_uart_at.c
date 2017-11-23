#include "ql_oe.h"


#define QL_UART1_DEV  "/dev/ttyHSL1\0"
#define QUEC_AT_PORT  "/dev/smd8"

static int fd_uart = -1;
static int smd_fd = -1;

static pthread_t thrd_uart_rcv;
static void* UartRecv_Proc(void* arg);
int Ql_SendAT(char* atCmd, char* finalRsp, long timeout_ms);

int main(int argc, char* argv[])
{
	//int iRet;
	int baudRate;
	//char strTmp[] = "uart test, =+-_0)9(8*7&6^5%4$3#2@1!`~)\n\0";

    if (argc < 2)
    {
          printf("Usage: %s <baud rate> \n", argv[0]);
          return -1;
    }

	printf("< OpenLinux: UART AT example >\n");

    baudRate = atoi(argv[1]);
	fd_uart = Ql_UART_Open(QL_UART1_DEV, baudRate, FC_NONE);
	printf("< open(\"%s\", %d)=%d\n", QL_UART1_DEV, baudRate, fd_uart);

	smd_fd = open(QUEC_AT_PORT, O_RDWR | O_NONBLOCK | O_NOCTTY);
	printf("< open(\"%s\")=%d >\n", QUEC_AT_PORT, smd_fd);

    /*
     *  Create a thread to handle uart rx data
     */
    if (pthread_create(&thrd_uart_rcv, NULL, UartRecv_Proc, NULL) != 0)
    {
        printf("Fail to create thread!\n");
    }

	while (fd_uart >=0)
	{
		//iRet = Ql_UART_Write(fd_uart, (const void*)strTmp, sizeof(strTmp));
		//printf("< write(fd=%d)=%d\n", fd_uart, iRet);
		sleep(1);
	}

	return 0;
}

void* UartRecv_Proc(void* arg)
{
	int iRet;
	fd_set fdset;
	struct timeval timeout = {3, 0};	// timeout 3s
	char buffer[100] = {0};

	while (fd_uart >=0)
	{
		FD_ZERO(&fdset); 
		FD_SET(fd_uart, &fdset); 
		iRet = select(fd_uart + 1, &fdset, NULL, NULL, &timeout);
		if (-1 == iRet)
		{
			printf("< failed to select >\n");
			exit(-1);
		}
		else if (0 == iRet)
		{// no data in Rx buffer
			printf("< no data >\n");
			timeout.tv_sec  = 3;
			timeout.tv_usec = 0;
		}
		else
		{// data is in Rx data
			if (FD_ISSET(fd_uart, &fdset)) 
			{
				do {
					memset(buffer, 0x0, sizeof(buffer));
					iRet = Ql_UART_Read(fd_uart, buffer, 100);
					printf("> read(uart)=%d:\n%s\n", iRet, buffer);

					Ql_SendAT(buffer, "OK", 3000);
				} while (100 == iRet);
			}
		}
	}
	return (void *)1;
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

					Ql_UART_Write(fd_uart, (const void*)strResponse, rdLen);
					
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
   	return 0;
}

