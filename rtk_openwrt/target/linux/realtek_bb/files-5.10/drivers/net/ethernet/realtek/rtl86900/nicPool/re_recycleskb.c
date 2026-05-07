#ifdef CONFIG_RTL_ETH_RECYCLED_SKB

#include "re_recycleconf.h"
#include "re_recycleskb.h"

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

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
static __cacheline_aligned_in_smp struct ring_buffer_recycle_t rbr;
static __cacheline_aligned_in_smp struct ring_buffer_recycle_t rbr_critical;
#endif
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
static __cacheline_aligned_in_smp struct sk_buff_head recycle_skb_queue[1<<RTL_POOL_NUM_SHIFT_BITS];
/*static __cacheline_aligned_in_smp struct sk_buff_head recycle_critical_skb_queue[1<<RTL_POOL_NUM_SHIFT_BITS];*/
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
static __cacheline_aligned_in_smp struct straight_recycle_t sra;
static __cacheline_aligned_in_smp struct straight_recycle_t sra_critical;
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
static __cacheline_aligned_in_smp spinlock_t llll_read_lock[1<<RTL_POOL_NUM_SHIFT_BITS];
static __cacheline_aligned_in_smp struct llist_head llll_skb_head[1<<RTL_POOL_NUM_SHIFT_BITS];
static __cacheline_aligned_in_smp atomic_t llll_num[1<<RTL_POOL_NUM_SHIFT_BITS]; 

/*static __cacheline_aligned_in_smp spinlock_t llll_critical_read_lock;
static __cacheline_aligned_in_smp struct llist_head llll_critical_skb_head;
static __cacheline_aligned_in_smp atomic_t llll_critical_num;*/
#endif

static unsigned int eth_skb_total_num[1<<RTL_POOL_NUM_SHIFT_BITS];
static unsigned int eth_skb_free_num[1<<RTL_POOL_NUM_SHIFT_BITS];
static unsigned int eth_skb_max_size[1<<RTL_POOL_NUM_SHIFT_BITS]={0};
static unsigned int eth_skb_pool[1<<RTL_POOL_NUM_SHIFT_BITS]={0};
unsigned int switch_off_recycle_skb=RTL_RECYCLE_SWITCH_ON;
EXPORT_SYMBOL(switch_off_recycle_skb);
//int lowest_eth_skb_free_num=MAX_ETH_SKB_NUM;

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
static int rtl_recycle_skb_cb_value=-1;
static DEFINE_SPINLOCK(rtl_recycle_skb_cb_lock);
static unsigned char rtl_recycle_skb_cb_check[RTL_SKB_CB_SECTION_MAX]={0};
#endif

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(void)
{
	if(rbr.state==RTL_RING_BUF_RECYCLE_FULL)return rbr.ring_size_mask+1;
	else if(rbr.state==RTL_RING_BUF_RECYCLE_EMPTY)return 0;
	else{
		if(rbr.wr_idx>rbr.rd_idx)return rbr.wr_idx-rbr.rd_idx;
		else return rbr.ring_size_mask+1-rbr.rd_idx+rbr.wr_idx;
	}
}

unsigned int rtl_free_eth_critical_skb_num(void)
{
	if(rbr_critical.state==RTL_RING_BUF_RECYCLE_FULL)return rbr_critical.ring_size_mask+1;
	else if(rbr_critical.state==RTL_RING_BUF_RECYCLE_EMPTY)return 0;
	else{
		if(rbr_critical.wr_idx>rbr_critical.rd_idx)return rbr_critical.wr_idx-rbr_critical.rd_idx;
		else return rbr_critical.ring_size_mask+1-rbr_critical.rd_idx+rbr_critical.wr_idx;
	}
}

unsigned int rtl_alloc_eth_skb_num()
{
	if(rbr.state==RTL_RING_BUF_RECYCLE_FULL)return 0;
	else if(rbr.state==RTL_RING_BUF_RECYCLE_EMPTY)return rbr.ring_size_mask+1;
	else{
		if(rbr.wr_idx>rbr.rd_idx)return rbr.ring_size_mask+1-rbr.wr_idx+rbr.rd_idx;
		else return rbr.rd_idx-rbr.wr_idx;
	}
}

unsigned int rtl_alloc_eth_critical_skb_num(void)
{
	if(rbr_critical.state==RTL_RING_BUF_RECYCLE_FULL)return 0;
	else if(rbr_critical.state==RTL_RING_BUF_RECYCLE_EMPTY)return rbr_critical.ring_size_mask+1;
	else{
		if(rbr_critical.wr_idx>rbr_critical.rd_idx)return rbr_critical.ring_size_mask+1-rbr_critical.wr_idx+rbr_critical.rd_idx;
		else return rbr_critical.rd_idx-rbr_critical.wr_idx;
	}
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&rbr.lock);
	//printk("[ENQ]rbr.wr_idx=%d, rd_idx=%d, state=%d\n",rbr.wr_idx,rbr.rd_idx,rbr.state);
	if(likely(rbr.state!=RTL_RING_BUF_RECYCLE_FULL)){
		rbr.ring_buffer_recycle_array[rbr.wr_idx]=newsk;
		rbr.wr_idx = (rbr.wr_idx+1)&rbr.ring_size_mask;		
		smp_mb();
		if(unlikely(rbr.wr_idx==rbr.rd_idx))rbr.state=RTL_RING_BUF_RECYCLE_FULL;
		else rbr.state=RTL_RING_BUF_RECYCLE_NORMAL;
		//printk("update wr_idx to %d!! state become to %d\n",rbr.wr_idx,rbr.state);
	}else{
		printk("abnormal!! free the skb as dynamically one..\n");
		newsk->recyclable=0;
		dev_kfree_skb_any(newsk);
	}
	RECYCLE_SPINUNLOCK(&rbr.lock);
}

__always_inline
void rtl_eth_skb_queue_head_critical(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&rbr_critical.lock);
	//printk("[ENQ]rbr_critical.wr_idx=%d, rd_idx=%d, state=%d\n",rbr_critical.wr_idx,rbr_critical.rd_idx,rbr_critical.state);
	if(likely(rbr_critical.state!=RTL_RING_BUF_RECYCLE_FULL)){
		rbr_critical.ring_buffer_recycle_array[rbr_critical.wr_idx]=newsk;
		rbr_critical.wr_idx = (rbr_critical.wr_idx+1)&rbr_critical.ring_size_mask;		
		smp_mb();
		if(unlikely(rbr_critical.wr_idx==rbr_critical.rd_idx))rbr.state=RTL_RING_BUF_RECYCLE_FULL;
		else rbr_critical.state=RTL_RING_BUF_RECYCLE_NORMAL;
		//printk("update wr_idx to %d!! state become to %d\n",rbr_critical.wr_idx,rbr_critical.state);
	}else{
		printk("abnormal!! free the skb as dynamically one..\n");
		newsk->recyclable=0;
		dev_kfree_skb_any(newsk);
	}
	RECYCLE_SPINUNLOCK(&rbr_critical.lock);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&rbr.lock);
	//printk("[DEQ]rbr.wr_idx=%d, rd_idx=%d, state=%d\n",rbr.wr_idx,rbr.rd_idx,rbr.state);
	if(likely(rbr.state!=RTL_RING_BUF_RECYCLE_EMPTY)){
		skb=rbr.ring_buffer_recycle_array[rbr.rd_idx];
		rbr.rd_idx = (rbr.rd_idx+1)&rbr.ring_size_mask;
		smp_mb();
		if(unlikely(rbr.wr_idx==rbr.rd_idx))rbr.state=RTL_RING_BUF_RECYCLE_EMPTY;
		else rbr.state=RTL_RING_BUF_RECYCLE_NORMAL;
		//printk("update rd_idx to %d!! state become to %d\n",rbr.rd_idx,rbr.state);
	}
	RECYCLE_SPINUNLOCK(&rbr.lock);
	return skb;
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue_critical(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&rbr_critical.lock);
	//printk("[DEQ]rbr_critical.wr_idx=%d, rd_idx=%d, state=%d\n",rbr_critical.wr_idx,rbr_critical.rd_idx,rbr_critical.state);
	if(likely(rbr_critical.state!=RTL_RING_BUF_RECYCLE_EMPTY)){
		skb=rbr_critical.ring_buffer_recycle_array[rbr_critical.rd_idx];
		rbr_critical.rd_idx = (rbr_critical.rd_idx+1)&rbr_critical.ring_size_mask;
		smp_mb();
		if(unlikely(rbr_critical.wr_idx==rbr_critical.rd_idx))rbr_critical.state=RTL_RING_BUF_RECYCLE_EMPTY;
		else rbr_critical.state=RTL_RING_BUF_RECYCLE_NORMAL;
		//printk("update rd_idx to %d!! state become to %d\n",rbr_critical.rd_idx,rbr_critical.state);
	}
	RECYCLE_SPINUNLOCK(&rbr_critical.lock);
	return skb;
}

#endif
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(int pool)
{
	return recycle_skb_queue[pool].qlen;
}

/*unsigned int rtl_free_eth_critical_skb_num(int pool)
{
	return recycle_critical_skb_queue.qlen;
}*/

unsigned int rtl_alloc_eth_skb_num(int pool)
{
	return eth_skb_free_num[pool] - recycle_skb_queue[pool].qlen;
}

/*unsigned int rtl_alloc_eth_critical_skb_num(int pool)
{
	return MAX_CRITICAL_ETH_SKB_NUM - recycle_critical_skb_queue.qlen;
}*/

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk, int pool)
{
	RECYCLE_SPINLOCK(&recycle_skb_queue[pool].lock);
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	__skb_queue_tail(&recycle_skb_queue[pool], newsk);
#else
	__skb_queue_head(&recycle_skb_queue[pool], newsk);
#endif
	RECYCLE_SPINUNLOCK(&recycle_skb_queue[pool].lock);
}

/*__always_inline
void rtl_eth_skb_queue_head_critical(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&recycle_critical_skb_queue.lock);
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	__skb_queue_tail(&recycle_critical_skb_queue, newsk);
#else
	__skb_queue_head(&recycle_critical_skb_queue, newsk);
#endif
	RECYCLE_SPINUNLOCK(&recycle_critical_skb_queue.lock);
}*/

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(int pool)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&recycle_skb_queue[pool].lock);
	skb=__skb_dequeue(&recycle_skb_queue[pool]);
	RECYCLE_SPINUNLOCK(&recycle_skb_queue[pool].lock);
	return skb;
}

/*__always_inline
struct sk_buff *rtl_eth_skb_dequeue_critical(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&recycle_critical_skb_queue.lock);
	skb=__skb_dequeue(&recycle_critical_skb_queue);
	RECYCLE_SPINUNLOCK(&recycle_critical_skb_queue.lock);
	return skb;
}*/
#endif
#if defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(void)
{
	return recycle_skb_queue.qlen;
}

unsigned int rtl_free_eth_critical_skb_num(void)
{
	return recycle_critical_skb_queue.qlen;
}

unsigned int rtl_alloc_eth_skb_num(void)
{
	return eth_skb_free_num - recycle_skb_queue.qlen;
}

unsigned int rtl_alloc_eth_critical_skb_num(void)
{
	return MAX_CRITICAL_ETH_SKB_NUM - recycle_critical_skb_queue.qlen;
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&recycle_skb_queue.lock);
	newsk->next = recycle_skb_queue.next;
	newsk->prev = (struct sk_buff *)&recycle_skb_queue;
	recycle_skb_queue.next = newsk;
	recycle_skb_queue.qlen++;
	RECYCLE_SPINUNLOCK(&recycle_skb_queue.lock);
}

__always_inline
void rtl_eth_skb_queue_head_critical(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&recycle_critical_skb_queue.lock);
	newsk->next = recycle_critical_skb_queue.next;
	newsk->prev = (struct sk_buff *)&recycle_critical_skb_queue;
	recycle_critical_skb_queue.next = newsk;
	recycle_critical_skb_queue.qlen++;
	RECYCLE_SPINUNLOCK(&recycle_critical_skb_queue.lock);
}

__always_inline
void __recycle_skb_peek(struct sk_buff *skb, struct sk_buff_head *list)
{
	struct sk_buff *next,*prev;
	list->qlen--;
	next=skb->next;
	prev=skb->prev;
	next->prev=prev;
	prev->next=next;
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&recycle_skb_queue.lock);
	skb = recycle_skb_queue.next;
	if(likely(skb!=(struct sk_buff *)&recycle_skb_queue))
		__recycle_skb_peek(skb, &recycle_skb_queue);
	else
		skb = NULL;
	RECYCLE_SPINUNLOCK(&recycle_skb_queue.lock);
	return skb;
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue_critical(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&recycle_critical_skb_queue.lock);
	skb = recycle_critical_skb_queue.next;
	if(likely(skb!=(struct sk_buff *)&recycle_critical_skb_queue))
		__recycle_skb_peek(skb, &recycle_critical_skb_queue);
	else
		skb = NULL;
	RECYCLE_SPINUNLOCK(&recycle_critical_skb_queue.lock);
	return skb;
}
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(void)
{
	return sra.idx;
}

unsigned int rtl_free_eth_critical_skb_num(void)
{
	return sra_critical.idx;
}

unsigned int rtl_alloc_eth_skb_num(void)
{
	return eth_skb_free_num - sra.idx;
}

unsigned int rtl_alloc_eth_critical_skb_num(void)
{
	return MAX_CRITICAL_ETH_SKB_NUM - sra_critical.idx;
}

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&sra.lock);
	sra.array[sra.idx]=newsk;
	sra.idx++;
	RECYCLE_SPINUNLOCK(&sra.lock);
}

__always_inline
void rtl_eth_skb_queue_head_critical(struct sk_buff *newsk)
{
	RECYCLE_SPINLOCK(&sra_critical.lock);
	sra_critical.array[sra_critical.idx]=newsk;
	sra_critical.idx++;
	RECYCLE_SPINUNLOCK(&sra_critical.lock);
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&sra.lock);
	if(likely(sra.idx)){
		sra.idx--;
		skb=sra.array[sra.idx];
	}
	RECYCLE_SPINUNLOCK(&sra.lock);
	return skb;
}

__always_inline
struct sk_buff *rtl_eth_skb_dequeue_critical(void)
{
	struct sk_buff *skb=NULL;
	RECYCLE_SPINLOCK(&sra_critical.lock);
	if(likely(sra_critical.idx)){
		sra_critical.idx--;
		skb=sra_critical.array[sra_critical.idx];
	}
	RECYCLE_SPINUNLOCK(&sra_critical.lock);
	return skb;
}
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
unsigned int rtl_free_eth_skb_num(int pool)
{
	return atomic_read(&llll_num[pool]);
}

/*unsigned int rtl_free_eth_critical_skb_num(void)
{
	return atomic_read(&llll_critical_num);
}*/

unsigned int rtl_alloc_eth_skb_num(int pool)
{
	return eth_skb_free_num[pool] - atomic_read(&llll_num[pool]);
}

/*unsigned int rtl_alloc_eth_critical_skb_num(void)
{
	return MAX_CRITICAL_ETH_SKB_NUM - atomic_read(&llll_critical_num);
}*/

__always_inline
void rtl_eth_skb_queue_head(struct sk_buff *newsk, int pool)
{
	llist_add(&newsk->recy_llnode, &llll_skb_head[pool]);
	atomic_inc(&llll_num[pool]);
}

/*__always_inline
void rtl_eth_skb_queue_head_critical(struct sk_buff *newsk)
{
	llist_add(&newsk->recy_llnode, &llll_critical_skb_head);
	atomic_inc(&llll_critical_num);
}*/

__always_inline
struct sk_buff *rtl_eth_skb_dequeue(int pool)
{
	struct llist_node *llll_node;
	struct sk_buff *skb;

	RECYCLE_SPINLOCK(&llll_read_lock[pool]);
	llll_node=llist_del_first(&llll_skb_head[pool]);
	RECYCLE_SPINUNLOCK(&llll_read_lock[pool]);
	if(llll_node==NULL)
		return NULL;
	skb=llist_entry(llll_node, struct sk_buff, recy_llnode);
	atomic_dec(&llll_num[pool]);
	return skb;
}

/*__always_inline
struct sk_buff *rtl_eth_skb_dequeue_critical(void)
{
	struct llist_node *llll_node;
	struct sk_buff *skb;

	RECYCLE_SPINLOCK(&llll_critical_read_lock);
	llll_node=llist_del_first(&llll_critical_skb_head);
	RECYCLE_SPINUNLOCK(&llll_critical_read_lock);
	if(llll_node==NULL)
		return NULL;
	skb=llist_entry(llll_node, struct sk_buff, recy_llnode);
	atomic_dec(&llll_critical_num);
	return skb;
}*/
#endif

extern int min_free_kbytes;
//#define MIN_LIMIT_PAGES (20*1024) //20M
//static unsigned long min_limit_pages = MIN_LIMIT_PAGES;
// default set 0, let userspace to adjust it
static unsigned long min_limit_pages = 0;
module_param(min_limit_pages, ulong, 0644);
#define K(x) ((x) << (PAGE_SHIFT - 10))
static struct sk_buff * rtl_netdev_alloc_skb_limit(unsigned int size){
	unsigned long free_pages, limit_pages;
	//printk("===> totalram %lu, freeram %lu, totalhigh %lu, freehigh %lu\n",
	//		val.totalram, val.freeram, val.totalhigh, val.freehigh);
	limit_pages = (min_free_kbytes > min_limit_pages) ? min_free_kbytes : min_limit_pages; 
	limit_pages += (size/512);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	free_pages = global_zone_page_state(NR_FREE_PAGES);
#else
	free_pages = global_page_state(NR_FREE_PAGES);
#endif
#ifdef CONFIG_HIGHMEM
	free_pages -= nr_free_highpages();
#endif
	free_pages = K(free_pages);
	if(free_pages > limit_pages) return __recycle_rtl_dev_alloc_skb(size);
	printk_ratelimited("[%s %d] %lu, %d\n", __func__, __LINE__, free_pages, min_free_kbytes);
	return NULL;
}

#ifdef ETH_RECYCLE_SKB_PROC
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
//static struct proc_dir_entry *proc_recycle_skb;
extern struct proc_dir_entry *realtek_proc;
struct proc_dir_entry *rtl_recycle_pe=NULL;

static int rtl_recycleskb_read(struct seq_file *seq, void *v)
{
	int i;

	//if recycle skb is disabled, nothing to be displayed.
	if(switch_off_recycle_skb){
		seq_printf(seq, "\nUsage:\n");
		seq_printf(seq, "echo on > /proc/realtek/recycle_skb\n - Switch on recycle skb mechanism.\n");
		return 0;
	}

#if	defined(RTL_RING_BUFFER_RECYCLE_SKB)
	seq_printf(seq, "\nRecycle SKB: ring-buffer");
#endif
#if	defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB)
	seq_printf(seq, "\nRecycle SKB: double-link-list");
#endif
#if	defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
	seq_printf(seq, "\nRecycle SKB: single-link-list");
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
	seq_printf(seq, "\nRecycle SKB: straight-array");
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
	seq_printf(seq, "\nRecycle SKB: lock_less-link-list");
#endif
	for(i=0;i<(0x1<<RTL_POOL_NUM_SHIFT_BITS);i++){
		if(eth_skb_pool[i]==0)continue;
		seq_printf(seq, "\npool[%d]",i);
		seq_printf(seq, "\neth_skb_total_num = %u eth_skb_free_num = %u, eth_skb_max_size = %u", eth_skb_total_num[i], eth_skb_free_num[i], eth_skb_max_size[i]);
		seq_printf(seq, "\nrecycle_skb_free_num:\t\t%10d", rtl_free_eth_skb_num(i));
		seq_printf(seq, "\nrecycle_skb_alloc_num:\t\t%10d", rtl_alloc_eth_skb_num(i));
		//seq_printf(seq, "\ntop_recycle_skb_alloc_num:\t\t%10d/%d", (eth_skb_free_num-lowest_eth_skb_free_num), eth_skb_free_num);
		//lowest_eth_skb_free_num=eth_skb_free_num;
		//seq_printf(seq, "\ncritical_recycle_skb_free_num:\t\t%10d\n", rtl_free_eth_critical_skb_num());
	}
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	{
		int i;
		seq_printf(seq, "\nSKB CB range: %d ~ %d",RTL_SKB_CB_SECTION_LOW,RTL_SKB_CB_SECTION_HIGH);
		seq_printf(seq, "\nSKB CB section registered: ");
		for(i=RTL_SKB_CB_SECTION_LOW;i<=RTL_SKB_CB_SECTION_HIGH;i++){
			if(rtl_recycle_skb_cb_check[i])
				seq_printf(seq, "%d ",i);
		}
	}
#endif
	seq_printf(seq, "\nUsage:\n");
	seq_printf(seq, "echo on > /proc/realtek/recycle_skb\n - Switch on recycle skb mechanism.\n");
	seq_printf(seq, "echo off > /proc/realtek/recycle_skb\n - Switch off recycle skb mechanism.\n");
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
	seq_printf(seq, "echo inc <pool> <count> > /proc/realtek/recycle_skb\n - Increase free skb number of pool, if new skb is available.\n");
	seq_printf(seq, "echo dec <pool> <count> > /proc/realtek/recycle_skb\n - Decrease free skb number of pool, if free skb enough.\n");
#endif
	return 0;
}

static ssize_t rtl_recycleskb_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	int i,operator=0;
	unsigned int original_state=switch_off_recycle_skb;
	unsigned long operand=0,flags,pool;
	char tmpbuf[64];
	char *strptr;
	char *tokptr;
	struct sk_buff *skb;

	//echo inc poolid 100 > /proc/recycle_skb
	//echo dec poolid 100 > /proc/recycle_skb
	//echo off > /proc/recycle_skb
	//echo on > /proc/recycle_skb

	//printk("in_irq=%d in_softirq=%d in_interrupt=%d in_serving_softirq=%d in_nmi=%d in_task=%d\n",
	//			in_irq(),in_softirq(),in_interrupt(),in_serving_softirq(),in_nmi(),in_task());

	if(count >= sizeof(tmpbuf))
		return -EINVAL;

	if (buf && !copy_from_user(tmpbuf, buf, count)) {
		for (i = 0; i < count; i++) {
			if (tmpbuf[i] < 0 || tmpbuf[i] > 255)
				goto errout;
		}
		tmpbuf[count] = '\0';
		strptr=tmpbuf;
		tokptr = strsep(&strptr,"\n ");
		if (tokptr==NULL){
			goto errout;
		}
		else if(strcasecmp(tokptr,"on")==0){
			if(original_state==RTL_RECYCLE_SWITCH_OFF){
				local_irq_save(flags);
				for(pool=0;pool<(1<<RTL_POOL_NUM_SHIFT_BITS);pool++){
					if(eth_skb_pool[pool]){
						for(i=0;i<eth_skb_total_num[i];i++){
							skb=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
							if(skb){
								__init_skb_recy_eth(skb);
								skb->recyclable=pool;
								__init_skb_recy_eth_debug(skb);
								rtl_eth_skb_queue_head(skb,pool);
							}
						}
						eth_skb_free_num[pool]=eth_skb_total_num[pool];
					}
				}
				switch_off_recycle_skb=RTL_RECYCLE_SWITCH_ON;
				local_irq_restore(flags);
			}
			printk("switch on!\n");
			return count;
		}else if(strcasecmp(tokptr,"off")==0){
			if(original_state==RTL_RECYCLE_SWITCH_ON){
				switch_off_recycle_skb=RTL_RECYCLE_SWITCH_OFF;
				//drain all free skb
				local_irq_save(flags);
				for(pool=0;pool<(1<<RTL_POOL_NUM_SHIFT_BITS);pool++){
					if(eth_skb_pool[pool]){
						while(rtl_free_eth_skb_num(pool)){
							skb=rtl_eth_skb_dequeue(pool);
							if(skb){
								skb->recyclable=0;
								dev_kfree_skb_any(skb);
							}
						}
					}
				}
				local_irq_restore(flags);
			}
			printk("switch off!\n");
			return count;
		}
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
		else if(switch_off_recycle_skb==RTL_RECYCLE_SWITCH_OFF){
			//when switch off should not operate free skb number.
			return count;
		}else if(strcasecmp(tokptr,"inc")==0){
			operator=1;
		}else if(strcasecmp(tokptr,"dec")==0){
			operator=-1;
		}else
			goto errout;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL){
			goto errout;
		}

		if(kstrtoul(tokptr, 0, &pool))
			goto errout;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL){
			goto errout;
		}

		if(kstrtoul(tokptr, 0, &operand))
			goto errout;

		//printk("operand=%d operator=%d delta=%d\n",operand,operator,operand*operator);
		local_irq_save(flags);
		if(operator < 0 && operand > rtl_free_eth_skb_num(pool)){
			local_irq_restore(flags);
			printk("dec over free skb num (%lu)\n",operand);
			goto errout;
		}

		for(i=0;i<operand;i++){
			if(operator>0){
				skb=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
				if(skb){
					__init_skb_recy_eth(skb);
					skb->recyclable=pool;
					__init_skb_recy_eth_debug(skb);
					rtl_eth_skb_queue_head(skb,pool);
					eth_skb_free_num[pool]++;
				}
			}else{
				skb=rtl_eth_skb_dequeue(pool);
				if(skb){
					skb->recyclable=0;
					dev_kfree_skb_any(skb);
					eth_skb_free_num[pool]--;
				}
			}
		}
		local_irq_restore(flags);
		printk("recycle_skb_free_num[%lu]:%d\n", pool, rtl_free_eth_skb_num(pool));
#endif
	}
errout:
	return count;
}

static int rtl_recycleskb_open(struct inode *inode, struct file *file)
{
	return single_open(file, rtl_recycleskb_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops rtl_recycleskb_ops = {
	.proc_open           = rtl_recycleskb_open,
	.proc_write          = rtl_recycleskb_write,
	.proc_read           = seq_read,
	.proc_lseek          = seq_lseek,
	.proc_release        = single_release,
};
#else
static const struct file_operations rtl_recycleskb_ops = {
	.owner          = THIS_MODULE,
	.open           = rtl_recycleskb_open,
	.write          = rtl_recycleskb_write,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

#endif //ETH_RECYCLE_SKB_PROC

int init_recycle_eth_skb_buf(unsigned int ring_size, unsigned int max_size, int forced_ring_size, int forced_new_pool)
{
	int i,pool=-1;
	int first_invalid=-1;
	struct sk_buff *skb;

	for(i=1;i<(0x1<<RTL_POOL_NUM_SHIFT_BITS);i++){
		if(eth_skb_pool[i]==0){
			if(first_invalid<0)
				first_invalid=i;
			continue;
		}else if(eth_skb_max_size[i]==max_size && forced_new_pool==0){
			pool=i;
			break;
		}
	}
	if(pool<0)pool=first_invalid;

	if(forced_ring_size){
		eth_skb_free_num[pool] = ring_size;
	}else{
		//memset(recycle_skb_buf, '\0', sizeof(struct recycle_skb_buf)*MAX_ETH_SKB_NUM);
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB_NUM) && (CONFIG_RTL865X_ETH_PRIV_SKB_NUM>0)
		//forced recycle pool size from preconfig or menuconfig
		eth_skb_free_num[pool] = CONFIG_RTL865X_ETH_PRIV_SKB_NUM;
#elif defined(CONFIG_RTL_ETH_RECYCLED_SKB_BY_MEM)
		struct sysinfo si;
		si_meminfo(&si);
		if(K(si.totalram) >= CONFIG_RTL_ETH_RECYCLED_SKB_THRESHOLD)
			eth_skb_free_num[pool] = CONFIG_RTL_ETH_RECYCLED_SKB_BY_MEM_HIGH;
		else
			eth_skb_free_num[pool] = CONFIG_RTL_ETH_RECYCLED_SKB_BY_MEM_LOW;
		
		if(eth_skb_free_num[pool] < ring_size) eth_skb_free_num[pool] = MAX_ETH_SKB_NUM + ring_size;
#else	
		eth_skb_free_num[pool] = MAX_ETH_SKB_NUM+ring_size;
#endif
	}
	eth_skb_total_num[pool] = eth_skb_free_num[pool];		//keep original total number

#if defined(RTL_RING_BUFFER_RECYCLE_SKB)
	rbr.rd_idx=0;
	rbr.wr_idx=0;
	rbr.state=RTL_RING_BUF_RECYCLE_EMPTY;
	spin_lock_init(&rbr.lock);

	for(i=0;(0x1<<i)<eth_skb_free_num;i++);
	if(i>=32){
		printk("pool is oversized...failed!\n");
		return;
	}

	eth_skb_free_num=(0x1<<i);
	rbr.ring_size_mask=eth_skb_free_num-1;

	rbr.ring_buffer_recycle_array = (struct sk_buff *)kmalloc(eth_skb_free_num*sizeof(struct sk_buff *),GFP_ATOMIC);
	if(!rbr.ring_buffer_recycle_array){
		printk("ring buffer recycle array allocate failed!\n");
		return;
	}

	rbr_critical.rd_idx=0;
	rbr_critical.wr_idx=0;
	rbr_critical.state=RTL_RING_BUF_RECYCLE_EMPTY;
	spin_lock_init(&rbr_critical.lock);
	for(i=0;(0x1<<i)<MAX_CRITICAL_ETH_SKB_NUM;i++);

	rbr_critical.ring_size_mask=(0x1<<i)-1;

	rbr_critical.ring_buffer_recycle_array = (struct sk_buff *)kmalloc((0x1<<i)*sizeof(struct sk_buff *),GFP_ATOMIC);
	if(!rbr_critical.ring_buffer_recycle_array){
		printk("ring buffer recycle critical array allocate failed!\n");
		return;
	}
#endif
#if defined(RTL_DOUBLE_LINK_LIST_RECYCLE_SKB) || defined(RTL_SINGLE_LINK_LIST_RECYCLE_SKB)
	skb_queue_head_init(&recycle_skb_queue[pool]);
	skb_queue_head_init(&recycle_critical_skb_queue[pool]);
#endif
#if defined(RTL_STRAIGHT_ARRAY_RECYCLE_SKB)
	sra.idx=0;
	spin_lock_init(&sra.lock);

	sra.array = (struct sk_buff *)kmalloc(eth_skb_free_num*sizeof(struct sk_buff *),GFP_ATOMIC);
	if(!sra.array){
		printk("straight recycle array allocate failed!\n");
		return;
	}

	sra_critical.idx=0;
	spin_lock_init(&sra_critical.lock);

	sra_critical.array = (struct sk_buff *)kmalloc(MAX_CRITICAL_ETH_SKB_NUM*sizeof(struct sk_buff *),GFP_ATOMIC);
	if(!sra_critical.array){
		printk("straight critical recycle array allocate failed!\n");
		return;
	}
#endif
#if defined(RTL_LL_LINK_LIST_RECYCLE_SKB)
	init_llist_head(&llll_skb_head[pool]);
	spin_lock_init(&llll_read_lock[pool]);
	atomic_set(&llll_num[pool], 0);

	/*init_llist_head(&llll_critical_skb_head);
	spin_lock_init(&llll_critical_read_lock);
	atomic_set(&llll_critical_num, 0);*/
#endif

	if(max_size<CROSS_LAN_MBUF_LEN)
		eth_skb_max_size[pool] = CROSS_LAN_MBUF_LEN;
	else
		eth_skb_max_size[pool] = max_size;
	eth_skb_pool[pool] = 1;
	//printk("%s eth_skb_free_num = %u, eth_skb_max_size = %u\n", __FUNCTION__, eth_skb_free_num, eth_skb_max_size);
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	if(rtl_recycle_skb_cb_value<0)rtl_recycle_skb_cb_value=rtl_recycle_skb_cb_section_register();
#endif
	//skb_queue_head_init(&recycle_skb_queue);
	//skb_queue_head_init(&recycle_critical_skb_queue);

	for(i=0;i<eth_skb_free_num[pool];i++){
		skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if(skb){
			__init_skb_recy_eth(skb);
			skb->recyclable=pool;
			__init_skb_recy_eth_debug(skb);
			rtl_eth_skb_queue_head(skb,pool);
		}//else
			//printk("ERROR!! no skb is allocated!!!!!!!!!\n\n\n\n\n\n\n\n\n");
	}

	/*for(i=0;i<MAX_CRITICAL_ETH_SKB_NUM;i++){
		skb = rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if(skb){
			__init_skb_recy_eth(skb);
			skb->recyclable=RTL_RECYCLE_CRITICAL_QUEUE;
			__init_skb_recy_eth_debug(skb);
			rtl_eth_skb_queue_head_critical(skb);
		}//else
			//printk("ERROR!! no skb is allocated!!!!!!!!!\n\n\n\n\n\n\n\n\n");
	}*///printk("\n\n\n\n\n\recycle_critical_skb_queue.len is %d\n\n\n\n",recycle_critical_skb_queue.qlen);

	//init rx_skb_queue
	memset(&rx_skb_queue, 0, sizeof(rx_skb_queue));
#ifdef ETH_RECYCLE_SKB_PROC
	if(!rtl_recycle_pe){
	    rtl_recycle_pe = proc_create_data("recycle_skb",
	                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
	                          realtek_proc, &rtl_recycleskb_ops, NULL);
	    if (!rtl_recycle_pe) 
	        printk("can't create proc entry for recycle_skb\n");
    }
	/*proc_recycle_skb = proc_create_data("recycle_skb", 0644, NULL, &rtl_recycleskb_ops, NULL);
	if(proc_recycle_skb == NULL) {
		printk("can't create proc entry for recycle_skb\n");
	}*/
#endif

	return pool;
}
EXPORT_SYMBOL(init_recycle_eth_skb_buf);

void deinit_recycle_eth_skb_buf(int pool)
{
	int i;
	struct sk_buff *skb;

	//wnic_switch_off_recycle_skb[pool]=WNIC_RECYCLE_SWITCH_OFF;
	
	while(rtl_free_eth_skb_num(pool)){
		skb=rtl_eth_skb_dequeue(pool);
		if(skb){
			skb->recyclable=0;
			dev_kfree_skb_any(skb);
		}
	}

	eth_skb_total_num[pool]=0;
	eth_skb_free_num[pool]=0;
	eth_skb_max_size[pool]=0;

	eth_skb_pool[pool]=0;

#ifdef ETH_RECYCLE_SKB_PROC
	if(rtl_recycle_pe){
		for(i=((0x1<<RTL_POOL_NUM_SHIFT_BITS)-1);i>=1;i--){
			if(eth_skb_pool[i]>0)break;
		}
		if(i<0 && rtl_recycle_pe)
			proc_remove(rtl_recycle_pe);
	}
#endif

}
EXPORT_SYMBOL(deinit_recycle_eth_skb_buf);

void recycle_skb_clean(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo;
	bool pfmemalloc;
	sk_buff_data_t		end;
	unsigned char		*head;
	unsigned int		truesize;
	unsigned char		recyclable;
	//unsigned char		prepared_recycle;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	unsigned char 		double_check_free;
#endif

	//keep these fields before clean
	pfmemalloc=skb->pfmemalloc;
	head=skb->head;
	end=skb->end;
	truesize=skb->truesize;
	recyclable=skb->recyclable;
	//prepared_recycle=skb->prepared_recycle;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	double_check_free=skb->double_check_free;
#endif

	memset(skb, 0, offsetof(struct sk_buff, tail));
#if 1 //clear RTK private header
	memset(&skb->__private_header_start, 0, offsetof(struct sk_buff, __private_header_end) -
											offsetof(struct sk_buff, __private_header_start));
#endif
	/* Account for allocated memory : skb + skb->head */
	skb->truesize=truesize;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
	atomic_set(&skb->users, 1);
#else
	refcount_set(&skb->users, 1);
#endif
	skb->head=head;
	skb->end=end;
	skb->pfmemalloc = pfmemalloc;
	skb->mac_header = (typeof(skb->mac_header))~0U;
	skb->transport_header = (typeof(skb->transport_header))~0U;
	skb->recyclable=recyclable;
	//skb->prepared_recycle=prepared_recycle;
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	skb->double_check_free=double_check_free;
	skb->skb_bak=skb;
	skb->head_bak=head;
#endif

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
	kmemcheck_annotate_variable(shinfo->destructor_arg);
#endif
}
EXPORT_SYMBOL(recycle_skb_clean);

__always_inline
void __recycle_rtl_eth_buf(struct sk_buff *skb)
{
	if(skb->cloned)recycle_skb_clean(skb);		//clean cloned skb
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	if(skb->double_check_free==0)printk("%s %d ERROR!!! double check free should be one...skb=%px\n",__func__,__LINE__,skb);
	else skb->double_check_free=0;
	{
		rtl_recycle_skb_debug_t rtl_skb_debug_parameter;
		rtl_skb_debug_parameter.cb=rtl_recycle_skb_cb_value;
		rtl_skb_debug_parameter.check_skb=skb;
		rtl_skb_debug_parameter.skb_pointer_off=-1;
		rtl_skb_debug_parameter.headroom=-1;
		rtl_skb_debug_parameter.dump_stack_en=1;
		rtl_recycle_skb_cb_section_enter(&rtl_skb_debug_parameter);
	}
#endif
}

__always_inline
void __recycle_frag_list(struct sk_buff *skb, void (*skb_queue_head)(struct sk_buff *skb, int pool), __u16 pool)
{
	if(skb->frag_list_recycle){
		//free frag_list and it's next till NULL
		struct sk_buff *tmp_next,*NEXT_free=skb_shinfo(skb)->frag_list;
		while(NEXT_free){
			tmp_next=NEXT_free->next;
			NEXT_free->next=NULL;
			__recycle_rtl_eth_buf(NEXT_free);
			skb_queue_head(NEXT_free, pool);
			NEXT_free=tmp_next;
		}
		skb_shinfo(skb)->frag_list=NULL;
		skb->data_len=0;
		skb->frag_list_recycle=0;
	}
}

int recycle_rtl_eth_buf(struct sk_buff *skb)
{
	if(likely(switch_off_recycle_skb==RTL_RECYCLE_SWITCH_ON)){
		__u16 pool=skb->recyclable;//printk("%s %d recyclable=%d\n",__FUNCTION__,__LINE__,recyclable);

		if(likely(pool>0)){
			__recycle_frag_list(skb, rtl_eth_skb_queue_head, pool);
			__recycle_rtl_eth_buf(skb);
			rtl_eth_skb_queue_head(skb, pool);
			return 1;
		}
	}
	return 0;
}
EXPORT_SYMBOL(recycle_rtl_eth_buf);

struct sk_buff *dev_alloc_skb_recy_eth(unsigned int size, int pool)
{
	/* first argument is not used */
	if(unlikely(size > eth_skb_max_size[pool] || switch_off_recycle_skb))
	{
		return NULL;
	}
	else
	{//printk("%s %d \n",__FUNCTION__,__LINE__);
		//dequeue one skb or null if empty
		//struct skb_shared_info *shinfo;// = skb_shinfo(skb);if(skb->head==NULL)printk("skb->head is NULL!!!skb->data=%p\n",skb->data);if(shinfo==NULL)printk("%s %d shinfo is null!\n",__FUNCTION__,__LINE__);
		//unsigned int skb_size;
		struct sk_buff *skb=rtl_eth_skb_dequeue(pool);
		if(likely(skb)){//printk("%s %d \n",__FUNCTION__,__LINE__);
			
#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
			if(skb->double_check_free)printk("%s %d ERROR!!! double check free should be zero...skb=%px\n",__func__,__LINE__,skb);
			else skb->double_check_free=1;
			{
				rtl_recycle_skb_debug_t rtl_skb_debug_parameter;
				rtl_skb_debug_parameter.cb=rtl_recycle_skb_cb_value;
				rtl_skb_debug_parameter.check_skb=skb;
				rtl_skb_debug_parameter.skb_pointer_off=-1;
				rtl_skb_debug_parameter.headroom=-1;
				rtl_skb_debug_parameter.dump_stack_en=1;
				rtl_recycle_skb_cb_section_leave(&rtl_skb_debug_parameter);
			}
#endif
			__init_skb_recy_eth(skb);
			//shinfo=skb_shinfo(skb);
			//atomic_set(&shinfo->dataref, 1);

			//eth_skb_free_num--;
			//eth_skb_alloc_num++;
			//if(rtl_free_eth_skb_num() < lowest_eth_skb_free_num)
				//lowest_eth_skb_free_num = rtl_free_eth_skb_num();
		}//if(skb==NULL)printk("%s %d skb=NULL!!!!\n",__FUNCTION__,__LINE__);
		return skb;
	}
}
EXPORT_SYMBOL(dev_alloc_skb_recy_eth);

struct sk_buff *recycle_skb_swap(struct sk_buff *skb)
{
	//allocate new skb, swap skb, enqueue old_skb back to queue, continue to protocol stack
	struct sk_buff *new_skb=NULL;
	int pool=skb->recyclable;

	//check for jumbo packet
	if(skb_shinfo(skb)->frag_list){//printk("%s has frag_list!%p\n",__FUNCTION__,skb_shinfo(skb)->frag_list);
		struct sk_buff *segs=skb_shinfo(skb)->frag_list;
		new_skb=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if(new_skb){//printk("%s has new_skb!\n",__FUNCTION__);
			struct sk_buff *frag_skb=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
			if(frag_skb){//printk("%s has frag_skb!\n",__FUNCTION__);
				struct sk_buff *tmp_skb=skb_shinfo(skb)->frag_list->next;
				if(tmp_skb){//printk("%s has tmp_skb!\n",__FUNCTION__);
					//loop for next
					struct sk_buff *swap_skb=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
					if(swap_skb){//printk("%s has swap_skb!\n",__FUNCTION__);
						struct sk_buff *next_skb=tmp_skb->next;
						frag_skb->next=swap_skb;
						while(next_skb){//printk("%s has next_skb!\n",__FUNCTION__);
							struct sk_buff *next=next_skb->next;
							struct sk_buff *alloc=NULL;
							alloc=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
							if(alloc){//printk("%s has alloc!\n",__FUNCTION__);
								swap_skb->next=alloc;
								swap_skb=alloc;
							}else{
								swap_skb=NULL;
								break;
							}
							next_skb=next;
						}
						if(swap_skb){
							//copy new_skb,frag_skb and all next_skb
							swap_skb=frag_skb->next;//printk("%s %d swap_skb=%p\n",__FUNCTION__,__LINE__,swap_skb);
							next_skb=skb_shinfo(skb)->frag_list->next;//printk("%s %d next_skb=%p\n",__FUNCTION__,__LINE__,next_skb);
							while(swap_skb){
								struct sk_buff *swap_next=swap_skb->next;//printk("%s %d swap_next=%p\n",__FUNCTION__,__LINE__,swap_next);
								struct sk_buff *next_skb_next=next_skb->next;//printk("%s %d next_skb_next=%p\n",__FUNCTION__,__LINE__,next_skb_next);
								__recycle_skb_copy(next_skb, swap_skb);
								swap_skb=swap_next;
								next_skb=next_skb_next;
							}
							//printk("%s %d skb_shinfo(skb)->frag_list=%p, frag_skb=%p\n",__FUNCTION__,__LINE__,skb_shinfo(skb)->frag_list,frag_skb);
							new_skb->data_len=skb->data_len;
							__recycle_skb_copy(skb, new_skb);//printk("%s %d\n",__FUNCTION__,__LINE__);
							__recycle_skb_copy(skb_shinfo(new_skb)->frag_list, frag_skb);//printk("%s %d skb_shinfo(skb)->frag_list=%p, frag_skb=%p\n",__FUNCTION__,__LINE__,skb_shinfo(skb)->frag_list,frag_skb);
							skb_shinfo(skb)->frag_list=skb_shinfo(new_skb)->frag_list;//printk("%s %d\n",__FUNCTION__,__LINE__);
							skb_shinfo(new_skb)->frag_list=frag_skb;//printk("%s %d\n",__FUNCTION__,__LINE__);
						}else{
							//no memory for swap
							//printk("%s %d alloc fail!!! free all!!\n",__FUNCTION__,__LINE__);
							swap_skb=frag_skb->next;
							while(swap_skb){
								struct sk_buff *next=swap_skb->next;
								dev_kfree_skb_any(swap_skb);//printk("free swap_skb!!\n");
								swap_skb=next;
							}
							dev_kfree_skb_any(frag_skb);//printk("free frag_skb!\n");
							dev_kfree_skb_any(new_skb);//printk("free new_skb!\n");
							new_skb=NULL;
						}
					}else{
						//no memory for swap
						dev_kfree_skb_any(frag_skb);
						dev_kfree_skb_any(new_skb);
						new_skb=NULL;
					}
				}else{//printk("%s has NO tmp_skb!\n",__FUNCTION__);
					//copy new_skb and frag_skb
					new_skb->data_len=skb->data_len;
					__recycle_skb_copy(skb, new_skb);//printk("%s %d skb->data_len=%d new_skb->data_len=%d\n",__FUNCTION__,__LINE__,skb->data_len,new_skb->data_len);
					__recycle_skb_copy(skb_shinfo(new_skb)->frag_list, frag_skb);//printk("%s %d skb_shinfo(skb)->frag_list=%p, frag_skb=%p\n",__FUNCTION__,__LINE__,skb_shinfo(skb)->frag_list,frag_skb);
					skb_shinfo(skb)->frag_list=skb_shinfo(new_skb)->frag_list;//printk("%s %d skb_shinfo(skb)->frag_list=%p\n",__FUNCTION__,__LINE__,skb_shinfo(skb)->frag_list);
					skb_shinfo(new_skb)->frag_list=frag_skb;//printk("%s %d skb_shinfo(new_skb)->frag_list=%p\n",__FUNCTION__,__LINE__,skb_shinfo(new_skb)->frag_list);
				}
			}else{
				//no memory for frag_skb
				dev_kfree_skb_any(new_skb);
				new_skb=NULL;
			}
		}
		//free old skb
		while (segs) {
			struct sk_buff *next = segs->next;
			dev_kfree_skb_any(segs);
			segs = next;
		}
		skb_shinfo(skb)->frag_list=NULL;		
	}else{
		new_skb=rtl_netdev_alloc_skb_limit(eth_skb_max_size[pool]);
		if(new_skb){//struct skb_shared_info *shinfo = skb_shinfo(new_skb);//printk("%s %d shinfo->nr_frags=%d skb->recyclable=%d shinfo->frag_list=%p\n",__FUNCTION__,__LINE__,shinfo->nr_frags,new_skb->recyclable,shinfo->frag_list);
			__recycle_skb_copy(skb, new_skb);
		}
	}
	dev_kfree_skb_any(skb);
	//printk("%s %d after recyclable swap!!qlen=%d\n",__FUNCTION__,__LINE__,skb_queue_len(&recycle_skb_queue));
	/*if(new_skb){
		struct sk_buff *skbinlist;
		//printk("has new_skb!!!!\n");
		//printk(KERN_INFO "==> Dump SKB (after pull 14 bytes), new_skb->len %u, new_skb->data_len %u\n", new_skb->len, new_skb->data_len);
		print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 16, 1, new_skb->data, new_skb->len - new_skb->data_len, true);

		if (skb_shinfo(new_skb)->frag_list) {
			for (skbinlist = skb_shinfo(new_skb)->frag_list; skbinlist; skbinlist = skbinlist->next) {
				//printk(KERN_INFO "==> Dump SKB in frag_list, new_skb->len (dump_len) %u\n", skbinlist->len);
				print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 16, 1, skbinlist->data, skbinlist->len, true);
			}
		}
	}*/
	return new_skb;
}
EXPORT_SYMBOL(recycle_skb_swap);

#ifdef CONFIG_WIRELESS_EXT
struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;	
	unsigned char recyclable;

	if (rx_skb_queue.qlen == 0) {
#if defined(CONFIG_LUNA_G3_SERIES)
		n = dev_alloc_skb(CA_NI_SKB_ALLOC_DATA_SIZE);
#else
		n = re8670_getAlloc(CROSS_LAN_MBUF_LEN);
#endif
	}
	else {
#ifdef RTK_QUE
		n = rtk_dequeue(&rx_skb_queue);
#else
		n = __skb_dequeue(&rx_skb_queue);
#endif
	}
	
	if (n == NULL) 
		return NULL;

	//keep original recyclable
	recyclable = n->recyclable;

	/* Set the tail pointer and length */	
	n->len = 0;
	skb_put(n, skb->len);	
	n->csum = skb->csum;	
	n->ip_summed = skb->ip_summed;	
	memcpy(n->data, skb->data, skb->len);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
	skb_copy_header(n, skb);
#else
	copy_skb_header(n, skb);
#endif

	//keep original recyclable
	n->recyclable=recyclable;
	return n;
}
#endif // CONFIG_NET_RADIO

#if	defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
int rtl_recycle_skb_cb_section_register(void)
{
	int i;
	spin_lock(&rtl_recycle_skb_cb_lock);
	for(i=RTL_SKB_CB_SECTION_LOW;i<=RTL_SKB_CB_SECTION_HIGH;i++){
		if(rtl_recycle_skb_cb_check[i]==0){
			rtl_recycle_skb_cb_check[i]=1;
			spin_unlock(&rtl_recycle_skb_cb_lock);
			printk("%s :skb cb section register=>%d\n", __func__, i);
			return i;
		}	
	}
	spin_unlock(&rtl_recycle_skb_cb_lock);
	return -1;
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_register);

int rtl_recycle_skb_cb_section_unregister(unsigned int cb)
{
	if(cb<RTL_SKB_CB_SECTION_LOW || cb>RTL_SKB_CB_SECTION_HIGH)
		return -1;
	spin_lock(&rtl_recycle_skb_cb_lock);
	rtl_recycle_skb_cb_check[cb]=0;
	spin_unlock(&rtl_recycle_skb_cb_lock);
	printk("%s :skb cb section unregister=>%d\n", __func__, cb);
	return 0;
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_unregister);

int rtl_recycle_skb_cb_section(rtl_recycle_skb_debug_t *rtl_skb_debug, int section_value)
{
	int i,err=0;

	if(rtl_skb_debug->check_skb==NULL){
		printk("%s: skb null..\n", __func__);
		return -1;
	}
	if(rtl_skb_debug->check_skb->recyclable==0){
		//printk("%s: not recycle skb..\n", __func__);
		return -1;
	}
	if(rtl_skb_debug->cb<RTL_SKB_CB_SECTION_LOW || rtl_skb_debug->cb>RTL_SKB_CB_SECTION_HIGH){
		printk("%s: skb cb[%d] out of range(%d~%d)..\n", __func__, rtl_skb_debug->cb, RTL_SKB_CB_SECTION_LOW, RTL_SKB_CB_SECTION_HIGH);
		return -1;
	}
	if(rtl_recycle_skb_cb_check[rtl_skb_debug->cb]==0){
		printk("%s: skb cb[%d] unregistered..\n", __func__, rtl_skb_debug->cb);
		return -1;
	}

	for(i=RTL_SKB_CB_SECTION_LOW;i<=RTL_SKB_CB_SECTION_HIGH;i++){
		if(i==rtl_skb_debug->cb){
			if(section_value<0 && rtl_skb_debug->check_skb->cb[i]==0){
				printk("%s: skb->cb[%d] section inconsistent!!skb=%px skb->data=%px skb->head=%px\n", __func__, i, rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head);
			}else if(rtl_skb_debug->check_skb->cb[i]==section_value){
				printk("%s: skb->cb[%d] section %s!!skb=%px skb->data=%px skb->head=%px\n", __func__, i, section_value?"re-enter":"re-leave", rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head);
				err++;
			}
		}else if(rtl_skb_debug->check_skb->cb[i]){
			printk("%s: skb->cb[%d] section overlap!!skb=%px skb->data=%px skb->head=%px\n", __func__, i, rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head);
			err++;
		}
	}
	if(section_value>=0)rtl_skb_debug->check_skb->cb[rtl_skb_debug->cb]=section_value;

	if(rtl_skb_debug->headroom>=0 && skb_headroom(rtl_skb_debug->check_skb) < rtl_skb_debug->headroom){
		printk("%s %d: skb data offset unmatch!! skb->data=%px skb->head=%px skb_headroom=%d < %d\n", __func__, __LINE__, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb->head, skb_headroom(rtl_skb_debug->check_skb), rtl_skb_debug->headroom);
		err++;
	}
	if(rtl_skb_debug->skb_pointer_off>=0 && rtl_skb_debug->check_skb->skb_bak != (struct sk_buff *)(*((ca_uint_t *)(rtl_skb_debug->check_skb->head+rtl_skb_debug->skb_pointer_off)))){
		printk("%s %d: skb data unmatch!! offset=%d skb_bak=%px skb->data=%px skb=%px skb->head=%px\n", __func__, __LINE__, rtl_skb_debug->skb_pointer_off, rtl_skb_debug->check_skb->skb_bak, rtl_skb_debug->check_skb->data, rtl_skb_debug->check_skb, rtl_skb_debug->check_skb->head);
		print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 16, 1, (rtl_skb_debug->check_skb->head+rtl_skb_debug->skb_pointer_off)-16, 32, true);
		err++;
	}
	if(rtl_skb_debug->check_skb->skb_bak != rtl_skb_debug->check_skb){
		printk("%s %d: skb bak unmatch!! skb_bak=%px skb=%px\n", __func__, __LINE__, rtl_skb_debug->check_skb->skb_bak, rtl_skb_debug->check_skb);
		err++;
	}
	if(rtl_skb_debug->check_skb->head_bak != rtl_skb_debug->check_skb->head){
		printk("%s %d: skb head bak unmatch!! skb->head_bak=%px skb->head=%px\n", __func__, __LINE__, rtl_skb_debug->check_skb->head_bak, rtl_skb_debug->check_skb->head);
		err++;
	}

	if(err)
		goto ERR_RET;

	return 0;

ERR_RET:
	if(rtl_skb_debug->dump_stack_en>0)
		dump_stack();
	return -1;
}

int rtl_recycle_skb_cb_section_enter(rtl_recycle_skb_debug_t *rtl_skb_debug)
{
	return rtl_recycle_skb_cb_section(rtl_skb_debug, RTL_SKB_CB_SECTION_ENTER);
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_enter);

int rtl_recycle_skb_cb_section_check(rtl_recycle_skb_debug_t *rtl_skb_debug)
{
	return rtl_recycle_skb_cb_section(rtl_skb_debug, -1);
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_check);

int rtl_recycle_skb_cb_section_leave(rtl_recycle_skb_debug_t *rtl_skb_debug)
{
	return rtl_recycle_skb_cb_section(rtl_skb_debug, RTL_SKB_CB_SECTION_LEAVE);
}
EXPORT_SYMBOL(rtl_recycle_skb_cb_section_leave);
#endif

#endif // CONFIG_RTL_ETH_RECYCLED_SKB
