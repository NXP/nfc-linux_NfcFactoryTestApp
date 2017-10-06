/*
*         Copyright (c), NXP Semiconductors Caen / France
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <poll.h>
#include <tml.h>

/* Describe PN71xx connection
 *  0 = Custom configuration
 *  1 = OM557x on Raspberry Pi
 *  2 = OM557x on UdooNeo
 */
#define CONFIGURATION    3

#if (CONFIGURATION == 1)
/* OM557x on Raspberry Pi */
 #define I2C_BUS         "/dev/i2c-1"
 #define I2C_ADDRESS     0x28
 #define PIN_INT         23
 #define PIN_ENABLE      24
#elif (CONFIGURATION == 2)
/* OM557x on UdooNeo */
 #define I2C_BUS         "/dev/i2c-1"
 #define I2C_ADDRESS     0x28
 #define PIN_INT         105
 #define PIN_ENABLE      149
#elif (CONFIGURATION == 3)
/* OM557x on BeagleBone black */
 #define I2C_BUS         "/dev/i2c-2"
 #define I2C_ADDRESS     0x28
 #define PIN_INT         61
 #define PIN_ENABLE      30
#else
/* Custom configuration */
 #define I2C_BUS         "/dev/i2c-1"
 #define I2C_ADDRESS     0x28
 #define PIN_INT         23
 #define PIN_ENABLE      24
#endif

#define EDGE_NONE    0
#define EDGE_RISING  1
#define EDGE_FALLING 2
#define EDGE_BOTH    3

static int iEnableFd    = 0;
static int iInterruptFd = 0;

static int verifyPin( int pin, int isoutput, int edge ) {
    char buf[40];
    int hasGpio = 0;

    sprintf( buf, "/sys/class/gpio/gpio%d", pin );
    int fd = open( buf, O_RDONLY );
    if ( fd <= 0 ) {
        // Pin not exported yet
        if ( ( fd = open( "/sys/class/gpio/export", O_WRONLY ) ) > 0 ) {
            sprintf(buf, "%d", pin);
            if ( write( fd, buf, strlen(buf)) == strlen(buf)) {
                hasGpio = 1;
            }
            close( fd );
        }
    } else {
        hasGpio = 1;
        close( fd );
    }
    usleep(100000);
    if ( hasGpio ) {
        // Make sure it is an output
        sprintf( buf, "/sys/class/gpio/gpio%d/direction", pin );
        fd = open( buf, O_WRONLY );
        if ( fd > 0 ) {
            if ( isoutput ) {
                write(fd,"out",3);
                close(fd);

                // Open pin and make sure it is off
                sprintf( buf, "/sys/class/gpio/gpio%d/value", pin );
                fd = open( buf, O_RDWR );
                if ( fd > 0 ) {
                    write( fd, "0", 1 );
                    return( fd );  // Success
                }
            } else {
                write(fd,"in",2);
                close(fd);

                if(edge != EDGE_NONE) {
                    // Open pin edge control
                    sprintf( buf, "/sys/class/gpio/gpio%d/edge", pin );
                    fd = open( buf, O_RDWR );
                    if ( fd > 0 ) {
                        char * edge_str = "none";
                        switch ( edge ) {
                          case EDGE_RISING:  edge_str = "rising"; break;
                          case EDGE_FALLING: edge_str = "falling"; break;
                          case EDGE_BOTH:    edge_str = "both"; break;
                          default: break;
                        }
                        write( fd, edge_str, strlen(edge_str));
                        close(fd);
                    }
                }
                // Open pin
                sprintf( buf, "/sys/class/gpio/gpio%d/value", pin );
                fd = open( buf, O_RDONLY );
                if ( fd > 0 ) {
                    return( fd ); // Success
                }
            }
        }
    }
    return( 0 );
}

static int pnGetint( void ) {
    char buf[2];
    int len;
    if (iInterruptFd <= 0) return -1;
    lseek(iInterruptFd, SEEK_SET, 0);
    len = read(iInterruptFd, buf, 2);
    if (len != 2) return 0;
    return (buf[0] != '0');
}

static void wait4interrupt( void ) {
    struct pollfd fds[1];
    fds[0].fd = iInterruptFd;
    fds[0].events = POLLPRI;
    int timeout_msecs = 10000;
    while (!pnGetint()) poll(fds, 1, timeout_msecs);
}

int tml_open(int * handle)
{
    iInterruptFd = verifyPin(PIN_INT, 0, EDGE_RISING);
    iEnableFd = verifyPin(PIN_ENABLE, 1, EDGE_NONE);
    *handle = open(I2C_BUS, O_RDWR | O_NOCTTY);
    if((*handle <= 0) || (iInterruptFd <= 0) || (iEnableFd <= 0)) goto error;
    if(ioctl(*handle, I2C_SLAVE, I2C_ADDRESS) < 0) goto error;

    return 0;

error:
    if (iEnableFd) close(iEnableFd);
    if (iInterruptFd) close(iInterruptFd);
    if (*handle) close(*handle);
    return -1;
}

void tml_close(int handle)
{
    if(iEnableFd) close(iEnableFd);
    if(iInterruptFd) close(iInterruptFd);
    if(handle) close(handle);
}

void tml_reset(int handle)
{
    if(iEnableFd) write(iEnableFd, "0", 1 );
    usleep(10 * 1000);
    if(iEnableFd) write(iEnableFd, "1", 1 );
    usleep(10 * 1000);
}

int tml_send(int handle, char *pBuff, int buffLen)
{
    int ret = write(handle, pBuff, buffLen);
    if(ret <= 0) {
        /* retry to handle standby mode */
        ret = write(handle, pBuff, buffLen);
        if(ret <= 0) return 0;
    }
    PRINT_BUF(">> ", pBuff, ret);
    return ret;
}

int tml_receive(int handle, char *pBuff, int buffLen)
{
    int numRead;
    struct timeval tv;
    fd_set rfds;
    int ret;

    FD_ZERO(&rfds);
    FD_SET(handle, &rfds);
    tv.tv_sec = 2;
    tv.tv_usec = 1;
    ret = select(handle+1, &rfds, NULL, NULL, &tv);
    if(ret <= 0) return 0;

    wait4interrupt();

    ret = read(handle, pBuff, 3);
    if (ret <= 0) return 0;
    numRead = 3;
    if(pBuff[2] + 3 > buffLen) return 0;

    ret = read(handle, &pBuff[3], pBuff[2]);
    if (ret <= 0) return 0;
    numRead += ret;

    PRINT_BUF("<< ", pBuff, numRead);

    return numRead;
}

int tml_transceive(int handle, char *pTx, int TxLen, char *pRx, int RxLen)
{
    if(tml_send(handle, pTx, TxLen) == 0) return 0;
    return tml_receive(handle, pRx, RxLen);
}
