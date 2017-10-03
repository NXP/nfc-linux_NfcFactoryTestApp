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
#include <unistd.h>
#include <tml.h>

int tml_open(int * handle)
{
    printf("!!! Dummy TML used ... no communication to PN7xx implemented !!!\n");
    return 0;
}

void tml_close(int handle)
{
}

void tml_reset(int handle)
{
}

int tml_send(int handle, char *pBuff, int buffLen)
{
    return 0;
}

int tml_receive(int handle, char *pBuff, int buffLen)
{
    return 0;
}

int tml_transceive(int handle, char *pTx, int TxLen, char *pRx, int RxLen)
{
    return 0;
}
