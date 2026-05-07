
#ifndef _RTK_IGMP_TINYPS_MCFWD_H
#define _RTK_IGMP_TINYPS_MCFWD_H

#include <rtk_igmp_struct.h>

rtk_igmp_nf_ret_e_t rtk_igmp_data_process(struct sk_buff *skb, rtk_igmp_pktHdr_t *pPkthdr, const struct net_device *SrcDev,const struct net_device *DstDev);


#endif

