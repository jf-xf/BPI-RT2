#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#if defined(CONFIG_RTL8686_SWITCH)
#include <common/error.h>
#include <common/util/rt_util.h>
#ifdef CONFIG_COMMON_RT_API
#include <rtk/rt/rt_switch.h>
#include <rtk/rt/rt_port.h>
#elif defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#include <rtk/ponmac.h>
#include <rtk/gpon.h>
#include <rtk/epon.h>
#endif
#include <rtk/switch.h>
#endif
#include "sockmark_define.h"
#include "rtk_interface.h"
#include "common.h"

const char *LANPHY_PORT_IFNAE[MAX_LANPHY_NUM] = {ALIASNAME_ETH0, ALIASNAME_ETH1, ALIASNAME_ETH2};
const char *LAN_PORT_IFNAE[MAX_LAN_NUM] = {ALIASNAME_ELAN0, ALIASNAME_ELAN1, ALIASNAME_ELAN2, ALIASNAME_ELAN3, 
											ALIASNAME_ELAN4, ALIASNAME_ELAN5, ALIASNAME_ELAN6, ALIASNAME_ELAN7, 
											ALIASNAME_ELAN8, ALIASNAME_ELAN9};
const char *WAN_PORT_IFNAME = ALIASNAME_NAS0;

#ifndef PON_VEIP_IF
#define PON_VEIP_IF "veip"
#endif
#ifndef PON_PPTP_IF
#define PON_PPTP_IF "pptp"
#endif
#ifndef PON_VEIP_IPHOST_IF
#define PON_VEIP_IPHOST_IF "veip0_"
#endif


int set_all_port_powerdown_state(unsigned int state)
{
#if defined(CONFIG_RTL8686_SWITCH)
	rtk_switch_devInfo_t devInfo;
	int port = 0;
	if(rtk_switch_deviceInfo_get(&devInfo) != RT_ERR_OK)
		printf("rtk_switch_deviceInfo_get failed: %s %d\n", __func__, __LINE__);

	for (port = devInfo.ether.min; port <= devInfo.ether.max; port++)
	{
		if(port>=32) break;

		if (devInfo.ether.portmask.bits[0] & 1<<port)
		{
			//All Port
			//set_port_powerdown_state(port, state);
			if(rtk_port_phyPowerDown_set(port, state) != RT_ERR_OK)
				printf("rtk_port_phyPowerDown_set failed: %s %d\n", __func__, __LINE__);
		}
	}
#endif
#if defined(CONFIG_EXTERNAL_PHY_POLLING)
	if (state == 0) { // phy power up
		rtk_mdio_c22_write(0, 0x1140);
	}
#endif
#if defined(CONFIG_RTL_8221B_SUPPORT)
	if (state == 0) { // phy power up
		system("/bin/echo power_up > /proc/rtl8221b/phy");
	}
#endif
	return 0;
}

int check_interface_exit(const char *ifname)
{
	char buf[64];
	struct stat st;
	snprintf(buf, sizeof(buf), "/sys/class/net/%s", ifname);
	if(stat(buf, &st) == 0)
		return 0;
	return -1;
}

int set_interfac_state(const char *ifname, int state)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "ifconfig %s %s", ifname, (state)?"up":"down");
	return system(buf);
}

int set_interfac_mac(const char *ifname, const char *mac, int index)
{
	char buf[64];
	unsigned long long u;
	unsigned char val[6], i;
	
	if(sscanf(mac, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5]) != 6 &&
	   sscanf(mac, "%02hhX-%02hhX-%02hhX-%02hhX-%02hhX-%02hhX", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5]) != 6 &&
	   sscanf(mac, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5]) != 6)
	{
		printf("Error MAC address !!\n");
		return -1;
	}
	if(index > 0)
	{
		u = 0;
		for (i = 0; i < 6; i++){ u = u << 8 | val[i]; }
		u = u + index;
		for (i = 5; i > 0; i--){ val[i] = u & 0xff; u = u >> 8 ; }
	}
	snprintf(buf, sizeof(buf), "ifconfig %s hw ether %02X%02X%02X%02X%02X%02X", ifname, 
									val[0], val[1], val[2], val[3], val[4], val[5]);
	return system(buf);
}

static int _port_get_phy_port(int logic_port)
{
	const char delim[] = ",";
	char buf[128], *ptr;
	unsigned char port_map_tbl[MAX_LAN_NUM];
	int i;

	if (logic_port < 0 || logic_port > MAX_LAN_NUM-1) {
		printf("\n%s:%d Invalid logic port ID %d\n", __FUNCTION__, __LINE__, logic_port);
		return -1;
	}

	if (!mib_get_s2(MIB_PORT_REMAPPING, (void *)buf, sizeof(buf)-1)) {
		printf("\n%s:%d mib_get_s PORT_REMAPPING fail!!!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	for(i=0; i<MAX_LAN_NUM; i++)
		port_map_tbl[i]=i;
	ptr = strtok(buf, delim);
	i=0;
	while ((ptr != NULL) && (i<MAX_LAN_NUM)) {
		port_map_tbl[i++] = atoi(ptr);
		ptr = strtok(NULL, delim);
	}

	return port_map_tbl[logic_port];
}

static int _port_set_dev_port_mapping(int port_id, const char *dev_name) {
	FILE* fp;
	char sysbuf[128];

	fp = fopen("/proc/eth_nic/dev_port_mapping", "r");
	if (fp)
	{
		fclose(fp);
		sprintf(sysbuf, "/bin/echo %d %s > /proc/eth_nic/dev_port_mapping", port_id, dev_name);
	}
	else
	{
#if defined(CONFIG_LUNA_G3_SERIES)
		sprintf(sysbuf, "/bin/echo %d %s > /proc/driver/cortina/ni/dev_port_mapping", port_id, dev_name);
#else
		sprintf(sysbuf, "/bin/echo %d %s > /proc/rtl8686gmac/dev_port_mapping", port_id, dev_name);
#endif
	}
	return system(sysbuf);
}

int rtk_port_get_wan_phyID(void)
{
	int wanPhyPort = -1;
	unsigned char pon_mode = 0;
	char buf[256]={0};
	
	if(mib_get_s2(MIB_PON_MODE, buf, sizeof(buf)))
		pon_mode = (unsigned char)atoi(buf);
	
	if(pon_mode == EPON_MODE || pon_mode == GPON_MODE || pon_mode == FIBER_MODE) {
		if(rt_switch_phyPortId_get(LOG_PORT_PON, &wanPhyPort) != 0)
			wanPhyPort = -1;	
	}
	else {
		if(mib_get_s2(MIB_WAN_PHY_PORT, buf, sizeof(buf)))
			wanPhyPort = (int)atoi(buf);
	}

	if(wanPhyPort < 0)
		wanPhyPort = -1;

	return wanPhyPort;
}

int setup_port(void)
{
	int i, SW_PORT_NUM = MAX_LAN_NUM;
	char mac[20] = "00E04C867001";

	mib_get_s2(MIB_ELAN_MAC_ADDR, mac, sizeof(mac));
	
#ifdef CONFIG_LAN_PORT_NUM
	if(CONFIG_LAN_PORT_NUM > 0 && CONFIG_LAN_PORT_NUM < SW_PORT_NUM)
		SW_PORT_NUM = CONFIG_LAN_PORT_NUM;
#endif

	// for NIC interface (eth0, eth1, eth2 - all use base MAC)
	for(i=0; i<MAX_LANPHY_NUM; i++)
	{
		const char *phyIf = LANPHY_PORT_IFNAE[i];
		if(check_interface_exit(phyIf) == 0)
		{
			set_interfac_state(phyIf, 0);
			set_interfac_mac(phyIf, mac, 0);  // NIC interfaces use base MAC
			set_interfac_state(phyIf, 1);
		}
	}
	
	// for LAN interface - give each port unique MAC (offset 2, 3, 4, ...)
	// WAN uses offset 1, so LAN ports start from offset 2
	for(i=0; i<MAX_LAN_NUM; i++)
	{
		const char *phyIf = LAN_PORT_IFNAE[i];
		if(check_interface_exit(phyIf) == 0)
		{
			int port;
			set_interfac_state(phyIf, 0);
			set_interfac_mac(phyIf, mac, i + 1);  // LAN ports: offset 1, 2, 3, ... (eth0.2=1, eth0.3=2)
			//set_interfac_state(phyIf, 1);

			// Only do port mapping for configured LAN ports (within SW_PORT_NUM)
			if(i < SW_PORT_NUM) {
				port = _port_get_phy_port(i);
				if (port >= 0) {
					_port_set_dev_port_mapping(port, phyIf);
				}
			}
		}
	}
	
	return 0;
}

int setup_wan_port(void)
{
	char mac[20] = "00E04C867001";
	const char *phyIf;
	char buf[512], ifname[32]; 
	unsigned char pon_mode = 0;
	unsigned int wan_port = 5;
	int i;
	
	mib_get_s2(MIB_ELAN_MAC_ADDR, mac, sizeof(mac));
	
	if(mib_get_s2(MIB_PON_MODE, buf, sizeof(buf)))
		pon_mode = (unsigned char)atoi(buf);

	if(mib_get_s2(MIB_WAN_PHY_PORT, buf, sizeof(buf)))
		wan_port = (unsigned int)atoi(buf);
	
	phyIf = WAN_PORT_IFNAME;
	printf("[setup_wan_port] WAN_PORT_IFNAME=%s, pon_mode=%d, wan_port=%u\n", phyIf, pon_mode, wan_port);

	if(check_interface_exit(phyIf) == 0)
	{
		set_interfac_state(phyIf, 0);
		/* Use MAC offset 1 for WAN interface to avoid "own address as source" warning
		 * when using bridge. This gives eth0.2 a different MAC than br-wan.
		 */
		set_interfac_mac(phyIf, mac, 1);
		//set_interfac_state(phyIf, 1);
		
		/* For physical Ethernet WAN (eth0.x), use ACCEPT policy to receive all packets.
		 * For PON interfaces (nas0), use DROP and rely on smux rules to selectively accept.
		 * Check interface name prefix to determine: eth* = physical Ethernet, nas* = PON
		 */
		if(strncmp(phyIf, "eth", 3) == 0 || pon_mode == ETH_MODE || pon_mode == FIBER_MODE) {
			snprintf(buf, sizeof(buf), "%s --if %s --set-if-rsmux --set-if-rx-policy ACCEPT "
										"--set-if-tx-policy CONTINUE --set-if-rxmc-policy ACCEPT "
										"--set-if-rx-multi 1", SMUXCTL, phyIf);
		} else {
			snprintf(buf, sizeof(buf), "%s --if %s --set-if-rsmux --set-if-rx-policy DROP "
										"--set-if-tx-policy CONTINUE --set-if-rxmc-policy DROP "
										"--set-if-rx-multi 1", SMUXCTL, phyIf);
		}
		printf("[setup_wan_port] pon_mode=%d, executing: %s\n", pon_mode, buf);
		system(buf);
		
		if(pon_mode == EPON_MODE) //EPON
		{
			char *p, *p_end, *alias = "epon-tx-default";
			for(i=0; i<=MAX_TAGS; i++)
			{
				p = buf; p_end = p+sizeof(buf);
				p += snprintf(p, (p_end-p), "%s --if %s --tx --tags %d --rule-remove-alias %s+", SMUXCTL, phyIf, i, alias);
				system(buf);
				
				p = buf; p_end = p+sizeof(buf);
				p += snprintf(p, (p_end-p), "%s --if %s --tx --tags %d", SMUXCTL, phyIf, i);
				p += snprintf(p, (p_end-p), " --set-skb-mark2 0x%llx 0x%llx", SOCK_MARK2_STREAMID_ENABLE_MASK, SOCK_MARK2_STREAMID_ENABLE_MASK);
				p += snprintf(p, (p_end-p), " --rule-priority %d --rule-alias %s --rule-append", 65535, alias);
				system(buf);
			}
			
			snprintf(ifname, sizeof(ifname), "%s%d", PON_VEIP_IF, 0);
			if(check_interface_exit(ifname) != 0)
			{
				snprintf(buf, sizeof(buf), "%s --if-create-name %s %s", SMUXCTL, phyIf, ifname);
				system(buf);
				
				alias = "epon-rx-default";
				for(i=0; i<=MAX_TAGS; i++)
				{
					p = buf; p_end = p+sizeof(buf);
					p += snprintf(p, (p_end-p), "%s --if %s --rx --tags %d --rule-remove-alias %s+", SMUXCTL, phyIf, i, alias);
					system(buf);
					
					p = buf; p_end = p+sizeof(buf);
					p += snprintf(p, (p_end-p), "%s --if %s --rx --tags %d --set-rxif %s --target ACCEPT", SMUXCTL, phyIf, i, ifname);
					p += snprintf(p, (p_end-p), " --rule-priority %d --rule-alias %s --rule-append", 100, alias);
					system(buf);
				}
			}
		}
		else if(pon_mode == GPON_MODE) //GPON
		{
			//config by TR142
		}
		else if(pon_mode == ETH_MODE || pon_mode == FIBER_MODE) //ETH
		{
			char *p, *p_end, *alias;
			/*tx*/
			//do nothing
			if(pon_mode == ETH_MODE)
				_port_set_dev_port_mapping(wan_port,phyIf);
			
			snprintf(ifname, sizeof(ifname), "%s%d", PON_VEIP_IF, 0);
			if(check_interface_exit(ifname) != 0)
			{
				snprintf(buf, sizeof(buf), "%s --if-create-name %s %s", SMUXCTL, phyIf, ifname);
				system(buf);
				
				alias = "eth-rx-default";
				for(i=0; i<=MAX_TAGS; i++)
				{
					p = buf; p_end = p+sizeof(buf);
					p += snprintf(p, (p_end-p), "%s --if %s --rx --tags %d --rule-remove-alias %s+", SMUXCTL, phyIf, i, alias);
					system(buf);
					
					p = buf; p_end = p+sizeof(buf);
					p += snprintf(p, (p_end-p), "%s --if %s --rx --tags %d --set-rxif %s --target ACCEPT", SMUXCTL, phyIf, i, ifname);
					p += snprintf(p, (p_end-p), " --rule-priority %d --rule-alias %s --rule-append", 100, alias);
					system(buf);
				}
			}
		}
		
		set_interfac_state(phyIf, 1);
	}
	
	return 0;
}
