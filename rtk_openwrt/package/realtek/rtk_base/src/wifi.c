#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdarg.h>
#include "wifi_mib.h"
#include "common.h"

const char *WLAN_PORT_IFNAME[MAX_WLAN_NUM] = {"wlan0", "wlan1"};
const char IWPRIV[] = "iwpriv";
enum{
	IWPRIV_STR = 1<<0,
	IWPRIV_BINSTR = 1<<2,
	IWPRIV_GETMIB = 1<<3,
	IWPRIV_VALIST = 1<<4
};

int iwpriv_cmd(unsigned int flags, const char *ifname, int MIBID, const char *cmd, const char *fmt, ...)
{
	char mib_val[512] = {0};
	char buf[1024] = {0}, *p;
	int len, i;
	
	if(flags & IWPRIV_GETMIB) { 
		if(MIBID >= MIB_MAX) goto error;
		if(mib_get_s2(MIBID, mib_val, sizeof(mib_val)) != 1)
			goto error;
		if(flags & IWPRIV_BINSTR)
			str2BinStr(mib_val, strlen(mib_val));
	}
	
	len = sizeof(buf) -1 ;
	p = &buf[0];
	p += snprintf(p, (len-(p-buf)), "%s %s %s ", IWPRIV, ifname, cmd);
	
	if(flags & IWPRIV_GETMIB) { 
		p += snprintf(p, (len-(p-buf)), fmt, mib_val);
	}
	else if(flags & IWPRIV_VALIST) { 
		va_list argv;
		va_start(argv, fmt);
		p += vsnprintf(p, (len-(p-buf)), fmt, argv);
	}
	//printf("==> %s\n", buf);
	//va_cmd("/bin/sh", 2, 1, "-c", buf);
	system(buf);
	return 0;
	
error:
	printf("[%s] error config %s MIBID(%d) cmd(%s)\n", __FUNCTION__, ifname, MIBID, cmd);
	return -1;
}

int _do_wlan_cal(const char *wlIf, int idx)
{
	if(!wlIf || *wlIf == '\0') return 1;
	
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_XCAP_AX, idx), "flash_set", "xcap=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_THERMAL_A, idx), "flash_set", "thermalA=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_THERMAL_B, idx), "flash_set", "thermalB=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_THERMAL_C, idx), "flash_set", "thermalC=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_THERMAL_D, idx), "flash_set", "thermalD=%s");
	/* RF TSSI DE calibration */
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_A, idx), "flash_set", "2G_cck_tssi_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_B, idx), "flash_set", "2G_cck_tssi_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_C, idx), "flash_set", "2G_cck_tssi_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_D, idx), "flash_set", "2G_cck_tssi_D=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_BW40_1S_TSSI_A, idx), "flash_set", "2G_bw40_1s_tssi_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_BW40_1S_TSSI_B, idx), "flash_set", "2G_bw40_1s_tssi_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_BW40_1S_TSSI_C, idx), "flash_set", "2G_bw40_1s_tssi_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_BW40_1S_TSSI_D, idx), "flash_set", "2G_bw40_1s_tssi_D=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_BW40_1S_TSSI_A, idx), "flash_set", "5G_bw40_1s_tssi_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_BW40_1S_TSSI_B, idx), "flash_set", "5G_bw40_1s_tssi_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_BW40_1S_TSSI_C, idx), "flash_set", "5G_bw40_1s_tssi_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_BW40_1S_TSSI_D, idx), "flash_set", "5G_bw40_1s_tssi_D=%s");

#if defined(WLAN_INTF_TSSI_SLOPE_K)
	/* RF TSSI SLOPE K, GAIN DIFF */
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_CCK_TSSI_GAIN_DIFF_A, idx), "flash_set", "2G_CCK_GAIN_DIFF_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_CCK_TSSI_GAIN_DIFF_B, idx), "flash_set", "2G_CCK_GAIN_DIFF_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_CCK_TSSI_GAIN_DIFF_C, idx), "flash_set", "2G_CCK_GAIN_DIFF_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_CCK_TSSI_GAIN_DIFF_D, idx), "flash_set", "2G_CCK_GAIN_DIFF_D=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_TSSI_GAIN_DIFF_A, idx), "flash_set", "2G_GAIN_DIFF_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_TSSI_GAIN_DIFF_B, idx), "flash_set", "2G_GAIN_DIFF_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_TSSI_GAIN_DIFF_C, idx), "flash_set", "2G_GAIN_DIFF_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_2G_TSSI_GAIN_DIFF_D, idx), "flash_set", "2G_GAIN_DIFF_D=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_5G_TSSI_GAIN_DIFF_A, idx), "flash_set", "5G_GAIN_DIFF_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_5G_TSSI_GAIN_DIFF_B, idx), "flash_set", "5G_GAIN_DIFF_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_5G_TSSI_GAIN_DIFF_C, idx), "flash_set", "5G_GAIN_DIFF_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_5G_TSSI_GAIN_DIFF_D, idx), "flash_set", "5G_GAIN_DIFF_D=%s");
	
	/* RF TSSI SLOPE K, CW DIFF */
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_CW_DIFF_A, idx), "flash_set", "2G_CCK_CW_DIFF_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_CW_DIFF_B, idx), "flash_set", "2G_CCK_CW_DIFF_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_CW_DIFF_C, idx), "flash_set", "2G_CCK_CW_DIFF_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_CCK_TSSI_CW_DIFF_D, idx), "flash_set", "2G_CCK_CW_DIFF_D=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_TSSI_CW_DIFF_A, idx), "flash_set", "2G_CW_DIFF_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_TSSI_CW_DIFF_B, idx), "flash_set", "2G_CW_DIFF_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_TSSI_CW_DIFF_C, idx), "flash_set", "2G_CW_DIFF_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_2G_TSSI_CW_DIFF_D, idx), "flash_set", "2G_CW_DIFF_D=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_TSSI_CW_DIFF_A, idx), "flash_set", "5G_CW_DIFF_A=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_TSSI_CW_DIFF_B, idx), "flash_set", "5G_CW_DIFF_B=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_TSSI_CW_DIFF_C, idx), "flash_set", "5G_CW_DIFF_C=%s");
	iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_BINSTR, wlIf, _W(HW_5G_TSSI_CW_DIFF_D, idx), "flash_set", "5G_CW_DIFF_D=%s");
#endif

	/* RF 2G RX GAIN K */
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_RX_GAINGAP_2G_CCK_AB, idx), "flash_set", "2G_rx_gain_cck=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_RX_GAINGAP_2G_OFDM_AB, idx), "flash_set", "2G_rx_gain_ofdm=%s");
	
	/* RF 2G RX GAIN K */
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_RX_GAINGAP_5GL_AB, idx), "flash_set", "5G_rx_gain_low=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_RX_GAINGAP_5GM_AB, idx), "flash_set", "5G_rx_gain_mid=%s");
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_RX_GAINGAP_5GH_AB, idx), "flash_set", "5G_rx_gain_high=%s");
	
	iwpriv_cmd(IWPRIV_GETMIB, wlIf, _W(HW_CUSTOM_PARA_PATH, idx), "flash_set", "para_path=%s");
	
	return 0;
}

int do_wlan_cal(const char *wlIf)
{
	int i;
	
	if(!wlIf || *wlIf == '\0')
		return 1;
	
	for(i=0; i<MAX_WLAN_NUM; i++)
	{
		if(WLAN_PORT_IFNAME[i] && !strcmp(WLAN_PORT_IFNAME[i], wlIf)) {
#ifndef CONFIG_2G_ON_WLAN0
			_do_wlan_cal(wlIf, i);
#else
			_do_wlan_cal(wlIf, MAX_WLAN_NUM - 1 - i);
#endif
		}
	}
	return 0;
}

int setup_wlan(void)
{
	int i;
	char mac[20] = "00E04C867001";
	
	mib_get_s2(MIB_ELAN_MAC_ADDR, mac, sizeof(mac));
	
	for(i=0; i<MAX_WLAN_NUM; i++)
	{
		const char *wlIf = WLAN_PORT_IFNAME[i];
		if(wlIf && check_interface_exit(wlIf) == 0)
		{
			set_interfac_state(wlIf, 0);
			/* LAN ports use offsets up to 5 (eth0.5), so start WLAN from 6 */
			set_interfac_mac(wlIf, mac, i + 5);
#ifndef CONFIG_2G_ON_WLAN0
			_do_wlan_cal(wlIf, i);
#else
			_do_wlan_cal(wlIf, MAX_WLAN_NUM-1-i);
#endif
			//set_interfac_state(wlIf, 1);
		}
	}

	return 0;
}
