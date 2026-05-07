/*	
 *	re_privskb.h
*/

#ifndef _RE_PRIVSKB_H_
#define _RE_PRIVSKB_H_
#include <linux/skbuff.h>
#ifdef CONFIG_RTL8686
#ifdef CONFIG_ARCH_LUNA_SLAVE
#include "re8686_dsp.h"
#else
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES) && !defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#include "re8686_rtl9602c.h"
#endif
#endif
//#elif defined(CONFIG_RTL9607C) || defined(CONFIG_RTL9603CVD)
#elif defined(CONFIG_RTL9607C) || defined(CONFIG_RTL9603CVD)
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL9603CVD_SERIES) && !defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#include "re8686_rtl9607c.h"
#endif
#else
#include "re867x.h"
#endif

#define MBUF_LEN        SKB_BUF_SIZE

#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+16)
//used to make the start of shared_info is cache line aligment.
#define SHARED_INFO_PAD		SMP_CACHE_BYTES

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
//#ifdef DELAY_REFILL_ETH_RX_BUF

#define MAX_PRE_ALLOC_RX_SKB	6500	
#if (CONFIG_RTL865X_ETH_PRIV_SKB_NUM == 0)
#define MAX_ETH_SKB_NUM	(RE8670_RX_RING_SIZE + MAX_PRE_ALLOC_RX_SKB + 600)
#else
#define MAX_ETH_SKB_NUM	CONFIG_RTL865X_ETH_PRIV_SKB_NUM
#endif

#define MAX_CRITICAL_ETH_SKB_NUM 256
#define MAX_ETH_DYNAMIC_SKB_NUM 3000

#define ETH_PRIV_SKB_PROC	1
#endif  //end CONFIG_RTL865X_ETH_PRIV_SKB

#if 0 /* 2012-2-29 krammer add */
#ifdef RTK_QUE
struct ring_que {
	int qlen;
	int qmax;	
	int head;
	int tail;
	struct sk_buff *ring[MAX_PRE_ALLOC_RX_SKB+1];
};
 static struct ring_que rx_skb_queue;
#else
 static struct sk_buff_head rx_skb_queue; 
#endif  //end RTK_QUE
#endif /* 2012-2-29 krammer add */


void init_priv_eth_skb_buf(void);
struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size);
struct sk_buff *dev_alloc_critical_skb_priv_eth(unsigned int size);

#endif /*_RE_PRIVSKB_H_*/
