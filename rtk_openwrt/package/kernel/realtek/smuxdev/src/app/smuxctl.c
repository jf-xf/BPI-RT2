/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <net/if.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <asm/byteorder.h>
#ifdef CONFIG_DEV_xDSL
#include <rtk/linux/rtk_smux.h>
#else
#include "rtk_smux.h"
#endif

#define RUN_TESTS 0
extern int run_test_cases(void);

/* tokens */
enum SMUXCTL_CMD{
	SMUXCTL_HELP				= 900,
	SMUXCTL_IF 					= 1000,
	SMUXCTL_RX 					,
	SMUXCTL_TX 					,
	SMUXCTL_RXMC				,
	SMUXCTL_TAGS 				,
	/* IF control */
	SMUXCTL_IF_CREATE_NAME			= 1010,
	SMUXCTL_IF_DELETE			,
	SMUXCTL_IF_APPEND_RSMUX		,
	SMUXCTL_IF_RX_MULTI			,
	SMUXCTL_IF_RX_POLICY		,
	SMUXCTL_IF_TX_POLICY		,
	SMUXCTL_IF_RXMC_POLICY		,
	SMUXCTL_IF_CARRIER_MODE		,
	/* Rule control */
	SMUXCTL_RULE_APPEND			= 1030,
	SMUXCTL_RULE_INSERT			,
	SMUXCTL_RULE_INSERT_BEFORE	,
	SMUXCTL_RULE_INSERT_AFTER	,
	SMUXCTL_RULE_REMOVE			,
	SMUXCTL_RULE_REMOVE_BY_FLT	,
	SMUXCTL_RULE_REMOVE_ALL		,
	SMUXCTL_RULE_REMOVE_ALIAS	,
	SMUXCTL_RULE_ALIAS			,
	SMUXCTL_RULE_PRIORITY		,
	SMUXCTL_RULE_RESETCOUNT		,
	/* Action */
	SMUXCTL_PUSH_TAG			= 1100,
	SMUXCTL_POP_TAG				,
	SMUXCTL_DUP_FORWARD			,
	SMUXCTL_SET_ETHTYPE			,
	SMUXCTL_SET_RXIF			,
	SMUXCTL_SET_TPID			,
	SMUXCTL_SET_VID				,
	SMUXCTL_SET_PRIORITY		,
	SMUXCTL_SET_CFI				,
	SMUXCTL_SET_SKB_MARK		,
#ifdef CONFIG_RTK_SKB_MARK2
	SMUXCTL_SET_SKB_MARK2            ,
#endif
	SMUXCTL_SET_DSCP			,
	SMUXCTL_COPY_8021Q			,
	SMUXCTL_COPY_TPID			,
	SMUXCTL_COPY_VID			,
	SMUXCTL_COPY_PRIORITY		,
	SMUXCTL_COPY_CFI			,
	SMUXCTL_MAP_DSCP_TO_PRI		,
	SMUXCTL_MAP_DSCP_TO_PRI_SET	,
	SMUXCTL_TARGET_SET			,
	SMUXCTL_REVERT_VLAN			,
	/* Filter */
	SMUXCTL_FLT_ETHTYPE			= 1200,
	SMUXCTL_FLT_TPID			,
	SMUXCTL_FLT_VID				,
	SMUXCTL_FLT_CFI				,
	SMUXCTL_FLT_PRIORITY		,
	SMUXCTL_FLT_ORG_TPID		,
	SMUXCTL_FLT_ORG_VID			,
	SMUXCTL_FLT_ORG_CFI			,
	SMUXCTL_FLT_ORG_PRIORITY	,
	SMUXCTL_FLT_TXIF            ,
	SMUXCTL_FLT_SKB_MARK        ,
#ifdef CONFIG_RTK_SKB_MARK2
	SMUXCTL_FLT_SKB_MARK2        ,
#endif
	SMUXCTL_FLT_UNI_MAC        	,
	SMUXCTL_FLT_DMAC			,
	SMUXCTL_FLT_SMAC			,
	SMUXCTL_FLT_SIP				,
	SMUXCTL_FLT_DIP	        	,
	SMUXCTL_FLT_SIP_RANGE		,
	SMUXCTL_FLT_DIP_RANGE      	,
	SMUXCTL_FLT_IPL4PROTO       ,
	SMUXCTL_FLT_MULTICAST		,
	SMUXCTL_FLT_UNI_DMAC_OUTERMOST_VLAN,
	SMUXCTL_FLT_DSCP_RANGE,
	/* DSCP mapping */
	SMUXCTL_DSCP_MAPPING_ENABLE	,
	SMUXCTL_DSCP_MAPPING_SET	,
	SMUXCTL_DSCP_MAPPING_DEFAULT,
	/* Other */
	SMUXCTL_SHOW_TABLE			= 1300,
	SMUXCTL_DEFAULT_TPID	    ,
	SMUXCTL_DEFAULT_VID			,
	SMUXCTL_RUN_TEST			= 5000,
};


static struct option longopts[] = {
	{"if",					required_argument,	NULL, SMUXCTL_IF}, 
	{"rx",					                0,	NULL, SMUXCTL_RX}, 
	{"tx",					                0,	NULL, SMUXCTL_TX}, 
	{"rxmc",					            0,	NULL, SMUXCTL_RXMC},
	{"tags",				required_argument,	NULL, SMUXCTL_TAGS}, 
	{"if-create-name",		required_argument,	NULL, SMUXCTL_IF_CREATE_NAME},
	{"if-delete",	    	required_argument,	NULL, SMUXCTL_IF_DELETE},	
	{"set-if-rsmux",				        0,	NULL, SMUXCTL_IF_APPEND_RSMUX},	
	{"set-if-rx-multi",		required_argument,	NULL, SMUXCTL_IF_RX_MULTI},
	{"set-if-rx-policy",	required_argument,	NULL, SMUXCTL_IF_RX_POLICY},
	{"set-if-tx-policy",	required_argument,	NULL, SMUXCTL_IF_TX_POLICY},
	{"set-if-rxmc-policy",	required_argument,	NULL, SMUXCTL_IF_RXMC_POLICY},
	{"carrier",				required_argument,	NULL, SMUXCTL_IF_CARRIER_MODE},
	{"rule-append", 	                    0,	NULL, SMUXCTL_RULE_APPEND},
	{"rule-insert", 	                    0,	NULL, SMUXCTL_RULE_INSERT},
	{"rule-insert-before", 	required_argument,	NULL, SMUXCTL_RULE_INSERT_BEFORE},
	{"rule-insert-after", 	required_argument,	NULL, SMUXCTL_RULE_INSERT_AFTER},
	{"rule-remove", 	    required_argument,	NULL, SMUXCTL_RULE_REMOVE},
	{"rule-remove-all", 	                0,	NULL, SMUXCTL_RULE_REMOVE_ALL},	
	{"rule-remove-alias", 	required_argument,	NULL, SMUXCTL_RULE_REMOVE_ALIAS},	
	{"rule-alias", 	        required_argument,	NULL, SMUXCTL_RULE_ALIAS},	
	{"rule-priority", 	    required_argument,	NULL, SMUXCTL_RULE_PRIORITY},
	{"rule-reset-count", 	                0,	NULL, SMUXCTL_RULE_RESETCOUNT},
	{"filter-ethertype",    required_argument,	NULL, SMUXCTL_FLT_ETHTYPE},	
	{"filter-tpid", 		required_argument,	NULL, SMUXCTL_FLT_TPID},	
	{"filter-vid", 		    required_argument,	NULL, SMUXCTL_FLT_VID},	
	{"filter-priority", 	required_argument,	NULL, SMUXCTL_FLT_PRIORITY},	
	{"filter-cfi", 			required_argument,	NULL, SMUXCTL_FLT_CFI},
	{"filter-org-tpid", 	required_argument,	NULL, SMUXCTL_FLT_ORG_TPID},	
	{"filter-org-vid", 		required_argument,	NULL, SMUXCTL_FLT_ORG_VID},	
	{"filter-org-priority", required_argument,	NULL, SMUXCTL_FLT_ORG_PRIORITY},	
	{"filter-org-cfi",		required_argument,	NULL, SMUXCTL_FLT_ORG_CFI},
	{"filter-txif", 		required_argument,	NULL, SMUXCTL_FLT_TXIF},
	{"filter-skb-mark", 	required_argument,	NULL, SMUXCTL_FLT_SKB_MARK},
#ifdef CONFIG_RTK_SKB_MARK2
	{"filter-skb-mark2", 	required_argument,	NULL, SMUXCTL_FLT_SKB_MARK2},
#endif
	{"filter-unicast-mac", 	required_argument,	NULL, SMUXCTL_FLT_UNI_MAC},
	{"filter-dmac", 		required_argument,	NULL, SMUXCTL_FLT_DMAC},
	{"filter-smac", 		required_argument,	NULL, SMUXCTL_FLT_SMAC},
	{"filter-sip", 			required_argument,	NULL, SMUXCTL_FLT_SIP},
	{"filter-dip", 			required_argument,	NULL, SMUXCTL_FLT_DIP},
	{"filter-sip-range", 	required_argument,	NULL, SMUXCTL_FLT_SIP_RANGE},
	{"filter-dip-range", 	required_argument,	NULL, SMUXCTL_FLT_DIP_RANGE},
	{"filter-ipl4proto", 	required_argument,	NULL, SMUXCTL_FLT_IPL4PROTO},
	{"filter-uni-dmac-outermost-vlan", required_argument, NULL, SMUXCTL_FLT_UNI_DMAC_OUTERMOST_VLAN},
	{"filter-multicast", 					0,	NULL, SMUXCTL_FLT_MULTICAST},
	{"filter-dscp-range",   required_argument,	NULL, SMUXCTL_FLT_DSCP_RANGE},
	{"default-tpid", 		required_argument,	NULL, SMUXCTL_DEFAULT_TPID},
	{"default-vid", 		required_argument,	NULL, SMUXCTL_DEFAULT_VID},		
	{"push-tag",    						0,	NULL, SMUXCTL_PUSH_TAG},	
	{"pop-tag",    					    	0,	NULL, SMUXCTL_POP_TAG},
	{"duplicate-forward",    				0,	NULL, SMUXCTL_DUP_FORWARD},
	{"set-ethertype",    	required_argument,	NULL, SMUXCTL_SET_ETHTYPE},	
	{"set-rxif",	    	required_argument,	NULL, SMUXCTL_SET_RXIF},
	{"set-tpid", 	    	required_argument,	NULL, SMUXCTL_SET_TPID},
	{"set-vid", 	    	required_argument,	NULL, SMUXCTL_SET_VID},
	{"set-priority", 	    required_argument,	NULL, SMUXCTL_SET_PRIORITY},
	{"set-cfi", 	    	required_argument,	NULL, SMUXCTL_SET_CFI},
	{"set-skb-mark", 	    required_argument,	NULL, SMUXCTL_SET_SKB_MARK},
#ifdef CONFIG_RTK_SKB_MARK2
	{"set-skb-mark2",            required_argument,  NULL, SMUXCTL_SET_SKB_MARK2},
#endif
	{"set-dscp", 	    	required_argument,	NULL, SMUXCTL_SET_DSCP},
	{"copy-8021q", 			required_argument,	NULL, SMUXCTL_COPY_8021Q},
	{"copy-tpid", 			required_argument,	NULL, SMUXCTL_COPY_TPID},
	{"copy-vid", 			required_argument,	NULL, SMUXCTL_COPY_VID},
	{"copy-priority", 		required_argument,	NULL, SMUXCTL_COPY_PRIORITY},
	{"copy-cfi", 			required_argument,	NULL, SMUXCTL_COPY_CFI},
	{"map-dscp-to-pbit",	required_argument,	NULL, SMUXCTL_MAP_DSCP_TO_PRI},
	{"map-dscp-to-pbit-set",required_argument,	NULL, SMUXCTL_MAP_DSCP_TO_PRI_SET},	
	{"target",				required_argument,	NULL, SMUXCTL_TARGET_SET},
	{"dscp-mapping-enable", required_argument,	NULL, SMUXCTL_DSCP_MAPPING_ENABLE},
	{"dscp-mapping-set", 	required_argument,	NULL, SMUXCTL_DSCP_MAPPING_SET},
	{"dscp-mapping-default",				0,	NULL, SMUXCTL_DSCP_MAPPING_DEFAULT},
	{"revert-vlan",							0,	NULL, SMUXCTL_REVERT_VLAN},
	{"show-table", 	    	                0,	NULL, SMUXCTL_SHOW_TABLE},
	{"run-test", 	    	                0,	NULL, SMUXCTL_RUN_TEST},
	{"help",				                0,	NULL, SMUXCTL_HELP},
	{ 0, 0, 0, 0}
};

static void print_usage(void) {
	printf("smuxctl <commands>\n");
	printf("  --if <if_name>\n");
	printf("          Set the target interface <if_name>\n\n");
	printf("  --rx	  Set the direction. Exclusive with --tx & --rxmc\n\n");
	printf("  --tx	  Set the direction. Exclusive with --rx & --rxmc\n\n");
	printf("  --rxmc  Set the direction. Exclusive with --rx & --tx\n\n");
	printf("  --tags <nbr_of_tags>\n");
	printf("          Set the number of tags <nbr_of_tags>\n\n");
	printf("  --if-create-name <real_if_name> <VLAN_if_name>\n");
	printf("          Creates a new <VLAN_if_name> and attaches it to the real device <real_if_name>\n\n");
	printf("  --if-delete <VLAN_if_name>\n");
	printf("          Delete the interface <VLAN_if_name>\n\n");
	printf("  --set-if-rsmux\n");
	printf("		  Modify created virtual device to RSMUX, rules can be added for this interface\n\n");
	printf("  --set-if-rx-multi <0/1>\n");
	printf("		  Set interface receive multicast/broadcast packet mode on RX direction. If the packet match the rules also do duplicate forward. Default is disabled\n\n");
	printf("  --set-if-rx-policy <ACCEPT/DROP/CONTINUE>\n");
	printf("		  Set rx default policy for the real SMUX interface\n\n");
	printf("  --set-if-tx-policy <ACCEPT/DROP/CONTINUE>\n");
	printf("		  Set tx default policy for the real SMUX interface\n\n");
	printf("  --set-if-rxmc-policy <ACCEPT/DROP/CONTINUE>\n");
	printf("		  Set rxmc default policy for the real SMUX interface\n\n");
	printf("  --carrier <AUTO/MANUAL>\n");
	printf("          Set netif carrier mode. if config auto, will change device carried by real IF. otherwise change by manual, ex: tr142\n\n");
	printf("  --rule-append\n");
	printf("          Append a new tagging rule at last of the specified tagging rule table\n\n");
	printf("  --rule-insert\n");
	printf("          Inserts a new tagging rule at head of the specified tagging rule table\n\n");
	printf("  --rule-insert-before <rule_id> \n");
	printf("          Inserts a new tagging rule before <rule_id> in the specified tagging rule table\n\n");
	printf("  --rule-insert-after <rule_id> \n");
	printf("          Inserts a new tagging rule after <rule_id> in the specified tagging rule table\n\n");
	printf("  --rule-remove <rule_id> \n");
	printf("          Removes a tagging rule that matches <rule_id> in the specified tagging rule table\n\n");
	printf("  --rule-remove-all \n");
	printf("          Removes all tagging rules in the specified tagging rule table\n\n");
	printf("  --rule-remove-alias <alias[+]> \n");
	printf("          Removes the tagging rule that matches <alias> in the specified tagging rule table\n\n");
	printf("  --rule-alias <alias>\n");
	printf("          Set alias name of the rule\n\n");
	printf("  --rule-priority <0-326767>\n");
	printf("          Set the rule priority, the higher value indicate highest priority\n\n");
	printf("  --rule-reset-count \n");
	printf("          Reset rule hit count\n\n");
	printf("  --filter-ethertype <ethertype> \n");
	printf("          Match the <ethertype> in the Ethernet Header\n\n");
	printf("  --filter-tpid <tpid> <tag_nbr>\n");
	printf("          Match the current <tpid> of the VLAN Header number <tag_nbr>, if <tag_nbr> is 0 indicate the outerest vlan\n\n");
	printf("  --filter-vid <vid> <tag_nbr>\n");
	printf("          Match the current <vid> of the VLAN Header number <tag_nbr>, if <tag_nbr> is 0 indicate the outerest vlan\n\n");
	printf("  --filter-priority <priority> <tag_nbr>\n");
	printf("          Match the current <priority> of the VLAN Header number <tag_nbr>, if <tag_nbr> is 0 indicate the outerest vlan\n\n");
	printf("  --filter-cfi <cfi> <tag_nbr>\n");
	printf("          Match the current <cfi> of the VLAN Header number <tag_nbr>, if <tag_nbr> is 0 indicate the outerest vlan\n\n");
	printf("  --filter-org-tpid <tpid> <tag_nbr>\n");
	printf("          Match the original <tpid> of the VLAN Header number <tag_nbr>\n\n");
	printf("  --filter-org-vid <vid> <tag_nbr>\n");
	printf("          Match the original <vid> of the VLAN Header number <tag_nbr>\n\n");
	printf("  --filter-org-priority <priority> <tag_nbr>\n");
	printf("          Match the original <priority> of the VLAN Header number <tag_nbr>\n\n");
	printf("  --filter-org-cfi <cfi> <tag_nbr>\n");
	printf("          Match the original <cfi> of the VLAN Header number <tag_nbr>\n\n");
	printf("  --filter-txif <VLAN_if_name>\n");
	printf("          Match the TX interface <VLAN_if_name>\n\n");
	printf("  --filter-skb-mark <mask> <value>\n");
	printf("          Match the <value> of the skb mark <mask> bit field\n\n");
#ifdef CONFIG_RTK_SKB_MARK2
	printf("  --filter-skb-mark2 <mask> <value>\n");
	printf("          Match the <value> of the skb mark2 <mask> bit field\n\n");
#endif
	printf("  --filter-unicast-mac <value>\n");
	printf("          Match the receive unicast packet destination MAC address with RX interface's dev_addr if rxif has been set\n\n");
	printf("  --filter-dmac <value> [<mask>]\n");
	printf("          Match the receive unicast packet destination MAC address\n\n");
	printf("  --filter-smac <value> [<mask>]\n");
	printf("          Match the receive unicast packet source MAC address\n\n");
	printf("  --filter-sip <value>\n");
	printf("          Match <value> with source IP in IPv4 header\n\n");
	printf("  --filter-dip <value>\n");
	printf("          Match <value> with destination IP in IPv4 header\n\n");
	printf("  --filter-sip-range <value1> <value2>\n");
	printf("          Check if source IP in IPv4 header in range of <value1> to <value2>\n\n");
	printf("  --filter-dip-range <value1> <value2>\n");
	printf("          Check if destination IP in IPv4 header in range of <value1> to <value2>\n\n");
	printf("  --filter-ipl4proto <value>\n");
	printf("          Match the protocol field in IP header.\n\n");
	printf("  --filter-uni-dmac-outermost-vlan <value>\n");
	printf("          Match the dmac from LUT foroutermost vlan ID.\n\n");
	printf("  --filter-multicast \n");
	printf("          Match the packet type is multicast\n\n");
	printf("  --filter-dscp-range <value>\n");
	printf("          Match bitmask <value> with DSCP in IPv4 header\n\n");
	printf("  --default-tpid <tpid>\n");
	printf("          Set the default TPID of a tagging rule table to <tpid>\n\n");
	printf("  --default-vid <vid>\n");
	printf("          Set the default VID of a tagging rule table to <vid>\n\n");
	printf("  --push-tag\n");
	printf("          Add the default VLAN tag of the corresponding tagging rule table as the new outer tag\n");
	printf("  --pop-tag\n");
	printf("          Remove the outermost VLAN tag\n\n");
	printf("  --duplicate-forward\n");
	printf("		  Clone SKB and forword to interface if rxif is set\n\n");
	printf("  --set-ethertype <ethertype>\n");
	printf("          Set the Ethertype value of the Ethernet Header to <ethertype>\n\n");
	printf("  --set-rxif <VLAN_if_name>\n");
	printf("          Forward frames in the RX direction to <VLAN_if_name> \n\n");
	printf("  --set-tpid <vid> <tag_nbr>\n");
	printf("          Set the TPID of the VLAN Header number <tag_nbr> to <tpid>\n\n");
	printf("  --set-vid <vid> <tag_nbr>\n");
	printf("          Set the VID of the VLAN Header number <tag_nbr> to <vid>\n\n");
	printf("  --set-priority <priority> <tag_nbr>\n");
	printf("          Set the priority of the VLAN Header number <tag_nbr> to <priority>\n\n");
	printf("  --set-cfi <cfi> <tag_nbr>\n");
	printf("          Set the priority of the VLAN Header number <tag_nbr> to <cfi>\n\n");
	printf("  --set-dscp <dscp>\n");
	printf("          Set the DSCP value to <dscp>\n\n");
	printf("  --copy-8021q <tag_nbr_src> <tag_nbr_dest>\n");
	printf("          Copy the entire VLAN Header from number <tag_nbr_src> to number <tag_nbr_dest>\n\n");
	printf("  --copy-tpid <tag_nbr_src> <tag_nbr_dest>\n");
	printf("          Copy the tpid of the VLAN Header from number <tag_nbr_src> to number <tag_nbr_dest>\n\n");
	printf("  --copy-vid <tag_nbr_src> <tag_nbr_dest>\n");
	printf("          Copy the vid of the VLAN Header from number <tag_nbr_src> to number <tag_nbr_dest>\n\n");
	printf("  --copy-priority <tag_nbr_src> <tag_nbr_dest>\n");
	printf("          Copy the priority of the VLAN Header from number <tag_nbr_src> to number <tag_nbr_dest>\n\n");
	printf("  --copy-cfi <tag_nbr_src> <tag_nbr_dest>\n");
	printf("          Copy the cfi of the VLAN Header from number <tag_nbr_src> to number <tag_nbr_dest>\n\n");
	printf("  --map-dscp-to-pbit <tag_nbr>\n");
	printf("          Mapping DSCP to VLAN priority of the VLAN Header number <tag_nbr>\n\n");
	printf("  --map-dscp-to-pbit-set <DSCP> <priority>\n");
	printf("		  Mapping IP layer DSCP <DSCP> to 802.1q header <priority> for the default table\n\n");
	printf("  --target <CONTINUE/ACCEPT/DROP>\n");
	printf("		  Modify the target of packets, final result of target decided by first matched rule\n\n");
	printf("  --revert-vlan\n");
	printf("		  Revert VLAN to original VLAN, if the packet without VLAN header, it will revert to untag.\n\n");
	printf("  --set-skb-mark <mask> <value>\n");
	printf("          Set <value> to skb mark <mask> bit field\n\n");
#ifdef CONFIG_RTK_SKB_MARK2
	printf("  --set-skb-mark2 <mask> <value>\n");
	printf("          Set <value> to skb mark2 <mask> bit field\n\n");
#endif
	printf("  --dscp-mapping-enable <value> <tag_nbr>\n");
	printf("          Enabling DSCP mapping function of the real interface to <value> effect on the VLAN Header number <tag_nbr>\n\n");
	printf("  --dscp-mapping-set <DSCP> <priority>\n");
	printf("		  Mapping IP layer DSCP <DSCP> to 802.1q header <priority> for the real interface\n\n");
	printf("  --dscp-mapping-default\n");
	printf("          Reset DSCP mapping function of the real interface to default value\n\n");
	printf("  --show-table\n");
	printf("          List all tagging rules stored in the specified tagging rule table\n\n");
	exit(0);
}

static int hex2num(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

int strToMAC(const char *str, unsigned char *mac)
{
	int i, a, b;
	if(str == NULL || mac == NULL)return -1;
	for (i = 0; i < 6; i++) 
	{
		a = hex2num(*str++);
		if (a < 0) return -1;
		b = hex2num(*str++);
		if (b < 0) return -1;
		*mac++ = (a << 4) | b;
		if(*str == ':' || *str == '-') str++;
	}
	return 0;
}

int strToDSCP(const char *str, unsigned char *dscp)
{
	int i, a, b;
	if(str == NULL || dscp == NULL)return -1;
	for (i = 0; i < 8; i++)
	{
		a = hex2num(*str++);
		if (a < 0) return -1;
		b = hex2num(*str++);
		if (b < 0) return -1;
		*dscp++ = (a << 4) | b;
	}
	return 0;
}

static void print_error(const struct option *opt, int index, const char *msg) {
	printf("%s: %s\n", opt[index].name, msg);
	exit(-1);
}

//take additional argument
static char *get_arg(int argc, char *argv[]) {
	char *arg;
	if (optind >= argc)
		return NULL;
	
	arg = argv[optind];
	if (arg[0] == '-')
		return NULL;
	
	optind++;
	return arg;
}

/* interfaces */
#if 1

static char *if_real_name, *if_virt_name;
static unsigned int if_flags;
static struct smux_ioctl_rule_t io_rule;
static unsigned int if_policy[SMUX_DIR_MAX];
static int if_carrier;
static int if_rx_multi;


#if 0 
static struct smux_interface_s *smux_interface_p;
static struct smux_interface_s *if_init(void) {
	unsigned int sz = sizeof(struct smux_interface_s);
	if (smux_interface_p)
		return smux_interface_p;
	
	smux_interface_p = malloc(sz);
	if (!smux_interface_p) {
		printf("failed to alloc memory\n");
		exit(1);
	}
	memset(smux_interface_p, 0, sz);
	return smux_interface_p;
}

static void if_fini(void) {
	if (smux_interface_p)
		free(smux_interface_p);
}

static void if_dump(void) {
	printf("smux_if: %p\n", smux_interface_p);
	if (!smux_interface_p) 
		return;
	printf(" real: %s, virt: %s, flags: %x\n", smux_interface_p->real_name, smux_interface_p->virt_name, smux_interface_p->dev_flags);
}

static void if_set_names(const char *real_name, const char *virt_name) {
	struct smux_interface_s *smuxif = if_init();
	
	if (real_name) 
		strncpy(smuxif->real_name,real_name,IFNAMSIZ);
	if (virt_name) 
		strncpy(smuxif->virt_name,virt_name,IFNAMSIZ);
}

static void if_set_flags(unsigned int flags) {
	struct smux_interface_s *smuxif = if_init();
	smuxif->dev_flags |= flags;
}
#endif 
#endif 

static int command;
static int set_cmd(int cmd) {
	if (command) {
		printf("previous command is set (%d)\n", command);
		exit(1);
	}
	command = cmd;
	return cmd;
}

#if 0 
static void send_command(int cmd, void *p) {
	int fd = open(DEVICE_NODE, O_RDWR);
	int ret;
	if (fd < 0) {
		printf("Failed to open %s\n", DEVICE_NODE);
		exit(-1);		
	}
	
	ret = ioctl(fd, cmd, p);
	if (ret < 0) {
		printf("ioctl cmd=%d returns %d\n", cmd, ret);
	}
	return 0;
}	
#endif 

/* library function  - to be moved */
static const char DEVICE_NODE[] = "/dev/" SMUXDEV_NODE_NAME;
int smux_fd;

#ifdef DEBUG
#define SMUX_DEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define SMUX_DEBUG(fmt, ...)
#endif

#define SMUX_LOG printf
#define SMUX_ERR
int smux_init(void) {
	int ret = 0, i;
	SMUX_DEBUG("%s\n",__func__);
	if_real_name = NULL;
	smux_fd = open(DEVICE_NODE, O_RDWR);
	if (smux_fd < 0) {
		SMUX_LOG("%s: failed\n", __func__);
		ret = errno;
	}
	for(i=0;i<SMUX_DIR_MAX;i++)
		if_policy[i] = SMUX_TARGET_END;
	
	if_carrier = SMUX_CARRIER_NONE;
	if_rx_multi = SMUX_RX_MULTI_NONE;
	return ret;
}

void smux_fini(void) {
	SMUX_DEBUG("%s\n",__func__);
	if(smux_fd) {
		close(smux_fd);
		smux_fd = 0;
	}
}

static inline int is_valid_if_name(const char *ifname) {
	return ((ifname!=NULL) && (strlen(ifname) < IFNAMSIZ));
}   

int check_mantatory_value(const char *ifname, struct smux_ioctl_rule_t *rule)
{
    int ret=1;
    if(!is_valid_if_name(ifname)){
        SMUX_DEBUG("%s: invalid interface\n",__func__);
        ret=0;
    }      
    if(rule->table.dir >= SMUX_DIR_MAX){
        SMUX_DEBUG("%s: invalid DIR\n",__func__);
        ret=0;
    } 
    if(rule->table.nr_tag < 0 || rule->table.nr_tag > SMUX_MAX_TAGS){
        SMUX_DEBUG("%s: invalid tag number\n",__func__);
        ret=0;
    }      
    return ret;
}

int smux_interface_parameter_set(const char *real_name, unsigned int flags) {
	struct smux_interface_s smuxif;
	int ret = 0;
	SMUX_DEBUG("Set: real=%s flags=%d\n",real_name,virt_name,flags);
	if (!is_valid_if_name(real_name)) {
		SMUX_LOG("invalid interface names\n");
		return -EINVAL;
	}
	
	strncpy(smuxif.real_name,real_name,IFNAMSIZ);
	smuxif.dev_flags = flags;
	memcpy(smuxif.policy, if_policy, sizeof(smuxif.policy));
	smuxif.carrier = if_carrier;
	smuxif.rx_multi = if_rx_multi;
	ret = ioctl(smux_fd, SMUXCTL_CMD_SET_IF_PARM, &smuxif);
	SMUX_DEBUG("Set: ret=%d\n",ret);
	return ret;
}

int smux_interface_create(const char *real_name, const char *virt_name, unsigned int flags) {
	struct smux_interface_s smuxif;
	int ret = 0;
	SMUX_DEBUG("Create: real=%s virt=%s flags=%d\n",real_name,virt_name,flags);
	if (!is_valid_if_name(real_name) || !is_valid_if_name(virt_name)) {
		SMUX_LOG("invalid interface names\n");
		return -EINVAL;
	}
	
	strncpy(smuxif.real_name,real_name,IFNAMSIZ);
	strncpy(smuxif.virt_name,virt_name,IFNAMSIZ);
	smuxif.dev_flags = flags;
	memcpy(smuxif.policy, if_policy, sizeof(smuxif.policy));
	smuxif.carrier = if_carrier;
	smuxif.rx_multi = if_rx_multi;
	ret = ioctl(smux_fd, SMUXCTL_CMD_IF_CREATE, &smuxif);
	SMUX_DEBUG("Create: ret=%d\n",ret);
	return ret;
}

int smux_interface_delete(const char *virt_name) {
	struct smux_interface_s smuxif;
	int ret = 0;
	SMUX_DEBUG("Delete: virt=%s\n",virt_name);
	if (!is_valid_if_name(virt_name)) {
		SMUX_LOG("invalid interface names\n");
		return -EINVAL;
	}
	strncpy(smuxif.virt_name,virt_name,IFNAMSIZ);
	ret = ioctl(smux_fd, SMUXCTL_CMD_IF_DELETE, &smuxif);
	SMUX_DEBUG("Delete: ret=%d\n",ret);
	return ret;
}

/* returns position id if success (>0), else negative number for error */
int smux_rule_insert(const char *dev_name, struct smux_ioctl_rule_t *rule) {
	int ret;
	SMUX_DEBUG("%s:\n",__func__);
	
	/* validity check */
	if (strlen(dev_name)>=IFNAMSIZ)
		return -EINVAL;
	strncpy(rule->table.ifname, dev_name, sizeof(rule->table.ifname)-1);
	
	ret = ioctl(smux_fd, SMUXCTL_CMD_RULE_ADD, rule);
	
	SMUX_DEBUG("%s: ret=%d id=%d\n",__func__,ret,rule->id);
	
	return ret;
}

int smux_rule_delete(const char *dev_name, struct smux_ioctl_rule_t *rule) {
	int ret;
	SMUX_DEBUG("%s:\n",__func__);
	
	/* validity check */
	if (strlen(dev_name)>=IFNAMSIZ)
		return -EINVAL;
	strncpy(rule->table.ifname, dev_name, sizeof(rule->table.ifname)-1);
	
	ret = ioctl(smux_fd, SMUXCTL_CMD_RULE_DELETE, rule);
	
	SMUX_DEBUG("%s: ret=%d id=%d\n",__func__,ret,rule->id);
	
	return ret;
}

int smux_rule_reset(const char *dev_name, struct smux_ioctl_rule_t *rule) {
	int ret;
	SMUX_DEBUG("%s:\n",__func__);
	
	/* validity check */
	if (dev_name==NULL || strlen(dev_name)>=IFNAMSIZ)
		return -EINVAL;
	strncpy(rule->table.ifname, dev_name, sizeof(rule->table.ifname)-1);
	
	ret = ioctl(smux_fd, SMUXCTL_CMD_RULE_RESET, rule);
	
	SMUX_DEBUG("%s: ret=%d\n",__func__,ret);
	
	return ret;
}

int smux_show_table(const char *dev_name, struct smux_ioctl_rule_t *rule) {
	int ret;
	SMUX_DEBUG("%s:\n",__func__);
	
	/* validity check */
	if (dev_name==NULL || strlen(dev_name)>=IFNAMSIZ)
		return -EINVAL;
	strncpy(rule->table.ifname, dev_name, sizeof(rule->table.ifname)-1);
	
	ret = ioctl(smux_fd, SMUXCTL_CMD_SHOW_TABLE, rule);
	
	SMUX_DEBUG("%s: ret=%d\n",__func__,ret);
	
	return ret;
}

int smux_set_default(const char *dev_name, struct smux_ioctl_rule_t *rule) {
    int ret;
    SMUX_DEBUG("%s:\n",__func__);

    /* validity check */
	if (strlen(dev_name)>=IFNAMSIZ)
		return -EINVAL;
	strncpy(rule->table.ifname, dev_name, sizeof(rule->table.ifname)-1);
	
	ret = ioctl(smux_fd, SMUXCTL_CMD_SET_DEFAULT, rule);	
	return 0;
}

int smux_set_qos(const char *dev_name, struct smux_ioctl_rule_t *rule) {
    int ret;
    SMUX_DEBUG("%s:\n",__func__);

    /* validity check */
    if (strlen(dev_name)>=IFNAMSIZ)
        return -EINVAL;
    strncpy(rule->table.ifname, dev_name, sizeof(rule->table.ifname)-1);
	
    ret = ioctl(smux_fd, SMUXCTL_CMD_SET_QOS, rule);	
    return 0;
}

int smux_rule_add_action(struct smux_rule_t *rule,enum smux_rule_action_e op, unsigned int arg0, unsigned arg1) {
	struct smux_rule_action_t *act;
	
	if (rule->action_count>=SMUX_ACTION_MAX) {
		printf("%s limit reached\n", __func__);
		return -EINVAL;
	}
	act = &rule->actions[rule->action_count];
	rule->action_count++;
	act->action = op;
	act->args[0] = arg0;
	act->args[1] = arg1;
	return 0;
}

#ifdef CONFIG_RTK_SKB_MARK2
int smux_rule_add_action_64(struct smux_rule_t *rule,enum smux_rule_action_e op, unsigned long long arg0, unsigned long long arg1) {
        struct smux_rule_action_t *act;

        if (rule->action_count>=SMUX_ACTION_MAX) {
                printf("%s limit reached\n", __func__);
                return -EINVAL;
        }
        act = &rule->actions[rule->action_count];
        rule->action_count++;
        act->action = op;
        act->args_64[0] = arg0;
        act->args_64[1] = arg1;
        return 0;
}
#endif

int smux_rule_add_action_set_rxif(struct smux_rule_t *rule,const char *rxif_name) {
	struct smux_rule_action_t *act;
	
	if (rule->action_count>=SMUX_ACTION_MAX) {
		printf("%s limit reached\n", __func__);
		return -EINVAL;
	}
	if (strlen(rxif_name) >= IFNAMSIZ) {
		printf("rxif name too long(%d)\n",strlen(rxif_name));
		return -EINVAL;
	}
	if (rule->rxif_name[0] != '\0') {
		printf("duplicated set-rxif (%s)\n", rule->rxif_name);
		return -EINVAL;
	}
	act = &rule->actions[rule->action_count];
	rule->action_count++;
	act->action = SMUX_ACT_SET_RXIF;
	strncpy(act->rxif_name, rxif_name, IFNAMSIZ);
	strncpy(rule->rxif_name, rxif_name, IFNAMSIZ);
	return 0;
}

int smux_rule_filter_set_sip(struct smux_rule_filter_t *f, uint32_t sip) {
	f->en_filter |= (1<<SMUX_FLT_SIP);
	f->sip = sip;
	return 0;
}

int smux_rule_filter_set_dip(struct smux_rule_filter_t *f, uint32_t dip) {
	f->en_filter |= (1<<SMUX_FLT_DIP);
	f->dip = dip;
	return 0;
}

int smux_rule_filter_set_sip_range(struct smux_rule_filter_t *f, uint32_t sip_start, uint32_t sip_end) {
	f->en_filter |= (1<<SMUX_FLT_SIP_RANGE);
	//not do endian
	f->sip_start = sip_start;
	f->sip_end = sip_end;
	return 0;
}

int smux_rule_filter_set_dip_range(struct smux_rule_filter_t *f, uint32_t dip_start, uint32_t dip_end) {
	f->en_filter |= (1<<SMUX_FLT_DIP_RANGE);
	//not do endian
	f->dip_start = dip_start;
	f->dip_end = dip_end;
	return 0;
}

int smux_rule_filter_set_ipl4proto(struct smux_rule_filter_t *f, uint8_t ipl4proto) {
	f->en_filter |= (1<<SMUX_FLT_IPL4PROTO);
	f->ipl4proto = ipl4proto;
	return 0;
}

int smux_rule_filter_set_multicast(struct smux_rule_filter_t *f) {
	f->en_filter |= (1<<SMUX_FLT_MULTICAST);
	return 0;
}

int smux_rule_filter_set_dscp_range(struct smux_rule_filter_t *f, uint8_t *dscp) {
	int i;
	f->en_filter |= (1<<SMUX_FLT_DSCP_RANGE);
	for(i=0;i<sizeof(f->ipdscp);i++) f->ipdscp[i] = (uint8_t) dscp[i];
	return 0;
}

int smux_rule_filter_set_ethtype(struct smux_rule_filter_t *f, uint16_t ethtype) {
	f->en_filter |= (1<<SMUX_FLT_ETHTYPE);
	//do not endian, the smuxdev build_info will do
	f->ethtype = ethtype;
	return 0;
}

int smux_rule_filter_set_uni_mac(struct smux_rule_filter_t *f, uint16_t check_mac) {
	f->en_filter |= (1<<SMUX_FLT_UNI_MAC);
	f->check_mac = check_mac;
	return 0;
}

int smux_rule_filter_set_dmac(struct smux_rule_filter_t *f, uint8_t *mac, uint8_t *mask) {
	int i;
	f->en_filter |= (1<<SMUX_FLT_DMAC);
	for(i=0;i<sizeof(f->dmac);i++) f->dmac[i] = (uint8_t) mac[i];
	for(i=0;i<sizeof(f->dmac_mask);i++) f->dmac_mask[i] = (uint8_t) mask[i];
	return 0;
}

int smux_rule_filter_set_smac(struct smux_rule_filter_t *f, uint8_t *mac, uint8_t *mask) {
	int i;
	f->en_filter |= (1<<SMUX_FLT_SMAC);
	for(i=0;i<sizeof(f->smac);i++) f->smac[i] = (uint8_t) mac[i];
	for(i=0;i<sizeof(f->smac_mask);i++) f->smac_mask[i] = (uint8_t) mask[i];
	return 0;
}

int smux_rule_filter_set_tpid(struct smux_rule_filter_t *f, int depth, uint16_t tpid) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_VTAGS);
	//f->vtags[depth].vlan_proto &= ~0xffff;
	f->vtags[depth].vlan_proto = htons(tpid);
	f->vtags[depth].vlan_proto_mask = 0xffff;
	return 0;
}

int smux_rule_filter_set_vid(struct smux_rule_filter_t *f, int depth, uint16_t vid) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	if (vid > 4095)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_VTAGS);
	f->vtags[depth].vlan_tci &= ~VID_MASK;
	f->vtags[depth].vlan_tci |= (htons(vid)&VID_MASK);
	f->vtags[depth].vlan_tci_mask |= VID_MASK;
	return 0;
}

int smux_rule_filter_set_check_outermost_vlan_map_dmac_by_lut(struct smux_rule_filter_t *f, uint8_t check_outermost_vlan_map_dmac_by_lut) {
	f->en_filter |= (1<<SMUX_FLT_UNI_DMAC_OUTERMOST_VLAN);
	f->check_outermost_vlan_map_dmac_by_lut = check_outermost_vlan_map_dmac_by_lut;

	return 0;
}

int smux_rule_filter_set_priority(struct smux_rule_filter_t *f, int depth, uint16_t priority) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	if (priority > 7)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_VTAGS);
	f->vtags[depth].vlan_tci &= ~PRIO_MASK;
	f->vtags[depth].vlan_tci |= (htons(priority<<PRIO_SHIFT) & PRIO_MASK);
	f->vtags[depth].vlan_tci_mask |= PRIO_MASK;
	return 0;
}

int smux_rule_filter_set_cfi(struct smux_rule_filter_t *f, int depth, uint16_t cfi) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	if (cfi > 1)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_VTAGS);
	f->vtags[depth].vlan_tci &= ~CFI_MASK;
	f->vtags[depth].vlan_tci |= (htons(cfi<<CFI_SHIFT) & CFI_MASK);
	f->vtags[depth].vlan_tci_mask |= CFI_MASK;
	return 0;
}

int smux_rule_filter_set_org_tpid(struct smux_rule_filter_t *f, int depth, uint16_t tpid) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_ORG_VTAGS);
	//f->vtags[depth].vlan_proto &= ~0xffff;
	f->org_vtags[depth].vlan_proto = htons(tpid);
	f->org_vtags[depth].vlan_proto_mask = 0xffff;
	return 0;
}

int smux_rule_filter_set_org_vid(struct smux_rule_filter_t *f, int depth, uint16_t vid) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	if (vid > 4095)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_ORG_VTAGS);
	f->org_vtags[depth].vlan_tci &= ~VID_MASK;
	f->org_vtags[depth].vlan_tci |= (htons(vid)&VID_MASK);
	f->org_vtags[depth].vlan_tci_mask |= VID_MASK;
	return 0;
}

int smux_rule_filter_set_org_priority(struct smux_rule_filter_t *f, int depth, uint16_t priority) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	if (priority > 7)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_ORG_VTAGS);
	f->org_vtags[depth].vlan_tci &= ~PRIO_MASK;
	f->org_vtags[depth].vlan_tci |= (htons(priority<<PRIO_SHIFT) & PRIO_MASK);
	f->org_vtags[depth].vlan_tci_mask |= PRIO_MASK;
	return 0;
}

int smux_rule_filter_set_org_cfi(struct smux_rule_filter_t *f, int depth, uint16_t cfi) {
	if (depth > SMUX_MAX_TAGS)
		return -EINVAL;
	if (cfi > 1)
		return -EINVAL;
	
	f->en_filter |= (1<<SMUX_FLT_ORG_VTAGS);
	f->org_vtags[depth].vlan_tci &= ~CFI_MASK;
	f->org_vtags[depth].vlan_tci |= (htons(cfi<<CFI_SHIFT) & CFI_MASK);
	f->org_vtags[depth].vlan_tci_mask |= CFI_MASK;
	return 0;
}

int smux_rule_filter_set_txif(struct smux_rule_filter_t *f, const char *txif_name) {
	if (strlen(txif_name) >= IFNAMSIZ) {
		printf("rxif name too long(%d)\n",strlen(txif_name));
		return -EINVAL;
	}
	f->txif_match = IFNAMSIZ;
	if(txif_name[strlen(txif_name)-1] == '+')
		f->txif_match = strlen(txif_name)-1;
	f->en_filter |= (1<<SMUX_FLT_TXIF_NAME);
	strncpy(f->txif_name, txif_name, f->txif_match);
	f->txif_name[f->txif_match] = '\0';
	
	return 0;
}

int smux_rule_filter_set_skb_mark(struct smux_rule_filter_t *f, uint32_t mask, uint32_t value) {
	f->en_filter |= (1<<SMUX_FLT_SKB_MARK);
	f->mark.mask = mask;
	f->mark.value = value;
	return 0;
}

#ifdef CONFIG_RTK_SKB_MARK2
int smux_rule_filter_set_skb_mark2(struct smux_rule_filter_t *f, uint64_t mask, uint64_t value) {
        f->en_filter |= (1<<SMUX_FLT_SKB_MARK2);
        f->mark2.mask = mask;
        f->mark2.value = value;
        return 0;
}
#endif

int smux_set_if_policy(int dir, const char *policy)
{
	if(dir >= SMUX_DIR_MAX) return 1;
	
	if(!strcasecmp(policy, "ACCEPT"))
		if_policy[dir] = SMUX_TARGET_ACCEPT;
	else if(!strcasecmp(policy, "DROP"))
		if_policy[dir] = SMUX_TARGET_DROP;
	else if(!strcasecmp(policy, "CONTINUE"))
		if_policy[dir] = SMUX_TARGET_CONTINUE;
	else 
		return 1;
	
	return 0;
}

int smux_set_if_carrier(const char *mode)
{
	if(!strcasecmp(mode, "AUTO"))
		if_carrier = SMUX_CARRIER_AUTO;
	else if(!strcasecmp(mode, "MANUAL"))
		if_carrier = SMUX_CARRIER_MANUAL;
	else 
		return 1;
	
	return 0;
}

int smux_set_if_rx_multi(int mode)
{
	if(mode == 0)
		if_rx_multi = SMUX_RX_MULTI_NORMAL;
	else if(mode == 1)
		if_rx_multi = SMUX_RX_MULTI_MATCH;
	else
		return 1;
	
	return 0;
}

int main(int argc, char *argv[])
{
	int c, longindex;
	unsigned int vals[2];
	unsigned long long vals_64[2];
	//char *arg1, *arg2;
	char *strs[2];
	unsigned char mac[6], mac_mask[6];
	int ret = 0;
	unsigned char dscp[DSCP_LEN];
	//unsigned int action_args[2];
	
	
	//printf("%s(%d),argc=%d\n",__func__,__LINE__,argc);
	if (smux_init() < 0)
		return -EINVAL;
	
	memset(&io_rule, 0, sizeof(io_rule));
	
	io_rule.table.dir = SMUX_DIR_MAX;
	io_rule.table.nr_tag = -1;
	
	while ((c = getopt_long(argc, argv, "", longopts, &longindex)) >= 0)  {
		//printf("c=%d optind=%d optopt=%d optarg=%s index=%d\n",c,optind,optopt,optarg?optarg:"<none>",longindex);
		char *endptr;
		switch(c) {
			
		case SMUXCTL_IF:
			//strncpy(io_rule.table.ifname, optarg, sizeof(io_rule.table.ifname)-1);
			if_real_name = optarg;
			break;
			
		case SMUXCTL_TAGS:
			if (1!=sscanf(optarg, "%d", &io_rule.table.nr_tag))
				print_error(longopts,longindex,"invalid number");
			break;
			
		case SMUXCTL_TX:
			io_rule.table.dir = SMUX_DIR_TX;
			break;
			
		case SMUXCTL_RX:
			io_rule.table.dir = SMUX_DIR_RX;
			break;

		case SMUXCTL_RXMC:
			io_rule.table.dir = SMUX_DIR_RXMC;
			break;
			
		case SMUXCTL_FLT_ETHTYPE:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr == '\0' && (vals[0] <= 0xffff)) // success 
				smux_rule_filter_set_ethtype(&io_rule.rule.filters, vals[0]);
			else
				print_error(longopts,longindex,"invalid number");			
			break;

		case SMUXCTL_FLT_TPID:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_tpid(&io_rule.rule.filters, vals[1], vals[0]);
			break;	
		
		case SMUXCTL_FLT_VID:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_vid(&io_rule.rule.filters, vals[1], vals[0]);
			break;

		case SMUXCTL_FLT_CFI:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_cfi(&io_rule.rule.filters, vals[1], vals[0]);
			break;	

		case SMUXCTL_FLT_PRIORITY:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_priority(&io_rule.rule.filters, vals[1], vals[0]);
			break;

		case SMUXCTL_FLT_ORG_TPID:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_org_tpid(&io_rule.rule.filters, vals[1], vals[0]);
			break;	

		case SMUXCTL_FLT_ORG_VID:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_org_vid(&io_rule.rule.filters, vals[1], vals[0]);
			break;

		case SMUXCTL_FLT_ORG_CFI:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_org_cfi(&io_rule.rule.filters, vals[1], vals[0]);
			break;	

		case SMUXCTL_FLT_ORG_PRIORITY:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");			
			
			smux_rule_filter_set_org_priority(&io_rule.rule.filters, vals[1], vals[0]);
			break;
		
		case SMUXCTL_FLT_TXIF:
		    strs[0] = optarg;
			smux_rule_filter_set_txif(&io_rule.rule.filters,strs[0]);
			break;	
			
		case SMUXCTL_FLT_UNI_MAC:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0')			
				print_error(longopts,longindex,"invalid number");	
			
			smux_rule_filter_set_uni_mac(&io_rule.rule.filters,vals[0]);
			break;
			
		case SMUXCTL_FLT_DMAC:
		case SMUXCTL_FLT_SMAC:
			strs[0] = optarg;
			if(strToMAC(strs[0], mac) != 0)
			{
				print_error(longopts,longindex,"invalid mac address");	
			}
			
			strs[0] = get_arg(argc, argv);
			if (strs[0])
			{
				if(strToMAC(strs[0], mac_mask) != 0)
				{
					print_error(longopts,longindex,"invalid mac mask address");	
				}
			}
			else
			{
				memset(mac_mask, 255, sizeof(mac_mask));
			}
			
			if(c == SMUXCTL_FLT_SMAC)
				smux_rule_filter_set_smac(&io_rule.rule.filters,mac,mac_mask);
			else
				smux_rule_filter_set_dmac(&io_rule.rule.filters,mac,mac_mask);
			break;

		case SMUXCTL_FLT_SIP:
			if(inet_pton(AF_INET, optarg, (void *)&vals[0])!=1)
			{
				print_error(longopts,longindex,"invalid IPv4 address");
				break;
			}
			
			smux_rule_filter_set_sip(&io_rule.rule.filters,vals[0]);
			break;

		case SMUXCTL_FLT_DIP:
			if(inet_pton(AF_INET, optarg, (void *)&vals[0])!=1)
			{
				print_error(longopts,longindex,"invalid IPv4 address");
				break;
			}
			
			smux_rule_filter_set_dip(&io_rule.rule.filters,vals[0]);
			break;

		case SMUXCTL_FLT_SIP_RANGE:
			if(inet_pton(AF_INET, optarg, (void *)&vals[0])!=1)
			{
				print_error(longopts,longindex,"invalid start IPv4 address");
				break;
			}

			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");

			if(inet_pton(AF_INET, strs[0], (void *)&vals[1])!=1)
			{
				print_error(longopts,longindex,"invalid end IPv4 address");
				break;
			}
			
			smux_rule_filter_set_sip_range(&io_rule.rule.filters,vals[0],vals[1]);
			break;

		case SMUXCTL_FLT_DIP_RANGE:
			if(inet_pton(AF_INET, optarg, (void *)&vals[0])!=1)
			{
				print_error(longopts,longindex,"invalid start IPv4 address");
				break;
			}

			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");

			if(inet_pton(AF_INET, strs[0], (void *)&vals[1])!=1)
			{
				print_error(longopts,longindex,"invalid end IPv4 address");
				break;
			}
			
			smux_rule_filter_set_dip_range(&io_rule.rule.filters,vals[0],vals[1]);
			break;

		case SMUXCTL_FLT_IPL4PROTO:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0')			
				print_error(longopts,longindex,"invalid number");	
			
			smux_rule_filter_set_ipl4proto(&io_rule.rule.filters,vals[0]);
			break;

		case SMUXCTL_FLT_MULTICAST:			
			smux_rule_filter_set_multicast(&io_rule.rule.filters);
			break;

		case SMUXCTL_FLT_DSCP_RANGE:	
			strs[0] = optarg;
			if(strToDSCP(strs[0], dscp) != 0)
			{
				print_error(longopts,longindex,"invalid dscp value");
			}

			smux_rule_filter_set_dscp_range(&io_rule.rule.filters,dscp);
			break;

		case SMUXCTL_FLT_UNI_DMAC_OUTERMOST_VLAN:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0')
				print_error(longopts,longindex,"invalid number");

			smux_rule_filter_set_check_outermost_vlan_map_dmac_by_lut(&io_rule.rule.filters,vals[0]);
			break;

		case SMUXCTL_FLT_SKB_MARK:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
		    strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");		
			smux_rule_filter_set_skb_mark(&io_rule.rule.filters, vals[0], vals[1]);
			break;

#ifdef CONFIG_RTK_SKB_MARK2
		case SMUXCTL_FLT_SKB_MARK2:
                        vals_64[0] = strtoull(optarg, &endptr, 0);
                        if (*endptr != '\0'){ // success
                                print_error(longopts,longindex,"invalid number");
                            break;
                    }
                    strs[0] = get_arg(argc, argv);
                        if (!strs[0])
                                print_error(longopts,longindex,"require two arguments");

                        vals_64[1] = strtoull(strs[0], &endptr, 0);
                        if (*endptr != '\0') // success
                                print_error(longopts,longindex,"invalid number");

                        smux_rule_filter_set_skb_mark2(&io_rule.rule.filters, vals_64[0], vals_64[1]);
                        break;
#endif

		case SMUXCTL_IF_CREATE_NAME:			
			set_cmd(SMUXCTL_CMD_IF_CREATE);
			if_real_name = optarg;
			if_virt_name = get_arg(argc,argv);			
			if (!if_virt_name) {
				print_error(longopts,longindex,"require two arguments");
			}						
			break;
			
		case SMUXCTL_PUSH_TAG:
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_PUSH_TAG, 0, 0);
			break;
			
		case SMUXCTL_POP_TAG:
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_POP_TAG, 0, 0);
			break;
			
		case SMUXCTL_DUP_FORWARD:
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_DUP_FORWARD, 0, 0);
			io_rule.rule.dup_forward = 1;
			break;
			
		case SMUXCTL_SET_RXIF:
			smux_rule_add_action_set_rxif(&io_rule.rule, optarg);
			break;
			
		case SMUXCTL_SET_ETHTYPE:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_ETHTYPE, vals[0], 0);
			break;

		case SMUXCTL_SET_TPID:
			vals[0] = (uint16_t)(strtoul(optarg, &endptr, 0)&0xffff);	
			vals[0] = htons(vals[0]);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");		
				
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_TPID, vals[0], vals[1]);
			break;
			
		case SMUXCTL_SET_VID:
			vals[0] = (uint16_t)(strtoul(optarg, &endptr, 0)&0xffff);	
			vals[0] = htons(vals[0]) & VID_MASK;
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");		
				
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_VID, vals[0], vals[1]);
			break;

		case SMUXCTL_SET_PRIORITY:
			vals[0] = (uint16_t)(strtoul(optarg, &endptr, 0)&0xffff);	
			vals[0] = htons(vals[0] << PRIO_SHIFT) & PRIO_MASK;
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_PRIORITY, vals[0], vals[1]);
			break;

		case SMUXCTL_SET_CFI:
			vals[0] = (uint16_t)(strtoul(optarg, &endptr, 0)&0xffff);	
			vals[0] = htons(vals[0] << CFI_SHIFT) & CFI_MASK;
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_CFI, vals[0], vals[1]);
			break;
			
		case SMUXCTL_SET_SKB_MARK:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
				
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");	
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_SKB_MARK, vals[0], vals[1]);
			break;

#ifdef CONFIG_RTK_SKB_MARK2
		case SMUXCTL_SET_SKB_MARK2:
                        vals_64[0] = strtoull(optarg, &endptr, 0);
                        if (*endptr != '\0') // success
                                print_error(longopts,longindex,"invalid args");

                        strs[0] = get_arg(argc, argv);
                        if (!strs[0])
                                print_error(longopts,longindex,"require two arguments");

                        vals_64[1] = strtoull(strs[0], &endptr, 0);
                        if (*endptr != '\0') // success
                                print_error(longopts,longindex,"invalid number");
                        smux_rule_add_action_64(&io_rule.rule, SMUX_ACT_SET_SKB_MARK2, vals_64[0], vals_64[1]);
                        break;
#endif

		case SMUXCTL_SET_DSCP:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0')			
				print_error(longopts,longindex,"invalid number");
			vals[0] = (vals[0] << IP_DSCP_SHIFT) & IP_DSCP_MASK;
			
			vals[1] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0')
				print_error(longopts,longindex,"invalid number");
			vals[1] = htonl(vals[1] << IPV6_DSCP_SHIFT) & IPV6_DSCP_MASK;
				
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_DSCP, vals[0], vals[1]);
			break;

		case SMUXCTL_COPY_8021Q:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_COPY_8021Q, vals[0], vals[1]);
			break;

		case SMUXCTL_COPY_TPID:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_COPY_TPID, vals[0], vals[1]);
			break;

		case SMUXCTL_COPY_VID:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_COPY_VID, vals[0], vals[1]);
			break;

		case SMUXCTL_COPY_PRIORITY:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_COPY_PRIORITY, vals[0], vals[1]);
			break;

		case SMUXCTL_COPY_CFI:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_COPY_CFI, vals[0], vals[1]);
			break;

		case SMUXCTL_MAP_DSCP_TO_PRI:
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_MAP_DSCP_TO_PRIORITY, vals[0], 0);
			break;
		
		case SMUXCTL_RULE_ALIAS:
			strcpy(io_rule.rule.alias_name, optarg);	
			break;
			
		case SMUXCTL_RULE_PRIORITY:
			io_rule.rule.priority = strtoul(optarg, &endptr, 0);
			break;
			
		case SMUXCTL_RULE_APPEND:
			io_rule.method = SMUX_RULE_METHOD_APPEND;			
			set_cmd(SMUXCTL_CMD_RULE_ADD);			
			break;
		
		case SMUXCTL_RULE_INSERT:
			io_rule.method = SMUX_RULE_METHOD_INSERT;			
			set_cmd(SMUXCTL_CMD_RULE_ADD);			
			break;
			
		case SMUXCTL_RULE_INSERT_BEFORE:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			io_rule.method = SMUX_RULE_METHOD_INSERT_BEFORE;
			io_rule.id = vals[0]; 		
			set_cmd(SMUXCTL_CMD_RULE_ADD);			
			break;
			
		case SMUXCTL_RULE_INSERT_AFTER:
			vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			io_rule.method = SMUX_RULE_METHOD_INSERT_AFTER;
			io_rule.id = vals[0]; 		
			set_cmd(SMUXCTL_CMD_RULE_ADD);			
			break;		
			
		case SMUXCTL_RULE_REMOVE:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
			io_rule.method = SMUX_RULE_METHOD_DELETE;	
			io_rule.id = vals[0];		
			set_cmd(SMUXCTL_CMD_RULE_DELETE);			
			break;
			
		case SMUXCTL_RULE_REMOVE_ALIAS:
			io_rule.method = SMUX_RULE_METHOD_DELETE_ALIAS;	
			strcpy(io_rule.arg, optarg);	
			set_cmd(SMUXCTL_CMD_RULE_DELETE);			
			break;
			
		case SMUXCTL_RULE_REMOVE_ALL:
			io_rule.method = SMUX_RULE_METHOD_DELETE_ALL;			
			set_cmd(SMUXCTL_CMD_RULE_DELETE);			
			break;

		case SMUXCTL_RULE_RESETCOUNT:
			io_rule.method = SMUX_RULE_METHOD_RESET_COUNT;			
			set_cmd(SMUXCTL_CMD_RULE_RESET);			
			break;
		
		case SMUXCTL_IF_DELETE:
			set_cmd(SMUXCTL_CMD_IF_DELETE);
			if_virt_name = optarg;
			break;

		case SMUXCTL_IF_APPEND_RSMUX:
			//if_set_flags(SMUXCTL_IF_FLAGS_MODE_RG);
			if_flags |= SMUXCTL_IF_FLAGS_RSMUX;
			break;
			
		case SMUXCTL_IF_RX_MULTI:
			strs[0] = optarg;
			if (!strs[0]) {
				print_error(longopts,longindex,"require arguments");
			}
			if(smux_set_if_rx_multi(atoi(strs[0])))
			{
				print_error(longopts,longindex,"error arguments");
			}
			if_flags |= SMUXCTL_IF_FLAGS_EDIT;
			break;

		case SMUXCTL_IF_RX_POLICY:
			strs[0] = optarg;
			if (!strs[0]) {
				print_error(longopts,longindex,"require arguments");
			}
			if(smux_set_if_policy(SMUX_DIR_RX, strs[0]))
			{
				print_error(longopts,longindex,"error arguments");
			}
			if_flags |= SMUXCTL_IF_FLAGS_EDIT;
			break;

		case SMUXCTL_IF_TX_POLICY:
			strs[0] = optarg;
			if (!strs[0]) {
				print_error(longopts,longindex,"require arguments");
			}
			if(smux_set_if_policy(SMUX_DIR_TX, strs[0]))
			{
				print_error(longopts,longindex,"error arguments");
			}
			if_flags |= SMUXCTL_IF_FLAGS_EDIT;
			break;

		case SMUXCTL_IF_RXMC_POLICY:
			strs[0] = optarg;
			if (!strs[0]) {
				print_error(longopts,longindex,"require arguments");
			}
			if(smux_set_if_policy(SMUX_DIR_RXMC, strs[0]))
			{
				print_error(longopts,longindex,"error arguments");
			}
			if_flags |= SMUXCTL_IF_FLAGS_EDIT;
			break;
			
		case SMUXCTL_IF_CARRIER_MODE:
			strs[0] = optarg;
			if (!strs[0]) {
				print_error(longopts,longindex,"require arguments");
			}
			if(smux_set_if_carrier(strs[0]))
			{
				print_error(longopts,longindex,"error arguments");
			}
			if_flags |= SMUXCTL_IF_FLAGS_EDIT;
			break;
			
		case SMUXCTL_SHOW_TABLE:
		    set_cmd(SMUXCTL_CMD_SHOW_TABLE);
			break;
		
		case SMUXCTL_RUN_TEST:
			set_cmd(SMUXCTL_CMD_TEST);
			break;
			
		case SMUXCTL_DEFAULT_TPID:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
		    io_rule.table.default_tpid = vals[0];
		    io_rule.method = SMUX_DEFAULT_METHOD_TPID;	
		    set_cmd(SMUXCTL_CMD_SET_DEFAULT);
			break;		
			
		case SMUXCTL_DEFAULT_VID:
		    vals[0] = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0'){ // success 				
				print_error(longopts,longindex,"invalid number");			
			    break;
		    }
		    io_rule.table.default_vlan_tci = vals[0];
		    io_rule.method = SMUX_DEFAULT_METHOD_VID;	
		    set_cmd(SMUXCTL_CMD_SET_DEFAULT);
			break;

		case SMUXCTL_DSCP_MAPPING_ENABLE:			
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			io_rule.qos.dscp_mapping_enable = vals[0];
			io_rule.qos.vlan_idx = vals[1];
			io_rule.method = SMUX_QOS_METHOD_DSCP_MAPPING_ENABLE;	
			set_cmd(SMUXCTL_CMD_SET_QOS);
			break;

		case SMUXCTL_DSCP_MAPPING_SET:			
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			io_rule.qos.dscp= vals[0];
			io_rule.qos.priority= vals[1];
			io_rule.method = SMUX_QOS_METHOD_DSCP_MAPPING_SET;	
			set_cmd(SMUXCTL_CMD_SET_QOS);
			break;

		case SMUXCTL_MAP_DSCP_TO_PRI_SET:			
			vals[0] = strtoul(optarg, &endptr, 0);			
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid args");
			strs[0] = get_arg(argc, argv);
			if (!strs[0])
				print_error(longopts,longindex,"require two arguments");
			
			vals[1] = strtoul(strs[0], &endptr, 0);
			if (*endptr != '\0') // success 				
				print_error(longopts,longindex,"invalid number");
			
			io_rule.qos.dscp= vals[0];
			io_rule.qos.priority= vals[1];
			io_rule.method = SMUX_QOS_METHOD_DSCP_MAPPING_CHANGE_DEFAULT;	
			set_cmd(SMUXCTL_CMD_SET_QOS);
			break;

		case SMUXCTL_DSCP_MAPPING_DEFAULT:
			io_rule.method = SMUX_QOS_METHOD_DSCP_MAPPING_DEFAULT;	
			set_cmd(SMUXCTL_CMD_SET_QOS);
			break;

		case SMUXCTL_TARGET_SET:
			if(!strcmp(optarg, "ACCEPT"))
				vals[0] = SMUX_TARGET_ACCEPT;
			else if(!strcmp(optarg, "DROP"))
				vals[0] = SMUX_TARGET_DROP;
			else if(!strcmp(optarg, "CONTINUE"))
				vals[0] = SMUX_TARGET_CONTINUE;
			else
			{
				print_error(longopts,longindex,"invalid args");
				break;
			}
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_SET_TARGET, vals[0], 0);
			break;
		
		case SMUXCTL_REVERT_VLAN:
			smux_rule_add_action(&io_rule.rule, SMUX_ACT_REVERT_VLAN, 1, 0);
			break;		
		
		case SMUXCTL_HELP:	
		    print_usage();	
			break;		
		default:
			//printf("c=%d optind=%d optopt=%d optarg=%s index=%d\n",c,optind,optopt,optarg?optarg:"<none>",longindex);
			//if (optind < argc) 
			//	printf("next=%s\n", argv[optind]);
			break;
		}
		
	}
	
	if(!command && if_flags > 0)
		command = SMUXCTL_CMD_SET_IF_PARM;

	if (!command) 
	{
		printf("No command !\n");
		print_usage();
	}
		
	if(command!=SMUXCTL_CMD_IF_CREATE && 
		command!=SMUXCTL_CMD_IF_DELETE && 
		command!=SMUXCTL_CMD_SET_IF_PARM && 
		command!=SMUXCTL_CMD_TEST && 
		command!=SMUXCTL_CMD_SHOW_TABLE &&
		command!=SMUXCTL_CMD_RULE_RESET
	){	
	    if(!check_mantatory_value(if_real_name,&io_rule))
	        print_usage();	
	}        
	
	//printf("%s(%d): command=%d\n",__func__,__LINE__,command);
		
	switch (command) {	
	case SMUXCTL_CMD_IF_CREATE:
		ret = smux_interface_create(if_real_name, if_virt_name, if_flags);
		break;
	case SMUXCTL_CMD_IF_DELETE:		
		ret = smux_interface_delete(if_virt_name);
		break;
	case SMUXCTL_CMD_RULE_ADD:
		ret = smux_rule_insert(if_real_name, &io_rule);
		/*if (ret == 0) 
			printf("New ID = %d\n", io_rule.id);*/
		break;
	case SMUXCTL_CMD_RULE_DELETE:
		ret = smux_rule_delete(if_real_name, &io_rule);
		break;
	case SMUXCTL_CMD_RULE_RESET:
		ret = smux_rule_reset(if_real_name, &io_rule);
		break;
	case SMUXCTL_CMD_SHOW_TABLE:
		ret = smux_show_table(if_real_name, &io_rule);	
		break;
	case SMUXCTL_CMD_SET_DEFAULT:
		ret = smux_set_default(if_real_name, &io_rule);	
		break;	
	case SMUXCTL_CMD_SET_QOS:
		ret = smux_set_qos(if_real_name, &io_rule);	
		break;
	case SMUXCTL_CMD_SET_IF_PARM:
		ret = smux_interface_parameter_set(if_real_name, if_flags);
		break;
#if RUN_TESTS
	case SMUXCTL_CMD_TEST:
		ret = run_test_cases();
		break;	
#endif 
	}
	
	if(ret != 0) printf("smuxctl failed(ret=%d) return errno(%d) %s \n", ret, errno, strerror(errno));
	//if_fini();
	smux_fini();
	
	return ret;
}
