#ifndef _RE_RECYCLESKB_H_
#define _RE_RECYCLESKB_H_

#include <linux/skbuff.h>
#include <linux/interrupt.h>
#ifdef CONFIG_RTK_PTOOL_CPU_PERF
#include <linux/ptool.h>		//performance
#endif
#include <linux/kernel.h>		//dump_stack
#include <linux/mm.h>			//get total memory size
#include <linux/vmstat.h>
#include <linux/highmem.h>

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include <ca_ni.h>
#include <ca_ext.h>

#define MBUF_LEN 	CA_NI_SKB_ALLOC_DATA_SIZE
#else
#include "../nicDriver/re8686_rtl9607c.h"

#define MBUF_LEN        SKB_BUF_SIZE
#endif

#if defined(RTL_PRIV_DATA_SIZE) && RTL_PRIV_DATA_SIZE > 0
//define reserve 0, because alloc_skb will append RTL_PRIV_DATA_SIZE, so share it
#define RTL_RESERVED_HEADER_SIZE	0 
#else
#define RTL_RESERVED_HEADER_SIZE	128
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN)
#else
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+16)
#endif

#if defined(CONFIG_CMCC)
#define MAX_PRE_ALLOC_RX_SKB	20000
#else
#define MAX_PRE_ALLOC_RX_SKB	5000
#endif

#define MAX_ETH_SKB_NUM	(MAX_PRE_ALLOC_RX_SKB + 600)
#define MAX_CRITICAL_ETH_SKB_NUM 256

#define ETH_RECYCLE_SKB_PROC	1

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
#define RTL_SKB_CB_SECTION_MAX		48
#define RTL_SKB_CB_SECTION_LOW		32
#define RTL_SKB_CB_SECTION_HIGH		40
#define RTL_SKB_CB_SECTION_ENTER	1
#define RTL_SKB_CB_SECTION_LEAVE	0
#endif

enum {
	RTL_RECYCLE_SWITCH_ON=0x0,
	RTL_RECYCLE_SWITCH_OFF=0x1,
};

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
enum {
	RTL_RING_BUF_RECYCLE_NORMAL=0x0,
	RTL_RING_BUF_RECYCLE_EMPTY,
	RTL_RING_BUF_RECYCLE_FULL,
};

struct ring_buffer_recycle_t
{
	unsigned int rd_idx;
	unsigned int wr_idx;
	unsigned int state;	//0: normal, 1:empty, 2:full
	unsigned int ring_size_mask;

	struct sk_buff **ring_buffer_recycle_array;
	spinlock_t	lock;
};
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
struct straight_recycle_t
{
	unsigned int idx;
	struct sk_buff **array;
	spinlock_t	lock;
};
#endif
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
typedef struct rtl_recycle_skb_debug_s
{
	struct sk_buff *check_skb;
	int cb;
	
	int skb_pointer_off;	//-1 to disable check
	int headroom;			//-1 to disable check
	int dump_stack_en;		//-1 to disable check
}rtl_recycle_skb_debug_t;
#endif

#if defined(CONFIG_PREEMPT)
#define RECYCLE_SPINLOCK(x) spin_lock_bh(x)
#define RECYCLE_SPINUNLOCK(x) spin_unlock_bh(x)
#else
#define RECYCLE_SPINLOCK(x) spin_lock(x)
#define RECYCLE_SPINUNLOCK(x) spin_unlock(x)
#endif

int init_recycle_eth_skb_buf(unsigned int ring_size, unsigned int max_size, int forced_ring_size, int forced_new_pool);
void deinit_recycle_eth_skb_buf(int pool);
struct sk_buff *dev_alloc_skb_recy_eth(unsigned int size, int pool);
#define dev_alloc_skb_priv_eth dev_alloc_skb_recy_eth
#define dev_alloc_critical_skb_priv_eth re8670_getAlloc
struct sk_buff *recycle_skb_swap(struct sk_buff *skb);
unsigned int rtl_free_eth_skb_num(int pool);
unsigned int rtl_alloc_eth_skb_num(int pool);

static inline void __init_skb_recy_eth(struct sk_buff *skb)
{
	skb->data = skb->head+RTL_RESERVED_HEADER_SIZE;		//reserve header room
	skb_reset_tail_pointer(skb);
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	/*Clear identicated amsdu offload cb while refill skb.
	 *_SKB_CB_ETH_AGG=32 defined @ g6_wifi_driver/include/xmit_osdep.h
	 */
	skb->cb[32] = 0x0;
#endif
#ifdef CONFIG_SKB_EXTENSIONS
	/* only useable after checking ->active_extensions != 0 */
	skb->extensions=NULL;
	skb->active_extensions=0;
#endif

#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	skb->skb_bak = skb;
	skb->head_bak = skb->head;
#endif
}

static inline void __init_skb_recy_eth_debug(struct sk_buff *skb)
{
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	rtl_recycle_skb_debug_t rtl_skb_debug_parameter;

	skb->double_check_free=0;

	rtl_skb_debug_parameter.cb=rtl_recycle_skb_cb_value;
	rtl_skb_debug_parameter.check_skb=skb;
	rtl_skb_debug_parameter.skb_pointer_off=-1;
	rtl_skb_debug_parameter.headroom=-1;
	rtl_skb_debug_parameter.dump_stack_en=1;
	rtl_recycle_skb_cb_section_enter(&rtl_skb_debug_parameter);
#endif
}

static inline void __recycle_skb_copy(struct sk_buff *skb, struct sk_buff *new_skb)
{
	unsigned char *head/*,*data*/;//printk("%s %d recyclable swap!!qlen=%d,skb=%p,new_skb=%p\n",__FUNCTION__,__LINE__,skb_queue_len(&recycle_skb_queue),skb,new_skb);
	sk_buff_data_t end/*,tail*/;
	unsigned int truesize;
	bool pfmemalloc;
#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	if(skb->fcIngressData.doLearning)
		memcpy(&new_skb->fcIngressData, &skb->fcIngressData, sizeof(rtk_fc_ingress_data_t));
#endif
	//printk("skb->truesize is %d, new_skb->truesize is %d\n",skb->truesize,new_skb->truesize);
	head=new_skb->head;
	end=new_skb->end;
	truesize=new_skb->truesize;
	pfmemalloc=new_skb->pfmemalloc;

	new_skb->head=skb->head;
	new_skb->data=skb->data;
	new_skb->tail=skb->tail;
	new_skb->end=skb->end;
	new_skb->truesize=skb->truesize;
	new_skb->pfmemalloc=skb->pfmemalloc;
	new_skb->len=skb->len;
	new_skb->dev=skb->dev;
#ifdef CONFIG_RTK_CONTROL_PACKET_PROTECTION
	new_skb->priority=skb->priority;
#endif
	new_skb->mark=skb->mark;
#ifdef CONFIG_RTK_SKB_MARK2
	new_skb->mark2=skb->mark2;
#endif
#ifdef CONFIG_RTL8192CD
	new_skb->cb[0]=skb->cb[0];
#endif
#ifdef CONFIG_SKB_EXTENSIONS
	/* only useable after checking ->active_extensions != 0 */
	new_skb->extensions=skb->extensions;
	new_skb->active_extensions=skb->active_extensions;
#endif

	skb->head=head;
	skb->end=end;
	skb->truesize=truesize;
	skb->pfmemalloc=pfmemalloc;
	skb->data_len=0;		//for jumbo packet should clear it here
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	skb->skb_bak=skb;
	skb->head_bak=head;
#endif
}

#if defined(CONFIG_LUNA_G3_SERIES)
#define __recycle_rtl_dev_alloc_skb(size)		__netdev_alloc_skb(NULL, size, GFP_ATOMIC|GFP_DMA);
#else
#define __recycle_rtl_dev_alloc_skb(size)		__netdev_alloc_skb(NULL, size+RTL_RESERVED_HEADER_SIZE, GFP_ATOMIC|GFP_DMA);
#endif

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
int rtl_recycle_skb_cb_section_register(void);
int rtl_recycle_skb_cb_section_unregister(unsigned int cb);
int rtl_recycle_skb_cb_section_enter(rtl_recycle_skb_debug_t *rtl_skb_debug);
int rtl_recycle_skb_cb_section_check(rtl_recycle_skb_debug_t *rtl_skb_debug);
int rtl_recycle_skb_cb_section_leave(rtl_recycle_skb_debug_t *rtl_skb_debug);
#endif

#endif /*_RE_RECYCLESKB_H_*/
