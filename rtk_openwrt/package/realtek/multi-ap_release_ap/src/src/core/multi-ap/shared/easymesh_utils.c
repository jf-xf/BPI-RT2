/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>          // errno
#include <sys/ioctl.h>      // ioctl()
#include <sys/socket.h>     // socket()
#include <linux/wireless.h> // struct iwreq
#include <unistd.h>         // close()
#include <features.h>
#include <netinet/ip.h>

#include "easymesh_utils.h"
#include "easymesh_datamodel.h"

#include "map_logger.h"

#include "map_ioctl.h"

// #include "subr_wlan.h"
#define SIOCDEVPRIVATEAXEXT 0x89f2 //SIOCDEVPRIVATE+2

#include "map_vendor_api.h"

#define BACKHAUL_LINK_STATUS_FILE "/tmp/map_backhaul_link_status"
#define CUSTOMIZED_STATUS_FILE "/tmp/map_customized_status"

#define MULTI_AP_TEARDOWN_BIT      (0x10)
#define MULTI_AP_FRONTHAUL_BSS_BIT (0x20)
#define MULTI_AP_BACKHAUL_BSS_BIT  (0x40)
#define MULTI_AP_BACKHAUL_STA_BIT  (0x80)

#define WIFI_STATION_STATE 0x00000008
#define WIFI_AP_STATE      0x00000010

static const uint8_t us_to_global_lookup[] = {
	0,
	115, 118, 124, 121, 125, 103, 103, 102, 102, 101,
	101, 81, 94, 95, 96, 0, 0, 0, 0, 0,
	0, 116, 119, 122, 126, 126, 117, 120, 123, 127,
	127, 83, 84, 180, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

static const uint8_t eu_to_global_lookup[] = {
	0,
	115, 118, 121, 81, 116, 119, 122, 117, 120, 123,
	83, 84, 0, 0, 0, 0, 125, 180, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

static const uint8_t jp_to_global_lookup[] = {
	0,
	115, 112, 112, 112, 112, 112, 109, 109, 109, 109,
	109, 113, 113, 113, 113, 110, 110, 110, 110, 110,
	114, 114, 114, 114, 111, 111, 111, 111, 111, 81,
	82, 118, 118, 121, 121, 116, 119, 119, 122, 122,
	117, 120, 120, 123, 123, 104, 104, 104, 104, 104,
	104, 105, 105, 105, 105, 105, 83, 84, 121, 180,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

static const uint8_t cn_to_global_lookup[] = {
	0,
	115, 118, 125, 116, 119, 126, 81, 83, 84, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

const char *VALID_INTERFACES_IN_BRIDGE[] = { "veth", "eth", "nas", "wlan", "veip0", NULL };

const char *VALID_ETH_INTERFACES_IN_BRIDGE[] = { "eth", "nas", "veip0", NULL };

#if defined(CONFIG_RTK_PON_XDSL)

/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
        const char *osrc = src;
        size_t nleft = dsize;

        /* Copy as many bytes as will fit. */
        if (nleft != 0) {
                while (--nleft != 0) {
                        if ((*dst++ = *src++) == '\0')
                                break;
                }
        }

        /* Not enough room in dst, add NUL and traverse rest of src. */
        if (nleft == 0) {
                if (dsize != 0)
                        *dst = '\0';            /* NUL-terminate dst */
                while (*src++)
                        ;
        }

        return(src - osrc - 1); /* count does not include NUL */
}

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t dsize)
{
        const char *odst = dst;
        const char *osrc = src;
        size_t n = dsize;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end. */
        while (n-- != 0 && *dst != '\0')
                dst++;
        dlen = dst - odst;
        n = dsize - dlen;

        if (n-- == 0)
                return(dlen + strlen(src));
        while (*src != '\0') {
                if (n != 0) {
                        *dst++ = *src;
                        n--;
                }
                src++;
        }
        *dst = '\0';

        return(dlen + (src - osrc));    /* count does not include NUL */
}
#endif

int _rtk_get_is_ax_support(char *interface)
{
	if (!interface) {
		printf("Invalid interface name in _rtk_get_is_ax_support, please check the error! \n");
		return 0;
	}

	char filename[64]  = { 0 };
	int  is_ax_support = 0;

	snprintf(filename, sizeof(filename), "/proc/net/rtl8852ae/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax_support = 1;

	snprintf(filename, sizeof(filename), "/proc/net/rtk_wifi6/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax_support = 2;

	return is_ax_support;
}

#define RTL8192CD_IOCTL_SET_MIB 0x89f1
uint8_t _rtk_set_mib(char *interface_name, char *item_str, char *value_str)
{
	// #ifdef _RTK_LINUX_
	// 	return rtk_linuk_set_mib(interface_name, item_str,value_str);
	// #else
	// 	return 0;
	// #endif
	int s;

	int cmd_id, is_ax_support = 0;

	struct iwreq wrq;
	struct ifreq rq;
	static char  mib_str_buf[512];
	snprintf(mib_str_buf, 512, "%s=%s", item_str, value_str);
	log_debug("Set mib for %s: %s\n", interface_name, mib_str_buf);

	/*** Initialize socket ***/
	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		log_error("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	is_ax_support = _rtk_get_is_ax_support(interface_name);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_SET_MIB;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_SET_MIB;
	}

	/*** fill mib string ***/
	wrq.u.data.pointer = (caddr_t)mib_str_buf;
	wrq.u.data.length  = strlen(mib_str_buf) + 1;

	memcpy(&rq, &wrq, sizeof(struct iwreq));

	/*** set mib ***/
	if (ioctl(s, cmd_id, &rq) < 0) {
		log_error("[PLATFORM] set_mib: %s(%s). Error to ioctl.(%d)\n", mib_str_buf, interface_name, errno);
		close(s);
		return 0;
	}

	close(s);
	return 1;
}

#define RTL8192CD_IOCTL_GET_MIB 0x89f2
uint8_t _rtk_get_mib(char *interfacename, char *mibname, void *result, int size)
{
	int skfd;
	int cmd_id, is_ax_support = 0;

	struct iwreq wrq;
	struct ifreq rq;
	char *       tmp = (char *)malloc(30);

	if (!tmp) {
		printf("[%s][%d] Failed to allocate buffer with malloc \n", __FUNCTION__, __LINE__);
		return 0;
	}

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		log_error("ioctl [RTL8192CD_IOCTL_GET_MIB] socket error\n");
		if (tmp) {
			free(tmp);
			tmp = NULL;
		}
		return 0;
	}

	is_ax_support = _rtk_get_is_ax_support(interfacename);

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	strlcpy(tmp, mibname, 30);

	wrq.u.data.pointer = tmp;

	if (is_ax_support) {
		wrq.u.data.flags  = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp) + 1;
		cmd_id            = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id            = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp);
	}

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* Do the request */
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		log_error("ioctl [RTL8192CD_IOCTL_GET_MIB] for mib %s on %s failed!\n", mibname, interfacename);
		close(skfd);
		if (tmp) {
			free(tmp);
			tmp = NULL;
		}
		return 0;
	}

	if (size)
		memcpy(result, tmp, size);
	else
		strcpy(result, (char *)tmp);

	if (tmp) {
		free(tmp);
		tmp = NULL;
	}
	close(skfd);
	return 0;
}

uint8_t is_interface_valid(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		log_error("Failed to create socket...\n");
		return 0;
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
	if (ioctl(skfd, SIOCGIFINDEX, &ifr) == -1) {
		close(skfd);
		return 0;
	}

	close(skfd);
	return 1;
}

uint8_t is_interface_up(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		log_error("Failed to create socket...\n");
		return 0;
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		close(skfd);
		return 0;
	}
	close(skfd);
	return (uint8_t) !!(ifr.ifr_flags & IFF_UP);
}

uint8_t _vxd_func_off_configure(char *interface_name, uint8_t value)
{
	uint8_t interface_up = is_interface_up(interface_name);

	if (!interface_up) {
		log_debug("Skip down interface %s\n", interface_name);
		return 1;
	}

	if (NULL != strstr(interface_name, "vxd")) {
	} else {
		log_debug("Skip unsupported vxd interface %s\n", interface_name);
		return 1;
	}

	if (value) {
		log_debug("Func off interface %s\n", interface_name);
		char *buf = "1";
		_rtk_set_mib(interface_name, "func_off", buf);
	} else {
		char *buf      = "0";
		int   func_off = 0;
		_rtk_get_mib(interface_name, "func_off", &func_off, 4);
		if (1 == func_off) {
			log_debug("Unfunc off interface %s\n", interface_name);
			_rtk_set_mib(interface_name, "func_off", buf);
		}
	}
	return 0;
}

uint8_t _ap_func_off_configure(char *interface_name, uint8_t is_controller, uint8_t configure_state, uint8_t is_enabled)
{
	uint8_t interface_up = is_interface_up(interface_name);
	int     func_off = 0;

	if (!interface_up) {
		log_debug("Skip down interface %s\n", interface_name);
		return 1;
	}

	if (is_controller) {
		return 0;
	}

	if (0 == (0x0F & configure_state)) {
		log_debug("Func off interface %s\n", interface_name);
		char *buf = "1";
		_rtk_set_mib(interface_name, "func_off", buf);
	} else {
		char *buf      = "0";
		//int   func_off = 0;
		_rtk_get_mib(interface_name, "func_off", &func_off, 4);
		if (1 == func_off && is_enabled) {
			log_debug("Unfunc off interface %s\n", interface_name);
			_rtk_set_mib(interface_name, "func_off", buf);
		}
	}
	return 0;
}

uint8_t _set_bss_type_ioctl(char *interface_name, uint8_t type)
{
	int          s;
	struct iwreq iwr;
	struct ifreq rq;
	char         tmp[1];
	int          cmd_id, is_ax_support = 0;

	if (NULL == strstr(interface_name, "wlan")) {
		log_error("[ERROR] bss_ioctl does not accept non-wlan device %s.\n", interface_name);
		return 0;
	}

	tmp[0] = type;

	log_debug("Preparing to set BSS type:\n");
	log_debug("  - Interface name = %s\n", interface_name);
	log_debug("  - BSS Type = %02x\n", type);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		log_error("[ERROR] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	log_debug("Set interface type\n");
	strncpy(iwr.ifr_name, interface_name, IFNAMSIZ - 1);
	iwr.u.data.pointer = tmp;
	iwr.u.data.length  = 1;

	is_ax_support = _rtk_get_is_ax_support(interface_name);

	if (is_ax_support) {
		iwr.u.data.flags = RTL8192CD_IOCTL_UPDATE_BSS;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_UPDATE_BSS;
	}

	memcpy(&rq, &iwr, sizeof(struct iwreq));
	if (ioctl(s, cmd_id, &rq) < 0) {
		log_error("[ERROR] ioctl('%s',RTL8192CD_IOCTL_UPDATE_BSS) returned with errno=%d (%s) while setting BSS\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	log_debug("BSS set!\n");

	close(s);
	return 1;
}

uint8_t configure_interface(char *interface_name, char *ssid, char *network_key, uint8_t auth_type, uint8_t encrypt_type, uint8_t network_type, uint8_t bss_index, char *signed_connector, struct easymesh_datamodel *data_container)
{
	uint8_t is_identical       = 1;
	uint8_t is_first_configure = 0;
	int     vap_idx            = 0;
	int     i                  = 0;
	uint8_t is_auth_identical  = 0;

	log_debug("Applying WSC configuration (%s): \n", interface_name);
	log_debug("  - SSID            : %s\n", ssid);
	log_debug("  - NETWORK_KEY     : %s\n", network_key);
	if (auth_type & AUTH_DPP && auth_type & AUTH_WPA3 && auth_type & AUTH_WPA2) {
		log_debug("  - AUTH_TYPE       : %02x, WPA2/3/DPP Mixed\n", auth_type);
	} else if (auth_type & AUTH_WPA3 && auth_type & AUTH_WPA2) {
		log_debug("  - AUTH_TYPE       : %02x, WPA2/3 Mixed\n", auth_type);
	} else if (auth_type & AUTH_WPA3) {
		log_debug("  - AUTH_TYPE       : %02x, WPA3\n", auth_type);
	} else if (auth_type & AUTH_WPA && auth_type & AUTH_WPA2) {
		log_debug("  - AUTH_TYPE       : %02x, WPA2 Mixed\n", auth_type);
	} else if (auth_type & AUTH_WPA2) {
		log_debug("  - AUTH_TYPE       : %02x, WPA2\n", auth_type);
	} else if (auth_type & AUTH_WEP) {
		log_debug("  - AUTH_TYPE       : %02x, WEP\n", auth_type);
	} else if (auth_type & AUTH_WPA) {
		log_debug("  - AUTH_TYPE       : %02x, WPA\n", auth_type);
	} else if (auth_type & AUTH_DISABLED) {
		log_debug("  - AUTH_TYPE       : %02x, OPEN\n", auth_type);
	} else {
		if((data_container->customize_features & SUPPORT_OPEN_ENCRYPT) == 0){
			log_debug("  - AUTH_TYPE       : %02x, Unspport!\n", auth_type);
			return 0;
		}
	}

	log_debug("  - ENCRYPT_TYPE    : %02x\n", encrypt_type);
	log_debug("  - BSS_TYPE        : %02x\n", network_type);
	log_debug("  - BSS_INDEX       : %02x\n", bss_index);

	if (auth_type & AUTH_DPP) {
		log_debug("  - SIGNED_CONNECTOR : %s\n", signed_connector);
	}

	if (strstr(interface_name, "wlan") != NULL) {
		struct easymesh_interface_mib *root_mib = NULL;
		struct easymesh_interface_mib *vap_mib  = NULL;

		for (i = 0; i < data_container->radio_data_nr; i++) {
			if (EASYMESH_RADIO_2G == data_container->radio_data[i].radio_band && strstr(interface_name, data_container->radio_name_24g)) {
				break;
			}
			if (EASYMESH_RADIO_5GL == data_container->radio_data[i].radio_band && strstr(interface_name, data_container->radio_name_5gl)) {
				break;
			}
			if (EASYMESH_RADIO_5GH == data_container->radio_data[i].radio_band && strstr(interface_name, data_container->radio_name_5gh)) {
				break;
			}
		}

		if (i >= data_container->radio_data_nr) {
			return 0;
		}

		if (strstr(interface_name, "vxd") != NULL) {
#ifdef RTK_HAPD_MULTI_AP
			int l, m;
			for (l = 0; l < data_container->radio_data_nr; l++) {
				if ((EASYMESH_RADIO_2G == data_container->radio_data[l].radio_band && strstr(interface_name, data_container->radio_name_24g))
				    || (EASYMESH_RADIO_5GL == data_container->radio_data[l].radio_band && strstr(interface_name, data_container->radio_name_5gl))
				    || (EASYMESH_RADIO_5GH == data_container->radio_data[l].radio_band && strstr(interface_name, data_container->radio_name_5gh))) {
					for (m = 0; m < data_container->radio_data[l].interface_nr; m++) {
						if ((data_container->radio_data[l].interface_mib[m].network_type & MULTI_AP_BACKHAUL_STA_BIT) && data_container->radio_data[l].interface_mib[m].is_enabled) {
							vap_idx = m;
							break;
						}
					}

					if(m >= data_container->radio_data[l].interface_nr)
						return 2;
					break;
				}
			}
#else
			vap_idx = data_container->radio_data[i].interface_nr - 1;
#endif
		} else if (strstr(interface_name, "vap") != NULL) {
			vap_idx = (interface_name[9] - '0') + 1;
		} else if (strstr(interface_name, "va") != NULL) {
			vap_idx = (interface_name[8] - '0') + 1;
		} else {
			vap_idx = 0;
		}

		root_mib = (struct easymesh_interface_mib *)&data_container->radio_data[i].interface_mib[0];
		vap_mib  = (struct easymesh_interface_mib *)&data_container->radio_data[i].interface_mib[vap_idx];

		if (MULTI_AP_TEARDOWN_BIT == network_type) {
			if (root_mib->network_type == network_type) {
				return 2;
			}

			root_mib->is_enabled     = 1;
			root_mib->need_configure = 1;
			root_mib->network_type = network_type;

			log_warn("[WARNING] Disable radio [%s] with tear_down!\n", interface_name);
			return 1;
		}

		////////////////////////////////
		// Enable interface
		////////////////////////////////
		if (!root_mib->is_enabled) {
			is_identical = 0;
			log_warn("[WARNING] Root for [%s] is not enabled!\n", interface_name);
			root_mib->is_enabled = 1;
		}

		if (strstr(interface_name, "va") != NULL) {
			if (!vap_mib->is_enabled) {
				log_warn("[WARNING] VAP [%s] is not enabled!\n", interface_name);
				vap_mib->is_enabled = 1;
				//vap_mib->need_configure = 1;
			}
		}

		if (!(network_type & (MULTI_AP_BACKHAUL_STA_BIT | MULTI_AP_BACKHAUL_BSS_BIT | MULTI_AP_FRONTHAUL_BSS_BIT))) {
			log_debug("%s is not a multi-ap bss, skip configuration\n", interface_name);
			vap_mib->need_configure = 0;
			return 2;
		}

		if (vap_mib->authentication_type == auth_type) {
			is_auth_identical = 1;
		} else if (strstr(interface_name, "vxd") && vap_mib->authentication_type >= auth_type && vap_mib->authentication_type & auth_type) {
			is_auth_identical = 1;
		}

		if (0 == strcmp(ssid, vap_mib->ssid)) {
			if ((0 == strcmp(network_key, vap_mib->network_key)) && (network_type == vap_mib->network_type) && (is_auth_identical)) {
				if (((strstr(interface_name, "vxd") && 0 == strcmp(data_container->radio_data[i].repeater_ssid, ssid)) || (is_interface_up(interface_name) && NULL == strstr(interface_name, "vxd"))) && is_identical) {
					log_debug("Setting is identical for interface %s, skip configuration\n", interface_name);
					vap_mib->need_configure = 0;
					return 2;
				}
				vap_mib->need_configure = 1;
				return 1;
			}
		}

		// Set SSID
		vap_mib->ssid = strdup(ssid);

		// Set BSS Type
		vap_mib->network_type = network_type;

		vap_mib->network_key = strdup(network_key);

		//first configuration judge
		if (vap_mib->authentication_type != auth_type) {
			is_first_configure = 1;
		}

		vap_mib->authentication_type = auth_type;

		vap_mib->encrypt_type = encrypt_type;

		//configured
		vap_mib->need_configure = 1;
		if (strstr(interface_name, "vxd") == NULL) {
			vap_mib->bss_index = bss_index;
		}

		if (signed_connector) {
			vap_mib->signed_connector = strdup(signed_connector);
		} else {
			vap_mib->signed_connector = strdup("null");
		}

		_set_bss_type_ioctl(interface_name, network_type);
	} else {
		log_error("[ERROR] Unconfigurable interface %s \n", interface_name);
		return 0;
	}

	if (is_first_configure) {
		return 3;
	}

	return 1;
}

uint8_t configureDevice(uint8_t wfa_mode, uint8_t config_nr, struct radio_config_data *config_data, uint8_t *identical_state, struct easymesh_datamodel *data_container, MAP_VLAN_CONFIG_T **vlan_config, uint8_t *vlan_config_nr)
{
	char interface_name[20] = "wlan0-va0";
	int  i = 0, j = 0, k = 0;

	int config_number_5gl = 0, config_number_5gh = 0, config_number_24g = 0;

	uint8_t is_identical = 1, need_full_reload = 0;

	char *root_name = NULL;

	uint8_t configure_state = data_container->configured_band; //get_configure_state();

	uint8_t bss_index;

	for (i = 0; i < config_nr; i++) {
		if (MAP_CONFIG_2G == config_data[i].radio_band) {
			config_number_24g++;
			root_name = data_container->radio_name_24g;
		} else if (MAP_CONFIG_5GL == config_data[i].radio_band) {
			config_number_5gl++;
			root_name = data_container->radio_name_5gl;
		} else if (MAP_CONFIG_5GH == config_data[i].radio_band) {
			config_number_5gh++;
			root_name = data_container->radio_name_5gh;
		} else {
			log_error("Unsupported Config Type %02x!\n", config_data[i].radio_band);
			return 0;
		}

		uint8_t *interface_need_enable = (uint8_t *)malloc(sizeof(uint8_t) * (data_container->radio_data[interface_name[4] - '0'].interface_nr - 1));
		memset(interface_need_enable, 0, sizeof(uint8_t) * (data_container->radio_data[interface_name[4] - '0'].interface_nr - 1));
		for (j = 0; j < config_data[i].bss_data_nr; j++) {
			bss_index = config_data[i].bss_data[j].bss_index;
			if(0 == bss_index) {
				sprintf(interface_name, "%s", root_name);
			} else {
				sprintf(interface_name, "%s-%s%d", root_name, data_container->vap_prefix, bss_index - 1);
			}

			log_debug("Configuring interface %s\n", interface_name);
			interface_need_enable[bss_index] = 1;
			uint8_t result = configure_interface(interface_name, config_data[i].bss_data[j].ssid,
			                                     config_data[i].bss_data[j].network_key, config_data[i].bss_data[j].authentication_type,
			                                     config_data[i].bss_data[j].encryption_type, config_data[i].bss_data[j].network_type,
			                                     config_data[i].bss_data[j].bss_index, config_data[i].bss_data[j].signed_connector, data_container);

			if (config_data[i].bss_data[j].vid) {
				log_debug("Configuring VLAN for %s %s %d\n", interface_name, config_data[i].bss_data[j].ssid, config_data[i].bss_data[j].vid);
				append_vlan_config(vlan_config, vlan_config_nr, config_data[i].bss_data[j].ssid, strlen(config_data[i].bss_data[j].ssid), config_data[i].bss_data[j].vid);
			}

			if (0 == result) {
				log_error("Error, cannot configure interface %s!\n", interface_name);
				free(interface_need_enable);
				return 0;
			} else if (result == 1) {
				if (MULTI_AP_TEARDOWN_BIT == config_data[i].bss_data[j].network_type) {
					for (k = 1; k < data_container->radio_data[interface_name[4] - '0'].interface_nr - 1; k++) {
						if (data_container->radio_data[interface_name[4] - '0'].interface_mib[k].is_enabled) {
							data_container->radio_data[interface_name[4] - '0'].interface_mib[k].network_type   = 0;
							data_container->radio_data[interface_name[4] - '0'].interface_mib[k].is_enabled     = 0;
							data_container->radio_data[interface_name[4] - '0'].interface_mib[k].need_configure = 1;
						}
					}
				} else {
					if (!data_container->radio_data[interface_name[4] - '0'].interface_mib[j].is_enabled) {
						data_container->radio_data[interface_name[4] - '0'].interface_mib[j].is_enabled     = 1;
						data_container->radio_data[interface_name[4] - '0'].interface_mib[j].need_configure = 1;
					}
				}

				is_identical = 0;
			} else if (3 == result) {
				is_identical     = 0;
				need_full_reload = 1;
			}

			if (!wfa_mode) {
				if (MULTI_AP_BACKHAUL_BSS_BIT & config_data[i].bss_data[j].network_type) {
					char vxd_interface_name[10] = "wlan0-vxd\0";
					vxd_interface_name[4]       = interface_name[4];
					result                      = configure_interface(vxd_interface_name, config_data[i].bss_data[j].ssid,
												config_data[i].bss_data[j].network_key, config_data[i].bss_data[j].authentication_type,
												config_data[i].bss_data[j].encryption_type, MULTI_AP_BACKHAUL_STA_BIT, config_data[i].bss_data[j].bss_index, NULL,
												data_container);
					if (0 == result) {
						log_error("Error, cannot configure interface %s!\n", interface_name);
						free(interface_need_enable);
						return 0;
					} else if (1 == result) {
						is_identical = 0;
					} else if (3 == result) {
						is_identical     = 0;
						need_full_reload = 1;
					}
				}
			}
		}

		for (k = 0; k < data_container->radio_data[interface_name[4] - '0'].interface_nr - 1; k++) {
			if (data_container->radio_data[interface_name[4] - '0'].interface_mib[k].is_enabled && interface_need_enable[k] == 0) {
				if (k == 0) {
					if (MULTI_AP_TEARDOWN_BIT == data_container->radio_data[interface_name[4] - '0'].interface_mib[k].network_type)
						continue;
					data_container->radio_data[interface_name[4] - '0'].interface_mib[k].network_type = MULTI_AP_TEARDOWN_BIT;
				} else {
					data_container->radio_data[interface_name[4] - '0'].interface_mib[k].is_enabled = 0;
				}
				log_debug("%s is not enabled.\n", interface_name);
				is_identical                                                                        = 0;
				data_container->radio_data[interface_name[4] - '0'].interface_mib[k].need_configure = 1;
			}
		}
		free(interface_need_enable);
	}

	if (0 != config_number_24g) {
		configure_state |= MAP_CONFIGURE_BAND_24G;
		if (is_identical) {
			*identical_state |= MAP_CONFIGURE_BAND_24G;
		} else {
			*identical_state &= 0xFE;
		}
	}

	if (3 == data_container->radio_number) {
		if (0 != config_number_5gl) {
			configure_state |= MAP_CONFIGURE_BAND_5GL;
			if (is_identical) {
				*identical_state |= MAP_CONFIGURE_BAND_5GL;
			} else {
				*identical_state &= (0xFF - MAP_CONFIGURE_BAND_5GL);
			}
		}
		if (0 != config_number_5gh) {
			configure_state |= MAP_CONFIGURE_BAND_5GH;
			if (is_identical) {
				*identical_state |= MAP_CONFIGURE_BAND_5GH;
			} else {
				*identical_state &= (0xFF - MAP_CONFIGURE_BAND_5GH);
			}
		}
	} else {
		if (0 != config_number_5gl || 0 != config_number_5gh) {
			configure_state |= MAP_CONFIGURE_BAND_5GF;
			if (is_identical) {
				*identical_state |= MAP_CONFIGURE_BAND_5GF;
			} else {
				*identical_state &= (0xFF - MAP_CONFIGURE_BAND_5GF);
			}
		}
	}

	log_debug("Configuring interface set_configure_state: %02x, identical_state: %02x\n", configure_state, *identical_state);

	data_container->configured_band = configure_state;

	if (need_full_reload) {
		return 2;
	}
	return 1;
}

void *map_service_thread(void *p)
{
	struct map_service_thread_data *thread_data = (struct map_service_thread_data *)p;

	vendor_register_message_receive_call_back_functions(thread_data->device_role);
	vendor_register_append_tlv_call_back_functions(thread_data->device_role);

	map_init(thread_data->queue_name, thread_data->device_role,
	         thread_data->configure_state, thread_data->multiap_profile,
			 thread_data->default_setting,
	         thread_data->config_nr, thread_data->config_data,
	         thread_data->al_interfaces,
			 thread_data->wfa_test_mode);
	return NULL;
}

void restart_root_and_vxd_interface(uint8_t radio_index)
{
	char interface_name[20] = {0};
	char tmpBuff[512] = {0};

	//down vxd interface
	snprintf(interface_name, sizeof(interface_name), "wlan%d-vxd", radio_index);
	if (is_interface_up(interface_name)) {
		log_info("(%s %d) %s is up, restart root interface on this %d band\n",__FUNCTION__,__LINE__,interface_name,radio_index);
		log_info("(%s %d) Down interface %s\n", __FUNCTION__, __LINE__, interface_name);
		snprintf(tmpBuff, sizeof(tmpBuff), "ifconfig %s down", interface_name);
        system(tmpBuff);
	} else {
		log_info("(%s %d) %s is not up, skip restart root interface on this %d band\n",__FUNCTION__,__LINE__,interface_name,radio_index);
		return;
	}

	//down root interface
	snprintf(interface_name, sizeof(interface_name), "wlan%d", radio_index);
	snprintf(tmpBuff, sizeof(tmpBuff), "ifconfig %s down", interface_name);
	log_info("(%s %d) Down interface %s\n", __FUNCTION__, __LINE__, interface_name);
	system(tmpBuff);

	//up root interface
	snprintf(tmpBuff, sizeof(tmpBuff), "ifconfig %s up", interface_name);
	log_info("(%s %d) Up interface %s\n", __FUNCTION__, __LINE__, interface_name);
	system(tmpBuff);

	//up vxd interface
	snprintf(interface_name, sizeof(interface_name), "wlan%d-vxd", radio_index);
	snprintf(tmpBuff, sizeof(tmpBuff), "ifconfig %s up", interface_name);
	log_info("(%s %d) Up interface %s\n", __FUNCTION__, __LINE__, interface_name);
    system(tmpBuff);

}

uint8_t configure_aps_func_off(uint8_t is_controller, uint8_t configure_state, struct easymesh_datamodel *database)
{
	char interface_name[40] = "wlan0-va0";
	char radio_name[20] = {0};
	int  i = 0, j = 0;
	uint8_t radio_data_num = database->radio_data_nr;
	uint8_t is_enabled = 0;
	uint8_t radio_band = 0;  // 2G or 5G
	char *vap_prefix = database->vap_prefix;
	int vap_num = database->max_bss_per_radio - 1; // 1: exclude root ap

	for (i = 0; i < radio_data_num; i++) {
		is_enabled = database->radio_data[i].interface_mib[0].is_enabled;
		radio_band = database->radio_data[i].radio_band;

		if (MAP_CONFIG_2G == radio_band) {
			sprintf(radio_name, "%s", database->radio_name_24g);
		}
		else if (MAP_CONFIG_5GL == radio_band) {
			sprintf(radio_name, "%s", database->radio_name_5gl);
		}
		else if (MAP_CONFIG_5GH == radio_band) {
			sprintf(radio_name, "%s", database->radio_name_5gh);
		}
		else {
			log_debug("Unsupported band!\n");
			continue;
		}

		/* root */
		sprintf(interface_name, "%s", radio_name);
		_ap_func_off_configure(interface_name, is_controller, configure_state, is_enabled);

		/* vap */
		for (j = 0; j < vap_num; j++) {
			is_enabled = database->radio_data[i].interface_mib[j + 1].is_enabled;
			sprintf(interface_name, "%s-%s%d", radio_name, vap_prefix, j);
			_ap_func_off_configure(interface_name, is_controller, configure_state, is_enabled);
		}
	}

	return 0;
}

uint8_t configure_vxds_func_off(uint8_t value, uint8_t radio_num)
{
	char interface_name[20] = "wlan0-vxd";
	int  i                  = 0;

	for (i = 0; i < radio_num; i++) {
		interface_name[4] = '0' + i;
		_vxd_func_off_configure(interface_name, value);
	}

	return 0;
}

uint8_t convert_to_global_op_class(uint8_t op_class, int domain)
{
	if (op_class > 130) {
		log_error("Unsupported op class %d\n", op_class);
		return 0;
	}

	if (op_class >= 60) {
		return op_class;
	}

	switch (domain) {
	case DOMAIN_FCC:
		return us_to_global_lookup[op_class];
	case DOMAIN_ETSI:
		return eu_to_global_lookup[op_class];
	case DOMAIN_MKK:
		return jp_to_global_lookup[op_class];
	case DOMAIN_CN:
		return cn_to_global_lookup[op_class];
	case DOMAIN_GLOBAL:
	case DOMAIN_WORLD_WIDE:
		return op_class;
	default:
		break;
	}

	log_error("Unknown op class %d for domain %d\n", op_class, domain);
	return 0;
}

uint8_t get_band_from_op_class(uint8_t op_class)
{
	if (op_class == 128 || op_class == 129) {
		return RADIO_BAND_5GF;
	}

	if(op_class > 80 && op_class <= 84) {
		return RADIO_BAND_2G;
	}

	if(op_class > 110 && op_class <= 120) {
		return RADIO_BAND_5GL;
	}

	if(op_class > 120 && op_class < 130) {
		return RADIO_BAND_5GH;
	}

	return RADIO_BAND_UNKNOWN;
}

uint8_t set_radio_channel(uint8_t band, uint8_t channel, uint8_t band_width, uint8_t sideband, struct easymesh_datamodel *data_container)
{
	struct easymesh_radio_mib *radio_data = NULL;

	int i = 0;
	for (i = 0; i < data_container->radio_data_nr; i++) {
		if (RADIO_BAND_2G == band && EASYMESH_RADIO_2G == data_container->radio_data[i].radio_band) {
			radio_data = &data_container->radio_data[i];
			break;
		}

		if ((RADIO_BAND_5GF == band || RADIO_BAND_5GL == band) && EASYMESH_RADIO_5GL == data_container->radio_data[i].radio_band) {
			radio_data = &data_container->radio_data[i];
			break;
		}

		if ((RADIO_BAND_5GF == band || RADIO_BAND_5GH == band) && EASYMESH_RADIO_5GH == data_container->radio_data[i].radio_band) {
			radio_data = &data_container->radio_data[i];
			break;
		}
	}

	if (NULL == radio_data) {
		log_error("Cannot set channel for band %02x!\n", band);
		return 1;
	}

	radio_data->radio_channel       = channel;
	radio_data->channel_bandwidth   = band_width;
	radio_data->control_sideband    = sideband;
	radio_data->need_change_channel = 1;

	return 0;
}

uint8_t process_rtk_vendor_spec_tlv(struct vendorSpecificTLV *vendor_tlv, uint8_t radio_data_num, struct easymesh_radio_mib * radio_data, char *vap_prefix)
{
	char *is_blocking = "0";

	char interface_name[20];

	int i = 0, j = 0;

	uint8_t cur_blocking_flag, new_blocking_flag;

	if (memcmp(vendor_tlv->vendor_oui, REALTEK_OUI, 3)) {
		log_debug("Non-RTK Vendor Specific TLV!\n");
		return 1;
	}

	for (i = 0; i < radio_data_num; i++) {

		for (j = 0; j < radio_data[i].interface_nr; j++) {
			if(radio_data[i].interface_mib[j].network_type != MULTI_AP_BACKHAUL_BSS_BIT)
				continue;

			if(j == 0) {
				log_error("[RTK] Get backhaul bss on root interface, should be vap interface...\n");
				return 0;
			}

			sprintf(interface_name, "wlan%d-%s%d", i, vap_prefix, j-1);

			//Handle max device limit blocking

			if (0 != _rtk_get_mib(interface_name, "multiap_max_device_reached", &cur_blocking_flag, 1)) {
				log_warn("[RTK] [%s] can not get channel for interface %s\n", __FUNCTION__, interface_name);
			}

			new_blocking_flag = *((uint8_t *)vendor_tlv->m);

			if (cur_blocking_flag == new_blocking_flag) {
				log_debug("[RTK] Same blocking flag received. Do nothing...\n");
				return 0;
			}

			if (0 == new_blocking_flag) {
				log_debug("[RTK] STOP blocking new AP joining...\n");
			} else if (1 == new_blocking_flag) {
				is_blocking = "1";
				log_debug("[RTK] START blocking new AP joining...\n");
			} else {
				log_debug("[RTK] UNKNOWN value in vendorSpecificTLV! Skipping mib set...\n");
				return 1;
			}

			_rtk_set_mib(interface_name, "multiap_max_device_reached", is_blocking);
		}
	}

	return 0;
}

uint8_t buffer_mib_channel(struct easymesh_datamodel *data_container)
{
	char tmp[4] = { 0 };
	int  i      = 0;
	int  result = 0;
	for (i = 0; i < data_container->radio_data_nr; i++) {
		if (EASYMESH_RADIO_5GL == data_container->radio_data[i].radio_band) {
			sprintf(tmp, "%d", data_container->radio_data[i].radio_channel);
			_rtk_set_mib(data_container->radio_name_5gl, "multiap_dfs_ap_mib_channel", tmp);
			result = 1;
		} else if (EASYMESH_RADIO_5GH == data_container->radio_data[i].radio_band) {
			sprintf(tmp, "%d", data_container->radio_data[i].radio_channel);
			_rtk_set_mib(data_container->radio_name_5gh, "multiap_dfs_ap_mib_channel", tmp);
			result = 1;
		}
	}

	if (!result) {
		log_error("Cannot get existing 5G channel!\n");
		return 1;
	}

	return 0;
}

void get_radio_interface(const char *radio_name, const char *prefix, const int idx, char *ifname)
{
	char interface[20] = { 0 };

	if (strlen(prefix) && idx == -1) {  /* VXD */
		sprintf(interface, "%s-%s,", radio_name, prefix);
		sprintf(ifname, "%s-%s", radio_name, prefix);
	}
	else if (strlen(prefix) && idx > 0) {  /* VAP */
		sprintf(interface, "%s-%s%d,", radio_name, prefix, idx - 1);
		sprintf(ifname, "%s-%s%d", radio_name, prefix, idx - 1);
	}
	else {  /* root */
		sprintf(interface, "%s,", radio_name);
		sprintf(ifname, "%s", radio_name);
	}
}

void add_single_radio_interface(char **buffer, const char *radio_name, const char *prefix, const int idx, char *ifname)
{
	char interface[20] = { 0 };

	if (strlen(prefix) && idx == -1) {  /* VXD */
		sprintf(interface, "%s-%s,", radio_name, prefix);
		sprintf(ifname, "%s-%s", radio_name, prefix);
	}
	else if (strlen(prefix) && idx > 0) {  /* VAP */
		sprintf(interface, "%s-%s%d,", radio_name, prefix, idx - 1);
		sprintf(ifname, "%s-%s%d", radio_name, prefix, idx - 1);
	}
	else {  /* root */
		sprintf(interface, "%s,", radio_name);
		sprintf(ifname, "%s", radio_name);
	}

	if (NULL == strstr(*buffer, interface))
		strcat(*buffer, interface);
}

void add_radio_interfaces(char **buffer, const char *radio_name, const char *vap_prefix, int vap_number, int vxd_number)
{
	char interface[20] = { 0 };
	int  name_len      = strlen(radio_name);
	int  i             = 0;

	strlcpy(interface, radio_name, sizeof(interface));
	interface[name_len]     = '-';

	if (vxd_number) {
		interface[name_len + 1] = 'v';
		interface[name_len + 2] = 'x';
		interface[name_len + 3] = 'd';
		interface[name_len + 4] = ',';
		interface[name_len + 5] = '\0';
		// log_trace("%s\n", interface);
		if (NULL == strstr(*buffer, interface))
			strcat(*buffer, interface);

		interface[name_len + 1] = '\0';
	}

	name_len += 1 + strlen(vap_prefix);

	for (i = vap_number - 1; i >= 0; i--) {
		strcat(interface, vap_prefix);
		interface[name_len]     = '0' + i;
		interface[name_len + 1] = ',';
		interface[name_len + 2] = '\0';
		// log_info("%s\n", interface);
		if (NULL == strstr(*buffer, interface))
			strcat(*buffer, interface);
	}
	strlcpy(interface, radio_name, sizeof(interface));
	name_len                = strlen(radio_name);
	interface[name_len]     = ',';
	interface[name_len + 1] = '\0';
	if (NULL == strstr(*buffer, interface))
		strcat(*buffer, interface);
}

uint8_t obtainRadioBandInfo(char *radio_name, int *bandwidth, int *sideband)
{

	// Driver channel_width enum is same as MAP_BAND_WIDTH

	int is_ax_support = 0;
	is_ax_support     = _rtk_get_is_ax_support(radio_name);

	if (is_ax_support) {
		uint8_t bw = 0, sb = 0;
		if (0 != _rtk_get_mib(radio_name, "use40M", &bw, 1)) {
			log_error("Can not get bandwidth for interface %s\n", radio_name);
			return 1;
		}

		if (0 != _rtk_get_mib(radio_name, "2ndchoffset", &sb, 1)) {
			log_error("Can not get sideband for interface %s\n", radio_name);
			return 1;
		}
		*bandwidth = bw;
		*sideband  = sb;
	} else {
		if (0 != _rtk_get_mib(radio_name, "use40M", bandwidth, 4)) {
			log_error("Can not get bandwidth for interface %s\n", radio_name);
			return 1;
		}

		if (0 != _rtk_get_mib(radio_name, "2ndchoffset", sideband, 4)) {
			log_error("Can not get sideband for interface %s\n", radio_name);
			return 1;
		}
	}

	if (1 == *sideband) {
		// PrimaryChannelUpperBehavior
		*sideband = SIDE_BAND_UPPER;
	} else if (2 == *sideband) {
		// PrimaryChannelLowerBehavior
		*sideband = SIDE_BAND_LOWER;
	}

	return 0;
}

uint8_t isUnoperableOpClass(uint8_t op_class, int bandwidth, int sideband)
{
	if(bandwidth < BAND_WIDTH_80MHZ && 128 == op_class) { // if controller is operating on 20 or 40Mhz, 128 should not be sent.
		return 1;
	}

	if(bandwidth < BAND_WIDTH_160MHZ && 129 == op_class) { // if controller is operating on 20, 40, or 80Mhz, 129 should not be sent.
		return 1;
	}

	if(bandwidth == BAND_WIDTH_20MHZ) {
			switch (op_class) {
			case 81:
			case 82:
			case 115:
			case 118:
			case 121:
			case 124:
			case 125:
				return 0;
			default:
				return 1;
		}
	}

	// For 80MHz and 40MHz
	if(sideband == SIDE_BAND_LOWER) {
		switch (op_class) {
			case 84:
			case 117:
			case 120:
			case 123:
			case 127:
				return 1;
		}
	} else if(sideband == SIDE_BAND_UPPER) {
		switch (op_class) {
			case 83:
			case 116:
			case 119:
			case 122:
			case 126:
				return 1;
		}
	}

	return 0;
}

void getOpClassBandInfo(uint8_t op_class, int *bandwidth, int *sideband)
{
	switch (op_class) {
	case 81:
	case 82:
	case 115:
	case 118:
	case 121:
	case 124:
	case 125:
		*bandwidth = BAND_WIDTH_20MHZ;
		*sideband  = SIDE_BAND_NONE;
		break;
	case 83:
	case 116:
	case 119:
	case 122:
	case 126:
		*bandwidth = BAND_WIDTH_40MHZ;
		*sideband  = SIDE_BAND_LOWER;
		break;
	case 84:
	case 117:
	case 120:
	case 123:
	case 127:
		*bandwidth = BAND_WIDTH_40MHZ;
		*sideband  = SIDE_BAND_UPPER;
		break;
	case 128:
		*bandwidth = BAND_WIDTH_80MHZ;
		*sideband  = SIDE_BAND_NONE;
		break;
	case 129:
		*bandwidth = BAND_WIDTH_160MHZ;
		*sideband  = SIDE_BAND_NONE;
		break;
	}
}

uint64_t convertVersionStringToNumber(char *device_version)
{
	uint8_t generation = 0, major = 0, update = 0, stage = 0;
	char    minor      = 0;
	char *  ptr        = NULL;
	char    buffer[64] = { 0 };

	strlcpy(buffer, device_version, sizeof(buffer));

	if ((ptr = strstr(buffer, "Alpha"))) {
		*ptr  = 0;
		stage = 0;
	} else if ((ptr = strstr(buffer, "Beta"))) {
		*ptr  = 0;
		stage  = 1;
	} else {
		stage = 0xFF;
	}

	sscanf(buffer, "EasyMesh V%hhu.%hhu%c.%hhu", &generation, &major, &minor, &update);

	return ((uint64_t)generation << 56) | ((uint64_t)major << 48)
	       | ((uint64_t)minor << 40) | ((uint64_t)update << 32)
	       | ((uint64_t)stage << 24);
}

char *get_valid_interface_name(const char *interface_name, const char **valid_interfaces)
{
	char *valid_interface_name = NULL;
	int   i                    = 0;
	if (interface_name == NULL)
		return NULL;
	while (valid_interfaces[i] != NULL) {
		if (NULL != (valid_interface_name = strstr(interface_name, valid_interfaces[i])))
			return valid_interface_name;
		i++;
	}
	return NULL;
}

void append_br_interfaces(char *bridge_name, char *interfaces)
{
	char buf_tmp[512];

	if (NULL == bridge_name) {
		log_error("bridge name cannot be NULL! \n");
		return;
	}

	FILE *brctl = popen("brctl show", "r");

	int   flag      = 0;
	char *ptr       = 0;
	char *interface = 0;
	if (brctl) {
		while (fgets(buf_tmp, 512, brctl) != 0) {
			// check bridge name, only inputted bridge_name will set flag to 1
			ptr = buf_tmp;

			if (*ptr >= ' ') {
				if (bridge_name && (ptr = strstr(buf_tmp, bridge_name))) {
					// Check whether it exactly equal to bridge name. Note: 9 is Tab, 32 is Space
					flag = ((ptr == buf_tmp || 9 == *(ptr - 1) || 32 == *(ptr - 1)) && 
							(9 == *(ptr + strlen(bridge_name)) || 32 == *(ptr + strlen(bridge_name))))? 1 : 0;
				}
				else
					flag = 0;
			}

			if (!flag)
				continue;

			interface = NULL;
			if (NULL != (ptr = get_valid_interface_name(buf_tmp, VALID_INTERFACES_IN_BRIDGE)))
				interface = ptr;

			if (interface) {
				strlcpy(buf_tmp, interface, sizeof(buf_tmp));
				buf_tmp[strcspn(buf_tmp, "\r\n")] = ',';
				if (strstr(interfaces, buf_tmp)) {
					continue;
				}
				strlcat(buf_tmp, interfaces, sizeof(buf_tmp));
				memcpy(interfaces, buf_tmp, 512);
			}
		}

		pclose(brctl);
	}
}

uint8_t update_hop_to_controller(char *interface_name, uint8_t distance)
{
	char tmp[4] = { 0 };
	int  result = 0;

	sprintf(tmp, "%d", distance);
	result = _rtk_set_mib(interface_name, "multiap_hop_to_controller", tmp);

	if (!result) {
		log_debug("Cannot update hop to controller for %s!\n", interface_name);
		return 1;
	}

	return 0;
}

uint8_t update_customized_status_to_file(struct customized_status *cus_status)
{
	FILE *fd = NULL;

	fd = fopen(CUSTOMIZED_STATUS_FILE, "we");

	if (fd == NULL) {
		log_error("Unable to create %s.\n", CUSTOMIZED_STATUS_FILE);
		return 1;
	}

	fprintf(fd, "physical = %u\n", cus_status->physical_dist);
	fprintf(fd, "logical = %u\n", cus_status->logical_dist);

	fclose(fd);
	return 0;
}

uint8_t *get_ie_pointer(uint8_t max_len, uint8_t *pbuf, uint8_t ie, uint8_t *len)
{
	uint8_t *p, j, tmp;
	*len = 0;
	j    = 4;
	p    = pbuf + 4;
	if (j >= max_len)
		return NULL;

	while (1) {
		if (*p == ie) {
			*len = *(p + 1);
			return p;
		} else {
			tmp = *(p + 1);
			p += (tmp + 2);
			j += (tmp + 2);
		}
		if (j >= max_len)
			break;
	}
	return NULL;
}

uint8_t check_bss_transition_support(uint8_t *frame, uint8_t frame_length)
{
	uint8_t ie_length;
	uint8_t *frame_pt = frame;
	int remaining_length = frame_length;
	uint8_t *extended_cap = get_ie_pointer(remaining_length, frame, EXTENDED_CAP_IE, &ie_length);

	while(extended_cap) {
		if(extended_cap[4] & 0x08) {
			return 1;
		}

		remaining_length -= extended_cap - frame_pt + 2 + ie_length;
		if (remaining_length <= 0)
			break;

		frame_pt = extended_cap + 2 + ie_length;

		extended_cap = get_ie_pointer(remaining_length, frame_pt, EXTENDED_CAP_IE, &ie_length);
	}

	return 0;
}

void update_backhaul_link_status(int status, char *interface_name)
{
	char *status_string = NULL;
	FILE *fd = NULL;

	switch (status) {
	case MAP_LINK_DOWN:
		status_string = "linkdown";
		break;
	case MAP_LINK_UP:
		status_string = "linkup";
		break;
	default:
		return;
	}

	fd       = fopen(BACKHAUL_LINK_STATUS_FILE, "we");

	if (fd == NULL) {
		log_error("Unable to create %s.\n", BACKHAUL_LINK_STATUS_FILE);
		return;
	}

	if (NULL != interface_name)
		fprintf(fd, "%s %s", status_string, interface_name);
	else
		fprintf(fd, "%s", status_string);

	fclose(fd);
}

uint8_t append_vlan_config(MAP_VLAN_CONFIG_T **vlan_config, uint8_t *vlan_config_nr, char *ssid, uint8_t ssid_len, uint16_t vid)
{
	int     i      = 0;
	uint8_t result = 0;
	for (i = 0; i < *vlan_config_nr; i++) {
		if (0 == strcmp((*vlan_config)[i].ssid, ssid)) {
			break;
		}
	}
	if (i == *vlan_config_nr) {
		*vlan_config_nr += 1;
		*vlan_config                               = (MAP_VLAN_CONFIG_T *)realloc(*vlan_config, sizeof(MAP_VLAN_CONFIG_T) * (*vlan_config_nr));
		(*vlan_config)[(*vlan_config_nr) - 1].ssid = (char *)malloc(sizeof(char) * (ssid_len + 1));
		memcpy((*vlan_config)[(*vlan_config_nr) - 1].ssid, ssid, ssid_len);
		(*vlan_config)[(*vlan_config_nr) - 1].ssid[ssid_len] = '\0';
		(*vlan_config)[(*vlan_config_nr) - 1].vid            = vid;
		log_debug("Appending VLAN %d %s %d\n", *vlan_config_nr, (*vlan_config)[(*vlan_config_nr) - 1].ssid, vid);
	} else {
		log_debug("Updating VLAN %d %s from VID %d to %d\n", *vlan_config_nr, (*vlan_config)[(*vlan_config_nr) - 1].ssid, (*vlan_config)[i].vid, vid);
		if (vid != (*vlan_config)[i].vid) {
			result = 1;
		}
		(*vlan_config)[i].vid = vid;
	}
	return result;
}

int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

void init_ctomusized_status(struct customized_status *cus_status)
{
	cus_status->logical_dist  = 0;
	cus_status->physical_dist = 0;
}

uint8_t *get_wsc_attribute(uint8_t *m, uint16_t m_size, uint16_t attribute_id)
{
	uint8_t *p;

	p = m;
	while (p - m < m_size) {
		uint16_t attr_type;
		uint16_t attr_len;

		attr_type = (*p << 8) | *(p + 1);
		p += 2;
		if (attribute_id == attr_type) {
			return p;
		}
		attr_len = (*p << 8) | *(p + 1);
		p += 2 + attr_len;
	}

	return NULL;
}

void update_customized_information(struct customized_status_info status_info)
{
	FILE *fp = fopen("/tmp/map_status", "we");
	if (fp != NULL) {
		fprintf(fp, "{\"m2_status\": %d } \n", status_info.current_status);
		fclose(fp);
	}
}

uint8_t update_status(uint8_t *current_status, uint8_t next_status)
{
	uint8_t update = 0;
	if (next_status == STATUS_CONTROLLER_LOST) {
		update =1;
		*current_status = next_status;
	} else if (next_status == STATUS_AUTOCONFIG_DONE) {
		update =1;
		*current_status = next_status;
	} else if (next_status == STATUS_M2_PROCESS_FAIL) {
		update =1;
		*current_status = next_status;
	} else if (next_status == STATUS_M1_SEND && *current_status <= STATUS_M1_SEND) {
		update=1;
		*current_status = next_status;
	}
	return update;
}

OP_CLASS *get_op_class_with_reg_domain(int domain)
{
	int       reg_op_classes_len = 0;
	OP_CLASS *reg_op_classes     = NULL;
	int       i                  = 0;
	int       index_buf[32]      = { 0 };
	for (i = 0; i < (int)(sizeof(GLOBAL_20MHZ_OP_CLASS) / sizeof(uint8_t)); i++) {
		if (DOMAIN_GLOBAL == domain || DOMAIN_WORLD_WIDE == domain) {
			switch (GLOBAL_20MHZ_OP_CLASS[i]) {
			case 82:
				continue;
			}
		} else if (DOMAIN_ETSI == domain) {
			switch (GLOBAL_20MHZ_OP_CLASS[i]) {
			case 82:
			case 124:
			case 125:
				continue;
			}
		} else if (DOMAIN_FCC == domain) {
			switch (GLOBAL_20MHZ_OP_CLASS[i]) {
			case 82:
				continue;
			}
		} else if (DOMAIN_MKK == domain) {
			switch (GLOBAL_20MHZ_OP_CLASS[i]) {
			case 124:
			case 125:
				continue;
			}
		} else if (DOMAIN_CN == domain) {
			switch (GLOBAL_20MHZ_OP_CLASS[i]) {
			case 82:
			case 121:
			case 124:
				continue;
			}
		}

		index_buf[reg_op_classes_len] = i;

		reg_op_classes_len += 1;
	}

	reg_op_classes = (OP_CLASS *)malloc(sizeof(OP_CLASS) * reg_op_classes_len);

	for (i = 0; i < reg_op_classes_len; i++) {
		int index                  = index_buf[i];
		reg_op_classes[i].op_class = GLOBAL_20MHZ_OP_CLASS[index];
	}
	reg_op_classes->op_class_len = reg_op_classes_len;
	return (OP_CLASS *)reg_op_classes;
}

#define SIOCMAP_GENERAL_IOCTL 0x8B83
uint8_t map_set_5g_bss_macs_ioctl(char *interface_name, uint8_t *buffer, uint16_t buffer_size)
{
	int          skfd;
	struct iwreq wrq;
	struct ifreq rq;
	int          err;
	uint8_t      operation_type = *buffer;
	uint16_t     tlv_length     = *((uint16_t *)(buffer + 1));

	if (operation_type != MAP_SEND_5G_BSS_MACS || tlv_length + 3 > buffer_size) {
		log_error("[Debug] map_set_5g_bss_macs_ioctl operation_type or length check fail ! %d, %d\n", operation_type, tlv_length + 3);
		return 1;
	}

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		log_error("ioctl [RTL8192CD_IOCTL_CSA_5G_LIST] socket error\n");
		return 1;
	}

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interface_name, sizeof(wrq.ifr_name));
	wrq.u.data.flags   = SIOCMAP_GENERAL_IOCTL;
	wrq.u.data.pointer = (caddr_t)buffer;
	wrq.u.data.length  = buffer_size;

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, SIOCDEVPRIVATEAXEXT, &rq) < 0) {
		err = errno;
		log_error("[ERROR] ioctl('%s', SIOCMAP_GENERAL_IOCTL) returned with errno=%d (%s) while passing 5G bss list to driver\n", interface_name, errno, strerror(errno));
		close(skfd);
		return err;
	}
	err = 0;

	close(skfd);
	return err;
}

void *zalloc(uint32_t size)
{
	void *p;

	if (0 == size)
		return NULL;

	p = calloc(1, size);

	if (NULL == p) {
		printf("ERROR: Out of memory!\n");
		exit(1);
	}

	return p;
}

uint8_t map_get_mac(char *name, uint8_t *mac_address)
{
	int          fd;
	struct ifreq s;

	strlcpy(s.ifr_name, name, sizeof(s.ifr_name));
	fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_IP);
	if (-1 == fd) {
		log_error("[PLATFORM] Could not open socket when obtaining MAC address of interface %s\n", name);
		return 0;
	}

	if (0 != ioctl(fd, SIOCGIFHWADDR, &s)) {
		log_error("[PLATFORM] Could not obtain MAC address of interface %s\n", name);
		close(fd);
		return 0;
	}
	close(fd);
	memcpy(mac_address, s.ifr_addr.sa_data, 6);
	return 1;
}

int ifconfig_helper(const char *if_name, int up)
{
	int fd;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		log_error("VLAN: %s: socket(AF_INET,SOCK_STREAM) "
			   "failed: %s", __func__, strerror(errno));
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, if_name, IFNAMSIZ);

	if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0) {
		log_error("VLAN: %s: ioctl(SIOCGIFFLAGS) failed "
			   "for interface %s: %s",
			   __func__, if_name, strerror(errno));
		close(fd);
		return -1;
	}

	if (up)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;

	if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0) {
		log_error("VLAN: %s: ioctl(SIOCSIFFLAGS) failed "
			   "for interface %s (up=%d): %s",
			   __func__, if_name, up, strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}