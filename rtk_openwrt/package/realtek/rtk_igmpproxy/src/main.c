#include "igmpproxy.h"
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef MULTICAST_LANWAN_BIND
#ifdef CONFIG_RTK_LIB_MIB
#include <rtk/mib.h>
#include <rtk/sysconfig.h>
#include <rtk/rtk_interface.h>
#endif
#endif

volatile int reset_mrouter_flag = 0;
volatile int update_flag = 0;
volatile int ifup_flag = 0;

#ifdef SEND_GENERAL_QUERY
struct igmp_timer general_query_timer;
#endif
#ifdef SEND_UNSOLICIT_REPORT
struct igmp_timer unsolicit_report_timer;
#endif
struct igmp_timer qtimer;

#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
struct igmpcfg igmpcfg_setting;
#endif

int IGMPCurrentVersion = 3;
static int IGMPVersionConfig = 1;
int IGMPPROXY_Debug_Set = 0;


int general_query_interval= 15;
int query_response_interval = 100;
int last_member_query_count = 2;
int robust_count = 2;
int last_member_query_interval = 100;
#if defined(CONFIG_CMCC)
int fast_leave = 1;
#else
int fast_leave = 0;
#endif
int fifofd = -1;

// Kaohj -- toggle IGMP rx
int IGMP_rx_enable;
int MRouter_created;
void igmp_timer_expired(void *arg);
void igmp_specific_timer_expired(void *arg);
void igmp_timer_unsolicit_report(void * arg);
int igmp_queryV3(__u32 dst, __u32 grp,__u8 mrt);
int igmp_report(__u32 dst, int if_idx);
int igmp_leave(__u32 grp, int if_idx);

int remove_fifo(void);
void reload_igmp_cfg_file(int signum);

#if __DEBUG__
static void pkt_debug(const char *buf)
{
int num2print = 20;
int i = 0;
	if(buf[0]==0x46)
		num2print = 24;
	for (i = 0; i < num2print; i++) {
		printf("%2.2x ", 0xff & buf[i]);
	}
	printf("\n");
	num2print = buf[3];
	for (; i < num2print; i++) {
		printf("%2.2x ", 0xff & buf[i]);
	}
	printf("\n");
}
#else
#define pkt_debug(buf)	do {} while (0)
#endif

//static int  igmp_id = 0;

char igmp_down_if_name[IFNAMSIZ];
char igmp_down_if_idx;

#ifdef CONFIG_IGMPPROXY_MULTIWAN
char igmp_up_if_name[MAXWAN][IFNAMSIZ];
#else
char igmp_up_if_name[IFNAMSIZ];
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
char igmp_up_if_idx[MAXWAN];
#else
char igmp_up_if_idx;
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_up_if_num;
#endif


#ifdef MULTICAST_LANWAN_BIND
struct IfDesc *bind_up[MAXWAN];
int bind_up_num = 0;
#endif

struct mcft_entry *mcpq = NULL;


void setIGMPVersionToV2(void)
{
    if((IGMPVersionConfig==1) && (IGMPCurrentVersion!=2))
    {
        IGMPCurrentVersion =2;
        printf("Detected IGMP V2 report, config IGMP Query to V2!\n");
    }
}

//int add_mcft(__u32 grp_addr)
struct mcft_entry * add_mcft(__u32 grp_addr, __u32 src_addr)
{
	struct mcft_entry *mcp;
#ifdef KEEP_GROUP_MEMBER
	struct mbr_entry *gcp;
#endif
	mcp = malloc(sizeof(struct mcft_entry));
	if(!mcp)
		return 0;
#ifdef KEEP_GROUP_MEMBER
	gcp = malloc(sizeof(struct mbr_entry));
	if (!gcp) {
		free(mcp);
		return 0;
	}
#endif
	mcp->grp_addr = grp_addr;
	// Kaohj -- add the first member
#ifdef KEEP_GROUP_MEMBER
	mcp->user_count = 1;
	gcp->user_addr = src_addr;
	gcp->next = NULL;
	mcp->grp_mbr = gcp;
#endif

#ifdef _SUPPORT_IGMPV3_
	mcp->igmp_ver = IGMP_VER_3;
	mcp->filter_mode = MCAST_INCLUDE;
	mcp->srclist = NULL;
	mcp->timer.lefttime = last_member_query_interval;
	mcp->timer.retry_left = last_member_query_count;
	mcp->mrt_state = 0;
#endif /*_SUPPORT_IGMPV3_*/

#ifdef MULTICAST_LANWAN_BIND
	memcpy(mcp->bind_up,bind_up,sizeof(struct IfDesc*)*bind_up_num);
	mcp->bind_up_num = bind_up_num;
#endif

	mcp->next = mcpq;
	mcpq = mcp;
	//return 0;
	return mcp;
}

// Kaohj --- add group timer for IGMP group-specific query.
int add_mcft_timer(struct mcft_entry *mcp)
{
	struct mcft_entry *p;

	/* Remove the entry from the  list. */
	for (p = mcpq; p!=0; p = p->next) {
		if(p->grp_addr == mcp->grp_addr
#ifdef MULTICAST_LANWAN_BIND
			&& rtk_is_mcft_equal(p,mcp->bind_up,mcp->bind_up_num)
#endif
		) {
#ifdef PERIODICAL_SPECIFIC_QUERY
			p->timer.retry_left = last_member_query_count;
			timeout(igmp_specific_timer_expired , p, MEMBER_QUERY_INTERVAL, &p->timer.ch);
#endif
			return 0;
		}
	}
	return -1;
}

#ifdef _SUPPORT_IGMPV3_
extern int igmp_del_mr( struct mcft_entry *mymcp,__u32 src );
#endif
int del_mcft(struct mcft_entry *mymcp)
{
	__u32 grp_addr = mymcp->grp_addr;
	struct mcft_entry **q, *p;
#ifdef KEEP_GROUP_MEMBER
	struct mbr_entry *gt, *gc;
#endif


	/* Remove the entry from the  list. */
	for (q = &mcpq; (p = *q); q = &p->next) {
		if(p->grp_addr == grp_addr
#ifdef MULTICAST_LANWAN_BIND
			&& rtk_is_mcft_equal(p,mymcp->bind_up,mymcp->bind_up_num)
#endif
		) {
			*q = p->next;
			// Kaohj -- free member list
#ifdef KEEP_GROUP_MEMBER
			gc = p->grp_mbr;
			while (gc) {
				gt = gc->next;
				free(gc);
				gc = gt;
			}
#endif
#ifdef _SUPPORT_IGMPV3_
			{
			  struct src_entry *s, *sn;
			  s=p->srclist;
			  while(s)
			  {
			  	sn=s->next;
				igmp_del_mr( mymcp, s->srcaddr );
			  	free(s);
			  	s=sn;
			  }
			}
#endif
			untimeout(&p->timer.ch);
			free(p);
			return 0;
		}
	}
	return -1;
}

// Kaohj --- delete group timer for IGMP group-specific query.
int del_mcft_timer(struct mcft_entry *mcp)
{
	struct mcft_entry *p;

	/* Remove the entry from the  list. */
	for (p = mcpq; p!=0; p = p->next) {
		if(p->grp_addr == mcp->grp_addr
#ifdef MULTICAST_LANWAN_BIND
			&& rtk_is_mcft_equal(p,mcp->bind_up,mcp->bind_up_num)
#endif
		) {
			untimeout(&p->timer.ch);
			return 0;
		}
	}
	return -1;
}

int chk_mcft(__u32 grp_addr)
{
struct mcft_entry *mcp = mcpq;
	while(mcp) {
		if(mcp->grp_addr == grp_addr
#ifdef MULTICAST_LANWAN_BIND
			&& rtk_is_mcft_equal(mcp,bind_up,bind_up_num)
#endif
		)
			return 1;
		mcp = mcp->next;
	}
	return 0;
}

struct mcft_entry * get_mcft(__u32 grp_addr)
{
struct mcft_entry *mcp = mcpq;
	while(mcp) {
		if(mcp->grp_addr == grp_addr
#ifdef MULTICAST_LANWAN_BIND
			&& rtk_is_mcft_equal(mcp,bind_up,bind_up_num)
#endif
		)
			return mcp;
		mcp = mcp->next;
	}
	return NULL;
}

int show_mcft_list(char *arg) {
	struct mcft_entry *mcp = mcpq;
	struct in_addr gaddr;
	struct in_addr tmp_addr;
	struct mbr_entry *gs;
	
	if (arg == NULL) {
		printf("\nmcpq list:\n");
		printf("\t\tGroup Address \tuser count\n");
		printf("\t\t------------- \t----------\n");
		while(mcp) {
			tmp_addr.s_addr = mcp->grp_addr;
			printf("\t\t%s \t%d\n", inet_ntoa(tmp_addr), mcp->user_count);
			mcp = mcp->next;
		}
		
		printf("\n");
	}
	else {
		gaddr.s_addr = 0;
		if (inet_aton(arg, &gaddr)==0)
			printf("Invalid Address %s\n", arg);
		printf("Group %s\n", arg);
		for (mcp=mcpq; mcp!=0; mcp=mcp->next) {
			if (mcp->grp_addr == gaddr.s_addr) {
				printf("Subscribers: %d\n", mcp->user_count);
				gs = mcp->grp_mbr;
				while (gs) {
					tmp_addr.s_addr = gs->user_addr;
					printf("\t\t%s\n", inet_ntoa(tmp_addr));
					gs = gs->next;
				}
				break;
			}
		}
		printf("\n");
	}
	return 0;
}

int num_mcft(void)
{
struct mcft_entry *mcp = mcpq;
int n = 0;
	while(mcp) {
		n++;
		mcp = mcp->next;
	}
	return n;
}

#ifdef KEEP_GROUP_MEMBER
// Kaohj -- attach user to group member list
//	0: fail
//	1: duplicate user
//	2: added successfully
int add_user(struct mcft_entry *mcp, __u32 src)
{
	struct mbr_entry *gcp;

	// check user
	gcp = mcp->grp_mbr;
	while (gcp) {
		if (gcp->user_addr == src)
			return 1;	// user exists
		gcp = gcp->next;
	}

	// add user
	gcp = malloc(sizeof(struct mbr_entry));
	if (!gcp) {
		return 0;
	}
	gcp->user_addr = src;
	gcp->next = mcp->grp_mbr;
	mcp->grp_mbr = gcp;
	mcp->user_count++;
	return 2;
}

// Kaohj -- remove user from group member list
// return: user count
int del_user(struct mcft_entry *mcp, __u32 src)
{
	struct mbr_entry **q, *p;

	/* Remove the entry from the  list. */
	q = &mcp->grp_mbr;
	p = *q;
	while (p) {
		if(p->user_addr == src) {
			*q = p->next;
			free(p);
			mcp->user_count--;
			return mcp->user_count;
		}
		q = &p->next;
		p = p->next;
	}

	return mcp->user_count;
}
#endif

/*
 * u_short in_cksum(u_short *addr, int len)
 *
 * Compute the inet checksum
 */
unsigned short in_cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    answer = ~sum;
    return (answer);
}

fd_set in_fds;		/* set of fds that wait_input waits for */
int max_in_fd;		/* highest fd set in in_fds */

/*
 * add_fd - add an fd to the set that wait_input waits for.
 */
void add_fd(int fd)
{
    FD_SET(fd, &in_fds);
    if (fd > max_in_fd)
	max_in_fd = fd;
}

/*
 * remove_fd - remove an fd from the set that wait_input waits for.
 */
void remove_fd(int fd)
{
    FD_CLR(fd, &in_fds);
}

/////////////////////////////////////////////////////////////////////////////
//	22/04/2004, Casey
/*
	Modified the following items:
	1.	delete all muticast router functions, xDSL router never use such function
	2.	igmp_handler only accept message for IGMP PROXY
	3.	IGMP proxy keep track on multicast address by mcft table,
		not multicast router module.

	igmp_handler rule:
	1.	only accept IGMP query from upstream interface, and it trigger
		downstream interface to send IGMP query.
	2.	only accept IGMP report from downstream interface, and it trigger
		upstream interface to send IGMP report.
	3.	when received IGMP report, recorded its group address as forwarding rule.
	4.	only accept IGMP leave from downstream interface, downstream interface
		will send IGMP general query twice to make sure there is no other member.
		If it cannot find any member, upstream interface will send IGMP leave.

	forwarding rule:
	1.	system only forward multicast packets from upstream interface to downstream interface.
	2.	system only forward multicast packets which group address learned by IGMP report.

*/
/////////////////////////////////////////////////////////////////////////////
//

#define RECV_BUF_SIZE	2048
char *recv_buf, *send_buf;
int igmp_socket;	/* down */
int igmp_socket2;	/* up */

#define IGMP_RESV_ADDR1 ("224.0.0.2")
#define IGMP_RESV_ADDR2 ("224.0.0.22")
/*
 * Add 224.0.0.2 (all routers) and 224.0.0.22 (IGMPv3) membership
 * on downstream interface in order to receive IGMP messages.
 */
static int add_IGMPMembership()
{
	struct IfDesc *dp;
	struct ip_mreq imr;	/* Ip multicast membership */
	
	dp = getIfByName(igmp_down_if_name);
	if(dp==NULL)
		return 0;
	
	/* setting imr structure */
	imr.imr_multiaddr.s_addr = inet_addr(IGMP_RESV_ADDR1);
	/*imr.imr_interface.s_addr = htonl(INADDR_ANY);*/
	imr.imr_interface.s_addr = dp->InAdr.s_addr;
	//printf("add membership at %s\n", inet_ntoa(dp->InAdr));
	
	if (setsockopt(MRouterFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq)) < 0)
	{
		igmpproxy_log(LOG_ERR,0, "%s(%d): IGMP IP_ADD_MEMBERSHIP %s failed.", __FUNCTION__, __LINE__, inet_ntoa(imr.imr_multiaddr));
		return -1;
	}
	
	imr.imr_multiaddr.s_addr = inet_addr(IGMP_RESV_ADDR2);
	if (setsockopt(MRouterFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq)) < 0)
	{
		igmpproxy_log(LOG_ERR,0, "%s(%d): IGMP IP_ADD_MEMBERSHIP %s failed.", __FUNCTION__, __LINE__, inet_ntoa(imr.imr_multiaddr));
		return -1;
	}
	
	return 0;
}

int init_igmp(void)
{
    recv_buf = malloc(RECV_BUF_SIZE);
    send_buf = malloc(RECV_BUF_SIZE);

    k_set_IP_router_alert(); /* Set 'Router Alert' IP option*/
    k_set_ttl(1);       /* restrict multicasts to one hop */
    k_set_loop(0);      /* disable multicast loopback     */
    
    FD_ZERO(&in_fds);
    max_in_fd = 0;

#ifdef SEND_GENERAL_QUERY
	// Kaohj -- start general IGMP Query peridically
	general_query_timer.type = TIMER_GENERAL_QUERY;
	timeout(igmp_timer_expired , &general_query_timer, general_query_interval, &general_query_timer.ch);
#endif

#ifdef SEND_UNSOLICIT_REPORT
  unsolicit_report_timer.type = TIMER_UNSOLT_REPORT;
  timeout(igmp_timer_unsolicit_report, &unsolicit_report_timer, UNSOLT_REPORT_INTERVAL, &unsolicit_report_timer.ch);
#endif
	add_IGMPMembership(); // Add 224.0.0.2, 224.0.0.22 for IGMP receiving
	// Kaohj --- enable IGMP rx
	IGMP_rx_enable = 1;
	return 0;
}

void shut_igmp_proxy(void)
{
	/* all interface leave multicast group */
}
#ifdef CONFIG_IGMPPROXY_MULTIWAN
// Kaohj -- add multicast membership to upstream interface(s)
int add_membership(struct mcft_entry *mcp)
{
	__u32 group = mcp->grp_addr;
struct ip_mreq mreq;
struct IfDesc *up_dp ;
int index;
int ret=0;

	for(index=0;index<igmp_up_if_num;index++)
	{
		up_dp= getIfByName(igmp_up_if_name[index]);

		if(up_dp==NULL)
			continue;
		if (up_dp->InAdr.s_addr == 0)
			continue;
#ifdef MULTICAST_LANWAN_BIND
		if(!rtk_is_bind_interface(up_dp,mcp))
			continue;
#endif

		/* join multicast group */
		mreq.imr_multiaddr.s_addr = group;
		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(MRouterFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret) {
			char FmtBu0[ 32 ], FmtBu1[ 32 ];
			printf("%s(%d): setsockopt IP_ADD_MEMBERSHIP %s error on %s(%s)!\n", __FUNCTION__, __LINE__,
				inet_ntop(AF_INET, &mreq.imr_multiaddr.s_addr, FmtBu0, sizeof(FmtBu0)), igmp_up_if_name[index],
				inet_ntop(AF_INET, &mreq.imr_interface.s_addr, FmtBu1, sizeof(FmtBu1)));
			perror("Add Membership");
			return ret;
		}
	}
	return ret;
}

// Add MRoute
int add_mfc(struct mcft_entry *mcp, __u32 src)
{
	__u32 group = mcp->grp_addr;
	struct MRouteDesc	mrd;
	int index;
	struct IfDesc *up_dp ;

	for(index=0;index<igmp_up_if_num;index++) {
		up_dp= getIfByName(igmp_up_if_name[index]);

		if(up_dp==NULL)
			continue;	
#ifdef MULTICAST_LANWAN_BIND
		if(!rtk_is_bind_interface(up_dp,mcp))
			continue;
#endif
		/* add multicast routing entry */
		mrd.OriginAdr.s_addr = 0;
		// Kaohj --- special case, save the subscriber IP to kernel
		// in order to take the subscriber IP (source IP) to the upstream server
		mrd.SubsAdr.s_addr = src;
		mrd.McAdr.s_addr = group;
		mrd.InVif = igmp_up_if_idx[index];
		memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
		mrd.TtlVc[(unsigned char)igmp_down_if_idx] = 1;
		addMRoute(&mrd);
	}
	return 1;
}

// Add group membershipt and MRoute
int add_mr(struct mcft_entry *mcp, __u32 src)
{
	add_membership(mcp);
	add_mfc(mcp, src);
	return 1;
}

// Kaohj -- delete multicast membership to upstream interface(s)
int del_membership(struct mcft_entry *mcp)
{
	__u32 group = mcp->grp_addr;
struct ip_mreq mreq;
struct IfDesc *up_dp ;
int ret=0;
int index;

	for(index=0;index<igmp_up_if_num;index++){
		up_dp= getIfByName(igmp_up_if_name[index]);
		if(up_dp==NULL)
			continue;
		if (up_dp->InAdr.s_addr == 0)
			continue;
#ifdef MULTICAST_LANWAN_BIND
		if(!rtk_is_bind_interface(up_dp,mcp))
			continue;
#endif

		/* drop multicast group */
		mreq.imr_multiaddr.s_addr = group;
		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(MRouterFD, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret) {
			char FmtBu0[ 32 ], FmtBu1[ 32 ];
			printf("%s(%d): setsockopt IP_DROP_MEMBERSHIP %s error on %s(%s)!\n", __FUNCTION__, __LINE__,
				inet_ntop(AF_INET, &mreq.imr_multiaddr.s_addr, FmtBu0, sizeof(FmtBu0)), igmp_up_if_name[index],
				inet_ntop(AF_INET, &mreq.imr_interface.s_addr, FmtBu1, sizeof(FmtBu1)));
			perror("Delete Membership");
		}
	}
	return ret;
}

// Delete MRoute
int del_mfc(struct mcft_entry *mcp)
{
	__u32 group = mcp->grp_addr;
	struct MRouteDesc	mrd;
	int index;
#ifdef MULTICAST_LANWAN_BIND
	struct IfDesc *up_dp ;
#endif

	for(index=0;index<igmp_up_if_num;index++) {
#ifdef MULTICAST_LANWAN_BIND
		up_dp= getIfByName(igmp_up_if_name[index]);
		if(up_dp==NULL)
			continue;	

		if(!rtk_is_bind_interface(up_dp,mcp))
			continue;
#endif

		
		/* delete multicast routing entry */
		mrd.OriginAdr.s_addr = 0;
		mrd.McAdr.s_addr = group;
		mrd.InVif = igmp_up_if_idx[index];
		memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
		delMRoute(&mrd);
	}
	return 1;
}

// Delete group membership and MRoute
int del_mr(struct mcft_entry *mcp)
{
	del_membership(mcp);
	del_mfc(mcp);
	return 1;
}
#else

// Kaohj -- add multicast membership to upstream interface(s)
int add_membership(struct mcft_entry *mcp)
{
	__u32 group = mcp->group;
struct ip_mreq mreq;
struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
int ret;

	if(up_dp==NULL)
		return 0;
	/* join multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(MRouterFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret) {
		//printf("setsockopt IP_ADD_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
		return ret;
	}

	igmpproxy_log(LOG_INFO,0, "igmpproxy: Add membership %s", inet_ntoa(mreq.imr_multiaddr));
	return 1;
}

// Add MRoute
int add_mfc(struct mcft_entry *mcp, __u32 src)
{
	__u32 group = mcp->grp_addr;
	struct MRouteDesc	mrd;

	/* add multicast routing entry */
	mrd.OriginAdr.s_addr = 0;
	// Kaohj --- special case, save the subscriber IP to kernel
	// in order to take the subscriber IP (source IP) to the upstream server
	mrd.SubsAdr.s_addr = src;
	mrd.McAdr.s_addr = group;
	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	mrd.TtlVc[(uint8)igmp_down_if_idx] = 1;
	addMRoute(&mrd);
	return 1;
}

// Add group membershipt and MRoute
int add_mr(struct mcft_entry *mcp, __u32 src)
{
	add_membership(mcp);
	add_mfc(mcp, src);
	return 1;
}

// Kaohj -- delete multicast membership to upstream interface(s)
int del_membership(struct mcft_entry *mcp)
{
	__u32 group = mcp->grp_addr;
struct ip_mreq mreq;
struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
int ret;

	if(up_dp==NULL)
		return 0;
	/* drop multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(MRouterFD, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret)
		printf("setsockopt IP_DROP_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	igmpproxy_log(LOG_INFO,0, "igmpproxy: Drop membership %s", inet_ntoa(mreq.imr_multiaddr));
	return 1;
}

// Delete MRoute
int del_mfc(struct mcft_entry *mcp)
{
	__u32 group = mcp->grp_addr;
	struct MRouteDesc	mrd;

	/* delete multicast routing entry */
	mrd.OriginAdr.s_addr = 0;
	mrd.McAdr.s_addr = group;
	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	delMRoute(&mrd);
	return 1;
}

// Delete group membership and MRoute
int del_mr(struct mcft_entry *mcp)
{
	del_membership(mcp);
	del_mfc(mcp);
	return 1;
}
#endif

extern int igmp_del_group( struct mcft_entry *mcp);

void igmp_specific_timer_expired(void *arg)
{
struct mcft_entry *mcp = arg;
	int query_interval;

	//printf("igmp_specific_timer_expired()\n");
	if(!mcp)
		return;

	mcp->timer.retry_left--;

	if(mcp->timer.retry_left <= 0) {
		// Kaohj --- check if group has already been dropped
#ifdef KEEP_GROUP_MEMBER
		if (mcp->user_count == 0) {
#endif
			igmp_del_group(mcp);
			del_mr(mcp);
			del_mcft(mcp);
#ifdef KEEP_GROUP_MEMBER
		}
#endif
		untimeout(&mcp->timer.ch);
	}
	else {
		//printf( "<t>\n" );
		// Kaohj --- last mmber ?
		if (mcp->user_count==0) {
			query_interval = last_member_query_interval;
		}
		else {
			query_interval = last_member_query_interval*3;
		}
		timeout(igmp_specific_timer_expired, mcp, query_interval/IGMP_TIMER_SCALE, &mcp->timer.ch);
        if(IGMPCurrentVersion==3)
		    igmp_queryV3(ALL_SYSTEMS, mcp->grp_addr, query_interval);
        else
		    igmp_query(ALL_SYSTEMS, mcp->grp_addr, query_interval);
	}
}

// Kaohj --- send general Query
void igmp_timer_expired(void *arg)
{
	struct igmp_timer *pqtimer;

	pqtimer = (struct igmp_timer *)arg;
	if (pqtimer->type == TIMER_GENERAL_QUERY) {
		timeout(igmp_timer_expired, pqtimer, general_query_interval, &pqtimer->ch);
	}
	else if (pqtimer->type == TIMER_DELAY_QUERY) {
	pqtimer->retry_left--;
	if (pqtimer->retry_left <= 0)
		untimeout(&pqtimer->ch);
	else
			timeout(igmp_timer_expired, pqtimer, 3, &pqtimer->ch);
	}

   if(IGMPCurrentVersion==3) {
       igmp_queryV3(ALL_SYSTEMS, 0, query_response_interval);
   } else {
       igmp_query(ALL_SYSTEMS, 0, query_response_interval);
   }
}

void igmp_timer_unsolicit_report(void *arg)
{
	struct igmp_timer *ptimer;
	struct mcft_entry *mcp = mcpq;

	ptimer = (struct igmp_timer *)arg;

	ptimer->unsol_report_count++;

	timeout(igmp_timer_unsolicit_report, ptimer, UNSOLT_REPORT_INTERVAL, &ptimer->ch);

	while(mcp)
	{
		//printf("the mcp->grp_addr is 0x%x\n", mcp->grp_addr);
		igmp_report(mcp->grp_addr, 0);
		mcp = mcp->next;
	}
}

int add_group_and_src( __u32 group, __u32 src )
{
	struct mcft_entry *mymcp;

	if(!IN_MULTICAST( ntohl(group) ))
		return 0;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return 0;
	/* TBD */
	/* should check if it's from downtream interface */

	if(!chk_mcft(group)) {
		// Group does not exist on router, add multicast address into if_table
	//	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);

		mymcp = add_mcft(group, src);
		if(!mymcp) {
			//printf("igmp_accept> add group to list fail!\n");
			return 0;
		}
		// Kaohj
		//add_mr(group);
		add_mr(mymcp, src);
#ifdef PERIODICAL_SPECIFIC_QUERY
		mymcp->timer.retry_left =last_member_query_count;
		timeout(igmp_specific_timer_expired , mymcp, MEMBER_QUERY_INTERVAL, &mymcp->timer.ch);
#endif
	}
	else {
		mymcp = get_mcft(group);
		if (mymcp)
		{
			untimeout(&mymcp->timer.ch);
#ifdef KEEP_GROUP_MEMBER
			add_user(mymcp, src);
#endif
#ifdef PERIODICAL_SPECIFIC_QUERY
			mymcp->timer.retry_left = last_member_query_count;
			timeout(igmp_specific_timer_expired , mymcp, MEMBER_QUERY_INTERVAL, &mymcp->timer.ch);
#endif
		}
	}


	return 0;
}

int del_group_and_src( __u32 group, __u32 src )
{
	struct mcft_entry *mymcp;
	int query_count;
#ifdef LEAVE_SPECIFIC_QUERY
	int query_interval;
#endif
#ifdef KEEP_GROUP_MEMBER
	int count;
#endif

	if(!IN_MULTICAST( ntohl(group) )) {
		//printf("igmp_accept> invalid multicast address or IGMP leave\n");
		return 0;
	}
	//printf("igmp_accept> receive IGMP Leave from %s,", inet_ntoa(ip->saddr));
	//printf("group = %s\n", inet_ntoa(igmp->group));

	/* TBD */
	/* should check if it's from downtream interface */
	if(chk_mcft(group)) {
		mymcp = get_mcft(group);
		// Group does exist on router
		if(mymcp) {
			query_count = last_member_query_count;
#ifdef LEAVE_SPECIFIC_QUERY
			query_interval = last_member_query_interval;
#endif
#ifdef KEEP_GROUP_MEMBER
			count = del_user(mymcp, src);
			if (count == 0) {// no member, drop it!
				del_mr(mymcp);
				del_mcft(mymcp);
				query_count = last_member_query_count;
#ifdef LEAVE_SPECIFIC_QUERY
				query_interval = last_member_query_interval;
#endif
			}
			else {
#ifdef PERIODICAL_SPECIFIC_QUERY
				query_count = MEMBER_QUERY_COUNT;
#ifdef LEAVE_SPECIFIC_QUERY
				query_interval = last_member_query_interval*3;
#endif
#else
				query_count = last_member_query_count;
#ifdef LEAVE_SPECIFIC_QUERY
				query_interval = last_member_query_interval*3;
#endif
#endif
			}
#endif
			mymcp->timer.retry_left = query_count;
#ifdef LEAVE_SPECIFIC_QUERY
			timeout(igmp_specific_timer_expired, mymcp, query_interval/IGMP_TIMER_SCALE, &mymcp->timer.ch);
            if(IGMPCurrentVersion==3)
			    igmp_queryV3(ALL_SYSTEMS, mymcp->grp_addr, query_interval);
            else
			    igmp_query(ALL_SYSTEMS, mymcp->grp_addr, query_interval);
#endif
		}
	}

	return 0;
}

/*
 * igmp_accept - handles the incoming IGMP packets
 *
 */


int igmp_accept(int recvlen, struct IfDesc *dp, int flag)
{
	register __u32 src, dst, group;
	struct iphdr *ip;
	struct igmphdr *igmp;
	int iphdrlen;
	struct igmpmsg *msg;

	if (recvlen < sizeof(struct iphdr)) {
		igmpproxy_log(LOG_WARNING, 0,
		    "received packet too short (%u bytes) for IP header", recvlen);
		return 0;
	}

	ip  = (struct iphdr *)recv_buf;
	src = ip->saddr;
	dst = ip->daddr;

	if(!IN_MULTICAST( ntohl(dst) ))	/* It isn't a multicast */
		return -1;
	if(chk_local(src)) 		/* It's our report looped back */
		return -1;
	if(dst == ALL_PRINTER)	/* It's MS-Windows UPNP all printers notify */
		return -1;

	pkt_debug(recv_buf);

	iphdrlen  = ip->ihl << 2;
	//ipdatalen = ip->tot_len;

	igmp        = (struct igmphdr *)(recv_buf + iphdrlen);
	group   = igmp->group;

	/* determine message type */
	//printf("igmp_accept> receive IGMP type [%x] from %s:", igmp->type, inet_ntoa(ip->saddr));
	//printf("%s\n", inet_ntoa(ip->daddr));
	switch (igmp->type) {
		case IGMP_HOST_MEMBERSHIP_QUERY:
			/* Linux Kernel will process local member query, it wont reach here */
			break;

		case IGMP_HOST_MEMBERSHIP_REPORT:
		case IGMPV2_HOST_MEMBERSHIP_REPORT:
			if(flag == FROM_WAN)
				break;

			if(group == ALL_PRINTER){	/* ignore join SSDP group for Dial-On Demand */
				break;
			}

			add_group_and_src( group, src );
			break;

 		case IGMP_HOST_V3_MEMBERSHIP_REPORT:
		
			if(flag == FROM_WAN)
				break;
			/* TBD */
			/* should check if it's from downtream interface */

			struct igmpv3_report *igmpv3;
			struct igmpv3_grec *igmpv3grec;
			__u16 rec_id;

			igmpv3 = (struct igmpv3_report *)igmp;
			//printf( "recv IGMP_HOST_V3_MEMBERSHIP_REPORT\n" );
			//printf( "igmpv3->type:0x%x\n", igmpv3->type );
			//printf( "igmpv3->ngrec:0x%x\n", ntohs(igmpv3->ngrec) );

			rec_id=0;
			igmpv3grec =  &igmpv3->grec[0];
			while( rec_id < ntohs(igmpv3->ngrec) )
			{

				//printf( "igmpv3grec[%d]->grec_type:0x%x\n", rec_id, igmpv3grec->grec_type );
				//printf( "igmpv3grec[%d]->grec_auxwords:0x%x\n", rec_id, igmpv3grec->grec_auxwords );
				//printf( "igmpv3grec[%d]->grec_nsrcs:0x%x\n", rec_id, ntohs(igmpv3grec->grec_nsrcs) );
				//printf( "igmpv3grec[%d]->grec_mca:%s\n", rec_id, inet_ntoa(igmpv3grec->grec_mca) );

				group = igmpv3grec->grec_mca;

				if(group == ALL_PRINTER){	/* ignore join SSDP group for Dial-On Demand */
					break;
				}

				switch( igmpv3grec->grec_type )
				{
				case IGMPV3_MODE_IS_INCLUDE:
				case IGMPV3_MODE_IS_EXCLUDE:
					if(chk_mcft(group))
					{
						//printf( "IS_IN or IN_EX\n" );
						add_group_and_src( group, src );
					}
					break;
				case IGMPV3_CHANGE_TO_INCLUDE:
					//printf( "TO_IN\n" );
					if( igmpv3grec->grec_nsrcs )
						add_group_and_src( group, src );
					else //empty
						del_group_and_src( group, src );
					break;
				case IGMPV3_CHANGE_TO_EXCLUDE:
					//printf( "TO_EX\n" );
					add_group_and_src( group, src );
					break;
				case IGMPV3_ALLOW_NEW_SOURCES:
					//printf( "ALLOW\n" );
					break;
				case IGMPV3_BLOCK_OLD_SOURCES:
					//printf( "BLOCK\n" );
					break;
				default:
					//printf( "!!! can't handle the group record types: %d\n", igmpv3grec->grec_type );
					break;
				}

				rec_id++;
				//printf( "count next: 0x%x %d %d %d %d\n", igmpv3grec, sizeof( struct igmpv3_grec ), igmpv3grec->grec_auxwords, ntohs(igmpv3grec->grec_nsrcs), sizeof( __u32 ) );
				igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof( __u32 ) );
				//printf( "count result: 0x%x\n", igmpv3grec );
			}
			break;

		case IGMP_HOST_LEAVE_MESSAGE :
			del_group_and_src( group, src );
			break;
		// Kaohj
		case IGMPMSG_NOCACHE: // ipmr_cache_report (no route for incomming group)
			//do nothing here.
			break;

		default:
			//printf("igmp_accept> receive IGMP Unknown type [%x] from %s:", igmp->type, inet_ntoa(ip->saddr));
			//printf("%s\n", inet_ntoa(ip->daddr));
			break;
    }
    return 0;
}


/*
 * igmp_report - send an IGMP Report packet, directly to linkp->send(), not via ip
 *
 * int igmp_report( longword ina, int ifno )
 * Where:
 *	ina	the group address to report.
 *      ifno	interface number
 *
 * Returns:
 *	0	if unable to send report
 *	1	report was sent successfully
 */

int igmp_report(__u32 dst, int if_idx)
{
    struct iphdr *ip;
    struct igmphdr *igmp;
    struct sockaddr_in sdst;
    struct IfDesc *dp;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
    int index;
#endif
    struct in_addr tmp_addr;

    ip = (struct iphdr *)send_buf;
    ip->saddr       = getAddrByVifIx(if_idx);
    ip->daddr       = dst;
    ip->tot_len     = MIN_IP_HEADER_LEN + IGMP_MINLEN;
    if (IN_MULTICAST(ntohl(dst))) {
		ip->ttl = 1;
	}
    else
		ip->ttl = MAXTTL;

    igmp                    = (struct igmphdr *)(send_buf + MIN_IP_HEADER_LEN);
    igmp->type         = IGMPV2_HOST_MEMBERSHIP_REPORT;
    igmp->code         = 0;
    igmp->group		   = dst;
    igmp->csum        = 0;
    igmp->csum        = in_cksum((u_short *)igmp, IGMP_MINLEN);

    bzero(&sdst, sizeof(sdst));
    sdst.sin_family = AF_INET;
    sdst.sin_addr.s_addr = dst;
    //fprintf(stderr, "igmpproxy: send report to %x\n", dst);
    //if (sendto(igmp_socket, send_buf,
    // Kaohj
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		for(index=0;index<igmp_up_if_num;index++)
		{
			dp= getIfByName(igmp_up_if_name[index]);

			if(dp==NULL)
				continue;

			ip->saddr = dp->InAdr.s_addr;

			k_set_if(dp->InAdr.s_addr);
			if (sendto(MRouterFD, igmp, IGMP_MINLEN, 0,
				(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
				tmp_addr.s_addr = ip->saddr;
				printf("igmp_report> sendto error, from %s ", inet_ntoa(tmp_addr));
				tmp_addr.s_addr = ip->daddr;
				printf("to %s\n", inet_ntoa(tmp_addr));
			}
		}
#else
		// Kaohj
		dp = getIfByName(igmp_up_if_name);
		if (!dp)
			return 0;
		k_set_if(dp->InAdr.s_addr);
		if (sendto(MRouterFD, igmp,
			IGMP_MINLEN, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
			tmp_addr.s_addr = ip->saddr;
			printf("igmp_report> sendto error, from %s ", inet_ntoa(tmp_addr));
			tmp_addr.s_addr = ip->daddr;
			printf("to %s\n", inet_ntoa(tmp_addr));
		}
#endif

    return 1;
}




/*
 * igmp_query - send an IGMP Query packet to downstream interface
 *
 * int igmp_query(__u32 dst, __u32 grp,__u8 mrt)
 * Where:
 *  dst		destination address
 *  grp		query group address
 *  MRT		Max Response Time in IGMP header (in 1/10 second unit)
 *
 * Returns:
 *	0	if unable to send
 *	1	packet was sent successfully
 */

int igmp_query(__u32 dst, __u32 grp,__u8 mrt)
{
    struct iphdr *ip;
    struct igmphdr *igmp;
    struct sockaddr_in sdst;
    struct in_addr tmp_addr;
	struct IfDesc *dp = getIfByName(igmp_down_if_name);

    k_set_if(dp->InAdr.s_addr);
    ip              = (struct iphdr *)send_buf;
    ip->saddr       = dp->InAdr.s_addr;
    ip->daddr       = dst;
    ip->tot_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN;
	ip->ttl = 1;

    igmp               = (struct igmphdr *)(send_buf + MIN_IP_HEADER_LEN);
   	igmp->type         = 0x11;
    igmp->code         = mrt;
    igmp->group 	   = grp;
    igmp->csum        = 0;
    igmp->csum        = in_cksum((u_short *)igmp, IGMP_MINLEN);

    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    sdst.sin_addr.s_addr = dst;

    if (sendto(MRouterFD, igmp, IGMP_MINLEN, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
		tmp_addr.s_addr = ip->saddr;
		printf("igmp_query> sendto error, from %s ", inet_ntoa(tmp_addr));
		tmp_addr.s_addr = ip->daddr;
		printf("to %s\n", inet_ntoa(tmp_addr));
    }

    return 0;
}

int igmp_queryV3(__u32 dst, __u32 grp,__u8 mrt)
{
    struct igmpv3_query	*igmpv3;
    struct sockaddr_in	sdst;
    struct IfDesc 	*dp = getIfByName(igmp_down_if_name);
    int totalsize=0;
    struct in_addr tmp_addr;

    k_set_if(dp->InAdr.s_addr);
    igmpv3            = (struct igmpv3_query *)send_buf;
    igmpv3->type      = 0x11;
    igmpv3->code      = mrt;
    igmpv3->csum      = 0;
    igmpv3->group     = grp;
    igmpv3->resv      = 0;
    igmpv3->suppress  = 1;
    igmpv3->qrv       = robust_count;
    igmpv3->qqic      = MEMBER_QUERY_INTERVAL;
    igmpv3->nsrcs     = 0;

    totalsize	      = sizeof(struct igmpv3_query);
    igmpv3->csum      = in_cksum((u_short *)igmpv3, totalsize );

    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    if(grp)
    	sdst.sin_addr.s_addr = grp;
    else
    	sdst.sin_addr.s_addr = ALL_SYSTEMS;

    tmp_addr.s_addr = grp;
    igmpproxy_log(LOG_INFO, 0, "%s> send to group %s\n", __FUNCTION__, inet_ntoa(tmp_addr));
    if (sendto(MRouterFD, igmpv3, totalsize, 0, (struct sockaddr *)&sdst, sizeof(sdst)) < 0)
    {
	printf("igmpv3_query> sendto error, from %s ", inet_ntoa(dp->InAdr));
	printf("to %s\n", inet_ntoa(sdst.sin_addr));
    }

    return 0;
}

/*
 * igmp_leave - send an IGMP LEAVE packet, directly to linkp->send(), not via ip
 *
 * int igmp_leave( longword ina, int ifno )
 * Where:
 *  	ina	the IP address to leave
 *  	ifno	interface number
 *
 * Returns:
 *	0	if unable to send leave
 *	1	report was sent successfully
 */

int igmp_leave(__u32 grp, int if_idx)
{
    struct iphdr *ip;
    struct igmphdr *igmp;
    struct sockaddr_in sdst;
    struct IfDesc *dp;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
    int index;
#endif
    struct in_addr tmp_addr;

    ip              = (struct iphdr *)send_buf;
    ip->daddr       = ALL_ROUTERS;
    ip->tot_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN;
	ip->ttl = 1;

    igmp               = (struct igmphdr *)(send_buf + MIN_IP_HEADER_LEN);
   	igmp->type         = 0x17;
    igmp->code         = 0;
    igmp->group 	   = grp;
    igmp->csum        = 0;
    igmp->csum        = in_cksum((u_short *)igmp, IGMP_MINLEN);

    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    sdst.sin_addr.s_addr = ALL_ROUTERS;
    //printf("send igmp leave\n");
    //syslog(LOG_INFO, "igmpproxy: send leave to %s: %s\n", igmp_up_if_name, inet_ntoa(grp));

    // Kaohj
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		for(index=0;index<igmp_up_if_num;index++)
		{
			dp= getIfByName(igmp_up_if_name[index]);

			if(dp==NULL)
				continue;
			ip->saddr       = dp->InAdr.s_addr;
			k_set_if(dp->InAdr.s_addr);
			if (sendto(MRouterFD, igmp, IGMP_MINLEN, 0,
					(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
				tmp_addr.s_addr = ip->saddr;
				printf("igmp_leave> sendto error, from %s ", inet_ntoa(tmp_addr));
				tmp_addr.s_addr = ip->daddr;
				printf("to %s\n", inet_ntoa(tmp_addr));
			}
    }
#else
		// Kaohj
		dp = getIfByName(igmp_up_if_name);
		ip->saddr       = dp->InAdr.s_addr;
		k_set_if(dp->InAdr.s_addr);
		if (sendto(MRouterFD, igmp, IGMP_MINLEN, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
			tmp_addr.s_addr = ip->saddr;
			printf("igmp_leave> sendto error, from %s ", inet_ntoa(tmp_addr));
			tmp_addr.s_addr = ip->daddr;
			printf("to %s\n", inet_ntoa(tmp_addr));
		}
#endif

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
#ifdef MULTICAST_LANWAN_BIND
static int isValidMedia(unsigned int ifIndex)
{
	MEDIA_TYPE_T mType;

	mType = MEDIA_INDEX(ifIndex);
	if (1
	#ifdef CONFIG_RTL8672_SAR
		&& mType!=MEDIA_ATM
	#endif
	#ifdef CONFIG_PTMWAN
		&& mType!=MEDIA_PTM
	#endif /*CONFIG_PTMWAN*/
	#ifdef CONFIG_ETHWAN
		&& mType!=MEDIA_ETH
	#endif
	#ifdef WLAN_WISP
		&& mType!=MEDIA_WLAN
	#endif
	)
		return 0;
	return 1;
}

char *ifGetName(int ifindex, char *buffer, unsigned int len)
{
	MEDIA_TYPE_T mType;

	if ( ifindex == DUMMY_IFINDEX )
		return 0;
	if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
	{
		mType = MEDIA_INDEX(ifindex);
		if (mType == MEDIA_ATM) {
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			if (VC_MINOR_INDEX(ifindex) == DUMMY_VC_MINOR_INDEX) // major vc devname
				snprintf( buffer, len,  "%s%u", ALIASNAME_VC, VC_MAJOR_INDEX(ifindex));
			else
				snprintf( buffer, len,  "%s%u_%u", ALIASNAME_VC, VC_MAJOR_INDEX(ifindex),  VC_MINOR_INDEX(ifindex));
#else
			snprintf( buffer, len,  "%s%u", ALIASNAME_VC, VC_INDEX(ifindex) );
#endif
		}
		else if (mType == MEDIA_ETH)
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			snprintf( buffer, len, "%s%d", ALIASNAME_MWNAS, ETH_INDEX(ifindex));
#else
			snprintf( buffer, len,  "%s%u", ALIASNAME_NAS, ETH_INDEX(ifindex) );
#endif
#ifdef CONFIG_PTMWAN
		else if (mType == MEDIA_PTM)
			snprintf( buffer, len, "%s%d", ALIASNAME_MWPTM, PTM_INDEX(ifindex));
#endif /*CONFIG_PTMWAN*/
#ifdef WLAN_WISP
		else if (mType == MEDIA_WLAN)
			snprintf( buffer, len, "wlan%d-vxd", ETH_INDEX(ifindex));
#endif
		else if (mType == MEDIA_IPIP)    // Mason Yu. Add VPN ifIndex
			snprintf( buffer, len, "ipip%u", IPIP_INDEX(ifindex));
		else
			return 0;

	}else{
		snprintf( buffer, len,  "ppp%u", PPP_INDEX(ifindex) );

	}

	return buffer;
}

int getIfIndexVidByName(char *pIfname,unsigned int *ifIndex,unsigned short *vid)
{
#ifdef CONFIG_RTK_LIB_MIB
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (!Entry.enable || !isValidMedia(Entry.ifIndex))
			continue;

		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));

		if(!strcmp(ifname,pIfname)){
			*ifIndex = Entry.ifIndex;
			*vid = Entry.vid;
			igmpproxy_log(LOG_INFO,0,"default binding to %s,ifIndex=%d,vid=%d\n",ifname,Entry.ifIndex,Entry.vid);
			return 0;
		}
	}
#endif
	return -1;
}



int rtk_is_bind_interface(struct IfDesc *up,struct mcft_entry *mymcp)
{
	int i;

	if(mymcp->bind_up_num == 0)
		return 1;
	
	for(i = 0;i < mymcp->bind_up_num;i++){
		if(up == (mymcp->bind_up[i]))
			return 1;
	}

	return 0;
}

int rtk_is_mcft_equal(struct mcft_entry *mymcp0,struct IfDesc **bind_up,int bind_up_num)
{
	int i = 0;
	
	if(mymcp0->bind_up_num == bind_up_num){
		for(i = 0;i < bind_up_num;i++){
			if((mymcp0->bind_up[i]) != bind_up[i]){
				return 0;
			}
		}

		return 1;
	}
	return 0;
}

int rtk_binding_all_untag_wan(void)
{
#ifdef CONFIG_RTK_LIB_MIB
	int total,i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	struct IfDesc *tmp_bind_up;
	
	total = mib_chain_total(MIB_ATM_VC_TBL);
	if (total > MAX_VC_NUM){
		igmpproxy_log(LOG_WARNING, 0, "MIB_ATM_VC_TBL num=%d exceed %d",total,MAX_VC_NUM);
		return -1;
	}

	for(i = 0; i < total; i++) {
		mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&Entry);
		if (!Entry.enable || !isValidMedia(Entry.ifIndex))
			continue;

		if(Entry.vid == 0){
			ifGetName(Entry.ifIndex,ifname,sizeof(ifname));

			tmp_bind_up = getIfByName((const char*)ifname);
			bind_up[bind_up_num++] = tmp_bind_up;
			igmpproxy_log(LOG_INFO,0,"binding to untag wan %s",ifname);
		}
	}
	igmpproxy_log(LOG_INFO,0,"bind untag wan num=%d",bind_up_num);
#endif
	return 0;
}
#if defined(CONFIG_USER_VLAN_MAPPING) && defined(CONFIG_RTK_DEV_AP)
int rtk_get_vlan_mapping_info(char *name,unsigned int *ifindex,unsigned short *vid)
{
	int i = 0,j = 0,ifidx = 0, wan_vid = 0, wan_vlan_index = 0, wanEntryVid = 0,totalwanNum = 0;
	MIB_CE_PORT_BINDING_T pEntry;
	MIB_CE_ATM_VC_T wanEntry;
	struct vlan_pair *vid_pair;
	char lan_ifname_temp[IFNAMSIZ] = {0};
	char lan_temp[IFNAMSIZ] = {0};

	memset(lan_temp, 0, sizeof(lan_temp));
	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3; j++)
	{
		if(strstr(name, (char *)ELANRIF[j]))
		{		
			ifidx = j;
			snprintf(lan_temp, sizeof(lan_temp), "%s", ELANRIF[j]);
			break;
		}
	}
	
#ifdef WLAN_SUPPORT
	for (j=PMAP_WLAN0; j<=PMAP_WLAN1_VAP_END; j++) 
	{
		if(strstr(name, (char *)wlan[j-PMAP_WLAN0]))
		{
			ifidx = j;
			snprintf(lan_temp, sizeof(lan_temp), "%s", wlan[j-PMAP_WLAN0]);
			break;
		}
	}
#endif // of WLAN_SUPPORT	

	memset(&pEntry, 0, sizeof(MIB_CE_PORT_BINDING_T));
	mib_chain_get(MIB_PORT_BINDING_TBL, ifidx, (void*)&pEntry);

	if((unsigned char)VLAN_BASED_MODE == pEntry.pb_mode)
	{
		vid_pair = (struct vlan_pair *)&(pEntry.pb_vlan0_a);
		for (i = 0; i < 4; i++)
		{
			if (vid_pair[i].vid_a <= 0) 
				continue;

			memset(lan_ifname_temp, 0, sizeof(lan_ifname_temp));
			snprintf(lan_ifname_temp, sizeof(lan_ifname_temp), "%s.%d", lan_temp, vid_pair[i].vid_a);
			if(strcmp(name, lan_ifname_temp) == 0)
			{
				wan_vid = vid_pair[i].vid_b;
				break;
			}
		}
	}
	else{ 
		//printf("[%s %d] this lan didn't use vlan binding name:%s\n", __FUNCTION__, __LINE__, name);
		*vid = 0;
		*ifindex = 0;
		return 0;
	}

	totalwanNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(j = 0; j < totalwanNum; j++){
		
		mib_chain_get(MIB_ATM_VC_TBL, j, (void*)&wanEntry);
		wanEntryVid = wanEntry.vid;
	
		if (!wanEntry.enable || (wanEntryVid != wan_vid))
			continue;

		//printf("%s %d wanvid:%d wanifIndex:%d +++++++++++++++++++\n", __FUNCTION__, __LINE__, wanEntry.vid, wanEntry.ifIndex);
		*vid = wanEntry.vid;
		*ifindex = wanEntry.ifIndex;
		return 1;
	}

	*vid = 0;
	*ifindex = 0;
	return 0;
}
#endif
int rtk_get_binding_wan_info(char *name,unsigned int *ifindex,unsigned short *vid)
{
#ifdef CONFIG_RTK_LIB_MIB
	int total,i,j,ret = 0;
	unsigned short itfGroup;
	MIB_CE_ATM_VC_T Entry;	
#if defined(CONFIG_USER_VLAN_MAPPING) && defined(CONFIG_RTK_DEV_AP)
	//add vlan-binding check
	unsigned int wanifindex = 0;
	unsigned short wanvid = 0;
	ret = rtk_get_vlan_mapping_info(name, &wanifindex, &wanvid);
	if(ret){
		*vid = wanvid;
		*ifindex = wanifindex;
		//printf("[%s %d]this lan is vlan binding ! vid:%d ifIndex:%d name:%s   ++++++++++++++++\n", __FUNCTION__, __LINE__, *vid, *ifindex, name);
		return 0;
	}
#endif

	total = mib_chain_total(MIB_ATM_VC_TBL);
	if (total > MAX_VC_NUM){
		igmpproxy_log(LOG_WARNING, 0, "MIB_ATM_VC_TBL num=%d exceed %d",total,MAX_VC_NUM);
		return -1;
	}

	for(i = 0; i < total; i++) {
		mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&Entry);
		if (!Entry.enable || !isValidMedia(Entry.ifIndex))
			continue;
		itfGroup = Entry.itfGroup;
		for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < ELANVIF_NUM; ++j)
		{
			if((itfGroup & (1<<j)) != 0)
			{
				if(!strcmp((char *)ELANVIF[j],name)){
					igmpproxy_log(LOG_INFO, 0, "Get LAN-WAN binding, lan interface=%s, wan ifIndex=%d,vid=%d\n",name,Entry.ifIndex,Entry.vid);
					*vid = Entry.vid;
					*ifindex = Entry.ifIndex;
					return 0;
				}
			}
		}
		
#ifdef WLAN_SUPPORT
		{
			for (j=PMAP_WLAN0; j<=PMAP_WLAN0_VAP3; j++) {
				if(wlan_en[j-PMAP_WLAN0])
					if((itfGroup & (1<<j)) != 0)
						if(!strcmp((char *)wlan[j-PMAP_WLAN0],name)){
							igmpproxy_log(LOG_INFO, 0, "Get LAN-WAN binding, lan interface=%s, wan ifIndex=%d,vid=%d\n",name,Entry.ifIndex,Entry.vid);
							*vid = Entry.vid;
							*ifindex = Entry.ifIndex;
							return 0;
						}
			}
		}

		{
			for (j=PMAP_WLAN1; j<=PMAP_WLAN1_VAP3; j++) {
				if(wlan_en[j-PMAP_WLAN0])
					if((itfGroup & (1<<j)) != 0)
						if(!strcmp((char *)wlan[j-PMAP_WLAN0],name)){
							igmpproxy_log(LOG_INFO, 0, "Get LAN-WAN binding, lan interface=%s, wan ifIndex=%d,vid=%d\n",name,Entry.ifIndex,Entry.vid);
							*vid = Entry.vid;
							*ifindex = Entry.ifIndex;
							return 0;
						}	
			}
		}

#endif // of WLAN_SUPPORT	
	}
	
	/* not binding,binding to the first wan */
	if(getIfIndexVidByName(igmp_up_if_name[0],ifindex,vid) < 0){
		igmpproxy_log(LOG_WARNING, 0,"get %s IfIndex/vid fail",igmp_up_if_name[0]);
		return -1;
	}
#endif
	return 0;
}

int rtk_recv_message(void)
{
	int recvlen;
	char adata[100];
	unsigned int ifindex;
	unsigned short vid;
	struct IfDesc *tmp_bind_up;
	char ifname[IFNAMSIZ];
	struct iovec iov;
	struct cmsghdr *cmsg;
	struct in_pktinfo_rtk *info;
	struct sockaddr_in from2;
	struct msghdr msg;
	struct ifreq ifr;
	

	msg.msg_name = (void *) &from2;                    // The from2 is src_IP of packet
	msg.msg_namelen = sizeof (struct sockaddr_in);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = (void *) adata;                  // This is a buf for ancillary data. Such as ifindex.
	msg.msg_controllen = sizeof adata;

	iov.iov_base = recv_buf;                          // The is a buf for the desired data. Such as RIP Packet(RIP header + RIP payload).
	iov.iov_len = RECV_BUF_SIZE;

	recvlen = recvmsg(MRouterFD, &msg, 0);
	if(recvlen < 0)
	{
		fprintf(stderr,"\nrecvmsg,Errno(%d): %s", errno, (errno <= 0) ? NULL : (const char *)strerror( errno ));
		return -1;
	}

	for (cmsg = (struct cmsghdr *) CMSG_FIRSTHDR(&msg);cmsg;
			 cmsg = (struct cmsghdr *) CMSG_NXTHDR(&msg, cmsg))
	{
		if (cmsg->cmsg_level ==IPPROTO_IP&&cmsg->cmsg_type == IP_PKTINFO_RTK &&
				cmsg->cmsg_len == CMSG_LEN(sizeof(struct in_pktinfo_rtk)))
		{
			bind_up_num = 0;
		
			info = (struct in_pktinfo_rtk *) (CMSG_DATA(cmsg));

			ifindex = info->ipi_ifindex;
			memset(&ifr, 0, sizeof(ifr));
			ifr.ifr_ifindex = ifindex;

			igmpproxy_log(LOG_WARNING,0, "ipi_ifindex=%d,ipi_spec_dst=0x%x,ipi_addr=0x%x,from_dev_ifindex_valid=%d,from_dev_ifindex=%d",
					info->ipi_ifindex,info->ipi_spec_dst.s_addr,info->ipi_addr.s_addr,info->from_dev_ifindex_valid,info->from_dev_ifindex);
			if(ifr.ifr_ifindex == 0)
			{
				return recvlen;
			}

			if (ioctl(MRouterFD, SIOCGIFNAME, &ifr))
			{
				igmpproxy_log(LOG_WARNING,0, "\nGet root iface name error,Errno(%d): %s", errno, (errno <= 0) ? NULL : (const char *)strerror( errno ));
				igmpproxy_log(LOG_WARNING,0, "ipi_ifindex=%d,ipi_spec_dst=0x%x,ipi_addr=0x%x,from_dev_ifindex_valid=%d,from_dev_ifindex=%d",
					info->ipi_ifindex,info->ipi_spec_dst.s_addr,info->ipi_addr.s_addr,info->from_dev_ifindex_valid,info->from_dev_ifindex);
				return -1;
			}

			if(strncmp(ifr.ifr_name, igmp_down_if_name, sizeof(igmp_down_if_name))){
				igmpproxy_log(LOG_WARNING,0,"receive igmp packet from up interface, if lan-wan binding enabled, not support");
				return -1;
			}	

			if(info->from_dev_ifindex_valid == 0){
				igmpproxy_log(LOG_WARNING, 0, "\nnot get bindind info");
				/* 224.0.0.2/224.0.0.22 .etc from br0,only binding to first wan */
				tmp_bind_up = getIfByName(igmp_up_if_name[0]);
				bind_up_num = 1;
				bind_up[0] = tmp_bind_up;
				
				return recvlen;
			}

			memset(&ifr, 0, sizeof(ifr));
			ifr.ifr_ifindex=info->from_dev_ifindex;  
			if (ioctl(MRouterFD, SIOCGIFNAME, &ifr) < 0)
			{
				igmpproxy_log(LOG_WARNING,0,"\nGet iface name error,Errno(%d): %s", errno, (errno <= 0) ? NULL : (const char *)strerror( errno ));
				igmpproxy_log(LOG_WARNING,0, "ipi_ifindex=%d,ipi_spec_dst=0x%x,ipi_addr=0x%x,from_dev_ifindex_valid=%d,from_dev_ifindex=%d",
					info->ipi_ifindex,info->ipi_spec_dst.s_addr,info->ipi_addr.s_addr,info->from_dev_ifindex_valid,info->from_dev_ifindex);
				return -1;
			}
			
			if(rtk_get_binding_wan_info(ifr.ifr_name,&ifindex,&vid) < 0)
				return -1;

			if(!vid){
				if(rtk_binding_all_untag_wan() < 0)
					return -1;
			}else{
				ifGetName(ifindex,ifname,sizeof(ifname));

				tmp_bind_up = getIfByName((const char*)ifname);
				bind_up_num = 1;
				bind_up[0] = tmp_bind_up;
				igmpproxy_log(LOG_INFO,0,"binding to tag wan %s",ifname);
			}

			return recvlen;
		}
	}

	/* not get IP_PKTINFO_RTK info */
	igmpproxy_log(LOG_WARNING, 0,"not get IP_PKTINFO_RTK info");
	return -1;
}
#endif


////////////////////////////////////////////////////////////////////////////////////


char* runPath = "/bin/igmpproxy";
char* pidfile = "/var/run/igmp_pid";

/*
 * On hangup, let everyone know we're going away.
 */
void clearMcFlow(void);
void hup(int signum)
{
	(void)signum;
  //It appears syslog can go into deadlock when it receives a signal where the signal handler also syslogs
  //igmpproxy_log( LOG_DEBUG, 0, "clean handler called" );
  if(MRouterFD != 0)
  	disableMRouter();
  if(fifofd >= 0)
  	remove_fifo();
  clearMcFlow();
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
  if(igmpcfg_setting.cfg_path){
	free(igmpcfg_setting.cfg_path);
  }
#endif
  unlink(pidfile);
  exit(EXIT_SUCCESS);

}

#ifdef CONFIG_DEV_xDSL
#define MCAST_HNAT_PROC	"/proc/rtl865x/hwMCast"

void RTK_Delete_HW_McastFlow(struct mcft_entry *mcp){
	unsigned char *bytes=NULL;
	char cmd[128]={0}, grp_addr[16]={0};
	struct in_addr tmp_addr;
#if defined(_SUPPORT_IGMPV3_)
	struct src_entry *s=NULL;
	char src_addr[16]={0};
#endif

	bytes=(unsigned char *)&(mcp->grp_addr);
	snprintf(grp_addr, sizeof(grp_addr), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

//	printf("%s> grp_addr=%s\n", __FUNCTION__, grp_addr);
#ifdef _SUPPORT_IGMPV3_
	s=mcp->srclist;
	if(s){
		while(s)
		{
			bytes=(unsigned char *)&(s->srcaddr);
			snprintf(src_addr, sizeof(src_addr), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

//			printf("%s> try to match=> fmode:%d, timer:%d, slist:%s\n", __FUNCTION__, mcp->filter_mode, s->timer.lefttime, src_addr );
			snprintf(cmd, sizeof(cmd), "echo del %s %s > %s", grp_addr, src_addr, MCAST_HNAT_PROC);
			printf("%s> cmd=%s\n", __FUNCTION__, cmd);
			system(cmd);
			s=s->next;
		}
	}else{
		snprintf(cmd, sizeof(cmd), "echo clear %s > %s", grp_addr, MCAST_HNAT_PROC);
		printf("%s> cmd=%s\n", __FUNCTION__, cmd);
		system(cmd);
	}
#else
	snprintf(cmd, sizeof(cmd), "echo clear %s > %s", grp_addr, MCAST_HNAT_PROC);
	printf("%s> cmd=%s\n", __FUNCTION__, cmd);
	system(cmd);
#endif

	return;
}
#endif


void clearMcFlow(void)
{
	printf("=====> %s\n", __FUNCTION__);
	struct mcft_entry *mcp = mcpq;
	while(mcp) {

#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
		rtk_fc_multicast_flow_delete(mcp->grp_addr);
#endif
#ifdef CONFIG_DEV_xDSL
		RTK_Delete_HW_McastFlow(mcp);
#endif
		mcp = mcp->next;
	}
}

void updateMcFlow()
{
	if(isIGMPSnoopingEnabled())
	{
		clearMcFlow();
		if(IGMPCurrentVersion==3)
			igmp_queryV3(ALL_SYSTEMS, 0, 1);
		else
			igmp_query(ALL_SYSTEMS, 0, 1);
	}
	else{
		printf("=====> %s\n", __FUNCTION__);
		struct mcft_entry *mcp = mcpq;
		while(mcp) {
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
			rtk_fc_multicast_flow_add(mcp->grp_addr);
#endif

			mcp = mcp->next;
		}
	}
}

void sig_doUpdate(int signum)
{
	(void)signum;
	update_flag = 1;
}

void doUpdate()
{
	reload_igmp_cfg_file(0);
	updateMcFlow();
}

// Kaohj added
// Comes here because upstream or downstream interface ip changed
// Usually, it is used by dynamic interface to sync its interface with
// the igmpproxy local database.
void sigifup(int signum)
{
	(void)signum;
	ifup_flag = 1;
}

void doifup()
{
	struct IfDesc *Dup, *Ddp;

	if (!MRouter_created)
		return;
	igmpproxy_log(LOG_INFO,0, "igmpproxy: SIGUSR1 caught\n");
	updateIfVc();
#ifdef CONFIG_IGMPPROXY_MULTIWAN
        int index;

	Ddp = getIfByName(igmp_down_if_name);
	if(Ddp) addVIF(Ddp);

	// update upstream/downstream interface ip into local database
	for(index = 0;index<igmp_up_if_num;index++)
	{
		Dup = getIfByName(igmp_up_if_name[index]);
		if ( Dup == NULL)
			continue;
		if (Dup->InAdr.s_addr != 0)
			igmp_up_if_idx[index] = addVIF(Dup);
	}
#else

	// get descriptors of upstream and downstream interfaces
	Dup = getIfByName(igmp_up_if_name);
	Ddp = getIfByName(igmp_down_if_name);
	if (Dup == NULL || Ddp == NULL)
		return;
	
	if(Ddp) addVIF(Ddp);
	if(Dup) addVIF(Dup);
#endif
}

#ifdef EMBED
#ifdef CONFIG_DEV_xDSL
#include <rtk/adslif.h>
#if defined(CONFIG_DSL_ON_SLAVE)
static char adslDevice[] = "/dev/xdsl_ipc";
#else
static char adslDevice[] = "/dev/adsl0";
#endif

static FILE* adslFp = NULL;
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) {
		printf("ERROR: failed to open %s, error(%s)\n",adslDevice, strerror(errno));
		return 0;
	};
	return 1;
}

static void close_adsl_drv(void)
{
	if(adslFp)
		fclose(adslFp);

	adslFp = NULL;
}

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
	if(open_adsl_drv()) {
		obcif_arg	myarg;
	    	myarg.argsize = (int) len;
	    	myarg.arg = (int) (rValue);

		if (ioctl(fileno(adslFp), id, &myarg) < 0) {
//	    	        printf("ADSL ioctl failed! id=%x\n",id );
			close_adsl_drv();
			return 0;
	       };

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}
#endif  //#ifdef CONFIG_DEV_xDSL
#endif // of EMBED

// Kaohj --- reset multicast mcft and MFC, send general IGMP Query
void sigResetMRT(int signum)
{
	(void)signum;
	reset_mrouter_flag = 1;
}

void doResetMRT()
{
	struct mcft_entry *mcp = mcpq;

#if defined(EMBED) && defined(CONFIG_DEV_xDSL) && !(defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN))
	Modem_LinkSpeed vLs;
#endif
	igmpproxy_log(LOG_INFO,0, "igmpproxy: SIGUSR2 caught\n");
#ifdef SEND_GENERAL_QUERY
	untimeout(&general_query_timer.ch);
#endif
#ifdef SEND_UNSOLICIT_REPORT
	untimeout(&unsolicit_report_timer.ch);
#endif
	untimeout(&qtimer.ch);
	IGMP_rx_enable = 0;
#if defined(EMBED) && defined(CONFIG_DEV_xDSL) && !(defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN))
	if ( adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
			RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0) // DSL link up
#else
	if(1)
#endif
	{
		// delay sending IGMP Query on link-up
		printf("igmpproxy: DSL up\n");
		// wait seconds to have more stable stream
		sleep(6);
	while(mcp) {
			//del_membership(mcp->grp_addr);
			add_membership(mcp);
			add_mfc(mcp, 0);
			add_mcft_timer(mcp);
		mcp = mcp->next;
	}
		IGMP_rx_enable = 1;
	qtimer.type = TIMER_DELAY_QUERY;
	qtimer.retry_left = 3;
	timeout(igmp_timer_expired , &qtimer, 2, &qtimer.ch);
#ifdef SEND_GENERAL_QUERY
	// Kaohj -- start general IGMP Query
	general_query_timer.type = TIMER_GENERAL_QUERY;
	timeout(igmp_timer_expired , &general_query_timer, general_query_interval, &general_query_timer.ch);
#endif
#ifdef SEND_UNSOLICIT_REPORT
	unsolicit_report_timer.type = TIMER_UNSOLT_REPORT;
  timeout(igmp_timer_unsolicit_report, &unsolicit_report_timer,
  																					UNSOLT_REPORT_INTERVAL, &unsolicit_report_timer.ch);
#endif
	}
	else {
		printf("igmpproxy: DSL down\n");
		IGMP_rx_enable = 0;
		while(mcp) {
			del_mfc(mcp);
			del_mcft_timer(mcp);
			del_membership(mcp);
			mcp = mcp->next;
		}
	}
}


static int initMRouter(void)
/*
** Inits the necessary resources for MRouter.
**
*/
{
	int Err;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	int i;
	int proxyUp = 0;
	int ret;
#endif
	struct IfDesc *Ddp, *Dup;

	buildIfVc();
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	// check upstream
	for(i = 0;i<igmp_up_if_num;i++)
	{
		Dup = getIfByName(igmp_up_if_name[i]);
		if (Dup==NULL || Dup->InAdr.s_addr==0)
			continue;
		else
			proxyUp = 1;
	}
	if(proxyUp == 0)
	{
		return 0;
	}
#endif

	switch( Err = enableMRouter() ) {
		case 0: break;
		case EADDRINUSE: igmpproxy_log( LOG_ERR, EADDRINUSE, "MC-Router API already in use" ); break;
		default: igmpproxy_log( LOG_ERR, Err, "MRT_INIT failed" );
	}
   #ifdef CONFIG_IGMPPROXY_MULTIWAN
        Ddp = getIfByName(igmp_down_if_name);

	if (Ddp==NULL )
		return 0;

	/* add downstream interface */
	ret = addVIF(Ddp);
	if(ret == -1)
		return 0;
	
	igmp_down_if_idx = ret;

	/* add upstream interface */
	for(i = 0;i<igmp_up_if_num;i++)
	{
		Dup = getIfByName(igmp_up_if_name[i]);
		if (Dup==NULL || Dup->InAdr.s_addr==0)
			continue;
		
		ret = addVIF(Dup);
		if(ret == -1)
			return 0;
		igmp_up_if_idx[i] = ret;
	}
   #else
	Ddp = getIfByName(igmp_down_if_name);
	Dup = getIfByName(igmp_up_if_name);
	if (Ddp==NULL || Dup==NULL)
		return 0;

	/* add downstream interface */
	igmp_down_if_idx = addVIF(Ddp);

	/* add upstream interface */
	if (Dup->InAdr.s_addr!=0)
		igmp_up_if_idx = addVIF(Dup);
#endif

  //atexit( clean );
  	MRouter_created = 1;
  	return 1;
}

void
write_pid()
{
	FILE *fp = fopen(pidfile, "w+");
	if (fp) {
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}
	else
	 	printf("Cannot create pid file\n");
}

void
clear_pid()
{
	FILE *fp = fopen(pidfile, "w+");
	if (fp) {
		fprintf(fp, "%d\n", 0);
		fclose(fp);
	}
	else
	 	printf("Cannot create pid file\n");
}


extern int MRouterFD;

void printUsage()
{
    printf("Usage: igmpproxy [-c 1|2|3] -d <down if> -u <up if1[,up if2,up if3...]>\n\n");
    printf("-c: IGMP Query Version config \n\t1: Auto detection\n\t2: Fix to use V2 to send Query\n\t3: Fix to use V3 to send query\n");
    printf("\tif there is no -c option, default using Auto dectection\n");
    printf("-d: Downstream interface config\n");
    printf("-u: Upstream interfaces config, use ',' to seperate interfaces. Ex: nas0_0,vc0\n\n");
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
    printf("-f: configuration file. ex: /tmp/igmpcfg \n");
#endif
    printf("Example: igmpproxy -c 1 -d br0 -u nas0_0,vc0\n");
    printf("Example: igmpproxy -d br0 -u nas0_0\n");
}

/*
 *	A FIFO special file is created by init_fifo() and recv_fifo() is managed
 *	to receive command and process it.
 */
#ifdef EMBED
char IGMP_FIFO[]="/var/run/igmp.fifo";
#else
char IGMP_FIFO[]="igmp.fifo";
#endif

/*
 * Create a FIFO special file IGMP_FIFO[] and open it for reading with
 * nonblocking mode.
 */
int init_fifo(void)
{
	if (access(IGMP_FIFO, F_OK) == -1)
	{
		if (mkfifo(IGMP_FIFO, 0644) == -1)
			printf("access %s failed: %s\n", IGMP_FIFO, strerror(errno));
	}
	
	fifofd = open(IGMP_FIFO, O_RDWR);
	if (fifofd == -1)
		printf("open %s failed\n", IGMP_FIFO);
	return fifofd;
}

int remove_fifo(void)
{
	if (fifofd >=0) {
		close(fifofd);
		unlink(IGMP_FIFO);
	}
	return 0;
}

struct command
{
	char	*cmd;
	int	(*func)(char *arg);
	char	*help;
};

static struct command cmds[];

static int show_usage()
{
	int i;
	
	printf("Usage:\n");
	printf("\techo [cmd] > /var/run/igmp.fifo\n");
	for (i=0; cmds[i].cmd!=NULL; i++) {
		printf("\t%s\n", cmds[i].help);
	}
	return 0;
}

static int set_loglevel(char *arg)
{
	int num;
	
	if (arg==NULL)
		printf("Log2Stderr = %d\n", Log2Stderr);
	else {
		num = atoi(arg);
		if (num > 0) {
			Log2Stderr = num;
			printf("Set Log2Stderr to %d\n", Log2Stderr);
		}
	}
	return 0;
}

#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
void set_parameter(char *cfg_path);
#else
void set_parameter(void);
#endif
void reload_igmp_cfg_file(int signum)
{
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
	printf("reload cfg file:%s\n", igmpcfg_setting.cfg_path);
	set_parameter(igmpcfg_setting.cfg_path);
	return;
#else
	set_parameter();
#endif
}
static int dump_igmp_cfg_setting()
{
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
	printf("=======igmp cfg file info start===========\n");
	printf("cfg_path:%s\n",(igmpcfg_setting.cfg_path)?igmpcfg_setting.cfg_path:"NULL");
	printf("query_interval:%d\n", igmpcfg_setting.query_interval);
	printf("query_response_interval:%d \n", igmpcfg_setting.query_response_interval);
	printf("last_member_query_count:%d\n", igmpcfg_setting.last_member_query_count);
	printf("robust:%d\n", igmpcfg_setting.robust);
	printf("group_leave_delay:%d\n", igmpcfg_setting.group_leave_delay);
	printf("fast leave:%d\n", igmpcfg_setting.fast_leave);
	printf("=======igmp cfg file info end===========\n");
#endif

	printf("=======final igmp cfg start===========\n");
	printf("general_query_interval:%d\n", general_query_interval);
	printf("query_response_interval:%d \n", query_response_interval);
	printf("last_member_query_count:%d\n", last_member_query_count);
	printf("robust_count:%d\n", robust_count);
	printf("last_member_query_interval:%d\n", last_member_query_interval);
	printf("fast_leave:%d\n", fast_leave);
	printf("=======final igmp cfg file info end===========\n");
	
	return 0;
}

static struct command cmds[] = {
	{"help", show_usage, "[help] Show usage."},
	{"log", set_loglevel, "[log <level>] set debug loglevel; show loglevel if <level> not specified."},
	{"mcft", show_mcft_list, "[mcft <group addr>] show subscripbers of <group addr>; show local mcft list if <group addr> not specified."},
	{"dumpcfg", dump_igmp_cfg_setting, "[dumpcfg] dump the input cfg content."},
	{NULL, NULL, ""}
};

static int parse_cmd(char *cmd)
{
	char *s;
	int i;
	
	s = strtok(cmd, " \n");
	for (i=0; cmds[i].cmd!=NULL; i++) {
		if (!strcmp(s, cmds[i].cmd)) {
			s = strtok(NULL, " \n");
			cmds[i].func(s);
			break;
		}
	}
	return 0;
}

// Read fifo command and process it.
// ex. # echo log 8 > /var/run/igmp.fifo
void recv_fifo(void)
{
	char buffer[256];
	int n;
	n = read(fifofd, buffer, sizeof(buffer)-1);
	if (n > 0) {
		buffer[n] = 0;
		parse_cmd(buffer);
	}
}

#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
static void init_igmpcfg_setting(struct igmpcfg* cfg)
{
	cfg->cfg_path=NULL;
	cfg->query_interval=-1;
	cfg->query_response_interval=-1;
	cfg->last_member_query_count=-1;
	cfg->robust=-1;
	cfg->group_leave_delay=-1;
	cfg->fast_leave=-1;
}

static void parser_igmp_cfg_setting(struct igmpcfg* cfg, char *index, char *value) 
{	
	if(0==strcmp(index, "query_interval")){
		int interval = atoi(value);
		if(interval < 0 ) return;
		cfg->query_interval = interval;
		//printf("IGMP Parsing: Name:%s value:%d\n", index, interval);	
		return;
	}
	if(0==strcmp(index, "query_response_interval")){
		int interval = atoi(value);
		if(interval < 0 ) return;
		cfg->query_response_interval = interval;
		//printf("IGMP Parsing: Name:%s value:%d\n", index, interval);
		return;
	}
	if(0==strcmp(index, "last_member_query_count")){
		int count = atoi(value);
		if(count < 0 ) return;
		cfg->last_member_query_count = count;
		//printf("IGMP Parsing: Name:%s value:%d\n", index, count);
		return;
	}
	if(0==strcmp(index, "robust")){
		int robust = atoi(value);
		if(robust < 0 ) return;
		cfg->robust = robust;
		//printf("IGMP Parsing: Name:%s value:%d\n", index, robust);		
		return;
	}
	if(0==strcmp(index, "group_leave_delay")){
		int delay= atoi(value);
		if(delay < 0) return;
		cfg->group_leave_delay = delay;
		//printf("IGMP Parsing: Name:%s value:%d\n", index, delay);
		return;
	}
	if(0==strcmp(index, "fast_leave")){
		int fast_leave = atoi(value);
		if(fast_leave < 0 || fast_leave > 1) return;
		cfg->fast_leave = fast_leave;
		return;
	}
	printf("IGMP Parsing: Unsupportted parameter:%s value:%s\n", index, value);
	return;
}

static void apply_igmp_cfg_setting(struct igmpcfg* cfg)
{
	if(cfg->query_interval!=-1){
		general_query_interval=cfg->query_interval;
	}else{	    
		general_query_interval=15; /*Default value*/
	}

	if(cfg->query_response_interval!=-1){
		query_response_interval=cfg->query_response_interval;
	}else{
		query_response_interval=100;
	}

	if(cfg->last_member_query_count!=-1){
		last_member_query_count=cfg->last_member_query_count;
	}else{
		last_member_query_count=2; /*Default value*/
	}

	if(cfg->robust!=-1){
		robust_count=cfg->robust;
	}else{
		robust_count=2;/*Default value*/
	}

	//last_member_query_interval should be setted after last_member_query_interval;
	if(cfg->group_leave_delay!=-1){
		last_member_query_interval=cfg->group_leave_delay;
		if(last_member_query_count)
			last_member_query_interval/=(100*last_member_query_count);
	}else{
		last_member_query_interval=2000; /*Default value*/
		if(last_member_query_count)
			last_member_query_interval/=(100*last_member_query_count);
	}

	if(cfg->fast_leave!=-1){
		fast_leave = cfg->fast_leave;
	}else{
		fast_leave = 0; /*Default disable.*/
	}
}
#endif

#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
void set_parameter(char *cfg_path)
#else
void set_parameter(void)
#endif
{
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
	FILE *ptr;
	char buff[128], parameter_name[128], parameter_value[128];
	init_igmpcfg_setting(&igmpcfg_setting);
	if(cfg_path!=NULL){
		igmpcfg_setting.cfg_path=strdup(cfg_path);
		ptr=fopen(cfg_path,"r");
		if(ptr!=NULL){
			while(fgets(buff, sizeof(buff)-1, ptr) != NULL){
				buff[sizeof(buff)-1] = '\0';
				sscanf(buff, "%s %s", (char*)&parameter_name, (char*)&parameter_value);
				parameter_name[sizeof(parameter_name)-1] = '\0';
				parameter_value[sizeof(parameter_value)-1] = '\0';
				parser_igmp_cfg_setting(&igmpcfg_setting, parameter_name, parameter_value);
			}
			fclose(ptr);
		}else{
			printf("igmpproxy cfg input path error! Path: %s\n", cfg_path);
		}
		//dump_igmp_cfg_setting();
		apply_igmp_cfg_setting(&igmpcfg_setting);
		return;
	}
#else
#ifdef CONFIG_RTK_LIB_MIB
	mib_get(MIB_IGMP_QUERY_INTERVAL,(void*)&general_query_interval);
	mib_get(MIB_IGMP_QUERY_RESPONSE_INTERVAL,(void*)&query_response_interval);
	mib_get(MIB_IGMP_LAST_MEMBER_QUERY_COUNT,(void*)&last_member_query_count);
	mib_get(MIB_IGMP_ROBUST_COUNT,(void*)&robust_count);
	if(mib_get(MIB_IGMP_GROUTP_LEAVE_DELAY,(void*)&last_member_query_interval))
	{
		if(last_member_query_count)
			last_member_query_interval /=(100*last_member_query_count);
		else
			last_member_query_interval = 100;
	}
#endif
#endif
}

int main(int argc, char **argv)
{
	int _argc = 0;
#ifdef	MULTICAST_LANWAN_BIND
	int on=1;
#endif
	char *_argv[9];
	pid_t pid;
	int execed = 0;
	int i;
	char *upstreamIfStr=NULL,*downstreamIfStr=NULL,*igmpVersonConfigStr=NULL;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	int index;
	char *ifStr=NULL;
	char *saveptr=NULL;
#endif
	char *cfg_path=NULL;

	/*
	 * Max argc: igmpproxy -c 1 -d br0 -u nas0_0,vc0 -f cfg_file -D
	 * Min argc: igmpproxy -d br0 -u nas0_0,vc0
	 */
	if ((argc>=11) ||(argc<5))
	{
		printUsage();
		exit(1);
	}

	#ifndef CONFIG_PACKAGE_procd
	if (strcmp(argv[argc-1], "-D") == 0) {
		argc--;
		execed = 1;
	}
	#endif

	while ((i = getopt(argc, argv, "c:d:u:f:q")) != EOF)
	{
		switch(i)
		{
			case 'd':
				downstreamIfStr= strdup(optarg);
				break;
			case 'u':
				upstreamIfStr= strdup(optarg);
				break;
			case 'c':
				igmpVersonConfigStr = strdup(optarg);
				IGMPVersionConfig = atoi(igmpVersonConfigStr);
				if(IGMPVersionConfig ==2 )
					IGMPCurrentVersion = 2;
				break;
			case 'q':
				IGMPPROXY_Debug_Set=1;
				break;
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
			case 'f':
				cfg_path=strdup(optarg);
				break;
#endif
			default:
				printf("Error! igmpproxy not support this argument! -%c\n ",i);
				break;
		}
	}

	#ifndef CONFIG_PACKAGE_procd
	if (!execed) {
		if ((pid = vfork()) < 0) {
			fprintf(stderr, "vfork failed\n");
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}

		for (_argc=0; _argc < argc; _argc++ )
			_argv[_argc] = argv[_argc];
		_argv[0] = runPath;
		_argv[argc++] = "-D";
		_argv[argc++] = NULL;
		execv(_argv[0], _argv);
		/* Not reached */
		fprintf(stderr, "Couldn't exec\n");
		_exit(1);

	} else 
	#endif
	{
		setsid();
	}

#ifdef CONFIG_IGMPPROXY_MULTIWAN
	igmp_up_if_num = 0;
	index =0;

	ifStr =  strtok_r(upstreamIfStr,",\0", &saveptr);
	while(ifStr !=NULL)
	{
		strcpy(igmp_up_if_name[index], ifStr);
		index++;
		igmp_up_if_num++;
		ifStr = strtok_r(NULL,",",&saveptr);
	}
#else
	strcpy(igmp_up_if_name, upstreamIfStr);
#endif
	strcpy(igmp_down_if_name, downstreamIfStr);
	if(upstreamIfStr)
		free(upstreamIfStr);
	if(downstreamIfStr)
		free(downstreamIfStr);
	if(igmpVersonConfigStr)
		free(igmpVersonConfigStr);

	write_pid();
#ifdef CONFIG_USER_IGMPPROXY_SUPPORT_CFG_FILE
	set_parameter(cfg_path);
#else
	set_parameter();
#endif
	if(cfg_path)
		free(cfg_path);
	
	signal(SIGHUP,sig_doUpdate);
	signal(SIGTERM, hup);
	signal(SIGUSR1, sigifup);
	signal(SIGUSR2, sigResetMRT);
	
	while (!initMRouter())
	{
		// Kaohj, polling every 2 seconds
		//printf("initMRouter fail\n");
		sleep(2);
	}
	init_igmp();

	add_fd(MRouterFD);
	
	// use fifo to read running commands
	init_fifo();
	if (fifofd >= 0)
		add_fd(fifofd);

	{int flags;
	flags = fcntl(MRouterFD, F_GETFL);
	if (flags == -1 || fcntl(MRouterFD, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set MRouterFD to nonblock: %m");
	}

#ifdef	MULTICAST_LANWAN_BIND
	if (setsockopt(MRouterFD, IPPROTO_IP, IP_PKTINFO_RTK, &on,sizeof(on)) < 0){
		fprintf(stderr, "Couldn't enable IP_PKTINFO_RTK\n");
		_exit(1);
	}
#endif
	
	printf("IGMP Proxy started.\n");

	/* process loop */
	while(1)
	{
		fd_set in;
		struct timeval tv;
		int ret;
		int recvlen;
		int flag = FROM_LAN;
		struct IfDesc *IfDp;
		char *ifname = NULL;
		struct iphdr *ip = NULL;
		struct igmphdr *igmp = NULL;
		
		IfDp = NULL;
		flag = FROM_LAN;
		ifname = igmp_down_if_name;

		if(update_flag == 1)
		{
			doUpdate();
			update_flag = 0;
		}

		if(ifup_flag == 1)
		{
			doifup();
			ifup_flag = 0;
		}

		if(reset_mrouter_flag == 1)
		{
			doResetMRT();
			reset_mrouter_flag = 0;
		}
// Kaohj
#ifdef _SUPPORT_IGMPV3_
		igmpv3_timer();
#endif
//#else
		calltimeout();
//#endif /*_SUPPORT_IGMPV3_*/

		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		in = in_fds;

		ret = select(max_in_fd+1, &in, NULL, NULL, &tv);

		if( ret <= 0 ){
			//printf("igmp: timeout\n");
			continue;
		}

		if(FD_ISSET(MRouterFD, &in))
		{
#ifndef MULTICAST_LANWAN_BIND
			struct iovec iov;
			struct cmsghdr *cmsg;
			struct in_pktinfo *pktinfo;
			struct sockaddr_in from2;
			struct msghdr msg;

			unsigned int ifindex = 0;

			char adata[100];
			struct ifreq ifr;

			msg.msg_name = (void *) &from2;                    // The from2 is src_IP of packet
			msg.msg_namelen = sizeof (struct sockaddr_in);
			msg.msg_iov = &iov;
			msg.msg_iovlen = 1;
			msg.msg_control = (void *) adata;                  // This is a buf for ancillary data. Such as ifindex.
			msg.msg_controllen = sizeof adata;

			iov.iov_base = recv_buf;                          // The is a buf for the desired data. Such as RIP Packet(RIP header + RIP payload).
			iov.iov_len = RECV_BUF_SIZE;

			recvlen = recvmsg(MRouterFD, &msg, 0);
			if(recvlen < 0)
			{
				if (errno != EINTR && errno !=EAGAIN) igmpproxy_log(LOG_ERR, errno, "recvfrom");
				continue;
			}

			if (recvlen < sizeof(struct iphdr)) 
			{
				igmpproxy_log(LOG_WARNING, 0, "received packet too short (%u bytes) for IP header", recvlen);
				continue;
			}

			ip = (struct iphdr *)recv_buf;
			igmp = (struct igmphdr *)(recv_buf + (ip->ihl << 2));

			/* only deal with normal packet */
			if (igmp->type != IGMPMSG_NOCACHE){
				cmsg = CMSG_FIRSTHDR (&msg);
				if (cmsg != NULL && cmsg->cmsg_level == IPPROTO_IP
													&& cmsg->cmsg_type  == IP_PKTINFO )
				{
						pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);

						ifindex = pktinfo->ipi_ifindex;

						memset(&ifr, 0, sizeof(ifr));
						ifr.ifr_ifindex = ifindex;

						//printf("[%s . %d]The ifr.ifr_ifindex is %d\n", __FILE__, __LINE__, ifr.ifr_ifindex);

						if (ioctl(MRouterFD, SIOCGIFNAME, &ifr))
						{
								igmpproxy_log(LOG_WARNING, errno, "Get iface name error");
								continue;
						}
				}

				if(!strncmp(ifr.ifr_name, igmp_down_if_name, sizeof(igmp_down_if_name)))
					flag = FROM_LAN;
				else
					flag = FROM_WAN;
				
				ifname = ifr.ifr_name;
			}
#else
			if((recvlen = rtk_recv_message()) < 0){
				//fprintf(stderr,"get lan wan binding info error\n");
				continue;
			}
#endif
			igmpproxy_log(LOG_DEBUG, 0, "recvfrom IGMP packet from intf(%s)", (ifname)?ifname:"");
			if(ifname) 
				IfDp = getIfByName(ifname);
			if(IGMP_rx_enable)
			{
#ifdef _SUPPORT_IGMPV3_
					igmpv3_accept(recvlen, IfDp, flag);
#else
					igmp_accept(recvlen, IfDp, flag);
#endif
			}
		}
		else if (FD_ISSET(fifofd, &in))
			recv_fifo(); // read commands from fifo
	}
	clearMcFlow();
	return 0;
}

