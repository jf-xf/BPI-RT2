#ifndef _MAP_BACKHAUL_H_
#define _MAP_BACKHAUL_H_

// New method is STP based, will not start eth_monitor thread, may need to change BACKHAUL_REAL_TIME_SWITCH implementation
#define EASYMESH_RENEW_IP

#ifdef EASYMESH_ETH_MONITOR
#define BACKHAUL_REAL_TIME_SWITCH
#endif

#if defined(BACKHAUL_REAL_TIME_SWITCH) || defined(EASYMESH_RENEW_IP)
#define BACKHAUL_LINK_INFO "/tmp/map_backhaul_uplink_info"
#endif

#endif
