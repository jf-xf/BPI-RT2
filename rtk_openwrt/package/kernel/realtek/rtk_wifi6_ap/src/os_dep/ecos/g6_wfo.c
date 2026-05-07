#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <linux/if_ether.h>
#if defined(WFO_RADIO_SENDER)
#include <wfo/wfo_cmd_radio.h>
#endif

#ifdef CONFIG_RTW_A4_STA
int check_srcmac_in_fdb_for_ax_driver(struct net_device *dev, const unsigned char *addr)
{
	/*
		do check at FleetConntrackDriver/src/rtk_fc_helper_ps.c
		rtk_fc_skb_netif_rx() -> __wfo_check_srcmac_for_ax_driver()
	*/
	return 0; 
}

int arc4random(void) /* copy from ecos3.0 packages/net/bsd_tcpip/v3_0/src/ecos/support.c */
{
	cyg_uint32 res = 0;
	static unsigned long seed = 0xDEADB00B;
	HAL_CLOCK_READ(&res);  // Not so bad... (but often 0..N where N is small)
	seed = ((seed & 0x007F00FF) << 7) ^
		((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
		(res << 13) ^ (res >> 9);    // using the clock too!
	return (int)seed;
}

void get_random_bytes(void *buf, size_t len) /* copy from ecos3.0 packages/net/bsd_tcpip/v3_0/src/ecos/support.c */
{
	unsigned long ranbuf, *lp;
	lp = (unsigned long *)buf;

	while (len > 0) {
		ranbuf = arc4random();
		*lp++ = ranbuf;
		len -= sizeof(ranbuf);
	}
}

#endif /* CONFIG_RTW_A4_STA */

#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TX_GMAC_AUTO_SEL_SUPPORT)
int rtk_fc_external_lut_process(bool add, char *wlan_devname, char *sta_mac)
{
	notify_fc(wlan_devname, sta_mac, add);
	return 0;
}
#endif

int rtk_eventd_netlink_init(void)
{
	return 0;
}

int get_nl_eventd_pid(void)
{
	/*
		Return value can not be 0,
		It will check this value in rtk_wlan_event_indicate().
	 */
	return 1;
}

struct sock* get_nl_eventd_sk(void)
{
	/*
		Return value can not be NULL,
		It will check this value in rtk_wlan_event_indicate().
	 */
	return (struct sock*)1;
}
