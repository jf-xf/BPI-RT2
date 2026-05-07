/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 40647 $
 * $Date: 2013-07-01 15:36:16 +0800 (Mon, 01 Jul 2013) $
 *
 * Purpose : Main function of the EPON OAM protocol stack user application
 *           It create two additional threads for packet Rx and state control
 *
 * Feature : Start point of the EPON OAM protocol stack. Use individual threads
 *           for packet Rx and state control
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/time.h>

#include <common/error.h>
#include <rtk/ponmac.h>
#if defined(CONFIG_PON_AUTO_SELECT_FEATURE)
//#include <LINUX/mib.h>
#endif
#include <rtk/rt/rt_ponmisc.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */
#define RESET_CDR_TIMES  1

/*
 * Function Declaration
 */
int
main(
    int argc,
    char *argv[])
{
#if defined(CONFIG_PON_DETECT_FEATURE)
    rtk_ponmac_mode_t check_mode, set_mode;
    unsigned int sd, sync;
    unsigned char buf[32];
    char *strMode=NULL;
    FILE *pFD;
    int reset_cnt=0;
    int ret;
    int i;
    rt_ponmisc_prbs_t prbsRxCfg=0, prbsTxCfg=0;

    if(argc<2)
    {
        printf("pon auto detect: get pon mode from input parameter fail");
        return 0;
    }
    
    /*1 and 2 is reference to runsdk.sh*/
    switch(atoi(argv[1]))
    {
        case 1:
            check_mode = PONMAC_MODE_GPON;
            break;
        case 2:
            check_mode = PONMAC_MODE_EPON;
            break;
        default:
            printf("pon auto detect: wrong pon mode fail\n\r");
            return 0;
    }

    sleep(5);/* wait 5 sec for PON_MAC ready */
    while(1)
    {
        sleep(2);
        /* check PRBS TX and RX enable */
        if((ret = rt_ponmisc_forcePRBS_get(&prbsTxCfg))!= RT_ERR_OK)
        {
            prbsTxCfg = RT_PONMISC_PRBS_OFF;
        }

        if((ret = rt_ponmisc_forcePRBS_rx_get(&prbsRxCfg))!= RT_ERR_OK)
        {
            prbsRxCfg = RT_PONMISC_PRBS_OFF;
        }

        if((prbsTxCfg != RT_PONMISC_PRBS_OFF) || (prbsRxCfg != RT_PONMISC_PRBS_OFF))
            continue;

        /* get pon link state */
        if((ret = rtk_ponmac_linkState_get(check_mode, &sd, &sync))!= RT_ERR_OK)
        {
            printf("pon auto detect: get link state faile 0x%x\n\r", ret);
            return 0;
        }
        /*printf("pon auto detect: pon mode=%d, get sd=%d, sync=%d\n\r",mib_pon_mode, sd, sync);*/
        if((sd == 0) && (sync == 0)) /* no link */
        {
            /*printf("pon auto detect: pon mode detect no link \n\r");*/
            reset_cnt = 0;
        }
        else
        if((sd == 1) && (sync == 0)) /* wrong pon mode */
        {
#if defined(CONFIG_PON_AUTO_SELECT_FEATURE)
            if(reset_cnt > RESET_CDR_TIMES)
                break;
#endif
            /* serdes cdr reset for re-lock */
            if((ret = rtk_ponmac_serdesCdr_reset()) != RT_ERR_OK)
            {
                printf("pon auto detect: serdes reset fail 0x%x\n\r", ret);
            }
            printf("pon auto detect: sd=1, sync=0, reset-cdr \n\r");
            reset_cnt++;
        }
        else
        if((sd == 1) && (sync == 1)) /* correct pon mode */
        {
            /*printf("pon auto detect: pon mode detect correct mode %d \n\r", check_mode);*/
            reset_cnt = 0;
        }
        else /* strange state */
        {
            printf("pon auto detect: pon mode detect strange!!! sd=%d, sync=%d!!\n\r", sd, sync);
        }
    }

#if defined(CONFIG_PON_AUTO_SELECT_FEATURE)
    printf("pon auto detect: pon mode detect wrong mode \n\r");
    if(check_mode == PONMAC_MODE_GPON)
    {
        set_mode = 2;//EPON_MODE;
    }
    else
    {
        set_mode = 1;//GPON_MODE;
    }

    printf("pon auto detect: MIB_PON_MODE set to mode=%d, and then reboot\n\r", set_mode);
    /* set pon mode */
    memset(buf, 0, 32);
    sprintf(buf, "mib set PON_MODE %d", set_mode);
    system(buf);
    memset(buf, 0, 32);
    sprintf(buf, "mib commit");
    system(buf);

    /* reboot device */
    system("reboot -f");
#endif
#endif
    return 0;
}

