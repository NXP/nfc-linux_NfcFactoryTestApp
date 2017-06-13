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

//#define DEBUG

#ifdef DEBUG
#define PRINT_BUF(x,y,z)  {int loop; printf(x); \
                           for(loop=0;loop<z;loop++) printf("%.2x ", y[loop]); \
                           printf("\n");}
#else
#define PRINT_BUF(x,y,z)
#endif

int tml_open(int * handle);
void tml_close(int handle);
void tml_reset(int handle);
int tml_send(int handle, char *pBuff, int buffLen);
int tml_receive(int handle, char *pBuff, int buffLen);
int tml_transceive(int handle, char *pTx, int TxLen, char *pRx, int RxLen);
