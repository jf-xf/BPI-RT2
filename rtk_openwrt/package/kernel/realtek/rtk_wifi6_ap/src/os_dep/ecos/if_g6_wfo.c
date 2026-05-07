 /******************************************************************************
  *
  * Name: if_819x_wlan.c - Realtek 819x wlan driver
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#include <pkgconf/system.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/eth/netdev.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <linux/if_ether.h>

#include "if_g6_wfo.h"
#include "pci_98d_wfo.h"

static bool rtk_g6_wlan_init(struct cyg_netdevtab_entry *tab);
static void rtk_g6_wlan_start(struct eth_drv_sc *sc, unsigned char *enaddr,
                           int flags);
static void rtk_g6_wlan_stop(struct eth_drv_sc *sc);
static int rtk_g6_wlan_control(struct eth_drv_sc *sc, unsigned long key,
                            void *data, int   data_length);
static int rtk_g6_wlan_can_send(struct eth_drv_sc *sc);
static void rtk_g6_wlan_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len, int total_len, unsigned long key);
static void rtk_g6_wlan_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
                          int sg_len);
static void rtk_g6_wlan_deliver(struct eth_drv_sc *sc);
static void rtk_g6_wlan_poll(struct eth_drv_sc *sc);
static int rtk_g6_wlan_int_vector(struct eth_drv_sc *sc);

static bool rtk_g6_wlan_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc;
	Rltk819x_t *g6_info;
	extern void setup_wifi_module_param(void);

	setup_wifi_module_param();

	sc = (struct eth_drv_sc *)(tab->device_instance);
	g6_info = (Rltk819x_t *)(sc->driver_private);
	g6_info->sc = sc;

	if (g6_info->device_num == 0) {
		rtk_wfo_pcie_init();
	}

	/* Initialize upper level driver */
	g6_info->dev = (struct net_device *)rtk_wfo_get_net_dev(g6_info->device_num);
	rtk_wfo_get_mac(g6_info->mac, g6_info->device_num);
	if (g6_info->device_num == 0) {
		rtk_wfo_get_irq(&g6_info->vector);
	}

	g6_info->dev->info = (void *)g6_info;

	(sc->funs->eth_drv->init)(sc, g6_info->mac);

	return true;
}

static void rtk_g6_wlan_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
	Rltk819x_t *g6_info = (Rltk819x_t *)(sc->driver_private);
	struct net_device *dev = g6_info->dev;

	if (dev->flags & IFF_UP)
		return;

	dev->netdev_ops->ndo_open(dev);
	dev->flags |= IFF_UP;
}

static void rtk_g6_wlan_stop(struct eth_drv_sc *sc)
{
	Rltk819x_t *g6_info = (Rltk819x_t *)(sc->driver_private);
	struct net_device *dev = g6_info->dev;

	if (dev->flags & IFF_UP) {
		dev->netdev_ops->ndo_stop(dev);
	}

	dev->flags &= ~IFF_UP;
}

static int rtk_g6_wlan_control(struct eth_drv_sc *sc, unsigned long key, void *data,
                 int data_length)
{
	Rltk819x_t *g6_info = (Rltk819x_t *)(sc->driver_private);
	struct net_device *dev = g6_info->dev;
	struct sockaddr sa;

	if (key >= SIOCDEVPRIVATE && key <= SIOCDEVPRIVATE + 15)
		return dev->netdev_ops->ndo_do_ioctl(dev, data, key);

	switch (key) {
	case ETH_DRV_SET_MAC_ADDRESS:
		if (ETH_ALEN != data_length)
			return 1;
		memcpy(sa.sa_data, data, ETH_ALEN);
		dev->netdev_ops->ndo_set_mac_address(dev, &sa);
		memcpy(g6_info->mac, data, ETH_ALEN);
		return 0;

#ifdef ETH_DRV_GET_MAC_ADDRESS
	case ETH_DRV_GET_MAC_ADDRESS:
		if (ETH_ALEN != data_length)
			return 1;
		memcpy(data, g6_info->mac, ETH_ALEN);
		return 0;
#endif
	case ETH_DRV_GET_IF_STATS_UD:
	case ETH_DRV_GET_IF_STATS:
	case ETH_DRV_SET_MC_ALL:
	default:
		return 1;
	}

	return 1;
}

static int rtk_g6_wlan_can_send(struct eth_drv_sc *sc)
{
	/* It shall return number of packets which could be accepted at this time
	 * but we return 1 at early develop stage need to be refine in the future
	 */
	return 1;
}

static void rtk_g6_wlan_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len,
              int total_len, unsigned long key)
{
	Rltk819x_t *g6_info = (Rltk819x_t *)(sc->driver_private);
	struct net_device *dev = g6_info->dev;
	struct sk_buff *skb = NULL;
	struct eth_drv_sg *last_sg;

	cyg_drv_isr_lock();

	skb = dev_alloc_skb(total_len);
	if (skb == NULL)
		goto free_ret;

	for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
		memcpy(skb->tail, (void *)(sg_list->buf), sg_list->len);
		skb_put(skb,  sg_list->len);
	}
	skb->dev = dev;
#if 0
	/* FIXME , how to get real skb length ? */
	int i, skb_len = 64;
	unsigned char *data;

	data = (unsigned char *)skb->data;
	for (i=0; (i < skb_len); i++) {
		if (i%16 ==0)
			diag_printf("\n%02X ", data [i]);
		else
			diag_printf("%02X ", data [i]);
	}
	diag_printf("\n");
#endif

	dev->netdev_ops->ndo_start_xmit(skb, dev);

free_ret:
	(g6_info->sc->funs->eth_drv->tx_done)(g6_info->sc, key, 0);

	cyg_drv_isr_unlock();
}

static void rtk_g6_wlan_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
              int sg_len)
{
	Rltk819x_t *g6_info = (Rltk819x_t *)(sc->driver_private);
	struct net_device *dev = g6_info->dev;
	struct sk_buff *skb = (struct sk_buff *)g6_info->skb;
	struct eth_drv_sg *last_sg;

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
		struct mbuf *m;

		m = (struct mbuf *)sg_list[0].buf;
		if (m) {
#if defined(CONFIG_RTL_8198D_TAROKO)
            rtl8198d_wfo_wlandev2portidx(skb->dev, &m->m_hdr.macportid, &m->m_hdr.macextportid);
#endif
#if 0
			//if the packet is small, copy it to mbuf and free cluster
			if (skb->len < MHLEN)
			{
				memcpy(m->m_pktdat, skb->data, skb->len);
				sg_list[0].buf = (CYG_ADDRESS)m->m_pktdat;
				sg_list[0].len = sizeof(struct ether_header);
				sg_list[1].buf = (caddr_t)m->m_pktdat+sizeof(struct ether_header);
				sg_list[1].len = skb->len-sizeof(struct ether_header);
				m->m_data = (caddr_t)sg_list[1].buf;
				m->m_len = sg_list[1].len;
				MCLFREE(skb->head);
				skb->head = NULL;
			}
			else
#endif
			{
				sg_list[0].buf = (CYG_ADDRESS)skb->data;
				sg_list[0].len = sizeof(struct ether_header);
				skb_pull(skb, sg_list[0].len);
				sg_list[1].buf = (CYG_ADDRESS)skb->data;
				sg_list[1].len = skb->len;
	
				m->m_ext.ext_buf = (caddr_t)skb->head;
				m->m_data = (caddr_t)sg_list[1].buf;
				m->m_len = sg_list[1].len;
				m->m_flags |= M_EXT;
				m->m_ext.ext_size = MCLBYTES;
				m->m_ext.ext_free = NULL;
				m->m_ext.ext_ref = NULL;
#ifdef CYGPKG_NET_OPENBSD_STACK
				m->m_ext.ext_handle = NULL;
#endif
			}
#if 0
			diag_printf("------------start------------------\n");
			diag_printf("sg_list[0].buf=%p\n", sg_list[0].buf);
				diag_printf("sg_list[0].len=%d\n", sg_list[0].len);
				diag_printf("sg_list[1].buf=%p\n", sg_list[1].buf);
				diag_printf("sg_list[1].len=%d\n", sg_list[1].len);
				diag_printf("m=%p\n", m);
				diag_printf("m->m_ext.ext_buf=%p\n", m->m_ext.ext_buf);
				diag_printf("m->m_ext.ext_size=%d\n", m->m_ext.ext_size);
				diag_printf("m->m_data=%p\n", m->m_data);
				diag_printf("m->m_len=%d\n", m->m_len);
				diag_printf("------------end---------------------\n");
				
	
				diag_printf("m->m_data=%p\n", m->m_data);
				diag_printf("m->m_len=%d + 14\n", m->m_len);
					{
					int i;
					unsigned char *data;
					data = (unsigned char *)skb->data -14;
					for (i=0;(i<m->m_len+14);i++){
						if (i%16 ==0)
						diag_printf("\n%02X ", data [i]);
						else
							diag_printf("%02X ", data [i]);
	
						}
			}
#endif
		}
		else {
			//no mbuf available => drop the packet
			dev_kfree_skb_any(skb);
		}
#else
	for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
		if (sg_list->buf != NULL) {
			memcpy((void *)(sg_list->buf), skb->data, sg_list->len);
			skb_pull(skb, sg_list->len);
		}
	}
#endif
}

static void rtk_g6_wlan_deliver(struct eth_drv_sc *sc)
{
	/* We switch to soft IRQ mechanism (CONFIG_SOFTIRQ_CALLED_FROM_ALARM_THREAD)
	 * so this shall never be called.
	 * We return directly
	 */
	return;
}

static void rtk_g6_wlan_poll(struct eth_drv_sc *sc)
{
	rtk_wfo_poll_irq();
	rtk_g6_wlan_deliver(sc);
}

static int rtk_g6_wlan_int_vector(struct eth_drv_sc *sc)
{
	return ((Rltk819x_t *)(sc->driver_private))->vector;
}

static Rltk819x_t rtk_g6_wifi_priv_data = { 0, 0 };
ETH_DRV_SC(rtk_g6_wlan_sc0,
		&rtk_g6_wifi_priv_data,
		"wlan1",
		rtk_g6_wlan_start,
		rtk_g6_wlan_stop,
		rtk_g6_wlan_control,
		rtk_g6_wlan_can_send,
		rtk_g6_wlan_send,
		rtk_g6_wlan_recv,
		rtk_g6_wlan_deliver,
		rtk_g6_wlan_poll,
		rtk_g6_wlan_int_vector
);

NETDEVTAB_ENTRY(rtk_g6_wlan0_netdev0,
		"rtk_g6_wlan1",
		rtk_g6_wlan_init,
		&rtk_g6_wlan_sc0
);




#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
#include "rtw_pltfm_postconf.h"

static bool rtk_g6_wlan_init_dummy(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)(tab->device_instance);
	printk("%s(): dev[%s] shouldn't be run, please check!!\n", __func__, sc->dev_name);
	return true;
}

//----- wlan1-vap0 -------------------------------------------------------------
#if (CONFIG_LIMITED_VAP_NUM>0)
static Rltk819x_t rtk_g6_wlan1_vap0_priv_data = { 1, 0 };
ETH_DRV_SC(rtk_g6_wlan1_vap0_sc,
		&rtk_g6_wlan1_vap0_priv_data,
		"wlan1-vap0",
		rtk_g6_wlan_start,
		rtk_g6_wlan_stop,
		rtk_g6_wlan_control,
		rtk_g6_wlan_can_send,
		rtk_g6_wlan_send,
		rtk_g6_wlan_recv,
		rtk_g6_wlan_deliver,
		rtk_g6_wlan_poll,
		rtk_g6_wlan_int_vector
);

cyg_netdevtab_entry_t rtk_g6_wlan1_vap0_netdev0 = {
	.name = "rtk_g6_wlan1-vap0",
	.init = rtk_g6_wlan_init_dummy,
	.device_instance = &rtk_g6_wlan1_vap0_sc,
};
#endif

//----- wlan1-vap1 -------------------------------------------------------------
#if (CONFIG_LIMITED_VAP_NUM>1)
static Rltk819x_t rtk_g6_wlan1_vap1_priv_data = { 2, 0 };
ETH_DRV_SC(rtk_g6_wlan1_vap1_sc,
		&rtk_g6_wlan1_vap1_priv_data,
		"wlan1-vap1",
		rtk_g6_wlan_start,
		rtk_g6_wlan_stop,
		rtk_g6_wlan_control,
		rtk_g6_wlan_can_send,
		rtk_g6_wlan_send,
		rtk_g6_wlan_recv,
		rtk_g6_wlan_deliver,
		rtk_g6_wlan_poll,
		rtk_g6_wlan_int_vector
);

cyg_netdevtab_entry_t rtk_g6_wlan1_vap1_netdev0 = {
	.name = "rtk_g6_wlan1-vap1",
	.init = rtk_g6_wlan_init_dummy,
	.device_instance = &rtk_g6_wlan1_vap1_sc,
};
#endif

//----- wlan1-vap2 -------------------------------------------------------------
#if (CONFIG_LIMITED_VAP_NUM>2)
static Rltk819x_t rtk_g6_wlan1_vap2_priv_data = { 3, 0 };
ETH_DRV_SC(rtk_g6_wlan1_vap2_sc,
		&rtk_g6_wlan1_vap2_priv_data,
		"wlan1-vap2",
		rtk_g6_wlan_start,
		rtk_g6_wlan_stop,
		rtk_g6_wlan_control,
		rtk_g6_wlan_can_send,
		rtk_g6_wlan_send,
		rtk_g6_wlan_recv,
		rtk_g6_wlan_deliver,
		rtk_g6_wlan_poll,
		rtk_g6_wlan_int_vector
);

cyg_netdevtab_entry_t rtk_g6_wlan1_vap2_netdev0 = {
	.name = "rtk_g6_wlan1-vap2",
	.init = rtk_g6_wlan_init_dummy,
	.device_instance = &rtk_g6_wlan1_vap2_sc,
};
#endif

//----- wlan1-vap3 -------------------------------------------------------------
#if (CONFIG_LIMITED_VAP_NUM>3)
static Rltk819x_t rtk_g6_wlan1_vap3_priv_data = { 4, 0 };
ETH_DRV_SC(rtk_g6_wlan1_vap3_sc,
		&rtk_g6_wlan1_vap3_priv_data,
		"wlan1-vap3",
		rtk_g6_wlan_start,
		rtk_g6_wlan_stop,
		rtk_g6_wlan_control,
		rtk_g6_wlan_can_send,
		rtk_g6_wlan_send,
		rtk_g6_wlan_recv,
		rtk_g6_wlan_deliver,
		rtk_g6_wlan_poll,
		rtk_g6_wlan_int_vector
);

cyg_netdevtab_entry_t rtk_g6_wlan1_vap3_netdev0 = {
	.name = "rtk_g6_wlan1-vap3",
	.init = rtk_g6_wlan_init_dummy,
	.device_instance = &rtk_g6_wlan1_vap3_sc,
};
#endif

//----- wlan1-vxd -------------------------------------------------------------
#ifdef CONFIG_RTW_REPEATER_MODE_SUPPORT
static Rltk819x_t rtk_g6_wlan1_vxd_priv_data = { 5, 0 };
ETH_DRV_SC(rtk_g6_wlan1_vxd_sc,
		&rtk_g6_wlan1_vxd_priv_data,
		"wlan1-vxd",
		rtk_g6_wlan_start,
		rtk_g6_wlan_stop,
		rtk_g6_wlan_control,
		rtk_g6_wlan_can_send,
		rtk_g6_wlan_send,
		rtk_g6_wlan_recv,
		rtk_g6_wlan_deliver,
		rtk_g6_wlan_poll,
		rtk_g6_wlan_int_vector
);

cyg_netdevtab_entry_t rtk_g6_wlan1_vxd_netdev0 = {
	.name = "rtk_g6_wlan1-vxd",
	.init = rtk_g6_wlan_init_dummy,
	.device_instance = &rtk_g6_wlan1_vxd_sc,
};
#endif

//------------------------------------------------------------------------------
void rtk_g6_wlan_vap_init(unsigned int dev_idx)
{
	switch(dev_idx) {
	#if (CONFIG_LIMITED_VAP_NUM>0)
		case 1:
			rtk_g6_wlan_init(&rtk_g6_wlan1_vap0_netdev0);
			break;
	#endif
	#if (CONFIG_LIMITED_VAP_NUM>1)
		case 2:
			rtk_g6_wlan_init(&rtk_g6_wlan1_vap1_netdev0);
			break;
	#endif
	#if (CONFIG_LIMITED_VAP_NUM>2)
		case 3:
			rtk_g6_wlan_init(&rtk_g6_wlan1_vap2_netdev0);
			break;
	#endif
	#if (CONFIG_LIMITED_VAP_NUM>3)
		case 4:
			rtk_g6_wlan_init(&rtk_g6_wlan1_vap3_netdev0);
			break;
	#endif
	#ifdef CONFIG_RTW_REPEATER_MODE_SUPPORT
		case CONFIG_LIMITED_VAP_NUM:
			rtk_g6_wlan_init(&rtk_g6_wlan1_vxd_netdev0);
			break;
	#endif
		default:
			printk("%s(): not support! dev_idx(%d)\n", __func__, dev_idx);
			break;
	}
}

void rtk_g6_wlan_vap_deinit(unsigned int dev_idx)
{
	//printk("In %s, detach dev_idx(%d)\n", __func__, dev_idx);
	switch(dev_idx) {
	#if (CONFIG_LIMITED_VAP_NUM>0)
		case 1:
			ether_ifdetach(&rtk_g6_wlan1_vap0_sc.sc_arpcom.ac_if, 0);
			break;
	#endif
	#if (CONFIG_LIMITED_VAP_NUM>1)
		case 2:
			ether_ifdetach(&rtk_g6_wlan1_vap1_sc.sc_arpcom.ac_if, 0);
			break;
	#endif
	#if (CONFIG_LIMITED_VAP_NUM>2)
		case 3:
			ether_ifdetach(&rtk_g6_wlan1_vap2_sc.sc_arpcom.ac_if, 0);
			break;
	#endif
	#if (CONFIG_LIMITED_VAP_NUM>3)
		case 4:
			ether_ifdetach(&rtk_g6_wlan1_vap3_sc.sc_arpcom.ac_if, 0);
			break;
	#endif
	#ifdef CONFIG_RTW_REPEATER_MODE_SUPPORT
		case CONFIG_LIMITED_VAP_NUM:
			ether_ifdetach(&rtk_g6_wlan1_vxd_sc.sc_arpcom.ac_if, 0);
			break;
	#endif
		default:
			printk("%s(): not support! dev_idx(%d)\n", __func__, dev_idx);
			break;
	}
}
#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */
