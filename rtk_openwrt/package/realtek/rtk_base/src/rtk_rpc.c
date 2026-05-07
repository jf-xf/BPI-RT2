#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/syslog.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"
#include "common.h"
#include "fc_api.h"
#include "pon.h"

#include "rtk/ponmac.h"
#ifdef CONFIG_GPON_FEATURE
#include "rtk/gpon.h"
#endif
#ifdef CONFIG_EPON_FEATURE
#include "rtk/epon.h"
#endif
#include "rtk/stat.h"
#include "rtk/switch.h"
#ifdef CONFIG_COMMON_RT_API
#include <rtk/rt/rt_stat.h>
#include <rtk/rt/rt_rate.h>
#include <rtk/rt/rt_l2.h>
#include <rtk/rt/rt_qos.h>
#ifdef CONFIG_EPON_FEATURE
#include <rtk/rt/rt_epon.h>
#endif
#ifdef CONFIG_GPON_FEATURE
#include <rtk/rt/rt_gpon.h>
#endif
#include <rtk/rt/rt_switch.h>
#include "rt_rate_ext.h"
#include "rt_igmp_ext.h"
#ifdef CONFIG_COMMON_RT_PONMISC
#include "rtk/rt/rt_ponmisc.h"
#endif
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#if defined(CONFIG_GPON_FEATURE)
#include <module/gpon/gpon.h>
#endif
#include <module/intr_bcaster/intr_bcaster.h>
#endif
#endif
#include <common/util/rt_util.h>

#ifdef CONFIG_RTK_OMCI_V1
#include <omci_api.h>
#include <gos_type.h>
#endif

#include <libubox/avl.h>
#include <libubox/usock.h>
#include <libubox/uloop.h>

//#include <rpcd/plugin.h>

#include "rtk_interface.h"
#ifdef CONFIG_USER_LANNETINFO
#include "lanNetInfo.h"
#endif
#include <uci.h>

#ifdef CONFIG_RTK_NETLINK_ROUTE_API
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif

#if defined(CONFIG_RTK_GENERIC_NETLINK_API)
#include "rtk_generic_netlink.h"
#endif

#ifdef CONFIG_RTK_RPC_DYNAMIC_DEBUG
#include <sys/syslog.h>

/*
 * priorities (these are ordered)
LOG_EMERG	0	system is unusable
LOG_ALERT	1	action must be taken immediately
LOG_CRIT	2	critical conditions
LOG_ERR		3	error conditions
LOG_WARNING	4	warning conditions
LOG_NOTICE	5	normal but significant condition
LOG_INFO	6	informational
LOG_DEBUG	7	debug-level messages
 */

static int g_debugLevel = LOG_WARNING;
#define RTKRPC_DEBUG(level, fmt, ...) if(g_debugLevel >= (level)) fprintf(stderr, "[rtk_rpc][%s:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define RTKRPC_DEBUG(level, fmt, args...) {}
#endif

extern char *LAN_PORT_IFNAE[MAX_LAN_NUM];
int read_pid(const char *filename)
{
	FILE *fp;
	int pid = 0;

	if ((fp = fopen(filename, "r")) == NULL)
		return -1;
	if(fscanf(fp, "%d", &pid) != 1 || kill(pid, 0) != 0){
		pid = 0;
	}
	fclose(fp);
	return pid;
}

int uci_get_option_val(char *name_p, char *name_o, char *val, int val_len)
{
	int ret = -1;

	if(!name_p || !name_o || !val || !val_len){
		RTKRPC_DEBUG(LOG_ERR, "invalid arguement.\n");
		return ret;
	}

	struct uci_context *uci_ctx = uci_alloc_context();
	struct uci_package *p = NULL;
	if(UCI_OK != uci_load(uci_ctx, name_p, &p)){
		RTKRPC_DEBUG(LOG_ERR, "uci load %s fail.\n", name_p);
		uci_free_context(uci_ctx);
		return ret;
	}

	struct uci_element *e=NULL;
	uci_foreach_element(&p->sections, e)
	{
		struct uci_section *s=uci_to_section(e);
		if(!s)
			continue;
		struct uci_option *o=uci_lookup_option(uci_ctx, s, name_o);
		if(o){
			if(UCI_TYPE_STRING == o->type){
				snprintf(val, val_len, "%s", o->v.string);
				ret = 0;
				RTKRPC_DEBUG(LOG_DEBUG, "get option(%s) value (%s)-(%s)\n", name_o, o->v.string, val);
				break;
			}
		}else{
			RTKRPC_DEBUG(LOG_ERR, "cannot get value of %s.\n", name_o);
		}
	}

	uci_unload(uci_ctx, p);
	uci_free_context(uci_ctx);
	return ret;
}

#if defined(ARP_LOOP_DETECTION)
#define MAP_CHECKER_PID_FILE      "/var/run/map_checker.pid"
#define NUM_WLAN_INTERFACE        2
#define	IFNAMSIZ	              16

void trigger_map_checker_down_vxd(char *ifname)
{
	uint8_t map_state = 0;
	char val[64]={0};
	if(uci_get_option_val("rtkmap", "controller", val, sizeof(val))==0){
		map_state = atoi(val);
	}

	RTKRPC_DEBUG(LOG_DEBUG, "map_state:%d, ifname:%s\n", map_state, ifname);
	if(map_state == 2)
	{
		int map_checker_pid = 0;
		char pid_file[64] = {0};
		snprintf(pid_file, sizeof(pid_file), MAP_CHECKER_PID_FILE);
		map_checker_pid = read_pid(pid_file);
		if (map_checker_pid > 0)
		{
			RTKRPC_DEBUG(LOG_DEBUG, "kill SIGUSR2 to map_checker.\n");
			kill(map_checker_pid, SIGUSR2);
		}
	}
}

void link_change_notify_to_map_checker(char *ifname)
{
	uint8_t map_state = 0;
	char val[64]={0};
	if(uci_get_option_val("rtkmap", "controller", val, sizeof(val))==0){
		map_state = atoi(val);
	}

	RTKRPC_DEBUG(LOG_DEBUG, "map_state:%d, ifname:%s\n", map_state, ifname);
	if((map_state == 2)&&
		((strncmp(ifname, ALIASNAME_ELAN_PREFIX, strlen(ALIASNAME_ELAN_PREFIX))==0)||
		(strncmp(ifname, ALIASNAME_NAS, strlen(ALIASNAME_NAS))==0)||
		(strstr(ifname, ALIASNAME_VXD))))
	{
		int map_checker_pid = 0;
		char pid_file[64] = {0};
		snprintf(pid_file, sizeof(pid_file), MAP_CHECKER_PID_FILE);
		map_checker_pid = read_pid(pid_file);
		if (map_checker_pid > 0)
		{
			RTKRPC_DEBUG(LOG_DEBUG, "kill SIGUSR1 to map_checker.\n");
			kill(map_checker_pid, SIGUSR1);
		}
	}
}
#endif

#if defined(CONFIG_RTK_GENERIC_NETLINK_API)
int g_init_wlan_genetlink = 0;
#define MAX_PAYLOAD                   2048
#define WIFI_EZMESH_ARP_LOOP          25

struct wlan_with_reason_event{
	unsigned char mac[6];
	char reason;
	char if_name[IFNAMSIZ];
};

/**
 * rtk_nla_attr_size - length of attribute size, NOT including padding
 * @param payload   length of payload
 * @return
 */
int rtk_nla_attr_size(int payload)
{
	return NLA_HDRLEN + payload;
}

/**
 * rtk_nla_total_size - total length of attribute including padding
 * @param payload   length of payload, NOT including NLA_HDR
 */
int rtk_nla_total_size(int payload)
{
	return NLA_ALIGN(rtk_nla_attr_size(payload));
}

/**
 * rtk_genlmsg_alloc - allocate generic netlink socket buffer
 * @family_hdr: genl family header length
 * @payload_size: genl payloal length
 *
 * Returns:
 *	NULL if fail
 *	socket buffer pointer if success
 */
void *rtk_genlmsg_alloc(int family_hdr, int payload_size)
{
    char *buf;
    int len;

	len = GENLMSG_TOTAL_LEN(family_hdr, payload_size);
    buf = (char *)malloc(len);
    if (!buf)
        return NULL;

    memset(buf, 0, len);
    return buf;
}

void rtk_genlmsg_free(void *buf)
{
	if (buf)
		free(buf);
}

int rtk_genlmsg_send(int sockfd, unsigned short nlmsg_type, unsigned int nlmsg_pid,
        unsigned char genl_cmd, unsigned char genl_version, unsigned int family_hdr,
        unsigned short nla_type, const void *nla_data, unsigned int nla_len)
{
	struct nlmsghdr *nlh = NULL;    //netlink message header
	struct genlmsghdr *glh = NULL;  //generic netlink message header
	void *userhdr = NULL;			//user specific header
	struct nlattr *nla = NULL;      //netlink attribute header
	struct sockaddr_nl nladdr;
	unsigned char *buf = NULL;
	int len = 0, count = 0, ret = 0;

	if ((nlmsg_type == 0) || (!nla_data) || (nla_len <= 0)){
		return -1;
	}

	buf = rtk_genlmsg_alloc(family_hdr, nla_len);
	if (!buf)
		return -1;

	len = GENLMSG_TOTAL_LEN(family_hdr, nla_len);	// get total nlmsg length

	nlh = (struct nlmsghdr *)buf;
	nlh->nlmsg_len = len;
	nlh->nlmsg_type = nlmsg_type;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = nlmsg_pid;

	glh = (struct genlmsghdr *)NLMSG_DATA(nlh);
	glh->cmd = genl_cmd;
	glh->version = genl_version;

	userhdr = (void *)GENLMSG_DATA(glh);
	if(family_hdr){
		RTKRPC_DEBUG(LOG_DEBUG, "TODO: create user header here.\n");
	}

	nla = (struct nlattr *)((char*)userhdr + family_hdr);	//the first attribute
	nla->nla_type = nla_type;
	nla->nla_len = rtk_nla_attr_size(nla_len);
	memcpy(NLA_DATA(nla), nla_data, nla_len);

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	count = 0;
	ret = 0;
	do {
		ret = sendto(sockfd, &buf[count], len - count, 0,
						(struct sockaddr *)&nladdr, sizeof(nladdr));
		if (ret < 0){
			if (errno != EAGAIN) {
				count = -1;
				goto out;
			}
		} else {
			count += ret;
		}
	}while (count < len);

out:
	rtk_genlmsg_free(buf);
	return count;
}

/**
 *
 * @param sockfd    generic netlink socket fd
 * @param buf       the 'buf' is including the struct nlmsghdr,
 *                  struct genlmsghdr and struct nlattr
 * @param len       size of 'buf'
 * @return  >0      size of genlmsg
 *          <0      error occur
 */
int rtk_genlmsg_recv(int sockfd, unsigned char *buf, unsigned int len)
{
	struct sockaddr_nl nladdr;
	struct msghdr msg;
	struct iovec iov;
	int ret;

	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = getpid();
	//    nladdr.nl_groups = 0xffffffff;

	iov.iov_base = buf;
	iov.iov_len = len;

	msg.msg_name = (void *)&nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	ret = recvmsg(sockfd, &msg, 0);
	ret = ret > 0 ? ret : -1;

	RTKRPC_DEBUG(LOG_DEBUG, "recv return %d\n", ret);
	return ret;
}

int rtk_genlmsg_dispatch(struct nlmsghdr *nlmsghdr, unsigned int nlh_len, int nlmsg_type,
						unsigned int family_hdr, int nla_type, unsigned char *buf, int *len)
{
	struct nlmsghdr *nlh=NULL;
	struct genlmsghdr *glh=NULL;
	void *userhdr = NULL;			//user specific header
	struct nlattr *nla=NULL;
	int nla_len=0, l=0, i=0, ret=-1;

	if (!nlmsghdr || !buf || !len)
		return ret;

	memset(buf, 0, *len);

	if (nlmsg_type && (nlmsghdr->nlmsg_type != nlmsg_type)){
		RTKRPC_DEBUG(LOG_DEBUG, "nlmsg_type=%d, nlmsghdr->nlmsg_type=%d\n", nlmsg_type, nlmsghdr->nlmsg_type);
		return ret;
	}

	for (nlh = nlmsghdr; NLMSG_OK(nlh, nlh_len); nlh = NLMSG_NEXT(nlh, nlh_len))
	{
		/* The end of multipart message. */
		if (nlh->nlmsg_type == NLMSG_DONE){
			RTKRPC_DEBUG(LOG_DEBUG, "get NLMSG_DONE\n");
			ret = 0;
			break;
		}

		if (nlh->nlmsg_type == NLMSG_ERROR){
			RTKRPC_DEBUG(LOG_DEBUG, "get NLMSG_ERROR\n");
			ret = -1;
			break;
		}

		glh = (struct genlmsghdr *)NLMSG_DATA(nlh);
		userhdr = (void *)GENLMSG_DATA(glh);
		if(family_hdr){
			RTKRPC_DEBUG(LOG_DEBUG, "TODO: create user header here.\n");
		}

		nla = (struct nlattr *)((char*)userhdr + family_hdr);	//the first attribute
		nla_len = nlh->nlmsg_len - GENL_HDRLEN;					//len of attributes
		for (i = 0; NLA_OK(nla, nla_len); nla = NLA_NEXT(nla, nla_len), ++i)
		{
			/* Match the family ID, copy the data to user */
			if (nla_type == nla->nla_type)
			{
				l = nla->nla_len - NLA_HDRLEN;
				*len = (*len > l)? l : *len;
				memcpy(buf, NLA_DATA(nla), *len);
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int rtk_genlmsg_get_family_id(int sockfd, const char *family_name)
{
	void *buf=NULL;
	int len;
	unsigned short id;
	int l;
	int ret;

	ret = rtk_genlmsg_send(sockfd, GENL_ID_CTRL, 0, CTRL_CMD_GETFAMILY, 1, 0,
			CTRL_ATTR_FAMILY_NAME, family_name, strlen(family_name) + 1);
	if (ret < 0){
		RTKRPC_DEBUG(LOG_ERR, "rtk genlmsg send fail.\n");
		return -1;
	}

	len = 256;
	buf = rtk_genlmsg_alloc(0, 256);
	if (!buf){
		RTKRPC_DEBUG(LOG_ERR, "rtk genlmsg alloc fail.\n");
		return -1;
	}

	len = GENLMSG_TOTAL_LEN(0, 256);	// get total nlmsg length
	len = rtk_genlmsg_recv(sockfd, buf, len);
	if (len < 0){
		RTKRPC_DEBUG(LOG_ERR, "rtk genlmsg recv fail.\n");
		rtk_genlmsg_free(buf);
		return len;
	}

	id = 0;
	l = sizeof(id);
	rtk_genlmsg_dispatch((struct nlmsghdr *)buf, len, 0, 0, CTRL_ATTR_FAMILY_ID, (unsigned char *)&id, &l);

	rtk_genlmsg_free(buf);

	return id > 0 ? id : -1;
}

int rtk_genlmsg_get_group_id(int sockfd, const char *family_name, const char *grp_name)
{
	void *buf;
	int len;
	unsigned char data[1024];
	unsigned int data_len=0, sub_nla_len=0;
	int nla_len=0, i=0, j=0;
	int ret;
	struct nlattr *nla=NULL, *sub_nla=NULL;
	int grp_id=0, found=0;

	ret = rtk_genlmsg_send(sockfd, GENL_ID_CTRL, 0, CTRL_CMD_GETFAMILY, 1, 0,
			CTRL_ATTR_FAMILY_NAME, family_name, strlen(family_name) + 1);
	if (ret < 0){
		RTKRPC_DEBUG(LOG_ERR, "rtk genlmsg send fail.\n");
		return -1;
	}

	len = 256;
	buf = rtk_genlmsg_alloc(0, 256);
	if (!buf){
		RTKRPC_DEBUG(LOG_ERR, "rtk genlmsg alloc fail.\n");
		return -1;
	}

	len = rtk_genlmsg_recv(sockfd, buf, len);
	if (len < 0){
		RTKRPC_DEBUG(LOG_ERR, "rtk genlmsg recv fail.\n");
		rtk_genlmsg_free(buf);
		return len;
	}

	memset(data, 0, sizeof(data));
	data_len = sizeof(data);
	rtk_genlmsg_dispatch((struct nlmsghdr *)buf, len, 0, 0, CTRL_ATTR_MCAST_GROUPS, (unsigned char *)data, &data_len);

	nla = (struct nlattr *)data;
	/* genl group is follow nlattr format */
	for (i = 0; !found && NLA_OK(nla, data_len); nla = NLA_NEXT(nla, data_len), ++i){
		sub_nla = NLA_DATA(nla);
		sub_nla_len = nla->nla_len;
		for (j = 0; NLA_OK(sub_nla, sub_nla_len); sub_nla = NLA_NEXT(sub_nla, sub_nla_len), ++j){
			if(sub_nla->nla_type == CTRL_ATTR_MCAST_GRP_ID){
				grp_id = *(int *)NLA_DATA(sub_nla);
			}
			if(sub_nla->nla_type == CTRL_ATTR_MCAST_GRP_NAME){
				if(0 == strcmp(grp_name, (char *)NLA_DATA(sub_nla)))
					found=1;
			}
		}
	}

	if(found){
		ret = grp_id;
	}else{
		ret = -1;
	}

	rtk_genlmsg_free(buf);
	return ret;
}

/* receive generic netlink info from wlan driver */
/*
	netlink payload:
		B0~B3                    B4~
		eventID                  data

	data content has the following two types:
		1. with reason
			B0~B5     B6             B7~
			Sta Addr  Reason         Interface Name
		2. with info
			B0~B5     B6-B(len+6-1)  B(len+6)~
			Sta Addr  info           Interface Name
*/
void recv_wlan_genetlink_event(struct uloop_fd *u, unsigned int events)
{
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nh=NULL;
	struct genlmsghdr *glh=NULL;
	struct nlattr *nla=NULL;
	int nla_len=0, i=0, len=0;
	char buf[MAX_PAYLOAD]={0};
	struct iovec iov = { buf, sizeof buf };
	struct msghdr msg;
	char *wlan_event_data = NULL;
	int event_id=0, attr_len=0;
	int sock_fd = u->fd;

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	//dest_addr.nl_groups = 0; /* unicast */
	dest_addr.nl_groups = 0xffffffff;

	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Read message from kernel */
	len = recvmsg(sock_fd, &msg, 0);
	if(len>0){
		RTKRPC_DEBUG(LOG_DEBUG, "recv msg.\n");
		for (nh = (struct nlmsghdr *) buf; NLMSG_OK (nh, len); nh = NLMSG_NEXT (nh, len))
		{
			/* The end of multipart message. */
			if (nh->nlmsg_type == NLMSG_DONE){
				RTKRPC_DEBUG(LOG_DEBUG, "NLMSG_DONE.\n");
				return;
			}
			if (nh->nlmsg_type == NLMSG_ERROR){
				RTKRPC_DEBUG(LOG_DEBUG, "NLMSG_ERROR.\n");
				return;
			}
			glh = (struct genlmsghdr *)NLMSG_DATA(nh);
			if(glh->cmd == RTL_WLAN_INDICATE_CMD_EVENT){
				RTKRPC_DEBUG(LOG_DEBUG, "recv cmd RTL_WLAN_INDICATE_CMD_EVENT.\n");
				nla = (struct nlattr *)GENLMSG_DATA(glh);	//the first attribute
				nla_len = nh->nlmsg_len - GENL_HDRLEN;		//len of attributes
				wlan_event_data = NULL;
				for (i = 0; NLA_OK(nla, nla_len); nla = NLA_NEXT(nla, nla_len), ++i)
				{
					RTKRPC_DEBUG(LOG_DEBUG, "[%d]nla->nla_type = %d, nla_len:%d.\n", i, nla->nla_type, nla->nla_len);
					if (RTL_WLAN_INDICATE_ATTR_MSG == nla->nla_type)
					{
						wlan_event_data = NLA_DATA(nla);
						attr_len = nla->nla_len;
						RTKRPC_DEBUG(LOG_DEBUG, "recv attr RTL_WLAN_INDICATE_ATTR_MSG(attr len:%d).\n", attr_len);
						break;
					}
				}
				break;
			}
		}

		if(wlan_event_data){
			memcpy(&event_id, wlan_event_data, sizeof(int));
			RTKRPC_DEBUG(LOG_DEBUG, "event_id:%d\n", event_id);
			switch (event_id){
#ifdef ARP_LOOP_DETECTION
				case WIFI_EZMESH_ARP_LOOP:
					RTKRPC_DEBUG(LOG_DEBUG, "recv WIFI_EZMESH_ARP_LOOP\n");
					struct wlan_with_reason_event *wlan_event_data_with_reason = (struct wlan_with_reason_event *)(wlan_event_data+sizeof(int));
					unsigned char wlan_ifname[10]={0};
					snprintf(wlan_ifname, sizeof(wlan_ifname), "%s", wlan_event_data_with_reason->if_name);
					RTKRPC_DEBUG(LOG_DEBUG, "ifname:%s, reason:%d, mac:%02x%02x%02x%02x%02x%02x\n",
						wlan_ifname, wlan_event_data_with_reason->reason,
						wlan_event_data_with_reason->mac[0], wlan_event_data_with_reason->mac[1], wlan_event_data_with_reason->mac[2],
						wlan_event_data_with_reason->mac[3], wlan_event_data_with_reason->mac[4], wlan_event_data_with_reason->mac[5]);
					trigger_map_checker_down_vxd(wlan_ifname);
					break;
#endif
				default:
					RTKRPC_DEBUG(LOG_DEBUG, "unknown event_id %d\n", event_id);
					break;
			}
		}
	}
	return;
}

struct uloop_fd wlan_genetlink_event_uloop;
int init_wlan_genetlink_event()
{
	int event_sock_fd=-1, retval;
	struct sockaddr_nl src_addr;
	int genl_family_id = 0, genl_grp_id = 0, ret = 0;

	if((event_sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_GENERIC))== -1)
	{
		RTKRPC_DEBUG(LOG_ERR, "Create Netlink Event Socket Fail!\n");
		return -1;
	}

	if(event_sock_fd>0)
	{
		memset(&src_addr, 0, sizeof(src_addr));
		src_addr.nl_family = AF_NETLINK;
		src_addr.nl_pid = getpid();

		retval = bind(event_sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
		if(retval < 0)
		{
			RTKRPC_DEBUG(LOG_ERR, "bind event_sock_fd fail!!!\n");
			close(event_sock_fd);
			return -1;
		}
	}

	/* get genl family id from genl control family */
	genl_family_id = rtk_genlmsg_get_family_id(event_sock_fd, RTL_GENL_WLAN_EVENT);
	if (genl_family_id <= 0)
		RTKRPC_DEBUG(LOG_ERR, "rtk_genlmsg_get_family_id failed for %s.\n", RTL_GENL_WLAN_EVENT);
	RTKRPC_DEBUG(LOG_DEBUG, "get family(%s) ID[%d]\n", RTL_GENL_WLAN_EVENT, genl_family_id);

	/* get genl group id from genl control family */
	genl_grp_id = rtk_genlmsg_get_group_id(event_sock_fd, RTL_GENL_WLAN_EVENT, RTL_GENL_WLAN_EVENT_GROUP);
	if (genl_grp_id <= 0)
		RTKRPC_DEBUG(LOG_ERR, "rtk_genlmsg_get_group_id failed for %s.\n", RTL_GENL_WLAN_EVENT_GROUP);
	RTKRPC_DEBUG(LOG_DEBUG, "get group(%s) ID[%d]\n", RTL_GENL_WLAN_EVENT_GROUP, genl_grp_id);

	/* Add Group into socket fd */
	if (setsockopt(event_sock_fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &genl_grp_id, sizeof(genl_grp_id)) < 0) {
		RTKRPC_DEBUG(LOG_ERR, "setsockopt NETLINK_ADD_MEMBERSHIP fail for group id(%d) < 0, %s\n", genl_grp_id, strerror(errno));
	}
	RTKRPC_DEBUG(LOG_DEBUG, "setsockopt NETLINK_ADD_MEMBERSHIP ok for group id(%d)\n", genl_grp_id);

	memset(&wlan_genetlink_event_uloop, 0, sizeof(wlan_genetlink_event_uloop));
	wlan_genetlink_event_uloop.cb = recv_wlan_genetlink_event;
	wlan_genetlink_event_uloop.fd = event_sock_fd;
	uloop_fd_add(&wlan_genetlink_event_uloop, ULOOP_READ);

	return event_sock_fd;
}
#endif //CONFIG_RTK_GENERIC_NETLINK_API

#ifdef CONFIG_RTK_RPC_DYNAMIC_DEBUG
char RTK_RPC_FIFO[]="/var/run/rtk_rpc.fifo";
/* ex. # echo log 7 > /var/run/rtk_rpc.fifo */
int parse_cmd_fifo(char *cmd)
{
	char *s;
	int num;

	s = strtok(cmd, " \n");
	while (s) {
		if (!strcmp(s, "log")) {
			s = strtok(NULL, " \n");
			if (s) {
				num = atoi(s);
				if (num > 0) {
					g_debugLevel = num;
					RTKRPC_DEBUG(LOG_WARNING, "Set g_debugLevel to %d\n", g_debugLevel);
				}
			}
			break;
		}
		s = strtok(NULL, " \n");
	}
	return 0;
}

void recv_cmd_fifo(struct uloop_fd *u, unsigned int events)
{
	char buffer[256]={0};
	int n;
	int fifofd = u->fd;

	n = read(fifofd, buffer, sizeof(buffer));
	if (n > 0) {
		n=(n > sizeof(buffer)-1) ? (sizeof(buffer)-1) : n;
		buffer[n] = '\0';
		parse_cmd_fifo(buffer);
	}
}

struct uloop_fd cmd_fifo_uloop;
int init_cmd_fifo()
{
	int fifofd = -1;

	if (access(RTK_RPC_FIFO, F_OK) == -1)
	{
		if (mkfifo(RTK_RPC_FIFO, 0644) == -1){
			RTKRPC_DEBUG(LOG_ERR, "access %s failed: %s\n", RTK_RPC_FIFO, strerror(errno));
			return fifofd;
		}
	}

	fifofd = open(RTK_RPC_FIFO, O_RDWR);
	if (fifofd == -1){
		RTKRPC_DEBUG(LOG_ERR, "open %s failed\n", RTK_RPC_FIFO);
		return fifofd;
	}

	memset(&cmd_fifo_uloop, 0, sizeof(cmd_fifo_uloop));
	cmd_fifo_uloop.cb = recv_cmd_fifo;
	cmd_fifo_uloop.fd = fifofd;
	uloop_fd_add(&cmd_fifo_uloop, ULOOP_READ);

	return fifofd;
}
#endif // CONFIG_RTK_RPC_DYNAMIC_DEBUG

#ifdef CONFIG_RTK_NETLINK_ROUTE_API
void link_change_dependency(char *ifname, unsigned int ifi_flags)
{
#ifdef ARP_LOOP_DETECTION
	link_change_notify_to_map_checker(ifname);
#endif
	return;
}

void recv_rtnetlink_event(struct uloop_fd *u, unsigned int events)
{
	int status, ret = 0, len, i;
	char buf[4096] = {0}, tmp_ifname[20]={0};
	struct nlmsghdr *h;
	struct ifinfomsg *ifi;
	struct rtattr *attribute;
	char *ifname = NULL;

	struct iovec iov = { buf, sizeof buf };
	struct sockaddr_nl snl;
	struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };

	int sockint = u->fd;
	status = recvmsg (sockint, &msg, 0);
	if (status < 0)
	{
		/* Socket non-blocking so bail out once we have read everything */
		if (errno == EWOULDBLOCK || errno == EAGAIN){
			RTKRPC_DEBUG(LOG_ERR, "read_netlink: errno: %d\n", errno);
			return;
		}
		/* Anything else is an error */
		RTKRPC_DEBUG(LOG_ERR, "read_netlink: Error recvmsg: %d\n", status);
		return;
	}

	if (status == 0)
	{
		RTKRPC_DEBUG(LOG_DEBUG, "read_netlink: EOF\n");
	}

	// We need to handle more than one message per 'recvmsg'
	for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (unsigned int) status);h = NLMSG_NEXT (h, status))
	{
		if (h->nlmsg_type == NLMSG_DONE)
		{
			RTKRPC_DEBUG(LOG_DEBUG, "read_netlink: NLMSG_DONE\n");
			return;
		}
		if (h->nlmsg_type == NLMSG_ERROR)
		{
			RTKRPC_DEBUG(LOG_DEBUG, "read_netlink: Message is an error - decode TBD\n");
			return;
		}
		if (h->nlmsg_type == RTM_NEWLINK)
		{
			ifi = NLMSG_DATA (h);
			len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
			for (attribute = IFLA_RTA(ifi); RTA_OK(attribute, len); attribute = RTA_NEXT(attribute, len))
			{
				switch(attribute->rta_type)
				{
					case IFLA_IFNAME:
						ifname = (char *) RTA_DATA(attribute);
						break;
					default:
						break;
				}
			}
			if(ifname && (ifi->ifi_family != AF_BRIDGE)) // link change
			{
				RTKRPC_DEBUG(LOG_DEBUG, "%s %s %s\n", ifname, (ifi->ifi_flags & IFF_UP) ? "up" : "down",
					(ifi->ifi_flags & IFF_RUNNING) ? "running" : "not running");

#if defined(CONFIG_RTK_GENERIC_NETLINK_API)
				if(g_init_wlan_genetlink == 0)
				{
					for(i = 0; i < NUM_WLAN_INTERFACE; i++)
					{
						memset(tmp_ifname,0,sizeof(tmp_ifname));
						snprintf(tmp_ifname, sizeof(tmp_ifname), "%s%d", ALIASNAME_WLAN, i);
						if((strlen(tmp_ifname) == strlen(ifname)) && !memcmp(ifname, tmp_ifname, strlen(tmp_ifname)) && (ifi->ifi_flags & IFF_UP))
						{
							g_init_wlan_genetlink = init_wlan_genetlink_event();
							break;
						}
					}
				}
#endif
				link_change_dependency(ifname, ifi->ifi_flags);
			}
		}
	}

	return;
}

struct uloop_fd rtnetlink_event_uloop;
int init_netlink_route_event()
{
	int sock = -1;
	struct sockaddr_nl nl_local;

	if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
	{
		RTKRPC_DEBUG(LOG_ERR, "Create Socket Fail!\n");
		return -1;
	}

	memset (&nl_local,0,sizeof(nl_local));
	nl_local.nl_family = AF_NETLINK;
	nl_local.nl_pad = 0;
	nl_local.nl_pid = getpid();
	nl_local.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_LINK | RTNLGRP_LINK;

	/* Netlink is not a reliable protocol. It tries its best to deliver
	 * a message to its destination(s), but may drop messages when an
	 * out-of-memory condition or other error occurs. */
	int buffersize = 0, bufflen = 0;
	bufflen=sizeof(buffersize);
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffersize, &bufflen);
	buffersize *= 2;	/* set rcv buff to "current rcv buff size" times 2 */
	setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, bufflen);
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffersize, &bufflen);

	if (bind(sock, (struct sockaddr *)&nl_local, sizeof(struct sockaddr)) == -1)
	{
		RTKRPC_DEBUG(LOG_ERR, "bind socket failure\n");
		close(sock);
		return -1;
	}

	memset(&rtnetlink_event_uloop, 0, sizeof(rtnetlink_event_uloop));
	rtnetlink_event_uloop.cb = recv_rtnetlink_event;
	rtnetlink_event_uloop.fd = sock;
	uloop_fd_add(&rtnetlink_event_uloop, ULOOP_READ);

	return sock;
}
#endif //CONFIG_RTK_NETLINK_ROUTE_API

static struct ubus_context *ctx;
static struct blob_buf b;

enum {
	MIB_NAME,
	MIB_VALUE,
	__MIB_MAX
};

static const struct blobmsg_policy mib_format[__MIB_MAX] = {
	[MIB_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING},
	[MIB_VALUE] = { .name = "value", .type = BLOBMSG_TYPE_STRING},
};

static int
rpc_mib_set(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__MIB_MAX];
	struct blob_attr *cur;
	char value_str[2048]={0};
	int ret = 0;

	blobmsg_parse(mib_format, __MIB_MAX, tb, blob_data(msg), blob_len(msg));

	cur = tb[MIB_NAME];
	if (!cur) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	const char *name = blobmsg_get_string(cur);
	
	cur = tb[MIB_VALUE];
	if (!cur) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	const char *value = blobmsg_get_string(cur);

	strncpy(value_str, value, sizeof(value_str));
	mib_init();
	
	ret = mib_set_s(name, value_str, strlen(value_str));
	
	mib_commit();
	mib_deinit();

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", ret);
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static const struct blobmsg_policy mib_get_format[MIB_VALUE] = {
	[MIB_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING},
};

static int
rpc_mib_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[MIB_VALUE];
	struct blob_attr *cur;
	int ret = 0;
	char value[2048]={0};

	blobmsg_parse(mib_get_format, MIB_VALUE, tb, blob_data(msg), blob_len(msg));

	cur = tb[MIB_NAME];
	if (!cur) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	const char *name = blobmsg_get_string(cur);
	mib_init();
	
	ret = mib_get_s(name, value, sizeof(value));
	
	mib_deinit();

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", ret);
	blobmsg_add_string(&b, "value", value);
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

#ifdef CONFIG_GPON_FEATURE
char * gpon_settings[] = {
	"WAN_PHY_PORT",
	"PON_MODE",
	"PON_SPEED",
	"GPON_SN",
	"GPON_PLOAM_PASSWD",
	"OMCI_SW_VER1",
	"OMCI_SW_VER2",
	"OMCI_OLT_MODE",
	"OMCC_VER",
	"OMCI_TM_OPT",
	"PON_VENDOR_ID",
	"HW_CWMP_PRODUCTCLASS",
	"HW_HWVER",
	"LOID",
	"LOID_PASSWD",
	NULL
};

static int
rpc_get_gpon_settings(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	char value[2048]={0};
	void *o;
	int i = 0;

	mib_init();

	blob_buf_init(&b, 0);
	o = blobmsg_open_table(&b, "mib");
	while(gpon_settings[i]!=NULL){
		memset(value, 0, sizeof(value));
		mib_get_s(gpon_settings[i], value, sizeof(value));
		blobmsg_add_string(&b, gpon_settings[i], value);
		i++;
	}
	blobmsg_close_table(&b, o);

	mib_deinit();

	//send ubus result
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static int
rpc_set_gpon_settings(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__MIB_MAX];
	struct blob_attr *cur;
	char value_str[2048]={0};
	int ret = 0, found=0, i=0;

	blobmsg_parse(mib_format, __MIB_MAX, tb, blob_data(msg), blob_len(msg));

	cur = tb[MIB_NAME];
	if (!cur) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	const char *name = blobmsg_get_string(cur);

	while(gpon_settings[i]!=NULL){
		if(!strcmp(gpon_settings[i], name)){
			found = 1;
			break;
		}
		i++;
	}

	if(!found){
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	
	cur = tb[MIB_VALUE];
	if (!cur) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	const char *value = blobmsg_get_string(cur);

	strncpy(value_str, value, sizeof(value_str));
	mib_init();
	
	ret = mib_set_s(name, value_str, sizeof(value_str));
	
	mib_commit();
	mib_deinit();

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", ret);
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static int
rpc_apply_gpon_settings(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	mib_init();

	rtk_pon_reinit();

	mib_deinit();

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", 0);
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static int
rpc_get_pon_status(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	TransceiverInfo_T info;
	GponStateInfo_T gpon_state;

	memset(&info, 0, sizeof(TransceiverInfo_T));
	rtk_get_transceiver_status(&info, TRANSCEIVER_TYPE_MAX);
	memset(&gpon_state, 0, sizeof(GponStateInfo_T));
	rtk_get_gpon_state(&gpon_state);

	mib_init();

	if(strlen(info.vender_name)==0){
		mib_get_s2(MIB_BOSA_VENDOR_NAME, info.vender_name, sizeof(info.vender_name));
	}

	if(strlen(info.part_number)==0){
		mib_get_s2(MIB_BOSA_PART_NUMBER, info.part_number, sizeof(info.part_number));
	}
	
	mib_deinit();
	blob_buf_init(&b, 0);
	
	blobmsg_add_string(&b, "vender_name", info.vender_name);
	blobmsg_add_string(&b, "part_number", info.part_number);
	blobmsg_add_string(&b, "temperature", info.temperature);
	blobmsg_add_string(&b, "voltage", info.voltage);
	blobmsg_add_string(&b, "tx_power", info.tx_power);
	blobmsg_add_string(&b, "rx_power", info.rx_power);
	blobmsg_add_string(&b, "bias_current", info.bias_current);

	//gpon parameter
	blobmsg_add_string(&b, "onu_state", gpon_state.onu_state);
	blobmsg_add_string(&b, "onu_id", gpon_state.onu_id);
	blobmsg_add_string(&b, "loid_status", gpon_state.loid_status);

	//send ubus result
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static int
rpc_get_pon_statistics(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	rt_stat_port_cntr_t counters;
	unsigned int pon_port_idx = 1;
	unsigned int pon_mode=0;
	char buf[256]={0};
	
	mib_init();

	if(mib_get_s2(MIB_PON_MODE, buf, sizeof(buf)))
		pon_mode = (unsigned int)atoi(buf);
	
	mib_deinit();

	if(rt_switch_phyPortId_get(LOG_PORT_PON, &pon_port_idx) != 0)
		printf("rt_switch_phyPortId_get failed: %s %d\n", __func__, __LINE__);
	
	if(rt_stat_port_getAll(pon_port_idx, &counters) != RT_ERR_OK)
		printf("rt_stat_port_getAll failed: %s %d\n", __func__, __LINE__);
	
	blob_buf_init(&b, 0);

	blobmsg_add_u64(&b, "bytes_sent", counters.ifOutOctets);
	blobmsg_add_u64(&b, "bytes_received", counters.ifInOctets);
	blobmsg_add_u32(&b, "packets_sent", counters.ifOutUcastPkts + counters.ifOutMulticastPkts+ counters.ifOutBrocastPkts);
	blobmsg_add_u32(&b, "packets_received", counters.ifInUcastPkts + counters.ifInMulticastPkts+ counters.ifInBroadcastPkts);
	blobmsg_add_u32(&b, "unicast_packets_sent", counters.ifOutUcastPkts);
	blobmsg_add_u32(&b, "unicast_packets_received", counters.ifInUcastPkts);
	blobmsg_add_u32(&b, "multicast_packets_sent", counters.ifOutMulticastPkts);
	blobmsg_add_u32(&b, "multicast_packets_received", counters.ifInMulticastPkts);
	blobmsg_add_u32(&b, "broadcast_packets_sent", counters.ifOutBrocastPkts);
	blobmsg_add_u32(&b, "broadcast_packets_received", counters.ifInBroadcastPkts);
	blobmsg_add_u32(&b, "packets_dropped", counters.ifOutDiscards);
	blobmsg_add_u32(&b, "packets_dropped_received", counters.ifInDiscards);
	blobmsg_add_u32(&b, "pause_packets_sent", counters.dot3OutPauseFrames);
	blobmsg_add_u32(&b, "pause_packets_received", counters.dot3InPauseFrames);
	blobmsg_add_u32(&b, "errors_packets_sent", counters.etherStatsTxCRCAlignErrors);
	blobmsg_add_u32(&b, "errors_packets_received", counters.dot3StatsSymbolErrors + counters.dot3ControlInUnknownOpcodes);
	if(pon_mode == GPON_MODE){
		rt_gpon_pm_counter_t counter;
		memset(&counter, 0, sizeof(counter));
		if(rt_gpon_pmCounter_get(RT_GPON_PM_TYPE_FEC, &counter) != RT_ERR_OK)
			printf("rtk_gpon_globalCounter_get failed: %s %d\n", __func__, __LINE__);
		blobmsg_add_u32(&b, "fec_errors", counter.fec.uncorrected_fec_codewords);

		memset(&counter, 0, sizeof(counter));
		if(rt_gpon_pmCounter_get(RT_GPON_PM_TYPE_XGEM, &counter) != RT_ERR_OK)
				printf("rtk_gpon_globalCounter_get failed: %s %d\n", __func__, __LINE__);
		blobmsg_add_u32(&b, "hec_errors", counter.xgem.received_xgem_header_hec_errors);
	}
#ifdef CONFIG_EPON_FEATURE
	else if(pon_mode == EPON_MODE){
		rt_epon_counter_t counter;
		memset(&counter, 0, sizeof(counter));
		if(rt_epon_mibCounter_get(&counter) != RT_ERR_OK)
			printf("rt_epon_mibCounter_get failed: %s %d\n", __func__, __LINE__);
		blobmsg_add_u32(&b, "fec_errors", counter.fecUncorrectedBlocks);
		blobmsg_add_u32(&b, "hec_errors", 0);
	}
#endif
	//send ubus result
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static int
rpc_reset_pon_statistics(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	unsigned int pon_port_idx = 1;

	if(rt_switch_phyPortId_get(LOG_PORT_PON, &pon_port_idx) != 0)
		printf("rt_switch_phyPortId_get failed: %s %d\n", __func__, __LINE__);

	rt_stat_port_reset(pon_port_idx);
	
	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", 0);
	ubus_send_reply(ctx, req, b.head);
	
	return UBUS_STATUS_OK;
}

static int
rpc_get_ponmisc_transceiver(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[MIB_VALUE];
	struct blob_attr *cur;
	int ret = 0;
	char value[2048] = {0};
	TransceiverInfo_T TransceiverInfo;
	memset(&TransceiverInfo, 0, sizeof(TransceiverInfo_T));

	blobmsg_parse(mib_get_format, MIB_VALUE, tb, blob_data(msg), blob_len(msg));

	cur = tb[MIB_NAME];
	if (!cur) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	const char *name = blobmsg_get_string(cur);
	//printf("[%s:%d] name: %s\n", __FUNCTION__, __LINE__, name);

	if (strcmp(name, "vendor-name") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_VENDOR_NAME);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.vender_name);
	}
	else if (strcmp(name, "part-number") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_VENDOR_PART_NUM);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.part_number);
	}
	else if (strcmp(name, "temperature") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_TEMPERATURE);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.temperature);
	}
	else if (strcmp(name, "voltage") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_VOLTAGE);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.voltage);
	}
	else if (strcmp(name, "bias-current") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_BIAS_CURRENT);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.bias_current);
	}
	else if (strcmp(name, "tx-power") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_TX_POWER);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.tx_power);
	}
	else if (strcmp(name, "rx-power") == 0) {
		ret = rtk_get_transceiver_status(&TransceiverInfo, TRANSCEIVER_TYPE_RX_POWER);
		snprintf(value, sizeof(value) ,"%s", TransceiverInfo.rx_power);
	}

	//printf("[%s:%d] result: %d, value: %s\n", __FUNCTION__, __LINE__, ret, value);

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", ret);
	blobmsg_add_string(&b, "value", value);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}
#endif

static int
rpc_flush_conntrack(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	system("echo flush > /proc/net/nf_conntrack");
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	system("echo 1 > /proc/fc/ctrl/flow_flush");
	system("echo 1 > /proc/fc/ctrl/rstconntrack");
#endif

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", 0);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

#ifdef CONFIG_USER_LANNETINFO
int flock_set(int fd, int type)
{
        struct flock lock;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_type = type;
        lock.l_pid = getpid();

        if((fcntl(fd, F_SETLKW, &lock)) < 0)
        {
                printf("Lock failed:type = %d\n", lock.l_type);
                return 1;
        }
        return 0;
}

int get_lan_net_info(lanHostInfo_t **ppLANNetInfoData, unsigned int *pCount)
{
	struct stat status;
	int fd, i, read_size, pid=0;
	lanHostInfo_t *pLANNetInfoData = NULL;

	if( (ppLANNetInfoData==NULL)||(pCount==NULL) )
		return -1;

	pid = read_pid(LANNETINFO_RUNFILE);
	if( pid>0 )
		kill(pid, SIGUSR2);

	usleep(60000);//30ms

	if ( stat(LANNETINFOFILE, &status) < 0 )
		return -3;

	fd = open(LANNETINFOFILE, O_RDONLY);
	if(fd < 0)
		return -3;

	read_size = (unsigned long)(status.st_size);
	*pCount = read_size / sizeof(lanHostInfo_t);

	pLANNetInfoData = (lanHostInfo_t *) malloc(read_size);

	if(pLANNetInfoData == NULL)\
		return -3;

	*ppLANNetInfoData = pLANNetInfoData;
	memset(pLANNetInfoData, 0, read_size);

	if(!flock_set(fd, F_RDLCK))
	{
		read_size = read(fd, (void*)pLANNetInfoData, read_size);
		flock_set(fd, F_UNLCK);
	}

	close(fd);
	return 0;
}

static int
rpc_get_hosts(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg)
{
	void *o=NULL, *a=NULL, *o_tmp=NULL, *a_tmp=NULL;
	lanHostInfo_t *pLanNetInfo=NULL;
	struct in_addr lanIP;
	char ipv6AddrStr[INET6_ADDRSTRLEN];
	unsigned int count=0;
	int idx;
	unsigned char buff[64];

        //send ubus result
        blob_buf_init(&b, 0);
	a = blobmsg_open_array(&b, "hosts");
	if(get_lan_net_info(&pLanNetInfo, &count) == 0)
	{
		for(idx=0; idx<count; idx++)
		{
			o = blobmsg_open_table(&b, "hosts");

			snprintf(buff, sizeof(buff), "%02x:%02x:%02x:%02x:%02x:%02x",
				pLanNetInfo[idx].mac[0], pLanNetInfo[idx].mac[1], pLanNetInfo[idx].mac[2],
				pLanNetInfo[idx].mac[3], pLanNetInfo[idx].mac[4], pLanNetInfo[idx].mac[5]);
			blobmsg_add_string(&b, "macaddr", buff);
			//blobmsg_add_string(&b, "address_source", "TODO");
			//blobmsg_add_u64(&b, "lease_time_remaining", 999999);
			char ifname[16];
			if(pLanNetInfo[idx].port >= Port_LAN1 && pLanNetInfo[idx].port <= Port_LAN10){
				snprintf(ifname, sizeof(ifname)-1, "eth0.%d", pLanNetInfo[idx].port+1);
				blobmsg_add_string(&b, "device", ifname);
			}else{
				blobmsg_add_string(&b, "device", "wlanX");
			}
			blobmsg_add_string(&b, "network", "lan");
			blobmsg_add_string(&b, "interface_type", connectionTypeString[pLanNetInfo[idx].connectionType]);
			//blobmsg_add_string(&b, "hostname", "TODO");
			lanIP.s_addr = pLanNetInfo[idx].ip;
			unsigned char active=strcmp(inet_ntoa(lanIP), "0.0.0.0");
			blobmsg_add_u8(&b, "active", (active==0)?0:1);
			blobmsg_add_u32(&b, "active_connections", (active==0)?0:1);
			blobmsg_add_string(&b, "ipaddr", inet_ntoa(lanIP));
			// ipv4 address array
			a_tmp = blobmsg_open_array(&b, "ipv4addr");
			blobmsg_add_string(&b, "ipaddr", inet_ntoa(lanIP));
			blobmsg_close_array(&b, a_tmp);

			//blobmsg_add_string(&b, "ipv6addr", pLanNetInfo[idx].ipv6Addr);
			if(inet_ntop(AF_INET6, pLanNetInfo[idx].ipv6Addr, ipv6AddrStr, INET6_ADDRSTRLEN) != NULL){
				// ipv6 address array
				a_tmp = blobmsg_open_array(&b, "ipv6addr");
				blobmsg_add_string(&b, "ipv6addr", ipv6AddrStr);
				blobmsg_close_array(&b, a_tmp);
			}

			// stats json object
			o_tmp = blobmsg_open_table(&b, "stats");
			blobmsg_add_u64(&b, "rx_bytes", pLanNetInfo[idx].rxBytes);
			blobmsg_add_u64(&b, "tx_bytes", pLanNetInfo[idx].txBytes);
			blobmsg_add_u32(&b, "rx_packets", pLanNetInfo[idx].rx_packets);
			blobmsg_add_u32(&b, "tx_packets", pLanNetInfo[idx].tx_packets);
			blobmsg_add_u32(&b, "rx_errors", pLanNetInfo[idx].rx_errors);
			blobmsg_add_u32(&b, "rx_dropped", pLanNetInfo[idx].rx_dropped);
			blobmsg_close_table(&b, o_tmp);
			blobmsg_close_table(&b, o);
		}
	}

	blobmsg_close_array(&b, a);
        ubus_send_reply(ctx, req, b.head);

        return UBUS_STATUS_OK;
}
#endif

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
int intr_socket_recv(int fd, intrBcasterMsg_t *pMsgData)
{
    int recvLen;
    int nlmsgLen;
    struct nlmsghdr     *pNlh = NULL;
    struct iovec                iov;
    struct sockaddr_nl  da;
    struct msghdr               msg;

    if (!pMsgData){
		printf("[%s] pMsgData not valid\n", __FUNCTION__);
        return -1;
    }

    // allocate memory
    nlmsgLen = NLMSG_SPACE(MAX_BYTE_PER_INTR_BCASTER_MSG);
    pNlh = (struct nlmsghdr *)malloc(nlmsgLen);
    if (!pNlh){
		printf("[%s] get pNlh fail\n", __FUNCTION__);
        return -1;
    }

    // fill struct nlmsghdr
    memset(pNlh, 0, nlmsgLen);

    // fill struct iovec
    memset(&iov, 0, sizeof(struct iovec));
    iov.iov_base = (void *)pNlh;
    iov.iov_len = nlmsgLen;

    // fill struct sockaddr_nl
    memset(&da, 0, sizeof(struct sockaddr_nl));
    da.nl_family = AF_NETLINK;

    // fill struct msghdr
    memset(&msg, 0x0, sizeof(struct msghdr));
    msg.msg_name = (void *)&da;
    msg.msg_namelen = sizeof(da);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // receive message
    recvLen = recvmsg(fd, &msg, 0);
    if (!NLMSG_OK(pNlh, recvLen)){
		printf("[%s]receive message fail\n", __FUNCTION__);
		free(pNlh);
        return -1;
    }

    memcpy(pMsgData, NLMSG_DATA(pNlh), MAX_BYTE_PER_INTR_BCASTER_MSG);
    free(pNlh);

    return 0;
}

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
static int onu_link_state = 0;
#endif

void recv_bcaster_event(struct uloop_fd *u, unsigned int events) 
{
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	int pre_onu_link_state = 0;
#endif
	intrBcasterMsg_t msgData;
	int fd = u->fd;

	if (!intr_socket_recv(fd, &msgData))
	{
		if (MSG_TYPE_NIC_EVENT == msgData.intrType)
		{
			if (msgData.intrSubType == NIC_EVENT_TYPE_NO_MEM)
			{
				printf("[NIC] no memory.\n");
				va_cmd("/bin/sh", 3, 1, "-c", "/etc/scripts/clean_memory.sh", "force2");
			}
			else if (msgData.intrSubType == NIC_EVENT_TYPE_MEM_RECOVERD)
				printf("[NIC] memory recovered.\n");
		}
#if defined(CONFIG_GPON_FEATURE)
		else if (MSG_TYPE_ONU_STATE == msgData.intrType)
		{
			pre_onu_link_state = onu_link_state;
			onu_link_state = msgData.intrSubType;
			printf("[RTM_OMCI] Line change from O%d to O%d !\n", pre_onu_link_state, onu_link_state);
		}
#endif
#if defined(CONFIG_EPON_FEATURE)
		if (MSG_TYPE_EPON_STATE == msgData.intrType)
		{
			pre_onu_link_state = onu_link_state;
			onu_link_state = msgData.intrSubType;
			printf("[EPON_OAM] Line change from O%d to O%d !\n", pre_onu_link_state, onu_link_state);
		}
#endif
	}

}

struct uloop_fd bcast_event_uloop;
int init_bcaster_event(void)
{
	int fd = -1;
	struct sockaddr_nl sa;

	if (-1 == (fd = socket(PF_NETLINK, SOCK_RAW, INTR_BCASTER_NETLINK_TYPE)))
	{
		printf("Create Interrupt Socket Fail!\n");
		return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups =	(1<<MSG_TYPE_OMCI_EVENT) | (1<<MSG_TYPE_ONU_STATE) | (1<<MSG_TYPE_EPON_STATE) 
					| (1<<MSG_TYPE_EPON_EVENT) | (1<<MSG_TYPE_PPPOE_EVENT) | (1<<MSG_TYPE_NIC_EVENT);
	if (-1 == bind(fd, (struct sockaddr*)&sa, sizeof(struct sockaddr)))
	{
		perror ("Bind Interrupt Socket Fail!\n");
		close(fd);
		return -1;
	}
	
	memset(&bcast_event_uloop, 0, sizeof(bcast_event_uloop));
	bcast_event_uloop.cb = recv_bcaster_event;
	bcast_event_uloop.fd = fd;
 	uloop_fd_add(&bcast_event_uloop, ULOOP_READ);
	return fd;
}
#endif

static int
rpc_get_board_info(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg)
{
        blob_buf_init(&b, 0);
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_COMMON_RT_PONMISC)
	void *a_temp = blobmsg_open_array(&b, "temperature");
	void *o_temp = NULL;

	// SoC Module Temperature
	o_temp = blobmsg_open_table(&b, "SoC");
	int TempIntger = 0;
	int TempDecimal = 0;
	rtk_switch_thermal_get(&TempIntger, &TempDecimal);
	blobmsg_add_string(&b, "description", "SoC");
	blobmsg_add_u32(&b, "temperature", TempIntger);
	blobmsg_close_table(&b, o_temp);

	// Pon Module Temperature
	o_temp = blobmsg_open_table(&b, "PonModule");
	rt_transceiver_data_t transceiver, readableCfg;
	rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_TEMPERATURE,&transceiver);
	_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
	blobmsg_add_string(&b, "description", "PonModule");
	blobmsg_add_string(&b, "temperature", readableCfg.buf);
	blobmsg_close_table(&b, o_temp);

	blobmsg_close_array(&b, a_temp);
#endif
        ubus_send_reply(ctx, req, b.head);
        return UBUS_STATUS_OK;
}

enum {
	IS_IGMPSNOOP_ENABLED,
	IS_MLDSNOOP_ENABLED,
	__SNOOP_MAX
};

static const struct blobmsg_policy igmpsnoop_format[] = {
	[IS_IGMPSNOOP_ENABLED] = { "is_igmpsnoop_enabled", BLOBMSG_TYPE_BOOL},
	[IS_MLDSNOOP_ENABLED] = { "is_mldsnoop_enabled", BLOBMSG_TYPE_BOOL},
};

static int
rpc_set_igmpsnoop(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__SNOOP_MAX];
	int igmpsnoop_en = 0;
	int mldsnoop_en = 0;

	blobmsg_parse(igmpsnoop_format, __SNOOP_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));

	if (tb[IS_IGMPSNOOP_ENABLED])
		igmpsnoop_en = blobmsg_get_bool(tb[IS_IGMPSNOOP_ENABLED]);

	if (tb[IS_MLDSNOOP_ENABLED])
		mldsnoop_en = blobmsg_get_bool(tb[IS_MLDSNOOP_ENABLED]);

//do something here
	int j, SW_PORT_NUM = CONFIG_LAN_PORT_NUM;;
	char ifName[MAX_LAN_NUM];
	char buf[256]={0};

	for(j = 0; j < SW_PORT_NUM; j++)
	{
		sprintf(ifName, "%s", LAN_PORT_IFNAE[j]);

		sprintf(buf, "echo DEVIFNAME %s SNOOPING_DISABLE %d > proc/igmp/ctrl/igmpConfig", ifName, !igmpsnoop_en);
		va_cmd(BIN_SH, 2, 1, "-c", buf);

		sprintf(buf, "echo DEVIFNAME %s SNOOPING_DISABLE %d > proc/igmp/ctrl/igmp6Config", ifName, !mldsnoop_en);
		va_cmd(BIN_SH, 2, 1, "-c", buf);
	}

	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", 0);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

enum {
	RATELIMIT_INDEX,
	RATELIMIT_RATE,
	__RATELIMIT_MAX
};


static const struct blobmsg_policy ratelimit_format[] = {
	[RATELIMIT_INDEX] = { "ratelimit_index", BLOBMSG_TYPE_STRING},
	[RATELIMIT_RATE] = { "ratelimit_rate", BLOBMSG_TYPE_STRING},
};

static int
rpc_add_ratelimit_rule(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__RATELIMIT_MAX];
	int ratelimit_idx = 0, ratelimit_rate = 0, meter_idx=-1;
	char value_str[2048]={0};
	blobmsg_parse(ratelimit_format, __SNOOP_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));
	if (tb[RATELIMIT_INDEX]) {
		const char *value1 = blobmsg_get_string(tb[RATELIMIT_INDEX]);
		ratelimit_idx = atoi(value1);
	}
	if (tb[RATELIMIT_RATE]){
		const char *value2 = blobmsg_get_string(tb[RATELIMIT_RATE]);
		ratelimit_rate=atoi(value2);
	}
//do something here
#ifdef CONFIG_COMMON_RT_API
	meter_idx = rtk_qos_share_meter_get(RTK_APP_TYPE_IP_QOS_RATE_SHAPING, ratelimit_idx, ratelimit_rate);
	if (meter_idx < 0)
		meter_idx = rtk_qos_share_meter_set(RTK_APP_TYPE_IP_QOS_RATE_SHAPING, ratelimit_idx, ratelimit_rate);
	if(meter_idx == -1)
	{
		printf("%s %d: rtk_qos_share_meter_set FAIL !\n", __func__, __LINE__);
	}
#endif
	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", meter_idx);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

static int
rpc_del_ratelimit_rule(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__RATELIMIT_MAX];
	int ratelimit_idx = 0, ratelimit_rate=0, meter_idx=-1;
	char value_str[2048]={0};
	blobmsg_parse(ratelimit_format, __SNOOP_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));
	if (tb[RATELIMIT_INDEX]) {
		const char *value1 = blobmsg_get_string(tb[RATELIMIT_INDEX]);
		ratelimit_idx = atoi(value1);
	}
	if (tb[RATELIMIT_RATE]){
		const char *value2 = blobmsg_get_string(tb[RATELIMIT_RATE]);
		ratelimit_rate=atoi(value2);
	}
//do something here
#ifdef CONFIG_COMMON_RT_API
	meter_idx = rtk_qos_share_meter_get(RTK_APP_TYPE_IP_QOS_RATE_SHAPING, ratelimit_idx, ratelimit_rate);
	if (meter_idx >= 0){
		rtk_qos_share_meter_delete(RTK_APP_TYPE_IP_QOS_RATE_SHAPING, ratelimit_idx);
	}
	else
		printf("%s %d: can't find the meter index:%d !\n", __func__, __LINE__, ratelimit_idx);
#endif
	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", 0);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

static int
rpc_flush_ratelimit_rule(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	rtk_qos_share_meter_flush(RTK_APP_TYPE_IP_QOS_RATE_SHAPING);
	//send ubus result
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "result", 0);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

enum {
        DHCPv6_INTERFACE,
        AFTR,
        __AFTR_MAX
};

static const struct blobmsg_policy update_aftr_format[] = {
        [DHCPv6_INTERFACE] = { "interface", BLOBMSG_TYPE_STRING},
        [AFTR] = { "aftr", BLOBMSG_TYPE_STRING},
};

static int
rpc_update_aftr(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg)
{
        struct blob_attr *tb[__AFTR_MAX];
        char *interface = NULL;
	char *aftr = NULL;
	unsigned int uci_change=0;

        blobmsg_parse(update_aftr_format, __AFTR_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));

        if (tb[DHCPv6_INTERFACE])
                interface = blobmsg_get_string(tb[DHCPv6_INTERFACE]);

        if (tb[AFTR])
		aftr = blobmsg_get_string(tb[AFTR]);

	struct uci_context *uci_ctx = uci_alloc_context();
	struct uci_package *network_package = NULL;
	if(UCI_OK == uci_load(uci_ctx, "network", &network_package)){
		struct uci_element *e=NULL;
		uci_foreach_element(&network_package->sections, e){
			struct uci_section *s=uci_to_section(e);
			if(!s) continue;
			struct uci_option *proto_o=uci_lookup_option(uci_ctx, s, "proto");
			if(proto_o && !strncmp(proto_o->v.string, "dslite", strlen(proto_o->v.string)+1)){
				struct uci_option *tunlink_o=uci_lookup_option(uci_ctx, s, "tunlink");
				if(tunlink_o && !strncmp(tunlink_o->v.string, interface, strlen(tunlink_o->v.string)+1)){
					struct uci_ptr ptr;
					char option_name[]="peeraddr";
					memset(&ptr, 0, sizeof(ptr));
					ptr.p = network_package;
					ptr.s = s;
					ptr.option = option_name;
					ptr.value = aftr;
					int ret=uci_set(uci_ctx, &ptr);
					uci_change++;
				}
			}
		}
	}

	if(uci_change) uci_commit(uci_ctx, &network_package, false);

        //send ubus result
        blob_buf_init(&b, 0);
        blobmsg_add_u32(&b, "result", 0);
        ubus_send_reply(ctx, req, b.head);

        return UBUS_STATUS_OK;
}

enum {
	DO_WLAN_HAPD_WLAN_INF,
	DO_WLAN_HAPD_HEXDUMP,
	DO_WLAN_HAPD_STA_MAC,
	__DO_WLAN_HAPD_MAX
};

static const struct blobmsg_policy do_wlan_hapd_format[__DO_WLAN_HAPD_MAX] = {
	[DO_WLAN_HAPD_WLAN_INF] = { .name = "wlan_inf", .type = BLOBMSG_TYPE_STRING},
	[DO_WLAN_HAPD_HEXDUMP] = { .name = "hexdump", .type = BLOBMSG_TYPE_STRING},
	[DO_WLAN_HAPD_STA_MAC] = { .name = "sta_mac", .type = BLOBMSG_TYPE_STRING},
};

static int
rpc_do_wlan_channel_change_event(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_HAPD_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_hapd_format, __DO_WLAN_HAPD_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_ap_sta_connect_event(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_HAPD_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_hapd_format, __DO_WLAN_HAPD_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_ap_sta_disconnect_event(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_HAPD_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_hapd_format, __DO_WLAN_HAPD_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_ap_sta_psk_mismatch_event(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_HAPD_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_hapd_format, __DO_WLAN_HAPD_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_hostapd_wps_new_settings(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_HAPD_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_hapd_format, __DO_WLAN_HAPD_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_hostapd_wps_success(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_HAPD_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_hapd_format, __DO_WLAN_HAPD_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

enum {
	DO_WLAN_WPAS_WLAN_INF,
	DO_WLAN_WPAS_MSG,
	DO_WLAN_WPAS_HEXDUMP,
	DO_WLAN_WPAS_STA_MAC,
	__DO_WLAN_WPAS_MAX
};

static const struct blobmsg_policy do_wlan_wpas_format[__DO_WLAN_WPAS_MAX] = {
	[DO_WLAN_WPAS_WLAN_INF] = { .name = "wlan_inf", .type = BLOBMSG_TYPE_STRING},
	[DO_WLAN_WPAS_MSG] = { .name = "msg", .type = BLOBMSG_TYPE_STRING},
	[DO_WLAN_WPAS_HEXDUMP] = { .name = "hexdump", .type = BLOBMSG_TYPE_STRING},
	[DO_WLAN_WPAS_STA_MAC] = { .name = "sta_mac", .type = BLOBMSG_TYPE_STRING},
};

static int
rpc_do_wlan_sta_connect_event(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_WPAS_MAX];
	struct blob_attr *cur;

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_wpas_format, __DO_WLAN_WPAS_MAX, tb, blob_data(msg), blob_len(msg));

	cur = tb[DO_WLAN_WPAS_WLAN_INF];
	if (!cur)
		return UBUS_STATUS_INVALID_ARGUMENT;
	const char *wlan_inf = blobmsg_get_string(cur);

	RTKRPC_DEBUG(LOG_DEBUG, "wlan_inf:%s\n", wlan_inf);

#if defined(ARP_LOOP_DETECTION)
	if(strstr(wlan_inf, ALIASNAME_VXD)){
		RTKRPC_DEBUG(LOG_DEBUG, "call link_change_notify_to_map_checker\n");
		link_change_notify_to_map_checker((char *)wlan_inf);
	}
#endif

	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_wpas_wps_success(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_WPAS_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_wpas_format, __DO_WLAN_WPAS_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

static int
rpc_do_wlan_store_wpas_wps_status(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__DO_WLAN_WPAS_MAX];

	RTKRPC_DEBUG(LOG_DEBUG, "enter...\n");

	blobmsg_parse(do_wlan_wpas_format, __DO_WLAN_WPAS_MAX, tb, blob_data(msg), blob_len(msg));
	//TODO
	return UBUS_STATUS_OK;
}

#if 0

static int
rpc_rtk_api_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	static struct ubus_method main_object_methods[] = {
		UBUS_METHOD("mib_set", rpc_mib_set, mib_format),
		UBUS_METHOD("mib_get", rpc_mib_get, mib_get_format),
		UBUS_METHOD_NOARG("get_gpon_settings", rpc_get_gpon_settings),
		UBUS_METHOD("set_gpon_settings", rpc_set_gpon_settings, mib_format),
		UBUS_METHOD_NOARG("apply_gpon_settings", rpc_apply_gpon_settings),
		UBUS_METHOD_NOARG("get_pon_status", rpc_get_gpon_status),
		UBUS_METHOD_NOARG("get_pon_statistics", rpc_get_gpon_statistics),
	};
	
	static struct ubus_object_type main_object_type =
		UBUS_OBJECT_TYPE("rpc", main_object_methods);
	
	static struct ubus_object main_object = {
		.name = "rtk-rpc",
		.type = &main_object_type,
		.methods = main_object_methods,
		.n_methods = ARRAY_SIZE(main_object_methods),
	};

	return ubus_add_object(ctx, &main_object);
}

struct rpc_plugin rpc_plugin = {
	.init = rpc_rtk_api_init
};

#else

static void server_main(void)
{
	fprintf(stderr, "server_main Start\n");
	int ret;

	static struct ubus_method main_object_methods[] = {
		UBUS_METHOD("mib_set", rpc_mib_set, mib_format),
		UBUS_METHOD("mib_get", rpc_mib_get, mib_get_format),
#ifdef CONFIG_GPON_FEATURE
		UBUS_METHOD_NOARG("get_gpon_settings", rpc_get_gpon_settings),
		UBUS_METHOD("set_gpon_settings", rpc_set_gpon_settings, mib_format),
		UBUS_METHOD_NOARG("apply_gpon_settings", rpc_apply_gpon_settings),
		UBUS_METHOD_NOARG("get_pon_status", rpc_get_pon_status),
		UBUS_METHOD_NOARG("get_pon_statistics", rpc_get_pon_statistics),
		UBUS_METHOD_NOARG("reset_pon_statistics", rpc_reset_pon_statistics),
		UBUS_METHOD("get_ponmisc_transceiver", rpc_get_ponmisc_transceiver, mib_get_format),
#endif
		UBUS_METHOD_NOARG("flush_conntrack", rpc_flush_conntrack),
#ifdef CONFIG_USER_LANNETINFO
		UBUS_METHOD_NOARG("hosts", rpc_get_hosts),
#endif
		UBUS_METHOD_NOARG("board_info", rpc_get_board_info),
		UBUS_METHOD("set_igmpsnoop", rpc_set_igmpsnoop, igmpsnoop_format),
		UBUS_METHOD("add_ratelimit_rule", rpc_add_ratelimit_rule, ratelimit_format),
		UBUS_METHOD("del_ratelimit_rule", rpc_del_ratelimit_rule, ratelimit_format),
		UBUS_METHOD_NOARG("flush_ratelimit_rule", rpc_flush_ratelimit_rule),
		UBUS_METHOD("update_aftr", rpc_update_aftr, update_aftr_format),

		UBUS_METHOD("do_wlan_channel_change_event", rpc_do_wlan_channel_change_event, do_wlan_hapd_format),
		UBUS_METHOD("do_wlan_ap_sta_connect_event", rpc_do_wlan_ap_sta_connect_event, do_wlan_hapd_format),
		UBUS_METHOD("do_wlan_ap_sta_disconnect_event", rpc_do_wlan_ap_sta_disconnect_event, do_wlan_hapd_format),
		UBUS_METHOD("do_wlan_ap_sta_psk_mismatch_event", rpc_do_wlan_ap_sta_psk_mismatch_event, do_wlan_hapd_format),
		UBUS_METHOD("do_wlan_hostapd_wps_new_settings", rpc_do_wlan_hostapd_wps_new_settings, do_wlan_hapd_format),
		UBUS_METHOD("do_wlan_hostapd_wps_success", rpc_do_wlan_hostapd_wps_success, do_wlan_hapd_format),


		UBUS_METHOD("do_wlan_sta_connect_event", rpc_do_wlan_sta_connect_event, do_wlan_wpas_format),
		UBUS_METHOD("do_wlan_wpas_wps_success", rpc_do_wlan_wpas_wps_success, do_wlan_wpas_format),
		UBUS_METHOD("do_wlan_store_wpas_wps_status", rpc_do_wlan_store_wpas_wps_status, do_wlan_wpas_format),
	};
	
	static struct ubus_object_type main_object_type =
		UBUS_OBJECT_TYPE("rpc", main_object_methods);
	
	static struct ubus_object main_object = {
		.name = "rtk-rpc",
		.type = &main_object_type,
		.methods = main_object_methods,
		.n_methods = ARRAY_SIZE(main_object_methods),
	};
	
	ret = ubus_add_object(ctx, &main_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));

	uloop_run();
	fprintf(stderr, "server_main End\n");
}

int main(int argc, char **argv)
{
	const char *ubus_socket = NULL;

	uloop_init();
	
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	init_bcaster_event();
#endif

#ifdef CONFIG_RTK_RPC_DYNAMIC_DEBUG
	init_cmd_fifo();
#endif

#ifdef CONFIG_RTK_NETLINK_ROUTE_API
	init_netlink_route_event();
#endif

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	server_main();

	ubus_free(ctx);
	uloop_done();

	return 0;
}
#endif
