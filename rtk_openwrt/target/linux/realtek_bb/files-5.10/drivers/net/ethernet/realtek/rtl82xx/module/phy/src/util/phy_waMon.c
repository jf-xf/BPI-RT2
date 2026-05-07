/*
 * Copyright (C) 2009-2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Revision:
 * Date: 2018-06-15
 *
 * Purpose : PHY waMon module.
 *
 * Feature : This Module is used to monitor PHY status.
 *
 */


#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <osal/phy_osal.h>
#include <rtk/default.h>
#include <rtk/port.h>
#include <dal/dal_phy.h>
#include <dal/dal_waMon.h>
#include <hwp/hw_profile.h>
#include <hal/common/halctrl.h>


#define WAMON_NAME_LEN      16
typedef void (*waMon_function)(void *);


typedef struct dal_waMon_cb_s {
    osal_thread_t       thread_id;
    uint32              scan_interval_us;
    waMon_function      waMon_func;
    uint32              wa_change_sem;
    char                name[WAMON_NAME_LEN];
} dal_waMon_cb_t;


static void _dal_waMon1000_thread(void *pInput);

static dal_waMon_cb_t   pWaMon_cb[]=
    {
        {0,1000000,_dal_waMon1000_thread, 0, "waMon 1000"},
    };

uint32 waMon_enable = ENABLED;
uint32 pktBuf_watchdog_cnt[RTK_MAX_NUM_OF_UNIT]={0};
uint32 macSerdes_watchdog_cnt[RTK_MAX_NUM_OF_UNIT]={0};
uint32 phySerdes_watchdog_cnt[RTK_MAX_NUM_OF_UNIT]={0};
uint32 phy_watchdog_cnt[RTK_MAX_NUM_OF_UNIT]={0};
uint32 fiber_rx_watchdog_cnt[RTK_MAX_NUM_OF_UNIT]={0};

static uint32   waMon_init;
/* Indicate PHY was reset by ESD. PHY's base mac ID will be set to 1 */
rtk_portmask_t dal_waMon_phyEsdRstEvn[RTK_MAX_NUM_OF_UNIT];
rtk_port_phyReconfig_callback_t pPhyReconfig_cb;


/* Function Name:
 *      dal_waMon_phyReconfig_portMaskSet
 * Description:
 *      Send the need ReConfig port mask back to user.
 * Input:
 *      phyReconfig_callback    - callback function for link change
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *
 */
int32 dal_waMon_phyReconfig_portMaskSet(uint32 unit, rtk_port_t port)
{
    hal_control_t *pHal_ctrl;
    rtk_portmask_t portMask;
    int loop;
    int mask_len, base_id;
    phy_type_t phyType;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    osal_memset(&portMask, 0, sizeof(rtk_portmask_t));

    /* Check whether device is exist in lower layer(HAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    if((phyType = HWP_PHY_MODEL_BY_PORT(unit, port)) == RTK_PHYTYPE_INVALID)
    {
        return RT_ERR_FAILED;
    }
    base_id = HWP_PHY_BASE_MACID(unit, port);
    mask_len = HWP_PHY_BASE_PHY_MAX(unit, port);

    for(loop = base_id; loop < (base_id + mask_len); loop++)
                        RTK_PORTMASK_PORT_SET(portMask,loop);


    if(pPhyReconfig_cb != NULL)
        (pPhyReconfig_cb)(unit, &portMask);
    else
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "No CallBack for port %d\n",port);

    return RT_ERR_OK;
} /* end of dal_waMon_phyReconfig_portMaskSet */


/* Function Name:
 *      dal_waMon_phyEsdRstEvn_set
 * Description:
 *      set the PHY ESD reset event
 * Input:
 *      unit            - unit
 *      base_mac_id     - PHY's base MAC id for indicating the PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *
 */
int32
dal_waMon_phyEsdRstEvn_set(uint32 unit, rtk_port_t base_mac_id)
{
    if (unit >= RTK_MAX_NUM_OF_UNIT)
        return RT_ERR_UNIT_ID;

    if (base_mac_id >= RTK_MAX_PORT_PER_UNIT)
        return RT_ERR_PORT_ID;

    RT_INIT_CHK(waMon_init);

    RTK_PORTMASK_PORT_SET(dal_waMon_phyEsdRstEvn[unit], base_mac_id);
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_waMon_phyReconfig_register
 * Description:
 *      Register callback function for PHY need to reconfigure notification
 * Input:
 *      phyReconfig_callback    - callback function for reconfigure notification
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
int32 dal_waMon_phyReconfig_register(rtk_port_phyReconfig_callback_t phyReconfig_callback)
{

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == phyReconfig_callback), RT_ERR_NULL_POINTER);

    pPhyReconfig_cb = phyReconfig_callback;

    return RT_ERR_OK;
} /* end of dal_waMon_phyReconfig_register */

/* Function Name:
 *      dal_waMon_phyReconfig_unregister
 * Description:
 *      UnRegister callback function for PHY need to reconfigure notification
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *
 */
int32 dal_waMon_phyReconfig_unregister(void)
{
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    pPhyReconfig_cb = NULL;

    return RT_ERR_OK;
} /* end of dal_waMon_phyReconfig_unregister */

/* Function Name:
 *      _dal_waMon1000_thread
 * Description:
 *      1000 ms task
 * Input:
 *      None.
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
static void _dal_waMon1000_thread(void *pInput)
{
    uint32  unit = 0;
    dal_waMon_cb_t *pWaMon_info = (dal_waMon_cb_t *)pInput;

    /* forever loop */
    while (pWaMon_info->scan_interval_us != 0)
    {
        /* wait semaphore for workaround scan interval */
        phy_osal_sem_take(pWaMon_info->wa_change_sem, pWaMon_info->scan_interval_us);
        if (waMon_enable == DISABLED)
        {
            continue;
        }

        HWP_UNIT_TRAVS_LOCAL(unit)
        {
//#if defined(CONFIG_SDK_WAMON_LINK_CHANGE_PROCESS)
//            _dal_waMon_portLinkChange_process(unit);
//#endif /* CONFIG_SDK_WAMON_LINK_CHANGE_PROCESS */

//#if defined(CONFIG_SDK_WA_FIBER_RX_WATCHDOG)
//            dal_phy_fiberRx_watchdog(unit);
//#endif

#if defined(CONFIG_SDK_WA_PHY_WATCHDOG)
            dal_phy_handler(unit);
#endif  /* CONFIG_SDK_WA_PHY_WATCHDOG */
        }

    }

}


/* Function Name:
 *      dal_waMon_init
 * Description:
 *      Initial Workaround Monitor component
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
int32 dal_waMon_init(void)
{
    int i;

    /* init value */
    waMon_init = INIT_NOT_COMPLETED;

    /* create semaphore for sync, this semaphore is empty in beginning */
    for(i = 0; i < sizeof(pWaMon_cb) / sizeof(dal_waMon_cb_t); i++)
    {
        pWaMon_cb[i].wa_change_sem = phy_osal_sem_create(0);

        if (0 == pWaMon_cb[i].wa_change_sem){
            RT_INIT_ERR(RT_ERR_FAILED, (MOD_INIT|MOD_DAL), "workaround monitor semaphore create failed");
            return RT_ERR_FAILED;
        }
    }

    waMon_init = INIT_COMPLETED;

    return RT_ERR_OK;

}

/* Function Name:
 *      dal_waMon_enable
 * Description:
 *      Enable workaround monitor thread
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Note:
 *      When enable workaround monitor thread
 *
 */
int32 dal_waMon_enable(void)
{
    int i;

    /* create thread */
    for(i = 0; i < sizeof(pWaMon_cb) / sizeof(dal_waMon_cb_t); i++)
    {
        if ((pWaMon_cb[i].thread_id) != 0)
            continue;

        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "scan_interval_us=%u",
               pWaMon_cb[i].scan_interval_us);

        pWaMon_cb[i].thread_id = phy_osal_thread_create(pWaMon_cb[i].name, RTK_DEFAULT_WA_MON_STACK_SIZE, RTK_DEFAULT_WA_MON_THREAD_PRI
                                , (void *)pWaMon_cb[i].waMon_func, &pWaMon_cb[i]);
        if (0 == (pWaMon_cb[i].thread_id))
        {
            RT_ERR(pWaMon_cb[i].thread_id, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_THREAD_CREATE_FAILED;
        }
    }

    return RT_ERR_OK;
}

