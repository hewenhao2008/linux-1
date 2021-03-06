#include "ql_oe.h"


#define QL_UART1_DEV  "/dev/ttyHSL0\0"

static int fd_uart = -1;

static pthread_t thrd_uart_rcv;
static void* UartRecv_Proc(void* arg);

int main(int argc, char* argv[])
{
	int iRet;
	int baudRate;
	char strTmp[] = "uart test, =+-_0)9(8*7&6^5%4$3#2@1!`~)\n\0";

    if (argc < 2)
    {
          printf("Usage: %s <baud rate> \n", argv[0]);
          return -1;
    }

	printf("< OpenLinux: UART example >\n");

    baudRate = atoi(argv[1]);
	fd_uart = Ql_UART_Open(QL_UART1_DEV, baudRate, FC_NONE);
	printf("< open(\"%s\", %d)=%d\n", QL_UART1_DEV, baudRate, fd_uart);
	
    /*
     *  Create a thread to handle uart rx data
     */
    if (pthread_create(&thrd_uart_rcv, NULL, UartRecv_Proc, NULL) != 0)
    {
        printf("Fail to create thread!\n");
    }

	while (fd_uart >=0)
	{
		iRet = Ql_UART_Write(fd_uart, (const void*)strTmp, sizeof(strTmp));
		printf("< write(fd=%d)=%d\n", fd_uart, iRet);
		sleep(1);
	}

	return 0;
}

static void* UartRecv_Proc(void* arg)
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
				} while (100 == iRet);
			}
		}
	}
	return (void *)1;
}
