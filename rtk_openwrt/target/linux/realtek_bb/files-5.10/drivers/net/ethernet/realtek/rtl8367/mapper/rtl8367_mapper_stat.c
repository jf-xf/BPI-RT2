/*
 * Copyright (C) 2021 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * Purpose : Definition of Statistic API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Statistic Counter Reset
 *           (2) Statistic Counter Get
 *
 */

#include <rtk_types.h>
#include <rtl8367_mapper_stat.h>
#include <stat.h>

#include <osal/memory.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <hal/common/halctrl.h>



/* Function Name:
 *      rtl8367_mapper_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */

int32
rtl8367_mapper_stat_port_reset(rtk_port_t port)
    {
    rtksw_port_t port_id;
    rtksw_api_ret_t retVal;

    //   printk("%s port:%d \n", __FUNCTION__, port);

#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    if ((retVal = rtksw_stat_port_reset(port_id)) != RT_ERR_OK)
        return (int32)retVal;


    // printk("%s pStatus:%d \n", __FUNCTION__, *pStatus);

    return RT_ERR_OK;

}



/* Function Name:
 *      rtl8367_mapper_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      port        - port id
 * Output:
 *      pPortCntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32
rtl8367_mapper_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPortCntrs)
{
    rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
    rtksw_stat_port_cntr_t port_cntrs;


    printk("%s port:%d \n", __FUNCTION__, port);
    
    
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortCntrs), RT_ERR_NULL_POINTER);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&port_cntrs, 0x00, sizeof(rtksw_stat_port_cntr_t));
    if ((retVal = rtksw_stat_port_getAll(port_id, &port_cntrs)) != RT_ERR_OK)
        return (int32)retVal;

    pPortCntrs->ifInOctets                              = port_cntrs.ifInOctets  ;
    pPortCntrs->ifInUcastPkts                           = port_cntrs.ifInUcastPkts;
    pPortCntrs->ifInMulticastPkts                       = port_cntrs.ifInMulticastPkts;
    pPortCntrs->ifInBroadcastPkts                       = port_cntrs.ifInBroadcastPkts;
    pPortCntrs->ifInDiscards                            = port_cntrs.dot1dTpPortInDiscards;

    pPortCntrs->ifOutOctets                             = port_cntrs.ifOutOctets;
    pPortCntrs->ifOutDiscards                           = port_cntrs.ifOutDiscards;
    pPortCntrs->ifOutUcastPkts                          = port_cntrs.ifOutUcastPkts;
    pPortCntrs->ifOutMulticastPkts                      = port_cntrs.ifOutMulticastPkts;
    pPortCntrs->ifOutBrocastPkts                        = port_cntrs.ifOutBrocastPkts;
    pPortCntrs->dot1dBasePortDelayExceededDiscards      = port_cntrs.dot1dBasePortDelayExceededDiscards;
    pPortCntrs->dot1dTpPortInDiscards                   = port_cntrs.dot1dTpPortInDiscards;
    pPortCntrs->dot1dTpHcPortInDiscards                 = 0;
    pPortCntrs->dot3InPauseFrames                       = port_cntrs.dot3InPauseFrames;
    pPortCntrs->dot3OutPauseFrames                      = port_cntrs.dot3OutPauseFrames;
    pPortCntrs->dot3OutPauseOnFrames                    = port_cntrs.dot3OutPauseFrames;
    pPortCntrs->dot3StatsAligmentErrors                 = 0;
    pPortCntrs->dot3StatsFCSErrors                      = port_cntrs.dot3StatsFCSErrors;
    pPortCntrs->dot3StatsSingleCollisionFrames          = port_cntrs.dot3StatsSingleCollisionFrames;
    pPortCntrs->dot3StatsMultipleCollisionFrames        = port_cntrs.dot3StatsMultipleCollisionFrames;
    pPortCntrs->dot3StatsDeferredTransmissions          = port_cntrs.dot3StatsDeferredTransmissions;
    pPortCntrs->dot3StatsLateCollisions                 = port_cntrs.dot3StatsLateCollisions;
    pPortCntrs->dot3StatsExcessiveCollisions            = port_cntrs.dot3StatsExcessiveCollisions;
    pPortCntrs->dot3StatsFrameTooLongs                  = port_cntrs.etherStatsOversizePkts;
    pPortCntrs->dot3StatsSymbolErrors                   = port_cntrs.ifOutMulticastPkts;
    pPortCntrs->dot3ControlInUnknownOpcodes             = port_cntrs.dot3ControlInUnknownOpcodes;
    pPortCntrs->etherStatsDropEvents                    = port_cntrs.etherStatsDropEvents;
    
    /*Use IGMP REG when REG 0x13e0 bit[8]=1*/
    pPortCntrs->etherStatsTxOctets                      = port_cntrs.ifOutOctets;
    pPortCntrs->etherStatsTxUndersizePkts               = port_cntrs.inIgmpSpecificQuery;
    pPortCntrs->etherStatsTxOversizePkts                = port_cntrs.inIgmpGeneralQuery;
    pPortCntrs->etherStatsTxPkts64Octets                = port_cntrs.inMldLeaves;
    pPortCntrs->etherStatsTxPkts65to127Octets           = port_cntrs.inIgmpLeaves;
    pPortCntrs->etherStatsTxPkts128to255Octets          = port_cntrs.inMldChecksumError;
    pPortCntrs->etherStatsTxPkts256to511Octets          = port_cntrs.inIgmpChecksumError;
    pPortCntrs->etherStatsTxPkts512to1023Octets         = port_cntrs.inMldSpecificQuery;
    pPortCntrs->etherStatsTxPkts1024to1518Octets        = port_cntrs.inMldGeneralQuery;
    pPortCntrs->etherStatsTxPkts1519toMaxOctets         = 0;
    pPortCntrs->etherStatsTxBcastPkts                   = port_cntrs.ifOutBrocastPkts;
    pPortCntrs->etherStatsTxMcastPkts                   = port_cntrs.ifOutMulticastPkts;
    pPortCntrs->etherStatsTxFragments                   = 0;
    pPortCntrs->etherStatsTxJabbers                     = 0;
    pPortCntrs->etherStatsTxCRCAlignErrors              = 0;

    pPortCntrs->etherStatsRxUndersizePkts               = port_cntrs.etherStatsUndersizePkts;
    pPortCntrs->etherStatsRxUndersizeDropPkts           = 0;
    pPortCntrs->etherStatsRxOversizePkts                = port_cntrs.etherStatsOversizePkts;
    pPortCntrs->etherStatsRxPkts64Octets                = port_cntrs.etherStatsPkts64Octets;
    pPortCntrs->etherStatsRxPkts65to127Octets           = port_cntrs.etherStatsPkts65to127Octets;
    pPortCntrs->etherStatsRxPkts128to255Octets          = port_cntrs.etherStatsPkts128to255Octets;
    pPortCntrs->etherStatsRxPkts256to511Octets          = port_cntrs.etherStatsPkts256to511Octets;
    pPortCntrs->etherStatsRxPkts512to1023Octets         = port_cntrs.etherStatsPkts512to1023Octets;
    pPortCntrs->etherStatsRxPkts1024to1518Octets        = port_cntrs.etherStatsPkts1024toMaxOctets;
    pPortCntrs->etherStatsRxPkts1519toMaxOctets         = 0;
    pPortCntrs->inOampduPkts                            = port_cntrs.inOampduPkts;
    pPortCntrs->outOampduPkts                           = port_cntrs.outOampduPkts;

    pPortCntrs->etherStatsOctets                        = pPortCntrs->ifInOctets + pPortCntrs->ifOutOctets;
    pPortCntrs->etherStatsBcastPkts                     = pPortCntrs->ifInBroadcastPkts + pPortCntrs->ifOutMulticastPkts;
    pPortCntrs->etherStatsMcastPkts                     = pPortCntrs->ifInMulticastPkts + pPortCntrs->ifOutMulticastPkts;
    pPortCntrs->etherStatsUndersizePkts                 = pPortCntrs->etherStatsTxUndersizePkts + pPortCntrs->etherStatsRxUndersizePkts;
    pPortCntrs->etherStatsOversizePkts                  = pPortCntrs->etherStatsTxOversizePkts + pPortCntrs->etherStatsRxOversizePkts;
    pPortCntrs->etherStatsFragments                     = 0;
    pPortCntrs->etherStatsJabbers                       = 0;
    pPortCntrs->etherStatsCollisions                    = 0;
    pPortCntrs->etherStatsCRCAlignErrors                = 0;
    pPortCntrs->etherStatsPkts64Octets                  = pPortCntrs->etherStatsTxPkts64Octets + pPortCntrs->etherStatsRxPkts64Octets;
    pPortCntrs->etherStatsPkts65to127Octets             = pPortCntrs->etherStatsTxPkts65to127Octets + pPortCntrs->etherStatsRxPkts65to127Octets;
    pPortCntrs->etherStatsPkts128to255Octets            = pPortCntrs->etherStatsTxPkts128to255Octets + pPortCntrs->etherStatsRxPkts128to255Octets;
    pPortCntrs->etherStatsPkts256to511Octets            = pPortCntrs->etherStatsTxPkts256to511Octets + pPortCntrs->etherStatsRxPkts256to511Octets;
    pPortCntrs->etherStatsPkts512to1023Octets           = pPortCntrs->etherStatsTxPkts512to1023Octets + pPortCntrs->etherStatsRxPkts512to1023Octets;
    pPortCntrs->etherStatsPkts1024to1518Octets          = pPortCntrs->etherStatsTxPkts1024to1518Octets + pPortCntrs->etherStatsRxPkts1024to1518Octets;

                   
        return RT_ERR_OK;
}




