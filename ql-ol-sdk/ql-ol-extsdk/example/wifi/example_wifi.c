#include "ql_oe.h"
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>

#define QUEC_AT_PORT    "/dev/smd8"

static int smd_fd = -1;
static char *on_off= "0";
static char *name = "QsoftAP";
static char *passwd = "1234567890";
static char look;

static char wifion_off[64] = "AT+QWIFI=";
static char wifiname[64]="AT+QWSSID=";
static char wifipasswd[64]="AT+QWAUTH=4,3,\"";

int Ql_SendAT(char* atCmd, char* finalRsp, long timeout_ms);

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -O --on_off    wifi on or off\n"
	     "  -N --name      wifi name\n"
	     "  -P --passwd    wifi passwd \n"
	     "  -L --look      look wifi stats name passwd \n"
	);
	exit(1);
}


static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "on_off",  1, 0, 'O' },
			{ "name",    1, 0, 'N' },
			{ "passwd",  1, 0, 'P' },
			{ "look",    1, 0, 'L' },
			{ NULL,      0, 0,  0 },
		};
		int c;

		c = getopt_long(argc, argv, "O:N:P:L:", lopts, NULL);

		if (c == -1)
			break;
		switch (c) {
		case 'O':
			on_off = optarg;
			strcat(wifion_off,on_off);
			printf("%s\n",wifion_off);
			Ql_SendAT(wifion_off, "OK", 1000);
			break;
		case 'N':
			name = optarg;
			strcat(wifiname,name);
			printf("%s\n",wifiname);
			Ql_SendAT(wifiname, "OK", 1000);
			break;
		case 'P':
			passwd = optarg;
			printf("passwd = %s\n",passwd);
			strcat(wifipasswd,passwd);
			strcat(wifipasswd,"\"");
			printf("%s\n",wifipasswd);
			Ql_SendAT(wifipasswd, "OK", 1000);
			break;
		case 'L':
			look =atoi(optarg);
			printf("look =%d\n",look);
			Ql_SendAT("AT+QWIFI?", "OK", 1000);
			Ql_SendAT("AT+QWSSID?", "OK", 1000);
			Ql_SendAT("AT+QWAUTH?", "OK", 1000);
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	printf("< Quectel OpenLinux: AT example >\n");
	
    	smd_fd = open(QUEC_AT_PORT, O_RDWR | O_NONBLOCK | O_NOCTTY);
	printf("< open(\"%s\")=%d >\n", QUEC_AT_PORT, smd_fd);
	int cfun,cops,cpin;

	cfun =Ql_SendAT("AT+CFUN?", "OK", 1000);
	cpin =Ql_SendAT("AT+CPIN?", "OK", 1000);
	Ql_SendAT("AT+CREG?", "OK", 1000);
	cops =Ql_SendAT("AT+COPS?", "OK", 1000);
	
	printf("cfun =%d,cpin = %d,cops =%d\n",cfun,cpin,cops);

	if(0 == cpin ){ //sim is ok can set wifi 
	printf("Start set wifi ............\n");
	sleep(2);
	parse_opts(argc, argv);
	}



   	close(smd_fd);
    printf("< Quectel OpenLinux: AT example end >\n\n");
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
   	return 0;
}

