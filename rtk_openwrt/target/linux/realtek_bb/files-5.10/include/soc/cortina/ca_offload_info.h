#ifndef __CA_OFFLOAD_INFO_H__
#define __CA_OFFLOAD_INFO_H__

#include <linux/in.h>
#include <linux/in6.h>
#include <soc/cortina/ca_packet_info.h>
#include <linux/skbuff.h>

#define CA_MAGIC	0x0D06F00D	/* dog food */

typedef struct {
	unsigned int magic; /* set to CA_MAGIC if ca_offload_info_t is filled by Cortina NE driver */

	ca_packet_info_t pkt_info; /* store ingress (original) packet contents */

	/* Policy-based routing includes source routing.
	 * egress_ifindex is valid when policy_route is true.
	 * ipv4_nh is valid when policy_route and pkt_info.is_ipv4 are both true.
	 * ipv6_nh is valid when policy_route and pkt_info.is_ipv6 are both true.
	 */
	bool policy_route;
	int egress_ifindex;
	union {
		struct in_addr ipv4_nh;
		struct in6_addr ipv6_nh;
	};

	bool sw_only;
	unsigned long lifetime; /* jiffies */

#if defined(CONFIG_CORTINA_KERNEL_HOOK)
	struct nf_conn *ct;
	bool ct_valid;
#endif
} ca_offload_info_t;

#endif /* __CA_OFFLOAD_INFO_H__ */
