/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/
#define COMPILE_RTK_L34_FC_MGR_MODULE 1

#include <linux/smp.h>

#include <rtk_fc_mgr.h>
#include <rtk_fc_mgrTRx.h>
#include <rtk_fc_api.h>
#include <rtk_fc_helper_multicast.h>
#include <rtk_fc_assistant.h>
#include <rtk_fc_struct.h>

//#define FC_SAMPLE_CODE

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) && defined(CONFIG_SMP)
static cpumask_t cpumask_gmac0 = {{0}};
static cpumask_t cpumask_gmac1 = {{0}};
static cpumask_t cpumask_11ac = {{0}};
static cpumask_t cpumask_11n = {{0}};
#endif

#if defined(CONFIG_FC_RTL9607C_SERIES)
#if defined(CONFIG_RTK_SOC_RTL8198D)
#define SEL_GMAC_MAX	1000000
#define FORCE_RR_NUM	10000
uint32_t gmac_decision_cnt[2] = {0};
inline void _rtk_fc_add_gmac_sel_cnt(void)
{
	gmac_decision_cnt[1]++;
	if (gmac_decision_cnt[1] >= SEL_GMAC_MAX) {
		gmac_decision_cnt[1] = 0;
		gmac_decision_cnt[0] = 0;
	}
}

inline void _rtk_fc_add_gmac_hash_cnt(void)
{
	gmac_decision_cnt[0]++;
	if (gmac_decision_cnt[0] >= FORCE_RR_NUM) {
		fc_mgr_db.wifi_rx_gmac_auto_sel_en = FALSE;
		rtk_fc_timer_mod_timer(fc_mgr_db.gmac_sel_timer, jiffies + CONFIG_HZ);
	}
}

static void _rtk_fc_gmac_sel_timer(unsigned long data)
{
	fc_mgr_db.wifi_rx_gmac_auto_sel_en = TRUE;
}

void rtk_fc_rx_gmac_sel_reset(void)
{
	int spa = 0;

	for(spa = RTK_FC_MAC_PORT0 ; spa < RTK_FC_MAC_PORT_MAX; spa++) {
		fc_mgr_db.wifi_rx_gmac_sel[spa] = RTK_FC_WIFI_GMAC_TBD;
	}
	fc_mgr_db.wifi_rx_gmac_sel_current = 0;
}

void rtk_fc_rx_gmac_sel_init(void)
{
	fc_mgr_db.gmac_sel_timer = rtk_fc_mgr_timer_list_kmalloc();
	timer_init_timer(fc_mgr_db.gmac_sel_timer, _rtk_fc_gmac_sel_timer);
	rtk_fc_timer_setup_timer(fc_mgr_db.gmac_sel_timer, _rtk_fc_gmac_sel_timer, 0);
	rtk_fc_rx_gmac_sel_reset();
}

void rtk_fc_rx_gmac_sel_set(int port, int mode)
{
	//printk(" => port: %d set select mode to %d\n",port, mode);
	fc_mgr_db.wifi_rx_gmac_sel[port] = mode;
}

void update_gmac_sel_by_port_status(int port, int linkup)
{
	if (linkup) {
		rt_port_speed_t speed;
		rt_port_duplex_t duplex;
		rt_port_speedDuplex_get(port, &speed, &duplex);
		/* Link rate faster than 1Gbps
		 * Set GMAC auto sel to RR
		 */
		if (speed >= RT_PORT_SPEED_10G) {
			rtk_fc_rx_gmac_sel_set(port, RTK_FC_WIFI_GMAC_RR);
			if (fc_db.wanPortMask.portmask & (1 << port)) {
				fc_mgr_db.wifi_rx_gmac_wan_use_rr = 1;
			}
		}
	} else {
		rtk_fc_rx_gmac_sel_set(port, RTK_FC_WIFI_GMAC_TBD);
		if (fc_db.wanPortMask.portmask & (1 << port)) {
			fc_mgr_db.wifi_rx_gmac_wan_use_rr = 0;
		}
	}
}
#endif

uint32 _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash(void)
{
	// round robin algorithm
	static uint32 gmac = 1;

	gmac = (gmac + 1) % GMAC_TRUNKING_NUM;

#if defined(CONFIG_RTK_SOC_RTL8198D)
	_rtk_fc_add_gmac_hash_cnt();
#endif

	return gmac;
}

uint32 _rtk_fc_wlan_rx_lookup_gmac_decision_by_port(uint32 spa)
{
	uint32 gmac = 0;

	if(fc_mgr_db.wifi_rx_gmac_sel[spa] == RTK_FC_WIFI_GMAC_TBD) {
		fc_mgr_db.wifi_rx_gmac_sel[spa] = (fc_mgr_db.wifi_rx_gmac_sel_current++) % GMAC_TRUNKING_NUM;	// 0, 1, (2), 0, 1, (2)...
		fc_mgr_db.wifi_rx_gmac_sel_current = (fc_mgr_db.wifi_rx_gmac_sel_current) % GMAC_TRUNKING_NUM;
		//printk("=====> %s: gmac select num = %d, gmac_sel_current = %d\n", __func__, fc_mgr_db.wifi_rx_gmac_sel[spa],
		//		fc_mgr_db.wifi_rx_gmac_sel_current);
	}
	if(fc_mgr_db.wifi_rx_gmac_sel[spa] == RTK_FC_WIFI_GMAC_RR) {
		gmac = _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash();
#if defined(CONFIG_RTK_SOC_RTL8198D)
		_rtk_fc_add_gmac_hash_cnt();
#endif
	} else {
		gmac = fc_mgr_db.wifi_rx_gmac_sel[spa];
#if defined(CONFIG_RTK_SOC_RTL8198D)
		_rtk_fc_add_gmac_sel_cnt();
#endif
	}
	

	return gmac;
}

uint32 _rtk_fc_wlan_rx_lookup_gmac_decision_by_da(unsigned char *da)
{
	int ret = 0;
	uint32 gmac = 0;
	rtk_fc_mgr_l2_info_t l2info = {0};

	if (da && (da[0] & 0x1)) {
		return 0;
	}

	if((ret = rtk_fc_l2Info_get(da, &l2info)) == SUCCESS) {
		if (l2info.isGMAC==0) {
			// Bridge
			gmac = _rtk_fc_wlan_rx_lookup_gmac_decision_by_port(l2info.spa);
		} else {
			// Routing/NAT
			if (fc_mgr_db.wifi_rx_gmac_wan_use_rr)
				gmac = _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash();
			else
				gmac = _rtk_fc_wlan_rx_lookup_gmac_decision_by_port(l2info.spa);
		}
	}

	return gmac;
}

uint32 _rtk_fc_wlan_rx_lookup_gmac(unsigned char *da)
{
	if(fc_mgr_db.wifi_rx_gmac_auto_sel_en) {
		return _rtk_fc_wlan_rx_lookup_gmac_decision_by_da(da);
	}
	else {
		return _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash();
	}
}
#endif

static void _rtk_fc_rx_final_process(fc_rx_info_t *pRxInfo, struct sk_buff *skb, void *extraInfo);

// wifi rx hardware lookup
static int rtk_fc_wlan_rx_lookup(struct sk_buff *skb, unsigned int wlan_dev_idx)
{
	/*
	 * IPI disabled or IPI execute function
	 */
	int ret = SUCCESS;
	rtk_fc_pmap_t *pPmap = &fc_mgr_db.wlanDevMap[wlan_dev_idx].portmap;
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	{
		fc_tx_info_t txInfo;
		fc_tx_info_t *ptxInfo = &txInfo;

		memset(&txInfo, 0, sizeof(txInfo));

		TXINFO_STAG_AWARE(ptxInfo)=1;
		TXINFO_EXTSPA(ptxInfo) = pPmap->macExtPortIdx;

		if(pPmap->macPortIdx == RTK_FC_MAC_PORT_MAINCPU) {
			//txInfo.tx_gmac_id = 0;
#if defined(CONFIG_FC_RTL9607C_SERIES)
#ifndef CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD
			if(fc_mgr_db.wifi_rx_gmac_auto_sel_en)
				TXINFO_GMAC_ID(ptxInfo) = _rtk_fc_wlan_rx_lookup_gmac_decision_by_da(skb->data);
			else if(fc_mgr_db.wifi_rx_hash_en)
				TXINFO_GMAC_ID(ptxInfo) = _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash();
#endif
#endif
		}
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
		else if(pPmap->macPortIdx == RTK_FC_MAC_PORT_MASTERCPU_CORE1) {
			TXINFO_GMAC_ID(ptxInfo)	 = 1;
		}
		else if(pPmap->macPortIdx == RTK_FC_MAC_PORT_SLAVECPU) {
			TXINFO_GMAC_ID(ptxInfo)	 = 2;
		}
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D)
#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
		if (ntohs(*(unsigned short *)(skb->data + ETH_ALEN + ETH_ALEN)) >= 0x0800)		// not amsdu pkt
#endif
		{	// wlan routing -> nic tx, increase hw counter
			rtk_fc_ext_count_wifi_rx_to_nic_tx_inc(0, skb->dev, skb, skb->data, 0);
		}
#endif

		// smp_nicTx_info.txInfo is ready.
		
#if defined(CONFIG_RTL8686NIC)
#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
		ret = re8686_wifi_hwlookup_deamsdu(skb, ptxInfo, (FC_NIC_TX_PRI_TO_RING >> (TXINFO_CPUTAG_PRI(ptxInfo)<<2)) & 0xf);
#else
		ret = re8686_send_with_txInfo(skb, ptxInfo, (FC_NIC_TX_PRI_TO_RING >> (TXINFO_CPUTAG_PRI(ptxInfo)<<2)) & 0xf);
#endif
#endif
		
		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+TXINFO_GMAC_ID(ptxInfo)].smp_static[smp_processor_id()]);
	}
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	{
	
		ca_ni_tx_config_t tx_config;
		ni_tx_core_config_t *pTx_core_config = &tx_config.core_config;

		memset(&tx_config, 0, sizeof(tx_config));

#if 0	//NIC TX for wifi HW lookup always carry header_a in current configuration
		if(fc_mgr_db.wlanDevMap[wlan_dev_idx].shareCpuPort)
#endif
		{
			pTx_core_config->bf.flow_id_set = TRUE;
			tx_config.flow_id = fc_mgr_db.wlanDevMap[wlan_dev_idx].wlandevidx;
		}
		tx_config.bypass_lso = TRUE;					// bypass dmalso and header_a if possible.

		FCMGR_PRK("wlan use cpu[0x%x] extport[%d] hwlookup flow_id %s [%d]", pPmap->macPortIdx, pPmap->macExtPortIdx, pTx_core_config->bf.flow_id_set?"set":"ignore", tx_config.flow_id);

		pTx_core_config->bf.ldpid = AAL_LPORT_L3_LAN;
		pTx_core_config->bf.lspid = pPmap->macPortIdx;
		pTx_core_config->bf.is_from_ca_tx = TRUE;
		pTx_core_config->bf.bypass_fwd_engine =  (pTx_core_config->bf.flow_id_set ? TRUE : FALSE);
#if defined(CONFIG_FC_RTL9607F_SERIES)
		tx_config.lspid_map = DMALSO_LSPID_MAP_IDX(tx_config.core_config.bf.lspid);
#endif
		FCMGR_PRK("CA NI hwlookup to %s port[%d]", skb->dev?skb->dev->name:"NULL", AAL_LPORT_L3_LAN);

		
		//ca_ni_start_xmit_native(skb, skb->dev, &tx_config);
		ret = nic_egress_start_xmit_for_fc_wifi_hw_lookup(skb, skb->dev, &tx_config);
		
		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC2_TX+RTK_FC_MACPORT_TO_GMAC(tx_config.core_config.bf.lspid)].smp_static[smp_processor_id()]);

	}
#endif

	return ret;
}


#if defined(CONFIG_SMP)

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_RX)

__IRAM_FC_NICTRX
int rtk_fc_smp_nic_rx_dispatch(rtk_fc_smp_nicRx_private_t *smp_nicRx_info)
{
	unsigned int srcSmpId = smp_processor_id();
	int cpu = 0;
	rtk_fc_nicrx_ipi_ctrl_t *smpnicrx = NULL;
	rtk_fc_smp_nicRx_work_info_t *pWorkinfo=NULL;
	fc_rx_info_t *pRxInfo = &smp_nicRx_info->rxInfo;
	bool highpri = (RXINFO_INT_PRI(pRxInfo)>=FC_NIC_RX_PRI_TO_HI_QUEUE) ? TRUE : FALSE;
	uint32 ringMaskSize = highpri ? (MAX_FC_NIC_RX_HIQUEUE_SIZE-1) : (MAX_FC_NIC_RX_QUEUE_SIZE-1);
	int freeidx = 0;
	
	if(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode == RTK_FC_DISPATCH_MODE_RPS)
	{
		
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
		
		u32 hash=0;

#if defined(CONFIG_RTK_APP_PORT_BINDING_SUPPORT)
		smp_nicRx_info->skb->from_dev = smp_nicRx_info->skb->dev; //from dev is removed
#endif			
		rtk_fc_skb_eth_type_trans(smp_nicRx_info->skb, smp_nicRx_info->skb->dev); //important, need skb->protocol before get skb->hash
	
		skb_reset_network_header(smp_nicRx_info->skb);

		hash = skb_get_hash(smp_nicRx_info->skb);
	
		cpu = fc_mgr_db.fc_rps_maps.cpus[reciprocal_scale(hash, fc_mgr_db.fc_rps_maps.len)];
	
		FCMGR_PRK("[FC RPS]Use cpu = %d, hash = %d\n", cpu, hash);
		/*
			In eth_type_trans(), skb->data's position will be start from iphdr, instead of from eth hdr.
			So we need to reset,
		*/
		
		smp_nicRx_info->skb->data -= (ETH_HLEN);
		smp_nicRx_info->skb->len += (ETH_HLEN);

		//cpu = rtk_fc_smp_nic_rx_rps_get_cpu(smp_nicRx_info->skb, &smp_nicRx_info->rxInfo);
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(likely(RXINFO_HASH_CRC16(pRxInfo)))
		{
			cpu =  fc_mgr_db.fc_rps_maps.cpus[( (RXINFO_HASH_CRC16(pRxInfo)) & 0xf)];
			if(cpu < 0 || cpu >= NR_CPUS)
			{	
				printk("[WARNING]Wrong cpu = %d!! use default cpu 2\n",cpu);
				cpu = 2;
			}
		}
		else
		{
			cpu = 2;
			FCMGR_PRK("Get Hash Failed! Use ipi default cpu.\n");
		}
#elif defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if(likely(RXINFO_FBI(pRxInfo))){
	
			cpu =  fc_mgr_db.fc_rps_maps.cpus[( (RXINFO_FB_HASH(pRxInfo)) & 0xf)];
			//FCMGR_PRK("[FC RPS]Use cpu = %d, hash = %d\n", cpu, hash);
		}
		else
		{
			if(highpri)
				cpu = fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_HIGHPRI_NIC_RX].smp_id;	
			else
				cpu = fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].smp_id;
			FCMGR_PRK("Get Hash Failed! Use ipi default cpu.\n");
		}
#endif
		

		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_RPS_CPU_DISTRIBUTE].smp_static[cpu]);
	}
	else if(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode == RTK_FC_DISPATCH_MODE_SMP_BY_PORT)
	{		
		if(RXINFO_SPA(pRxInfo)<RTK_FC_MAC_PORT_MAX && fc_mgr_db.fc_rps_maps.port_to_cpu[RXINFO_SPA(pRxInfo)] != -1 )
			cpu = fc_mgr_db.fc_rps_maps.port_to_cpu[RXINFO_SPA(pRxInfo)];
		else
			cpu = RTK_FC_DEFAULT_DISPATCH_CPU;

		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_RPS_CPU_DISTRIBUTE].smp_static[cpu]);
	}
	else {	
		if(highpri)
			cpu = fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_HIGHPRI_NIC_RX].smp_id;	
		else
			cpu = fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].smp_id;
	}

	if(unlikely(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode == RTK_FC_DISPATCH_MODE_RPS || 
				fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode == RTK_FC_DISPATCH_MODE_SMP_BY_PORT)
	)
	{
		smpnicrx = fc_mgr_db.nicrx_ipi[cpu];
		
		if(highpri)ringMaskSize = (MAX_FC_NIC_RX_QUEUE_SIZE-1);
	}
	else 
	{
		if(highpri)
			smpnicrx = fc_mgr_db.nicrx_hi_ipi[srcSmpId];
		else 
			smpnicrx = fc_mgr_db.nicrx_ipi[srcSmpId];
	}
	
	//FCMGR_PRK("buf state[%d] put pkt to %s queue freeidx[%d] target cpu[%d]", bufstate, highpri?"high":"low", freeidx, cpu);

	freeidx = atomic_read(&smpnicrx->priv_free_idx);
	pWorkinfo = &smpnicrx->priv_work[freeidx];
	prefetch(pWorkinfo);

	if(unlikely(((freeidx+MAX_FC_NIC_RX_QUEUE_SIZE-atomic_read(&smpnicrx->priv_sched_idx)) & ringMaskSize) 
		>= (ringMaskSize-FC_IPI_QUEUE_RSVED_CNT)) )		
	{
		if(atomic_read(&smpnicrx->csd_available)==IPI_SCHED) {
			//no free rx_works, lots of skb are waiting to process
			dev_kfree_skb_any(smp_nicRx_info->skb);
			if(unlikely(fc_mgr_db.smpStatistic))
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_IPI_TO_FC_RX_DROP].smp_static[smp_processor_id()]);
			
			return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
		}
	}
	

//2ENQUEUE:
	//Setup information
	memcpy(&(pWorkinfo->smp_nicRx_info), smp_nicRx_info, sizeof(pWorkinfo->smp_nicRx_info));
	
	++freeidx;
	smp_mb(); 
	freeidx &= ringMaskSize;
	smp_mb(); 
	atomic_set(&smpnicrx->priv_free_idx, freeidx);		
	
//2DISPATCH:

	if(atomic_read(&smpnicrx->csd_available)==IPI_IDLE) 
	{
		atomic_set(&smpnicrx->csd_available, IPI_SCHED);
		
		//FCMGR_PRK("dispatch: buf state[%d] freeidx[%d] target cpu[%d]", bufstate, freeidx, cpu);
	
		if(rtk_fc_g_smp_call_function_single_async(cpu, &smpnicrx->csd) != SUCCESS) {
			FCMGR_PRK("smp call function not ready");
		}
	}
	
	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
}

__IRAM_FC_SHORTCUT
void rtk_fc_smp_nic_rx_tasklet(void *info)
{
	rtk_fc_nicrx_ipi_ctrl_t *smpnicrx = info;
	tasklet_hi_schedule(&smpnicrx->tasklet);
}



__IRAM_FC_SHORTCUT
static void rtk_fc_smp_nic_rx_processone(rtk_fc_smp_nicRx_work_info_t *work)
{
	fc_rx_info_t *pRxInfo;
	struct sk_buff *skb;
	void *extraInfo;

	skb=work->smp_nicRx_info.skb;
	prefetch(skb);
	pRxInfo = &work->smp_nicRx_info.rxInfo;
	extraInfo = work->smp_nicRx_info.extraInfo;

	if(unlikely(fc_mgr_db.smpStatistic))
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_IPI_TO_FC_RX].smp_static[smp_processor_id()]);
	
	_rtk_fc_rx_final_process(pRxInfo, skb, extraInfo);
	
	return;
}

// call_single_data function
__IRAM_FC_SHORTCUT
void rtk_fc_smp_nic_rx_exec(unsigned long data)
{
	// de-queue
	rtk_fc_nicrx_ipi_ctrl_t *smpnicrx = (rtk_fc_nicrx_ipi_ctrl_t *)data;
	unsigned int maxcnt=MAX_FC_NIC_RX_QUEUE_SIZE<<1, cnt, __cnt;
	rtk_fc_smp_nicRx_work_info_t *pWorkinfo=NULL;
	unsigned long int break_jiffies=jiffies+(CONFIG_HZ<<1);
	bool timeout = FALSE;
	int freeidx, scheduleidx;

	local_bh_disable();
	
	scheduleidx = atomic_read(&smpnicrx->priv_sched_idx);

	do{	
		
		freeidx = atomic_read(&smpnicrx->priv_free_idx);

		if(scheduleidx == freeidx) {
			__cnt = cnt =0;
			break;
		}else if(freeidx> scheduleidx)
			__cnt = cnt = freeidx - scheduleidx;
		else
			__cnt = cnt = (freeidx + smpnicrx->priv_work_cnt) - scheduleidx;
	
		//FCMGR_PRK("exec: buf state[%d] scheduleidx[%d] work state %d", bufstate, scheduleidx, atomic_read(&pOldestOne->state));

		do{	
			pWorkinfo = &smpnicrx->priv_work[scheduleidx];	
			prefetch(pWorkinfo);
			
			/*
			 * fc ingress process
			 */
			rtk_fc_smp_nic_rx_processone(pWorkinfo);

			--cnt;
			--maxcnt;
			scheduleidx += 1;
			smp_mb(); 
			// get next one
			scheduleidx &= (smpnicrx->priv_work_cnt-1);		
			smp_mb(); 
			atomic_set(&smpnicrx->priv_sched_idx, scheduleidx);
		
			if(unlikely(time_is_before_jiffies(break_jiffies))) {
				timeout = TRUE;
			}	
			
		}while(!timeout && (cnt!=0) && (maxcnt!=0));

	} while(!timeout && (maxcnt!=0));

	
	if(timeout || (maxcnt==0) || (__cnt!=0))
	{	
		smpnicrx->empty_check = IPI_EMPTY_CHK_NONE;
		rtk_fc_smp_nic_rx_tasklet(smpnicrx);
	}else {
		if(smpnicrx->empty_check == IPI_EMPTY_CHK_NONE) {
			atomic_set(&smpnicrx->csd_available, IPI_IDLE);// to allow smp_call_function
			smpnicrx->empty_check = IPI_EMPTY_CHK_EXEC;
			tasklet_hi_schedule(&smpnicrx->tasklet_empty_check);
		}else {		
			smpnicrx->empty_check = IPI_EMPTY_CHK_NONE;
		}
	}

	local_bh_enable();

	return;
}
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_TX)
__IRAM_FC_SHORTCUT
int rtk_fc_smp_nic_tx_dispatch(rtk_fc_smp_nicTx_private_t *smp_nicTx_info)
{
	unsigned int srcSmpId = smp_processor_id();
	rtk_fc_smp_nicTx_work_t *pFirstOne=NULL;
	int freeidx = 0;
	
	freeidx = atomic_read(&fc_mgr_db.nictx_free_idx[srcSmpId]);

	if(unlikely(((freeidx+MAX_FC_NIC_TX_QUEUE_SIZE-atomic_read(&fc_mgr_db.nictx_sched_idx[srcSmpId])) & (MAX_FC_NIC_TX_QUEUE_SIZE-1)) 
		>= (MAX_FC_NIC_TX_QUEUE_SIZE-FC_IPI_QUEUE_RSVED_CNT)) )
	{
		if((atomic_read(&fc_mgr_db.nictx_ipi[srcSmpId]->csd_available)==IPI_SCHED)) {
			//no free rx_works, lots of skb are waiting to process
			dev_kfree_skb_any(smp_nicTx_info->skb);
			if(unlikely(fc_mgr_db.smpStatistic))
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_FC_IPI_TO_NIC_TX_DROP].smp_static[smp_processor_id()]);
			
			return SUCCESS;
		}
		
	}

	pFirstOne = &fc_mgr_db.nictx_work[srcSmpId][freeidx];

//2ENQUEUE:	
	//Setup information
	memcpy(&pFirstOne->smp_nicTx_info, smp_nicTx_info, sizeof(pFirstOne->smp_nicTx_info));
	
	freeidx += 1;
	smp_mb(); 
	freeidx &= (MAX_FC_NIC_TX_QUEUE_SIZE-1);
	smp_mb(); 
	
	atomic_set(&fc_mgr_db.nictx_free_idx[srcSmpId], freeidx);

//2DISPATCH:
	if(atomic_read(&fc_mgr_db.nictx_ipi[srcSmpId]->csd_available)==IPI_IDLE)
	{
		atomic_set(&fc_mgr_db.nictx_ipi[srcSmpId]->csd_available, IPI_SCHED);

		if(rtk_fc_g_smp_call_function_single_async(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_TX].smp_id, &fc_mgr_db.nictx_ipi[srcSmpId]->csd) != SUCCESS) {
			FCMGR_PRK("smp call function not ready");
		}
	}

	return SUCCESS;
}

__IRAM_FC_NICTRX
void rtk_fc_smp_nic_tx_tasklet(void *info)
{
	rtk_fc_smp_ipi_ctrl_t *smpctrl = info;
	tasklet_hi_schedule(&smpctrl->tasklet);
}

// call_single_data function
__IRAM_FC_NICTRX
void rtk_fc_smp_nic_tx_exec(unsigned long data)
{
	// de-queue
	int ret = SUCCESS;
	unsigned int srcSmpId = data;
	unsigned int maxcnt = MAX_FC_NIC_TX_QUEUE_SIZE, cnt, __cnt;
	rtk_fc_smp_nicTx_work_t *pOldestOne = NULL;
	unsigned long int break_jiffies=jiffies+(CONFIG_HZ<<1);
	bool timeout = FALSE;
	int freeidx, scheduleidx;
	
	scheduleidx = atomic_read(&fc_mgr_db.nictx_sched_idx[srcSmpId]);

	do{
		freeidx = atomic_read(&fc_mgr_db.nictx_free_idx[srcSmpId]);
		if(scheduleidx == freeidx) {
			__cnt = cnt =0;
			break;
		}else if(freeidx> scheduleidx)
			__cnt = cnt = freeidx - scheduleidx;
		else
			__cnt = cnt = (freeidx + MAX_FC_NIC_TX_QUEUE_SIZE) - scheduleidx;
		
		do{	
			pOldestOne = &fc_mgr_db.nictx_work[srcSmpId][scheduleidx];
			
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
			{
				fc_tx_info_t *ptxInfo;
				ptxInfo = &(pOldestOne->smp_nicTx_info.txInfo);

				ret = re8686_send_with_txInfo(pOldestOne->smp_nicTx_info.skb, ptxInfo, (FC_NIC_TX_PRI_TO_RING >> (TXINFO_CPUTAG_PRI(ptxInfo)<<2)) & 0xf);
				if(unlikely(fc_mgr_db.smpStatistic))
					atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+TXINFO_GMAC_ID(ptxInfo)].smp_static[smp_processor_id()]);
			}
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
			{
				ca_ni_tx_config_t tx_config;

				memset(&tx_config, 0, sizeof(tx_config));
				tx_config.flow_id = pOldestOne->smp_nicTx_info.flow_id;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				tx_config.stream_id_en = pOldestOne->smp_nicTx_info.stream_id_en;
				tx_config.stream_id = pOldestOne->smp_nicTx_info.stream_id;
				if(unlikely(pOldestOne->smp_nicTx_info.force_def_tx_en))
					tx_config.force_def_tx_en = pOldestOne->smp_nicTx_info.force_def_tx_en;
				if(unlikely(pOldestOne->smp_nicTx_info.chk_sel_en)){
#if defined(CONFIG_FC_RTL8277C_SERIES)
					tx_config.chk_sel_en = pOldestOne->smp_nicTx_info.chk_sel_en;
					tx_config.chk_sel = pOldestOne->smp_nicTx_info.chk_sel;
#endif
				}
#endif
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				tx_config.dma_aft_l2fib_enable = pOldestOne->smp_nicTx_info.lso_para0.bf.dma_aft_l2fib_enable;
				tx_config.dma_aft_l2fib_index = pOldestOne->smp_nicTx_info.lso_para0.bf.dma_aft_l2fib_index;
#if defined(CONFIG_FC_RTL9607F_SERIES)
				tx_config.dmaaft_map = pOldestOne->smp_nicTx_info.dmaaft_map;
#endif
#endif
				memcpy(&(tx_config.core_config), &(pOldestOne->smp_nicTx_info.core_config), sizeof(tx_config.core_config));
				tx_config.lso_para0.wrd = pOldestOne->smp_nicTx_info.lso_para0.wrd;


				//ca_ni_start_xmit_native(pOldestOne->smp_nicTx_info.skb, pOldestOne->smp_nicTx_info.dev, &tx_config);
				ret = nic_egress_start_xmit_for_fc_dirTx(pOldestOne->smp_nicTx_info.skb, pOldestOne->smp_nicTx_info.skb->dev, &tx_config);
				if(unlikely(fc_mgr_db.smpStatistic))
					atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+(tx_config.core_config.bf.lspid-RTK_FC_MAC_PORT_CPU)].smp_static[smp_processor_id()]);
			}
#endif


			--cnt;
			--maxcnt;
			scheduleidx += 1;
			smp_mb(); 
			// get next one
			scheduleidx &= (MAX_FC_NIC_TX_QUEUE_SIZE-1);		
			smp_mb(); 
			atomic_set(&fc_mgr_db.nictx_sched_idx[srcSmpId], scheduleidx);
		
			if(unlikely(time_is_before_jiffies(break_jiffies))) {
				timeout = TRUE;
			}	
			
		}while(!timeout && (cnt!=0) && (maxcnt!=0));

	} while(!timeout && (maxcnt!=0));

	if(timeout || (maxcnt==0) || __cnt!=0) {
		fc_mgr_db.nictx_ipi[srcSmpId]->empty_check = IPI_EMPTY_CHK_NONE;
		rtk_fc_smp_nic_tx_tasklet(fc_mgr_db.nictx_ipi[srcSmpId]);
	}else {
		if(fc_mgr_db.nictx_ipi[srcSmpId]->empty_check == IPI_EMPTY_CHK_NONE) {		// no pkt, do empty check and set idle state
			atomic_set(&fc_mgr_db.nictx_ipi[srcSmpId]->csd_available, IPI_IDLE);		// to allow smp_call_function
			fc_mgr_db.nictx_ipi[srcSmpId]->empty_check = IPI_EMPTY_CHK_EXEC;
			tasklet_hi_schedule(&fc_mgr_db.nictx_ipi[srcSmpId]->tasklet_empty_check);
		}else {																	// no pkt and finish empty check, return
			fc_mgr_db.nictx_ipi[srcSmpId]->empty_check = IPI_EMPTY_CHK_NONE;
		}
	}

	return;

}

#endif

#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_RX)
int rtk_fc_smp_wlan_rx_dispatch(rtk_fc_wlanrx_info_t *pWlanRxInfo)
{
	unsigned int wlanRxSrcSmpId = smp_processor_id();
	unsigned int wlanband = fc_mgr_db.wlanDevMap[pWlanRxInfo->wlandevidx].band;
	rtk_fc_smp_wlanRx_work_t *pFirstOne=NULL;
	int freeidx = 0;
	
	freeidx = atomic_read(&fc_mgr_db.wlanrx_free_idx[wlanRxSrcSmpId]);
	
	pFirstOne = &fc_mgr_db.wlanrx_work[wlanRxSrcSmpId][freeidx];

	if(unlikely(((freeidx+MAX_FC_WLAN_RX_QUEUE_SIZE-atomic_read(&fc_mgr_db.wlanrx_sched_idx[wlanRxSrcSmpId])) & (MAX_FC_WLAN_RX_QUEUE_SIZE-1)) 
		>= (MAX_FC_WLAN_RX_QUEUE_SIZE-FC_IPI_QUEUE_RSVED_CNT)) )
	{
		if(atomic_read(&fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->csd_available)==IPI_SCHED) {
			//no free rx_works, lots of skb are waiting to process
			dev_kfree_skb_any(pWlanRxInfo->skb);
			if(unlikely(fc_mgr_db.smpStatistic))
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_IPI_BAND0_HW_LOOKUP_DROP+wlanband].smp_static[smp_processor_id()]);
			return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
		}
	}

//2ENQUEUE:	
	//Setup information
	memcpy(&pFirstOne->smp_wlanRx_info, pWlanRxInfo, sizeof(pFirstOne->smp_wlanRx_info));
	
	freeidx += 1;
	freeidx &= (MAX_FC_WLAN_RX_QUEUE_SIZE-1);
	smp_mb(); 
	
	atomic_set(&fc_mgr_db.wlanrx_free_idx[wlanRxSrcSmpId], freeidx);

//2 DISPATCH:
	if(atomic_read(&fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->csd_available)==IPI_IDLE)
	{
		atomic_set(&fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->csd_available, IPI_SCHED);
		if(rtk_fc_g_smp_call_function_single_async(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_WLAN0_RX+wlanband].smp_id, &fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->csd) != SUCCESS) {
			FCMGR_PRK("smp call function not ready");
		}
	}

	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
}

void rtk_fc_smp_wlan_rx_tasklet(void *info)
{
	rtk_fc_smp_ipi_ctrl_t *smpctrl = info;
	tasklet_hi_schedule(&smpctrl->tasklet);
}

// call_single_data function
void rtk_fc_smp_wlan_rx_exec(unsigned long data)
{
	// de-queue
	int ret = SUCCESS;
	unsigned int wlanRxSrcSmpId = data;
	unsigned int maxcnt=MAX_FC_WLAN_RX_QUEUE_SIZE, cnt, __cnt;
	rtk_fc_smp_wlanRx_work_t *pOldestOne = NULL;
	unsigned long int break_jiffies=jiffies+(CONFIG_HZ<<1);
	int freeidx, scheduleidx;
	bool timeout = FALSE;


	local_bh_disable();
	
	scheduleidx = atomic_read(&fc_mgr_db.wlanrx_sched_idx[wlanRxSrcSmpId]);

	do{
		freeidx = atomic_read(&fc_mgr_db.wlanrx_free_idx[wlanRxSrcSmpId]);
		if(scheduleidx == freeidx) {
			__cnt = cnt =0;
			break;
		}else if(freeidx> scheduleidx)
			__cnt = cnt = freeidx - scheduleidx;
		else
			__cnt = cnt = (freeidx + MAX_FC_WLAN_RX_QUEUE_SIZE) - scheduleidx;
		
		do{	
			pOldestOne = &fc_mgr_db.wlanrx_work[wlanRxSrcSmpId][scheduleidx];
			
			ret = rtk_fc_wlan_rx_lookup(pOldestOne->smp_wlanRx_info.skb, pOldestOne->smp_wlanRx_info.wlandevidx);

			--cnt;
			--maxcnt;
			scheduleidx += 1;
			smp_mb(); 
			// get next one
			scheduleidx &= (MAX_FC_WLAN_RX_QUEUE_SIZE-1);		
			smp_mb(); 
			atomic_set(&fc_mgr_db.wlanrx_sched_idx[wlanRxSrcSmpId], scheduleidx);

			if(unlikely(time_is_before_jiffies(break_jiffies))) {
				timeout = TRUE;
			}	
			
		}while(!timeout && (cnt!=0) && (maxcnt!=0));

	} while(!timeout && (maxcnt!=0));

	local_bh_enable();

#if 0
	if(timeout || (maxcnt==0) || (__cnt!=0)) {
		fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->empty_check = IPI_EMPTY_CHK_NONE;
		rtk_fc_smp_wlan_rx_tasklet(fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]);
	}else {
		if(fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->empty_check == IPI_EMPTY_CHK_NONE) {
			atomic_set(&fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->csd_available, IPI_IDLE);// to allow smp_call_function
			fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->empty_check = IPI_EMPTY_CHK_EXEC;
			tasklet_hi_schedule(&fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->tasklet_empty_check);
		}else {
			fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->empty_check = IPI_EMPTY_CHK_NONE;
		}
	}
#else
	if(timeout || (maxcnt==0))	
		rtk_fc_smp_wlan_rx_tasklet(fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]);
	else	
		atomic_set(&fc_mgr_db.wlanrx_ipi[wlanRxSrcSmpId]->csd_available, IPI_IDLE);// to allow smp_call_function
#endif
	
	return;

}

#endif

#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_TX)
__IRAM_FC_NICTRX
int rtk_fc_smp_wlan_tx_dispatch(rtk_fc_wlantx_info_t *pWlanTxInfo)
{
	unsigned int wlanTxSrcSmpId = smp_processor_id();
	unsigned int wlanband = fc_mgr_db.wlanDevMap[pWlanTxInfo->wlandevidx].band;
	rtk_fc_smp_wlanTx_work_t *pFirstOne=NULL;
	int freeidx = 0;

	freeidx = atomic_read(&fc_mgr_db.wlantx_free_idx[wlanTxSrcSmpId]);
	
	if(unlikely(((freeidx+MAX_FC_WLAN_TX_QUEUE_SIZE-atomic_read(&fc_mgr_db.wlantx_sched_idx[wlanTxSrcSmpId])) & (MAX_FC_WLAN_TX_QUEUE_SIZE-1)) 
		>= (MAX_FC_WLAN_TX_QUEUE_SIZE-FC_IPI_QUEUE_RSVED_CNT)) )
	{
		if(atomic_read(&fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->csd_available)==IPI_SCHED) {
			//no free tx_works, lots of skb are waiting to process
			dev_kfree_skb_any(pWlanTxInfo->skb);
			if(unlikely(fc_mgr_db.smpStatistic))
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_IPI_TO_WIFI_BAND0_TX_DROP+wlanband].smp_static[smp_processor_id()]);
			
			return RTK_FC_DEVTX_OK;
		}
	}
	
	pFirstOne = &fc_mgr_db.wlantx_work[wlanTxSrcSmpId][freeidx];

//2ENQUEUE:	
	//Setup information
	memcpy(&pFirstOne->smp_wlanTx_info, pWlanTxInfo, sizeof(pFirstOne->smp_wlanTx_info));

	freeidx += 1;
	smp_mb();
	freeidx &= (MAX_FC_WLAN_TX_QUEUE_SIZE-1);
	smp_mb();
	atomic_set(&fc_mgr_db.wlantx_free_idx[wlanTxSrcSmpId], freeidx);

//2DISPATCH:
	if(atomic_read(&fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->csd_available)==IPI_IDLE)
	{
		atomic_set(&fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->csd_available, IPI_SCHED);
		
		if(rtk_fc_g_smp_call_function_single_async(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_WLAN0_TX+wlanband].smp_id, &fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->csd) != SUCCESS) {
			FCMGR_PRK("smp call function not ready");
		}
	}

	return RTK_FC_DEVTX_OK;
}

void rtk_fc_smp_wlan_tx_tasklet(void *info)
{
	rtk_fc_smp_ipi_ctrl_t *smpctrl = info;
	tasklet_hi_schedule(&smpctrl->tasklet);
}

// call_single_data function
void rtk_fc_smp_wlan_tx_exec(unsigned long data)
{
	// de-queue
	int ret = SUCCESS;
	unsigned int maxcnt=MAX_FC_WLAN_TX_QUEUE_SIZE, cnt, __cnt;
	rtk_fc_smp_wlanTx_work_t *pOldestOne = NULL;
	unsigned long int break_jiffies=jiffies+(CONFIG_HZ<<1);
	bool timeout = FALSE;
	unsigned int wlanTxSrcSmpId = data;
	int freeidx, scheduleidx;
	struct sk_buff *skb;
	rtk_fc_wlan_devidx_t wlan_dev_idx;
	struct net_device	*dev;
	unsigned int wlanband;
	bool if_wifiTxQ_stpopped = FALSE;

	scheduleidx = atomic_read(&fc_mgr_db.wlantx_sched_idx[wlanTxSrcSmpId]);


	local_bh_disable();
	
	do{
		freeidx = atomic_read(&fc_mgr_db.wlantx_free_idx[wlanTxSrcSmpId]);
		if(scheduleidx == freeidx) {
			__cnt = cnt =0;
			break;
		}else if(freeidx> scheduleidx)
			__cnt = cnt = freeidx - scheduleidx;
		else
			__cnt = cnt = (freeidx + MAX_FC_WLAN_TX_QUEUE_SIZE) - scheduleidx;
		
		do{	
			pOldestOne = &fc_mgr_db.wlantx_work[wlanTxSrcSmpId][scheduleidx];
					
			skb=pOldestOne->smp_wlanTx_info.skb;
			wlan_dev_idx = pOldestOne->smp_wlanTx_info.wlandevidx;
			dev = skb->dev;

			if(unlikely((fc_mgr_db.wlanDevMap[wlan_dev_idx].valid == FALSE) || !dev || (dev && !netif_running(dev)) )) {
				dev_kfree_skb_any(skb);	
				ret = RTK_FC_DEVTX_BUSY;
			}else{

#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
				{
					rtl_recycle_skb_debug_t rtl_skb_debug_parameter;
					rtl_skb_debug_parameter.cb=fc_mgr_db.rtl_fc_skb_cb_value;
					rtl_skb_debug_parameter.check_skb=skb;
					rtl_skb_debug_parameter.skb_pointer_off=-1;
					rtl_skb_debug_parameter.headroom=-1;
					rtl_skb_debug_parameter.dump_stack_en=0;
					rtl_recycle_skb_cb_section_enter(&rtl_skb_debug_parameter);
					rtl_recycle_skb_cb_section_leave(&rtl_skb_debug_parameter);
				}
#endif

				//rtl8192cd_start_xmit(skb,dev);
				ret = fc_mgr_db.wlanDevMap[wlan_dev_idx].wlan_native_devops->ndo_start_xmit(skb, dev);
				
				if(netif_queue_stopped(dev))
					if_wifiTxQ_stpopped = TRUE; //wifi TX queue is stopped

#if defined(CONFIG_FC_QTNA_WIFI_AX)					
				if(ret != RTK_FC_DEVTX_OK)
					dev_kfree_skb_any(skb); //free skb here
#endif

			}

			--cnt;
			--maxcnt;
			scheduleidx += 1;
			smp_mb(); 
			// get next one
			scheduleidx &= (MAX_FC_WLAN_TX_QUEUE_SIZE-1);		
			smp_mb(); 
			atomic_set(&fc_mgr_db.wlantx_sched_idx[wlanTxSrcSmpId], scheduleidx);

			if(unlikely(fc_mgr_db.smpStatistic))
			{		
				wlanband = fc_mgr_db.wlanDevMap[wlan_dev_idx].band;
				
				if(ret==RTK_FC_DEVTX_OK)
					atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BAND0_TX+wlanband].smp_static[smp_processor_id()]);
				else
					atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BAND0_TX_DROP+wlanband].smp_static[smp_processor_id()]);
				if(if_wifiTxQ_stpopped)
					atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BAND0_TXQ_STOPPED_TIMES+wlanband].smp_static[smp_processor_id()]);
			}
			
			if(unlikely(time_is_before_jiffies(break_jiffies))) {
				timeout = TRUE;
			}	
			
		}while(!timeout && (cnt!=0) && (maxcnt!=0) && (ret==RTK_FC_DEVTX_OK) && (!if_wifiTxQ_stpopped));


	}while(!timeout && (maxcnt!=0) && (ret==RTK_FC_DEVTX_OK) && (!if_wifiTxQ_stpopped));

	local_bh_enable();

#if 0	
	if(timeout || (maxcnt==0) || (__cnt!=0)) {
		fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->empty_check = IPI_EMPTY_CHK_NONE;
		rtk_fc_smp_wlan_tx_tasklet(fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]);
	}else {
		if(fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->empty_check == IPI_EMPTY_CHK_NONE) {
			atomic_set(&fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->csd_available, IPI_IDLE);// to allow smp_call_function
			fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->empty_check = IPI_EMPTY_CHK_EXEC;
			tasklet_hi_schedule(&fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->tasklet_empty_check);
		}else {
			fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->empty_check = IPI_EMPTY_CHK_NONE;
		}
	}
#else
	if(timeout || (maxcnt==0))	
		rtk_fc_smp_wlan_tx_tasklet(fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]);
	else	
		atomic_set(&fc_mgr_db.wlantx_ipi[wlanTxSrcSmpId]->csd_available, IPI_IDLE);// to allow smp_call_function
#endif

	return;
}
#endif



__IRAM_FC_NICTRX
int rtk_fc_smp_ps_netif_rx_dispatch(rtk_fc_ps_netif_rx_info_t *pNetif_rxInfo)
{
	unsigned int ps_netif_rxSrcSmpId = smp_processor_id();
	rtk_fc_smp_ps_netif_rx_work_t *pFirstOne=NULL;
	int freeidx = 0;

	freeidx = atomic_read(&fc_mgr_db.ps_netif_rx_free_idx[ps_netif_rxSrcSmpId]);
	
	if(unlikely(((freeidx+MAX_FC_PS_NETIF_RX_QUEUE_SIZE-atomic_read(&fc_mgr_db.ps_netif_rx_sched_idx[ps_netif_rxSrcSmpId])) & (MAX_FC_PS_NETIF_RX_QUEUE_SIZE-1)) 
		>= (MAX_FC_PS_NETIF_RX_QUEUE_SIZE-FC_IPI_QUEUE_RSVED_CNT)) )
	{
		if(atomic_read(&fc_mgr_db.ps_netif_rx_ipi[ps_netif_rxSrcSmpId]->csd_available)==IPI_SCHED) {
			//no free tx_works, lots of skb are waiting to process
			dev_kfree_skb_any(pNetif_rxInfo->skb);
			if(unlikely(fc_mgr_db.smpStatistic))
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_PS_NETIF_RX_DROP].smp_static[smp_processor_id()]);
			
			return RTK_FC_DEVTX_OK;
		}
	}

		
	pFirstOne = &fc_mgr_db.ps_netif_rx_work[ps_netif_rxSrcSmpId][freeidx];

//2ENQUEUE:	
	//Setup information
	memcpy(&pFirstOne->smp_ps_netif_rx_info, pNetif_rxInfo, sizeof(pFirstOne->smp_ps_netif_rx_info));

	freeidx += 1;
	smp_mb();
	freeidx &= (MAX_FC_PS_NETIF_RX_QUEUE_SIZE-1);
	smp_mb();
	atomic_set(&fc_mgr_db.ps_netif_rx_free_idx[ps_netif_rxSrcSmpId], freeidx);

//2DISPATCH:
	if(atomic_read(&fc_mgr_db.ps_netif_rx_ipi[ps_netif_rxSrcSmpId]->csd_available)==IPI_IDLE)
	{
		atomic_set(&fc_mgr_db.ps_netif_rx_ipi[ps_netif_rxSrcSmpId]->csd_available, IPI_SCHED);
		if(rtk_fc_g_smp_call_function_single_async(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_PS_NETIF_RX].smp_id, &fc_mgr_db.ps_netif_rx_ipi[ps_netif_rxSrcSmpId]->csd) != SUCCESS) {
			FCMGR_PRK("smp call function not ready");
		}
	}

	return RTK_FC_DEVTX_OK;
}

void rtk_fc_smp_ps_netif_rx_tasklet(void *info)
{
	rtk_fc_smp_ipi_ctrl_t *smpctrl = info;
	tasklet_hi_schedule(&smpctrl->tasklet);
}

int rtk_fc_netif_rx_with_hook_handler(struct sk_buff *skb)
{

	int ret=RTK_FC_NIC_RX_CONTINUE;

	if( fc_mgr_db._rtk_fc_post_nic_skb_rx)	
	{
		fc_rx_info_t rxInfo;		
		memset(&rxInfo,0,sizeof(rxInfo));
		if(skb->fcIngressData.ingressPort==RTK_FC_MAC_PORT_PON)
		{
			rxInfo.opts3.bit.pon_sid_or_extspa = skb->fcIngressData.ponStreamId;
		}
		rxInfo.opts3.bit.src_port_num = skb->fcIngressData.ingressPort;
		rxInfo.opts3.bit.internal_priority = skb->priority;
		ret = fc_mgr_db._rtk_fc_post_nic_skb_rx(skb, &rxInfo);		
	}
	
	rtk_fc_skb_rxhook_skb_handling(skb, ret);

	return SUCCESS;

}


// call_single_data function
void rtk_fc_smp_ps_netif_rx_exec(unsigned long data)
{
	// de-queue
	int ret = SUCCESS;
	unsigned int maxcnt=MAX_FC_PS_NETIF_RX_QUEUE_SIZE, cnt;
	rtk_fc_smp_ps_netif_rx_work_t *pOldestOne = NULL;
	unsigned long int break_jiffies=jiffies+(CONFIG_HZ<<1);
	bool timeout = FALSE;
	unsigned int ps_netif_rxSrcSmpId = data;
	int freeidx, scheduleidx;
	struct sk_buff *skb;
	struct net_device	*dev;

	scheduleidx = atomic_read(&fc_mgr_db.ps_netif_rx_sched_idx[ps_netif_rxSrcSmpId]);

	local_bh_disable();
	
	do{
		freeidx = atomic_read(&fc_mgr_db.ps_netif_rx_free_idx[ps_netif_rxSrcSmpId]);
		if(scheduleidx == freeidx) {
			cnt =0;
			break;
		}else if(freeidx> scheduleidx)
			cnt = freeidx - scheduleidx;
		else
			cnt = (freeidx + MAX_FC_PS_NETIF_RX_QUEUE_SIZE) - scheduleidx;
		
		do{	
			pOldestOne = &fc_mgr_db.ps_netif_rx_work[ps_netif_rxSrcSmpId][scheduleidx];
					
			skb=pOldestOne->smp_ps_netif_rx_info.skb;
			dev = skb->dev;

			if(unlikely( !dev || (dev && !netif_running(dev)) )) {
				dev_kfree_skb_any(skb);	
				ret = RTK_FC_DEVTX_BUSY;
			}else{
				rtk_fc_netif_rx_with_hook_handler(skb);
			}

			--cnt;
			--maxcnt;
			scheduleidx += 1;
			smp_mb(); 
			// get next one
			scheduleidx &= (MAX_FC_PS_NETIF_RX_QUEUE_SIZE-1);		
			smp_mb(); 
			atomic_set(&fc_mgr_db.ps_netif_rx_sched_idx[ps_netif_rxSrcSmpId], scheduleidx);

			if(unlikely(fc_mgr_db.smpStatistic))
			{		
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_PS_NETIF_RX].smp_static[smp_processor_id()]);
			}
			
			if(unlikely(time_is_before_jiffies(break_jiffies))) {
				timeout = TRUE;
			}	
			
		}while(!timeout && (cnt!=0) && (maxcnt!=0) );


	}while(!timeout && (maxcnt!=0) );

	local_bh_enable();
	
	if(timeout || (maxcnt==0))
		rtk_fc_smp_ps_netif_rx_tasklet(fc_mgr_db.ps_netif_rx_ipi[ps_netif_rxSrcSmpId]);
	else
		atomic_set(&fc_mgr_db.ps_netif_rx_ipi[ps_netif_rxSrcSmpId]->csd_available, IPI_IDLE);// to allow smp_call_function

	return;
}




int rtk_fc_smp_trx_init(void)
{
	int nicrx_ipi_cpu, nictx_ipi_cpu, wifirx_ipi_cpu;
	rtk_fc_dispatch_mode_t dispatch_mode;
	int nr_cpus = 1;

	nr_cpus =  num_online_cpus();
	
	dispatch_mode.mode = RTK_FC_DISPATCH_MODE_NONE;
	dispatch_mode.smp_id = 0;

	// default
	nicrx_ipi_cpu = -1;
	nictx_ipi_cpu = -1;
	wifirx_ipi_cpu = -1;
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	if(nr_cpus == 2) {
		nicrx_ipi_cpu = -1;
		nictx_ipi_cpu = 0;
		wifirx_ipi_cpu = 0;
	} else if(nr_cpus >= 4){
		nicrx_ipi_cpu = 2;
		nictx_ipi_cpu = 0;
		wifirx_ipi_cpu = 0;
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

	if(nr_cpus ==2) {
		nicrx_ipi_cpu = -1;
		nictx_ipi_cpu = 0;
		wifirx_ipi_cpu = 1;
	} else if(nr_cpus >= 4){
		nicrx_ipi_cpu = 2;
		nictx_ipi_cpu = 0;
		wifirx_ipi_cpu = -1;
	}
#endif
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	{
		cpumask_gmac0.bits[0]=0x2;  //CPU 1
		rtk_fc_irq_set_affinity_hint(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9

		cpumask_gmac1.bits[0]=0x2;  //CPU 1
		rtk_fc_irq_set_affinity_hint(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10

		cpumask_11ac.bits[0]=0x4;  //CPU 2
		rtk_fc_irq_set_affinity_hint(BSP_PCIE_11AC_IRQ, &cpumask_11ac); //PCIE 0

		cpumask_11n.bits[0]=0x2;  //CPU 1
		rtk_fc_irq_set_affinity_hint(BSP_PCIE_11N_IRQ, &cpumask_11n); //PCIE 1
	}
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_RX)
	/*
	 *	initialization for NIC(-to-FC) Rx dispatch
	 */

	// receive high priority packet to Linux directly.
	dispatch_mode.mode = RTK_FC_DISPATCH_MODE_NONE;
	dispatch_mode.smp_id = 0;
	rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_HIGHPRI_NIC_RX, NULL);
		
	if(nicrx_ipi_cpu==-1)
	{
		dispatch_mode.mode = RTK_FC_DISPATCH_MODE_NONE;
		dispatch_mode.smp_id = 0;
		rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_NIC_RX, NULL);
	}
	else
	{
		dispatch_mode.mode = RTK_FC_DISPATCH_MODE_IPI;
		dispatch_mode.smp_id = nicrx_ipi_cpu;
		rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_NIC_RX, NULL);
	}
#endif	

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_TX)
	/*
	 *	initialization for (FC-to-)NIC Tx dispatch
	 */
	dispatch_mode.mode = RTK_FC_DISPATCH_MODE_IPI;
	dispatch_mode.smp_id = nictx_ipi_cpu;
	rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_NIC_TX, NULL);
#endif


#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_RX)
	/*
	 *	initialization for WLAN Rx dispatch (nic hardware lookup)
	 */

	if(wifirx_ipi_cpu ==-1){
		dispatch_mode.mode = RTK_FC_DISPATCH_MODE_NONE;
		dispatch_mode.smp_id = 0;
		rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN0_RX, NULL);
		rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN1_RX, NULL);
		//rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN2_RX, NULL);

	}else{
		dispatch_mode.mode = RTK_FC_DISPATCH_MODE_IPI;
		dispatch_mode.smp_id = wifirx_ipi_cpu;
		rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN0_RX, NULL);
		rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN1_RX, NULL);
		//rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN2_RX, NULL);
	}
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_TX)

	dispatch_mode.mode = RTK_FC_DISPATCH_MODE_NONE;
	dispatch_mode.smp_id = 0;
	rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN0_TX, NULL);
	rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN1_TX, NULL);
	rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_WLAN2_TX, NULL);
#endif

	dispatch_mode.mode = RTK_FC_DISPATCH_MODE_IPI;
	dispatch_mode.smp_id = 0;
	rtk_fc_mgr_dispatchMode_set(dispatch_mode, RTK_FC_MGR_DISPATCH_MASK_ALL, RTK_FC_MGR_DISPATCH_PS_NETIF_RX, NULL);


	fc_mgr_db.fc_rps_maps.mode = RTK_FC_RPS_DISPATCH_MODE_END;
	
	return SUCCESS;
}

#endif

void rtk_fc_skb_rxhook_skb_handling(struct sk_buff *skb, int ret)
{
	if(ret == RTK_FC_NIC_RX_CONTINUE) {
		rtk_fc_skb_eth_type_trans(skb, skb->dev);
		rtk_fc_skb_netif_rx(skb);
	}else if(ret == RTK_FC_NIC_RX_STOP) {
		dev_kfree_skb_any(skb);
	}else if(ret == RTK_FC_NIC_RX_STOP_SKBNOFREERE){
		// donothing
	}else {
		FCMGR_ERR("unexpected ret %d", ret);
	}
}
int rtk_fc_skb_nic_rxhook_register(fc_rx_callback rxfunc)
{
	fc_mgr_db._rtk_fc_post_nic_skb_rx = rxfunc;
	return SUCCESS;
}
int rtk_fc_skb_wifi_rxhook_register(fc_rx_callback rxfunc)
{
	fc_mgr_db._rtk_fc_post_wifi_skb_rx = rxfunc;
	return SUCCESS;
}

#if defined(FC_SAMPLE_CODE)
int rtk_fc_rx_callback_test_NIC(struct sk_buff *skb, fc_rx_info_t *pRxInfo)
{
	FCMGR_PRK("skb dev %s", skb->dev->name);
	return  RTK_FC_NIC_RX_CONTINUE;
}
int rtk_fc_rx_callback_test_WIFI(struct sk_buff *skb, fc_rx_info_t *pRxInfo)
{
	if(pRxInfo==NULL)
		FCMGR_PRK("skb dev %s and pkt is mc/mc", skb->dev->name);
	else	
		FCMGR_PRK("skb dev %s and pkt is uc", skb->dev->name);
	
	return  RTK_FC_NIC_RX_CONTINUE;
}
int rtk_fc_wifi_client_mode_test_CB(struct net_device *dev, struct sk_buff *skb, bool rx)
{
	if(dev==NULL || skb==NULL)
		FCMGR_PRK("dev or skb  NULL");
	else
		FCMGR_PRK("isWifiRx %d dev %s skb %px ", rx, dev->name, skb);

	if(skb->data){
		if(rx)
			memset(&skb->data[0], 0xA0, 6);
		else
			memcpy(&skb->data[6], &dev->dev_addr[0], 6);
	}
	
	return  SUCCESS;
}
void rtk_fc_l2_refresh_notifier(unsigned char *mac, unsigned short vid, unsigned char anyflowestablish)
{
	if(mac==NULL)
		FCMGR_PRK("mac is null");
	else
		FCMGR_PRK("L2 mac %pM is still alive, vid = %d, anyflowestablish = %d", mac, vid, anyflowestablish);
}
#endif

int _rtk_fc_set_skbMark_vlaue(struct sk_buff *skb, rtk_fc_mgr_skbmarkTarget_t target, int32 value)
{
	int startBit, len, mark1or2;
			
	startBit = fc_mgr_db.mgr_skbmark[target].startBit;
	len 	 = fc_mgr_db.mgr_skbmark[target].len;
	mark1or2 = fc_mgr_db.mgr_skbmark[target].mark1or2;

	if(mark1or2 == 0) {
		skb->mark &= ~(((1<<len)-1) << startBit);
		skb->mark |= (value << startBit);
	}
#if defined(CONFIG_RTK_SKB_MARK2)
	else if(mark1or2 == 1) {		
		skb->mark2 &= ~(((1LL<<len)-1) << startBit);
		skb->mark2 |= ((unsigned long long)value << startBit);
	}
#endif
	
	return SUCCESS;
}

int _rtk_fc_get_skbMark_vlaue(struct sk_buff *skb, rtk_fc_mgr_skbmarkTarget_t target)
{
	int startBit, len, mark1or2;
	uint32 mark_value = 0;
			
	startBit = fc_mgr_db.mgr_skbmark[target].startBit;
	len 	 = fc_mgr_db.mgr_skbmark[target].len;
	mark1or2 = fc_mgr_db.mgr_skbmark[target].mark1or2;

	if(mark1or2 == 0)
		mark_value = (skb->mark>>startBit)&((1<<len)-1);
#if defined(CONFIG_RTK_SKB_MARK2)
	else if(mark1or2 == 1)
		mark_value = (uint32)(skb->mark2>>startBit)&((1LL<<len)-1);
#endif
	
	return mark_value;
}

void _rtk_fc_set_streamId_to_skbMark(struct sk_buff *skb, fc_rx_info_t *pRxInfo)
{
#if !defined(CONFIG_FC_CAG3_SERIES) && !defined(CONFIG_FC_RTL8198F_SERIES)
	/*
	*  If ingressPort is PON port,
	*  then set rx_info's stream id to skb->mark from fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN].startBit
	*											   to fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].startBit+len
	*/
	u32 skbmark=0;
#if defined(CONFIG_RTK_SKB_MARK2)
	u64 skbmark2=0;
#endif

	int streamId_startBit, streamId_len, enableBit_startBit;

	streamId_startBit  = fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID].startBit;
	streamId_len       = fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID].len;
	enableBit_startBit = fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID_EN].startBit;

	if(enableBit_startBit == RTK_FC_MGR_RMK_UNDEF)
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB)
	{
		skb->mark = skbmark;		//clear mark to zero
#if defined(CONFIG_RTK_SKB_MARK2)
		skb->mark2 = skbmark2;		//clear mark2 to zero
#endif
		return;
	}
#else
		return;

	skbmark=skb->mark;		//apply streamID without overwritten
#if defined(CONFIG_RTK_SKB_MARK2)
	skbmark2=skb->mark2;	//apply streamID without overwritten
#endif
#endif
	
	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID].mark1or2 == 0){
		uint32 ori_skbmark = skb->mark;
#if !defined(CONFIG_RTL_ETH_RECYCLED_SKB)
		u32 clear_streamId_bit=0;
		// step 1: Create a mask to clear the stream id position in skb->mark, e.g. 1111  11 00	0000  0 111  1111  1111  1111  1111  , total 32bits.																			-> | stream id |<-
		clear_streamId_bit = 0xffffffff^((1 << (streamId_startBit + streamId_len)) - 1) ^ ((1 << (streamId_startBit)) - 1);

		// step 2: Clear the bits.
		skbmark &= clear_streamId_bit;
#endif
		// step 3: Set streamId bits
		skbmark |= (RXINFO_PON_SID(pRxInfo)<<(streamId_startBit));
		FCMGR_PRK("set streamID %u to skb->mark = 0x%08X (original:0x%08X)", (RXINFO_PON_SID(pRxInfo)), skbmark, ori_skbmark);
	}
	// step 4: Set Enable bit
	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID_EN].mark1or2==0)
	{
		uint32 ori_skbmark = skb->mark;
		skbmark |= 1 << enableBit_startBit; 			// If enable bit is set on mark1
		FCMGR_PRK("set streamID_en to skb->mark = 0x%08X (original:0x%08X)", skbmark, ori_skbmark);
	}
		
#if defined(CONFIG_RTK_SKB_MARK2)
	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID].mark1or2 == 1){
		uint64 ori_skbmark = skb->mark2;
#if !defined(CONFIG_RTL_ETH_RECYCLED_SKB)
		u64 clear_streamId_bit_mark2=0;
		// step 1: Create a mask to clear the stream id position in skb->mark
		clear_streamId_bit_mark2 = 0xffffffffffffffff^((1LL << (streamId_startBit + streamId_len)) - 1) ^ ((1LL << (streamId_startBit )) - 1);

		// step 2: Clear the bits.

		skbmark2 &= clear_streamId_bit_mark2;
#endif
		// step 3: set stream id.
		skbmark2 |= ((unsigned long long)(RXINFO_PON_SID(pRxInfo)))<<(streamId_startBit);

		FCMGR_PRK("set streamID %u to skb->mark2 = 0x%016llX (original:0x%016llX)", (RXINFO_PON_SID(pRxInfo)), skbmark2, ori_skbmark);
	}
	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID_EN].mark1or2==1)
	{
		uint64 ori_skbmark = skb->mark2;
		skbmark2 |= 1LL << enableBit_startBit; 		// If enable bit is set on mark2
		FCMGR_PRK("set streamID_en to skb->mark2 = 0x%016llX (original:0x%016llX)", skbmark2, ori_skbmark);
	}
#endif

	skb->mark = skbmark;
#if defined(CONFIG_RTK_SKB_MARK2)
	skb->mark2 = skbmark2;
#endif
#endif
	return;
}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
static int _rtk_fc_skipFcEgressFunc(struct sk_buff *skb, int dpmask)
{
	fc_tx_info_t	*ptxInfo, txInfo;
	uint32 egress_qid = 0, egress_streamIdEn = 0, egress_streamId = 0;
	uint8 ip_summed;

	if(unlikely(fc_mgr_db.smpStatistic)){
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_FROM_PS_RX_SKIP_FC].smp_static[smp_processor_id()]);
	}

	FCMGR_PRK("[TX] SKB[%p] tx to pmask %d ", skb, dpmask);

	/*skipFcEgressFunc:
	- not support NIC vlan offload function
	- not support sw host policing counting
	*/
		
	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_QID].startBit != RTK_FC_MGR_RMK_UNDEF)
		egress_qid = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_QID);

	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID_EN].startBit != RTK_FC_MGR_RMK_UNDEF)
		egress_streamIdEn = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_STREAMID_EN);

	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID].startBit != RTK_FC_MGR_RMK_UNDEF)
	{
		egress_streamId = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_STREAMID);
		if(atomic_read(&fc_mgr_db.epon_llid_format) == 1)
		{
			egress_streamId <<= 4;
			egress_streamId |= egress_qid;
			egress_streamId &= ((1 << 7) -1); // stream ID has 7 bits
		}
	}
	
	// nic tx
	ptxInfo = &txInfo;
	memset(ptxInfo, 0, sizeof(fc_tx_info_t));

	TXINFO_EOR(ptxInfo)			= 0;
	TXINFO_FS(ptxInfo) 			= 1;
	TXINFO_LS(ptxInfo) 			= 1;
	rtk_fc_skb_ip_summed_get(skb, &ip_summed);
	if(ip_summed!= CHECKSUM_NONE)
	{
		// CHECKSUM_NONE/Tx: The skb was already checksummed by the protocol, or a checksum is not required.
		TXINFO_IPCS(ptxInfo)			= 1;
		TXINFO_L4CS(ptxInfo)			= 1;
	}
	TXINFO_CRC(ptxInfo)			= 1;
	TXINFO_DATA_LEN(ptxInfo)	= skb->len;
	TXINFO_CPUTAG(ptxInfo) 		= 1;

	TXINFO_SVLAN_ACT(ptxInfo)   = 0;
	TXINFO_CVLAN_ACT(ptxInfo)   = 0;
			
	TXINFO_SVLAN_CFI(ptxInfo)		= 0;
	TXINFO_CVLAN_CFI(ptxInfo)		= 0;
	TXINFO_TX_PORTMSK(ptxInfo)      = dpmask;

	TXINFO_ASPRI(ptxInfo)		= 1;
	TXINFO_CPUTAG_PRI(ptxInfo) 	= egress_qid;
	TXINFO_KEEP(ptxInfo)		= 0;
	TXINFO_DISLRN(ptxInfo) 		= 1;
	if(dpmask & (1<<RTK_FC_MAC_PORT_PON)){
		TXINFO_CPUTAG_PSEL(ptxInfo)		= egress_streamIdEn;
		TXINFO_TX_DST_STREAM_ID(ptxInfo)= egress_streamId;
	}
	TXINFO_L34_KEEP(ptxInfo)		= 1;
	TXINFO_GMAC_ID(ptxInfo)		    = 0;
	TXINFO_EXTSPA(ptxInfo) 			= 0;
	TXINFO_TX_PPPOE_ACT(ptxInfo)    = 0;
	TXINFO_TX_PPPOE_IDX(ptxInfo)	= 0;

	{
#if defined(CONFIG_RTL8686NIC)
		re8686_send_with_txInfo(skb, ptxInfo, (FC_NIC_TX_PRI_TO_RING >> (TXINFO_CPUTAG_PRI(ptxInfo)<<2)) & 0xf);
		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+TXINFO_GMAC_ID(ptxInfo)].smp_static[smp_processor_id()]);
#endif		

	}
	return SUCCESS;
}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
static int _rtk_fc_skipFcEgressFunc(struct sk_buff *skb, int dport)
{
	uint32 egress_qid = 0, egress_streamIdEn = 0, egress_streamId = 0;
	ca_ni_tx_config_t ca_tx_config;
	
	if(unlikely(fc_mgr_db.smpStatistic)){
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_FROM_PS_RX_SKIP_FC].smp_static[smp_processor_id()]);
	}
	
	FCMGR_PRK("[TX] SKB[%p] tx to port %d ", skb, dport);

	/*skipFcEgressFunc:
	- not support NIC vlan offload function
	- not support sw host policing counting
	*/

	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_QID].startBit != RTK_FC_MGR_RMK_UNDEF)
		egress_qid = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_QID);

	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID_EN].startBit != RTK_FC_MGR_RMK_UNDEF)
		egress_streamIdEn = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_STREAMID_EN);

	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_STREAMID].startBit != RTK_FC_MGR_RMK_UNDEF)
	{
		egress_streamId = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_STREAMID);
		if(atomic_read(&fc_mgr_db.epon_llid_format) == 1)
		{
			egress_streamId <<= 4;
			egress_streamId |= egress_qid;
			egress_streamId &= ((1 << 7) -1); // stream ID has 7 bits
		}
	}

	memset(&ca_tx_config, 0, sizeof(ca_tx_config));

#if defined(CONFIG_FC_CAG3_SERIES)
	if((dport == RTK_FC_MAC_PORT_PON) && (egress_streamIdEn == TRUE))
	{
		/* add stag for PON SID */

		uint16 svlanTCI = egress_streamId;
		int off = 0, insertOff = 0;
		u8 *pData = skb->data;
		struct vlan_hdr *pCvlan_hdr = NULL, *pSvlan_hdr = NULL;
		struct ethhdr *eth_hdr = NULL;
		int parsing_fail = FALSE;

		{
			//parsing CVLAN tag
			if(unlikely((skb == NULL) || (skb->data == NULL) || (skb->dev == NULL) ||
				((skb->len - skb->data_len) < ETH_HLEN)/*non linear skbuff len < ETH_HLEN*/))
				parsing_fail = TRUE;

			if(parsing_fail != TRUE)
			{
				off += (ETH_HLEN-2);

				// SVLAN
				if((*(u16*)(pData+off)) == htons(ETH_P_8021AD))
				{
					//already have Stag. Insert Stag fail, keep original
				}
				else
				{
					off+=VLAN_HLEN;

					// CVLAN
					if((*(u16*)(pData+off)) == htons(ETH_P_8021Q))
					{
						pCvlan_hdr = (struct vlan_hdr *)(pData+off+2);
						svlanTCI |= (ntohs(pCvlan_hdr->h_vlan_TCI) & VLAN_PRIO_MASK); //svlan pri = cvlan pri
					}

					insertOff += (ETH_HLEN - 2);
					if(rtk_fc_skb_skb_cow_head(skb, VLAN_HLEN) < 0)
					{
						//"skb head room is not enough. Insert Stag fail, keep original
					}
					else
					{
						rtk_fc_skb_skb_push(skb, VLAN_HLEN, &pData);
						memmove(pData, pData + VLAN_HLEN, insertOff);
						skb->mac_header -= VLAN_HLEN;

						eth_hdr = (struct ethhdr *)pData;
						eth_hdr->h_proto = htons(ETH_P_8021AD);

						pSvlan_hdr = (struct vlan_hdr *)(pData+insertOff+2);
						pSvlan_hdr->h_vlan_TCI = htons(svlanTCI);
					}
				}
			}
		}
	}

#elif !defined(CONFIG_FC_RTL8198F_SERIES)
	
	if((dport == RTK_FC_MAC_PORT_PON) && (egress_streamIdEn == TRUE)) { // PON SID
		// to pon	
		rtk_fc_coreDB_info_t coredb;
		
		coredb.data_sid.index = egress_streamId;
		rtk_fc_coreDBInfo_get(RTK_FC_COREDB_OPS_STREAMID, &coredb);

		if(!coredb.data_sid.entry.valid) {
			dev_kfree_skb_any(skb);
			FCMGR_ERR("invalid sid %d, drop packet.", egress_streamId);
			return SUCCESS;
		}

		ca_tx_config.core_config.bf.ldpid = coredb.data_sid.entry.ldpid;
		ca_tx_config.core_config.bf.txq_index = coredb.data_sid.entry.cos;	
		
		ca_tx_config.core_config.bf.flow_id_set = TRUE;
		ca_tx_config.flow_id = coredb.data_sid.entry.gemid;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		ca_tx_config.stream_id = egress_streamId;
		ca_tx_config.stream_id_en = 1;
#endif
		
		ca_tx_config.core_config.bf.lspid = RTK_FC_MAC_PORT_CPU1;	// for mirror function: if enabling pon port(0x7) egress mirror, the ldpid may be 0x20~0x2F
	}else 

#endif
	{
		ca_tx_config.core_config.bf.ldpid = dport;
		ca_tx_config.core_config.bf.txq_index = egress_qid;
		
		ca_tx_config.core_config.bf.lspid = RTK_FC_MAC_PORT_CPU;
	}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	{
		rtk_fc_coreDB_info_t coredb;
		rtk_fc_coreDBInfo_get(RTK_FC_COREDB_OPS_TX_WO_HDR, &coredb);
		if(unlikely(coredb.data_tx_ctrl))
			ca_tx_config.force_def_tx_en = TRUE;
#if defined(CONFIG_FC_RTL9607F_SERIES)
		ca_tx_config.dmaaft_map = TXINFO_DMAAFT_MAP_IDX_DIR_TX(ca_tx_config.core_config.bf.lspid);
#endif
	}
#endif

	ca_tx_config.core_config.bf.is_from_ca_tx =  TRUE;
	ca_tx_config.core_config.bf.bypass_fwd_engine =  TRUE;
	
	//nic_egress_start_xmit(skb, skb->dev, &ca_tx_config);
	nic_egress_start_xmit_for_fc_dirTx(skb, skb->dev, &ca_tx_config);
	
	if(unlikely(fc_mgr_db.smpStatistic))
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+(ca_tx_config.core_config.bf.lspid-RTK_FC_MAC_PORT_CPU)].smp_static[smp_processor_id()]);

	return SUCCESS;
}
#endif

void _rtk_fc_skb_update_dev(fc_rx_info_t *pRxInfo, struct sk_buff *skb, void *extraInfo)
{
	struct net_device *rxdev=NULL;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	extern struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pOriRxInfo);
	struct rx_info rxInfo;

	if(RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo))
		return;
	RXINFO_SPA((&rxInfo)) = RXINFO_SPA(pRxInfo);

	if((rxdev=decideRxDevice((struct re_private *) extraInfo, &rxInfo))!=NULL)
		skb->dev = rxdev;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	extern struct net_device* ca_ni_decide_rx_device(ca_eth_private_t *cep, unsigned int phy_src_port);
	extern void updateRxStatic(struct sk_buff *skb);

	if(RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo))
		return;

	rtk_fc_decide_rx_device_by_spa(RXINFO_SPA(pRxInfo), &rxdev);
	if(rxdev){
		skb->dev = rxdev;
		updateRxStatic(skb);
	}
#endif

	return;
}

static inline void _rtk_fc_skb_manipulation(fc_rx_info_t *pRxInfo, struct sk_buff *skb)
{
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
	//check spa for streamID
	if(pRxInfo && RXINFO_SPA(pRxInfo)==RTK_FC_MAC_PORT_PON)
		_rtk_fc_set_streamId_to_skbMark(skb, pRxInfo);
	else{
		skb->mark=0; //clear in case residual streamID cause disconnect
#ifdef CONFIG_RTK_SKB_MARK2
		skb->mark2=0;	//clear in case residual streamID cause disconnect
#endif
	}
#endif
#if defined(CONFIG_RTK_APP_PORT_BINDING_SUPPORT)
	skb->from_dev = skb->dev;
#endif
	if(pRxInfo && IS_COPY2CPU_PKT(pRxInfo))
	{
		skb->fcIngressData.isHwCopy2CPU=1;
		skb->fcIngressData.doLearning=0;
	}			
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	// for wifi rx, make sure cb is done before netif_rx.
	if(pRxInfo && RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo) && fc_mgr_db._rtk_fc_wifi_client_mode_cb) {
		rtk_fc_wlan_devmap_t *devmap = NULL;
		rtk_fc_wlan_devmap_get(RXINFO_CPU_EXTSPA(pRxInfo), &devmap);
		if(devmap && devmap->attr.client_mode) {
			fc_mgr_db._rtk_fc_wifi_client_mode_cb(skb->dev, skb, TRUE);
		}
	}
#endif
}

static inline void _rtk_fc_post_processing(fc_rx_info_t *pRxInfo, struct rt_skbuff rtskb, struct sk_buff **skb)
{
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
	if(rtskb.skb->recyclable) {*skb=recycle_skb_swap(*skb);if(*skb==NULL)return;}
	else if((*skb)->recyclable){*skb=rtskb.skb;}//swap at rtk_fc_ingress_flowLearning, renew skb pointer!
#endif
	_rtk_fc_skb_manipulation(pRxInfo, *skb);

}

static void _rtk_fc_rx_final_process(fc_rx_info_t *pRxInfo, struct sk_buff *skb, void *extraInfo)
{
	rtk_fc_nic_rx_t ret;
	struct rt_skbuff rtskb;
	
#if defined(CONFIG_RTK_FC_PTOOL_CPU_PERF)
	PROFILE_START
#endif
	// prefetch for packet parser
	prefetch(&skb->len);
	prefetch(&skb->data);	
	rtk_fc_converter_skb(skb, &rtskb);

#if defined(CONFIG_RTK_FC_PTOOL_CPU_PERF)
	PROFILE_END
#endif

	if((ret = rtk_fc_ingress_flowLearning(&rtskb, pRxInfo)) == RTK_FC_NIC_RX_STOP_SKBNOFREERE) {
		// shortcut done
	}else {
		_rtk_fc_post_processing(pRxInfo, rtskb, &skb);
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB    
		if(!skb) return;
#endif
		switch(ret) {
			case RTK_FC_NIC_RX_CONTINUE:
			case RTK_FC_NIC_RX_NETIFRX_BY_FC:
				_rtk_fc_skb_update_dev(pRxInfo, skb, extraInfo);
				if((ret == RTK_FC_NIC_RX_CONTINUE) && fc_mgr_db._rtk_fc_post_nic_skb_rx) {
					ret = fc_mgr_db._rtk_fc_post_nic_skb_rx(skb, pRxInfo);
					rtk_fc_skb_rxhook_skb_handling(skb, ret);
					return;
				}
				if((ret == RTK_FC_NIC_RX_NETIFRX_BY_FC) && fc_mgr_db._rtk_fc_post_wifi_skb_rx) {
					ret = fc_mgr_db._rtk_fc_post_wifi_skb_rx(skb, pRxInfo);
					rtk_fc_skb_rxhook_skb_handling(skb, ret);
					return;
				}
				rtk_fc_skb_eth_type_trans(skb, skb->dev);
				rtk_fc_skb_netif_rx(skb);
				break;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)				
			case RTK_FC_NIC_RX_IPSEC_RELEARN:
			{
				struct ethhdr *eth= NULL;
				_rtk_fc_post_processing(pRxInfo, rtskb, &skb);
				
				skb_reset_mac_header(skb);

				eth = (struct ethhdr *)skb->data;
				skb_pull_inline(skb, ETH_HLEN);
				skb->protocol = eth->h_proto;
				FCMGR_PRK("IPSEC hw lookup miss and netif rx, DA: %pM, SA: %pM, eth->h_proto = 0x%04x, net_device = %s", &eth->h_dest[0], &eth->h_source[0],eth->h_proto, (skb->dev)?skb->dev->name:"NULL");
				
				rtk_fc_skb_netif_rx(skb);
				break;
			}
#endif			
			case RTK_FC_NIC_RX_STOP:
				dev_kfree_skb_any(skb);
				break;
			//case RTK_FC_NIC_RX_STOP_SKBNOFREERE:
				// done
				// break;
			case RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS:				
				if(fc_mgr_db._rtk_fc_post_wifi_skb_rx) {
					ret = fc_mgr_db._rtk_fc_post_wifi_skb_rx(skb, pRxInfo);
					rtk_fc_skb_rxhook_skb_handling(skb, ret);
					return;
				}		
				rtk_fc_skb_netif_rx(skb);
				break;
			default:
				dev_kfree_skb_any(skb);
				FCMGR_ERR("ingress learning ret = %d", ret);
				break;
		}
	}
}

__IRAM_FC_NICTRX
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) && defined(CONFIG_RX_WIFI_FF_AGG)
int rtk_fc_nicRx_to_wifiTx_agg(int gmac, struct sk_buff *skb, struct rx_info *pOriRxInfo)
{
	fc_rx_info_t rxInfo;
	rtk_fc_wlan_devidx_t wlandevid = fc_mgr_db.wlan_port_to_devidx[gmac][__ffs(pOriRxInfo->opts3.bit.fb_hash_or_dst_portmsk)];

	// shared extport - return to basic rx hook function because l2 lookup is required.
	if(wlandevid >= RTK_FC_WLANX_END_INTF)
	{
		FCMGR_ERR("wlan dev id[%d] shared extport should not enter here...%s!! packet drop here.",wlandevid,__FUNCTION__);
		dev_kfree_skb_any(skb);
		return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
	}

	rxInfo.opts3.bit.internal_priority = pOriRxInfo->opts3.bit.internal_priority;
	
	if(unlikely(fc_mgr_db.smpStatistic)){
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BANDx_TX_FF_AGG].smp_static[smp_processor_id()]);
	}

	dma_cache_wback_inv((unsigned long)skb->data,/*(u32)skb->end-(u32)skb->data*/skb->len);

	rtk_fc_wlan_hard_start_xmit(wlandevid, skb, &rxInfo);

	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
}

int rtk_fc_nicRx_to_nicTx_agg(int gmac, struct sk_buff *skb, struct rx_info *pOriRxInfo)
{
	rtk_fc_smp_nicTx_private_t smp_nicTx_info={0};
    fc_tx_info_t *ptxInfo=&smp_nicTx_info.txInfo;

    smp_nicTx_info.skb = skb;

    TXINFO_FS(ptxInfo) = 1;
    TXINFO_LS(ptxInfo) = 1;
    TXINFO_STAG_AWARE(ptxInfo)=1;

    TXINFO_DISLRN(ptxInfo) = 1;
    TXINFO_L34_KEEP(ptxInfo) = 1;

    TXINFO_GMAC_ID(ptxInfo)=gmac;
    if(gmac==0){
            FCMGR_PRK("from gmac 0 send to port 1!!!");
            TXINFO_TX_PORTMSK(ptxInfo) = 0x2;       //send to port 1
    }else{
            FCMGR_PRK("from gmac 1 send to port 0!!!");
            TXINFO_TX_PORTMSK(ptxInfo) = 0x1;       //send to port 0
    }

    rtk_fc_nic_tx(&smp_nicTx_info);

	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
}
#endif

__IRAM_FC_NICTRX
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_nicRx_to_wifiTx(struct re_private *cp, struct sk_buff *skb, struct rx_info *pOriRxInfo)
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
int rtk_fc_nicRx_to_wifiTx(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv)
#endif	
{

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(pOriRxInfo->opts2.bit.reason == CPU_REASON_L34_FWD)  {
		uint32 dst_extp_cnt = pOriRxInfo->opts3.bit.fb_hash_or_dst_portmsk;
		int trapToPs = 0;

		if(pOriRxInfo->opts3.bit.fb_hash_or_dst_portmsk==0x1)
		{
			//for hardware copy to cpu  isHwCopy2CPU / FC_DBUG_TOPSREASON_COPY2CPU , do not search wifi fastfwd 
			return RTK_FC_NIC_RX_CONTINUE;
		}

		RTK_FC_ONE_COUNT(dst_extp_cnt);

		if(dst_extp_cnt==1) {
			fc_rx_info_t rxInfo;
			rtk_fc_wlan_devidx_t wlandevid = fc_mgr_db.wlan_port_to_devidx[cp->gmac][__ffs(pOriRxInfo->opts3.bit.fb_hash_or_dst_portmsk)];

#if defined(CONFIG_RTK_SOC_RTL8198D)
			if (wlandevid == RTK_FC_WLANX_MULTI_INTF) {
				rtk_fc_mgr_l2_info_t l2info = {0};

				if (rtk_fc_l2Info_get(skb->data, &l2info) == SUCCESS) {		// by da
					if (l2info.wlan_dev_idx < RTK_FC_WLANX_END_INTF)
						wlandevid = l2info.wlan_dev_idx;
				}
			}
#endif

			// shared extport - return to basic rx hook function because l2 lookup is required.
			if(wlandevid >= RTK_FC_WLANX_END_INTF)
				return RTK_FC_NIC_RX_CONTINUE;
		
			rxInfo.opts3.bit.internal_priority = pOriRxInfo->opts3.bit.internal_priority;
			
			if(unlikely(fc_mgr_db.smpStatistic)){
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BANDx_TX_FF].smp_static[smp_processor_id()]);
			}
			
#if defined(CONFIG_RTK_SOC_RTL8198D)
			if((skb->data[0]&1)==1)
			{
				if(rtk_fc_check_user_group((unsigned char*)skb->data) == 1)
				{
					//trap to ps case : not goto wifi fastforward, so skb dev is required 
					{
						extern struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pOriRxInfo);
						
						struct net_device *rxdev=NULL;
						if((rxdev=decideRxDevice(cp, pOriRxInfo))!=NULL)
							skb->dev = rxdev;
					}
					trapToPs = 1;
				}
			}
#endif

			if(trapToPs == 0)
			{
#if defined(CONFIG_RTK_SOC_RTL8198D)
				// WAN -> WLAN hw routing
				rtk_fc_ext_count_nic_rx_to_wifi_tx_inc(skb, RXINFO_SPA(pOriRxInfo), RXINFO_L3ROUTING(pOriRxInfo), wlandevid);
#endif

				rtk_fc_wlan_hard_start_xmit(wlandevid, skb, &rxInfo);
				return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
			}
		}
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	if(RTK_FC_IS_WIFI_FF_UC(nh_priv) == 1)
#else
	if(RTK_FC_IS_WIFI_FF_UC(nh_priv))
#endif
	{
		fc_rx_info_t rxInfo = {0};
		fc_rx_info_t *pRxInfo = &rxInfo;
		RXINFO_IS_WIFI_FF(pRxInfo) = RTK_FC_IS_WIFI_FF_UC(nh_priv);

		rxInfo.opts3.bit.internal_priority = RTK_FC_WIFI_FF_COS(nh_priv);

		if(unlikely(fc_mgr_db.smpStatistic)){
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BANDx_TX_FF].smp_static[smp_processor_id()]);
		}

		rtk_fc_wlan_hard_start_xmit(RTK_FC_WIFI_FF_DEVID(nh_priv), skb, &rxInfo);
				
		return RTK_FC_NIC_RX_STOP_SKBNOFREERE;

	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if (RTK_FC_IS_WIFI_FF_UC(nh_priv) == 2)
	{
		fc_rx_info_t rxInfo = {0};
		fc_rx_info_t *pRxInfo = &rxInfo;
		RXINFO_IS_WIFI_FF(pRxInfo) = RTK_FC_IS_WIFI_FF_UC(nh_priv);
		/*printk("[%s][%d] wifi amsdu\n", __func__, __LINE__);*/
		rxInfo.opts3.bit.internal_priority = RTK_FC_WIFI_FF_COS(nh_priv);

                if(unlikely(fc_mgr_db.smpStatistic)){
                        atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BANDx_TX_FF].smp_static[smp_processor_id()]);
                }
                skb->cb[32]=0x5a;

                rtk_fc_wlan_hard_start_xmit(RTK_FC_WIFI_FF_DEVID(nh_priv), skb, &rxInfo);

                return RTK_FC_NIC_RX_STOP_SKBNOFREERE;

	}
#endif
		
#endif

	return RTK_FC_NIC_RX_CONTINUE;
}

__IRAM_FC_NICTRX
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_skb_rx(struct re_private *cp, struct sk_buff *skb, struct rx_info *pOriRxInfo)
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
int rtk_fc_skb_rx(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv)
#endif	
{
	fc_rx_info_t *pRxInfo, rxInfo;
	bool if_skipFcIngressFunc = FALSE;
	bool if_skipFcFunc_to_skbMARK = FALSE;
	bool highpri = FALSE;
	void *extraInfo = NULL;
	
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	HEADER_A_T *hdrA = nh_priv->hdr_a;
	HEADER_CPU_T *hdrCPU = nh_priv->hdr_cpu;
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D)
	int trapToPs = 0;
#endif
	int ret = 0;
	int copyToCPU=FALSE;

	if(unlikely(fc_mgr_db.smpStatistic))
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC_RX].smp_static[smp_processor_id()]);

	pRxInfo = &rxInfo;
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	extraInfo = cp;	//for skb dev information

	/*
	 * rxinfo translation
	 */ 
	pRxInfo->opts2.dw = pOriRxInfo->opts2.dw;
	pRxInfo->opts3.dw = pOriRxInfo->opts3.dw;
	
	// aditional sw fields in rxinfo
	RXINFO_GMAC(pRxInfo)= cp->gmac;

#if defined(CONFIG_FC_RTL9607C_SERIES)
	// special control: DS to P7 and to achieve bandwidth control, acl rule is controlled by user. acl action is "trap-to-slave-cpu".
	if(fc_mgr_db.pon_ds_p7_loopback_en && RXINFO_SPA(pRxInfo) == RTK_FC_MAC_PORT_SLAVECPU)
		RXINFO_SPA(pRxInfo) = RTK_FC_MAC_PORT_PON;
#endif

#if defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	if(RXINFO_GMAC(pRxInfo) == 1) {
		RXINFO_GMAC(pRxInfo) = 0;
	}
#elif defined(CONFIG_FC_WIFI_TX_GMAC_AUTO_SEL_SUPPORT)
	//if(RXINFO_GMAC(pRxInfo) == 1 || RXINFO_GMAC(pRxInfo) == 2)
		RXINFO_GMAC(pRxInfo) = 0;
#endif

#if !defined(CONFIG_FC_WIFI_RX_NONE) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	if(RXINFO_SPA(pRxInfo) == RTK_FC_MAC_PORT_MASTERCPU_CORE1)
		RXINFO_SPA(pRxInfo) = RTK_FC_MAC_PORT_MAINCPU;
#if defined(CONFIG_FC_WIFI_RX_GMAC_AUTO_SEL_SUPPORT) && (GMAC_TRUNKING_NUM == 3 )
#ifdef CONFIG_RTK_WFO_GMAC2_SHARING
	/* port 7 (port 9 be set to port 7) extspa 1 (RTK_FC_MAC_EXT_PORT0) for "5G STA" => change to 9
	   port 7 extspa 2/4/6 for "2.4G STA" => no change
	 */
	if((RXINFO_SPA(pRxInfo) == RTK_FC_MAC_PORT_SLAVECPU) && 
		(RXINFO_CPU_EXTSPA(pRxInfo) != RTK_FC_MAC_EXT_PORT1) && 
		(RXINFO_CPU_EXTSPA(pRxInfo) != RTK_FC_MAC_EXT_PORT3) && 
		(RXINFO_CPU_EXTSPA(pRxInfo) != RTK_FC_MAC_EXT_PORT5))
		RXINFO_SPA(pRxInfo) = RTK_FC_MAC_PORT_MAINCPU;
#else
	if(RXINFO_SPA(pRxInfo) == RTK_FC_MAC_PORT_SLAVECPU)
		RXINFO_SPA(pRxInfo) = RTK_FC_MAC_PORT_MAINCPU;
#endif
#endif
#endif // end of CONFIG_FC_WIFI_RX_NONE
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if(RXINFO_REASON(pRxInfo)==CPU_REASON_L34_FWD && RXINFO_DST_EXTPIDX(pRxInfo)==0x1)
		{
			copyToCPU=TRUE;
		}
#endif

	if((RXINFO_REASON(pRxInfo)==CPU_REASON_L34_FWD && copyToCPU==FALSE) 
		|| (RXINFO_REASON(pRxInfo)==CPU_REASON_LUT)	// Hybrid mode support l2 forwarding but no flooding
	) {
#if defined(CONFIG_RTK_SOC_RTL8198D)
		if(RXINFO_REASON(pRxInfo)==CPU_REASON_L34_FWD) 
		{
			if((skb->data[0]&1)==1)
			{
				if(rtk_fc_check_user_group((unsigned char*)skb->data) == 1)
				{
					//trap to ps case : not goto wifi fastforward, so skb dev is required 
					{
						extern struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pOriRxInfo);
					
						struct net_device *rxdev=NULL;
						if((rxdev=decideRxDevice(cp, pOriRxInfo))!=NULL)
							skb->dev = rxdev;
					}
					trapToPs = 1;
				}
			}
		}
		if(trapToPs == 0)
#endif
		{
			ret = rtk_fc_fastfwd_directXmit(skb, pRxInfo);
			return ret;
		}
	}

#if defined(CONFIG_RTK_SOC_RTL8198D)
	// traffic count: wlan rx -> nic tx, if hw acc fail, decrease counter here!!!
	rtk_fc_ext_count_wifi_rx_to_nic_tx_dec(skb, RXINFO_SPA(pRxInfo), RXINFO_CPU_EXTSPA(pRxInfo));
#endif

#if !defined(CONFIG_RTL_ETH_RECYCLED_SKB)
	if(RXINFO_SPA(pRxInfo)==RTK_FC_MAC_PORT_PON)
		_rtk_fc_set_streamId_to_skbMark(skb, pRxInfo);
#endif

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	/*
	 * rxinfo translation
	 */ 	
	memset(pRxInfo, 0, sizeof(fc_rx_info_t));

	RXINFO_SPA(pRxInfo)		= hdrA->bits.lspid;
	RXINFO_INT_PRI(pRxInfo) = hdrA->bits.cos;
#if defined(CONFIG_FC_SPECIAL_FAST_FORWARD)
	if(hdrA->bits.lspid == 0x14 || hdrA->bits.lspid == 0x15 || hdrA->bits.lspid == 0x16 || hdrA->bits.lspid == 0x17)
	{
		if(printk_ratelimit())	
			printk(KERN_INFO "[VXLAN] VXLAN Strange packet from lspid %x, drop! \n", hdrA->bits.lspid);
		return RTK_FC_NIC_RX_STOP;
	}
	
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_FC_RTL8277C_SERIES)
	if(RXINFO_SPA(pRxInfo)==RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE) {
		RXINFO_SPA(pRxInfo)=RTK_FC_MAC_PORT_PON;
		hdrA->bits.lspid = RTK_FC_MAC_PORT_PON;	/* for hwnat 0 */
	}
#endif
	if(RXINFO_SPA(pRxInfo)==RTK_FC_MAC_PORT_MC_MIRRORED) {
		if(fc_mgr_db.mirror_enable)
		{
			RXINFO_SPA(pRxInfo) = fc_mgr_db.mirrored_port;
			hdrA->bits.lspid = fc_mgr_db.mirrored_port;	/* for hwnat 0 */
		}
		else
			return RTK_FC_NIC_RX_STOP;
	}

	/* get packet info from mdata */
	RXINFO_REASON(pRxInfo) = MDATAL_REASON(hdrCPU->mdata_raw.mdata_l);
	RXINFO_HASH_CRC16(pRxInfo) = MDATAL_CRC16(hdrCPU->mdata_raw.mdata_l);
	RXINFO_HASH_CRC32(pRxInfo) = MDATAH_CRC32(hdrCPU->mdata_raw.mdata_h);
	
	/* TO WLAN - fast forward, store destination wlan dev ID into RXINFO_DST_EXTPIDX */
	if((hdrA->bits.lspid == AAL_LPORT_MC) && (skb->data[0]&0x1) && (hdrA->bits.bits_32_63.pkt_info.pol_id != 0))
	{
		// MC: fwd by l2FE mce + arb table, pol_id stands for wifi dev id
		if(hdrA->bits.bits_32_63.pkt_info.pol_id == RTK_FC_COPY2CPU_INTF)
		{
			RXINFO_REASON(pRxInfo) = CPU_REASON_FLOW_COPY;
			copyToCPU=TRUE;
		}
		else
		{
			// MC: fwd by l2FE mce + arb table, pol_id stands for wifi dev id
			RXINFO_REASON(pRxInfo) = CPU_REASON_L34_FWD;
			RXINFO_DST_EXTPIDX(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id; // to which wlan dev index
		}

	}
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	else if((hdrCPU->mdata_raw.mdata_l&0xffff) && MDATAL_SWID(hdrCPU->mdata_raw.mdata_l) >= RTK_FC_IPSEC_SWID_BASE&& RXINFO_SPA(pRxInfo)==RTK_FC_IPSEC_HWLOOKUP_MACPORT )
	{
		FCMGR_PRK("[IPSEC]hw lookup miss (reason: %d)! table index(mdata)= %d\n", RXINFO_REASON(pRxInfo), MDATAL_SWID(hdrCPU->mdata_raw.mdata_l) - RTK_FC_IPSEC_SWID_BASE);
		RXINFO_DST_EXTPIDX(pRxInfo) = MDATAL_SWID(hdrCPU->mdata_raw.mdata_l); // to which wlan dev index
	}
#endif	
	else if(RTK_FC_IS_WIFI_FF_UC(nh_priv))
	{
		
		// UC: fwd by L3FE mainhash, matadata stands for wifi flow id s
		RXINFO_REASON(pRxInfo) = CPU_REASON_L34_FWD;
#if 0// wifi FF: ldpid may be 0x10~0x0x13, and no need RXINFO_GMAC info
		RXINFO_GMAC(pRxInfo)= (hdrA->bits.ldpid - RTK_FC_MAC_PORT_WLAN_CPU0);			// to which mac (cpu) port
#endif
		RXINFO_DST_EXTPIDX(pRxInfo) = MDATAL_SWID(hdrCPU->mdata_raw.mdata_l); // to which wlan dev index

	}
	if((RXINFO_REASON(pRxInfo) != CPU_REASON_L34_FWD) && (RXINFO_SPA(pRxInfo) != RTK_FC_MAC_PORT_PON) && MDATAL_SWID(hdrCPU->mdata_raw.mdata_l)
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
		&& RXINFO_SPA(pRxInfo)!=RTK_FC_IPSEC_HWLOOKUP_MACPORT
#endif
	){

		// FROM WLAN - hardware lookup miss or wlan-to-wlan
		RXINFO_CPU_EXTSPA(pRxInfo) = MDATAL_SWID(hdrCPU->mdata_raw.mdata_l);  // ext port -> rx_extportspa
	}

	if(MDATAL_REASON_SW(hdrCPU->mdata_raw.mdata_l) == RXINFO_REF_TRAP_RSN_DUAL_DS)
	{
		// HW CRC is calculated by inner header (RXINFO_REF_TRAP_RSN_DUAL_DS is assigned by dual pop CLS)
		RXINFO_HASH_CRC_BY_DUAL_INNER(pRxInfo) = 1;
	}

#if defined(CONFIG_FC_RTL8277C_SERIES)
	if((RXINFO_REASON(pRxInfo) < CPU_REASON_FLOWMISS) && !(MDATAL_REASON_SW(hdrCPU->mdata_raw.mdata_l) == RXINFO_REF_TRAP_RSN_ACL))
	{
		/*
			(HW original reason < CPU_REASON_FLOWMISS) AND (not acl trap but hash double check failed) => need to recalculate CRC.
		*/
		RXINFO_HASH_CRC_NEED_RECAL_HW_RSN(pRxInfo) = 1;
	}
	else if((MDATAL_REASON_SW(hdrCPU->mdata_raw.mdata_l) == RXINFO_REF_TRAP_RSN_ACL) && (RXINFO_REASON(pRxInfo) == CPU_REASON_FLOWMISS))
	{
		/*
			SW work around for acl trap:
			1. set t2_ctrl to RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0  => for HW CRC calculation (will be calculate to 5-tuple CRC).
			3. (MDATAL_REASON_SW(mdata_l) == RXINFO_REF_TRAP_RSN_ACL)		=> for SW to recover ACL trap reason											=> for not to be Drop by MC hash profile default action (MC hash profile use policer rate 0 to drop packet)
			===
			HW reason:
			[5 tuple]
			- flow miss		<= ACL trap may be this reason and flow miss 				(NO need to recalculate CRC)
			- ACL trap		<= ACL trap may be this reason and hash double check failed	(NO need to recalculate CRC)
			[2 tuple]
			- flow miss		<= ACL trap may be this reason 								(need to recalculate CRC due to hash tuple be set to RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0)
			- ACL trap		<= ACL trap may be this reason and hash double check failed	(need to recalculate CRC due to hash tuple be set to RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0)
			[MC]
			- flow miss		<= ACL trap may be this reason 								(need to recalculate CRC due to hash tuple be set to RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0)
			- ACL trap		<= ACL trap may be this reason and hash double check failed	(need to recalculate CRC due to hash tuple be set to RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0)
			===
		*/
		RXINFO_HASH_CRC_NEED_RECAL_ACL_TRP(pRxInfo) = 1;
	}
	if((MDATAL_REASON(hdrCPU->mdata_raw.mdata_l) == CPU_REASON_ACL) || 
		(MDATAL_REASON_SW(hdrCPU->mdata_raw.mdata_l) == RXINFO_REF_TRAP_RSN_ACL)){
		//CLS reference info, work around mdata[16:63] will be overrite by hash CRC16/32, sw debug only
		RXINFO_REASON(pRxInfo) = CPU_REASON_ACL;
		RXINFO_REFIDX(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id;
	}

	if(MDATAL_REASON_SW(hdrCPU->mdata_raw.mdata_l) == RXINFO_REF_TRAP_RSN_ACL_TO_PS){
		RXINFO_REASON(pRxInfo) = CPU_REASON_ACL_TRAP_PS;
		FCMGR_PRK("[ACL] Trap to ps!! spa: 0x%x rule info: %d", RXINFO_SPA(pRxInfo), hdrA->bits.bits_32_63.pkt_info.pol_id);
	}
#elif defined(CONFIG_FC_RTL9607F_SERIES)
	if((RXINFO_REASON(pRxInfo) < CPU_REASON_FLOWMISS))
	{
		/*
			(HW original reason < CPU_REASON_FLOWMISS) AND (not acl trap but hash double check failed) => need to recalculate CRC.
		*/
		RXINFO_HASH_CRC_NEED_RECAL_HW_RSN(pRxInfo) = 1;
	}

	if((RXINFO_REASON(pRxInfo) == CPU_REASON_ACL_TRAP_PS) || (RXINFO_REASON(pRxInfo) == CPU_REASON_ACL)){
		//CLS reference info, work around mdata[16:63] will be overrite by hash CRC16/32, sw debug only
		RXINFO_REFIDX(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id;
		if(RXINFO_REASON(pRxInfo) == CPU_REASON_ACL_TRAP_PS)
			FCMGR_PRK("[ACL] Trap to ps!! spa: 0x%x rule info: %d", RXINFO_SPA(pRxInfo), hdrA->bits.bits_32_63.pkt_info.pol_id);
	}

#endif

#else

	// TO WLAN - fast forward, store destination wlan dev ID into RXINFO_DST_EXTPIDX
	if((hdrA->bits.lspid == AAL_LPORT_MC) && (skb->data[0]&0x1) && (hdrA->bits.bits_32_63.pkt_info.pol_id != 0))
	{
		// MC: fwd by l2FE mce + arb table, pol_id stands for wifi dev id
		if(hdrA->bits.bits_32_63.pkt_info.pol_id == RTK_FC_COPY2CPU_INTF)
		{
			RXINFO_REASON(pRxInfo) = CPU_REASON_FLOW_COPY;
			copyToCPU=TRUE;
		}
		else
		{
			// MC: fwd by l2FE mce + arb table, pol_id stands for wifi dev id
			RXINFO_REASON(pRxInfo) = CPU_REASON_L34_FWD;
			RXINFO_DST_EXTPIDX(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id; // to which wlan dev index
		}

	}
	else if(RTK_FC_IS_WIFI_FF_UC(nh_priv))
	{
		// UC: fwd by L3FE mainhash, matadata stands for wifi flow id s
		RXINFO_REASON(pRxInfo) = CPU_REASON_L34_FWD;

		RXINFO_DST_EXTPIDX(pRxInfo) = (hdrCPU->mdata_raw.mdata_l&0xffff); // to which wlan dev index

	}
	
	if((1 << RXINFO_SPA(pRxInfo)) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK){

		// FROM WLAN - hardware lookup miss or wlan-to-wlan

		RXINFO_CPU_EXTSPA(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id;			// ext port -> rx_extportspa
	}

	if(hdrCPU->mdata_raw.mdata_h&RXINFO_REF_VALID_BIT){
		if(hdrCPU->mdata_raw.mdata_h&RXINFO_REF_TRAP_RSN_BIT){
			switch((hdrCPU->mdata_raw.mdata_h&RXINFO_REF_TRAP_RSN_BIT)>>RXINFO_REF_TRAP_RSN_SHIFT_BIT){
				case RXINFO_REF_TRAP_RSN_ACL:
					RXINFO_REASON(pRxInfo) = CPU_REASON_ACL;
					break;
				case RXINFO_REF_TRAP_RSN_FLOWMISS:
					RXINFO_REASON(pRxInfo) = CPU_REASON_FLOWMISS;
					break;
				case RXINFO_REF_TRAP_RSN_UNKNOWN_DA:
					RXINFO_REASON(pRxInfo) = CPU_REASON_UNKNOWN_DA;
					break;
				case RXINFO_REF_TRAP_RSN_ACL_TO_PS:
					RXINFO_REASON(pRxInfo) = CPU_REASON_ACL_TRAP_PS;
					FCMGR_PRK("[ACL] Trap to ps!! spa: 0x%x rule info: 0x%x", RXINFO_SPA(pRxInfo), hdrCPU->mdata_raw.mdata_h&RXINFO_REF_VALID_BIT);
					break;
				default:
					break;
			}
		}else if(hdrCPU->mdata_raw.mdata_h&RXINFO_REF_ACL_RSN_BIT){
			RXINFO_REASON(pRxInfo) = CPU_REASON_ACL_NON_TRAP;
		}
		RXINFO_REFIDX(pRxInfo) = hdrCPU->mdata_raw.mdata_h&RXINFO_REF_VALID_BIT;
	}
#endif

	if(RXINFO_REASON(pRxInfo)==CPU_REASON_L34_FWD && copyToCPU==FALSE       && RXINFO_DST_EXTPIDX(pRxInfo)!=0 ){
		//FCMGR_PRK("[WiFi FF] skb from dev: %s, spa: 0x%x", skb->dev?skb->dev->name:"NULL", RXINFO_SPA(pRxInfo));
		ret = rtk_fc_fastfwd_directXmit(skb, pRxInfo);

		return RTK_FC_NIC_RX_STOP_SKBNOFREERE; 
			
	}
	else if(RXINFO_SPA(pRxInfo) == RTK_FC_MAC_PORT_PON){
#if defined(CONFIG_FC_RTL8277C_SERIES)
		/*
			OVER MTU:
			from PON to wifi and has no pol1 action: enable MTU check, recover PON RX streamID from pol1_idx if MTU check failed.
			from PON to wifi and has pol1 action:    disable MTU check due to PON RX streamID can not be recovered.
			=====
			TTL CHECK FAIL:
			for single header and dual outer:
				check IP TTL by CLS
			for dual inner:
				from PON to wifi and has no pol1 action: enable MTU check, recover PON RX streamID from pol1_idx if MTU check failed.
				from PON to wifi and has pol1 action:    disable MTU check due to PON RX streamID can not be recovered.
		*/
		if(((RXINFO_REASON(pRxInfo) == CPU_REASON_MTU) || (RXINFO_REASON(pRxInfo) == CPU_REASON_TTL)) && (hdrA->bits.bits_32_63.pkt_info.pol_en == 0))
			RXINFO_PON_SID(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id & 0xff;
		else
			RXINFO_PON_SID(pRxInfo) = MDATAL_SWID(hdrCPU->mdata_raw.mdata_l);	//RX stream id from mdata[7:0]
#elif defined(CONFIG_FC_RTL9607F_SERIES)
		RXINFO_PON_SID(pRxInfo) = MDATAL_SWID(hdrCPU->mdata_raw.mdata_l);	//RX stream id from mdata[7:0]
#else
		RXINFO_PON_SID(pRxInfo) = hdrA->bits.bits_32_63.pkt_info.pol_id;
#endif
#if !defined(CONFIG_RTL_ETH_RECYCLED_SKB)
		_rtk_fc_set_streamId_to_skbMark(skb, pRxInfo);
#endif
	}
	else if(RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo))
	{
#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
		unsigned char *pSnap_data=skb->data;
		//if we receive a snap packet, transform it back to ethernet packet here!!
		if(ntohs(*((unsigned char *)pSnap_data+ETH_ALEN+ETH_ALEN))<0x0800){
			pSnap_data+=8;								//DA(6B)+SA(6B)+LEN(2B)+LLC(3B)+SNAP(5B)-DA(6B)-SA(6B)-ETHTYPE(2B)=8
			memmove(pSnap_data,skb->data,ETH_ALEN<<1);	//move DA and SA to ahead of ETHTYPE of SNAP header
			skb_pull(skb,8);
		}
#endif
		FCMGR_PRK("[WiFI Rx] hw lookup miss!! spa: 0x%x wifi_flow_id: %d", RXINFO_SPA(pRxInfo), RXINFO_CPU_EXTSPA(pRxInfo));
	}
#endif

	{
		// SKIP FC
		if(unlikely((1<<fc_mgr_db.hwnat_mode) & HWNAT_MODE_SKIP_FC_FUNC_BITMASK))//Trap to ps mode
			if_skipFcIngressFunc = TRUE;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		else if(RXINFO_REASON(pRxInfo) == CPU_REASON_ACL_TRAP_PS)//Trap to ps by ACL decision
		{
			if_skipFcIngressFunc = TRUE;
			if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_SKIPFCFUNC].startBit != RTK_FC_MGR_RMK_UNDEF)
				if_skipFcFunc_to_skbMARK = TRUE;
		}
#endif
		else {
#if 0
			if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_SKIPFCFUNC].startBit != RTK_FC_MGR_RMK_UNDEF)
				if_skipFcIngressFunc = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_SKIPFCFUNC);
#endif
			if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_SKIPFCFUNC].startBit != RTK_FC_MGR_RMK_UNDEF && skb->cb[1]){
				if_skipFcIngressFunc = if_skipFcFunc_to_skbMARK = TRUE;
			}
		}
		
		if(if_skipFcIngressFunc)
		{
			/*skip FC ingress: not support sw host policing counting*/
			if(unlikely(fc_mgr_db.smpStatistic)){
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC_RX_SKIP_FC].smp_static[smp_processor_id()]);
			}

			//recover wifi netdev
			if(RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo)) {
				if(rtk_fc_wlanDevIdx2dev(RXINFO_CPU_EXTSPA(pRxInfo), &skb->dev)!=SUCCESS) {
					FCMGR_ERR("could not recover wifi dev id %d to correct netdev\n", RXINFO_CPU_EXTSPA(pRxInfo));			
					dev_kfree_skb_any(skb);
					return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
				}
			}
			
			//((not wifi HW lookup miss packet and skipFcFunc is enabled) OR (disable hwnat_mode),
			FCMGR_PRK("[RX] SKB[%p] from port %d with len %d dev: %s -- continue rx to ps)\n", skb, RXINFO_SPA(pRxInfo), skb->len, skb->dev?skb->dev->name:"NULL");

			_rtk_fc_skb_manipulation(pRxInfo, skb);
			if(if_skipFcFunc_to_skbMARK)_rtk_fc_set_skbMark_vlaue(skb, FC_MGR_SKBMARK_SKIPFCFUNC, 1);

			return RTK_FC_NIC_RX_CONTINUE;
		}
	}

	FCMGR_PRK("[RX] SKB[%p] len %d from dev: %s, spa: 0x%x\n", skb, skb->len, skb->dev?skb->dev->name:"NULL", RXINFO_SPA(pRxInfo));

	highpri = (RXINFO_INT_PRI(pRxInfo)>=FC_NIC_RX_PRI_TO_HI_QUEUE) ? TRUE : FALSE;


#if defined(CONFIG_SMP) && defined(CONFIG_RTK_L34_FC_IPI_NIC_RX)
	if(	(highpri && fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_HIGHPRI_NIC_RX].mode==RTK_FC_DISPATCH_MODE_IPI) ||
		(!highpri && fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode==RTK_FC_DISPATCH_MODE_IPI) ||
		( fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode==RTK_FC_DISPATCH_MODE_RPS) ||
		(!highpri && fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_RX].mode==RTK_FC_DISPATCH_MODE_SMP_BY_PORT)

	) 

	{
		rtk_fc_smp_nicRx_private_t smp_nicRx_info;
		smp_nicRx_info.skb = skb;
		smp_nicRx_info.extraInfo = extraInfo;

		memcpy(&(smp_nicRx_info.rxInfo), pRxInfo, sizeof(smp_nicRx_info.rxInfo));
		
		ret = rtk_fc_smp_nic_rx_dispatch(&smp_nicRx_info);

		return ret;
	}
	else
#endif
	{
		_rtk_fc_rx_final_process(pRxInfo, skb, extraInfo);
		return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
	}
	
}

int rtk_fc_skb_tx(struct sk_buff *skb, struct net_device *dev)
{
	//int ret;
	int dpmask = 0;
	struct rt_skbuff rtskb;
	int if_skipFcEgressFunc = FALSE;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	ca_eth_private_t *cep = netdev_priv(dev);
	int dport = cep->port_cfg.tx_ldpid;
#endif

	if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_SKIPFCFUNC].startBit != RTK_FC_MGR_RMK_UNDEF)
		if_skipFcEgressFunc = _rtk_fc_get_skbMark_vlaue(skb, FC_MGR_SKBMARK_SKIPFCFUNC);

	if(unlikely((1<<fc_mgr_db.hwnat_mode) & HWNAT_MODE_SKIP_FC_FUNC_BITMASK))
		if_skipFcEgressFunc = TRUE;
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	dpmask = ((struct re_dev_private*)netdev_priv(dev))->txPortMask;
	if(if_skipFcEgressFunc)
	{
		/*skipFcEgressFunc is TRUE*/
		_rtk_fc_skipFcEgressFunc(skb, dpmask);	
		return 0;
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

	if((dport == AAL_LPORT_L3_LAN) || (dport == AAL_LPORT_L3_WAN))
	{
		// tx to eth0 or nas0 - dirTx to 0x18/0x19 is forbidden
		dev_kfree_skb_any(skb);
		//printk("dirtx to %d is forbidden! drop packet", dport);
		return 0;	//NETDEV_TX_OK
	}
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
	if(skb->fcIngressData.ipsec_skip_kernel && if_skipFcEgressFunc)
	{
		FCMGR_PRK("[IPSEC]skipFcEgressFunc but need to encrypt! Set skipFcEgressFunc to FALSE and doLearning FALSE");
		if_skipFcEgressFunc = FALSE;
		skb->fcIngressData.doLearning = FALSE;
	}
#endif

	if(if_skipFcEgressFunc)
	{
		/*skipFcEgressFunc is TRUE*/
		_rtk_fc_skipFcEgressFunc(skb, dport);
		return 0;
	}
	
	dpmask = (1<<dport);

#endif

	rtk_fc_converter_skb(skb, &rtskb);

	FCMGR_PRK("[TX] SKB[%p] dev %s txpmask 0x%x ", skb, dev->name, dpmask);
		
	return rtk_fc_egress_flowLearning(&rtskb, dev, dpmask, RTK_FC_WLANX_NOT_FOUND);
}

// EXPORT API for local out traffic to PON port
int rtk_fc_nic_tx_with_pon_sid(struct sk_buff *skb, uint32 ponsid)
{
	if(!skb) return FAIL;

	// prepare
	_rtk_fc_set_skbMark_vlaue(skb, FC_MGR_SKBMARK_STREAMID_EN, 1);
	_rtk_fc_set_skbMark_vlaue(skb, FC_MGR_SKBMARK_STREAMID, ponsid);

	// send
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	_rtk_fc_skipFcEgressFunc(skb, (1<<RTK_FC_MAC_PORT_PON));
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	_rtk_fc_skipFcEgressFunc(skb, RTK_FC_MAC_PORT_PON);
#endif

	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int rtk_fc_nic_tx(void *pNicTx_privateInfo_data)
{
	rtk_fc_smp_nicTx_private_t *pNicTx_privateInfo = (rtk_fc_smp_nicTx_private_t *)pNicTx_privateInfo_data;
#if defined(CONFIG_RTL8686NIC)
	fc_tx_info_t *ptxInfo;
#endif
	uint8 force_dirTx;

	force_dirTx = FALSE;
	
#if defined(CONFIG_RTK_FC_PTOOL_CPU_PERF)
	PROFILE_START
#endif


#if defined(CONFIG_RTK_SOC_RTL8198D)
	if (rtk_fc_is_highpri_pkt(pNicTx_privateInfo->skb->data))
		force_dirTx = TRUE;
#endif

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	if(pNicTx_privateInfo->isHwlookup)
		force_dirTx = TRUE;
#endif

#if defined(CONFIG_SMP) && defined(CONFIG_RTK_L34_FC_IPI_NIC_TX)
	if((fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_TX].mode == RTK_FC_DISPATCH_MODE_IPI) && (smp_processor_id()==fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_TX].smp_id))
	{
		//avoid CPU self-ipi to reduce extra enqueue/dequeue/soft-interrupt time
		force_dirTx = TRUE;
	}
#endif

#if defined(CONFIG_SMP) && defined(CONFIG_RTK_L34_FC_IPI_NIC_TX)
	if(unlikely(force_dirTx==FALSE && fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_NIC_TX].mode == RTK_FC_DISPATCH_MODE_IPI
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		&& !pNicTx_privateInfo->isHwlookup
#endif
	))
	{
		//nic tx dispatch mode (dirtx && ipi enabled)
		rtk_fc_smp_nic_tx_dispatch(pNicTx_privateInfo);
	}
	else
#endif
	{
		//nic tx (ipi disabled)
#if defined(CONFIG_RTL8686NIC)

		ptxInfo = &(pNicTx_privateInfo->txInfo);

		re8686_send_with_txInfo(pNicTx_privateInfo->skb, ptxInfo, (FC_NIC_TX_PRI_TO_RING >> (TXINFO_CPUTAG_PRI(ptxInfo)<<2)) & 0xf);
		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+TXINFO_GMAC_ID(ptxInfo)].smp_static[smp_processor_id()]);

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

		ca_ni_tx_config_t tx_config={0};
		if(!pNicTx_privateInfo->isHwlookup)
		{
			//memset(&tx_config, 0, sizeof(tx_config));
			tx_config.flow_id = pNicTx_privateInfo->flow_id;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			tx_config.stream_id_en = pNicTx_privateInfo->stream_id_en;
			tx_config.stream_id = pNicTx_privateInfo->stream_id;
			if(unlikely(pNicTx_privateInfo->force_def_tx_en))
				tx_config.force_def_tx_en = pNicTx_privateInfo->force_def_tx_en;
			if(unlikely(pNicTx_privateInfo->chk_sel_en)){
#if defined(CONFIG_FC_RTL8277C_SERIES)
				tx_config.chk_sel_en = pNicTx_privateInfo->chk_sel_en;
				tx_config.chk_sel = pNicTx_privateInfo->chk_sel;
#endif
			}
#endif
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			tx_config.dma_aft_l2fib_enable = pNicTx_privateInfo->lso_para0.bf.dma_aft_l2fib_enable;
			tx_config.dma_aft_l2fib_index = pNicTx_privateInfo->lso_para0.bf.dma_aft_l2fib_index;
#if defined(CONFIG_FC_RTL9607F_SERIES)
			tx_config.dmaaft_map = pNicTx_privateInfo->dmaaft_map;
#endif
#endif
			memcpy(&(tx_config.core_config), &(pNicTx_privateInfo->core_config), sizeof(tx_config.core_config));
			tx_config.lso_para0.wrd = pNicTx_privateInfo->lso_para0.wrd;

			prefetch(&pNicTx_privateInfo->skb->len);
			//ca_ni_start_xmit_native(pNicTx_privateInfo->skb, pNicTx_privateInfo->dev, &tx_config);
			nic_egress_start_xmit_for_fc_dirTx(pNicTx_privateInfo->skb, pNicTx_privateInfo->skb->dev, &tx_config);
		}
		else
		{
			ni_tx_core_config_t *pTx_core_config = &tx_config.core_config;
					
			{
				pTx_core_config->bf.flow_id_set = TRUE;
				tx_config.flow_id = pNicTx_privateInfo->flow_id;
			}
			tx_config.bypass_lso = TRUE;					// bypass dmalso and header_a if possible.

			FCMGR_PRK("[HWLOOKUP]hwlookup flow_id %s [%d]\n", pTx_core_config->bf.flow_id_set?"set":"ignore", tx_config.flow_id);			

			pTx_core_config->bf.lspid = pNicTx_privateInfo->hwlookup_lspid;// RTK_FC_IPSEC_HWLOOKUP_MACPORT or RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#else // support lspid_map table
			tx_config.lspid_map = pNicTx_privateInfo->hwlookup_lspid_mapIdx;
#endif
			pTx_core_config->bf.ldpid = AAL_LPORT_L3_LAN;
			pTx_core_config->bf.is_from_ca_tx = TRUE;
			pTx_core_config->bf.bypass_fwd_engine =  (pTx_core_config->bf.flow_id_set ? TRUE : FALSE);
			FCMGR_PRK("[HWLOOKUP]hwlookup to %s, flow_id %s [%d] lspid: %d, ldpid: %d\n", pNicTx_privateInfo->skb->dev?pNicTx_privateInfo->skb->dev->name:"NULL", pTx_core_config->bf.flow_id_set?"set":"ignore", tx_config.flow_id, pNicTx_privateInfo->hwlookup_lspid, AAL_LPORT_L3_LAN );

			
			//ca_ni_start_xmit_native(skb, skb->dev, &tx_config);
			nic_egress_start_xmit_for_fc_wifi_hw_lookup(pNicTx_privateInfo->skb, pNicTx_privateInfo->skb->dev, &tx_config);
		}
		
		if(unlikely(fc_mgr_db.smpStatistic))
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_GMAC0_TX+(tx_config.core_config.bf.lspid-RTK_FC_MAC_PORT_CPU)].smp_static[smp_processor_id()]);

#endif
	}

#if defined(CONFIG_RTK_FC_PTOOL_CPU_PERF)
	PROFILE_END
#endif
	return SUCCESS;
}

/*
 * The low level rx handling function of fastforward dev.
 * - the major task is doing hardware lookup if uc packet, ingress (mac) learning if mc/bc packet.
 * - user needs to take care skb-<data, data must start from ethernet header.
 * - user needs to handle return value.
 * - NOTICE: recommend using rtk_fc_fastfwd_netif_rx() and rtk_fc_fastfwd_napi_gro_receive() instead of this one.
*/
int _rtk_fc_skb_wifi_rx(struct sk_buff *skb)
{
	rtk_fc_wlan_devidx_t wlan_dev_idx = RTK_FC_WLANX_NOT_FOUND;
	
	if(unlikely(fc_mgr_db.smpStatistic)){
#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
		if(ntohs(*(unsigned short *)(skb->data+ETH_ALEN+ETH_ALEN))<0x0800)
			atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_WIFI_AMSDU_RX].smp_static[smp_processor_id()]);
		else
#endif
		atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STATIC_WIFI_RX].smp_static[smp_processor_id()]);
		FCMGR_PRK("[RX] SKB[%p] len %d from dev: %s", skb, skb->len, skb->dev ? skb->dev->name : "NULL");
	}
	
	if(unlikely((1<<fc_mgr_db.hwnat_mode) & HWNAT_MODE_SKIP_FC_FUNC_BITMASK)){
		// direct rx to ps
		FCMGR_PRK("bypass - continue rx to ps ... dev: %s", ((skb) && (skb->dev)) ? skb->dev->name : "NULL");
		return RTK_FC_NIC_RX_CONTINUE;	//driver needs to do netif_rx()!!
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(unlikely(fc_mgr_db.wifi_flow_crtl_info.wifi_flow_ctrl_auto_en)){
		if(!strncmp(skb->dev->name, "wlan0", 5)) //packet from wlan0
			fc_mgr_db.wifi_flow_crtl_info.wlan0_accumulate_bit += (((skb->len+4)&0x3fff)<<3);

		else if(!strncmp(skb->dev->name, "wlan1", 5)) //packet from wlan1
			fc_mgr_db.wifi_flow_crtl_info.wlan1_accumulate_bit += (((skb->len+4)&0x3fff)<<3);

	}
#endif
		
	_rtk_fc_dev2wlanDevIdx(skb->dev, &wlan_dev_idx);
	
	if(unlikely(wlan_dev_idx >= RTK_FC_WLANX_END_INTF)) {	
		FCMGR_PRK("unknown wlan dev %s - return rx continue to wifi driver ...", ((skb) && (skb->dev)) ? skb->dev->name : "NULL");
		return RTK_FC_NIC_RX_CONTINUE;
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES) || defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
	// make all wifi packets to do HW lookup to carry HW CRC
#else
	//Use flow-based hw to acclerate unicast and calculate flow hash index when trap
	if(!(skb->data[0]&1))
#endif
	{
		
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if(unlikely(fc_mgr_db.skipHwlookUp_stat.status))
		{
			if((fc_mgr_db.wlanDevMap[wlan_dev_idx].portmap.macPortIdx == fc_mgr_db.skipHwlookUp_stat.portInfo.macPortIdx) && (fc_mgr_db.wlanDevMap[wlan_dev_idx].portmap.macExtPortIdx == fc_mgr_db.skipHwlookUp_stat.portInfo.macExtPortIdx))
			{
				if (time_after(jiffies, fc_mgr_db.skipHwlookUp_stat.last_jiffies + 5 * CONFIG_HZ)) {	// 5s
					FCMGR_DBG("clear skipHwlookUp_stat[%d][%d] for waiting too long, current jiffies = %lu, last_jiffies = %lu",
						fc_mgr_db.skipHwlookUp_stat.portInfo.macPortIdx, fc_mgr_db.skipHwlookUp_stat.portInfo.macExtPortIdx,
						jiffies, fc_mgr_db.skipHwlookUp_stat.last_jiffies);

					rtk_fc_mgr_skipHwlookUp_stat_clear();
				}
				else {
					FCMGR_PRK("skip hw look up due to skipHwlookUp_stat is ON, port %d(%d)", fc_mgr_db.wlanDevMap[wlan_dev_idx].portmap.macPortIdx, fc_mgr_db.wlanDevMap[wlan_dev_idx].portmap.macExtPortIdx);
					if(ntohs(*(unsigned short *)(skb->data + ETH_ALEN + ETH_ALEN)) < 0x0800)
					{
						atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_STA_OFFLINE_AMSDU_DROP].smp_static[smp_processor_id()]);
						return RTK_FC_NIC_RX_STOP;
					}else
						goto FC_INGRESS_LRN;
				}
			}
		}
#endif
		// supported wifi dev! do hw lookup
#if defined(CONFIG_SMP) && defined(CONFIG_RTK_L34_FC_IPI_WIFI_RX)
		if(unlikely(fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_WLAN0_RX+fc_mgr_db.wlanDevMap[wlan_dev_idx].band].mode 
			== RTK_FC_DISPATCH_MODE_IPI)) 
		{
			/*
			 * ipi enabled
			 */ 
			rtk_fc_wlanrx_info_t wlanrxinfo={0};

			wlanrxinfo.skb = skb;
			wlanrxinfo.wlandevidx = wlan_dev_idx;
			
			rtk_fc_smp_wlan_rx_dispatch(&wlanrxinfo);		
		}
		else
#endif
		{
			/*
			 * ipi disabled
			 */ 			
			rtk_fc_wlan_rx_lookup(skb, wlan_dev_idx);
		}

		return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
FC_INGRESS_LRN:
#endif

#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
	return RTK_FC_NIC_WIFI_IGR_LEARNING;
#else
	/*for the packets from wifi and no need to do HW look up (non-unicast),
	  go to FC ingress_flowLearning (for ingress LUT learning)*/
	{
		struct rt_skbuff rtskb;
		int ret;
		rtk_fc_converter_skb(skb, &rtskb);
		ret = rtk_fc_fastfwd_ingress_rcv(&rtskb);
		if(ret == RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS)
			ret = RTK_FC_NIC_RX_CONTINUE; // for wifi RX without HW lookup packets, the ret should not be RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS, for Foolproof here
			
		if(fc_mgr_db._rtk_fc_post_wifi_skb_rx) {
			//fc_rx_info_t rxInfo={0};
			ret = fc_mgr_db._rtk_fc_post_wifi_skb_rx(skb, NULL);
			rtk_fc_skb_rxhook_skb_handling(skb, ret);
			return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
		}
		return ret;
	}
#endif
}

/*
 * FastForward Dev Rx Function
 *  - Instead of netif_rx(), fastforward dev call this FC API to process rx packets.
*/
int _rtk_fc_fastfwd_netif_rx(struct sk_buff *skb)
{
	int ret = 0, status = NET_RX_DROP;

	skb_push(skb, 14);
	ret = rtk_fc_skb_wifi_rx(skb);

#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
	/*for the packets from wifi and no need to do HW look up (non-unicast),
	  go to FC ingress_flowLearning (for ingress LUT learning)*/
	if(ret==RTK_FC_NIC_WIFI_IGR_LEARNING)
	{
		struct rt_skbuff rtskb;
		rtk_fc_converter_skb(skb, &rtskb);
		ret = rtk_fc_fastfwd_ingress_rcv(&rtskb);
		if(ret == RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS)
			ret = RTK_FC_NIC_RX_CONTINUE; // for wifi RX without HW lookup packets, the ret should not be RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS, for Foolproof here
		skb=rtskb.skb;
	}
#endif

	if(ret==RTK_FC_NIC_RX_STOP_SKBNOFREERE)
	{
		status = NET_RX_SUCCESS;
	}
	else if(ret==RTK_FC_NIC_RX_CONTINUE)
	{
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
		if(skb->recyclable) {skb=recycle_skb_swap(skb);if(skb==NULL)return NET_RX_DROP;}
		_rtk_fc_skb_manipulation(NULL, skb);
#endif

		if(fc_mgr_db._rtk_fc_post_wifi_skb_rx) {
			//fc_rx_info_t rxInfo={0};
			ret = fc_mgr_db._rtk_fc_post_wifi_skb_rx(skb, NULL);
			rtk_fc_skb_rxhook_skb_handling(skb, ret);
			return NET_RX_SUCCESS;
		}
		
		rtk_fc_skb_eth_type_trans(skb, skb->dev);
		
		status = netif_rx (skb);
	}
	else if(ret==RTK_FC_NIC_RX_STOP)
	{
		kfree_skb(skb);
		status = NET_RX_DROP;
	}

	return status;
}

/*
 * FastForward Dev Rx Function
 *  - Instead of napi_gro_receive(), fastforward dev call this FC API to process rx packets.
*/
gro_result_t _rtk_fc_fastfwd_napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb)
{
	int ret = 0, status = GRO_DROP;

	skb_push(skb, 14);
	ret = rtk_fc_skb_wifi_rx(skb);

#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
	/*for the packets from wifi and no need to do HW look up (non-unicast),
	  go to FC ingress_flowLearning (for ingress LUT learning)*/
	if(ret==RTK_FC_NIC_WIFI_IGR_LEARNING)
	{
		struct rt_skbuff rtskb;
		rtk_fc_converter_skb(skb, &rtskb);
		ret = rtk_fc_fastfwd_ingress_rcv(&rtskb);
		if(ret == RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS)
			ret = RTK_FC_NIC_RX_CONTINUE; // for wifi RX without HW lookup packets, the ret should not be RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS, for Foolproof here
		skb=rtskb.skb;
	}
#endif

	if(ret==RTK_FC_NIC_RX_STOP_SKBNOFREERE)
	{
		status = NET_RX_SUCCESS;
	}
	else if(ret==RTK_FC_NIC_RX_CONTINUE)
	{
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
		if(skb->recyclable) {skb=recycle_skb_swap(skb);if(skb==NULL)return NET_RX_DROP;}
		_rtk_fc_skb_manipulation(NULL, skb);
#endif

		if(fc_mgr_db._rtk_fc_post_wifi_skb_rx) {
			//fc_rx_info_t rxInfo={0};
			ret = fc_mgr_db._rtk_fc_post_wifi_skb_rx(skb, NULL);
			rtk_fc_skb_rxhook_skb_handling(skb, ret);
			return NET_RX_SUCCESS;
		}
		
		rtk_fc_skb_eth_type_trans(skb, skb->dev);
		
		status = napi_gro_receive(napi, skb);
	}
	else if(ret==RTK_FC_NIC_RX_STOP)
	{
		kfree_skb(skb);
		status = NET_RX_DROP;
	} 

	return status;
}

/*
 * FastForward Dev Tx Function
 *  - when fastfwd dev register to FC driver, dev tx function will be replaced by rtk_fc_fastfwd_dev_xmit().
*/
int rtk_fc_fastfwd_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct rt_skbuff rtskb;
	rtk_fc_wlan_devidx_t wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

	_rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx);
				
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	// for wifi tx, make sure cb is done before FC egress learning.
	if(fc_mgr_db._rtk_fc_wifi_client_mode_cb && 
			wlanDevIdx < RTK_FC_WLANX_END_INTF && fc_mgr_db.wlanDevMap[wlanDevIdx].attr.client_mode) {
		struct sk_buff *newskb = skb_copy(skb, GFP_ATOMIC);
		if(newskb) {
			dev_kfree_skb_any(skb);
			skb = newskb;
			fc_mgr_db._rtk_fc_wifi_client_mode_cb(skb->dev, skb, FALSE);
		}else
			FCMGR_ERR("wifi client interface skb copy fail.");
	}
#endif

	rtk_fc_converter_skb(skb, &rtskb);

	return rtk_fc_fastfwd_egress_xmit(&rtskb, dev, wlanDevIdx);
}

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
extern int rtk_fc_internal_crypto_set_src_desc(unsigned int first_dw, unsigned int second_dw);
extern int rtk_fc_internal_crypto_set_dst_desc(unsigned int first_dw, unsigned int second_dw, unsigned char start, unsigned char end);
extern int rtk_fc_internal_crypto_ps_pop_done(void);
extern int rtk_fc_internal_crypto_send_desc(void);
extern int rtk_fc_internal_crypto_wait_put_desc_done(void);
extern int rtk_fc_internal_crypto_register_hw_crypto_ready_cnt_callback(rtk_fc_crypto_hw_crypto_ready_cnt_callback cb_func);

int _rtk_fc_crypto_set_src_desc(unsigned int first_dw, unsigned int second_dw)
{
	rtk_fc_internal_crypto_set_src_desc(first_dw, second_dw);
	return 0;
}

int _rtk_fc_crypto_set_dst_desc(unsigned int first_dw, unsigned int second_dw, unsigned char start, unsigned char end)
{
	rtk_fc_internal_crypto_set_dst_desc(first_dw, second_dw, start, end);
	return 0;
}

int _rtk_fc_crypto_ps_pop_done(void)
{
	return rtk_fc_internal_crypto_ps_pop_done();
}

int _rtk_fc_crypto_send_desc(void)
{
	return rtk_fc_internal_crypto_send_desc();
}

int _rtk_fc_crypto_wait_put_desc_done(void)
{
	return rtk_fc_internal_crypto_wait_put_desc_done();
}

int _rtk_fc_crypto_register_hw_crypto_ready_cnt_callback(rtk_fc_crypto_hw_crypto_ready_cnt_callback cb_func)
{
	rtk_fc_internal_crypto_register_hw_crypto_ready_cnt_callback(cb_func);
	return 0;
}
#endif

int rtk_fc_trx_init(void)
{
	rtk_fc_export_symbol_t ext_symbol;

	memset(&ext_symbol, 0, sizeof(rtk_fc_export_symbol_t));
	ext_symbol._rtk_fc_wlan_register = _rtk_fc_wlan_register;
	ext_symbol._rtk_fc_wlan_devMask2RtmbssidMask =  _rtk_fc_wlan_devMask2RtmbssidMask;
	ext_symbol._rtk_fc_dev2wlanDevIdx = _rtk_fc_dev2wlanDevIdx;
	ext_symbol._rtk_fc_dev_get_realdev_phyport = _rtk_fc_dev_get_realdev_phyport;
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	ext_symbol._rtk_fc_wifi_amsdu_pe_offload_mac_id_set = _rtk_fc_wifi_amsdu_pe_offload_mac_id_set;
	ext_symbol._rtk_fc_wifi_amsdu_pe_offload_mac_id_del = _rtk_fc_wifi_amsdu_pe_offload_mac_id_del;
	ext_symbol._rtk_fc_wifi_amsdu_pe_offload_mac_id_flush = _rtk_fc_wifi_amsdu_pe_offload_mac_id_flush;
	ext_symbol._rtk_fc_wifi_amsdu_pe_offload_mode_set = _rtk_fc_wifi_amsdu_pe_offload_mode_set;
	ext_symbol._rtk_fc_wifi_amsdu_pe_offload_mode_get = _rtk_fc_wifi_amsdu_pe_offload_mode_get;
	ext_symbol._rtk_fc_wifi_dev_attr_set 				= _rtk_fc_wifi_dev_attr_set;
	ext_symbol._rtk_fc_wifi_dev_attr_get 				= _rtk_fc_wifi_dev_attr_get;
	ext_symbol._rtk_fc_wifi_dev_to_devidx_get 			= _rtk_fc_wifi_dev_to_devidx_get;
	ext_symbol._rtk_fc_wifi_client_mode_cb_register 		= _rtk_fc_wifi_client_mode_cb_register;
#endif

	ext_symbol._rtk_fc_skb_wifi_rx = _rtk_fc_skb_wifi_rx;
	ext_symbol._rtk_fc_fastfwd_netif_rx = _rtk_fc_fastfwd_netif_rx;
	ext_symbol._rtk_fc_fastfwd_napi_gro_receive = _rtk_fc_fastfwd_napi_gro_receive;

	ext_symbol._rtk_fc_igmp_mdb_callback_register = _rtk_fc_igmp_mdb_callback_register;
	ext_symbol._rtk_fc_igmp_mdb_callback_unregister = _rtk_fc_igmp_mdb_callback_unregister;


#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	ext_symbol._rtk_fc_crypto_set_src_desc = _rtk_fc_crypto_set_src_desc;
	ext_symbol._rtk_fc_crypto_set_dst_desc = _rtk_fc_crypto_set_dst_desc;
	ext_symbol._rtk_fc_crypto_ps_pop_done = _rtk_fc_crypto_ps_pop_done;
	ext_symbol._rtk_fc_crypto_send_desc = _rtk_fc_crypto_send_desc;
	ext_symbol._rtk_fc_crypto_wait_put_desc_done = _rtk_fc_crypto_wait_put_desc_done;
	ext_symbol._rtk_fc_crypto_register_hw_crypto_ready_cnt_callback = _rtk_fc_crypto_register_hw_crypto_ready_cnt_callback;
#endif

	rtk_fc_ext_symbol_register(&ext_symbol);
	
	rtk_fc_wlan_init();
	
#if defined(CONFIG_FC_RTL9607C_SERIES)
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_fc_rx_gmac_sel_init();
#endif
#if defined(CONFIG_FC_WIFI_RX_HASH_SUPPORT)
	fc_mgr_db.wifi_rx_hash_en = TRUE;
#endif
#if defined(CONFIG_FC_WIFI_RX_GMAC_AUTO_SEL_SUPPORT)
	fc_mgr_db.wifi_rx_gmac_auto_sel_en = TRUE;
#endif

#endif

#if defined(CONFIG_SMP)
	rtk_fc_smp_trx_init();
#endif

#if defined(FC_SAMPLE_CODE)
	{
		// sample code
		rtk_fc_skb_nic_rxhook_register(rtk_fc_rx_callback_test_NIC);
		rtk_fc_skb_wifi_rxhook_register(rtk_fc_rx_callback_test_WIFI);
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		rtk_fc_wifi_client_mode_cb_register(rtk_fc_wifi_client_mode_test_CB);
#endif

		rtk_fc_reg_lut_notify(rtk_fc_l2_refresh_notifier);
	}
#endif

	return SUCCESS;

}

EXPORT_SYMBOL(rtk_fc_nic_tx_with_pon_sid);
EXPORT_SYMBOL(rtk_fc_skb_nic_rxhook_register);
EXPORT_SYMBOL(rtk_fc_skb_wifi_rxhook_register);

