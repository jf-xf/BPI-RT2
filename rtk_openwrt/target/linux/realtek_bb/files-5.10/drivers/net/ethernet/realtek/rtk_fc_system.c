#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#ifdef CONFIG_RTL8686_SWITCH
#include "rtk/stat.h"
#endif
#ifdef CONFIG_RTL8686NIC
#ifdef CONFIG_RTL9607C_SERIES
#include "re8686_rtl9607c.h"
#endif
#elif defined(CONFIG_LUNA_G3_SERIES)
#include "ca_ne_autoconf.h"
#include "ca_ni.h"
#endif
#ifdef CONFIG_COMMON_RT_API
#include "rt_stat.h"
#include "rt_stat_ext.h"
#endif
#include <common/error.h>

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
static int is_rtk_nic_dev(struct net_device *dev)
{
	u32 flags;

	flags = rtk_netdev_get_flags(dev);
	if((flags&RTK_IFF_DOMAIN_ELAN) && netdev_priv(dev)
		&& !(flags&RTK_IFF_VSMUX)
	){
		return 1;
	}
#if defined(CONFIG_RTK_DEV_AP)
	else if((flags&RTK_IFF_DOMAIN_WAN) && netdev_priv(dev)
			&& !(flags&RTK_IFF_VSMUX)
	){
		return 1;
	}
#endif
	return 0;
}
#endif

#if defined(CONFIG_RTL_MULTI_ETH_WAN)
static int is_rtk_wan_dev(struct net_device *dev)
{
	u32 flags = rtk_netdev_get_flags(dev);
	if(((flags&RTK_IFF_DOMAIN_WAN) || strstr(dev->name, "ppp"))
		&& (dev->flags & IFF_UP) && netif_carrier_ok(dev)
	){
		return 1;
	}

	return 0;
}
#endif

int rtk_dev_update_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
#ifdef CONFIG_COMMON_RT_API
 	rt_stat_port_cntr_t pPortCntrs;
#endif

#ifdef CONFIG_RTL_MULTI_LAN_DEV
	int phyPortId = -1;

	if(is_rtk_nic_dev(dev))
	{
#ifdef CONFIG_RTL8686NIC
		unsigned int i, txPortMask = 0;
		txPortMask = ((struct re_dev_private*)netdev_priv(dev))->txPortMask;
		for(i=0 ; i<10 ; i++)
		{
			if( txPortMask & (1<<i)){
				phyPortId = i;
				break;
			}
		}
#elif defined(CONFIG_LUNA_G3_SERIES)
		phyPortId = ((struct ca_eth_private*)netdev_priv(dev))->port_cfg.tx_ldpid;
#endif
		if(phyPortId != -1)
		{
#ifdef CONFIG_COMMON_RT_API
			if (rt_stat_port_getAll(phyPortId, &pPortCntrs) == RT_ERR_OK)
			{
				//memset(stats, 0, sizeof(*stats));
				stats->rx_bytes = (unsigned long long) pPortCntrs.ifInOctets;
				stats->rx_packets = (unsigned long long) (pPortCntrs.ifInUcastPkts+pPortCntrs.ifInMulticastPkts+pPortCntrs.ifInBroadcastPkts);
				stats->rx_errors += (unsigned long long) (pPortCntrs.dot3StatsAligmentErrors+pPortCntrs.dot3StatsFCSErrors+pPortCntrs.dot3StatsSymbolErrors+pPortCntrs.etherStatsCRCAlignErrors);
				stats->rx_dropped += (unsigned long long) (pPortCntrs.ifInDiscards);
				stats->rx_length_errors += (unsigned long long) (pPortCntrs.etherStatsRxUndersizePkts+pPortCntrs.etherStatsRxOversizePkts+pPortCntrs.etherStatsCRCAlignErrors);
				stats->multicast = (unsigned long long) pPortCntrs.ifInMulticastPkts;
				stats->tx_bytes = (unsigned long long) pPortCntrs.ifOutOctets;
				stats->tx_packets = (unsigned long long) (pPortCntrs.ifOutUcastPkts+pPortCntrs.ifOutMulticastPkts+pPortCntrs.ifOutBrocastPkts);
				stats->tx_errors += (unsigned long long) pPortCntrs.etherStatsTxCRCAlignErrors;
				stats->tx_dropped += (unsigned long long) pPortCntrs.ifOutDiscards;
				stats->collisions += (unsigned long long) (pPortCntrs.dot3StatsSingleCollisionFrames+pPortCntrs.dot3StatsMultipleCollisionFrames);
				return 0;
			}
#endif
		}
		return -1;
	}
#endif
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	if(is_rtk_wan_dev(dev))
	{
#ifdef CONFIG_COMMON_RT_API
		rt_stat_netif_mib_t netifMibCnt = {0};
		if(rt_stat_netifMib_get(dev->name, &netifMibCnt) == RT_ERR_OK)
		{
			//memset(stats, 0, sizeof(*stats));
			stats->rx_bytes += (unsigned long long) netifMibCnt.ingress_byte_count;
			stats->rx_packets += (unsigned long long) netifMibCnt.ingress_packet_count;
			stats->tx_bytes += (unsigned long long) netifMibCnt.egress_byte_count;
			stats->tx_packets += (unsigned long long) netifMibCnt.egress_packet_count;
			return 0;
		}
		else
			return -1;
#endif
	}
#endif
	return -1;
}

EXPORT_SYMBOL(rtk_dev_update_stats);
#endif