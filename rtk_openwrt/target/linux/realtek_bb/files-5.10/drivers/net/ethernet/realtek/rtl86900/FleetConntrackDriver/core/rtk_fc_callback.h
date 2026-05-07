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
*/

#ifndef __RTK_FC_CALLBACK__
#define __RTK_FC_CALLBACK__

#include <linux/netdevice.h>
#include <net/netfilter/nf_conntrack_tuple.h>
// VLAN


#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))// 8277C/9607F no need indMac
int rtk_fc_indMac_idx_get(unsigned int l2Idx, int *indMacIdx);
#endif
//int rtk_fc_netif_add_tempEntry(int swEntIdx);
//int rtk_fc_netif_add_by_flow(int swEntIdx);
int rtk_fc_netif_update_by_flow(uint8 swEntIdx,rtk_fc_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_fc_flow_direction_t dir,rtk_fc_dev_type_t devType);
int rtk_fc_netif_get_freeEntry(uint32 psIfidxStackKey,int16 *swNetifIdx);
int rtk_fc_devGwMac_update_and_add(struct net_device *dev,int updatePortBySw,int32 *index);
int rtk_fc_gwMac_lut_add( uint8* mac,int16* index,int isWanGwMac,int forceLearningPort,int macportidx,int macextportidx,int wlanidx);
int rtk_fc_netif_sw_del(int swEntIdx);
int rtk_fc_devGwMac_del(struct net_device *dev);
int rtk_fc_hwNetif_del_by_swNetif(int swEntIdx);

//int rtk_fc_netif_del(int swEntIdx);
int rtk_fc_tables_init(void);
int rtk_fc_tableReset(void);
int rtk_fc_eventHandler_inetAddr(void *ptr, unsigned long event);
int rtk_fc_eventHandler_inet6Addr(void *ptr, unsigned long event);
int rtk_fc_eventHandler_netDev(void *ptr, unsigned long event);
void rtk_fc_netif_port_info_set(int swEntIdx, char macportidx, char macextportidx, char wlanidx);




#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

int rtk_fc_rtl9607c_API_lut_add(void* input_data);
int rtk_fc_rtl9607c_API_lut_del(void* input_data);
int rtk_fc_rtl9607c_API_lut_query(unsigned char *mac, int *lutIdx);
int rtk_fc_rtl9607c_API_init(void);

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
int rtk_fc_ca_API_lut_add(void* input_data);
int rtk_fc_ca_API_lut_del(void* input_data);
int rtk_fc_ca_API_lut_query(unsigned char *mac, int *lutIdx);
int rtk_fc_ca_API_init(void);
int rtk_fc_pf_exit(void);
int rtk_fc_flow_tuple_init(void);


#endif


#endif // __RTK_FC_CALLBACK__

