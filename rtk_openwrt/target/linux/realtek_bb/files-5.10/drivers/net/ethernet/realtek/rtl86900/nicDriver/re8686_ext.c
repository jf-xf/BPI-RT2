//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/list.h>

#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#elif defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#elif defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#elif defined(CONFIG_RTL9607C_SERIES)
#include "re8686_rtl9607c.h"
#elif defined(CONFIG_RTL8198D_SERIES)
#include "re8686_rtl8198d.h"
#else
#include "re8686_nic.h"
#endif

#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
int FastPath_Check(struct sk_buff *skb);
#endif

#define PTP_TRAP      0x7e
#define PTP_MIRROR    0x7f

#define DBG_PKT_FIFO_COUNT  16

#define DEV2CP(dev)  (((struct re_dev_private*)netdev_priv(dev))->pCp)

/*#define FPGA_9602C_DEFINED 1*/

struct pkt_dbg_s
{
    /* Capture the latest packet */
    unsigned int enable;
    unsigned int pkt_length;
    struct rx_info rx_desc;
    unsigned char raw_pkt[SKB_BUF_SIZE];
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
    unsigned int rx_gmac;
#endif
    /* Capture up to FIFO size packets */
    unsigned char fifo_valid_count;
    unsigned char raw_pkt_fifo[DBG_PKT_FIFO_COUNT][64];
};


typedef struct drv_nic_hook_entry
{
	int portmask;
	int priority;
	p2rfunc_t do_rx;
	struct list_head list;

}drv_nic_hook_entry_t ;

/* for rx handler mantian*/
static struct list_head rxHookHead;

/* Use to store the dump packet settings and data */
static struct pkt_dbg_s re_dbg_data;


static int
drv_nic_insert_entry(drv_nic_hook_entry_t *entry)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

		printk("priority: %d, portmask %d, rx: %p\n",cur->priority,cur->portmask,cur->do_rx);

	    if(cur->priority <= entry->priority){
			cur->list.prev->next = &entry->list;
			entry->list.next = &cur->list;
			entry->list.prev = cur->list.prev;
			cur->list.prev = &entry->list;
			printk("insert here, priority: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);
			return 0;
		}
	}
	/*can't search means it's the first entry*/
	list_add(&entry->list,&rxHookHead);
	printk("first entry: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);

	return 0;
}


/*	Purpose: Used for hook rx callback function
*	Parameters:
*  		portmask - this callback function want to receive from which ports
*		priority	- the priority for callback fun, 0~100, 0: lowest, 100: highest
*		rx		- callback function
*		Note-
*			Please update the priority of your callback in here and the doc file in
*			http://cadinfo.realtek.com.tw/svn/CN/Switch/trunk/dumb/document/Apollo/Internal Spec/NIC/[Application Note] GMAC_v1.0.0_20120806.doc
*
*/


int
drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx)
{

	drv_nic_hook_entry_t *entry;
	entry = (drv_nic_hook_entry_t*)kmalloc(sizeof(drv_nic_hook_entry_t),GFP_KERNEL);
	if(!entry){
		return -1;
	}
	/*assigned value to entry*/
	entry->do_rx = rx;
	entry->portmask = portmask;
	entry->priority = priority;
	/*insert entry depend on priority*/
	drv_nic_insert_entry(entry);
	return 0;
}


int
drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

	    	if(cur->do_rx == rx && cur->portmask == portmask && cur->priority==priority){
			list_del(&cur->list);
			kfree(cur);
			return 0;
		}
	}
	return -1;
}


static void
drv_nic_rxhook_clear(void)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

	    	if(cur!=NULL){
			list_del(&cur->list);
			kfree(cur);
		}
	}
	return;
}


/*__IRAM*/
static int
drv_nic_rx_list(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;
	int ret;
	int portNum;

#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
	int skip_rg=0;
#endif

#ifdef FPGA_9602C_DEFINED
    if(skb->data[12] == 0x88 && skb->data[13] == 0x99 && skb->data[14] == 0x04)
    {
        memset(pRxInfo, 0x0, sizeof(struct rx_info));
        pRxInfo->opts2.bit.cputag = 1;
        pRxInfo->opts3.bit.reason = skb->data[15];
        pRxInfo->opts3.bit.internal_priority = ((skb->data[16]>>5)&0x7);
        pRxInfo->opts1.bit.l3routing = ((skb->data[17]>>7)&0x1);
        pRxInfo->opts1.bit.origformat = ((skb->data[17]>>6)&0x1);
        pRxInfo->opts3.bit.src_port_num = (skb->data[17]&0x7);
        pRxInfo->opts1.bit.pctrl = ((skb->data[18]>>7)&0x1);
        pRxInfo->opts2.bit.pon_stream_id = (skb->data[18]&0x7f);
        pRxInfo->opts3.bit.dst_port_mask = (skb->data[19]&0x7f);

        memmove(&skb->data[12], &skb->data[20], skb->len-20);
        skb_trim(skb, skb->len-8);

    }
#endif

#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
	skip_rg = FastPath_Check(skb);
#endif

    portNum = (int)GMAC_RXINFO_SRC_PORT_NUM(pRxInfo);
	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

		if(cur->portmask & (1 << portNum)){
#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
			if(skip_rg && cur->priority == RE8686_RXPRI_RG)
				continue;
#endif

			ret = cur->do_rx(cp,skb,pRxInfo);
			if(ret==RE8670_RX_STOP){
				if(skb){
					__kfree_skb(skb);
				}
				return 0;
		    }
			else if(ret==RE8670_RX_STOP_SKBNOFREE)	//return without free skb
				return 0;
		}
	}
	return 0;
}


int
drv_nic_rxhook_init(void)
{
	INIT_LIST_HEAD(&rxHookHead);
	/*add for default re8686 rx handler*/
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
	drv_nic_register_rxhook(0x7ff,RE8686_RXPRI_DEFAULT,re8670_rx_skb);
#else
	drv_nic_register_rxhook(0x7f,RE8686_RXPRI_DEFAULT,re8670_rx_skb);
#endif

	/*register to re8686 netdev*/
	re8686_register_rxfunc_all_port(&drv_nic_rx_list);

	return 0;
}

void
drv_nic_rxhook_exit(void)
{
	/*register to re8686 netdev*/
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
	re8686_reset_rxfunc_to_default(0U);
	re8686_reset_rxfunc_to_default(1U);
	re8686_reset_rxfunc_to_default(2U);
#else
	re8686_reset_rxfunc_to_default();
#endif
	/*clear all hook*/
	drv_nic_rxhook_clear();
}



int re8686_dump_rx(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
    if(re_dbg_data.rx_gmac != 0xffU && re_dbg_data.rx_gmac != cp->gmac)
    {
        return RE8670_RX_CONTINUE;
    }
#endif
	/* Copy the descriptor and packet content */
	memcpy(&re_dbg_data.rx_desc, pRxInfo, sizeof(struct rx_info));
    re_dbg_data.pkt_length = (u32)SKB_BUF_SIZE > skb->len ? skb->len : (u32)SKB_BUF_SIZE;
    memcpy(&re_dbg_data.raw_pkt[0], skb->data, re_dbg_data.pkt_length);

    if(re_dbg_data.fifo_valid_count < DBG_PKT_FIFO_COUNT)
    {
        memcpy(&re_dbg_data.raw_pkt_fifo[re_dbg_data.fifo_valid_count], skb->data, 64U);
        re_dbg_data.fifo_valid_count ++;
    }

	return RE8670_RX_CONTINUE;
	/* Use the original default process function to maintain the same action */
	/*re8670_rx_skb(cp, skb, pRxInfo);*/
}

/* This is the higest prio */
#define TX_PRIO_HI 4 

int re8686_tx_with_Info_dbg(unsigned char *pPayload, unsigned short length, void *pInfo)
{
    struct sk_buff *skb;
    struct tx_info mask;
#ifdef FPGA_9602C_DEFINED
    struct tx_info currentInfo;
    memset(&currentInfo, 0x0, sizeof(struct tx_info));
#endif

    if((NULL == pPayload) || (NULL == pInfo)) {
        return -1;
    }

    skb = dev_alloc_skb((u32)length+12U);//re8670_getAlloc(SKB_BUF_SIZE);
	if (unlikely(NULL == skb)) {
        return -1;
	}

    /* Clear and set all CPU tag bits to 1 as mask */
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
    memset(&mask, 0, sizeof(mask));
    GMAC_TXINFO_KEEP(&mask)             = 0x1U;
    //GMAC_TXINFO_BLU(&mask)              = 0x1;
    //GMAC_TXINFO_VSEL(&mask)             = 0x1;
    GMAC_TXINFO_DISLRN(&mask)           = 0x1U;
    //GMAC_TXINFO_CPUTAG_IPCS(&mask)      = 0x1;
    //GMAC_TXINFO_CPUTAG_L4CS(&mask)      = 0x1;
    GMAC_TXINFO_CPUTAG_PSEL(&mask)      = 0x1U;
    GMAC_TXINFO_ASPRI(&mask)            = 0x1U;
    GMAC_TXINFO_CPUTAG_PRI(&mask)       = 0x7U;
    GMAC_TXINFO_TX_PPPOE_ACTION(&mask)  = 0x3U;
    GMAC_TXINFO_TX_PPPOE_IDX(&mask)     = 0x7U;
    //GMAC_TXINFO_EFID(&mask)             = 0x1;
    //GMAC_TXINFO_ENHANCE_FID(&mask)      = 0x7;
    GMAC_TXINFO_EXTSPA(&mask)           = 0x7U;
    GMAC_TXINFO_TX_PMASK(&mask)         = 0x3ffU;
    GMAC_TXINFO_TX_DST_STREAM_ID(&mask) = 0x7fU;
    GMAC_TXINFO_L34_KEEP(&mask)         = 0x1U;
    /* Add tx command bits to 1 as mask */
    GMAC_TXINFO_IPCS(&mask)             = 0x1U;
    GMAC_TXINFO_L4CS(&mask)             = 0x1U;
    GMAC_TXINFO_CVLAN_VIDL(&mask)       = 0xffU;
    GMAC_TXINFO_CVLAN_PRIO(&mask)       = 0x7U;
    GMAC_TXINFO_CVLAN_CFI(&mask)        = 0x1U;
    GMAC_TXINFO_CVLAN_VIDH(&mask)       = 0xfU;
#else
    memset(&mask, 0, sizeof(mask));
    GMAC_TXINFO_KEEP(&mask)             = 0x1;
    GMAC_TXINFO_BLU(&mask)              = 0x1;
    GMAC_TXINFO_VSEL(&mask)             = 0x1;
    GMAC_TXINFO_DISLRN(&mask)           = 0x1;
    GMAC_TXINFO_CPUTAG_IPCS(&mask)      = 0x1;
    GMAC_TXINFO_CPUTAG_L4CS(&mask)      = 0x1;
    GMAC_TXINFO_CPUTAG_PSEL(&mask)      = 0x1;
    GMAC_TXINFO_ASPRI(&mask)            = 0x1;
    GMAC_TXINFO_CPUTAG_PRI(&mask)       = 0x7;
    GMAC_TXINFO_TX_PPPOE_ACTION(&mask)  = 0x3;
    GMAC_TXINFO_TX_PPPOE_IDX(&mask)     = 0x7;
    GMAC_TXINFO_EFID(&mask)             = 0x1;
    GMAC_TXINFO_ENHANCE_FID(&mask)      = 0x7;
    GMAC_TXINFO_EXTSPA(&mask)           = 0x7;
    GMAC_TXINFO_TX_PMASK(&mask)         = 0x3f;
    GMAC_TXINFO_TX_DST_STREAM_ID(&mask) = 0x7f;
    GMAC_TXINFO_L34_KEEP(&mask)         = 0x1;
    /* Add tx command bits to 1 as mask */
    GMAC_TXINFO_IPCS(&mask)             = 0x1;
    GMAC_TXINFO_L4CS(&mask)             = 0x1;
    GMAC_TXINFO_VIDL(&mask)             = 0xff;
    GMAC_TXINFO_PRIO(&mask)             = 0x7;
    GMAC_TXINFO_CFI(&mask)              = 0x1;
    GMAC_TXINFO_VIDH(&mask)             = 0xf;
#endif

    skb_put(skb, (u32)length);
    memcpy(skb->data, pPayload, (u32)length);

#ifdef FPGA_9602C_DEFINED
    {
        unsigned char *tmp_pint;
        skb_put(skb, 8);
		memmove(&skb->data[20], &skb->data[12], length-12);
        tmp_pint = skb->data+12;
        *tmp_pint = 0x88;
        tmp_pint++;
        *tmp_pint = 0x99;
        tmp_pint++;
        *tmp_pint = 0x04;
        tmp_pint++;
        *tmp_pint = ((struct tx_info *)pInfo)->opts3.bit.tx_portmask;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts2.bit.aspri << 3) |
                    (((struct tx_info *)pInfo)->opts2.bit.cputag_pri);

        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.l34_keep << 1) |
                    (((struct tx_info *)pInfo)->opts1.bit.cputag_psel << 4) |
                    (((struct tx_info *)pInfo)->opts1.bit.dislrn << 5) |
                    (((struct tx_info *)pInfo)->opts1.bit.keep << 7);
        tmp_pint++;
        *tmp_pint = 0x0;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.tx_dst_stream_id);
    }
    return re8686_send_with_txInfo_and_mask(skb, (struct tx_info *)&currentInfo, TX_PRIO_HI, &mask);
#else
    return re8686_send_with_txInfo_and_mask(skb, (struct tx_info *)pInfo, TX_PRIO_HI, &mask);
#endif
}

int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo)
{
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
	struct re_private_root *root_cp;
	struct re_private *cp;
	struct net_device *dev;
#endif
    struct sk_buff *skb;

#ifdef FPGA_9602C_DEFINED
    struct tx_info currentInfo;
    memset(&currentInfo, 0x0, sizeof(struct tx_info));
#endif

    if((NULL == pPayload) || (NULL == pInfo)) {
        return -1;
    }

#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
#define TX_GMAC_ID	0
	dev = dev_get_by_name(&init_net, "eth0");
	if(dev==NULL)
	{
		printk("no such dev named eth0 !!\n");
		return -1;
	}
	root_cp = DEV2CP(dev);
	cp = root_cp->re_private_data_ptr[TX_GMAC_ID];
	GMAC_SPIN_LOCK(&cp->tx_lock);
	skb = dev_alloc_skb(length+12);
	if (unlikely(NULL == skb)) {
		GMAC_SPIN_UNLOCK(&cp->tx_lock);
		return -1;
	}
	GMAC_SPIN_UNLOCK(&cp->tx_lock);
#else
    skb = dev_alloc_skb(length+12);//re8670_getAlloc(SKB_BUF_SIZE);
	if (unlikely(NULL == skb)) {
        return -1;
	}
#endif

    skb_put(skb, length);
    memcpy(skb->data, pPayload, length);

#ifdef FPGA_9602C_DEFINED
    {
        unsigned char *tmp_pint;
        skb_put(skb, 8);
        memmove(&skb->data[20], &skb->data[12], length-12);
        tmp_pint = skb->data+12;
        *tmp_pint = 0x88;
        tmp_pint++;
        *tmp_pint = 0x99;
        tmp_pint++;
        *tmp_pint = 0x04;
        tmp_pint++;
        *tmp_pint = ((struct tx_info *)pInfo)->opts3.bit.tx_portmask;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts2.bit.aspri << 3) |
                    (((struct tx_info *)pInfo)->opts2.bit.cputag_pri);
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.l34_keep << 1) |
                    (((struct tx_info *)pInfo)->opts1.bit.cputag_psel << 4) |
                    (((struct tx_info *)pInfo)->opts1.bit.dislrn << 5) |
                    (((struct tx_info *)pInfo)->opts1.bit.keep << 7);
        tmp_pint++;
        *tmp_pint = 0x0;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.tx_dst_stream_id);
    }
    return re8686_send_with_txInfo(skb, (struct tx_info *)&currentInfo, TX_PRIO_HI);
#else
    return re8686_send_with_txInfo(skb, (struct tx_info *)pInfo, TX_PRIO_HI);
#endif
}

int re8686_rx_pktDump_set(unsigned int enable)
{
    if(0U == enable)
    {
        if(re_dbg_data.enable != 0U)
        {
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
            drv_nic_unregister_rxhook(0x7ff,RE8686_RXPRI_DUMP,re8686_dump_rx);
#else
            drv_nic_unregister_rxhook(0x7f,RE8686_RXPRI_DUMP,re8686_dump_rx);
#endif
            re_dbg_data.enable = 0U;
        }
    }
    else
    {
        if(re_dbg_data.enable != 1U)
        {
            /*add for debug*/
#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
            drv_nic_register_rxhook(0x7ff,RE8686_RXPRI_DUMP,re8686_dump_rx);
#else
            drv_nic_register_rxhook(0x7f,RE8686_RXPRI_DUMP,re8686_dump_rx);
#endif
            re_dbg_data.enable = 1U;
        }
    }

    return 0;
}

int re8686_rx_pktDump_get(unsigned char *pPayload, unsigned short buf_length, unsigned short *pPkt_len, void *pInfo, unsigned int *pEnable)
{
    int len;

    len = (re_dbg_data.pkt_length > buf_length) ? (int)buf_length : (int)re_dbg_data.pkt_length;
    *pEnable = re_dbg_data.enable;
    *pPkt_len = (u16)re_dbg_data.pkt_length;

    if(0 == len)
        return 0;

    *((struct rx_info *)pInfo) = re_dbg_data.rx_desc;
    memcpy(pPayload, &re_dbg_data.raw_pkt[0], (u32)len);

    return 0;
}

int re8686_rx_pktFifoDump_get(unsigned int fifo_idx, unsigned char *pPayload, unsigned short buf_length)
{
    int len;

    if(fifo_idx >= re_dbg_data.fifo_valid_count)
        return -1;

    len = (64 > buf_length) ? buf_length : 64;

    if(0 == len)
        return 0;

    memcpy(pPayload, &re_dbg_data.raw_pkt_fifo[fifo_idx], (u32)len);

    return 0;
}

int re8686_rx_pktDump_clear(void)
{
    memset(&re_dbg_data.rx_desc, 0, sizeof(struct rx_info));
    memset(&re_dbg_data.raw_pkt[0], 0, sizeof(unsigned char) * SKB_BUF_SIZE);
    re_dbg_data.pkt_length = 0U;

    memset(&re_dbg_data.raw_pkt_fifo, 0, sizeof(unsigned char) * 64 * DBG_PKT_FIFO_COUNT);
    re_dbg_data.fifo_valid_count = 0U;

    return 0;
}

#if defined(CONFIG_RTL9607C_SERIES) || defined(CONFIG_RTL8198D_SERIES)
int re8686_rx_gmac_set(unsigned int gmac)
{
    re_dbg_data.rx_gmac = gmac;
    return 0;
}

int re8686_rx_gmac_get(unsigned int *pGmac)
{
    *pGmac = re_dbg_data.rx_gmac;
    return 0;
}
#endif

/* Export symbols */
EXPORT_SYMBOL(re8686_rx_pktDump_clear);
EXPORT_SYMBOL(re8686_tx_with_Info_dbg);
EXPORT_SYMBOL(re8686_tx_with_Info);
EXPORT_SYMBOL(re8686_rx_pktDump_set);
EXPORT_SYMBOL(re8686_rx_pktDump_get);
EXPORT_SYMBOL(re8686_send_with_txInfo);
EXPORT_SYMBOL(drv_nic_register_rxhook);
EXPORT_SYMBOL(drv_nic_rxhook_init);
EXPORT_SYMBOL(drv_nic_unregister_rxhook);
EXPORT_SYMBOL(drv_nic_rxhook_exit);
#if defined(CONFIG_RTL9607C_SERIES)
EXPORT_SYMBOL(re8686_rx_gmac_set);
EXPORT_SYMBOL(re8686_rx_gmac_get);
#endif
