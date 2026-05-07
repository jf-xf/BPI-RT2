#ifndef __RTK_FC_WIFI_AMSDU_OFFLOAD__
#define __RTK_FC_WIFI_AMSDU_OFFLOAD__


#define RTK_WIFI_DFT_AMSDU_PKT_SIZE		3568

typedef struct rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_s
{
	unsigned int power_saving		:1;		// If 1, update power saving status
	unsigned int amsdu_pkt_size		:1;	// If 1, update amsdu packet size
	unsigned int p_wifi_dev			:1;
}rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t;

typedef struct rtk_fc_wifi_amsdu_pe_offload_sta_info_s
{
	unsigned int power_saving		:1;		//power saving status, 1: on; 0: off
	unsigned int amsdu_pkt_size;			//amsdu packet size
	struct net_device *p_wifi_dev;
}rtk_fc_wifi_amsdu_pe_offload_sta_info_t;

typedef enum rtk_fc_wifi_amsdu_pe_offload_mode_e
{
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP64 = 0,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_EPP64_END = RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP256,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP256,
	RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_END,
}rtk_fc_wifi_amsdu_pe_offload_mode_t;


typedef struct rtk_fc_wifi_dev_attr_s
{
	unsigned int block_relay:1;			// If 1, drop same ssid forwarding pkt
	unsigned int client_mode:1;		// If 1, hwnat driver call wifi api to modify pkt include mac and arp, dhcp content.
}rtk_fc_wifi_dev_attr_t;

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mac_id_set
* Description:
*	set mac_id and its related info for Wifi PE offload.
* Input:
*	mac_id		- mac_id
*   sta_conf_sel- wifi STA configuration selection
*   sta_conf    - wifi STA information
*	mac_addr	- MAC address of mac_id
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mac_id_set(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mac_id_del
* Description:
*	delete mac_id and its related info for Wifi PE offload.
* Input:
*	mac_id		- mac_id
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mac_id_del(unsigned int mac_id);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mac_id_flush
* Description:
*	clear all mac_id and its related info for Wifi PE offload.
* Input:
*	N/A
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mac_id_flush(void);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mode_set
* Description:
*	set wifi amsdu offload mode
* Input:
*	mode		- mode
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mode_set(rtk_fc_wifi_amsdu_pe_offload_mode_t mode);

/* Function Name:
*	rtk_fc_wifi_amsdu_pe_offload_mode_get
* Description:
*	get wifi amsdu offload mode
* Input:
*	mode		- mode
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_wifi_amsdu_pe_offload_mode_get(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode);

/* Function Name:
*	rtk_fc_wifi_dev_attr_set
* Description:
*	set dev attribute according to dev ptr, caller should call get first and then set what you want to change.
* Input:
*	dev			- net_dev pointer
*	attr			- attribute
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_dev_attr_set(struct net_device *dev, rtk_fc_wifi_dev_attr_t attr);

/* Function Name:
*	rtk_fc_wifi_dev_attr_get
* Description:
*	get dev attribute according to dev ptr
* Input:
*	dev			- net_dev pointer
* Output:
*	attr			- attribute
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_dev_attr_get(struct net_device *dev, rtk_fc_wifi_dev_attr_t *attr);

/* Function Name:
*	rtk_fc_wifi_dev_to_devidx_get
* Description:
*	get fc wifi dev index
* Input:
*	dev			- net_dev pointer
* Output:
*	wlan_dev_idx	- wifi dev index
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
int rtk_fc_wifi_dev_to_devidx_get(struct net_device *dev, unsigned int *wlan_dev_idx);

/* Function Name:
*	rtk_fc_wifi_repeater_cb_register
* Description:
*	wifi provide pkt modificaton callback
* Input:
*	rtk_fc_wfo_callback	- cb func
* Output:
*	N/A
* Return:
*	0	   		- SUCCESS
*	others  		- error code
*/
typedef int (*rtk_fc_wifi_callback)(struct net_device *dev, struct sk_buff *skb, bool rx);
int rtk_fc_wifi_client_mode_cb_register(rtk_fc_wifi_callback pfunc);


#endif

