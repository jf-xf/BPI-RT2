#ifndef RTK_GENERIC_NETLINK_H
#define RTK_GENERIC_NETLINK_H

/*
 *  <------- NLA_HDRLEN ------> <-- NLA_ALIGN(payload)-->
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 * |        Header       | Pad |     Payload       | Pad |
 * |   (struct nlattr)   | ing |                   | ing |
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 *  <-------------- nlattr->nla_len -------------->
 */

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/genetlink.h>

//Family name
#define RTL_GENL_WLAN_EVENT             "WIFI6_INDICATE"
//Group
#define RTL_GENL_WLAN_EVENT_GROUP       "WIFI6_EVENT_GRP"

//Attr
enum rtk_wlan_event_nl_attr {
  RTL_WLAN_INDICATE_ATTR_UNSPEC,
  RTL_WLAN_INDICATE_ATTR_MSG,
  __RTL_WLAN_INDICATE_ATTR_MAX,
};
#define RTL_WLAN_INDICATE_ATTR_MAX (__RTL_WLAN_INDICATE_ATTR_MAX - 1)

//Command
enum rtk_wlan_event_nl_commands {
  RTL_WLAN_INDICATE_CMD_UNSPEC,
  RTL_WLAN_INDICATE_CMD_EVENT,
  __RTL_WLAN_INDICATE_CMD_MAX,
};
#define RTL_WLAN_INDICATE_CMD_MAX (__RTL_WLAN_INDICATE_CMD_MAX - 1)

#define GENLMSG_DATA(glh)       ((void*)(((char*)glh) + GENL_HDRLEN))
#define NLA_DATA(nla)           ((void *)((char*)(nla) + NLA_HDRLEN))
#define NLA_NEXT(nla,len)       ((len) -= NLA_ALIGN((nla)->nla_len), \
                                      (struct nlattr*)(((char*)(nla)) + NLA_ALIGN((nla)->nla_len)))
#define NLA_OK(nla,len)         ((len) >= (int)sizeof(struct nlattr) && \
                                    (nla)->nla_len >= sizeof(struct nlattr) && \
                                    (nla)->nla_len <= (len))

int rtk_nla_total_size(int payload);
int rtk_genlmsg_get_family_id(int sockfd, const char *family_name);
int rtk_genlmsg_get_group_id(int sockfd, const char *family_name, const char *grp_name);
int rtk_genlmsg_send(int sockfd, unsigned short nlmsg_type, unsigned int nlmsg_pid,
        unsigned char genl_cmd, unsigned char genl_version, unsigned int family_hdr,
        unsigned short nla_type, const void *nla_data, unsigned int nla_len);

#define GENLMSG_TOTAL_LEN(family_hdr, len) NLMSG_SPACE(GENL_HDRLEN+(family_hdr)+rtk_nla_total_size(len))

#endif //RTK_GENERIC_NETLINK_H