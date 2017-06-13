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

static char gNfcController_generation = 0;

static void RfOn (int handle)
{
    char NCIStartDiscovery[] = {0x21, 0x03, 0x09, 0x04, 0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x06, 0x01};
    char NCIRfOn[] = {0x2F, 0x3D, 0x02, 0x20, 0x01};
    char Answer[256];

    if (gNfcController_generation == 1) {
        printf("Continuous RF ON test - please tap a card ...\n");
        tml_transceive(handle, NCIStartDiscovery, sizeof(NCIStartDiscovery), Answer, sizeof(Answer));
        if((Answer[0] != 0x41) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) {
            printf("Cannot start discovery loop\n");
            return;
        }
        do {
            tml_receive(handle,  Answer, sizeof(Answer));
        } while ((Answer[0] != 0x61) || ((Answer[1] != 0x05) && (Answer[1] != 0x03)));
    }
    else {
        printf("Continuous RF ON test\n");
        tml_transceive(handle, NCIRfOn, sizeof(NCIRfOn), Answer, sizeof(Answer));
    }
    printf("NFC Controller is now in continuous RF ON mode - Press enter to stop\n");
    fgets(Answer, sizeof(Answer), stdin);
}

static void Prbs (int handle)
{
    char NCIPrbsPN7120[] = {0x2F, 0x30, 0x04, 0x00, 0x00, 0x01, 0x01};
    char NCIPrbsPN7150[] = {0x2F, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01};
    char Answer[256];
    int tech, bitrate;

    printf("PRBS test:\n");
    printf(" Select technology (A=0, B=1, F=2: ");
    scanf("%d", &tech);
    printf(" Select bitrate (106=0, 212=1, 424=2, 848=3: ");
    scanf("%d", &bitrate);

    if (gNfcController_generation == 1) {
        NCIPrbsPN7120[3] = tech;
        NCIPrbsPN7120[4] = bitrate;
        tml_transceive(handle, NCIPrbsPN7120, sizeof(NCIPrbsPN7120), Answer, sizeof(Answer));
    }
    else {
        NCIPrbsPN7150[5] = tech;
        NCIPrbsPN7150[6] = bitrate;
        tml_transceive(handle, NCIPrbsPN7150, sizeof(NCIPrbsPN7150), Answer, sizeof(Answer));
    }
    printf("NFC Controller is now in PRBS mode - Press enter to stop\n");
    fgets(Answer, sizeof(Answer), stdin);
}

static void Standby (int handle)
{
    char NCIEnableStandby[] = {0x2F, 0x00, 0x01, 0x01};
    char Answer[256];

    tml_transceive(handle, NCIEnableStandby, sizeof(NCIEnableStandby), Answer, sizeof(Answer));
    if((Answer[0] != 0x4F) || (Answer[1] != 0x00) || (Answer[3] != 0x00)) {
        printf("Cannot set the NFC Controller in standby mode\n");
        return;
    }
    
    /* Wait to allow PN71xx entering the standby mode */
    usleep(500 * 1000);

    printf("NFC Controller is now in standby mode - Press enter to stop\n");
    fgets(Answer, sizeof(Answer), stdin);
}

int main()
{
    int nHandle;
    char NCICoreReset[] = {0x20, 0x00, 0x01, 0x01};
    char NCICoreInit[] = {0x20, 0x01, 0x00};
    char NCIDisableStandby[] = {0x2F, 0x00, 0x01, 0x00};
    char Answer[256];
    int NbBytes = 0;
    
    printf("\n----------------------------\n");
    printf("NFC Factory Test Application\n");
    printf("----------------------------\n");

    if(tml_open(&nHandle) != 0) {
        printf("Cannot connect to PN71xx NFC controller\n");
        return -1;
    }

    tml_reset(nHandle);
    tml_transceive(nHandle, NCICoreReset, sizeof(NCICoreReset), Answer, sizeof(Answer));
    
    NbBytes = tml_transceive(nHandle, NCICoreInit, sizeof(NCICoreInit), Answer, sizeof(Answer));
    if((NbBytes < 19) || (Answer[0] != 0x40) || (Answer[1] != 0x01) || (Answer[3] != 0x00))    {
        printf("Error communicating with PN71xx NFC Controller\n");
        return -1;
    }
    /* Retrieve NXP-NCI NFC Controller generation */
    if (Answer[17+Answer[8]] == 0x08) {
        printf("PN7120 NFC controller detected\n");
        gNfcController_generation = 1;
    }
    else if (Answer[17+Answer[8]] == 0x10) {
        printf("PN7150 NFC controller detected\n");
        gNfcController_generation = 2;
    }
    else {
        printf("Wrong NFC controller detected\n");
        return -1;
    }

    tml_transceive(nHandle, NCIDisableStandby, sizeof(NCIDisableStandby), Answer, sizeof(Answer));
    
    printf("Select the test to run:\n");
    printf("\t 1. Continuous RF ON\n");
    printf("\t 2. PRBS\n");
    printf("\t 3. Standby mode\n");
    printf("Your choice: ");
    scanf("%d", &NbBytes);
    
    switch(NbBytes)    {
        case 1: RfOn(nHandle);    break;
        case 2: Prbs(nHandle);     break;
        case 3: Standby(nHandle);     break;
        default: printf("Wrong choice\n");    break;
    }

    fgets(Answer, sizeof(Answer), stdin);
    
    tml_reset(nHandle);
    tml_close(nHandle);
    
    return 0;
}
