#ifndef _IGMPPROXY_H_
#define _IGMPPROXY_H_

#include "mclab.h"
#include "timeout.h"
#include <fcntl.h>
#include <signal.h>
#ifdef EMBED
#include <rtk/options.h>
#endif
#ifdef CONFIG_RTK_LIB_MIB
#include <rtk/sysconfig.h>
#endif

struct vlan_pair {
	unsigned short vid_a;
	unsigned short vid_b;
};

//add function declaration on it instead of include header. Becaue there is redeclaration issue
extern int isIGMPSnoopingEnabled(void);
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
extern int rtk_fc_multicast_flow_add(unsigned int group);
extern int rtk_fc_multicast_flow_delete(unsigned int group);
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
#define  MAXWAN 8
#else 
#define  MAXWAN 1
#endif

// Send Group-specific query periodically (keepalive polling)
#define PERIODICAL_SPECIFIC_QUERY
// Send Group-specific query on leave (fast leave)
//#define LEAVE_SPECIFIC_QUERY
// Maintain the group members in order to do immediately leave
#define KEEP_GROUP_MEMBER
// Send IGMP General Query periodically
#define SEND_GENERAL_QUERY

#if defined(CONFIG_YUEME) || defined(CONFIG_CU)
#define SEND_UNSOLICIT_REPORT
#endif
//#define SEND_UNSOLICIT_REPORT

/* IGMP timer and default values */
//#define LAST_MEMBER_QUERY_INTERVAL	1	// second
//#define LAST_MEMBER_QUERY_COUNT		2
//#define QUERY_RESPONSE_INTERVAL		10
// Kaohj --- group-specific query in periodical
#ifdef PERIODICAL_SPECIFIC_QUERY
#define MEMBER_QUERY_INTERVAL		30	// second
#define MEMBER_QUERY_COUNT		3
#endif
//#define GENERAL_QUERY_INTERVAL		15	// second
#define UNSOLT_REPORT_INTERVAL   30  //second

#define TIMER_GENERAL_QUERY		1
#define TIMER_DELAY_QUERY		2
#define TIMER_UNSOLT_REPORT  3

/* IGMP group address */
#define ALL_SYSTEMS			htonl(0xE0000001)	// General Query - 224.0.0.1
#define ALL_ROUTERS			htonl(0xE0000002)	// Leave - 224.0.0.2	
#define ALL_ROUTERS_V3			htonl(0xE0000016)	// Leave - 224.0.0.22
#define ALL_PRINTER			htonl(0xEFFFFFFA)	// notify all printer - 239.255.255.250
#define CLASS_D_MASK			0xE0000000		// the mask that defines IP Class D
#define IPMULTI_MASK			0x007FFFFF		// to get the low-order 23 bits

/* header length */
#define MIN_IP_HEADER_LEN		20
#define IGMP_MINLEN			8

/* IGMP v3 type */
#define IGMP_HOST_V3_MEMBERSHIP_REPORT	0x22

extern int general_query_interval;
extern int query_response_interval;
extern int last_member_query_count;
extern int robust_count;
extern int last_member_query_interval;

// group member entry
struct mbr_entry {
	struct mbr_entry	*next;
	__u32			user_addr;
};

struct igmp_timer {
	int		type;			// timer type

	union
	{
		int		retry_left;		// retry counts left
		int   unsol_report_count;   //unsolicit report count 
	};
	struct callout	ch;
#ifdef _SUPPORT_IGMPV3_
	unsigned int	lefttime;
#endif /*_SUPPORT_IGMPV3_*/
};

#ifdef _SUPPORT_IGMPV3_
struct src_entry {
	struct src_entry	*next;
	__u32			srcaddr;
	struct igmp_timer	timer;
};
#endif /*_SUPPORT_IGMPV3_*/

struct mcft_entry {
	struct mcft_entry 	*next;
	__u32			grp_addr;
	__u32			user_count;	// group member count
	struct mbr_entry 	*grp_mbr;
	struct igmp_timer	timer;
#ifdef _SUPPORT_IGMPV3_
	int			igmp_ver;
	int			filter_mode;
	struct src_entry	*srclist;
	int			mrt_state; //0:disable, 1: enable all sources
#endif /*_SUPPORT_IGMPV3_*/
#ifdef MULTICAST_LANWAN_BIND
	struct IfDesc *bind_up[MAXWAN];
	int bind_up_num;
#endif
};

#ifdef MULTICAST_LANWAN_BIND
#define IP_PKTINFO_RTK 		100

struct in_pktinfo_rtk {
	int		ipi_ifindex;
	struct in_addr	ipi_spec_dst;
	struct in_addr	ipi_addr;
	int 	from_dev_ifindex;
	int 	from_dev_ifindex_valid;
};
#endif


#ifdef _SUPPORT_IGMPV3_
/*IGMP version*/
#define IGMP_VER_2		2
#define IGMP_VER_3		3
extern int igmpv3_accept(int recvlen, struct IfDesc *dp, int flag);
extern void igmpv3_timer(void);
#endif /*_SUPPORT_IGMPV3_*/

extern char igmp_down_if_name[IFNAMSIZ];
extern char igmp_down_if_idx;

#ifdef CONFIG_IGMPPROXY_MULTIWAN
extern char igmp_up_if_name[MAXWAN][IFNAMSIZ];
#else
extern char igmp_up_if_name[IFNAMSIZ];
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
extern char igmp_up_if_idx[MAXWAN];
#else
extern char igmp_up_if_idx;
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
extern int igmp_up_if_num;
#endif

extern char *recv_buf;
extern char *send_buf;

extern struct mcft_entry *mcpq;

extern int igmp_query(__u32 dst, __u32 grp,__u8 mrt);

extern struct mcft_entry * add_mcft(__u32 grp_addr, __u32 src_addr);
extern int del_mcft(struct mcft_entry *mymcp);
extern int chk_mcft(__u32 grp_addr);
extern struct mcft_entry * get_mcft(__u32 grp_addr);

extern void k_set_ttl(int t);
extern void k_set_loop(int l);
extern void k_set_if(unsigned int ifa);
extern void k_set_IP_router_alert();
extern int chk_local( __u32 addr );
extern unsigned long getAddrByVifIx(int ix);
extern void updateIfVc();
extern void calltimeout();
extern int add_user(struct mcft_entry *mcp, __u32 src);
extern int del_user(struct mcft_entry *mcp, __u32 src);
extern unsigned short in_cksum(unsigned short *addr, int len);
extern int igmp_leave(__u32 grp, int if_idx);

#ifdef MULTICAST_LANWAN_BIND
int rtk_is_mcft_equal(struct mcft_entry *mymcp0,struct IfDesc **bind_up,int bind_up_num);
int rtk_get_binding_wan_info(char *name,unsigned int *ifindex,unsigned short *vid);
int rtk_is_bind_interface(struct IfDesc *up,struct mcft_entry *mymcp);
#endif

#ifdef CONFIG_DEV_xDSL
void RTK_Delete_HW_McastFlow(struct mcft_entry *mcp);
#endif

#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
struct igmpcfg{
	char *cfg_path;
	int query_interval;
	int query_response_interval;
	int last_member_query_count;
	int robust;
	int group_leave_delay;
	int fast_leave;
};
#endif

#define FROM_WAN 1
#define FROM_LAN 0

#endif /*_IGMPPROXY_H_*/
