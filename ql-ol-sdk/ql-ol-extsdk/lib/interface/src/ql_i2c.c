
#include "ql_oe.h"

#define DEBUG_EN  0
#if DEBUG_EN
#define _DEBUG(fmtString, ...)     printf(fmtString, ##__VA_ARGS__)
#else
#define _DEBUG(fmtString, ...)
#endif

int Ql_I2C_Init(char *dev_name)
{
    int fd_i2c = open(dev_name, O_RDWR);
    _DEBUG("< open(%s, O_RDWR)=%d >\n", dev_name, fd_i2c);
    if (fd_i2c < 0)  
    {
        _DEBUG("< Fail to open i2c >\n");
        return -1;  
    }  
    return fd_i2c;
}

int Ql_I2C_Read(int fd, unsigned short slaveAddr, unsigned char ofstAddr,  unsigned char* ptrBuff, unsigned short length)
{
	int iRet = 0;
	
	struct i2c_msg i2c_msgs[] = {
	    [0] = {
		    .addr  = slaveAddr,
		    .flags = 0, // write
		    .buf   = &ofstAddr,
		    .len   = 1,
	    },
	    [1] = {
		    .addr  = slaveAddr,
		    .flags = I2C_M_RD,
		    .buf   = ptrBuff,
		    .len   = length, 
	    },
	};

	struct i2c_rdwr_ioctl_data msgset = {
		.msgs = i2c_msgs,
		.nmsgs = 2,
	};

	iRet = ioctl(fd, I2C_RDWR, &msgset);
 	if (iRet < 0)
	{
		_DEBUG("%s, read failed rc : %d \n", __FUNCTION__, iRet);
	}
    _DEBUG("%s, line: %d, addr: 0x%x, value[0]: %x, rc: %d \n", 
        __FUNCTION__, __LINE__, i2c_msgs[0].addr, ptrBuff[0], iRet);

	return iRet;
}

#if 0
int Ql_I2C_Write(int fd, unsigned short slaveAddr, unsigned char ofstAddr,  unsigned char* ptrData, unsigned short length)
{
	int iRet = 0;
	
	struct i2c_msg i2c_msgs = {
	    .addr  = slaveAddr,
	    .flags = 0, // write
	    .buf   = NULL,
	    .len   = length + 1,
	};
	i2c_msgs.buf = (unsigned char*)malloc(length + 1);
	i2c_msgs.buf[0] = ofstAddr; // target address
	memcpy(&i2c_msgs.buf[1], ptrData, length);

	struct i2c_rdwr_ioctl_data msgset = {
		.msgs  = &i2c_msgs,
		.nmsgs = 1,
	};

	iRet = ioctl(fd, I2C_RDWR, &msgset);
	free(i2c_msgs.buf);
 	if (iRet < 0)
	{
		_DEBUG("%s, write failed, iRet=%d \n", __FUNCTION__, iRet);
	}

	return iRet;
}

#else

int Ql_I2C_Write(int fd, unsigned short slaveAddr, unsigned char ofstAddr,  unsigned char* ptrData, unsigned short length)
{
    int iRet = 0;
    uint8_t write_buff[3] = {0, 0, 0}; 

    struct i2c_msg i2c_msgs = {
        .addr  = slaveAddr,
        .flags = 0, // write
        .buf   = (void *)write_buff,
        .len   = length + 1,
    };
    struct i2c_rdwr_ioctl_data msgset = {
        .msgs  = &i2c_msgs,
        .nmsgs = 1,
    };
    
    write_buff[0] = ofstAddr;
    if(1 == length)
    {
        write_buff[1] = ptrData[0];
    }
    else if(2 == length)
    {
        write_buff[2] = ptrData[0];
        write_buff[1] = ptrData[1];
    }
    else
    {
        return -1;
    }

    printf("address=0x%x, offset=%d, byte_num=%d, value[1]=%d, value[2]=%d\n",
            slaveAddr,    ofstAddr,    length,    write_buff[1], write_buff[2]);
    
    iRet = ioctl(fd, I2C_RDWR, &msgset);
    //free(i2c_msgs.buf);
    if (iRet < 0)
    {
        _DEBUG("%s, write failed, iRet=%d \n", __FUNCTION__, iRet);
    }

    return iRet;
}
#endif

int Ql_I2C_Deinit(int fd)
{
    close(fd);
}
