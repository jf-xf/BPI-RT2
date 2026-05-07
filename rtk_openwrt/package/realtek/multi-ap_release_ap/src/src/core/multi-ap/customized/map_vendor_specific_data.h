#ifndef _MAP_VENDOR_SPECIFIC_DATA_H_
#define _MAP_VENDOR_SPECIFIC_DATA_H_

#define MAP_NETWORK_STA_INFO		"/var/tmp/map_agent_sta_info/%s.info"

#if 1//def RTK_ETHERNET_ADAPTER
#define RTK_FAILED      -1
#define RTK_SUCCESS     0

#define MAC_ADDR_LEN					6
#define LAN_DEV_NAME_MAX_LEN 64
#define MAX_LANNET_IF_NAME_LENGTH			64
#define WLAN_NEGOIA_RATE_LEN		10
#define WLAN_NEGOIA_RATE_LEN		10
#define MAX_LANNET_BRAND_NAME_LENGTH		64
#define MAX_LANNET_MODEL_NAME_LENGTH		64
#define MAX_LANNET_OS_NAME_LENGTH			64
#define MAX_SSID_LEN 33
//#ifdef RTK_ETHERNET_LANNETINFO
#define LANNETINFOFILE	"/var/lannetinfo"
#define LANNETINFO_RUNFILE	"/var/run/lannetinfo.pid"

#define MAX_DEV_TIME_LEN		64
#define MAX_LANNET_DEV_NAME_LENGTH			64
#define MAX_IPV6_CNT 5
#define IP6_ADDR_LEN					16
#ifdef RTK_RETRY_GET_FLOCK
#define MAX_READLANNETINFO_INTERVAL 50
#endif
//#else
#define LAN_DEV_LINK_INFO	"/var/lan_dev_link_info"
#define ETHGETINFO_RUNFILE   "/var/run/ethGetInfo.pid"
#define CMD_l2TABLE_INFO_REQ				0x1
#define CMD_l2TABLE_INFO_RESP		    	0x2
#define GET_L2INFO_ALL 0
#define MAXL2TBNUM 512


typedef struct msgHeader{
	long mtype;
	long request;
	int cmd;
	int flag;
	short realnum;
}msgHeader_t;

typedef struct ethReqInfoMsg{
        msgHeader_t msg_hdr;
        char payload[0];
}ethReqInfoMsg_t;
typedef struct ethRespInfoMsg {
        msgHeader_t msg_hdr;
        char payload[0];
}ethRespInfoMsg_t;
//#endif

typedef enum rtk_lan_device_connect_type
{
	LAN_DEV_CONN_UNKNOWN,
	LAN_DEV_CONN_WIRED,
	LAN_DEV_CONN_WLAN_2G,
	LAN_DEV_CONN_WLAN_5G
}LAN_DEV_CONN_TYPE;

typedef struct rtk_packet_s{
	unsigned int rx_packets;
	unsigned int tx_packets;
	unsigned int rx_errors;
	unsigned int rx_dropped;
}rtk_packet_T;

//#ifdef RTK_ETHERNET_LANNETINFO
typedef enum lanHost_ipv6_type_e
{
	LANHOST_IPV6_TYPE_LINKLOCAL = 1,
	LANHOST_IPV6_TYPE_GLOBAL
}lanHost_ipv6_type_t;

typedef struct lanhost_ipv6_s{ //this struct must 4-byte alignment
	char ipv6Addr[IP6_ADDR_LEN];
	lanHost_ipv6_type_t type;
	unsigned int use;
}lanhost_ipv6_t;

typedef struct lanHostInfo_s
{
	unsigned char		mac[MAC_ADDR_LEN];
	char				devName[MAX_LANNET_DEV_NAME_LENGTH];
	unsigned char		devType;	/*CMCC [0-other, 1-phone, 2-pc, 3-pad, 4-stb, 5-ap]; ELSE[0-phone 1-pad 2-PC 3-STB 4-other  0xff-unknown] */
	unsigned int		ip; /* network order */
	lanhost_ipv6_t		lanhost_ipv6s[MAX_IPV6_CNT];
	unsigned char		ipv6Addr[IP6_ADDR_LEN];
	unsigned char		connectionType;	/* 0- wired 1-wireless */
	unsigned char		port;	/* 0-wifi, 1- lan1, 2-lan2, 3-lan3, 4-lan4 */
	unsigned char		phy_port;
	char        		ifName[MAX_LANNET_IF_NAME_LENGTH];
	char				brand[MAX_LANNET_BRAND_NAME_LENGTH];
	char				model[MAX_LANNET_MODEL_NAME_LENGTH];
	char				os[MAX_LANNET_OS_NAME_LENGTH];
	unsigned int		onLineTime;
	unsigned int 		upRate;   /* in unit of kbps */
	unsigned int 		downRate; /* in unit of kbps */
	unsigned long long	rxBytes;
	unsigned long long	txBytes;
	unsigned char		firstConnect;
	unsigned char		disConnect;
	unsigned char		ntpFlag;/* 1:valid, 0:invalid*/
	char 				latestActiveTime[MAX_DEV_TIME_LEN];
	char				latestInactiveTime[MAX_DEV_TIME_LEN];
	unsigned char 		controlStatus;
	unsigned char  		internetAccess; /* default is 2 */
	unsigned char  		storageAccess; /* default is 1 */
	unsigned int		rx_packets;
	unsigned int		tx_packets;
	unsigned int		rx_errors;
	unsigned int		rx_dropped;
	unsigned int		firstStatistic;
	unsigned char		offline;	/* 0-do not care anymore 1-offline, if this is set should not recycle*/
	unsigned long		offline_expired_time;
} lanHostInfo_t;

typedef struct rtk_Negotiation_rate_s{
	char rxRate[WLAN_NEGOIA_RATE_LEN];
	char txRate[WLAN_NEGOIA_RATE_LEN];
}rtk_Negotiation_rate_T;

typedef struct rtk_lan_dhcp_client_info
{
	unsigned int ip;
	unsigned char mac[MAC_ADDR_LEN];
	unsigned int expires;
	unsigned char hostname[LAN_DEV_NAME_MAX_LEN];
}LAN_DHCP_CLIENT_INFO;

struct rtk_dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;       /* network order */
	u_int32_t expires;      /* host order */
	u_int32_t interfaceType;
	u_int8_t hostName[64];
	unsigned int active_time;
#ifdef _PRMT_X_CT_SUPPER_DHCP_LEASE_SC
	int category;
	int isCtcVendor;
	char szVendor[36];
	char szModel[36];
	char szFQDN[64];
#endif
#ifdef _PRMT_X_CU_EXTEND_
	u_int32_t hosttype;
#endif
};
//#else
typedef struct rtk_l2TableEhtry
{
	int port;
	u_int8_t macaddr[6];
}rtk_l2TableEhtry_t;

//#endif

typedef struct rtk_lan_device_base_info 
{
	unsigned int ip;
	unsigned char mac[MAC_ADDR_LEN];
	char dev_name[LAN_DEV_NAME_MAX_LEN];
	unsigned int expires;  //only for dhcp client
	LAN_DEV_CONN_TYPE conn_type;
	char conn_ifname[MAX_LANNET_IF_NAME_LENGTH];
	int conn_port;
	unsigned int link_time;
	int is_alive;
	char rxrate[WLAN_NEGOIA_RATE_LEN];
	char txrate[WLAN_NEGOIA_RATE_LEN];
	unsigned char devType;/*CMCC [0-other, 1-phone, 2-pc, 3-pad, 4-stb, 5-ap]; ELSE[0-phone 1-pad 2-PC 3-STB 4-other  0xff-unknown] */
	unsigned int tx_bytes;
	unsigned int rx_bytes;
	unsigned int tx_packets;
	unsigned int rx_packets;
	unsigned int rx_errors;
	unsigned int rx_dropped;
	unsigned int up_speed;
	unsigned int down_speed;

	char brand[MAX_LANNET_BRAND_NAME_LENGTH];
	char model[MAX_LANNET_MODEL_NAME_LENGTH];
	char os[MAX_LANNET_OS_NAME_LENGTH];

	unsigned char rssi;   //for wlan 
    unsigned char ssid[MAX_SSID_LEN];
	unsigned char internetAccess; /* default is 2 */
	unsigned char storageAccess; /* default is 1 */
#if 1//def RTK_ETHERNET_LANNETINFO
	char access_time[MAX_DEV_TIME_LEN];
#endif

	rtk_packet_T first_stats;
}LAN_DEV_BASE_INFO;

typedef struct rtk_l2_tab_info
{
	int port_num;
	unsigned char mac[MAC_ADDR_LEN];
	char          ifName[MAX_LANNET_IF_NAME_LENGTH];
	int is_alive;
}RTK_L2_TAB_INFO;

extern char * map_fill_topo_response_vendor_data(char *ip_addr, char *device_name);
extern int map_process_topo_response_vendor_data(unsigned char *m, unsigned short m_nr);
extern int map_write_sta_info_to_file(char *file_name, char *buffer);
extern int rtk_ethernet_get_l2_list(RTK_L2_TAB_INFO *pL2List, int array_len, int *real_num, int flag);//flag 0:l2_all, 1:l2_wired, 2:l2_wlan
#endif
#endif
