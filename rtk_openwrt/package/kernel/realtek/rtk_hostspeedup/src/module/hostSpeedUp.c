#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/miscdevice.h>
#include <net/snmp.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>
#include <linux/if_pppox.h>
#include <linux/netfilter_ipv4.h>
#include <net/xfrm.h>
#include <linux/mroute.h>
#include <linux/netlink.h>
#include <linux/ip.h>
#include <net/tcp.h>
#include <linux/workqueue.h>
#include <linux/ftrace.h>
#include <linux/tracepoint.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include "ip_speedup.h"
#include "hostSpeedUp.h"
#if defined(CONFIG_RTL8686)
#include "bspchip.h"
#elif defined(CONFIG_RTL9607C)
#include "bspchip.h"
#include "bspchip_9607c.h"
#include "re8686_rtl9607c.h"
#elif defined(CONFIG_ARCH_CORTINA)
//#include <soc/cortina/g3_registers.h>
#include "ca_ne_autoconf.h"
#include "ca_ni.h"
#include "ca_ext.h"
//#include "aal_ni.h"
#define CA_ONE_SEC_COUNTER ((unsigned int)1953125)    //125MHz/64
#endif

extern int drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx);
extern int drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx);

#define ip_inet_ntoa(x,v) _ip_inet_ntoa(x,v)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static void host_timeout(struct timer_list *timer);
#else
static void host_timeout(unsigned long data);
#endif
static int dbgflag = 0;

#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL9603CVD_SERIES) || defined(CONFIG_RTL8198D_SERIES)
	#if defined(CONFIG_TX_HW_LSO)
		#define MAX_BUF_LEN_DEF (1024*20)
	#else
		#define MAX_BUF_LEN_DEF (1024*4)
	#endif
	#define SKB_CACHE_ALLOC 1
#else
	#define MAX_BUF_LEN_DEF (1024*20)
	#define SKB_CACHE_ALLOC 1
#endif

#ifdef SKB_CACHE_ALLOC
static struct kmem_cache *skb_upload_cache = NULL;
#endif
static unsigned int MAX_BUF_LEN = MAX_BUF_LEN_DEF;
static unsigned int max_buf_len;

static inline char *_ui8tod( unsigned char n, char *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}
static char *_ip_inet_ntoa(unsigned char* ina,int ipver)
{
	static char buf[64];
	char *p = buf;
	unsigned char *ucp = (unsigned char *)ina;

	if(ipver==0)
	{
		p = _ui8tod( ucp[0] & 0xFF, p);
		*p++ = '.';
		p = _ui8tod( ucp[1] & 0xFF, p);
		*p++ = '.';
		p = _ui8tod( ucp[2] & 0xFF, p);
		*p++ = '.';
		p = _ui8tod( ucp[3] & 0xFF, p);
		*p++ = '\0';
	}
	else
	{
		int i=0;
		memset(buf, 0, sizeof(buf));
		for(i=0; i<IP6_ADDR_LEN; i++)
		{		
			sprintf(buf+strlen(buf),"%02X",ina[i]);
			if((i&1) && (i!=(IP6_ADDR_LEN-1)))
				sprintf(buf+strlen(buf),":");
		}
	}

	return (buf);
}

#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
static unsigned int speedtest_skip_fc_Bit=0;
#endif//endof CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE

#define TRACK_NUM 16

//__DRAM unsigned int HostSpeedUP=0;
/*
 * modify history
 *  ?? - 20181104:  0 - disable speedup / 1 - enable speedup
 *  20181105 - ??:  0 - disable speedup / not 0 - enable speedup
 */
volatile unsigned int HostSpeedUP=0;

struct timer_list ht_timer;
int HOST_TIMEOUT = 10; /*10 sec*/
int HTTP_PORT = 80;


#define SPEEDUP_CONN_NUM		50
#define SPEEDUP_HASH_SIZE 		64	//must be power of 2

struct sack_block {
	u32 start_seq;
	u32 end_seq;
};

struct _speedup_stream_track {
	u8 dstip[IP6_ADDR_LEN];		/* server Addr */
	u8 srcip[IP6_ADDR_LEN];		/* client Addr */
	u16 dport;		/* server Port */
	u16 sport;		/* client port */
	u32 totalbytes;	/* file segment size */
	u16 id;			/* ACK ID */
	u16 cnt;		/* rx pkt count */
	u32 rcv_nxt;		/* next sequence no. */
	u32 dupSegCnt;
	u32 rcvtime;	/* unit:us*/
	u32 sack_blk_num;
	struct sack_block sack_blk[TCP_NUM_SACKS];
	/************ below for upload speedup ************/
	u32 push;
	u32 sndtime;	/* unit:us*/
	volatile u32 sndPktNum;
	u32 wscale;		/* window scaling size of peer server */
	u32 snd_mss;		/* send mss */
	u32 seqNo;		/* seqno for current trasmitting packet */
	u32 ack_seq;	/* tcph->ack_seq of data */
	u32 ackedSeqNo;	/* last data acked by server */
	u32 frozenAckCnt; /* maybe server dropped all packets after ackedSeqNo, then client should retransmit all subsequent packets */
	u32 upload_sack_blk_num;
	struct sack_block upload_sack_blk[TCP_NUM_SACKS];
	u32 direction;	//0-download 1-upload
	u32 stat_pkt;	//received ack num.
	u32 payload_len;
	u32 last_push_len;
	u32 burstDataSize;
	/********************* end ************************/
#ifdef IP_SPEEDUP_OUTORDER_DBG
	u32 ackSeq;
	u32 blk_num;
	struct sack_block blk[TCP_NUM_SACKS];
#endif
	speedup_pktHdr_t pkthdr;
	spinlock_t sst_lock;
	//struct sk_buff *skb;		/* skb queued for quickly ack */
	//struct _speedup_stream_track *next;
	struct list_head node;
	/* add to active stream list */
	struct list_head activeNode;
};
static struct _speedup_stream_track sst[SPEEDUP_CONN_NUM];
static struct list_head sstFreeHeadList;
static struct list_head sstAciveHeadList;
DEFINE_RWLOCK(speedup_lock);
DEFINE_SPINLOCK(speedup_fastack_lock);
DEFINE_SPINLOCK(speedup_fastack_check_lock);
DEFINE_SPINLOCK(speedup_stat_lock);
DEFINE_SPINLOCK(speedup_stat_check_lock);
DEFINE_SPINLOCK(speedup_stat_totalrx_lock);

struct _host_track_st {		
	u8 dstip[IP6_ADDR_LEN];	/* local ip(client) */
	u8 srcip[IP6_ADDR_LEN];	/* remote ip(server) */
	u8 ipver;
	u16 dport;	
	u16 sport;
	u16 ref_cnt;	//support more than 1 url download test. dont care it now.
	u8  direction;	//0-download 1-upload 2-both
	u8  active;
	u8  bypassFastAck;	//default 0, if set to 1, bypass ip_speedup module and deliver to socket.
	u8  sack;	//default 1, means sack-permitted: true
	//struct _speedup_stream_track *listHead;
	struct list_head hash[SPEEDUP_HASH_SIZE];
};
static struct _host_track_st host_track[10];

#define DELAY_WORK
#ifdef DELAY_WORK //delay_work
static struct delayed_work upload_work;
static struct delayed_work download_work;
#else
//static struct tasklet_struct upload_tx_tasklets;
static struct work_struct   upload_work;
static struct work_struct   download_work;
#endif

/* support up to 10 conntrack for speed test */
enum SPEEDUP_PROC_RT {
	SPEEDUP_STREAM_ADD,
	SPEEDUP_STREAM_DEL,
	SPEEDUP_STREAM_GET,
};

#define SAMPLE_NUM 60
static u8 sampletime=0;
static unsigned int hostspeeduprate[SAMPLE_NUM];//byte/s
static unsigned int bypassFastAckFlag=0;
static unsigned char forceFastAck=0;
static unsigned int speedupPathFixed=0;

unsigned short FAST_ACK_WIN_SIZE=0xff00;
static volatile unsigned int dataAckCnt=10;
static volatile unsigned int fastAckTimer = 100;	//unit:us
static volatile unsigned int sendDelayTime = 50;	//unit:us
static volatile unsigned int maxSendDelayTime = 20000;	//unit:us

static volatile unsigned int highRateAckThresh = 10;
static volatile unsigned int lowRateAckThresh = 5;
#ifdef CONFIG_RTL9607C
#define SPEEDTEST_TIMER_IDX	7
#else
#define SPEEDTEST_TIMER_IDX	3
#endif
unsigned int frozen_ack_num=2;
#define UPLOAD_BURST_NUM	5
unsigned int upload_burst_num=UPLOAD_BURST_NUM;
unsigned int upload_burst_num_update=0;
unsigned int upload_burst_num_update_threshold=25;//50/s
unsigned int upload_zero_wins_check_time=0;

//TODO
#ifdef CONFIG_ARCH_CORTINA
void __iomem *tmr_cnt2_addr=NULL;
#define REAL_SPEEDTEST_TIME  readl_relaxed(tmr_cnt2_addr)
/*
unsigned int get_real_speedtest_time(void)
{
	PER_TMR_CNT2_t tmr_cnt2;
	void __iomem *addr;
	addr=ioremap( PER_TMR_CNT2, 4);
	tmr_cnt2.wrd = readl_relaxed(addr);
	iounmap(addr);
	return tmr_cnt2.wrd;
}
*/
#else
#define REAL_SPEEDTEST_TIME	(REG32(BSP_TC0CNT + (SPEEDTEST_TIMER_IDX << 4)))
#endif

/* below variable for debug only */
#ifdef IP_SPEEDUP_DEBUG
static unsigned int duplicatePktCnt=0;
static unsigned int inOrderPktCnt=0;
static unsigned int outOrderPktCnt=0;
static unsigned int revisePktCnt=0;
static unsigned int delayAckPktCnt=0;
static unsigned int longDelayAckPktCnt=0;
static unsigned int greateoutOrderPktCnt=0;
unsigned int g_vec_nr[CONFIG_NR_CPUS]={10};
static char * softirq_new_name[11] = {
	"HI", "TIMER", "NET_TX", "NET_RX", "BLOCK", "BLOCK_IOPOLL",
	"TASKLET", "SCHED", "HRTIMER", "RCU", "UNKNOWN"
};
static unsigned int softirq_delay_stats[11];
static unsigned int delay_ack_num_on_cpu[CONFIG_NR_CPUS];
static unsigned int sack_exceed_num=0;
#endif
static unsigned int seqnoIntervalIgnoreThreshold=30000;
static unsigned int fastAckCalibrationStartTime=0;
static unsigned int fastAckCalibrationRxBytes=0;

static unsigned int speedtestSampleStartTime=0;
static unsigned int speedtestSampleRxBytes=0;
static unsigned long long speedtestTotalRxBytes=0;
static unsigned long speedtestRealtimeStart=0;
static unsigned long speedtestRealtimeEnd=0;

/*
 * new variable to indicate if upload testing is going...
 */
static unsigned int doUploadTest = 0;
static unsigned int doDownloadTest = 0;

//for debug
#ifdef IP_SPEEDUP_DEBUG
static unsigned int sack_ack_count = 0;
static unsigned int normal_xmit_count=0;
static unsigned int pause_xmit_count=0;
static unsigned int all_ack_count = 0;
static unsigned int all_ack_pause_count = 0;
static unsigned int workqueue_tx_pkt=0;
static unsigned int force_tx_count=0;
static unsigned int long_delay_count=0;
static unsigned int quick_xmit_pkt=0;
static unsigned int total_xmit_pkt=0;
static unsigned int ps_xmit_pkt=0;
static unsigned int zero_win_retrans=0;
static unsigned int zero_win_retrans_last_check=0;
static unsigned int force_retrans_count=0;
static unsigned int fail_xmit_pkt=0;
static unsigned int over_burst_count=0;
static unsigned int send_delay_time[100];
#endif
extern void __ip_select_ident(struct net *net, struct iphdr *iph, int segs);
static inline void host_track_on(u8* dstip, u8* srcip, u16 dport, u16 sport, int direction, int ipver);
static void setup_Timer_for_speedup(void);
static void upload_sack_try_remove_blk(struct _speedup_stream_track *pEntry);
extern void memDump (void *start, u32 size, char * strHeader);
//#define HOSTCHECKDBG
#ifdef HOSTCHECKDBG
#define HOSTPRINTK printk
#else
#define HOSTPRINTK(f,a...) do {} while (0)
#endif

#if 0
inline static u32
speedup_cpu_matched(void)
{
#ifdef CONFIG_SMP
	/* TODO: eth0 interrupt is exec in cpu 1 for 9607C, so speedup should only be implemented in cpu 1,
	 * eth0 interrupt is default mapping to CPU 0, which is defined in bsp_gic_intr_map array of irq.c
	 * while romedriver will remapping it to CPU 1 in rtl86900/romeDriver/rtk_rg_apollo_liteRomeDriver.c
	 * such as: irq_set_affinity(52, &cpumask); //GMAC9
	 *			cpu_mask = 0x2 -> CPU1
	 * if mapping relationship is changed, below patch should also be modified, remember!!!
	 */
#if CONFIG_NR_CPUS == 4
	if (1 != smp_processor_id())
		return 0;
#else
	if (0 != smp_processor_id())
		return 0;
#endif
#endif

	return 1;
}
#endif
static inline int ip_cmp(u8*ip1,u8*ip2,int ipver)
{
	int cmp_len=0;
	int i=0;
	if(ip1==NULL || ip2==NULL)
		return -2;
	if(ipver == 0)
		cmp_len=IP_ADDR_LEN;
	else
		cmp_len=IP6_ADDR_LEN;
		
	for(i=0; i<cmp_len; i++)
	{
		if(ip1[i] != ip2[i])
			break;
	}
	if(i==cmp_len)
		return 0;
	return -1;	
}

static inline void set_skb_tx_info(struct sk_buff *skb)
{
#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
#if defined(CONFIG_RTK_SKB_MARK2) 
	skb->mark2 |= (1<<speedtest_skip_fc_Bit);
#endif
#endif
}

inline static u32
speedup_hash_entry(u8* sip, u32 sport, u8* dip, u32 dport)
{
	register __u8 hash = 0;
	
	hash = (sip[0]^sip[1]^sip[2]^sip[3]);
	hash ^= (dip[0]^dip[1]^dip[2]^dip[3]);
	hash ^= ((sport>>8)^sport);
	hash ^= ((dport>>8)^dport);
	
	return (SPEEDUP_HASH_SIZE-1) & (hash ^ (hash >> 4));
}

static void resetSstTbl(void)
{
	int i;

	memset(sst, 0, sizeof(sst));

	INIT_LIST_HEAD(&sstFreeHeadList);
	INIT_LIST_HEAD(&sstAciveHeadList);
	for (i=0; i<SPEEDUP_CONN_NUM; i++)
	{
		INIT_LIST_HEAD(&sst[i].node);
		INIT_LIST_HEAD(&sst[i].activeNode);
		list_add_tail(&sst[i].node, &sstFreeHeadList);
	}
}

static void  retSSTEntryToFreeList(struct _speedup_stream_track * pEntry)
{
	list_add_tail(&pEntry->node, &sstFreeHeadList);
}

static struct _speedup_stream_track *getFreeSSTEntry(void)
{
	struct _speedup_stream_track *pEntry=NULL;

	if (list_empty(&sstFreeHeadList))
		return NULL;
	
	pEntry = list_first_entry(&sstFreeHeadList, struct _speedup_stream_track, node);
	list_del_init(&pEntry->node);
	
	return (pEntry);
}

static void delHostTrackEntry(struct _host_track_st *ht)
{
	struct _speedup_stream_track *pEntry, *pNextEntry;
	int i;

	for (i=0; i<SPEEDUP_HASH_SIZE; i++)
	{
		list_for_each_entry_safe(pEntry, pNextEntry, &ht->hash[i], node)
		{
			if (/*pEntry->direction &&*/ pEntry->pkthdr.skb)
			{
				dev_kfree_skb(pEntry->pkthdr.skb);
				pEntry->pkthdr.skb = NULL;
			}
			
			list_del(&pEntry->activeNode);		
			list_del(&pEntry->node);
			retSSTEntryToFreeList(pEntry);
		}
	}

	memset(ht, 0, sizeof(struct _host_track_st));
}

static void _host_track_init(void)
{
	int i, j;
	
	memset(&host_track, 0, sizeof(host_track));
	for (i=0; i<10; i++)
	{
		for (j=0; j<SPEEDUP_HASH_SIZE; j++)
			INIT_LIST_HEAD(&host_track[i].hash[j]);
	}
	resetSstTbl();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
	timer_setup(&ht_timer, host_timeout, 0);
#else
	init_timer(&ht_timer);
	ht_timer.data = (unsigned long)host_track;
	ht_timer.function = host_timeout;
#endif
}

static void fast_ack_calibration_init(void)
{
	fastAckCalibrationStartTime = 0;
	fastAckCalibrationRxBytes = 0;
}

void fast_ack_calibration_enter(int pktLen)
{
	//unsigned long flags;

	//local_irq_save(flags);
	if (0 == fastAckCalibrationStartTime)
		fastAckCalibrationStartTime = jiffies;
	spin_lock(&speedup_fastack_lock);
	fastAckCalibrationRxBytes += pktLen;
	spin_unlock(&speedup_fastack_lock);
	if (((jiffies - fastAckCalibrationStartTime) >= 50)	&& spin_trylock(&speedup_fastack_check_lock))//500ms
	{
		/*
		 * 700Mbps == 91750400Bps             half:45875200
		 * 600Mbps == 78643200Bps             half:39321600
		 * 500Mbps == 65536000Bps             half:32768000
		 * 300Mbps == 39321600Bps             half:19660800
		 * 100Mbps == 13107200Bps             half:6553600
		 */
		if (fastAckCalibrationRxBytes >= 19660800) //300Mbps
		{
			//FAST_ACK_WIN_SIZE = 0x4000;
			dataAckCnt = highRateAckThresh;
			//fastAckTimer = 50;
		}
		else if (fastAckCalibrationRxBytes <= 6553600) //100Mbps
		{
			dataAckCnt = lowRateAckThresh;
			//fastAckTimer = 50;
			//FAST_ACK_WIN_SIZE = 0x0800;
		}
	
		fastAckCalibrationRxBytes = 0;
		fastAckCalibrationStartTime = jiffies;
		spin_unlock(&speedup_fastack_check_lock);
	}
	//local_irq_restore(flags);
}

static void speedtest_sample_init(void)
{
	int i = 0;
	if(!doUploadTest)
	{
	speedtestSampleStartTime = 0;
	speedtestSampleRxBytes = 0;
	speedtestTotalRxBytes = 0;
	speedtestRealtimeStart = 0;
	speedtestRealtimeEnd = 0;
	if(forceFastAck)
		bypassFastAckFlag = 0;
	else
	bypassFastAckFlag = 1;
	speedupPathFixed = 0;

	memset(hostspeeduprate, 0, sizeof(hostspeeduprate));
	sampletime = 0;

#ifdef IP_SPEEDUP_DEBUG
	inOrderPktCnt = 0;
	outOrderPktCnt = 0;
	duplicatePktCnt = 0;
	revisePktCnt = 0;
	delayAckPktCnt = 0;
	longDelayAckPktCnt = 0;
	greateoutOrderPktCnt = 0;
	for (i = 0; i < CONFIG_NR_CPUS; i++)
	{
		g_vec_nr[i] = 10;
		delay_ack_num_on_cpu[i] = 0;
	}
	for (i = 0; i < 11; i++)
	{
		softirq_delay_stats[i] = 0;
	}
	sack_exceed_num=0;
#endif
	}
	else
	{
#ifdef IP_SPEEDUP_DEBUG
	sack_ack_count = 0;
	normal_xmit_count = 0;
	pause_xmit_count = 0;
	all_ack_count = 0;
	all_ack_pause_count = 0;
	force_tx_count = 0;
	long_delay_count = 0;
	over_burst_count = 0;
	workqueue_tx_pkt = 0;
	quick_xmit_pkt = 0;
	total_xmit_pkt = 0;
	fail_xmit_pkt = 0;
	ps_xmit_pkt = 0;
	zero_win_retrans = 0;
	zero_win_retrans_last_check = 0;
	force_retrans_count = 0;
#endif
	upload_zero_wins_check_time=0;
	upload_burst_num=UPLOAD_BURST_NUM;
	}
}

void speedtest_sample_enter(int pktLen)
{
	unsigned int time = REAL_SPEEDTEST_TIME;
	unsigned int diff;
	unsigned int rate;
	spin_lock(&speedup_stat_lock);
	speedtestSampleRxBytes += pktLen;
	spin_unlock(&speedup_stat_lock);
	if (0 == speedtestSampleStartTime)
	{
		speedtestSampleStartTime = time;
		return;
	}
	diff=get_time_duration(speedtestSampleStartTime);
#ifdef CONFIG_ARCH_CORTINA
	if ((diff >= CA_ONE_SEC_COUNTER) && spin_trylock(&speedup_stat_check_lock))	//1s
#else
	if ((diff >= 1000000) && spin_trylock(&speedup_stat_check_lock))	//1s
#endif
	{
		if (sampletime < SAMPLE_NUM)
		{
			rate = speedtestSampleRxBytes>>10;
#ifdef CONFIG_ARCH_CORTINA
			if ((rate > 100) /*&& (diff < (CA_ONE_SEC_COUNTER/10*11))*/)//discard any sample long than 1.1s
#else
			if ((rate > 100) /*&& (diff < 1100000)*/)//discard any sample long than 1.1s
#endif
			{
#ifdef CONFIG_ARCH_CORTINA
				diff /= (CA_ONE_SEC_COUNTER/1000);
#else
				diff /= 1000;			
#endif
				rate = rate*1000/diff;
				if (0 == sampletime)
				{
					hostspeeduprate[sampletime] = rate;
					//if rx rate lower than 10Mbps, go normal path
					if (rate > 1280)
						bypassFastAckFlag = 0;
					speedupPathFixed = 1;
				}
				else
					hostspeeduprate[sampletime] = hostspeeduprate[sampletime-1] + rate;
				
				sampletime++;
			}
			speedtestSampleRxBytes = 0;
		}
		
		speedtestSampleStartTime = time;
		spin_unlock(&speedup_stat_check_lock);
	}
}

void speedtest_payload_statistic(unsigned int payloadLen)
{
	spin_lock(&speedup_stat_totalrx_lock);
	if(speedtestRealtimeStart == 0)
		speedtestRealtimeStart=jiffies;
	speedtestRealtimeEnd=jiffies;
	speedtestTotalRxBytes += payloadLen;
	spin_unlock(&speedup_stat_totalrx_lock);
}

static void _speedup_stream_track_record(u8* dstip, u8* srcip, u16 dport, u16 sport, int direction, unsigned int wscale, unsigned int mss, int ipver)
{
	struct _speedup_stream_track *pEntry, *pNewEntry;
	unsigned int hashKey;
	int i;

	/* get host track entry */
	for (i=0; i<10; i++)
	{
		if (0 == host_track[i].active)
			break;
		
		if ((host_track[i].ipver == ipver) &&
			((host_track[i].dstip[0] == 0) || (ip_cmp(host_track[i].dstip, srcip, ipver) == 0)) &&
			((host_track[i].dport == 0) || (host_track[i].dport == sport)) &&
			(ip_cmp(host_track[i].srcip,dstip, ipver) == 0) &&
			((host_track[i].sport == 0) || (host_track[i].sport == dport)))
		{//rule alerasy existed.
			break;
		}
	}

	if ((i >= 10) || (0 == host_track[i].active))
	{
		if (i >= 10)
			return;
		else
		{
			//unsigned char zero_ip_addr[IP6_ADDR_LEN]={0};
			//host_track_on(zero_ip_addr, dstip, 0, dport, direction, ipver);//new entry will be save to host_track[i]
			host_track_on(srcip, dstip, sport, dport, direction, ipver);
		}
	}

	hashKey = speedup_hash_entry(srcip, sport, dstip, dport);
	list_for_each_entry(pEntry, &host_track[i].hash[hashKey], node)
	{
		if (((0 == pEntry->dstip[0]) || (ip_cmp(host_track[i].dstip,dstip, ipver) == 0)) &&
			((0 == pEntry->srcip[0]) || (ip_cmp(host_track[i].srcip,srcip, ipver) == 0)) &&
			((0 == pEntry->dport) || (pEntry->dport == dport)) &&
			((0 == pEntry->sport) || (pEntry->sport == sport)))
		{
			printk("%s speedup stream already exist!\n", __func__);
			return;
		}
	}

	pNewEntry = getFreeSSTEntry();
	if (NULL == pNewEntry)
	{
		printk("%s can not get free sst entry!\n", __func__);
		return;
	}
	if(ipver == 0)
	{
		struct iphdr iph;
		memcpy(pNewEntry->dstip, dstip, IP_ADDR_LEN);
		memcpy(pNewEntry->srcip, srcip, IP_ADDR_LEN);
		memcpy(&(iph.daddr), dstip, IP_ADDR_LEN);
		memcpy(&(iph.saddr), srcip, IP_ADDR_LEN);
		iph.protocol = IPPROTO_TCP;
		__ip_select_ident(&init_net, &iph, 1);
		pNewEntry->id = ntohs(iph.id);
	}
	else
	{
		memcpy(pNewEntry->dstip, dstip, IP6_ADDR_LEN);
		memcpy(pNewEntry->srcip, srcip, IP6_ADDR_LEN);
	}
	pNewEntry->dport = dport;
	pNewEntry->sport = sport;
	pNewEntry->totalbytes = 0;
	pNewEntry->rcv_nxt = 0;
	pNewEntry->pkthdr.skb = NULL;
	pNewEntry->sack_blk_num = 0;
	pNewEntry->upload_sack_blk_num = 0;
	pNewEntry->wscale = wscale;
	pNewEntry->snd_mss = mss;
	pNewEntry->sndPktNum = 0;
	pNewEntry->direction = direction;
	pNewEntry->seqNo = 0;
	pNewEntry->ack_seq = 0;
	pNewEntry->ackedSeqNo = 0;
	pNewEntry->frozenAckCnt = 0;
	pNewEntry->push = 0;
	pNewEntry->cnt = 0;
	spin_lock_init(&pNewEntry->sst_lock);

	list_add_tail(&pNewEntry->node, &host_track[i].hash[hashKey]);
	list_add_tail(&pNewEntry->activeNode, &sstAciveHeadList);
	//printk("%s add stream src %x/%d dst %x/%d dir %s\n", __func__, srcip, sport, dstip, dport, direction?"upload":"download");
}

static void _speedup_stream_track_del(u8* dstip, u8* srcip, u16 dport, u16 sport, int ipver)
{
	struct _speedup_stream_track *pEntry, *pNextEntry;
	unsigned int hashKey;
	int i;

	/* get host track entry */
	for (i=0; i<10; i++)
	{
		if (0 == host_track[i].active)
			break;

		if ((host_track[i].ipver == ipver) &&
			((host_track[i].dstip[0] == 0) || (ip_cmp(host_track[i].dstip, srcip, ipver) == 0)) &&
			((host_track[i].dport == 0) || (host_track[i].dport == sport)) &&
			(ip_cmp(host_track[i].srcip,dstip, ipver) == 0) &&
			((host_track[i].sport == 0) || (host_track[i].sport == dport)))
		{//rule alerasy existed.
			break;
		}
	}

	if ((i >= 10) || (0 == host_track[i].active))
	{
		if(ipver)
			printk("%s can not find speedup stream track entry (%pI6c/%d -> %pI6c/%d)\n", __func__, srcip, sport, dstip, dport);
		else
			printk("%s can not find speedup stream track entry (%pI4/%d -> %pI4/%d)\n", __func__, srcip, sport, dstip, dport);
		return;
	}

	hashKey = speedup_hash_entry(srcip, sport, dstip, dport);

	list_for_each_entry_safe(pEntry, pNextEntry, &host_track[i].hash[hashKey], node)
	{
		if (((0 == pEntry->dstip[0]) || (ip_cmp(pEntry->dstip, dstip, ipver) == 0)) &&
			((0 == pEntry->srcip[0]) || (ip_cmp(pEntry->srcip, srcip, ipver) == 0)) &&
			((0 == pEntry->dport) || (pEntry->dport == dport)) &&
			((0 == pEntry->sport) || (pEntry->sport == sport)))
		{
			if (/*pEntry->direction &&*/ pEntry->pkthdr.skb)
			{
				dev_kfree_skb(pEntry->pkthdr.skb);
				pEntry->pkthdr.skb = NULL;
			}
			list_del_init(&pEntry->node);		
			list_del_init(&pEntry->activeNode);
			retSSTEntryToFreeList(pEntry);
			break;
		}
	}
	host_track[i].active = 0;
	//printk("%s del stream src %x/%d dst %x/%d\n", __func__, srcip, sport, dstip, dport);
}

int HostSpeedUP_Stat_get(u8* dstip, u8* srcip, u16 dport, u16 sport,int ipver)
{
	struct _speedup_stream_track *pEntry;
	unsigned int hashKey;
	u32 rcvBytes;
	int i;

	/* get host track entry */
	for (i=0; i<10; i++)
	{
		if (0 == host_track[i].active)
			break;

		if ((host_track[i].ipver == ipver) &&
			((host_track[i].dstip[0] == 0) || (ip_cmp(host_track[i].dstip, srcip, ipver) == 0)) &&
			((host_track[i].dport == 0) || (host_track[i].dport == sport)) &&
			(ip_cmp(host_track[i].srcip,dstip, ipver) == 0) &&
			((host_track[i].sport == 0) || (host_track[i].sport == dport)))
		{//rule alerasy existed.
			break;
		}
	}

	if ((i >= 10) || (0 == host_track[i].active))
	{
		if(ipver)
			printk("%s can not find speedup stream track entry (%pI6c/%d -> %pI6c/%d)\n", __func__, srcip, sport, dstip, dport);
		else
			printk("%s can not find speedup stream track entry (%pI4/%d -> %pI4/%d)\n", __func__, srcip, sport, dstip, dport);
		
		return 0;
	}

	hashKey = speedup_hash_entry(srcip, sport, dstip, dport);
	list_for_each_entry(pEntry, &host_track[i].hash[hashKey], node)
	{
		if (((0 == pEntry->dstip[0]) || (ip_cmp(pEntry->dstip,dstip, ipver) == 0)) &&
			((0 == pEntry->srcip[0]) || (ip_cmp(pEntry->srcip,srcip, ipver) == 0)) &&
			((0 == pEntry->dport) || (pEntry->dport == dport)) &&
			((0 == pEntry->sport) || (pEntry->sport == sport)))
		{
			spin_lock_bh(&pEntry->sst_lock);
			rcvBytes = pEntry->totalbytes;
			pEntry->totalbytes = 0;
			spin_unlock_bh(&pEntry->sst_lock);
			return (rcvBytes);
		}
	}

	return 0;
}

#if 0
void print_debug_info(u32 dstip, u32 srcip, u16 dport, u16 sport)
{
	struct _speedup_stream_track *pEntry, *pNextEntry;
	unsigned int hashKey;
	int i, j;
	int streamNum=0;

	hashKey = speedup_hash_entry(dstip, dport, srcip, sport);
	printk("[unknown %d] %x/%d -> %x/%d\n", hashKey, srcip, sport, dstip, dport);
	printk("	stream table list:\n");
	for (i=0; i<10; i++)
	{
		for (j=0; j<SPEEDUP_HASH_SIZE; j++)
		{
			list_for_each_entry_safe(pEntry, pNextEntry, &host_track[i].hash[j], node)
			{
				if (((0 == pEntry->dstip) || (pEntry->dstip == srcip)) &&
					((0 == pEntry->srcip) || (pEntry->srcip == dstip)) &&
					((0 == pEntry->dport) || (pEntry->dport == sport)) &&
					((0 == pEntry->sport) || (pEntry->sport == dport)))
				{
					printk("	match in hash %d, calc hash is %d\n", j, hashKey);
					return;
				}
				streamNum++;
				printk("	[%d]stream %d src %x/%d dst %x/%d\n", j, streamNum, pEntry->srcip, pEntry->sport,
							pEntry->dstip, pEntry->dport);
			}
		}
	}
}
#endif

unsigned int get_speedtest_time(void)
{
	return REAL_SPEEDTEST_TIME;
}

inline unsigned int get_time_duration(unsigned int startTime)
{
	unsigned int currtime;
	unsigned int duration;	//unit us;

	currtime = REAL_SPEEDTEST_TIME;
#ifdef CONFIG_ARCH_CORTINA
	if (currtime <= startTime)
		duration = startTime-currtime;
	else
		duration = 0xFFFFFFF - currtime + startTime;
#else
	if (currtime >= startTime)
		duration = currtime - startTime;
	else
		duration = 0xFFFFFFF - startTime + currtime;
#endif
	return duration;
}

/*
 * RETURN VALUE: 1 means we should send ack now; 0 means don't need to send ack
 */
static inline int should_ack_now(unsigned int rcvtime)
{
	unsigned int duration;	//unit us;

	duration = get_time_duration(rcvtime);

#ifdef IP_SPEEDUP_DEBUG
#ifdef CONFIG_ARCH_CORTINA
	if (duration >= (CA_ONE_SEC_COUNTER/10000))
#else
	if (duration >= 100)
#endif
	{
		longDelayAckPktCnt++;
		softirq_delay_stats[g_vec_nr[smp_processor_id()]]++;
	}
#endif
#ifdef CONFIG_ARCH_CORTINA
	if (duration >= 2*fastAckTimer)//ACK NOW
#else
	if (duration >= fastAckTimer)//ACK NOW
#endif
		return 1;
	
	return 0;
}

static inline int should_send_now(unsigned int sndtime, unsigned int delayTime)
{
	unsigned int duration;	//unit us;

	duration = get_time_duration(sndtime);
#ifdef CONFIG_ARCH_CORTINA
	if (duration >= delayTime*2)
#else
	if (duration >= delayTime)
#endif
		return 1;
	
	return 0;
}


#ifdef IP_SPEEDUP_DEBUG
unsigned int get_softirq_delay_stats(int vec_nr)
{
	return (softirq_delay_stats[vec_nr]);
}
#endif

static void tcp_option_write(struct tcphdr *tcph, speedup_pktHdr_t *pPktHdr, struct _speedup_stream_track *pEntry)
{
	int valid_sacks = 0;
	unsigned int *ptr;
	int tcp_doff = 5;
	int opt_ts = 1, i;
	unsigned int tsval, tsecr;

	if ((TCP_NUM_SACKS == pEntry->sack_blk_num) || 
		(pPktHdr->f_isdsack && (pEntry->sack_blk_num >= 3)))
		opt_ts = 0;

	if (opt_ts)
	{
		/* search timestamp option */
		char *pTmp = (unsigned char *)(tcph + 1);
		int optLen = (tcph->doff<<2) - sizeof(struct tcphdr);

		while (optLen > 0) {
			int opcode = *pTmp++;
			int opsize;

			switch (opcode) {
				case TCPOPT_EOL:	/* EOL */
					optLen = 0;
					break;
				case TCPOPT_NOP:	/* NOP Ref: RFC 793 section 3.1 */
					optLen--;
					break;
				default:
					opsize = *pTmp++;
					if (opsize < 2) /* "silly options" */
					{
						optLen = 0;
						break;
					}
					if (opsize > optLen)
					{
						optLen = 0; /* don't parse partial options */
						break;
					}
					if (TCPOPT_TIMESTAMP == opcode)	/* timestamps */
					{
						tsecr = *(unsigned int *)pTmp;
						tsval = *(unsigned int *)(pTmp + 4);
						
						/* TS options found, stop parse any other options */
						do {
							ptr = (unsigned int *)((__u32 *)tcph + tcp_doff);
							*ptr++ = htonl((TCPOPT_NOP << 24) |
									   (TCPOPT_NOP << 16) |
									   (TCPOPT_TIMESTAMP << 8) |
									   TCPOLEN_TIMESTAMP);
							*ptr++ = tsval;
							*ptr++ = tsecr;
							
							tcp_doff += 3;
						} while (0);
						
						optLen = 0;
						break;
					}
					pTmp += opsize-2;
					optLen -= opsize;
					break;
			}
		}
	}
	
	if (pPktHdr->f_isdsack)
	{
		if (pEntry->sack_blk_num > 0)
		{
			valid_sacks = (12 - tcp_doff)>>1;
			valid_sacks = (valid_sacks < pEntry->sack_blk_num)?valid_sacks:pEntry->sack_blk_num;
		}

		ptr = (unsigned int *)((__u32 *)tcph + tcp_doff);
		/* block0 is DSACK, others are sack */
		*ptr++ = htonl((TCPOPT_NOP << 24) |
						(TCPOPT_NOP << 16) |
						(TCPOPT_SACK << 8) |
						(TCPOLEN_SACK_BASE + ((valid_sacks + 1)<<3)/*((valid_sacks + 1) * TCPOLEN_SACK_PERBLOCK)*/));
		/* block 0 for DSACK */
		if (pPktHdr->seqNo < pEntry->rcv_nxt)
		{
			*ptr++ = htonl(pPktHdr->seqNo);
			*ptr++ = htonl(pEntry->rcv_nxt);
		}
		else
		{
			*ptr++ = htonl(pPktHdr->seqNo);
			*ptr++ = htonl(pPktHdr->seqNo+pPktHdr->dataLen);
		}
		/* other sack blocks */
		for (i = 0; i < valid_sacks; ++i)
		{
			*ptr++ = htonl(pEntry->sack_blk[i].start_seq);
			*ptr++ = htonl(pEntry->sack_blk[i].end_seq);
		}

		tcp_doff += 3 + /* DSACK block: nop + nop + kind + length + left edge + right edge */
					(valid_sacks<<1);
	}
	else if (pEntry->sack_blk_num > 0)
	{
		valid_sacks = (14 - tcp_doff)>>1;
		valid_sacks = (valid_sacks < pEntry->sack_blk_num)?valid_sacks:pEntry->sack_blk_num;

		ptr = (unsigned int *)((__u32 *)tcph + tcp_doff);
		*ptr++ = htonl((TCPOPT_NOP	<< 24) |
				   (TCPOPT_NOP	<< 16) |
				   (TCPOPT_SACK <<	8) |
				   (TCPOLEN_SACK_BASE + (valid_sacks<<3)/*(valid_sacks * TCPOLEN_SACK_PERBLOCK)*/));
		
		for (i = 0; i < valid_sacks; ++i)
		{
			*ptr++ = htonl(pEntry->sack_blk[i].start_seq);
			*ptr++ = htonl(pEntry->sack_blk[i].end_seq);
		}
		
		tcp_doff += 1 + /* SACK block: nop + nop + kind + length */
					(valid_sacks<<1);
	}
	
	tcph->doff = tcp_doff;
}

static void generate_upload_cached_skb(struct _speedup_stream_track *pEntry, speedup_pktHdr_t *pPktHdr, struct sk_buff *skb, int swap_addr)
{
	struct sk_buff *new_skb;
	struct tcphdr *tcph;
	struct iphdr *iph;
	struct ipv6hdr *ip6h;
	unsigned char *pL2Hdr;
	unsigned char *pPPPoEHdr;
	unsigned int l3Len=0, l4Len=0;
	unsigned int tmp;
	int i;

	//printk("%s enter!\n", __func__);
#ifdef CONFIG_ARCH_CORTINA
	new_skb = skb_copy_expand(skb, 4, 1514-skb->len, GFP_ATOMIC);
#else
	new_skb = skb_copy_expand(skb, 0, 1514-skb->len, GFP_ATOMIC);
#endif
	if (new_skb == NULL)
	{
		printk("%s %d no more memory for speedtest\n", __func__, __LINE__);
		return;
	}
	memcpy(&pEntry->pkthdr, pPktHdr, sizeof(speedup_pktHdr_t));
	pEntry->pkthdr.pStreamInfo = (void *)pEntry;
	pEntry->pkthdr.skb = new_skb;

	pL2Hdr = pEntry->pkthdr.pL2Hdr = new_skb->data;
	pEntry->pkthdr.pL3Hdr = new_skb->data + pEntry->pkthdr.l3HdrOffset;
	pEntry->pkthdr.pL4Hdr = new_skb->data + pEntry->pkthdr.l4HdrOffset;
	
	if (pEntry->pkthdr.f_ispppoe) {
		pPPPoEHdr = new_skb->data + pEntry->pkthdr.pppoeHdrOffset;
	}
	
	new_skb->dev = pPktHdr->skb->dev;
	new_skb->mark = pPktHdr->skb->mark;
#if defined(CONFIG_RTK_SKB_MARK2) 
	new_skb->mark2 = pPktHdr->skb->mark2;
#endif
	if(pEntry->snd_mss == 0)
		pEntry->snd_mss = new_skb->dev->mtu - pPktHdr->l4HdrOffset - sizeof(struct tcphdr);

	tcph = (struct tcphdr*)(pEntry->pkthdr.pL4Hdr);
	pEntry->ack_seq = ntohl(tcph->seq);
	
	/* remove tcp option */
	tcph->doff = 5;
	l4Len = (tcph->doff<<2);

	if(pEntry->direction != 0) {
		//pEntry->sndPktNum = upload_burst_num;
		pEntry->sndPktNum = 0;
		pEntry->push = 1;
		tmp = max_buf_len - pPktHdr->l4HdrOffset - sizeof(struct tcphdr);
		tmp = (tmp / pEntry->snd_mss) * pEntry->snd_mss;
		tmp = (tmp <= 0) ? pEntry->snd_mss : tmp;
		pEntry->payload_len = tmp;
		pEntry->burstDataSize = UPLOAD_BURST_NUM*max_buf_len*2;
		tcph->window = htons(pEntry->burstDataSize / pEntry->wscale);
	}
	else {
		pEntry->sndPktNum = 0;
		pEntry->payload_len = 0;
		pEntry->burstDataSize = 0;
		tcph->window = htons((pEntry->snd_mss * 120) / pEntry->wscale);
	}
	//printk("===> [%s] skb %p dev %s payload %d burst %d L2 %p L3 %p L4 %p window %u wscale %d\n", __FUNCTION__, new_skb, new_skb->dev->name, pEntry->payload_len, pEntry->burstDataSize, pEntry->pkthdr.pL2Hdr, pEntry->pkthdr.pL3Hdr, pEntry->pkthdr.pL4Hdr, ntohs(tcph->window), pEntry->wscale);
	
	new_skb->len = pEntry->pkthdr.l4HdrOffset + l4Len;
	//new_skb->tail = new_skb->data + new_skb->len;
	
	/* modify mac header */
	if(swap_addr) {
		for (i=0; i<3; i++)
		{
			tmp = *((unsigned short *)pL2Hdr + i);
			*((unsigned short *)pL2Hdr +i) = *((unsigned short *)pL2Hdr + 3 + i);
			*((unsigned short *)pL2Hdr + 3 + i) = tmp;
		}
	}
	
	/* modify pppoe header */
	if (pEntry->pkthdr.f_ispppoe)
	{
		/* modify pppoe length field */
		*(unsigned short *)(pPPPoEHdr + 4) = htons(l3Len + l4Len + pEntry->payload_len + 2);
	}

	/*modify tcp header */
	if(swap_addr) {
		tmp = tcph->source;
		tcph->source = tcph->dest;
		tcph->dest = tmp;
		
		pEntry->pkthdr.sport = tcph->source;
		pEntry->pkthdr.dport = tcph->dest;
	}
	//tcph->ack_seq = tcph->seq;
	tcph->seq = htonl(pEntry->seqNo);
	//Flags
	*((unsigned char *)tcph + 13) = 0;
	tcph->ack = 1;
	tcph->check = 0;
	new_skb->csum = csum_partial((void *)(tcph+1), pEntry->payload_len, 0);

    /*modify IP Header */
	if(pPktHdr->f_isipv4)
	{
		iph = (struct iphdr *)(pEntry->pkthdr.pL3Hdr);
		iph->tot_len = htons(l3Len + l4Len + pEntry->payload_len);
		iph->frag_off = htons(IP_DF);
		iph->ttl = 64;
		if(swap_addr) {
			tmp = iph->saddr;
			iph->saddr = iph->daddr;
			iph->daddr = tmp;
			
			memcpy(pPktHdr->saddr, &(iph->saddr), IP_ADDR_LEN);
			memcpy(pPktHdr->daddr, &(iph->daddr), IP_ADDR_LEN);
		}
		iph->id = htons(pEntry->id++);
		ip_send_check(iph);

		tcph->check = tcp_v4_check(l4Len+pEntry->payload_len, iph->saddr, iph->daddr,
							 csum_partial(tcph,
									  tcph->doff << 2,
									  new_skb->csum));
	}
	else
	{
		u8 addr_tmp[IP6_ADDR_LEN];
		ip6h = (struct ipv6hdr *)(pEntry->pkthdr.pL3Hdr);	
		ip6h->payload_len = htons(l3Len + l4Len + pEntry->payload_len - 40);
		ip6h->hop_limit = 64;
		if(swap_addr) {
			memcpy(addr_tmp, &(ip6h->saddr), IP6_ADDR_LEN);
			memcpy(&(ip6h->saddr), &(ip6h->daddr), IP6_ADDR_LEN);
			memcpy(&(ip6h->daddr), addr_tmp, IP6_ADDR_LEN);
			
			memcpy(pPktHdr->saddr, &(iph->saddr), IP6_ADDR_LEN);
			memcpy(pPktHdr->daddr, &(iph->daddr), IP6_ADDR_LEN);
		}
		tcph->check = tcp_v6_check(l4Len+pEntry->payload_len, &(ip6h->saddr), &(ip6h->daddr),
							 csum_partial(tcph,
									  tcph->doff << 2,
									  new_skb->csum));
	}
}

int _send_upload_data(struct _speedup_stream_track *pEntry)
{
	struct sk_buff *skb = NULL;
	struct tcphdr *tcph;
	struct iphdr *iph;
	struct ipv6hdr *ipv6h;
	int max_push_len;
	int totoal_len;
	int payload_len;
	int tot_len, l4Len;
	netdev_tx_t tx_ret;
#ifdef SKB_CACHE_ALLOC
	void *data;
#endif

	//printk("%s enter!\n", __func__);
	if (pEntry->pkthdr.skb == NULL)
		return 0;

	l4Len = sizeof(struct tcphdr);

#ifdef SKB_CACHE_ALLOC
	data = kmem_cache_alloc(skb_upload_cache, GFP_ATOMIC);
//printk("[%s:%d] data %p\n", __func__, __LINE__, data);
	if(data) {
		skb = build_skb(data, 0);
		if(skb) {
			skb_reserve(skb, NET_SKB_PAD);
			totoal_len = skb_tailroom(skb);
			if(pEntry->frozenAckCnt >= frozen_ack_num)
				payload_len = pEntry->snd_mss;
			else 
			{
				payload_len = totoal_len - (pEntry->pkthdr.l4HdrOffset + l4Len);
				if(payload_len > pEntry->payload_len)
					payload_len = pEntry->payload_len;
				payload_len = (payload_len/pEntry->snd_mss) * pEntry->snd_mss;
			}
			totoal_len = payload_len + (pEntry->pkthdr.l4HdrOffset + l4Len);
		}
		//printk("[%s:%d] totoal_len %d payload_len %d\n", __func__, __LINE__, totoal_len, payload_len);
	}
	
	if(unlikely(data == NULL || skb == NULL))
	{
		printk_ratelimited("[%s:%d] no more memory for speedtest\n", __func__, __LINE__);
		if(data) kmem_cache_free(skb_upload_cache, data);
		return 0;
	}
#else
	if(pEntry->frozenAckCnt >= frozen_ack_num)
		payload_len = pEntry->snd_mss;
	else 
	{
		payload_len = pEntry->payload_len;
		payload_len = (payload_len/pEntry->snd_mss) * pEntry->snd_mss;
	}
	totoal_len = payload_len + pEntry->pkthdr.l4HdrOffset + l4Len;

	//skb = skb_copy(pEntry->pkthdr.skb, GFP_ATOMIC);
	skb = alloc_skb(totoal_len, GFP_ATOMIC);//dev_alloc_skb(totoal_len);
	
	if (skb == NULL)
	{
		printk("%s %d no more memory for speedtest\n", __func__, __LINE__);
		return 0;
	}
#endif
	skb_put(skb, totoal_len);
	memcpy(skb->data, pEntry->pkthdr.skb->data, (pEntry->pkthdr.l4HdrOffset + l4Len));
	skb->dev = pEntry->pkthdr.skb->dev;
	skb->mark = pEntry->pkthdr.skb->mark;
#if defined(CONFIG_RTK_SKB_MARK2) 
	skb->mark2 = pEntry->pkthdr.skb->mark2;
#endif
	skb_shinfo(skb)->gso_size = pEntry->snd_mss;
	skb_shinfo(skb)->nr_frags = 0;

	tcph = (struct tcphdr*)(skb->data + pEntry->pkthdr.l4HdrOffset);
	tcph->seq = htonl(pEntry->seqNo);
	tcph->doff = l4Len >> 2;
	//tcph->window = htons((payload_len * pEntry->sndPktNum) / pEntry->wscale);
	
	max_push_len = pEntry->snd_mss * 10;
	tcph->psh = 0;
	pEntry->last_push_len += payload_len;
	if(pEntry->frozenAckCnt >= frozen_ack_num || payload_len >= max_push_len || pEntry->last_push_len >= max_push_len) {
		tcph->psh = 1;
		pEntry->last_push_len = 0;
	}
	
	/* calculate L3+L4+payloadLen */
	tot_len = pEntry->pkthdr.l4HdrOffset - pEntry->pkthdr.l3HdrOffset + l4Len + payload_len;

	if(pEntry->pkthdr.f_isipv4)
	{
		unsigned short id_org;
		iph = (struct iphdr *)(skb->data + pEntry->pkthdr.l3HdrOffset);
		id_org = iph->id;
		iph->id = htons(pEntry->id++);
		iph->tot_len = htons(tot_len);	
	}
	else {
		ipv6h = (struct ipv6hdr*)(skb->data + pEntry->pkthdr.l3HdrOffset);
		ipv6h->payload_len = htons(l4Len + payload_len);
	}
	
	if (pEntry->pkthdr.f_ispppoe)
	{
		struct pppoe_hdr *pppoe;
		pppoe = (struct pppoe_hdr *)(skb->data + pEntry->pkthdr.pppoeHdrOffset);
		pppoe->length = htons(tot_len + 2);
	}

	/* speedtest skb can be free in bottomhalf, it will be helpfull to reduce loading */
	*(unsigned int *)skb->cb = 0xFEEDFEED;
	
	skb->ip_summed = CHECKSUM_COMPLETE;
#ifdef CONFIG_ARCH_CORTINA
	//skb_reset_mac_header(skb);
	skb_set_network_header(skb, pEntry->pkthdr.l3HdrOffset);
	skb_set_transport_header(skb, pEntry->pkthdr.l4HdrOffset);
#endif

	set_skb_tx_info(skb);
	tx_ret = skb->dev->netdev_ops->ndo_start_xmit(skb, skb->dev);
	
	if(tx_ret != NETDEV_TX_OK) {
		//dev_kfree_skb_any(skb);
#ifdef IP_SPEEDUP_DEBUG
		fail_xmit_pkt++;
#endif
		return -1;
	}
	pEntry->seqNo += payload_len;
	if(pEntry->frozenAckCnt < frozen_ack_num)
		pEntry->totalbytes += payload_len;
	pEntry->sndPktNum--;
	//printk("[%s] seq %x ackedseq %x payload_len %d\n",__func__,pEntry->seqNo,pEntry->ackedSeqNo, payload_len);

	upload_sack_try_remove_blk(pEntry);
#ifdef IP_SPEEDUP_DEBUG
#ifdef CONFIG_ARCH_CORTINA
	if (get_time_duration(pEntry->sndtime) > 1000)
#else
	if (get_time_duration(pEntry->sndtime) > 500)
#endif
		long_delay_count++;
#endif
	pEntry->sndtime = REAL_SPEEDTEST_TIME;
#ifdef IP_SPEEDUP_DEBUG
	total_xmit_pkt++;
#endif

	return 1;
}
#ifdef DELAY_WORK
static inline void schedule_upload_work(void)
{
	//queue_delayed_work(system_highpri_wq, &upload_work, 0);
	mod_delayed_work(system_highpri_wq, &upload_work, 0);
}

static inline void schedule_upload_work_delay(void)
{
	queue_delayed_work(system_highpri_wq, &upload_work, 1*(CONFIG_HZ));
}

static inline void schedule_download_work(void)
{
	//schedule_delayed_work(&download_work, 0);
	mod_delayed_work(system_highpri_wq, &download_work, 0);
}

static inline void schedule_download_work_delay(void)
{
	queue_delayed_work(system_highpri_wq, &download_work, 1*(CONFIG_HZ));
}
#else
static inline void schedule_upload_work(void)
{
	queue_work(system_highpri_wq, &upload_work);
}

static inline void schedule_download_work(void)
{
	schedule_work(&download_work);
}
#endif
void send_upload_data_now(speedup_pktHdr_t *pPktHdr)
{
	struct _speedup_stream_track *pEntry = (struct _speedup_stream_track *)pPktHdr->pStreamInfo;
	//unsigned int count=0;

	//printk("%s enter!\n", __func__);
	if (spin_trylock(&pEntry->sst_lock))
	{
		while (pEntry->sndPktNum)
		{
			if (pEntry->ackedSeqNo && (pEntry->seqNo > (pEntry->ackedSeqNo + pEntry->burstDataSize)))
				break;

			if (!_send_upload_data(pEntry))
				break;
#ifdef IP_SPEEDUP_DEBUG
			quick_xmit_pkt++;
#endif
			//if (count++ >= 1)
				break;
		}
		spin_unlock(&pEntry->sst_lock);
		
		if (pEntry->sndPktNum)
			schedule_upload_work();
	}
	else
		schedule_upload_work();
	//printk("%s exit!\n", __func__);
}

void start_xmit_upload_data(void)
{
	struct _speedup_stream_track *pEntry;

	//printk("%s enter!\n", __func__);
	read_lock_bh(&speedup_lock);
	list_for_each_entry(pEntry, &sstAciveHeadList, activeNode)
	{
		if (0 == pEntry->direction)
		{
			continue;
		}

		if (0 == pEntry->sndPktNum)
		{
			continue;
		}
		
		if (spin_trylock(&pEntry->sst_lock))
		{
			while (pEntry->sndPktNum > 0)
			{
				if (pEntry->ackedSeqNo && (pEntry->seqNo > (pEntry->ackedSeqNo + pEntry->burstDataSize)))
					break;

				if (!_send_upload_data(pEntry))
					break;

#ifdef IP_SPEEDUP_DEBUG
				ps_xmit_pkt++;
#endif
			}
			spin_unlock(&pEntry->sst_lock);
		}
		continue;
	}
	read_unlock_bh(&speedup_lock);
	//printk("%s exit!\n", __func__);
}
void update_upload_burst_num(void)
{
	unsigned int diff=0;
	if(!upload_burst_num_update)
		return;
	if(upload_zero_wins_check_time == 0)
	{
		upload_zero_wins_check_time = REAL_SPEEDTEST_TIME;
		return;
	}
	else
		diff = get_time_duration(upload_zero_wins_check_time);

#ifdef CONFIG_ARCH_CORTINA
	if (diff < (CA_ONE_SEC_COUNTER/2)) //0.5s
#else
	if (diff < (1000000/2))	//0.5s
#endif
		return;

	upload_zero_wins_check_time = REAL_SPEEDTEST_TIME;
	if((zero_win_retrans-zero_win_retrans_last_check) > upload_burst_num_update_threshold)
	{
		if((zero_win_retrans-zero_win_retrans_last_check) > (upload_burst_num_update_threshold*2))
			upload_burst_num /= 2;
		else
			upload_burst_num--;
		printk("upload burst num down to:%u\n", upload_burst_num);
	}
	else if(zero_win_retrans == zero_win_retrans_last_check)
	{
		upload_burst_num++;
		printk("upload burst num up to:%u\n", upload_burst_num);
	}
	if(upload_burst_num < 3)
		upload_burst_num = 3;
	else if(upload_burst_num > UPLOAD_BURST_NUM)
		upload_burst_num = UPLOAD_BURST_NUM;
	zero_win_retrans_last_check = zero_win_retrans;
}
void wq_do_upload_speedtest(struct work_struct *p_work)
{
	struct _speedup_stream_track *pEntry;
	int activeFlag, aliveFlag;
	unsigned int send_len;

	//printk("%s enter!\n", __func__);
	read_lock_bh(&speedup_lock);
	aliveFlag = activeFlag = 0;
	list_for_each_entry(pEntry, &sstAciveHeadList, activeNode) {
		if (0 == pEntry->direction)
			continue;

		aliveFlag++;
		
		if (spin_trylock(&pEntry->sst_lock)) {
			send_len = 0;
			if(pEntry->seqNo > pEntry->ackedSeqNo) 
				send_len = pEntry->seqNo - pEntry->ackedSeqNo;
			
			if ((0 == pEntry->sndPktNum) && should_send_now(pEntry->sndtime, maxSendDelayTime)) {
				pEntry->sndPktNum = 1;
				pEntry->push = 1;
#ifdef IP_SPEEDUP_DEBUG
				force_tx_count++;
#endif
			}
			else if (send_len > pEntry->burstDataSize) {
#ifdef IP_SPEEDUP_DEBUG
				over_burst_count++;
#endif
				if (pEntry->sndPktNum)
					activeFlag ++;
				spin_unlock(&pEntry->sst_lock);
				continue;
			}

			if (pEntry->sndPktNum > 0) {
				if(pEntry->frozenAckCnt > frozen_ack_num) {
					force_retrans_count++;
					pEntry->frozenAckCnt = 0;
					pEntry->seqNo = pEntry->ackedSeqNo;
				}

				if (_send_upload_data(pEntry)) {
#ifdef IP_SPEEDUP_DEBUG
					workqueue_tx_pkt++;
#endif
				}
			}

			if (pEntry->sndPktNum)
				activeFlag ++;

			spin_unlock(&pEntry->sst_lock);
		}
		update_upload_burst_num();
	}
	read_unlock_bh(&speedup_lock);
	
	//if (activeFlag && xmitFlag)
	//	goto do_xmit;

	if (activeFlag)
		schedule_upload_work();
#ifdef DELAY_WORK
	else if(aliveFlag)
		schedule_upload_work_delay();
#endif
	//printk("%s exit!\n", __func__);
}

void wq_do_download_speedtest(struct work_struct *p_work)
{
	struct _speedup_stream_track *pEntry;
	int activeFlag, aliveFlag;
	//struct sk_buff*skb=NULL;

	if (!doUploadTest)
		return;
	
	//printk("%s enter!\n", __func__);
	if (doUploadTest)
		fastAckTimer = 500;
	else
		fastAckTimer = 100;
	
	read_lock_bh(&speedup_lock);

	aliveFlag = activeFlag = 0;
	list_for_each_entry(pEntry, &sstAciveHeadList, activeNode)
	{
		if (0 == pEntry->direction)
		{
			/* ack for download stream */
			spin_lock(&pEntry->sst_lock);
			if (NULL == pEntry->pkthdr.skb)//no ack should be sent
			{
				spin_unlock(&pEntry->sst_lock);
				continue;
			}

			if ((pEntry->cnt < dataAckCnt) && !should_ack_now(pEntry->rcvtime))
			{
				spin_unlock(&pEntry->sst_lock);
				aliveFlag++;
				continue;
			}

			/* send ack immediately */
			delay_ack_xmit(&pEntry->pkthdr);
#ifdef IP_SPEEDUP_DEBUG
			delayAckPktCnt++;
#endif
			aliveFlag++;
			spin_unlock(&pEntry->sst_lock);

			continue;
		}
	}

	read_unlock_bh(&speedup_lock);

	if (aliveFlag)
#ifdef  DELAY_WORK
		schedule_download_work_delay();
#else
		schedule_download_work();
#endif
	//printk("%s exit!\n", __func__);
}

/*
 * modify history:
 * 1. TCP options size should be up to 40, so only 4 SACK blocks should be included in a packet
 *    make sure doff <= 15
 */

int delay_ack_xmit(speedup_pktHdr_t *pPktHdr)
{
	struct _speedup_stream_track *pEntry = (struct _speedup_stream_track *)pPktHdr->pStreamInfo;
	struct sk_buff *skb = pPktHdr->skb;
	struct tcphdr *tcph;
	struct iphdr *iph;
	struct ipv6hdr *ip6h;
	unsigned int l4Len, len, totoal_len;
	int ret=1;
	
	if (pEntry == NULL || skb == NULL || pEntry->pkthdr.skb == NULL) {
		return 0;
	}
	
	//40 bytes as tcp option
	totoal_len = pEntry->pkthdr.l4HdrOffset + sizeof(struct tcphdr) + 40;

	if (pEntry->pkthdr.skb == skb) {
		skb = alloc_skb(totoal_len, GFP_ATOMIC);
		if(skb == NULL) return 0;
	}
	else {
		skb_reset_tail_pointer(skb);
		if(skb_tailroom(skb) < totoal_len) {
			//printk("==> [%s] no available data of skb(%p) len %d size %d\n", __FUNCTION__, skb, skb_tailroom(skb), totoal_len);
			skb = alloc_skb(len, GFP_ATOMIC);
			if(skb == NULL) return 0;
			ret = 2;
		}
	}
	
	skb_put(skb, totoal_len);
	len = pEntry->pkthdr.l4HdrOffset + sizeof(struct tcphdr);
	memcpy(skb->data, pEntry->pkthdr.skb->data, len);

	skb->dev = pEntry->pkthdr.skb->dev;
	skb->mark = pEntry->pkthdr.skb->mark;
#if defined(CONFIG_RTK_SKB_MARK2) 
	skb->mark2 = pEntry->pkthdr.skb->mark2;
#endif
	tcph = (struct tcphdr*)(skb->data + pEntry->pkthdr.l4HdrOffset);
	tcp_option_write(tcph, pPktHdr, pEntry);
	tcph->seq = htonl(pEntry->seqNo);
	tcph->ack_seq = htonl(pEntry->rcv_nxt);
	tcph->ack = 1;
	l4Len = tcph->doff << 2;
	
	/* check SKB len */
	len = pEntry->pkthdr.l4HdrOffset + l4Len;
	if (totoal_len > len) {
		skb->len = len;
	}

	/* calculate L3+L4+payloadLen */
	len = pEntry->pkthdr.l4HdrOffset - pEntry->pkthdr.l3HdrOffset + l4Len;
	
	/*modify IP Header */
	if (pPktHdr->f_isipv4)
	{
		iph = (struct iphdr *)(skb->data + pEntry->pkthdr.l3HdrOffset);
		iph->tot_len = htons(len);
		iph->frag_off = htons(IP_DF);
		iph->ttl = 64;
		iph->id = htons(pEntry->id++);
	}
	else
	{
		ip6h = (struct ipv6hdr*)(skb->data + pEntry->pkthdr.l3HdrOffset);
		ip6h->payload_len = htons(l4Len);
	}
	
	if (pEntry->pkthdr.f_ispppoe)
	{
		struct pppoe_hdr *pppoe;
		pppoe = (struct pppoe_hdr *)(skb->data + pEntry->pkthdr.pppoeHdrOffset);
		pppoe->length = htons(len+2);;
	}
	
	skb->ip_summed = CHECKSUM_COMPLETE;
#ifdef CONFIG_ARCH_CORTINA
	//skb_reset_mac_header(skb);
	skb_set_network_header(skb, pEntry->pkthdr.l3HdrOffset);
	skb_set_transport_header(skb, pEntry->pkthdr.l4HdrOffset);
#endif

	pEntry->cnt = 0;

	set_skb_tx_info(skb);
	skb->dev->netdev_ops->ndo_start_xmit(skb, skb->dev);
	
	return ret;
}

/* 20180103: sometimes http server will wait for ack to send the next segment, if speedup module 
 * didn't send ack because dataAckCnt is not reached, so the interval between the two segment will 
 * be too large, 
 * there is no timer for such case, so I think ack should be triggered when NIC Rx handler processed
 * all packet in rx ring and rx ring is empty.
 * We have tested this patch, and found that there are too many ack sent and throughput is poor. is it
 * an ugly idea???
 * 20180104: I have add a real count for timer check.
 *           modify return value type from void to int, 1 means some segment is not acked, 0 means no 
 *           segment wait for ack.
 */
int send_delay_ack_timer(void)
{
	struct _speedup_stream_track *pEntry;
	int unAckSegment=0;
	int ackedNum=0;

	/* FIXME: we must waiting for all thread up, 2s is enough??? 
	 *  20180105: I think maybe waiting for at least 5 stream established is a better idea.
	 */
	//if (sampletime <= 2)
	//	return 0;
	//if (!speedup_cpu_matched())
		//return 0;
	
#ifdef IP_SPEEDUP_DEBUG
	delay_ack_num_on_cpu[smp_processor_id()]++;
#endif
XMIT_ACK:
	read_lock(&speedup_lock);
	list_for_each_entry(pEntry, &sstAciveHeadList, activeNode)
	{
		if (pEntry->direction == 1)
		{
		#if 1
			int xmitCnt=0;
			if (!spin_trylock(&pEntry->sst_lock))
				continue;
			
			if ((0 == pEntry->sndPktNum) || !should_send_now(pEntry->sndtime, sendDelayTime))
			{
				spin_unlock(&pEntry->sst_lock);
				if (pEntry->sndPktNum)
					schedule_upload_work();
				continue;
			}
			//printk("%s %d\n", __func__, __LINE__);
			while (pEntry->sndPktNum && (xmitCnt<5))
			{
				if (pEntry->ackedSeqNo && (pEntry->seqNo > (pEntry->ackedSeqNo + pEntry->burstDataSize)))
					break;

				if (!_send_upload_data(pEntry))
					break;
				xmitCnt++;
			}
			//printk("%s %d\n", __func__, __LINE__);
			spin_unlock(&pEntry->sst_lock);
			
			if (pEntry->sndPktNum)
				schedule_upload_work();
		#endif
			continue;
		}

		/* below for download test processing */
		if (unlikely(doUploadTest))
			continue;
		
		if (!spin_trylock(&pEntry->sst_lock))
			continue;
		if (NULL == pEntry->pkthdr.skb)//no ack should be sent
		{
			spin_unlock(&pEntry->sst_lock);
			continue;
		}

		if ((pEntry->cnt < dataAckCnt) && !should_ack_now(pEntry->rcvtime))
		{
			spin_unlock(&pEntry->sst_lock);
			unAckSegment = 1;
			continue;
		}
		
#ifdef IP_SPEEDUP_DEBUG
		delayAckPktCnt++;
#endif
		/* send ack immediately */
		delay_ack_xmit(&pEntry->pkthdr);
		spin_unlock(&pEntry->sst_lock);
		ackedNum++;
	}
	read_unlock(&speedup_lock);
	if ((ackedNum >= 5) && unAckSegment)
	{
		ackedNum = 0;
		unAckSegment = 0;
		
		goto XMIT_ACK;
	}
	return (unAckSegment);
}

int send_delay_ack_timer_unlock(void)
{
	struct _speedup_stream_track *pEntry;
	int unAckSegment=0;
	int ackedNum=0;

	/* FIXME: we must waiting for all thread up, 2s is enough??? 
	 *  20180105: I think maybe waiting for at least 5 stream established is a better idea.
	 */
	//if (sampletime <= 2)
	//	return 0;
	//if (!speedup_cpu_matched())
		//return 0;
		
XMIT_ACK:	
	list_for_each_entry(pEntry, &sstAciveHeadList, activeNode)
	{
		if (pEntry->direction == 1)
			continue;

		/* below for download test processing */
		if (!spin_trylock(&pEntry->sst_lock))
			continue;
		if (NULL == pEntry->pkthdr.skb)//no ack should be sent
		{
			spin_unlock(&pEntry->sst_lock);
			continue;
		}

		if ((pEntry->cnt < dataAckCnt) && !should_ack_now(pEntry->rcvtime))
		{
			spin_unlock(&pEntry->sst_lock);
			unAckSegment = 1;
			continue;
		}
		
#ifdef IP_SPEEDUP_DEBUG
		delayAckPktCnt++;
#endif
		/* send ack immediately */
		delay_ack_xmit(&pEntry->pkthdr);
		spin_unlock(&pEntry->sst_lock);
		ackedNum++;
	}
	if ((ackedNum >= 5) && unAckSegment)
	{
		ackedNum = 0;
		unAckSegment = 0;
		
		goto XMIT_ACK;
	}
	return (unAckSegment);
}

static inline int sack_extend_blk(struct sack_block *sp, u32 seq,
				  u32 end_seq, int *dsack)
{
	if ((seq <= sp->end_seq) && (sp->start_seq <= end_seq))
	{
		*dsack = 1;
		if (seq < sp->start_seq)
		{
			sp->start_seq = seq;
			*dsack = 0;
		}
		if (end_seq > sp->end_seq)
		{
			sp->end_seq = end_seq;
			*dsack = 0;
		}
		return 1;
	}
	return 0;
}

/* These routines update the SACK block as out-of-order packets arrive or
 * in-order packets close up the sequence space.
 */
static inline void sack_maybe_coalesce(struct _speedup_stream_track *pEntry)
{
	int this_sack;
	struct sack_block *sp = &pEntry->sack_blk[0];
	struct sack_block *swalk = sp + 1;
	int dsack;

	/* See if the recent change to the first SACK eats into
	 * or hits the sequence space of other SACK blocks, if so coalesce.
	 */
	for (this_sack = 1; this_sack < pEntry->sack_blk_num;) {
		if (sack_extend_blk(sp, swalk->start_seq, swalk->end_seq, &dsack)) {
			int i;

			/* Zap SWALK, by moving every further SACK up by one slot.
			 * Decrease num_sacks.
			 */
			pEntry->sack_blk_num--;
			for (i = this_sack; i < pEntry->sack_blk_num; i++)
				sp[i] = sp[i + 1];
			continue;
		}
		this_sack++, swalk++;
	}
}

static int sack_force_extend_blk(struct _speedup_stream_track *pEntry, u32 seq, u32 end_seq)
{
	struct sack_block *sp = &pEntry->sack_blk[0];
	struct sack_block *sp_near = NULL;
	u32 near_len = 0;
	int cur_sacks = pEntry->sack_blk_num;
	int this_sack;
	for (this_sack = 0; this_sack < cur_sacks; this_sack++, sp++) {
		u32 len = (sp->start_seq > end_seq)?(sp->start_seq - end_seq):(seq - sp->end_seq);
		if((near_len == 0) || (len<near_len))
		{
			sp_near = sp;
			near_len = len;
		}
	}
	if(sp_near)
	{
		if(sp_near->start_seq > end_seq)
			sp_near->start_seq = seq;
		else
			sp_near->end_seq = end_seq;
		while(sp_near != &pEntry->sack_blk[0])
		{
			swap(*sp_near, *(sp_near - 1));
			sp_near--;
		}
	}
	return 0;
}
/*
 * 20180911' modify return value, return 1 when DSACK found, return 0 when out of order.
 */
static int sack_new_ofo_blk(struct _speedup_stream_track *pEntry, u32 seq, u32 end_seq)
{
	struct sack_block *sp = &pEntry->sack_blk[0];
	int cur_sacks = pEntry->sack_blk_num;
	int this_sack;
	int dsack=0;

	if (!cur_sacks)
		goto new_sack;

	for (this_sack = 0; this_sack < cur_sacks; this_sack++, sp++) {
		if (sack_extend_blk(sp, seq, end_seq, &dsack)) {
			/* Rotate this_sack to the first one. */
			for (; this_sack > 0; this_sack--, sp--)
				swap(*sp, *(sp - 1));
			if (dsack)
				return 1;
			if (cur_sacks > 1)
				sack_maybe_coalesce(pEntry);
			return 0;
		}
	}

	/* Could not find an adjacent existing SACK, build a new one,
	 * put it at the front, and shift everyone else down.  We
	 * always know there is at least one SACK present already here.
	 *
	 * If the sack array is full, forget about the last one.
	 */
	if (this_sack >= TCP_NUM_SACKS) {
#ifdef IP_SPEEDUP_DEBUG
		sack_exceed_num++;
#endif
#if 1
		sack_force_extend_blk(pEntry, seq, end_seq);
		return 0;
#else
		this_sack--;
		pEntry->sack_blk_num--;
		sp--;
#endif
	}
	for (; this_sack > 0; this_sack--, sp--)
		*sp = *(sp - 1);

new_sack:
	/* Build the new head SACK, and we're done. */
	sp->start_seq = seq;
	sp->end_seq = end_seq;
	pEntry->sack_blk_num++;
	return 0;
}

/* RCV.NXT advances, some SACKs should be eaten. */
static void sack_try_remove_blk(struct _speedup_stream_track *pEntry)
{
	struct sack_block *sp;
	int num_sacks;
	int this_sack;

	/* Empty ofo queue, hence, all the SACKs are eaten. Clear. */
	if (0 == pEntry->sack_blk_num)
		return;

	sp = &pEntry->sack_blk[0];
	num_sacks = pEntry->sack_blk_num;
	
	for (this_sack = 0; this_sack < num_sacks;) {
		/* Check if the start of the sack is covered by RCV.NXT. */
		if (pEntry->rcv_nxt >= sp->start_seq) {
			int i;

			if (pEntry->rcv_nxt < sp->end_seq)
				pEntry->rcv_nxt = sp->end_seq;
			
			/* Zap this SACK, by moving forward any other SACKS. */
			for (i = this_sack+1; i < num_sacks; i++)
				pEntry->sack_blk[i-1] = pEntry->sack_blk[i];
			num_sacks--;
			continue;
		}
		this_sack++;
		sp++;
	}
	pEntry->sack_blk_num = num_sacks;
}

/* RCV.NXT advances, some SACKs should be eaten. */
static void upload_sack_try_remove_blk(struct _speedup_stream_track *pEntry)
{
	struct sack_block *sp;
	int num_sacks;
	int this_sack;

	/* Empty ofo queue, hence, all the SACKs are eaten. Clear. */
	if (0 == pEntry->upload_sack_blk_num)
		return;

	sp = &pEntry->upload_sack_blk[0];
	num_sacks = pEntry->upload_sack_blk_num;
	
	for (this_sack = 0; this_sack < num_sacks;) {
		/* Check if the start of the sack is covered by RCV.NXT. */
		if (pEntry->seqNo >= sp->start_seq) {
			int i;

			if (pEntry->seqNo < sp->end_seq)
				pEntry->seqNo = sp->end_seq;
			
			/* Zap this SACK, by moving forward any other SACKS. */
			for (i = this_sack+1; i < num_sacks; i++)
				pEntry->upload_sack_blk[i-1] = pEntry->upload_sack_blk[i];
			num_sacks--;
			continue;
		}
		this_sack++;
		sp++;
	}
	pEntry->upload_sack_blk_num = num_sacks;
}

static inline int ack_now(struct _speedup_stream_track *pEntry, speedup_pktHdr_t *pPktHdr)
{
	int ret;
	/*pEntry->cnt = 0;
	
	if (pEntry->pkthdr.skb != NULL)
	{
		__kfree_skb(pEntry->pkthdr.skb);
		pEntry->pkthdr.skb = NULL;
	}*/
	pPktHdr->pStreamInfo = (void *)pEntry;
	
	/* ACK right now! */
	ret = delay_ack_xmit(pPktHdr);
	if(ret == 1)
		return SPEEDUP_RET_MATCH_ACK;
	else if(ret == 2)
		return SPEEDUP_RET_MATCH_NO_ACK;
	else 
		return SPEEDUP_RET_UNMATCH;
}

/*
 * return value: 1 - the first upload data captured, 0 - any other case
 *               just care the first upload data for this func.
 */
int speedtest_stream_shortcut_tx_learn(void *pHostTrack, speedup_pktHdr_t *pPktHdr)
{
	struct _host_track_st *ht;
	struct _speedup_stream_track *pEntry;
	unsigned int hashKey;
	char *pTmp;
	int optLen, i, j;
	u32 blk_num;
	struct sack_block blk[TCP_NUM_SACKS];	
	int dsack;
	int ret = 0;
	struct iphdr *iph;
	struct ipv6hdr *ip6h;
	u8 *saddr,*daddr;
	unsigned int payload_len=0;
	struct tcphdr *tcph;
	struct sk_buff *skb;
	ht = (struct _host_track_st *)pHostTrack;
	
	skb = pPktHdr->skb;
	if(ht->ipver == 0)
	{
		iph = (struct iphdr*)pPktHdr->pL3Hdr;
		saddr = (u8*)&(iph->saddr);
		daddr = (u8*)&(iph->daddr);
		payload_len = pPktHdr->dataLen;
	}
	else
	{
		ip6h = (struct ipv6hdr*)pPktHdr->pL3Hdr;
		saddr = (u8*)&(ip6h->saddr);
		daddr = (u8*)&(ip6h->daddr);
		payload_len = pPktHdr->dataLen;
	}
	tcph = (struct tcphdr *)pPktHdr->pL4Hdr;
	
	
	if (0 == ht->direction)//download only
	{
		/* when speedupPathFixed is set, it means download stream will all go FAST ACK path or go normal path, dont need to learn anymore */
		if (speedupPathFixed && bypassFastAckFlag)
			return 0;
		
		if (!ht->sack)
			return 0;
	}
	
	hashKey = speedup_hash_entry(saddr, tcph->source, daddr, tcph->dest);
	list_for_each_entry(pEntry, &ht->hash[hashKey], node)
	{
		if (((0 == pEntry->dstip[0]) || (ip_cmp(pEntry->dstip,daddr, ht->ipver) == 0)) &&
			((0 == pEntry->srcip[0]) || (ip_cmp(pEntry->srcip,saddr, ht->ipver) == 0)) &&
			((0 == pEntry->dport) || (pEntry->dport == tcph->dest)) &&
			((0 == pEntry->sport) || (pEntry->sport == tcph->source)))
		{
			spin_lock(&pEntry->sst_lock);
			pEntry->totalbytes += payload_len;

			if (NULL == pEntry->pkthdr.skb)
			{
				pEntry->seqNo = ntohl(tcph->seq) + payload_len;
				pEntry->ack_seq = ntohl(tcph->ack_seq);
				generate_upload_cached_skb(pEntry, pPktHdr, skb, 0);
				ret = 1;
			}
			else {
				spin_unlock(&pEntry->sst_lock);
				return ret;
			}

			/* below for doanload processing */
			if ((speedupPathFixed && bypassFastAckFlag) || (!ht->sack))
			{
				spin_unlock(&pEntry->sst_lock);
				return ret;
			}
			
#ifdef IP_SPEEDUP_OUTORDER_DBG
			pEntry->ackSeq = ntohl(tcph->ack_seq);
#endif			
			/* update rcv_nxt */
			if (pEntry->rcv_nxt < ntohl(tcph->ack_seq))
			{
				pEntry->rcv_nxt = ntohl(tcph->ack_seq);
			}
			
			pTmp = (unsigned char *)(tcph + 1);
			optLen = (tcph->doff<<2) - sizeof(struct tcphdr);
			
			while (optLen > 0) {
				int opcode = *pTmp++;
				int opsize;
			
				switch (opcode) {
					case TCPOPT_EOL:	/* EOL */
						optLen = 0;
						break;
					case TCPOPT_NOP:	/* NOP Ref: RFC 793 section 3.1 */
						optLen--;
						break;
					case TCPOPT_SACK:
						opsize = *pTmp++;
						blk_num = ((opsize-2)>>3);
						if (ntohl(tcph->ack_seq) > ntohl(*(unsigned int *)pTmp))//DSACK
						{
							blk_num -= 1;
							pTmp += 8;
						}
						for (i=0; i<blk_num; i++)
						{
							blk[i].start_seq = ntohl(*(unsigned int *)pTmp);
							blk[i].end_seq = ntohl(*(unsigned int *)(pTmp + 4));
							pTmp += 8;
						}
#ifdef IP_SPEEDUP_OUTORDER_DBG
						pEntry->blk_num = blk_num;
						memcpy(pEntry->blk, blk, sizeof(struct sack_block)*blk_num);
#endif

						/* coalesce block of ACK and local record */
						for (i=0; i<blk_num; i++)
						{
							for (j=0; j<pEntry->sack_blk_num; j++)
							{
								if (sack_extend_blk(&pEntry->sack_blk[j], blk[i].start_seq, blk[i].end_seq, &dsack))
									break;
							}

							if (j >= pEntry->sack_blk_num)
							{
								if (TCP_NUM_SACKS == pEntry->sack_blk_num)
								{
									pEntry->sack_blk[TCP_NUM_SACKS-1] = blk[i];
								}
								else
								{
									pEntry->sack_blk[j] = blk[i];
									pEntry->sack_blk_num++;
								}
							}
						}

						sack_maybe_coalesce(pEntry);
						sack_try_remove_blk(pEntry);
						spin_unlock(&pEntry->sst_lock);
						return 0;
					default:
						opsize = *pTmp++;
						if (opsize < 2) /* "silly options" */
						{
							optLen = 0;
							break;
						}
						if (opsize > optLen)
						{
							optLen = 0; /* don't parse partial options */
							break;
						}
						pTmp += opsize-2;
						optLen -= opsize;
						break;
				}
			}

#ifdef IP_SPEEDUP_OUTORDER_DBG
			/* remove invalid SACK block */
			i = 0;
			while (i < pEntry->blk_num)
			{
				if (pEntry->ackSeq >= pEntry->blk[i].start_seq)
				{
					for (j=i+1; j<pEntry->blk_num; j++)
						pEntry->blk[j-1] = pEntry->blk[j];
					pEntry->blk_num -= 1;
					
					continue;
				}
				
				i++;
			}
#endif
			/* remove invalid block */
			sack_try_remove_blk(pEntry);
			spin_unlock(&pEntry->sst_lock);
			return ret;
		}
	}
	return 0;
}

void speedtest_stream_shortcut_rx_learn(void *pHostTrack, speedup_pktHdr_t *pPktHdr, u32 dataLen)
{
	struct _host_track_st *ht;
	struct _speedup_stream_track *pEntry;
	unsigned int hashKey;

	/* when speedupPathFixed is set, it means download stream will all go FAST ACK path or go normal path, dont need to learn anymore */
	if (speedupPathFixed)
		return;
	
	ht = (struct _host_track_st *)pHostTrack;
	if (!ht->sack)
		return;
	
	hashKey = speedup_hash_entry(pPktHdr->daddr, pPktHdr->dport, pPktHdr->saddr, pPktHdr->sport);
	list_for_each_entry(pEntry, &ht->hash[hashKey], node)
	{
		if (((0 == pEntry->dstip[0]) || ip_cmp(pEntry->dstip,pPktHdr->saddr,ht->ipver)==0) &&
			((0 == pEntry->srcip[0]) || ip_cmp(pEntry->srcip,pPktHdr->daddr,ht->ipver)==0) &&
			((0 == pEntry->dport) || (pEntry->dport == pPktHdr->sport)) &&
			((0 == pEntry->sport) || (pEntry->sport == pPktHdr->dport)))
		{
			if (pEntry->direction)
				return;
			if (0 == pEntry->rcv_nxt)
			{
				//pEntry->rcv_nxt = pPktHdr->seqNo + dataLen;
				//printk("waiting for ack by PS!!!\n");
				return;
			}
			else if (pPktHdr->seqNo < pEntry->rcv_nxt)
			{
				return;
			}
			else if (pPktHdr->seqNo > pEntry->rcv_nxt)
			{
				spin_lock(&pEntry->sst_lock);
				sack_new_ofo_blk(pEntry, pPktHdr->seqNo, pPktHdr->seqNo + dataLen);
				spin_unlock(&pEntry->sst_lock);
				return;
			}
			spin_lock(&pEntry->sst_lock);
			pEntry->rcv_nxt = pPktHdr->seqNo + dataLen;
			sack_try_remove_blk(pEntry);
			spin_unlock(&pEntry->sst_lock);
			return;
		}
	}
}

/* shortcut for speedtest 
 * PARAMETER
 *   dstip -  client Addr
 *   srcip -  server Addr
 * RETURN VALUE
 *   1 -  success;  0 -  fail
 */
int speedtest_stream_shortcut_check(void *pHostTrack, speedup_pktHdr_t *pPktHdr, u32 dataLen)
{
	struct _host_track_st *ht;
	struct _speedup_stream_track *pEntry;
	unsigned int hashKey;
	int ret=SPEEDUP_RET_UNMATCH;

	ht = (struct _host_track_st *)pHostTrack;
	hashKey = speedup_hash_entry(pPktHdr->daddr, pPktHdr->dport, pPktHdr->saddr, pPktHdr->sport);
	list_for_each_entry(pEntry, &ht->hash[hashKey], node)
	{
		if (((0 == pEntry->dstip[0]) || ip_cmp(pEntry->dstip,pPktHdr->saddr,ht->ipver)==0) &&
			((0 == pEntry->srcip[0]) || ip_cmp(pEntry->srcip,pPktHdr->daddr,ht->ipver)==0) &&
			((0 == pEntry->dport) || (pEntry->dport == pPktHdr->sport)) &&
			((0 == pEntry->sport) || (pEntry->sport == pPktHdr->dport)))
		{
			if (pEntry->direction)
				return SPEEDUP_RET_UNMATCH;
			/*
			 * Rules for Generating ACK
			 * 1. When one end sends a data segment to the other end, it must include an ACK.  That gives the next sequence number it expects to receive. (Piggyback)
			 * 2. The receiver needs to delay sending (until another segment arrives or 500ms) an ACK segment if there is only one outstanding in-order segment. 
			 *    It prevents ACK segments from creating extra traffic.
			 * 3. There should not be more than 2 in-order unacknowledged segments at any time. It prevent the unnecessary retransmission
			 * 4. When a segment arrives with an out-of-order sequence number that is higher than expected, the receiver immediately sends an ACK segment 
			 *    announcing the sequence number of the next expected segment. (for fast retransmission)
			 * 5. When a missing segment arrives, the receiver sends an ACK segment to announce the next sequence number expected.
			 * 6. If a duplicate segment arrives, the receiver immediately sends an ACK.
			 */
			spin_lock(&pEntry->sst_lock);
			if (0 == pEntry->rcv_nxt)
			{
				pEntry->rcv_nxt = pPktHdr->seqNo + dataLen;
				pEntry->totalbytes += dataLen;
				ret = ack_now(pEntry, pPktHdr);
				spin_unlock(&pEntry->sst_lock);
				return ret;
			}

			if (pPktHdr->seqNo < pEntry->rcv_nxt)		//missing segment or duplicate segment arrives, ack immediately.
			{
#ifdef IP_SPEEDUP_DEBUG
				duplicatePktCnt++;
#endif
				if (ht->sack)
				{
					/* send D-SACK */
					pPktHdr->f_isdsack = 1;
					ret=ack_now(pEntry, pPktHdr);
					spin_unlock(&pEntry->sst_lock);
					return ret;
				}
				else
				{
					/* QL if contineously send unseen segment ack, we should reassign seqno */
					pEntry->dupSegCnt++;
					//if (pEntry->dupSegCnt >= 5)
					if (pEntry->dupSegCnt >= 2)	// Mark, some environment will send unseen frame twice and goto slow-start, so adjust to 2 to let server start xmit
					{
#ifdef IP_SPEEDUP_DEBUG
						revisePktCnt++;
#endif
						pEntry->dupSegCnt = 0;
						pEntry->rcv_nxt = pPktHdr->seqNo + dataLen;
					}
					else
					{
						/* LOG: ack the lost segment only, is it better to ack the newest segment? 
						 * 20180104 It seems ack the newest segment get the better result.
						 */
					}
					
					ret = ack_now(pEntry, pPktHdr);
					spin_unlock(&pEntry->sst_lock);
					return ret;
				}
			}
			else if (pPktHdr->seqNo > pEntry->rcv_nxt)
			{
#ifdef IP_SPEEDUP_DEBUG
				outOrderPktCnt++;
#endif
				if (ht->sack)
				{
					if (sack_new_ofo_blk(pEntry, pPktHdr->seqNo, pPktHdr->seqNo + dataLen))
					{
#ifdef IP_SPEEDUP_DEBUG
						duplicatePktCnt++;
#endif
						pPktHdr->f_isdsack = 1;
						ret=ack_now(pEntry, pPktHdr);
						spin_unlock(&pEntry->sst_lock);
						return ret;
					}
					
					pEntry->totalbytes += dataLen;
					
					/* send SACK */
					/* it seems too many ack here, delay is the better choice */
					pEntry->cnt++;			
					if (pEntry->cnt >= dataAckCnt)	//ack immediately
					{
						ret=ack_now(pEntry, pPktHdr);
						spin_unlock(&pEntry->sst_lock);
						return ret;
					}
					
					spin_unlock(&pEntry->sst_lock);
					
					//if (likely(doUploadTest))
#ifdef  DELAY_WORK
					schedule_download_work_delay();
#else
					schedule_download_work();
#endif
					return SPEEDUP_RET_MATCH_NO_ACK;
				}
				else
				{
					if(pPktHdr->seqNo > (pEntry->rcv_nxt+seqnoIntervalIgnoreThreshold))
					{
#ifdef IP_SPEEDUP_DEBUG
						greateoutOrderPktCnt++;
#endif
						pEntry->totalbytes += dataLen;
						
						ret = ack_now(pEntry, pPktHdr);
						spin_unlock(&pEntry->sst_lock);
						return ret;
					}
				}
			}
			
#ifdef IP_SPEEDUP_DEBUG
			inOrderPktCnt++;
#endif
			
			pEntry->dupSegCnt = 0;
			
			pEntry->rcv_nxt = pPktHdr->seqNo + dataLen;
			if (ht->sack)
			{
				sack_try_remove_blk(pEntry);
			}
			pEntry->totalbytes += dataLen;
			pEntry->cnt++;
			
			/* because we have no delay timer for ack, so speedup will send ack for the last data packet for delay requirement. */
			if (pEntry->cnt >= dataAckCnt)	//ack immediately
			{
				ret=ack_now(pEntry, pPktHdr);
				spin_unlock(&pEntry->sst_lock);
				/*if (unlikely(doUploadTest))
					schedule_download_work();
				else
					send_delay_ack_timer_unlock();*/			
				return ret;
			}		
			
			/*if (NULL == pEntry->pkthdr.skb)
			{
				pEntry->rcvtime = REAL_SPEEDTEST_TIME;
				
				pEntry->pkthdr = *pPktHdr;
				pEntry->pkthdr.pStreamInfo = (void *)pEntry;
				spin_unlock(&pEntry->sst_lock);
				if (unlikely(doUploadTest))
					schedule_download_work();
				return SPEEDUP_RET_MATCH_QUEUED;
			}*/
			spin_unlock(&pEntry->sst_lock);
			//if (likely(doUploadTest))
#ifdef  DELAY_WORK
			schedule_download_work_delay();
#else
			schedule_download_work();
#endif
			return SPEEDUP_RET_MATCH_NO_ACK;
		}
	}

	return SPEEDUP_RET_UNMATCH;
}

/*
 * upload speedtest, only handle ACK here
 */
int speedtest_upload_shortcut_enter(void *pHostTrack, speedup_pktHdr_t *pPktHdr)
{
	struct _host_track_st *ht;
	struct _speedup_stream_track *pEntry;
	unsigned int hashKey, send_len;

	//printk("==> [%s:%d] enter\n", __func__, __LINE__);
	ht = (struct _host_track_st *)pHostTrack;
	hashKey = speedup_hash_entry(pPktHdr->daddr, pPktHdr->dport, pPktHdr->saddr, pPktHdr->sport);
	list_for_each_entry(pEntry, &ht->hash[hashKey], node)
	{
		if ((pEntry->direction == 1) &&
			((0 == pEntry->dstip[0]) || (ip_cmp(pEntry->dstip, pPktHdr->saddr, ht->ipver)==0)) &&
			((0 == pEntry->srcip[0]) || (ip_cmp(pEntry->srcip, pPktHdr->daddr, ht->ipver)==0)) &&
			((0 == pEntry->dport) || (pEntry->dport == pPktHdr->sport)) &&
			((0 == pEntry->sport) || (pEntry->sport == pPktHdr->dport)))
		{
			spin_lock(&pEntry->sst_lock);

			pPktHdr->pStreamInfo = (void *)pEntry;
			pEntry->stat_pkt++;

			do {
				struct tcphdr *tcph = (struct tcphdr*)pPktHdr->pL4Hdr;
				char *pTmp = (unsigned char *)(tcph + 1);
				int optLen = (tcph->doff<<2) - sizeof(struct tcphdr);
				int sack_flag = 0;
				u32 start_seq;
				int win_size, available_size;
				int send_num, available_num;

				//if (tcph->ack_seq <= pEntry->ackedSeqNo)
				//{
				//	break;
				//}
				
				/*
				 * in such case:
				 * 1. some packets maybe lost
				 * 2. some packets have been sent are not acked
				 * anyway, we should check sack options firestly, if ack without SACK option, I think the last packets maybe acked later, just
				 * continue to send based on the last seqNo; otherwise, I think we should retransmit the hole packets.
				 */
				while (optLen > 0) {
					int opcode = *pTmp++;
					int opsize;
				
					switch (opcode) {
						case TCPOPT_EOL:	/* EOL */
							optLen = 0;
							break;
						case TCPOPT_NOP:	/* NOP Ref: RFC 793 section 3.1 */
							optLen--;
							break;
						case TCPOPT_SACK:
							sack_flag = 1;
							
							pEntry->upload_sack_blk_num = 0;
							
							opsize = *pTmp++;
							start_seq = ntohl(*(u32 *)pTmp);
							opsize -= 2;
							optLen -= 2;
							if (start_seq < pPktHdr->ackNo)
							{
								//this is DSACK
								pTmp += 8;	//DSACK option
								opsize -= 8;
								optLen -= 8;
								
								if (opsize <= 0)
								{
									sack_flag = 0;
								}
								else
								{
									//update ofo blk
									while (optLen > 0)
									{
										pEntry->upload_sack_blk[pEntry->upload_sack_blk_num].start_seq = ntohl(*(u32 *)pTmp);
										pTmp += 4;
										pEntry->upload_sack_blk[pEntry->upload_sack_blk_num].end_seq = ntohl(*(u32 *)pTmp);
										pTmp += 4;
										
										opsize -= 8;
										optLen -= 8;

										pEntry->upload_sack_blk_num++;
									}
								}
							}
							else
							{
								//this maybe out order DSACK or SACK.
								//update ofo blk
								while (optLen > 0)
								{
									pEntry->upload_sack_blk[pEntry->upload_sack_blk_num].start_seq = ntohl(*(u32 *)pTmp);
									pTmp += 4;
									pEntry->upload_sack_blk[pEntry->upload_sack_blk_num].end_seq = ntohl(*(u32 *)pTmp);
									pTmp += 4;
									
									opsize -= 8;
									optLen -= 8;
								
									pEntry->upload_sack_blk_num++;
								}
							}
							break;
						default:
							opsize = *pTmp++;
							if (opsize < 2) /* "silly options" */
							{
								optLen = 0;
								break;
							}
							if (opsize > optLen)
							{
								optLen = 0; /* don't parse partial options */
								break;
							}
							pTmp += opsize-2;
							optLen -= opsize;
							break;
					}
				}

				//printk("[%s]upload seq %x ack %x ackedseq %x\n",__func__,pEntry->seqNo, pPktHdr->ackNo,pEntry->ackedSeqNo);
				if (pEntry->ackedSeqNo < pPktHdr->ackNo || pEntry->seqNo < pEntry->ackedSeqNo)
				{
					pEntry->ackedSeqNo = pPktHdr->ackNo;
					pEntry->frozenAckCnt = 0;
				}
				else if (pEntry->ackedSeqNo == pPktHdr->ackNo) {
					pEntry->frozenAckCnt++;
				}
				
				win_size = ntohs(tcph->window) * pEntry->wscale;
				send_len = 0;
				if(pEntry->seqNo > pEntry->ackedSeqNo) 
					send_len = pEntry->seqNo - pEntry->ackedSeqNo;
				available_size = (win_size - send_len);
				if(available_size > 0) 
				{
#if 1
					if((pEntry->frozenAckCnt % 20) == 1) {
						pEntry->payload_len /= 2;
						pEntry->payload_len = (pEntry->payload_len / pEntry->snd_mss) * pEntry->snd_mss;
					}
					else if(pEntry->frozenAckCnt == 0){
						pEntry->payload_len += pEntry->snd_mss;
					}
					
					if(pEntry->payload_len > available_size ) {
						pEntry->payload_len = (available_size / pEntry->snd_mss) * pEntry->snd_mss;
					} 

					if(pEntry->payload_len < pEntry->snd_mss)
						pEntry->payload_len = pEntry->snd_mss;
					else if(pEntry->payload_len > max_buf_len) {
						pEntry->payload_len = max_buf_len - pPktHdr->l4HdrOffset - sizeof(struct tcphdr);;
						pEntry->payload_len = (pEntry->payload_len / pEntry->snd_mss) * pEntry->snd_mss;
					}
					//printk("===> %p payload_len %d\n", pEntry, pEntry->payload_len);
#endif	
					available_num = available_size / pEntry->payload_len;
				} else {
					available_num = 0;
				}
				
				if(available_num > 0) {
					send_num = (available_num > upload_burst_num) ? upload_burst_num : available_num;
				} else {
					send_num = 0;
				}
				
				//if(win_size == 0 || pEntry->frozenAckCnt)
				//	printk("=> %p sack_flag %d sndPktNum %d frozenAckCnt %d seqNo %u ackedSeqNo %u ackNo %u win_size %d send_num %d(%d)\n", pEntry, sack_flag, pEntry->sndPktNum, pEntry->frozenAckCnt, pEntry->seqNo, pEntry->ackedSeqNo, pPktHdr->ackNo, win_size, send_num, available_num);

				if (0 == sack_flag)
				{
					/* continue to xmit by order
					 * we should care window size, unacked size, etc...
					 */	
					{
						if(pEntry->frozenAckCnt > 0 && pEntry->frozenAckCnt <= frozen_ack_num && win_size > pEntry->snd_mss)
						{
							//printk("#################### zero window retrans(%u)!####################\n",pPktHdr->ackNo);
#ifdef IP_SPEEDUP_DEBUG	
							zero_win_retrans++;
#endif
							//pEntry->frozenAckCnt = 0;
							pEntry->seqNo = pPktHdr->ackNo;
							pEntry->sndPktNum = 1;
							pEntry->push = 1;
							spin_unlock(&pEntry->sst_lock);
							schedule_upload_work();
							return SPEEDUP_RET_MATCH_NO_ACK;//SPEEDUP_RET_MATCH_ACK;
						}
						else if(pEntry->frozenAckCnt > frozen_ack_num)
						{
#ifdef IP_SPEEDUP_DEBUG
							pause_xmit_count++;
#endif
							pEntry->sndPktNum = 0;		
							spin_unlock(&pEntry->sst_lock);
							return SPEEDUP_RET_MATCH_NO_ACK;
						}

						if(send_num <= 0) {
#ifdef IP_SPEEDUP_DEBUG
							pause_xmit_count++;
#endif
							pEntry->sndPktNum = 0;
							spin_unlock(&pEntry->sst_lock);
							return SPEEDUP_RET_MATCH_NO_ACK;
						}
#ifdef IP_SPEEDUP_DEBUG
						normal_xmit_count++;
#endif
						/* xmit #send_num packets now!*/
						pEntry->sndPktNum = send_num;

#ifdef IP_SPEEDUP_DEBUG
						if (all_ack_count < 100)
						{
#ifdef CONFIG_ARCH_CORTINA
							send_delay_time[all_ack_count] = get_time_duration(pEntry->sndtime)/2;
#else
							send_delay_time[all_ack_count] = get_time_duration(pEntry->sndtime);
#endif
						}
						
						all_ack_count++;
#endif
						pEntry->push = 1;
						spin_unlock(&pEntry->sst_lock);
						schedule_upload_work();
						return SPEEDUP_RET_MATCH_NO_ACK;
					}
				}
				else
				{
					if(send_num <= 0) {
#ifdef IP_SPEEDUP_DEBUG
						pause_xmit_count++;
#endif
						pEntry->sndPktNum = 0;
						spin_unlock(&pEntry->sst_lock);
						return SPEEDUP_RET_MATCH_NO_ACK;
					}

#ifdef IP_SPEEDUP_DEBUG
					sack_ack_count++;
#endif
					//delay xmit hole packet
					pEntry->seqNo = pPktHdr->ackNo;
					pEntry->sndPktNum = send_num;
					spin_unlock(&pEntry->sst_lock);
					schedule_upload_work();
					return SPEEDUP_RET_MATCH_NO_ACK;
				}
			} while (0);
			spin_unlock(&pEntry->sst_lock);
			return SPEEDUP_RET_MATCH_NO_ACK;
		}
	}

	return SPEEDUP_RET_UNMATCH;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static void host_timeout(struct timer_list *timer) 
#else
static void host_timeout(unsigned long data) 
#endif
{
	//struct _host_track_st *ht = (struct _host_track_st *)data;
	int i;

	if (!list_empty(&sstAciveHeadList))
	{
		mod_timer(&ht_timer, jiffies+(HOST_TIMEOUT*HZ));
		return;
	}
	write_lock(&speedup_lock);
	for (i=0; i<10; i++)
	{
		host_track[i].active = 0;
	}
	resetSstTbl();
	write_unlock(&speedup_lock);
	
	printk("host_track timeout!\n");

};

static inline void host_track_on(u8* dstip, u8* srcip, u16 dport, u16 sport, int direction, int ipver)
{
	int first_on=1;
	int i, j;

	if (unlikely(!HostSpeedUP))
	{
		printk("host speedup is off!\n");
		return;
	}
	
	//printk("host_track_on(%s:%d,%s:%d) %s\n",ip_inet_ntoa(dstip),dport,ip_inet_ntoa(srcip),sport, (direction==1)?"upload":"download");
	for (i=0; i<10; i++)
	{
		if (host_track[i].active)
		{
			/* host tracking is already enabled */
			first_on = 0;

			if ((host_track[i].ipver == ipver) &&
				(ip_cmp(host_track[i].dstip, dstip, ipver)==0) &&
				(host_track[i].dport == dport) &&
				(ip_cmp(host_track[i].srcip, srcip, ipver)==0) &&
				(host_track[i].sport == sport))
			{//rule alerasy existed.
				if (host_track[i].direction != direction)
					host_track[i].direction = 2;
				host_track[i].ref_cnt++;
				break;
			}
		}
		else
		{
			//no matched rule found, add a new rule.
			host_track[i].ipver = ipver;
			if(ipver==0)
			{
				memcpy(host_track[i].dstip,dstip,IP_ADDR_LEN);
				memcpy(host_track[i].srcip,srcip,IP_ADDR_LEN);
			}
			else
			{
				memcpy(host_track[i].dstip,dstip,IP6_ADDR_LEN);
				memcpy(host_track[i].srcip,srcip,IP6_ADDR_LEN);
			}

			host_track[i].dport = dport;
			host_track[i].sport = sport;
			host_track[i].direction = direction;
			host_track[i].ref_cnt = 1;
			for (j=0; j<SPEEDUP_HASH_SIZE; j++)
				INIT_LIST_HEAD(&host_track[i].hash[j]);
			host_track[i].active = 1;
			host_track[i].bypassFastAck = 0;
			host_track[i].sack = 1;
			break;
		}
	}

	if (direction)
		doUploadTest = 1;
	else
		doDownloadTest = 1;

	if (first_on)
	{
	    speedtest_sample_init();
		fast_ack_calibration_init();
	}
	
	mod_timer(&ht_timer, jiffies+(HOST_TIMEOUT*HZ));
}

static inline void clear_host_track(void)
{
	struct _speedup_stream_track *pEntry;
	int i;

	for (i=0; i<10; i++)
	{
		host_track[i].active = 0;
	}
	
	list_for_each_entry(pEntry, &sstAciveHeadList, activeNode)
	{
		if (/*pEntry->direction &&*/ pEntry->pkthdr.skb)
		{
			dev_kfree_skb(pEntry->pkthdr.skb);
			pEntry->pkthdr.skb = NULL;
		}
	}
	
	resetSstTbl();
	
	del_timer(&ht_timer);

	//HostSpeedUP = 0;
	doUploadTest = 0;
	doDownloadTest = 0;
}

static inline void host_track_off(u8* dstip, u8* srcip, u16 dport, u16 sport, int direction, int ipver)
{
	int i;

	/* avoid user app to turn off host speedup on ohter cpu */
	//if (!speedup_cpu_matched())
	//	return;
	
	printk("host_track_off enter\n");
	if ((NULL == dstip) && (NULL == srcip) && (0 == sport) && (0 == dport))
	{
		if (2 == direction)
		{
			return clear_host_track();
		}
		else
		{
			/* check if all host_track should be removed */
			for (i=0; i<10; i++)
			{
				if (host_track[i].active)
				{
					if (host_track[i].direction != direction)
						break;
				}
				else
					break;
			}
			if ((i>=10) || (0 == host_track[i].active))
			{
				return clear_host_track();
			}
			
			for (i=0; i<10; i++)
			{
del_one_host_track:
				if ((i < 10) && (host_track[i].active))
				{
					if (host_track[i].direction == direction)
					{
						if (1 == direction)
							doUploadTest = 0;
						
						delHostTrackEntry(&host_track[i]);
						
						/* FIXME: is it a good idea to move following rule ahead? */
						if ((9 != i) && (0 != host_track[i+1].active))
						{
							for (; i<10; i++)
							{
								if (i < 9)
								{
									memcpy(&host_track[i], &host_track[i+1], sizeof(struct _host_track_st));
									if (0 == host_track[i+1].active)
										break;
								}
								else
								{
									memset(&host_track[i], 0, sizeof(struct _host_track_st));
								}
							}
						}
						goto del_one_host_track;
					}
					else if (2 == host_track[i].direction)
					{
						if (1 == direction)
						{
							host_track[i].direction = 0;
							doUploadTest = 0;
						}
						else
							host_track[i].direction = 1;
					}
				}
				else
					break;
			}
		}
	}
	else
	{
		for (i=0; i<10; i++)
		{
			if (host_track[i].active)
			{
				if ((host_track[i].ipver == ipver) &&
					(ip_cmp(host_track[i].dstip, dstip, ipver)==0) &&
					(host_track[i].dport == dport) &&
					(ip_cmp(host_track[i].srcip, srcip, ipver)==0) &&
					(host_track[i].sport == sport))
				{//rule found
					host_track[i].ref_cnt--;
					if (0 == host_track[i].ref_cnt)
					{//delete it
						delHostTrackEntry(&host_track[i]);
						
						/* FIXME: is it a good idea to move following rule ahead? */
						if ((9 != i) && (0 != host_track[i+1].active))
						{
							for (; i<10; i++)
							{
								if (i < 9)
								{
									memcpy(&host_track[i], &host_track[i+1], sizeof(struct _host_track_st));
									if (0 == host_track[i+1].active)
										break;
								}
								else
								{
									memset(&host_track[i], 0, sizeof(struct _host_track_st));
								}
							}
						}
					}
					break;
				}
			}
			else
				break;
		}
	}
}

static inline void host_track_set(u8* dstip, u8* srcip, u16 dport, u16 sport, int bypass, int sack, int ipver)
{
	int i;

	if (unlikely(!HostSpeedUP))
	{
		printk("host speedup is off!\n");
		return;
	}
	
	//printk("host_track_on(%s:%d,%s:%d)\n",ip_inet_ntoa(dstip),dport,ip_inet_ntoa(srcip),sport);
	for (i=0; i<10; i++)
	{
		if (host_track[i].active)
		{
			if ((host_track[i].ipver == ipver) &&
					(ip_cmp(host_track[i].dstip, dstip, ipver)==0) &&
					(host_track[i].dport == dport) &&
					(ip_cmp(host_track[i].srcip, srcip, ipver)==0) &&
					(host_track[i].sport == sport))
			{//rule alerasy existed.
				if (bypass != -1)
					host_track[i].bypassFastAck = bypass;
				if (sack != -1)
					host_track[i].sack= sack;
				break;
			}
		}
		else
			break;
	}
}

static inline int host_match(u8 *dstip, u8 *srcip, u16 dport, u16 sport, int ipver)
{
	int i;
	int r=0;
//	static unsigned long ps_update_time=0;

	for (i=0; i<10; i++)
	{
		if (0 == host_track[i].active)
			break;
		
		if ((host_track[i].ipver == ipver) &&
				((host_track[i].dstip[0] == 0) || (ip_cmp(host_track[i].dstip, dstip, ipver)==0)) && 
				(ip_cmp(host_track[i].srcip, srcip, ipver)==0) && 
			((host_track[i].dport==0)||(host_track[i].dport == dport)) && 
			((host_track[i].sport==0)||(host_track[i].sport == sport)))
		{
			r = 1;
			break;
		}
	}
#if 0	
	if (r){
		if (jiffies >= ps_update_time)
		{
			mod_timer(&ht_timer, jiffies+(HOST_TIMEOUT*HZ));
			ps_update_time = jiffies + 10;
		}
	}
#endif
	return r;
}

int isHostSpeedUpEnable(void)
{
    return (HostSpeedUP);
}

int isDownloadHostSpeedUpEnable(void)
{
    return doDownloadTest;
}

void speedtest_packet_parser(struct sk_buff *skb, speedup_pktHdr_t *pPktHdr)
{
	struct tcphdr *tcph;
	unsigned char *pData = skb->data, count = 0;

	memset(pPktHdr, 0, sizeof(speedup_pktHdr_t));
	
	pPktHdr->skb = skb;
	pPktHdr->pL2Hdr = pData;

	pData += 12;
	do {
		if (*(unsigned short *)pData == htons(0x8100))
			pData += 4;
		else if (*(unsigned short *)pData == htons(0x88a8))
			pData += 4;
		else break;
	} while(count++ < 4);
	
	if (*(unsigned short *)pData == htons(0x8864))
	{
		pPktHdr->f_ispppoe = 1;
		pData += 2;
		pPktHdr->pPPPoEHdr = pData;
		pPktHdr->pppoeHdrOffset = pPktHdr->pPPPoEHdr - pPktHdr->pL2Hdr;
	}

	if ((pPktHdr->f_ispppoe && (*(unsigned short *)(pData+6) == htons(0x0021))) ||
		(*(unsigned short *)pData == htons(0x0800)))
	{
		struct iphdr *iph;
		pPktHdr->f_isipv4 = 1;
		
		if (pPktHdr->f_ispppoe) pData += 8;
		else pData += 2;
		
		pPktHdr->pL3Hdr = pData;
		pPktHdr->l3HdrOffset = pPktHdr->pL3Hdr - pPktHdr->pL2Hdr;

		iph = (struct iphdr *)pPktHdr->pL3Hdr;
		memcpy(pPktHdr->saddr, &(iph->saddr), IP_ADDR_LEN);
		memcpy(pPktHdr->daddr, &(iph->daddr), IP_ADDR_LEN);
		
		pPktHdr->pL4Hdr = pPktHdr->pL3Hdr + (iph->ihl << 2);
		pPktHdr->l4HdrOffset = pPktHdr->pL4Hdr - pPktHdr->pL2Hdr;

		if (iph->protocol == IPPROTO_TCP)
		{
			pPktHdr->f_istcp = 1;		
			tcph = (struct tcphdr*)pPktHdr->pL4Hdr;	
			pPktHdr->sport = tcph->source;
			pPktHdr->dport = tcph->dest;
			pPktHdr->seqNo = ntohl(tcph->seq);
			pPktHdr->ackNo = ntohl(tcph->ack_seq);
			pPktHdr->dataLen = skb->len - pPktHdr->l4HdrOffset - (tcph->doff<<2);
		}
	}
	else if((pPktHdr->f_ispppoe && (*(unsigned short *)(pData+6) == htons(0x0057))) ||
		(*(unsigned short *)pData == htons(0x86dd)))
	{
		unsigned char nexthead;
		struct ipv6hdr *ip6h;
		int ext_opt_len=0;
		pPktHdr->f_isipv6 = 1;
		
		if (pPktHdr->f_ispppoe) pData += 8;
		else pData += 2;
		pPktHdr->pL3Hdr = pData;
		pPktHdr->l3HdrOffset = pPktHdr->pL3Hdr - pPktHdr->pL2Hdr;
		
		ip6h = (struct ipv6hdr *)pPktHdr->pL3Hdr;
		memcpy(pPktHdr->saddr, &(ip6h->saddr), IP6_ADDR_LEN);
		memcpy(pPktHdr->daddr, &(ip6h->daddr), IP6_ADDR_LEN);
		
		nexthead = ip6h->nexthdr;
		pData += 40;
		while ( (nexthead == IPPROTO_HOPOPTS) || (nexthead == IPPROTO_ROUTING) || (nexthead == IPPROTO_FRAGMENT)
			 || (nexthead == IPPROTO_DSTOPTS) || (nexthead == IPPROTO_MH))
		{
			nexthead = *pData;			
			pData += (*(pData+1)+1)<<3;
			ext_opt_len += (*(pData+1)+1)<<3;
		}
		
		pPktHdr->pL4Hdr = pData;
		pPktHdr->l4HdrOffset = pPktHdr->pL4Hdr - pPktHdr->pL2Hdr;
		
		if(nexthead == IPPROTO_TCP)
		{
			pPktHdr->f_istcp = 1;
			tcph = (struct tcphdr*)pPktHdr->pL4Hdr;	
			pPktHdr->sport = tcph->source;
			pPktHdr->dport = tcph->dest;
			pPktHdr->seqNo = ntohl(tcph->seq);
			pPktHdr->ackNo = ntohl(tcph->ack_seq);
			pPktHdr->dataLen = skb->len - pPktHdr->l4HdrOffset - (tcph->doff<<2);
		}
	}
}

void * shouldSpeedUp(u32 dir, u8* dstip, u8* srcip, u16 dport, u16 sport, int ipver)
{
	int ret = 0;
	int i;
	static unsigned long update_time=0;
	
    if (0 == dir)
    {//local in
    	for (i=0; i<10; i++)
		{
			if (0 == host_track[i].active)
				break;
			
	        if ((host_track[i].ipver == ipver) &&
				((host_track[i].dstip[0] == 0) || (ip_cmp(host_track[i].dstip, dstip, ipver)==0)) && 
				(ip_cmp(host_track[i].srcip, srcip, ipver)==0) && 
				((host_track[i].dport==0)||(host_track[i].dport == dport)) && 
				((host_track[i].sport==0)||(host_track[i].sport == sport)))
        	{
	            ret = 1;
				break;
        	}
		}
    }
    else
    {//local out
    	for (i=0; i<10; i++)
		{
			if (0 == host_track[i].active)
				break;
			
	        if ((host_track[i].ipver == ipver) &&
				((host_track[i].dstip[0] == 0) || (ip_cmp(host_track[i].dstip, srcip, ipver)==0)) && 
				(ip_cmp(host_track[i].srcip, dstip, ipver)==0) && 
				((host_track[i].dport==0)||(host_track[i].dport == sport)) && 
				((host_track[i].sport==0)||(host_track[i].sport == dport)))
        	{
	            ret = 1;
				break;
        	}
		}
    }
	
	if (ret)
	{
		if (jiffies >= update_time)
		{
			mod_timer(&ht_timer, jiffies+(HOST_TIMEOUT*HZ));
			update_time = jiffies + 10;
		}

		return (void *)&host_track[i];
	}

    return (NULL);
}

int shouldBypassSpeedUP(void *pHostTrack)
{
 	struct _host_track_st *ht = (struct _host_track_st *)pHostTrack;

	return ht->bypassFastAck || bypassFastAckFlag;
}

int isUploadSpeedTestStream(void *pHostTrack)
{
	struct _host_track_st *ht = (struct _host_track_st *)pHostTrack;

	if ((1 == ht->direction) || (2 == ht->direction))
		return 1;
	return 0;
}

int getUploadAckStat(speedup_pktHdr_t *pPktHdr)
{
	struct _speedup_stream_track *pEntry = (struct _speedup_stream_track *)pPktHdr->pStreamInfo;

	if (pEntry == NULL)
		return 0;
	else
		//return (pEntry->pkthdr.skb) ? 1 : 0;
		return (pEntry->stat_pkt > 5) ? 1 : 0;
}

int speeduptest_rxhook(struct net_device *dev, struct sk_buff *skb)
{
	speedup_pktHdr_t pktHdr;
	struct tcphdr *tcph;
	void *pHostTrack;
	int ret;
	int ipver=0;

	if (isHostSpeedUpEnable())
	{
    	speedtest_packet_parser(skb, &pktHdr);
		//printk("==> [RX_HOOK] skb %p(%p) len %d dev %s data %p L2 %p L3 %p L4 %p sport %d dport %d\n", skb, pktHdr.skb, skb->len, skb->dev->name, skb->data, pktHdr.pL2Hdr, pktHdr.pL3Hdr, pktHdr.pL4Hdr, ntohs(pktHdr.sport), ntohs(pktHdr.dport));
		if ((pktHdr.f_isipv4||pktHdr.f_isipv6) && pktHdr.f_istcp)
        {
        	if(pktHdr.f_isipv6)
				ipver = 1;

			tcph = (struct tcphdr*)pktHdr.pL4Hdr;
			if (tcph->syn || tcph->fin || tcph->rst) {
				return RE8670_RX_CONTINUE;
			}
			
        	read_lock(&speedup_lock);
            if ((pHostTrack = shouldSpeedUp(0, pktHdr.daddr, pktHdr.saddr, pktHdr.dport, pktHdr.sport, ipver)) != NULL)
            {
				if (/*(0 == pktHdr.dataLen) &&*/ tcph->ack)
				{
					if (isUploadSpeedTestStream(pHostTrack))
					{
						ret = speedtest_upload_shortcut_enter(pHostTrack, &pktHdr);
						if (SPEEDUP_RET_MATCH_ACK == ret)
						{
							send_upload_data_now(&pktHdr);
							read_unlock(&speedup_lock);

							if (getUploadAckStat(&pktHdr))
								return RE8670_RX_STOP;
							else
								return RE8670_RX_CONTINUE;
								
						}
						else if (SPEEDUP_RET_MATCH_NO_ACK == ret)
						{
							read_unlock(&speedup_lock);
					
							if (getUploadAckStat(&pktHdr))
								return RE8670_RX_STOP;
							else
								return RE8670_RX_CONTINUE;
								
						}
						else
						{
							read_unlock(&speedup_lock);
							return RE8670_RX_CONTINUE;
						}
					}
				}
				
				//fast_ack_calibration_enter(skb->len);

            	/* calc rx rate */
				speedtest_sample_enter(skb->len);
				speedtest_payload_statistic(pktHdr.dataLen);

            	ret = speedtest_stream_shortcut_check(pHostTrack, &pktHdr, pktHdr.dataLen);

				read_unlock(&speedup_lock);
				if (SPEEDUP_RET_MATCH_ACK == ret)
        		{
					return RE8670_RX_STOP_SKBNOFREE;
				}
				else if (SPEEDUP_RET_MATCH_NO_ACK == ret)
				{
					return RE8670_RX_STOP;
				}
				else if (SPEEDUP_RET_MATCH_QUEUED == ret)
				{
					return RE8670_RX_STOP_SKBNOFREE;
				}
				else{
					return RE8670_RX_CONTINUE;
				}
			}
			read_unlock(&speedup_lock);
		}
	}
	
	return RE8670_RX_CONTINUE;
}

#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL9603CVD_SERIES) || defined(CONFIG_RTL8198D_SERIES)
int re8670_host_speedup (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	struct net_device *dev = (skb)? skb->dev : NULL;
	return speeduptest_rxhook(dev, skb);
}
#elif defined(CONFIG_ARCH_CORTINA)
int ca_ni_host_speedup(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv)
{
	return speeduptest_rxhook(dev, skb);
}
#endif

#if 1
/*
direction:
1, Up stream
2, Down stream
*/
int isHostCheck(struct sk_buff *skb, int hook, int direction) {
	struct iphdr *iph = ip_hdr(skb);	
	//const struct iphdr *iph = ((struct sk_buff *)skb)->nh.iph;
	struct tcphdr *th;
	
	if (unlikely(!HostSpeedUP))
		return 0;

	//if (unlikely( iph->frag_off & 0x3fff )) /* Ignore fragment */		 
	if (unlikely( iph->frag_off & htons(IP_MF|IP_OFFSET) ))
	{
		HOSTPRINTK(KERN_ERR "[%s][%d]\n", __FUNCTION__, __LINE__);
		return 0;		
	}
	
	//if( IPPROTO_TCP != iph->protocol )
	//	return 0;
		
	th=(struct tcphdr *) ((void *) iph + iph->ihl*4);
	if (th->syn)
		return 0;

//printk("%s(%d):hook%d, s=%x:%d d=%x:%d dir=%d\n",__func__,__LINE__,hook,iph->saddr,th->source,iph->daddr,th->dest,direction);	
	switch (hook) {
	case NF_INET_LOCAL_IN:
		if (host_match((u8*)&(iph->daddr),(u8*)&(iph->saddr),th->dest,th->source,0)) {
			if ( dbgflag )
				HOSTPRINTK(KERN_ERR "[%s][%d] DATA0 %x:%d<-%x:%d\n", __FUNCTION__, __LINE__,ntohl(iph->daddr),ntohs(th->dest),ntohl(iph->saddr),ntohs(th->source));
			return 1;
		}
		break;
	case NF_INET_LOCAL_OUT:
//printk("%s(%d):hook%d, s=%x:%d d=%x:%d\n",__func__,__LINE__,hook,iph->saddr,th->source,iph->daddr,th->dest);
		if (host_match((u8*)&(iph->saddr),(u8*)&(iph->daddr),th->source,th->dest,0)) {			
			if ( dbgflag )
				HOSTPRINTK(KERN_ERR "[%s][%d] DATA1 %x:%d->%x:%d\n", __FUNCTION__, __LINE__,ntohl(iph->saddr),ntohs(th->source),ntohl(iph->daddr),ntohs(th->dest));
		#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
			/* force bypass FC tx learning */
			skb->mark2 |= (1<<speedtest_skip_fc_Bit);
		#endif
			return 1;
		}
		break;
	case NF_INET_PRE_ROUTING:
		if (host_match((u8*)&(iph->daddr),(u8*)&(iph->saddr),th->dest,th->source,0)) {
			if ( dbgflag )
				HOSTPRINTK(KERN_ERR "[%s][%d] DATA2 %x:%d<-%x:%d\n", __FUNCTION__, __LINE__,ntohl(iph->daddr),ntohs(th->dest),ntohl(iph->saddr),ntohs(th->source));
			return 1;
		}
		break;
	case NF_INET_POST_ROUTING:
		if (skb->dev) // only care about local out 
			return 0;
//printk("%s(%d):hook%d, s=%x:%d d=%x:%d\n",__func__,__LINE__,hook,iph->saddr,th->source,iph->daddr,th->dest);
		if (host_match((u8*)&(iph->saddr),(u8*)&(iph->daddr),th->source,th->dest,0)) {
			if ( dbgflag )
				HOSTPRINTK(KERN_ERR "[%s][%d] DATA3 %x:%d->%x:%d\n", __FUNCTION__, __LINE__,ntohl(iph->saddr),ntohs(th->source),ntohl(iph->daddr),ntohs(th->dest));
			return 1;
		}
		break;
	default:
		return 0;
	}
	return 0;
}
#else
int isHostCheck(struct sk_buff *skb, int hook, int direction)
{
	return 0;
}
#endif

EXPORT_SYMBOL(speedtest_packet_parser);
EXPORT_SYMBOL(shouldSpeedUp);
EXPORT_SYMBOL(shouldBypassSpeedUP);
EXPORT_SYMBOL(isHostSpeedUpEnable);
EXPORT_SYMBOL(speedtest_stream_shortcut_check);
EXPORT_SYMBOL(fast_ack_calibration_enter);
EXPORT_SYMBOL(speedtest_sample_enter);
EXPORT_SYMBOL(send_delay_ack_timer);
EXPORT_SYMBOL(HostSpeedUP_Stat_get);
EXPORT_SYMBOL(getUploadAckStat);
EXPORT_SYMBOL(isUploadSpeedTestStream);
EXPORT_SYMBOL(speedtest_stream_shortcut_rx_learn);
EXPORT_SYMBOL(speedtest_stream_shortcut_tx_learn);
EXPORT_SYMBOL(speedtest_payload_statistic);
EXPORT_SYMBOL(speedup_lock);
//EXPORT_SYMBOL(delay_ack_xmit);
EXPORT_SYMBOL(send_upload_data_now);
EXPORT_SYMBOL(speedtest_upload_shortcut_enter);
EXPORT_SYMBOL(start_xmit_upload_data);

//EXPORT_SYMBOL(speedup_ack_send_callback);
//EXPORT_SYMBOL(FAST_ACK_WIN_SIZE);

int HostSpeedUP_read_proc(struct seq_file *seq, void *v)
{
	seq_printf(seq, "HostSpeedUP=%d\n", HostSpeedUP);
	return 0;
}

static int proc_HostSpeedUP_open(struct inode *inode, struct file *file)
{
	return single_open(file, HostSpeedUP_read_proc, inode->i_private);
}

static void speeduptest_txhook(void *data, struct sk_buff *skb, struct net_device *dev) 
{
	void *pHostTrack;
	int ret;
	int ipver=0;
	speedup_pktHdr_t pktHdr;
				
	if(dev && skb && dev->dev.parent && 
		!(skb->pkt_type == PACKET_BROADCAST || skb->pkt_type == PACKET_MULTICAST))
	{
		if (isHostSpeedUpEnable())
		{
			speedtest_packet_parser(skb, &pktHdr);
			if ((pktHdr.f_isipv4||pktHdr.f_isipv6) && pktHdr.f_istcp)
			{
				if(pktHdr.f_isipv6)
					ipver = 1;
				//printk("==> [TX_HOOK] skb %p(%p) len %d dev %s data %p L2 %p L3 %p L4 %p sport %d dport %d\n", skb, pktHdr.skb, skb->len, skb->dev->name, skb->data, pktHdr.pL2Hdr, pktHdr.pL3Hdr, pktHdr.pL4Hdr, ntohs(pktHdr.sport), ntohs(pktHdr.dport));
				read_lock(&speedup_lock);
				if ((pHostTrack = shouldSpeedUp(1, pktHdr.daddr, pktHdr.saddr, pktHdr.dport, pktHdr.sport, ipver)) != NULL)
				{
					//if (isUploadSpeedTestStream(pHostTrack))
					{
						ret = speedtest_stream_shortcut_tx_learn(pHostTrack, &pktHdr);
					}
				}
				read_unlock(&speedup_lock);
			}
		}
	}
}
#ifdef TRACEPOINTS_ENABLED
struct tracepoints_array {
    const char *name;
	struct tracepoint *tp;
    void *func;
    char init;
};

static struct tracepoints_array trace_events[] = {
    {.name = "net_dev_start_xmit", .func = speeduptest_txhook, .tp = NULL, .init = 0}, };

#define FOR_EACH_TRACE(i) \
    for (i = 0; i < sizeof(trace_events) / sizeof(struct tracepoints_array); i++)

static void lookup_tracepoints(struct tracepoint *tp, void *ignore) {
    int i;
    FOR_EACH_TRACE(i) {
        if (strcmp(trace_events[i].name, tp->name) == 0) trace_events[i].tp = tp;
    }
}
#else
extern int (*speedup_tx_handler)(void *data, struct sk_buff *skb, struct net_device *dev);
#endif
		
static void HostSpeedUP_config_check(void)
{
	static unsigned char hook_register = 0;
#ifdef TRACEPOINTS_ENABLED
	int i;
#endif
	
	write_lock_bh(&speedup_lock);
	if(HostSpeedUP > 0) 
	{
		if(hook_register <= 0) 
		{
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL9603CVD_SERIES) || defined(CONFIG_RTL8198D_SERIES)
			drv_nic_register_rxhook(0x7ff,RE8686_RXPRI_PATCH,re8670_host_speedup);
#endif
#ifdef CONFIG_ARCH_CORTINA
			drv_nic_register_rxhook(0xfffffff,RENIC_RXPRI_PATCH, ca_ni_host_speedup);
#endif
#ifdef TRACEPOINTS_ENABLED
			FOR_EACH_TRACE(i) {
				if (trace_events[i].tp != NULL && trace_events[i].init <= 0) {
					tracepoint_probe_register(trace_events[i].tp, trace_events[i].func, NULL);
					trace_events[i].init = 1;
					printk("==> HOOK trace '%s' func %p\n", trace_events[i].name, trace_events[i].func);
				}
			}
#else
			rcu_assign_pointer(speedup_tx_handler, speeduptest_txhook);
#endif
			hook_register = 1;
		}
	}
	else if(HostSpeedUP <= 0) 
	{
		if(hook_register) 
		{
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL9603CVD_SERIES) || defined(CONFIG_RTL8198D_SERIES)
			drv_nic_unregister_rxhook(0x7ff, RE8686_RXPRI_PATCH, re8670_host_speedup);
#endif
#ifdef CONFIG_ARCH_CORTINA
			drv_nic_unregister_rxhook(0xfffffff,RENIC_RXPRI_PATCH, ca_ni_host_speedup);
#endif
#ifdef TRACEPOINTS_ENABLED
			FOR_EACH_TRACE(i) {
				if (trace_events[i].tp != NULL && trace_events[i].init) {
					tracepoint_probe_unregister(trace_events[i].tp, trace_events[i].func, NULL);
					trace_events[i].init = 0;
				}
			}
#else
			rcu_assign_pointer(speedup_tx_handler, NULL);
#endif
			hook_register = 0;
		}
		host_track_off(0, 0, 0, 0, 2, 0);
#ifdef SKB_CACHE_ALLOC
		if(skb_upload_cache) kmem_cache_shrink(skb_upload_cache);
#endif
	}
	write_unlock_bh(&speedup_lock);
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
ssize_t HostSpeedUP_write_proc(struct file *file, const char __user *buffer, size_t len, loff_t *ppos)
#else
int HostSpeedUP_write_proc(struct file *file, const char __user *buffer, size_t len, loff_t *ppos)
#endif
{	
	unsigned char tmpBuf[16] = {0};
	int count = (len > 15) ? 15 : len;
	unsigned int tmp;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		//printk("\n[%s cpu %d] %s\n", __func__, smp_processor_id(), tmpBuf);
		tmp = simple_strtoul(tmpBuf, NULL, 16);
		switch(tmp)
		{
			case 0:
				if (HostSpeedUP > 0)
					HostSpeedUP--;
				
				HostSpeedUP_config_check();

				printk("Turn off Host Speed UP HostSpeedUP:%d\n",HostSpeedUP);
				break;
			case 1:
				HostSpeedUP++;
				HostSpeedUP_config_check();
				printk("Turn on Host Speed UP HostSpeedUP:%d\n",HostSpeedUP);
				setup_Timer_for_speedup();
				break;
			case 2:
				forceFastAck = 1;
				printk("Turn on forceFastAck\n");
				break;
			case 3:
				forceFastAck = 0;
				printk("Turn off forceFastAck\n");
				break;
			case 4:
				upload_burst_num_update = 1;
				printk("Turn on upload_burst_num dynamic update\n");
				break;
			case 5:
				upload_burst_num_update = 0;
				printk("Turn off upload_burst_num dynamic update\n");
				break;
		}
		
		
		return count;
	}	
	return -EFAULT;
}

int HostSpeedUP_dbg_read_proc(struct seq_file *seq, void *v)
{
	unsigned int time_counter=REAL_SPEEDTEST_TIME;
	seq_printf(seq, "highRateAckThresh=%d highRateAckThresh=%d\n", highRateAckThresh, lowRateAckThresh);
	seq_printf(seq, "hw_time_counter:%u\n",time_counter);
	return 0;
}

static int proc_HostSpeedUP_dbg_open(struct inode *inode, struct file *file)
{
	return single_open(file, HostSpeedUP_dbg_read_proc, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
ssize_t HostSpeedUP_dbg_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
#else
int HostSpeedUP_dbg_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
#endif
{
	char		tmpbuf[64];
	char		*strptr;
	char		*tokptr;

	if (buf && !copy_from_user(tmpbuf, buf, count)) {
		tmpbuf[count] = '\0';
		strptr=tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		highRateAckThresh = simple_strtol(tokptr, NULL, 0);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		lowRateAckThresh = simple_strtol(tokptr, NULL, 0);

		printk("highRateAckThresh=%d highRateAckThresh=%d\n", highRateAckThresh, lowRateAckThresh);
	}
	else
	{
errout:
		printk("echo 10 5 > proc/speedtest_dbg\n");
	}
	return count;
}


void HostSpeedUP_USAGE(void)
{
	printk("\nexample: local-ip and remote-ip is necessary! remote/local port is optional\n");
	printk("\n1.match 4 tuples \n");	
	printk("  echo \"add rip 192.168.1.40 rport 5000 lip 192.168.1.1 lport 5001\" > proc-path\n");
	printk("\n2.match 2 tuples \n");	
	printk("  echo \"add rip 192.168.1.40 lip 192.168.1.1\" > proc-path\n");

}

void dump_host_track(void)
{
	int i, idx=0;
	
	printk("------- host_track --------\n");
	for (i=0; i<10; i++)
	{
		if (host_track[i].active)
		{
			printk("%d:\n", idx++);
			printk("\tactive=%d\n", host_track[i].active);
			printk("\tdstip=%s\n", ip_inet_ntoa(host_track[i].dstip,host_track[i].ipver));
			printk("\tdport=%d\n", ntohs(host_track[i].dport));
			printk("\tsrcip=%s\n", ip_inet_ntoa(host_track[i].srcip,host_track[i].ipver));
			printk("\tsport=%d\n", ntohs(host_track[i].sport));
			printk("\ref=%d\n", host_track[i].ref_cnt);
		}
	}
	if (0 == idx)
		printk("host track is not active!\n");
}

unsigned int getAvgRate(unsigned int samplenum)
{
	unsigned int avgRate=0;

	/* always drop the first sample */
	if(samplenum >= 3)
	{
		avgRate = (hostspeeduprate[samplenum-1]-hostspeeduprate[0])/(samplenum-1);
		return avgRate;
	}
	return 0;
}

int HostSpeedUP_Info_read_proc(struct seq_file *seq, void *v)
{
	unsigned int rate, expectMinRate=0, expectMaxRate=0xffffffff, averageRate=0;
	unsigned int i, idx=0;
	unsigned int tmp;
	int delta=0;
	unsigned int samplenum = sampletime;
	unsigned long realtime;

	if (samplenum > 0)
	{
		/* add debug info for further debug */
		printk("rate[%d]: %d ", samplenum, hostspeeduprate[0]);
		for(i = 1; i < samplenum; i++)
		{
			printk("%d ", hostspeeduprate[i] - hostspeeduprate[i-1]);
		}
		printk("\n");
		
		averageRate = getAvgRate(samplenum);
		if(averageRate != 0)
		{
			expectMinRate = (averageRate*7)>>3;
			expectMaxRate = (averageRate*9)>>3;
		}
	}
	//seq_printf(seq,"totalbyte(%u):%llu\n",(speedtestSampleRxBytes>1024)?(samplenum+1):samplenum,
	//	speedtestTotalRxBytes);

	if(speedtestRealtimeEnd>speedtestRealtimeStart)
		realtime = speedtestRealtimeEnd-speedtestRealtimeStart;
	else
		realtime = (long)speedtestRealtimeEnd-(long)speedtestRealtimeStart;
	seq_printf(seq,"speedtestRealtimeStart=%lu, speedtestRealtimeEnd=%lu, realtime=%lu\n",speedtestRealtimeStart, speedtestRealtimeEnd, realtime);
	if((realtime%HZ)>=(HZ/2))
		realtime = (realtime/HZ)+1;
	else
		realtime = realtime/HZ;
	seq_printf(seq,"totalbyte(%lu):%llu\n",realtime, speedtestTotalRxBytes);
	
#if 0
	seq_printf(seq,"true sample:");
	for(i = 0; i < samplenum; i++)
	{
		seq_printf(seq,"%d ", hostspeeduprate[i]);
	}
	if(speedtestSampleRxBytes>1024 && samplenum>1)
		seq_printf(seq,"%d", hostspeeduprate[samplenum-1]+(speedtestSampleRxBytes>>10));	
	seq_printf(seq,"\n");
#endif
	seq_printf(seq,"valid sample num:%d\n",samplenum);
	for(i=0; i<samplenum; i++)
	{
		tmp = hostspeeduprate[i];

		if (i >= 3)
		{
			rate = hostspeeduprate[i] - hostspeeduprate[i-1];
			if (rate < expectMinRate)
			{
				delta += expectMinRate - rate;
			}
			else if (rate > expectMaxRate)
			{
				delta += expectMaxRate - rate;
			}
		}
		
		tmp += delta;

		if (averageRate < 83200)//650Mbps
			seq_printf(seq, "%u ", tmp);
		else
			seq_printf(seq, "%u ", tmp + tmp/10);
	}
	seq_printf(seq,"\n");
	
	seq_printf(seq, "real sample:\n");
	delta = 0;
	if (averageRate < 96000)//750Mbps
	{
		for (i=0; i<samplenum; i++)
			seq_printf(seq, "%u ", hostspeeduprate[i]);
	}
	else
	{
		for(i=0; i<samplenum; i++)
		{
			if (0 == i)
				seq_printf(seq, "%u ", hostspeeduprate[i]);
			else
			{
				rate = hostspeeduprate[i] - hostspeeduprate[i-1];
				if ((rate >= 102400) && (rate < 108800))//800MBps-850MBps
					delta += (rate>>4);
				else if ((rate >= 96000) && (rate < 102400))//750MBps-800MBps
					delta += rate/10;
				seq_printf(seq, "%u ", hostspeeduprate[i] + delta);
			}
		}
	}
	seq_printf(seq,"\n");
	
	for (i=0; i<10; i++)
	{
		if (host_track[i].active)
		{
			seq_printf(seq, "%d:\n", idx++);
			seq_printf(seq, "\tdstip=%s\n", ip_inet_ntoa(host_track[i].dstip,host_track[i].ipver));
			seq_printf(seq, "\tdport=%d\n", ntohs(host_track[i].dport));
			seq_printf(seq, "\tsrcip=%s\n", ip_inet_ntoa(host_track[i].srcip,host_track[i].ipver));
			seq_printf(seq, "\tsport=%d\n", ntohs(host_track[i].sport));
			seq_printf(seq, "\tref=%d\n", host_track[i].ref_cnt);
			seq_printf(seq, "\tbypass=%d\n", (host_track[i].bypassFastAck | bypassFastAckFlag));
			seq_printf(seq, "\tsack=%d\n", host_track[i].sack);
			seq_printf(seq, "\tdir=%d\n", host_track[i].direction);
		}
	}
	if (0 == idx)
		seq_printf(seq, "host track is not active\n");
#ifdef IP_SPEEDUP_DEBUG
	seq_printf(seq, "inOrderPktCnt %d outOrderPktCnt %d duplicatePktCnt %d revisePktCnt %d delayAckPktCnt %d longDelayAckPktCnt %d greateoutOrderPktCnt %d\n", 
			inOrderPktCnt, outOrderPktCnt, duplicatePktCnt, revisePktCnt, delayAckPktCnt, longDelayAckPktCnt, greateoutOrderPktCnt);
	for (i=0; i<11; i++)
	{
		if (softirq_delay_stats[i] != 0)
			seq_printf(seq, "softirq %d %s long delay %d times\n", i, softirq_new_name[i], softirq_delay_stats[i]);
	}
	for(i=0; i<CONFIG_NR_CPUS; i++)
	{
		seq_printf(seq, "delay ack on cpu%u: %u\n", i, delay_ack_num_on_cpu[i]);
	}
	seq_printf(seq, "sack exceed num:%u\n", sack_exceed_num);
#endif
#ifdef IP_SPEEDUP_DEBUG
	seq_printf(seq, "pause_xmit_count :%u\n", pause_xmit_count);
	seq_printf(seq, "normal_xmit_count :%u\n", normal_xmit_count);
	seq_printf(seq, "sack_ack_count :%u\n", sack_ack_count);
	seq_printf(seq, "force_tx_count :%u\n", force_tx_count);
	seq_printf(seq, "workqueue_tx_pkt :%u\n", workqueue_tx_pkt);
	seq_printf(seq, "quick_xmit_pkt :%u\n", quick_xmit_pkt);
	seq_printf(seq, "ps_xmit_pkt :%u\n", ps_xmit_pkt);
	seq_printf(seq, "total_xmit_pkt :%u\n", total_xmit_pkt);
	seq_printf(seq, "fail_xmit_pkt :%u\n", fail_xmit_pkt);
	seq_printf(seq, "zero_win_retrans :%u\n", zero_win_retrans);
	seq_printf(seq, "force_retrans_count :%u\n", force_retrans_count);
	seq_printf(seq, "long_delay_count :%u\n", long_delay_count);
	seq_printf(seq, "over_burst_count :%u\n", over_burst_count);
	seq_printf(seq, "all_ack_count :%u/%u\n", all_ack_count, all_ack_pause_count);
	for (i=0; i<all_ack_count && i<100; i++)
		seq_printf(seq, "%u ", send_delay_time[i]);
	seq_printf(seq, "\n");
#endif
	seq_printf(seq, "max_buf_len: %u (max %u)\n", max_buf_len, MAX_BUF_LEN);
	seq_printf(seq, "upload test: %s\n", doUploadTest?"going":"stop");
	
	return 0;
}

static int proc_HostSpeedUP_Info_open(struct inode *inode, struct file *file)
{
	return single_open(file, HostSpeedUP_Info_read_proc, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
ssize_t HostSpeedUP_Info_write_proc(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
#else
int HostSpeedUP_Info_write_proc(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
#endif
{	
	char	tmpbuf[512]={0};
	u8 dip[IP6_ADDR_LEN]={0}, sip[IP6_ADDR_LEN]={0};
	u16 dport=0, sport=0, key=0;
	int bypass=-1, sack=-1;
	int action;	//0-add 1-del 2-set
	int upload = 0;
	int ipv=0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count))
	{
		char *strptr,*split_str;

		//printk("\n[%s cpu %d] %s\n", __func__, smp_processor_id(), tmpbuf);
		tmpbuf[count] = '\0';
		strptr=tmpbuf;
		split_str=strsep(&strptr," ");
		/*parse command*/
		if ((strcasecmp(split_str, "add") == 0) || (strcasecmp(split_str, "set") == 0))
		{
			if (strcasecmp(split_str, "add") == 0)
				action = 0;
			else
				action = 2;
			
			while(1){
				split_str=strsep(&strptr," ");
				if(strcasecmp(split_str, "lip") == 0)
				{
					split_str=strsep(&strptr," ");
					if(strchr(split_str,'.'))
						in4_pton(split_str,-1,dip,-1,NULL);
					else
					{
						in6_pton(split_str,-1,dip,-1,NULL);
						ipv = 1;
					}
				}
				else if(strcasecmp(split_str, "lport") == 0)
				{
					split_str=strsep(&strptr," ");
					dport=simple_strtol(split_str, NULL, 0);
					dport=htons(dport);
					key=1;
//printk("%s-%d dport=%d\n",__func__,__LINE__,dport);

				}
				else if(strcasecmp(split_str, "rip") == 0)
				{
					split_str=strsep(&strptr," ");
					if(strchr(split_str,'.'))
						in4_pton(split_str,-1,sip,-1,NULL);
					else
					{
						in6_pton(split_str,-1,sip,-1,NULL);
						ipv = 1;
					}
				}
				else if(strcasecmp(split_str, "rport") == 0)
				{
					split_str=strsep(&strptr," ");
					sport=simple_strtol(split_str, NULL, 0);
					sport=htons(sport);
					key=1;					
//printk("%s-%d sport=%d\n",__func__,__LINE__,sport);

				}
				else if (strcasecmp(split_str, "bypass") == 0)
				{
					split_str=strsep(&strptr," ");
					bypass = simple_strtol(split_str, NULL, 0);
				}
				else if (strcasecmp(split_str, "sack") == 0)
				{
					split_str=strsep(&strptr," ");
					sack = simple_strtol(split_str, NULL, 0);
				}
				else if (strcasecmp(split_str, "upload") == 0)
				{
					split_str=strsep(&strptr," ");
					upload = simple_strtol(split_str, NULL, 0);
				}
				if (strptr==NULL) break;
			}
		}
		else if(strcasecmp(split_str, "del") == 0)
		{
			action = 1;
			
			while(1){
				split_str=strsep(&strptr," ");
				if(strncasecmp(split_str, "info", 4) == 0)
				{
					write_lock_bh(&speedup_lock);
					host_track_off(0, 0, 0, 0, 0, 0);
					write_unlock_bh(&speedup_lock);
					dump_host_track();
					goto err_skip_track;
				}
				if(strncasecmp(split_str, "uploadinfo", 4) == 0)
				{
					write_lock_bh(&speedup_lock);
					host_track_off(0, 0, 0, 0, 1, 0);
					write_unlock_bh(&speedup_lock);
					dump_host_track();
					goto err_skip_track;
				}
				else if(strcasecmp(split_str, "lip") == 0)
				{
					split_str=strsep(&strptr," ");
					if(strchr(split_str,'.'))
						in4_pton(split_str,-1,dip,-1,NULL);
					else
					{
						in6_pton(split_str,-1,dip,-1,NULL);
						ipv = 1;
					}
				}
				else if(strcasecmp(split_str, "lport") == 0)
				{
					split_str=strsep(&strptr," ");
					dport=simple_strtol(split_str, NULL, 0);
					dport = htons(dport);
					key=1;
				}
				else if(strcasecmp(split_str, "rip") == 0)
				{
					split_str=strsep(&strptr," ");
					if(strchr(split_str,'.'))
						in4_pton(split_str,-1,sip,-1,NULL);
					else
					{
						in6_pton(split_str,-1,sip,-1,NULL);
						ipv = 1;
					}
				}
				else if(strcasecmp(split_str, "rport") == 0)
				{
					split_str=strsep(&strptr," ");
					sport=simple_strtol(split_str, NULL, 0);
					sport=htons(sport);
					key=1;					
				}
				if (strptr==NULL) break;
			}
		}
		else if(strcasecmp(split_str, "dbg") == 0)
		{
			split_str=strsep(&strptr," ");
			dbgflag = simple_strtol(split_str, NULL, 0);
			printk("dbgflag %d\n", dbgflag);
			goto err_skip_track;
		}
		else if(strcasecmp(split_str, "max_buf_len") == 0)
		{
			unsigned int len;
			split_str=strsep(&strptr," ");
			len = simple_strtol(split_str, NULL, 0);
			if(len < 512)
				printk("error to set max_buf_len(%u), the min limit is 512\n", len);
#ifdef SKB_CACHE_ALLOC
			else if(len > MAX_BUF_LEN)
				printk("error to set max_buf_len(%u), the max limit is %u\n", len, MAX_BUF_LEN);
#endif
			else {
				max_buf_len = len;
				printk("max_buf_len %u\n", max_buf_len);
			}
			goto err_skip_track;
		}
		else{
			printk("error command!\n");
		}

	}
	if(!key){
		printk("you must set one rule at less\n");
		HostSpeedUP_USAGE();		
		goto err_skip_track;
	}
	if(!sip[0] /*|| !dip[0]*/){
		printk("you must set rip(Remote IP) or lip (Local IP)(ex: A.B.C.D=192.168.1.1)\n");
		HostSpeedUP_USAGE();		
		goto err_skip_track;

	}
	write_lock_bh(&speedup_lock);
	if (0 == action)//add
		host_track_on(dip, sip, dport, sport, upload, ipv);
	else if (2 == action)//set
		host_track_set(dip, sip, dport, sport, bypass, sack, ipv);
	else
		host_track_off(dip, sip, dport, sport, 2, ipv);
	write_unlock_bh(&speedup_lock);
	//dump_host_track();

	//printk("%s parse command done!\n", __func__);
	err_skip_track:
	
	return count;
}

#if 0
static void dump_host_stream(void)
{
	struct _speedup_stream_track *pEntry, *pNextEntry;
	int i, j;

	for (i=0; i<10; i++)
	{
		if (0 == host_track[i].active)
			break;

		for (j=0; j<SPEEDUP_HASH_SIZE; j++)
		{
			list_for_each_entry_safe(pEntry, pNextEntry, &host_track[i].hash[j], node)
			{
				printk("stream %d hash %d total size %d \n", i, j, pEntry->totalbytes);
				printk("	dstip=%s\n", ip_inet_ntoa(pEntry->dstip));
				printk("	dport=%d\n", pEntry->dport);
				printk("	srcip=%s\n", ip_inet_ntoa(pEntry->srcip));
				printk("	sport=%d\n", pEntry->sport);
				printk("	id=%d\n", pEntry->id);
				printk("	seq=%x\n", pEntry->rcv_nxt);
			}
		}
	}
}
#endif

static int HostSpeedUP_Stream_read_proc(struct seq_file *seq, void *v)
{
	struct _speedup_stream_track *pEntry;
	int i, j, k;

	for (i=0; i<10; i++)
	{
		if (0 == host_track[i].active)
			break;

		for (j=0; j<SPEEDUP_HASH_SIZE; j++)
		{
			list_for_each_entry(pEntry, &host_track[i].hash[j], node)
			{
				seq_printf(seq, "stream %d hash %d total size %d \n", i, j, pEntry->totalbytes);
				seq_printf(seq, "	dstip=%s\n", ip_inet_ntoa(pEntry->dstip, host_track[i].ipver));
				seq_printf(seq, "	dport=%d\n", ntohs(pEntry->dport));
				seq_printf(seq, "	srcip=%s\n", ip_inet_ntoa(pEntry->srcip, host_track[i].ipver));
				seq_printf(seq, "	sport=%d\n", ntohs(pEntry->sport));
				seq_printf(seq, "	wscale=%u\n", pEntry->wscale);
				seq_printf(seq, "	mss=%u\n", pEntry->snd_mss);
				seq_printf(seq, "	id=%d\n", pEntry->id);
				seq_printf(seq, "	seq=%x\n", pEntry->rcv_nxt);
				seq_printf(seq, "	dir=%x\n", pEntry->direction);
				seq_printf(seq, "	payload_len=%u\n", pEntry->payload_len);
				seq_printf(seq, "	sack_blk_num=%d\n", pEntry->sack_blk_num);
				for (k=0; k<pEntry->sack_blk_num; k++)
				{
					seq_printf(seq, "	sack_blk[%d]=%x-%x\n", k, pEntry->sack_blk[k].start_seq, pEntry->sack_blk[k].end_seq);
				}
#ifdef IP_SPEEDUP_OUTORDER_DBG
				seq_printf(seq, "	ACK Seq=%x\n", pEntry->ackSeq);
				seq_printf(seq, "	ACK blk_num=%x\n", pEntry->blk_num);
				for (k=0; k<pEntry->blk_num; k++)
				{
					seq_printf(seq, "	blk[%d]=%x-%x\n", k, pEntry->blk[k].start_seq, pEntry->blk[k].end_seq);
				}
#endif
			}
		}
	}

	return 0;
}

static int proc_HostSpeedUP_Stream_open(struct inode *inode, struct file *file)
{
	return single_open(file, HostSpeedUP_Stream_read_proc, inode->i_private);
}

/* echo get/set sip %%s sport %%d dip %%s dport %%s > HostSpeedUP-STREAM */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
ssize_t HostSpeedUP_Stream_write_proc(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
#else
int HostSpeedUP_Stream_write_proc(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
#endif
{
	char tmpbuf[512]={0};
	u8 dip[IP6_ADDR_LEN], sip[IP6_ADDR_LEN];
	u16 dport=0, sport=0, key=0;
	int uploadFlag=0;
	unsigned int wscale = 2;
	unsigned int mss = 0;
	int action=0;
	int ipv=0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count))
	{
		char *strptr,*split_str;

		//printk("\n[%s cpu %d] %s\n", __func__, smp_processor_id(), tmpbuf);
		tmpbuf[count] = '\0';
		strptr=tmpbuf;
		split_str=strsep(&strptr," ");
		/*parse command*/
		if (strcasecmp(split_str, "add") == 0)
		{
			action = SPEEDUP_STREAM_ADD;
		}
		else if (strcasecmp(split_str, "del") == 0)
		{
			action = SPEEDUP_STREAM_DEL;
		}
		else
		{
			printk("%s error command!\n", __func__);
			goto err_skip_track;
		}
		
		while(strptr)
		{
			split_str=strsep(&strptr," ");
			if(split_str == NULL)
			{
				break;
			}
			else if(strcasecmp(split_str, "sip") == 0)
			{
				split_str=strsep(&strptr," ");
				if(strchr(split_str,'.'))
					in4_pton(split_str,-1,sip,-1,NULL);
				else
				{
					in6_pton(split_str,-1,sip,-1,NULL);
					ipv = 1;
				}
				key = 1;
			}
			else if(strcasecmp(split_str, "sport") == 0)
			{
				split_str=strsep(&strptr," ");
				sport=simple_strtol(split_str, NULL, 0);
				sport=htons(sport);
				key=1;
				//printk("%s-%d sport=%d\n",__func__,__LINE__,sport);
			}
			else if(strcasecmp(split_str, "dip") == 0)
			{
				split_str=strsep(&strptr," ");
				if(strchr(split_str,'.'))
					in4_pton(split_str,-1,dip,-1,NULL);
				else
				{
					in6_pton(split_str,-1,dip,-1,NULL);
					ipv = 1;
				}
				key = 1;
			}
			else if(strcasecmp(split_str, "dport") == 0)
			{
				split_str=strsep(&strptr," ");
				dport=simple_strtol(split_str, NULL, 0);
				dport=htons(dport);
				key=1;					
				//printk("%s-%d dport=%d\n",__func__,__LINE__,dport);
			}
			else if(strcasecmp(split_str, "upload") == 0)
			{
				split_str=strsep(&strptr," ");
				uploadFlag = simple_strtol(split_str, NULL, 0);
				//printk("%s-%d dport=%d\n",__func__,__LINE__,dport);
			}
			else if(strcasecmp(split_str, "snd_wscale") == 0)
			{
				split_str=strsep(&strptr," ");
				wscale = simple_strtol(split_str, NULL, 0);
			}
			else if(strcasecmp(split_str, "snd_mss") == 0)
			{
				split_str=strsep(&strptr," ");
				mss = simple_strtol(split_str, NULL, 0);
			}
		}
	}
	
	if(!key){
		printk("you must set one rule at less\n");
		printk("e.g\n");
		printk("echo add sip 192.168.2.1 sport 2000 dip 10.10.10.10 dport 80 total 2000000 > HostSpeedUP-STREAM\n");		
		goto err_skip_track;
	}

#if 0
	printk("------- host_stream_track --------\n");
	printk("dstip=%s\n", ip_inet_ntoa(dip));
	printk("dport=%d\n", dport);
	printk("srcip=%s\n", ip_inet_ntoa(sip));
	printk("sport=%d\n", sport);
#endif
	write_lock_bh(&speedup_lock);
	
	if (SPEEDUP_STREAM_ADD == action)
		_speedup_stream_track_record(dip, sip, dport, sport, !!uploadFlag, wscale, mss, ipv);
	else if (SPEEDUP_STREAM_DEL == action)
		_speedup_stream_track_del(dip, sip, dport, sport, ipv);
	write_unlock_bh(&speedup_lock);

	//printk("%s parse command done!\n", __func__);
	//dump_host_stream();
err_skip_track:
	return count;
}

#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
static int HostSpeedUP_skipFc_read_proc(struct seq_file *seq, void *v)
{
	seq_printf(seq, "skipFcFunc Bit = %d\n", speedtest_skip_fc_Bit);

	return 0;
}

static int proc_HostSpeedUP_skipFc_open(struct inode *inode, struct file *file)
{
	return single_open(file, HostSpeedUP_skipFc_read_proc, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
ssize_t HostSpeedUP_skipFc_write_proc(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
#else
int HostSpeedUP_skipFc_write_proc(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
#endif
{
	char tmpbuf[512]={0}; 
	unsigned int vInt;

	if (buffer && !copy_from_user(tmpbuf, buffer, count))
	{	
		vInt = simple_strtoul(tmpbuf, NULL, 10);
		if (vInt >= 64)
		{
			printk("skb->mark2 bit should be less than 64\n");
			return -EFAULT;
		}
		speedtest_skip_fc_Bit = vInt;
		
		return count;
	}	
	return -EFAULT;
}
#endif//endof CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops hostspeedup_dbg_fops = {
    .proc_open           = proc_HostSpeedUP_dbg_open,
    .proc_read           = seq_read,
    .proc_write          = HostSpeedUP_dbg_write_proc,
    .proc_lseek         = seq_lseek,
    .proc_release        = single_release,
};
#else
static const struct file_operations hostspeedup_dbg_fops = {
	.owner          = THIS_MODULE,
	.open           = proc_HostSpeedUP_dbg_open,
	.read           = seq_read,
	.write          = HostSpeedUP_dbg_write_proc,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops hostspeedup_fops = {
    .proc_open           = proc_HostSpeedUP_open,
    .proc_read           = seq_read,
    .proc_write          = HostSpeedUP_write_proc,
    .proc_lseek         = seq_lseek,
    .proc_release        = single_release,
};
#else
static const struct file_operations hostspeedup_fops = {
	.owner          = THIS_MODULE,
	.open           = proc_HostSpeedUP_open,
	.read           = seq_read,
	.write          = HostSpeedUP_write_proc,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops hostspeedup_info_fops = {
    .proc_open           = proc_HostSpeedUP_Info_open,
    .proc_read           = seq_read,
    .proc_write          = HostSpeedUP_Info_write_proc,
    .proc_lseek         = seq_lseek,
    .proc_release        = single_release,
};
#else
static const struct file_operations hostspeedup_info_fops = {
	.owner          = THIS_MODULE,
	.open           = proc_HostSpeedUP_Info_open,
	.read           = seq_read,
	.write          = HostSpeedUP_Info_write_proc,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops hostspeedup_stream_fops = {
    .proc_open           = proc_HostSpeedUP_Stream_open,
    .proc_read           = seq_read,
    .proc_write          = HostSpeedUP_Stream_write_proc,
    .proc_lseek         = seq_lseek,
    .proc_release        = single_release,
};
#else
static const struct file_operations hostspeedup_stream_fops = {
	.owner          = THIS_MODULE,
	.open           = proc_HostSpeedUP_Stream_open,
	.read           = seq_read,
	.write          = HostSpeedUP_Stream_write_proc,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif

#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops hostspeedup_skipFc_fops = {
    .proc_open           = proc_HostSpeedUP_skipFc_open,
    .proc_read           = seq_read,
    .proc_write          = HostSpeedUP_skipFc_write_proc,
    .proc_lseek         = seq_lseek,
    .proc_release        = single_release,
};
#else
static const struct file_operations hostspeedup_skipFc_fops = {
	.owner          = THIS_MODULE,
	.open           = proc_HostSpeedUP_skipFc_open,
	.read           = seq_read,
	.write          = HostSpeedUP_skipFc_write_proc,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#endif
#endif//endof CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE

static void setup_Timer_for_speedup(void)
{
//TODO
#ifdef CONFIG_ARCH_CORTINA
	if(tmr_cnt2_addr == NULL){
	PER_TMR_LOADE_t tmr_loade;
	PER_TMR_CTRL2_t tmr_ctrl2;
	PER_TMR_LD2_t tmr_LD2;
	void __iomem *addr;

	printk("====   set SPEEDTEST CA TIMER2 =====\n");
	tmr_LD2.wrd = 0xffffffff;
	addr = ioremap(PER_TMR_LD2, 4);
	writel_relaxed(tmr_LD2.wrd, addr);
	iounmap(addr);
	tmr_loade.wrd = 0;
	tmr_loade.bf.update_tmr2 = 1;
	addr = ioremap(PER_TMR_LOADE, 4);
	writel_relaxed(tmr_loade.wrd, addr);
	iounmap(addr);
	tmr_ctrl2.wrd = 0;
	tmr_ctrl2.bf.timen2 = 1;
	tmr_ctrl2.bf.rlmode2 = 0;
	tmr_ctrl2.bf.clksel2 = 1;//0.5us
	addr = ioremap(PER_TMR_CTRL2, 4);
	writel_relaxed(tmr_ctrl2.wrd, addr);
	iounmap(addr);
	tmr_cnt2_addr=ioremap( PER_TMR_CNT2, 4);
	}
#else
	int offset;

	printk("====   SPEEDTEST TIMER %d   =====\n", SPEEDTEST_TIMER_IDX);
	
	offset = SPEEDTEST_TIMER_IDX * 0x10;
	/* disable timer */
	REG32(BSP_TC0CTL + offset) = 0x0; /* disable timer before setting CDBR */
	/* initialize timer registers */
	REG32(BSP_TC0CTL + offset) |= (200) << 0;	//1us
	REG32(BSP_TC0DATA + offset) = 0xFFFFFFF;
	/* enable timer */
	REG32(BSP_TC0CTL + offset) |= BSP_TCEN | BSP_TCMODE_TIMER;
	REG32(BSP_TC0INT + offset) &= ~BSP_TCIE;

	printk("--------------------------------\n");
	printk("BSP_CTRLR(%08x): %x\n", BSP_TC0CTL + offset, REG32(BSP_TC0CTL + offset));
	printk("BSP_DATAR(%08x): %x\n", BSP_TC0DATA + offset, REG32(BSP_TC0DATA + offset));
	printk("BSP_INTRR(%08x): %x\n", BSP_TC0INT + offset, REG32(BSP_TC0INT + offset));
	printk("--------------------------------\n");
#endif
}

static long host_speedup_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	long ret = -EFAULT;
	void __user *argp = (void __user *)arg;
	
	switch(cmd) {
		case SIOCSPEEDTEST: {
			int ipver=0;
			struct speedtest_arg sp_arg;
			
			if (copy_from_user(&sp_arg, argp, sizeof(struct speedtest_arg)))
				return -EFAULT;
			
			switch(sp_arg.cmd)
			{
				case 1:
					if(sp_arg.ip_familly == AF_INET6)
						ipver = 1;
					sp_arg.ip_familly = HostSpeedUP_Stat_get(sp_arg.dip, sp_arg.sip, sp_arg.dport, sp_arg.sport, ipver);
					break;

			}
			if(copy_to_user(argp, &sp_arg, sizeof(struct speedtest_arg)))
				return -EFAULT;
			ret = 0;
			break;
		}
	}

	return ret;
}

static const struct file_operations host_speedup_fops = {
    .owner			= THIS_MODULE,
    //.write		= host_speedup_write,
    //.open			= host_speedup_open,
    //.release		= host_speedup_close,	
    .llseek 		= no_llseek,
	.unlocked_ioctl = host_speedup_ioctl,
};

struct miscdevice host_speedup_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_SPEEDUP_NAME,
    .fops = &host_speedup_fops,
};

static int __init HostSpeedUP_proc_init(void)
{
	int err;
	struct proc_dir_entry *entry=NULL;
	
	max_buf_len = MAX_BUF_LEN/2;
	
	err = misc_register(&host_speedup_device);
    if (err) {
        pr_err("can't misc_register :(\n");	
        goto error0;
    }

    _host_track_init();
	setup_Timer_for_speedup();

	//memset(&upload_tx_tasklets, 0, sizeof(struct tasklet_struct));
	//upload_tx_tasklets.func=(void (*)(unsigned long))send_upload_data_tasklet;
	//upload_tx_tasklets.data=(unsigned long)NULL;
#ifdef DELAY_WORK
	INIT_DELAYED_WORK(&upload_work, wq_do_upload_speedtest);
	INIT_DELAYED_WORK(&download_work, wq_do_download_speedtest);
#else
	INIT_WORK(&upload_work, wq_do_upload_speedtest);
	INIT_WORK(&download_work, wq_do_download_speedtest);
#endif

	entry = proc_create_data("HostSpeedUP", 0644, NULL, &hostspeedup_fops,NULL);
	if (entry == NULL)
	{
		printk("%s-%d create proc HostSpeedUP fail!\n", __func__, __LINE__);	
		goto error1;
	}
	
	entry = proc_create_data("HostSpeedUP-Info", 0644, NULL, &hostspeedup_info_fops,NULL);
	if (entry == NULL)
	{
		printk("%s-%d create proc HostSpeedUP-Info fail!\n", __func__, __LINE__);		
		goto error2;
	}

	entry = proc_create_data("HostSpeedUP-STREAM", 0644, NULL, &hostspeedup_stream_fops,NULL);
	if (entry == NULL)
	{
		printk("%s-%d create proc HostSpeedUP-STREAM fail!\n", __func__, __LINE__);		
		goto error3;
	}

	entry = proc_create_data("speedtest_dbg", 0644, NULL, &hostspeedup_dbg_fops,NULL);
	if (entry == NULL)
	{
		printk("%s-%d create proc speedtest_dbg fail!\n", __func__, __LINE__); 	
		goto error4;
	}
	
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	entry = proc_create_data("HostSpeedUP-SkipFcBit", 0644, NULL, &hostspeedup_skipFc_fops,NULL);
	if (entry == NULL)
	{
		printk("%s-%d create proc HostSpeedUP-SkipFcBit fail!\n", __func__, __LINE__); 	
		goto error;
	}
#endif//endof CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#ifdef TRACEPOINTS_ENABLED
	for_each_kernel_tracepoint(lookup_tracepoints, NULL);
#endif
#ifdef SKB_CACHE_ALLOC
	skb_upload_cache = kmem_cache_create("skb_speedtest_upload", MAX_BUF_LEN, 0, 0, NULL);
	if(skb_upload_cache == NULL)
	{
		printk("%s-%d Cannot allocate upload skb cache!\n", __func__, __LINE__); 	
		goto error;
	}
#endif

	return 0;

error:
#if 0 //def CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	remove_proc_entry("HostSpeedUP-SkipFcBit", NULL);
#endif
	remove_proc_entry("speedtest_dbg", NULL);
error4:
	remove_proc_entry("HostSpeedUP-STREAM", NULL);
error3:	
	remove_proc_entry("HostSpeedUP-Info", NULL);
error2:	
	remove_proc_entry("HostSpeedUP", NULL);
error1:
#ifdef CONFIG_ARCH_CORTINA
	if(tmr_cnt2_addr != NULL){
		iounmap(tmr_cnt2_addr);
		tmr_cnt2_addr = NULL;
	}
#endif
#ifdef DELAY_WORK 
	cancel_delayed_work_sync(&upload_work);
	cancel_delayed_work_sync(&download_work);
#else
	cancel_work_sync(&upload_work);
	cancel_work_sync(&download_work);
#endif
	misc_deregister(&host_speedup_device);	
error0:
	return -ENOMEM;
}

static void __exit HostSpeedUP_proc_exit(void)
{
	HostSpeedUP = 0;
	HostSpeedUP_config_check();
#ifdef DELAY_WORK 
	cancel_delayed_work_sync(&upload_work);
	cancel_delayed_work_sync(&download_work);
#else
	cancel_work_sync(&upload_work);
	cancel_work_sync(&download_work);
#endif
	remove_proc_entry("HostSpeedUP", NULL);
	remove_proc_entry("HostSpeedUP-Info", NULL);
	remove_proc_entry("HostSpeedUP-STREAM", NULL);
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	remove_proc_entry("HostSpeedUP-SkipFcBit", NULL);
#endif
	remove_proc_entry("speedtest_dbg", NULL);
	
#ifdef CONFIG_ARCH_CORTINA
	if(tmr_cnt2_addr != NULL){
		iounmap(tmr_cnt2_addr);
		tmr_cnt2_addr = NULL;
	}
#endif
#ifdef SKB_CACHE_ALLOC
	if(skb_upload_cache) kmem_cache_destroy(skb_upload_cache);
#endif
	misc_deregister(&host_speedup_device);
}

module_param(MAX_BUF_LEN, int, S_IRUGO);

module_init(HostSpeedUP_proc_init)
module_exit(HostSpeedUP_proc_exit)

MODULE_DESCRIPTION("HostSpeedUP");
MODULE_AUTHOR("sd5@realtk.com>");
MODULE_LICENSE("GPL");






