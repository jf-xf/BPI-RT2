#ifndef _IP_SPEEDUP_H
#define _IP_SPEEDUP_H

#include <linux/ioctl.h>

#define DEV_SPEEDUP_NAME "host_speed_up"
#define DEV_SPEEDUP_PATH "/dev/"DEV_SPEEDUP_NAME

#define SPEEDUP_NODE_MAGIC 0x11
#define SIOCSPEEDTEST	_IOWR(SPEEDUP_NODE_MAGIC, 0, struct speedtest_arg)

#define IP6_ADDR_LEN					16
#define IP_ADDR_LEN					4

struct speedtest_arg{
	unsigned char cmd;
	unsigned int ip_familly;
	unsigned char sip[IP6_ADDR_LEN];
	unsigned char dip[IP6_ADDR_LEN];
	unsigned short sport;
	unsigned short dport;
};

#endif /*_IP_SPEEDUP_H*/

