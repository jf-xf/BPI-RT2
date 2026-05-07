/* 
 * In compile phase,
 * gen_rtk_script_env.sh will auto parser this file to create script reference file
 * please follow specific naming rule
 **/

// ip route tables
// naming rule should have "RT_TBL" suffix
#define SKB_MARK_RT_TBL		32
#define STATIC_RT_TBL		257
#define PD_RT_TBL		258
#define LAN_RT_TBL		260

// ip rule prioritys
// naming rule should have "RULE_PRI" prefix
#define RULE_PRI_PHY_PORT_OIF_RT	25000
#define RULE_PRI_STATIC_RT		27900
#define RULE_PRI_DHCP6S_PD_RT		27945
#define RULE_PRI_WAN_NPT_PD_RT		27950
#define RULE_PRI_LAN_RT			28000
#define RULE_PRI_SRC_RT			29000
#define RULE_PRI_DOMAIN_RT		29001
#define RULE_PRI_SKB_MARK_RT		29100
#define RULE_PRI_OIF_RT			29101
#define RULE_PRI_PD_RT			29200
#define RULE_PRI_VPN_RT			29300
#define RULE_PRI_SKB_MARK_PROHIBIT	30000
