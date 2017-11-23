#include "ql_oe.h"
#include "ql_eint.h"


static Enum_PinName m_GpioPin = PINNAME_GPIO5;

static void InterruptHdlr_Test(Enum_PinName  pinName, boolean level, void* param);
static int  interrupt_id = 2016;
static void* thrd_fdMonitor(void* arg);
static int CreateInterruptMonitor(Enum_PinName pinName);


int main(int argc, char* argv[])
{
	int iRet;
	Enum_EintType senseType = EINT_SENSE_BOTH;

	printf("< OpenLinux: interrupt example >\n");

    /*
     * open eint pin, and create monitor for the eint level
     */
    CreateInterruptMonitor(m_GpioPin);

    // Enable / Disable eint
    if (argc > 1)
    {
        if (strncmp(argv[1], "disable", 7) == 0)
        {
            senseType = EINT_SENSE_NONE;
        }
        else if (strncmp(argv[1], "rising", 6) == 0)
        {
            senseType = EINT_SENSE_RISING;
        }
        else if (strncmp(argv[1], "falling", 7) == 0)
        {
            senseType = EINT_SENSE_FALLING;
        }
    }
    if (EINT_SENSE_NONE == senseType)
    {
        iRet = Ql_EINT_Disable(m_GpioPin);
        printf("< Ql_EINT_Disable: pin=%d, iRet=%d >\n", m_GpioPin, iRet);
    } else {
	    iRet = Ql_EINT_Enable(m_GpioPin, senseType);
	    printf("< Ql_EINT_Enable: pin=%d, type=%d, iRet=%d >\n", m_GpioPin, senseType, iRet);
    }
    while (1)
	{
	}
 
    return 0;
}

static void InterruptHdlr_Test(Enum_PinName  pinName, boolean activeLevel, void* param)
{
    printf("< %s: pin=%d, active level=%d, callback param=%d >\n", __func__, pinName, activeLevel, *((int*)param));
}

//--------------------------------------------------------------------------------------------------
/**
 * This thread is used to monitor the specified FD.
 * And callback to the upper application.
 */
//--------------------------------------------------------------------------------------------------
#define MAX_EPOLL_EVENTS 32
static pthread_t monitorFd_thrd = 0;
static int monitorFd_epoll = -1;
static int monFd_01;

static void* thrd_fdMonitor(void* arg)
{
    int iResult;
    struct epoll_event epollEventList[MAX_EPOLL_EVENTS];

    printf("Enter fdMonitor thread, PID:%lu\n", pthread_self());
    
    // Enter the infinite loop of the thread.
    for (;;)
    {
        iResult = epoll_wait(monitorFd_epoll, epollEventList, MAX_EPOLL_EVENTS, -1);
        printf("epoll_wait()=%d\n", iResult);
        if (iResult > 0)
        {
            int i;
            for (i = 0; i < iResult; i++)
            {
                int currFd = epollEventList[i].data.fd;
                printf("events:%u, fd:%d, ptr:%X \n", epollEventList[i].events, currFd, (uint32_t)epollEventList[i].data.ptr);                

                // read interrupt level
                {
                    // Just read a single character
                    char buf[1];

                    // Seek to the start of the file - this is required to prevent
                    // repeated triggers - see https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
                    printf("Seek to start of file %d \n", currFd);
                    lseek(currFd, 0, SEEK_SET);

                    if (read(currFd, buf, 1) != 1)
                    {
                        printf("Unable to read value for GPIO %d", m_GpioPin);
                        return NULL;
                    }
                    printf("Read value %c from value file for callback\n", buf[0]);

                    InterruptHdlr_Test(m_GpioPin, '1' == buf[0], &interrupt_id);
                }
            }
        }
    }
}

int CreateInterruptMonitor(Enum_PinName pinName)
{
    int monFd = 0;
    int iRet;

    printf("\n--- %s ---\n", __func__);

    monFd = Ql_EINT_Open(pinName);
	printf("< Open EINT: pin=%d, iRet=%d >\n", pinName, monFd);
	if (monFd < 0)
	{
	    exit(-1);
	}
	monFd_01 = monFd;
    
	// Create a file descriptor monitor
	// Create epoll file descriptor
	if (-1 == monitorFd_epoll)
	{
	    monitorFd_epoll = epoll_create1(0);
	    printf("monitorFd_epoll=%d\n", monitorFd_epoll);
	}
	
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLPRI;	// Non-deferrable by default.
    ev.data.fd = monFd;
    // ev.data.ptr = hdlr;  // union
    printf("< ev.events=%d, ev.data.fd=%d, monFd=%d, ev.data.ptr=%X >\n", ev.events, ev.data.fd, monFd, (uint32_t)ev.data.ptr);
    iRet = epoll_ctl(monitorFd_epoll, EPOLL_CTL_ADD, monFd, &ev);
    printf("epoll_ctl(add,...)=%d \n", iRet);
    if (-1 == iRet)
    {
        if (errno == EPERM)
        {
            printf("fd %d doesn't support epoll(), assuming always readable and writeable.", monFd);
            //isAlwaysReady = true;
        } else {
            printf("epoll_ctl(ADD) failed for fd %d. errno = %d (%m)", monFd, errno);
        }
        return -3;
    } else {
        printf("start to monitor interrupt level/edage \n");
    }

    // Create a thread to monitor the FD
    if (0 == monitorFd_thrd)
    {
        if (pthread_create(&monitorFd_thrd, NULL, thrd_fdMonitor, NULL) != 0)
        {
            printf("Fail to create fd-monitor thread!\n");
            return -4;
        }
    }
    return RES_OK;
}


