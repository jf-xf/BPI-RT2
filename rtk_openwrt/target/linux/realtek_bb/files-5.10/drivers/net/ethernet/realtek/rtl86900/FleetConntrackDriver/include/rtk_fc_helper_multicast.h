/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/
	
#ifndef __RTK_FC_HELPER_MULTICAST_H__
#define __RTK_FC_HELPER_MULTICAST_H__

#include <rtk_fc_helper.h>
#include <rtk_fc_port.h>

#if defined(CONFIG_FC_RTL8198F_SERIES)
#include <mcast.h>
#endif
//#include <soc/cortina/rtk_common_utility.h>





#define IS_8367_LAN_PHY_PORT 1
#define IS_8226_LAN_PHY_PORT 0


//#define CONFIG_RTK_USER_GROUP_SUPPORT

#define MAX_USER_GROUP_NUMBER 128

#define DEL_USER_GROUP_FLAG	2
#define ADD_USER_GROUP_FLAG	1

#define IP4_USER_GROUP_FLAG 0
#define IP6_USER_GROUP_FLAG 1


typedef struct rtk_fc_userGroup_s
{
	unsigned short ipVersion;
	unsigned short valid;
	unsigned int groupAddr[4];
}rtk_fc_userGroup_t;

extern rtk_fc_userGroup_t userGroupList[MAX_USER_GROUP_NUMBER];

typedef struct rtk_fc_mcPort_s
{
	uint32	isWlan:1;		//0:reference to phyPortIdx 1:reference to wlanDevMbssid
	uint32	dmacTrans:1;
	uint32	portId:8;		// if isWlan portId referenct to rtk_fc_wlan_devidx_t
	int16	ifidx;		// if invalid init to FAIL 
	uint8	dmac[6];
} rtk_fc_mcPort_t;


typedef struct rtk_fc_table_mcConfigTbl_s
{
	uint32	confGroup[4];
	uint32	confSource[4];
	int16	confIgrCvid;	//-1 UNTAG
	int16	confIgrSvid;	//-1 UNTAG
	//uint16	streamId;

	//uint8	valid:1;
	uint8 	isIpv6:1;
	uint8	careSource:1;
	uint8	careIngressSvid:1;	
	uint8	careIngressCvid:1;
	//uint8	careIngressStreamId:1;
	uint8	copy2cpu:1;
	uint8	do_delete:1;			//kernel mode
	uint8	dftGrpFwd:1;			//this is default forward entry
	uint8	cnt_egrInfo;
	rtk_fc_mcPort_t	 egrInfo[RT_MCCFG_MAX_NUM];	//
	struct list_head lsMcConfigEntry;
} rtk_fc_table_mcConfigTbl_t;


#if defined(CONFIG_RTK_SOC_RTL8198D)
int _rtk_fc_check_user_group(unsigned char * skbData);
int _rtk_fc_add_lutMac(unsigned int*_group ,unsigned short isIpv6);
#endif


typedef int (*fc_igmp_mdb_callback)(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg);
extern int rtk_fc_igmp_mdb_callback_register(fc_igmp_mdb_callback rxfunc);
extern int rtk_fc_igmp_mdb_callback_unregister(void);
extern int _rtk_fc_igmp_mdb_callback_register(fc_igmp_mdb_callback rxfunc);
extern int _rtk_fc_igmp_mdb_callback_unregister(void);

extern int rtk_fc_igmp_mdb_callback_handler(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg);
extern int rtk_fc_igmp_mdb_isRegister(void);


extern int rtk_fc_set_user_group(unsigned int* groupAddr, unsigned short action,unsigned short ipVersion );
extern int rtk_fc_get_user_group_by_index(unsigned int index,unsigned short *valid, unsigned int* groupAddress, unsigned short* ipVersion);
extern int rtk_fc_get_user_group(unsigned int* groupAddress, unsigned short ipVersion);
extern int rtk_check_user_group_flow(unsigned int*groupip,unsigned short isIpv6,unsigned int* sourceip,unsigned int sourcePort,unsigned int destPort);
extern int rtk_fc_check_user_group(unsigned char * skbData);
extern int rtk_fc_del_user_group(unsigned int*_group ,unsigned short isIpv6);
extern int rtk_fc_add_user_group(unsigned int*_group ,unsigned short isIpv6);
extern int rtk_fc_add_lutMac(unsigned int*_group ,unsigned short isIpv6);



extern unsigned int (*rtl_get_8226_lan_dev_portnum_hook)(void );
extern unsigned int (*rtl_get_portnum_in_8367_hook)(unsigned int portnum);


extern int rtk_l2_8367_igmp_protocol_init(void);
extern int rtk_l2_8367_ipMcastAddr_add(u32 dip, u32 sip, u32 portmask);
extern int rtk_l2_ipMcastGroupEntry_add(ipaddr_t ip_addr, unsigned int vid, rtk_portmask_t *pPortmask);
extern int rtk_l2_ipMcastGroupEntry_del(ipaddr_t ip_addr, unsigned int vid);
extern int rtk_l2_8367_ipMcastAddr_del(u32 dip, u32 sip);
extern int rtk_l2_8367_trapUnkownMC_froward(void);
extern int rtk_mirror_port0_set(void);
extern int rtk_l2_8367_mcastAddr_add(char *dmac,u32 portmask, u32 vlan);
extern int rtk_l2_8367_mcastAddr_del(char *dmac, u32 vid);
extern int rtk_l2_8367_ipMcastAddr_get(u32 dip, u32 sip ,rtk_portmask_t* portmask);


typedef struct rtk_fc_l2_mcast_members_s
{
	uint32 in_ctagif;
	uint32 in_src_ipv4_addr;
	uint32 in_dst_ipv4_addr;
	uint32 addr32[4];
	uint32 out_portmask;
	uint32 out_user_priority;
	uint32 out_cpri;
	uint32 out_cvlan_id;
	uint32 out_cvid_format_act;
	uint32 out_user_pri_act;
	uint32 out_smac_trans;
}rtk_fc_l2_mcast_members_t;



#if defined(CONFIG_FC_RTL8198F_SERIES)

int rtk_l2_set_8367_ipMcastAddr(rtk_fc_l2_mcast_members_t *pP3, unsigned int l2mcgid,unsigned int wan_portMask,unsigned int ipv6);
int rtk_fc_get_portNumber_in_8367(unsigned int portnum,unsigned int wan_portMask);
int rtk_fc_check_8367Port(u32 port_number,u32 wan_portMask);
int rtk_l2_del_8367_ipMcastAddr(u32 dip, u32 sip, u32 vid);
int rtk_l2_del_8367_ip6McastAddr(u32 * mcAddr,u32 vid);
#endif





#endif //__RTK_FC_HELPER_MULTICAST_H__

