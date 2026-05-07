#ifndef __RTK_FC_MAPPER_API_H__
#define __RTK_FC_MAPPER_API_H__

#include <rt_ext_mapper.h>

#define ETHER_ADDR_LEN 6


/* Function Name:
*	   rtk_fc_l3SwMeter_set
* Description:
*	   set L3 or sw meter
* Input:
*      virMeterType	- virtual meter type
*      hwIdx			- L3 meter hw index or sw meter index
*      meterConf 	- meter configuration
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_l3SwMeter_set(rt_rate_meter_type_t virMeterType, int hwIdx, rt_internal_meterConfig_t meterConf);

/* Function Name:
*	   rtk_fc_l3SwMeter_get
* Description:
*	   get L3 or sw meter
* Input:
*      virMeterType	- virtual meter type
*      hwIdx			- L3 meter hw index or sw meter index      
* Output:
*	   meterConf 	- meter configuration
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_l3SwMeter_get(rt_rate_meter_type_t virMeterType, int hwIdx, rt_internal_meterConfig_t *meterConf);

/* Function Name:
*	   rtk_fc_l3SwMeterSize_get
* Description:
*	   get numer of L3 and sw meter
* Input:
*      N/A
* Output:
*	   meterSize		- L3 and sw meter size
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_l3SwMeterSize_get(rt_internal_meterSize_t *meterSize);

/* Function Name:
*	   rtk_fc_l3SwMeterUsedState_get
* Description:
*	   get L3 or sw meter used state
* Input:
*      vir_index		- virtual meter index 
* Output:
*	   ifUsed		- if the virtual meter is been used
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_l3SwMeterUsedState_get(int vir_index, bool *ifUsed);

/* Function Name:
*	   rtk_fc_l3PortUniType_set
* Description:
*	   set port type to nodify l34 module
* Input:
*      port			- pyhsical port index 
*      uniType		- port type
* Output:
*      None
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_l3PortUniType_set(rt_port_t port, rt_port_uniType_t uniType);


/* Function Name:
 *      rtk_fc_macAddr_learning_limit_get
 * Description:
 *      Get configuration of learning limit of MAC address of the port
 * Input:
 *      port_idx        			- Port index
 * Output:
 *      pLearningLimitNumber 	- Learning limit number of MAC address of the port
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of learning limit of MAC address of the port
 */
int rtk_fc_macAddr_learning_limit_get(rt_phy_port_t port_idx, int *pLearningLimitNumber);

/* Function Name:
 *      rtk_fc_macAddr_learning_limit_set
 * Description:
 *      Set configuration of learning limit of MAC address of the port
 * Input:
 *      port_idx        			- Port index
 *      learningLimitNumber 		- Learning limit number of MAC address of the port  
 * Output:
 *       
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of learning limit of MAC address of the port
 */
int rtk_fc_macAddr_learning_limit_set(rt_phy_port_t port_idx, int learningLimitNumber);

/* Function Name:
 *      rtk_fc_l3fe_unknownDAStormCtrl_set
 * Description:
 *      tuen on/off uuc storm control.
 * Input:
 *      port_idx        			- Port index
 *      enable 					- State is enabled or disabled
 *      l3fe_pol_idx				- Assign policer index
 * Output:
 *       None
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 */
int rtk_fc_l3fe_unknownDAStormCtrl_set(rt_port_t port, bool enable, int l3fe_pol_idx);

/* Function Name:
 *      rtk_fc_l3fe_unknownDAStormCtrl_get
 * Description:
 *     get the state of uuc storm control.
 * Input:
 *      port_idx        			- Port index
 * Output:
 *      enable 					- State is enabled or disabled
 *      l3fe_pol_idx				- Assigned policer index
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 */
int rtk_fc_l3fe_unknownDAStormCtrl_get(rt_port_t port, bool *enable, int *l3fe_pol_idx);

/* Function Name:
 *      rtk_fc_l2_addr_add
 * Description:
 *     to notify l2 entry is added.
 * Input:
 *      pL2Addr        			- l2 entry
 * Output:
 *      None
 * Return:
 *      0		- SUCCESS
 *      others	- error code
 */
int rtk_fc_l2_addr_add(rt_l2_ucastAddr_t *pL2Addr);

/* Function Name:
 *      rtk_fc_l2_addr_del
 * Description:
 *     to notify l2 entry is deleted.
 * Input:
 *      pL2Addr        			- l2 entry
 * Output:
 *      None
 * Return:
 *      0		- SUCCESS
 *      others	- error code
 */
int rtk_fc_l2_addr_del(rt_l2_ucastAddr_t *pL2Addr);

/* Function Name:
 *      rtk_fc_l2_addr_get
 * Description:
 *     to get valid l2 uc entry.
 * Input:
 *      pScanIdx				- scan entry from assigned index
 *      pL2Addr        			- l2 entry
 * Output:
 *      pScanIdx				- valid index after assigned entry
 * Return:
 *      0		- SUCCESS
 *      others	- error code
 */
int rtk_fc_l2_addr_get(int32 *pScanIdx, rt_l2_ucastAddr_t *pL2Addr);

/* Function Name:
 *      rtk_fc_l2_ageTime_set
 * Description:
 *     sync l2 aging time to fc sw l2.
 * Input:
 *      ageTime				- timeout value, unit sec.
 * Output:
 *      None
 * Return:
 *      0		- SUCCESS
 *      others	- error code
 */
int rtk_fc_l2_ageTime_set(uint32 ageTime);

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
/* Function Name:
 *      rtk_fc_pon_sid_set
 * Description:
 *     to notify pon stream id configuration
 * Input:
 *      sid        			- stream id, 0~127
 *      sidInfo			- ldpid, cos, flowid
 * Output:
 *      None
 * Return:
 *      0		- SUCCESS
 *      others	- error code
 */
int rtk_fc_pon_sid_set(uint16 sid, rt_ponmisc_sidInfo_t sidInfo);

/* Function Name:
 *      rtk_fc_mirror_port_set
 * Description:
 *     to notify mirror configuration
 * Input:
 *      enable        			- valid state
 *      mirroring_port		- monitor port
 *      mirrored_port		- traffic tx/rx port
 * Output:
 *      None
 * Return:
 *      0		- SUCCESS
 *      others	- error code
 */
int rtk_fc_mirror_port_set(bool enable, uint32 mirroring_port, uint32 mirrored_port);
#endif

/* Function Name:
*	   rtk_fc_igmp_mcMode
* Description:
*	   setting Multicast learning Mode 
* Input:
*	   rtk_fc_igmp_mcMode	- mcMode
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_igmp_mcMode(rt_igmp_multicast_learning_mode_t mcMode);


/* Function Name:
*	   rtk_fc_igmp_unknownMulticast_action
* Description:
*	   setting unknown multicast action
* Input:
*	   rt_igmp_unknownMc_cfg_t	- mcUnknCfg
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_igmp_unknownMulticast_action(rt_igmp_unknownMc_cfg_t	mcUnknCfg);

/* Function Name:
*	   rtk_fc_igmp_check_userGroup
* Description:
*	   check if user group address is vaild
* Input:
*	   rt_igmp_unknownMc_cfg_t 	- user group infomation
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/


int rtk_fc_igmp_check_userGroup(rtk_igmp_userGroup_cfg_t *userGroup);


/* Function Name:
*	   rtk_fc_igmp_mcGroupDevSet
* Description:
*	   set multicast DevConfig
* Input:
*	   mcDevConfig	- Group Dev info.
*
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_igmp_mcGroupDevSet(rt_igmp_group_devPort_cfg_t *mcDevConfig);



/* Function Name:
*	   rtk_fc_igmp_mcGroupDevGet
* Description:
*	   get multicast DevConfig
* Input:
*	   mcDevConfig	- Group Dev info.
*
* Output:
*	   mcDevConfig	- Group Dev info.
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_igmp_mcGroupDevGet(rt_igmp_group_devPort_cfg_t *mcDevConfig);


/* Function Name:
*	   rtk_fc_igmp_mcGroupSet
* Description:
*	   add/del multicast flow 
* Input:
*	   rtk_fc_mc_set_t	- Group info.
*
*	   set act0_pmsk==0 and act1_pmsk==0 to Delete multicast flow
*	   set groupAddr zero to flush all multicast group
*	   act0_pmsk/act1_pmsk bit's mapping 
*				[Port0-Port4 		-> bit's 0-4   ]	
*				[Wlan0 SSID0-SSID4 	-> bit's 11-15 ] 
*				[Wlan1 SSID0-SSID4 	-> bit's 17-21 ]
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_igmp_mcGroupSet(rt_igmp_multicast_group_cfg_t mcConfig);
int rtk_fc_igmp_mcflowCheck(rt_igmp_multicast_group_cfg_t* pmcConfig);


/* Function Name:
*	   rtk_fc_netifMib_get
* Description:
*	   getting netif counter
* Input:
*      devName            - netif dev name
* Output:
*	   pNetifMibCnt	- netif counter
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_netifMib_get(char *devName, rt_stat_netif_mib_t *pNetifMib);

/* Function Name:
*	   rtk_fc_netifMib_clear
* Description:
*	   clear netif counter
* Input:
*      devName            - netif dev name
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_netifMib_clear(char *devName);

#if defined(CONFIG_FC_RTL8277C_SERIES)
//TODO: 9607F flow cache mib
/* Function Name:
 *      rtk_fc_flowCache_mib_get
 * Description:
 *      Get flow counters by flow index.
 * Input:
 *      flow_idx			- Flow index
 * Output:
 *      pFlowCacheMib	- Flow cache MIB
 * Return:
 *      0              - SUCCESS
 *      others         - error code
 * Note:
 *      The API can get flow cache counters
 */
int rtk_fc_flowCache_mib_get ( uint32 flow_idx, rt_stat_flowCache_mib_t* pFlowCacheMib );

/* Function Name:
 *      rtk_fc_flowCache_mib_reset
 * Description:
 *      Reset flow counters by flow index.
 * Input:
 *      flow_idx	- Flow index
 * Output:
 *      None.
 * Return:
 *      0              - SUCCESS
 *      others         - error code
 * Note:
 *      The API can reset flow counters
 */
int rtk_fc_flowCache_mib_reset ( uint32 flow_idx );
#endif

 /* Function Name:
 *      rtk_fc_flowMib_get
 * Description:
 *      get flow counters by flow mib index.
 * Input:
 *      idx            - flow mib index
 * Output:
 *      pflowMibCnt    - flow counters (include ingress/egress packet count and byte count).
 * Return:
 *      0              - SUCCESS
 *      others         - error code
 */
int rtk_fc_flowMib_get(int idx, rt_stat_flow_mib_t *pflowMibCnt);


 /* Function Name:
 *      rtk_fc_flowMib_clear
 * Description:
 *      clear flow counters by flow mib index.
 * Input:
 *      idx            - flow mib index
 * Output:
 *     N/A
 * Return:
 *      0              - SUCCESS
 *      others         - error code
 */
int rtk_fc_flowMib_clear(int idx);

 /* Function Name:
 *      rtk_fc_aclMib_get
 * Description:
 *      get acl counters by acl mib index.
 * Input:
 *      idx            - acl mib index
 * Output:
 *      paclMibCnt    - acl counters
 * Return:
 *      0              - SUCCESS
 *      others         - error code
 */
int rtk_fc_aclMib_get(int idx, rt_stat_acl_mib_t *paclMibCnt);

/* Function Name:
* 	 rtk_fc_aclMib_set
* Description:
* 	 set acl counters type by acl mib index.
* Input:
* 	 idx			- acl mib index
* Output:
* 	 paclMibCnt    - acl counters
* Return:
* 	 0				- SUCCESS
* 	 others 		- error code
*/
int rtk_fc_aclMib_set(int idx, rt_stat_acl_mib_t paclMibCnt);

 /* Function Name:
 *      rtk_fc_aclMib_clear
 * Description:
 *      clear acl counters by acl mib index.
 * Input:
 *      idx            - acl mib index
 * Output:
 *     N/A
 * Return:
 *      0              - SUCCESS
 *      others         - error code
 */
int rtk_fc_aclMib_clear(int idx);

/* Function Name:
*	   rtk_fc_hostPoliceControl_set
* Description:
*	   setting host policing entry 
* Input:
*      idx            - host policing entry index
*	  pHostPoliceControl	- host policing entry configuration
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_hostPoliceControl_set(int idx, rt_rate_host_policer_control_t hostPoliceControl);

/* Function Name:
*	   rtk_fc_hostPoliceControl_get
* Description:
*	   getting host policing entry 
* Input:
*      idx            - host policing entry index
* Output:
*	   hostPoliceControl	- host policing entry configuration
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_hostPoliceControl_get(int idx, rt_rate_host_policer_control_t *pHostPoliceControl);

/* Function Name:
*	   rtk_fc_hostPoliceMib_get
* Description:
*	   getting host counter
* Input:
*      idx            - host policing entry index
* Output:
*	   pHostMibCnt	- host counter
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_hostPoliceMib_get(int idx, rt_rate_host_policer_mib_t *pHostMibCnt);

/* Function Name:
*	   rtk_fc_hostPoliceMib_clear
* Description:
*	   clear host counter
* Input:
*      idx            - host policing entry index
* Output:
*	   N/A
* Return:
*	   0	   - SUCCESS
*	   others  - error code
*/
int rtk_fc_hostPoliceMib_clear(int idx);

/* Function Name:
 *      rt_stat_gemFilter_set
 * Description:
 *      Set gem filter rule.
 * Input:
 *      index		- rule index, 0-31
 *      conf		- enabled state, type, vlan filter
 * Output:
 *      None.
 * Return:
 *	  0	   		- SUCCESS
 *      others  		- error code
 */
int rtk_fc_stat_gemFilter_set(uint32 index, rt_stat_gemFilter_conf_t conf);

/* Function Name:
 *      rtk_fc_stat_gemFilter_get
 * Description:
 *      Get gem filter rule.
 * Input:
 *      index		- rule index, 0-31
 * Output:
 *      pConf		- enabled state, type, vlan filter
 * Return:
 *	  0	   		- SUCCESS
 *      others  		- error code
 */
int rtk_fc_stat_gemFilter_get(uint32 index, rt_stat_gemFilter_conf_t* pConf);

/* Function Name:
 *      rtk_fc_stat_gemFilterMib_reset
 * Description:
 *      Reset gem filter rule.
 * Input:
 *      index		- rule index, 0-31
 * Output:
 *      None.
 * Return:
 *	  0	   		- SUCCESS
 *      others  		- error code
 */
int rtk_fc_stat_gemFilterMib_reset(uint32 index);

/* Function Name:
 *      rtk_fc_stat_gemFilterMib_get
 * Description:
 *      Get gem filter mib.
 * Input:
 *      index		- rule index, 0-31
 * Output:
 *      pCntr		- mib
 * Return:
 *	  0	   		- SUCCESS
 *      others  		- error code
 */
int rtk_fc_stat_gemFilterMib_get(uint32 index, rt_stat_port_cntr_t* pCntr);


/* Function Name:
 *      rtk_fc_sw_rate_limit_get
 * Description:
 *      Get configuration of software rate limit of the type
 * Input:
 *      type        			- Type of software rate limit
 * Output:
 *      pSwRateLimit_conf 	- Configuration of software rate limit of the type
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of software rate limit of the type
 */
int rtk_fc_sw_rate_limit_get(rt_rate_sw_rate_limit_type_t type, rt_rate_sw_rate_limit_conf_t *pSwRateLimit_conf);

/* Function Name:
 *      rtk_fc_sw_rate_limit_set
 * Description:
 *      Set configuration of software rate limit of the type
 * Input:
 *      type        			- Type of software rate limit
 *      swRateLimit_conf 	- Configuration of software rate limit of the type
 * Output:
 *      
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of software rate limit of the type
 */
int rtk_fc_sw_rate_limit_set(rt_rate_sw_rate_limit_type_t type, rt_rate_sw_rate_limit_conf_t swRateLimit_conf);

/* Function Name:
 *      rtk_fc_aclFilterAndQos_add
 * Description:
 *      Add acl rule 
 * Input:
 *      acl_filter        - a acl rule include the patterns which need to be filtered, and related action
 * Output:
 *      acl_filter_idx        - the index of the added acl rule
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can add acl filter and qos remarking rule
 */
int rtk_fc_aclFilterAndQos_add(rt_acl_filterAndQos_t *acl_filter, int *acl_filter_idx);

/* Function Name:
 *      rtk_fc_aclFilterAndQos_del
 * Description:
 *      Delete acl filter rule 
 * Input:
 *      acl_filter_idx        - the index of the acl rule which need to be delete
 * Output:
 *      None.
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can delete acl rule
 */
int rtk_fc_aclFilterAndQos_del(int acl_filter_idx);

/* Function Name:
 *      rtk_fc_aclFilterAndQos_get
 * Description:
 *      Get acl filter rule 
 * Input:
 *      acl_filter_idx        - the index of the acl rule which need to get
 * Output:
 *      acl_filter        - the acl rule which be found
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get acl filter rule by rule index
 */
int rtk_fc_aclFilterAndQos_get(int acl_filter_idx, rt_acl_filterAndQos_t *acl_filter);

/* Function Name:
 *		rt_acl_reserved_status_get
 * Description:
 *		Get reserved acl status by reserved type 
 * Input:
 *		reserved type		  	- the type of the reserve acl which need to get
 * Output:
 *		status		  			- the reserved acl status which be found
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can get reserved acl status by rule type
 */
 int rtk_fc_acl_reserved_status_get(rt_acl_reserved_type_t reserved_type, int *status);

/* Function Name:
 * 	 rt_acl_reserved_status_set
 * Description:
 * 	 Set reserved acl status by reserved type 
 * Input:
 * 	 reserved type			 - the type of the reserve acl which need to set
 * Output:
 * 	 status 				 - the reserved acl status should be update
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 * 	 The API can update reserved acl status by rule type
 */
int rtk_fc_acl_reserved_status_set(rt_acl_reserved_type_t reserved_type, int status);

/* Function Name:
 *      rtk_fc_wlan_macAddr_learning_limit_get
 * Description:
 *      Get configuration of learning limit of MAC address of the wlan device
 * Input:
 *      wlan_idx        			- wlan index
 *      device_idx        			- deivce index
 * Output:
 *      pMacAddr_learning_limit 	- Configuration of learning limit of MAC address of the wlan device
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of learning limit of MAC address of the wlan device
 */
int rtk_fc_wlan_macAddr_learning_limit_get(rt_wlan_index_t wlan_idx, rt_wlan_mbssid_index_t device_idx, rt_misc_macAddr_learning_limit_t *pMacAddr_learning_limit);

/* Function Name:
 *      rtk_fc_wlan_macAddr_learning_limit_set
 * Description:
 *      Set configuration of learning limit of MAC address of the wlan device
 * Input:
 *      wlan_idx        			- wlan index
 *      device_idx        			- deivce index
 *      policerControl 			- Configuration of learning limit of MAC address of the wlan device
 * Output:
 *      
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of learning limit of MAC address of the wlan device
 */
int rtk_fc_wlan_macAddr_learning_limit_set(rt_wlan_index_t wlan_idx, rt_wlan_mbssid_index_t device_idx, rt_misc_macAddr_learning_limit_t macAddr_learning_limit);

/* Function Name:
 *		rtk_fc_wan_access_limit_get
 * Description:
 *		Get configuration of wan access limit
 * Input:
 * Output:
 *		pWan_access_limit 	- Configuration of wan access limit
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can get configuration of wan access limit
 */
int rtk_fc_wan_access_limit_get(rt_misc_wan_access_limit_t *pWan_access_limit);
	
/* Function Name:
 *		rt_misc_wan_access_limit_set
 * Description:
 *		Set configuration of wan access limit
 * Input:
 *		wan_access_limit 	- Configuration of wan access limit
 * Output:
 *		
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can set configuration of wan access limit
 */
int rtk_fc_wan_access_limit_set(rt_misc_wan_access_limit_t wan_access_limit);

/* Function Name:
 *		rt_misc_burst_packet_set
 * Description:
 *		Setup user-defined packet sending to desired port for predetermined number and rate.
 * Input:
 *		burst_packet	- Configuration of burst packet
 * Output:
 *		
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can set configuration of burst packet creation or obliteration
 */
int rtk_fc_burst_packet_set(rt_misc_burst_packet_t burst_packet);

/* Function Name:
 *		rtk_fc_wanPhyPortMask_set
 * Description:
 *		Set wan phy port mask
 * Input:
 *		wan_portMask 	- Configuration of wan portmask
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can set configuration of wan portmask
 */
int rtk_fc_wanPhyPortMask_set(rt_misc_wan_phyPortMask_t wan_portMask);

/* Function Name:
 *		rtk_fc_wanPhyPortMask_get
 * Description:
 *		Get wan phy port mask
 * Input:
 * Output:
 *		wan_portMask 	- Configuration of wan portmask
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can get configuration of wan portmask
 */
int rtk_fc_wanPhyPortMask_get(rt_misc_wan_phyPortMask_t *pWan_portMask);


/* Function Name:
 *		rtk_fc_wanFastFwdDev_add
 * Description:
 *		Add one fast forward device to WAN configuration
 * Input:
 *		dev_name 	- fast forward device name
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can add one fast forward device to WAN configuration
 */
int rtk_fc_wanFastFwdDev_add(char *dev_name);

/* Function Name:
 *		rtk_fc_wanFastFwdDev_del
 * Description:
 *		Delete one fast forward device from WAN configuration
 * Input:
 *		dev_name 	- fast forward device name
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can delete one fast forward device from WAN configuration
 */
int rtk_fc_wanFastFwdDev_del(char *dev_name);


/* Function Name:
 *		rtk_fc_vlanGroup_add
 * Description:
 *		Add vlan group mac limit 
 * Input:
 *		port 		- port index
 *		groupLimit 	- groupLimit
 *		pGroupIdx 	- group index pointer
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 */
int rtk_fc_vlanGroupLrnLimit_add(int port, int groupLimit, int *pGroupIdx);

/* Function Name:
 *		rtk_fc_vlanGroup_del
 * Description:
 *		Del vlan group mac limit 
 * Input:
 *		groupIdx 	- group index
 *		vlanId 		- gvlanId
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 */

int rtk_fc_vlanGroupLrnLimit_del(int groupIdx, int vlanId);



/* Function Name:
 *		rtk_fc_vlanGroup_set
 * Description:
 *		set vlan group mac limit 
 * Input:
 *		groupIdx 	- group index
 *		vlanId 		- gvlanId
 *		groupLimit	- groupLimit
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 */

int rtk_fc_vlanGroupLrnLimit_set(int groupIdx, int vlanId, int groupLimit);

/* Function Name:
 *		rtk_fc_vlanGroup_get
 * Description:
 *		Get vlan group mac limit 
 * Input:
 *		groupIdx 	- group index
 *		pGroup		- group info pointer
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 */

int rtk_fc_vlanGroupLrnLimit_get(int groupIdx, rt_misc_vlanGroupMacLimit_info_t *pGroup);

/* Function Name:
 *		rtk_fc_vlanGroup_find
 * Description:
 *		Find vlan group mac limit 
 * Input:
 *		port	 	- port index
 *		vlanId		- vlanId
 *		pGroupIdx	- group index pointer
 * Output:
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 */

int rtk_fc_vlanGroupLrnLimit_find(int port, int vlanId, int *pGroupIdx);






/* Function Name:
 *		rtk_fc_flow_callbackFunc_register
 * Description:
 *		register callback function
 * Input:
 *		cbFuncType	- callback function type
  *		pCbFunc	- callback function pointer
 * Output:
 *		None.
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can register callback function.
 *		Please note that data type of pCbFunc is varied according to the callback function type.
 */
int rtk_fc_flow_callbackFunc_register(rt_flow_callback_func_t cbFuncType, void* pCbFunc);

/* Function Name:
 *		rtk_fc_flow_operation_add
 * Description:
 *		make a flow operation
 * Input:
 *		flowOp	- flow oparation
  *		pOpData	- data for oparation
 * Output:
 *		None.
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can make an operation to flows.
 *		Please note that data type of pOpData is varied according to the operation.
 */
int rtk_fc_flow_operation_add(rt_flow_ops_t flowOp, rt_flow_ops_data_t* pOpData);

/* Function Name:
 *		rtk_fc_flow_hwnatMode_set
 * Description:
 *		set configuration of hwnat mode
 * Input:
 *		hwnatMode	- Configuration of hwnat mode
 * Output:
 *		None.
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can set configuration of hwnat mode
 */
int rtk_fc_flow_hwnatMode_set(rt_flow_hwnat_mode_t hwnatMode);

/* Function Name:
 *		rtk_fc_flow_hwnatMode_get
 * Description:
 *		get configuration of hwnat mode
 * Input:
 *		None.
 * Output:
 *		pHwnatMode	- Configuration of hwnat mode
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *		The API can get configuration of hwnat mode
 */
int rtk_fc_flow_hwnatMode_get(rt_flow_hwnat_mode_t *pHwnatMode);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
/* Function Name:
 *      rtk_fc_qos_aqm_get
 * Description:
 *      Get configuration of aqm
 * Input:
 *      
 * Output:
 *      pAqm_cfg 	- Configuration of aqm
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of aqm
 */
int rtk_fc_qos_aqm_get(rt_qos_aqm_cfg_t* pAqm_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_set
 * Description:
 *      Set configuration of aqm
 * Input:
 *      aqm_cfg 	- Configuration of aqm
 * Output:
 *
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of aqm
 */
int rtk_fc_qos_aqm_set(rt_qos_aqm_cfg_t aqm_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_cos_get
 * Description:
 *      Get configuration of aqm cos
 * Input:
 *      
 * Output:
 *      pAqm_cos_cfg 	- Configuration of aqm cos
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of aqm cos
 */
int rtk_fc_qos_aqm_cos_get(rt_qos_aqm_cos_cfg_t* pAqm_cos_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_cos_set
 * Description:
 *      Set configuration of aqm cos
 * Input:
 *      aqm_cos_cfg 	- Configuration of aqm cos
 * Output:
 *
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of aqm cos
 */
int rtk_fc_qos_aqm_cos_set(rt_qos_aqm_cos_cfg_t aqm_cos_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_threshold_get
 * Description:
 *      Get configuration of aqm threshold
 * Input:
 *      
 * Output:
 *      pAqm_threshold_cfg 	- Configuration of aqm threshold
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of aqm threshold
 */
int rtk_fc_qos_aqm_threshold_get(rt_qos_aqm_threshold_cfg_t* pAqm_threshold_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_threshold_set
 * Description:
 *      Set configuration of aqm threshold
 * Input:
 *      aqm_threshold_cfg 	- Configuration of aqm threshold
 * Output:
 *
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of aqm threshold
 */
int rtk_fc_qos_aqm_threshold_set(rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_timer_get
 * Description:
 *      Get configuration of aqm timer
 * Input:
 *      
 * Output:
 *      pAqm_timer_cfg 	- Configuration of aqm timer
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get configuration of aqm timer
 */
int rtk_fc_qos_aqm_timer_get(rt_qos_aqm_timer_cfg_t* pAqm_timer_cfg);


/* Function Name:
 *      rtk_fc_qos_aqm_timer_set
 * Description:
 *      Set configuration of aqm timer
 * Input:
 *      aqm_timer_cfg 	- Configuration of aqm timer
 * Output:
 *
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can set configuration of aqm timer
 */
int rtk_fc_qos_aqm_timer_set(rt_qos_aqm_timer_cfg_t aqm_timer_cfg);		

/* Function Name:
 *      rtk_fc_qos_aqm_flow_statistic_get
 * Description:
 *      Get flow statistic
 * Input:
 *      flow_idx
 * Output:
 *      pAqm_flow_stat 	- flow statistic
 * Return:
 *		0		- SUCCESS
 *		others	- error code
 * Note:
 *      The API can get flow statistic
 */
int rtk_fc_qos_aqm_flow_statistic_get(rt_qos_aqm_flow_stat_t* pAqm_flow_stat);


//PIE
/* Function Name:
 *      rtk_fc_qos_aqm_pie_enable_get
 * Description:
 *      Get configuration of aqm pie enable
 * Input:
 *      
 * Output:
 *      pAqmPie_enable 	- Configuration of aqm pie enable
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie enable
 */
int32 rtk_fc_qos_aqm_pie_enable_get(rt_qos_aqm_pie_enable_t* pAqmPie_enable);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_enable_set
 * Description:
 *      Set configuration of aqm pie enable
 * Input:
 *      aqmPie_enable 	- Configuration of aqm pie enable
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie enable
 */
int32 rtk_fc_qos_aqm_pie_enable_set(rt_qos_aqm_pie_enable_t aqmPie_enable);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_match_get
 * Description:
 *      Get configuration of aqm pie match
 * Input:
 *      
 * Output:
 *      pAqmPie_match 	- Configuration of aqm pie match
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie match
 */
int32 rtk_fc_qos_aqm_pie_match_get(rt_qos_aqm_pie_match_t* pAqmPie_match);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_match_set
 * Description:
 *      Set configuration of aqm pie match
 * Input:
 *      aqmPie_match 	- Configuration of aqm pie match
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie match
 */
int32 rtk_fc_qos_aqm_pie_match_set(rt_qos_aqm_pie_match_t aqmPie_match);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_mapping_get
 * Description:
 *      Get configuration of aqm pie mapping
 * Input:
 *      
 * Output:
 *      pAqmPie_mapping 	- Configuration of aqm pie mapping
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie mapping
 */
int32 rtk_fc_qos_aqm_pie_mapping_get(rt_qos_aqm_pie_mapping_t* pAqmPie_mapping);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_mapping_set
 * Description:
 *      Set configuration of aqm pie mapping
 * Input:
 *      aqmPie_mapping 	- Configuration of aqm pie mapping
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie mapping
 */
int32 rtk_fc_qos_aqm_pie_mapping_set(rt_qos_aqm_pie_mapping_t aqmPie_mapping);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_threshold_get
 * Description:
 *      Get configuration of aqm pie threshold
 * Input:
 *      
 * Output:
 *      pAqmPie_threshold 	- Configuration of aqm pie threshold
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie threshold
 */
int32 rtk_fc_qos_aqm_pie_threshold_get(rt_qos_aqm_pie_threshold_t* pAqmPie_threshold);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_threshold_set
 * Description:
 *      Set configuration of aqm pie threshold
 * Input:
 *      aqmPie_threshold 	- Configuration of aqm pie threshold
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie threshold
 */
int32 rtk_fc_qos_aqm_pie_threshold_set(rt_qos_aqm_pie_threshold_t aqmPie_threshold);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_dpScale_get
 * Description:
 *      Get configuration of aqm pie dpScale
 * Input:
 *      
 * Output:
 *      pAqmPie_dpScale 	- Configuration of aqm pie dpScale
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie dpScale
 */
int32 rtk_fc_qos_aqm_pie_dpScale_get(rt_qos_aqm_pie_dpScale_t* pAqmPie_dpScale);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_dpScale_set
 * Description:
 *      Set configuration of aqm pie dpScale
 * Input:
 *      aqmPie_dpScale 	- Configuration of aqm pie dpScale
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie dpScale
 */
int32 rtk_fc_qos_aqm_pie_dpScale_set(rt_qos_aqm_pie_dpScale_t aqmPie_dpScale);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_dpThreshold_get
 * Description:
 *      Get configuration of aqm pie dpThreshold
 * Input:
 *      
 * Output:
 *      pAqmPie_dpThreshold 	- Configuration of aqm pie dpThreshold
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie dpThreshold
 */
int32 rtk_fc_qos_aqm_pie_dpThreshold_get(rt_qos_aqm_pie_dpThreshold_t* pAqmPie_dpThreshold);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_dpThreshold_set
 * Description:
 *      Set configuration of aqm pie dpThreshold
 * Input:
 *      aqmPie_dpThreshold 	- Configuration of aqm pie dpThreshold
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie dpThreshold
 */
int32 rtk_fc_qos_aqm_pie_dpThreshold_set(rt_qos_aqm_pie_dpThreshold_t aqmPie_dpThreshold);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_dpBurst_get
 * Description:
 *      Get configuration of aqm pie dpBurst
 * Input:
 *      
 * Output:
 *      pAqmPie_dpBurst 	- Configuration of aqm pie dpBurst
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie dpBurst
 */
int32 rtk_fc_qos_aqm_pie_dpBurst_get(rt_qos_aqm_pie_dpBurst_t* pAqmPie_dpBurst);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_dpBurst_set
 * Description:
 *      Set configuration of aqm pie dpBurst
 * Input:
 *      aqmPie_dpBurst 	- Configuration of aqm pie dpBurst
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie dpBurst
 */
int32 rtk_fc_qos_aqm_pie_dpBurst_set(rt_qos_aqm_pie_dpBurst_t aqmPie_dpBurst);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_qdelay_get
 * Description:
 *      Get configuration of aqm pie qdelay
 * Input:
 *      
 * Output:
 *      pAqmPie_qdelay 	- Configuration of aqm pie qdelay
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie qdelay
 */
int32 rtk_fc_qos_aqm_pie_qdelay_get(rt_qos_aqm_pie_qdelay_t* pAqmPie_qdelay);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_qdelay_set
 * Description:
 *      Set configuration of aqm pie qdelay
 * Input:
 *      aqmPie_qdelay 	- Configuration of aqm pie qdelay
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie qdelay
 */
int32 rtk_fc_qos_aqm_pie_qdelay_set(rt_qos_aqm_pie_qdelay_t aqmPie_qdelay);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_rate_get
 * Description:
 *      Get configuration of aqm pie rate
 * Input:
 *      
 * Output:
 *      pAqmPie_rate 	- Configuration of aqm pie rate
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie rate
 */
int32 rtk_fc_qos_aqm_pie_rate_get(rt_qos_aqm_pie_rate_t* pAqmPie_rate);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_rate_set
 * Description:
 *      Set configuration of aqm pie rate
 * Input:
 *      aqmPie_rate 	- Configuration of aqm pie rate
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie rate
 */
int32 rtk_fc_qos_aqm_pie_rate_set(rt_qos_aqm_pie_rate_t aqmPie_rate);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_cfg_get
 * Description:
 *      Get configuration of aqm pie cfg
 * Input:
 *      
 * Output:
 *      pAqmPie_cfg 	- Configuration of aqm pie cfg
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie cfg
 */
int32 rtk_fc_qos_aqm_pie_cfg_get(rt_qos_aqm_pie_cfg_t* pAqmPie_cfg);

/* Function Name:
 *      rtk_fc_qos_aqm_pie_cfg_set
 * Description:
 *      Set configuration of aqm pie cfg
 * Input:
 *      aqmPie_cfg 	- Configuration of aqm pie cfg
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie cfg
 */
int32 rtk_fc_qos_aqm_pie_cfg_set(rt_qos_aqm_pie_cfg_t aqmPie_cfg);
#endif

#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
/* Function Name:
 *      rtk_fc_pe_http_test
 * Description:
 *      Set configuration of pe http test
 * Input:
 *      http_test_req	- Configuration of http test request 
 * Output:
 *		pHttp_test_result
 * Return:
 *	   0			  - SUCCESS
 *	   others		  - error code
 * Note:
 *      The API can set configuration of pe http test
 */
int rtk_fc_pe_http_test(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result);
#endif
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
/* Function Name:
 *      rtk_fc_pe_crypto_test
 * Description:
 *      Set configuration of pe crypto test
 * Input:
 *      crypto_req	- Configuration of crypto test request 
 *	  fc_saInfo_idx - saInfo index of FC (for add cmd)
 *	  wait_pe_return -wait for return value of pe (sync send)
 * Output:
 *		
 * Return:
 *	  please refer to enum rt_pe_ret_t
 * Note:
 *      The API can set configuration of pe crypto test
 */
rt_pe_ret_t rtk_fc_pe_crypto_test(rt_pe_crypto_request_t crypto_req, int32 fc_saInfo_idx, uint32 wait_pe_return);
#endif

int rtk_fc_crytpo_key_set(rt_crypto_key_set_t *rt_crypto_key_set_cfg, int *key_index);

int rtk_fc_crytpo_key_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg);

int rtk_fc_crytpo_key_del_by_index(int *key_index);


int rtk_fc_crytpo_key_get_index(int *key_index);
int rtk_fc_crytpo_keyhash_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg);

int rtk_fc_crytpo_keyhash_del_by_index(int *key_index);


int rtk_fc_crytpo_keyhash_get_index(int *key_index);

#if defined(CONFIG_RTK_SOC_RTL8198D)
/* Function Name:
*	   rtk_fc_hwFlowMib_get
* Description:
*	   get hw flow counters by hw flow mib index.
* Input:
*	   idx			  - flow mib index
* Output:
*	   pflowMibCnt	  - flow counters (include ingress/egress packet count and byte count).
* Return:
*	   0			  - SUCCESS
*	   others		  - error code
*/
int rtk_fc_hwFlowMib_get(int idx, rt_stat_flow_mib_t *pflowMibCnt);

/* Function Name:
 *		rtk_fc_brNetifInfo_get
 * Description:
 *		get bridge netif info.
 * Input:
 *		br_name		- bridge netif name.
 * Output:
 *		br_mac		- bridge netif mac address.
 *		br_ip		- bridge netif ip address.
 * Return:
 *		0			  			- SUCCESS
 *		others					- error code
 * Note:
 *		The API can get bridge netif info.
 */
int rtk_fc_brNetifInfo_get(char *br_name, rtk_mac_t *br_mac, uint32 *br_ip);

#endif

int rtk_fc_rt_mapper_api_init(void);

#endif
