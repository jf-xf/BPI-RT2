/*
 * Copyright (C) 2016 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#include <linux/module.h>
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <linux/if_pppox.h>
#include <net/ipv6.h>
#include <net/addrconf.h>
#include "../../../../net/bridge/br_private.h"

#ifdef CONFIG_RTK_L34_XPON_PLATFORM
#include <common/rt_error.h>
#include <bspchip.h> // for smp_affinity setting
#endif

#include "rtk_fc_struct.h"
#include "rtk_fc_callback.h"
#include "rtk_fc_define.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_driver.h"
#include "rtk_fc_proc.h"
#include "rtk_fc_dualHeader.h"
#include "rtk_fc_acl.h"
#include "rtk_fc_multicast.h"
#include "rtk_fc_mappingAPI.h"
#include "rtk_fc_external.h"
#include "rtk_fc_mgr.h"
#include "rtk_fc_helper.h"

#include <rtk/acl.h>
#include <rtk/cpu.h>
#include "rtk_fc_vxlan.h"
#include "rtk_fc_ipsec.h"



#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include "rtk/l2.h"
#if defined(CONFIG_FC_CA8277AB_SERIES)
#include "rtk_rg_g3_internal.h"
#endif
#include <rtk_fc_asic_common.h>

#include <ca_ni.h>

#include <aal_l3_te_cb.h>

#if defined(CONFIG_REALTEK_IPC2RCPU)
#include "ca_ipc_pe.h"
#endif

#endif

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

#include <net/xfrm.h>
#include <crypto/aead.h>
#include <crypto/internal/geniv.h>

extern rtk_fc_ipsec_saInfo_ops_t ipsec_kernel_hook_func;
		
void *rtk_fc_cryptoEngine_baseaddr = NULL;
EXPORT_SYMBOL(rtk_fc_cryptoEngine_baseaddr);



#endif
char *dualHdrType_name[]={
	"NONE",
	"DSLITE",
	"PPTP",
	"L2TP",
	"IPSEC",
	"GRE_ETH_BR",
	"VXLAN",
	"6RD",
	"MAPT",
	"END",
	
};


extern char *name_of_dualHdrType[];

static const char *const rtk_fc_fwd_type_name[RTK_FC_FWDTYPE_END] = {
	[RTK_FC_FWDTYPE_BRIDGE] = "bridge",
	[RTK_FC_FWDTYPE_ROUTING] = "routing",
	[RTK_FC_FWDTYPE_NAPT] = "napt",
	[RTK_FC_FWDTYPE_NATLOOPBACK] = "natloopback",
};

#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
void _rtk_fc_pe_http_test_free_resource(void)
{
	int32 i;

	//del cls
	for(i=0; i<MAX_PE_HTTP_CONNECTION_NUM; i++)
	{
		if(fc_db.http_test_cls_idx[i] != FAIL)
		{
			if(i==0)
				EVENT("[HTTP Test] set http_test_cls_idx to invalid");
			_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE, &fc_db.http_test_cls_idx[i]);
			fc_db.http_test_cls_idx[i] = FAIL;
		}
	}
	
	//free buffer
	if(fc_db.pe_http_pageInfo.pPages)
	{
		EVENT("[HTTP Test] Free http page");
		_rtk_fc_page_free(&fc_db.pe_http_pageInfo);
	}

	//del dma aft
	if(fc_db.http_test_dma_aft_idx!=FAIL && fc_db.http_test_dma_aft_idx<RTK_FC_TABLESIZE_DMAAFTACTION)
	{
		_rtk_fc_dmaAftAction_del(fc_db.http_test_dma_aft_idx);
		EVENT("[HTTP Test] set dma aft idx %d to invalid", fc_db.http_test_dma_aft_idx);
		fc_db.http_test_dma_aft_idx = FAIL;
	}

#if defined(RT_PE_HTTP_TEST_DMA_LSO_BACKPRESSURE)
	if(fc_db.http_test_dma_lso_regValue[0] != FAIL)
	{
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
		//dma_lso backpressure
		rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[0], DMA_SEC_DMA_LSO_VP_CONTROL + (DMA_SEC_DMA_LSO_VP_STRIDE * RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID));
		rtk_ne_reg_write(fc_db.http_test_dma_lso_regValue[1], TE_CB_WIFI_SSID_MAP);
		rtk_ne_reg_write(fc_db.http_test_dma_lso_regValue[2], TE_CB_WIFI_CTRL);
		rtk_ne_reg_write(fc_db.http_test_dma_lso_regValue[3], TE_CB_WIFI_BUF_THRSH);
#else	//CONFIG_FC_RTL9607F_SERIES

		EVENT("Del dmalso backpressure ... rule_idx=%u", fc_db.http_test_dma_lso_regValue[0]);
		rtk_asic_dmalso_backpressure_rule_del(fc_db.http_test_dma_lso_regValue[0]);
#endif		
		EVENT("[HTTP Test] set dma lso backpressure to invalid");
		fc_db.http_test_dma_lso_regValue[0] = FAIL;
	}
#endif		
	if(fc_db.http_test_dma_lso_regValue[4] != FAIL)
	{
		//recover dma lso settings
		rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[4], DMA_SEC_SS_CTRL);
		rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[5], DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA1);
		rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[6], DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA0);
		//rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[7], DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		rtk_dma_lso_reg_write(0xc0000000|RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		//recover dma lso axi
		rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[8], DMA_SEC_DMA_GLB_DMA_LSO_AXI_USER_SEL0);
		rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[9], DMA_SEC_DMA_GLB_AXI_USER_PAT3);

		EVENT("[HTTP Test] set dma lso shaper to invalid");
		fc_db.http_test_dma_lso_regValue[4] = FAIL;
	}
}

static int rtk_ipc_cb_http_test_handler(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id, const void *msg_data, ca_uint16_t *msg_size)
{
	//EVENT("msg_no %d trans_id %d msg_size %d\n", msg_no, trans_id, *msg_size);
	//EVENT("session_id %d cpu_id %d \n\t", peer.session_id, peer.cpu_id);

	switch(msg_no)
	{
		case RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH:
		case RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_FINISH:
		case RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_FINISH:		
			if(*msg_size != sizeof(rtk_http_test_request_t))
			{
				WARNING("[HTTP handler][msg_no %d] msg size is not matched, remote msg_size=%d but local msg_size=%d", msg_no, *msg_size, (uint32)sizeof(rtk_http_test_request_t));
			}
			//else
			{
				rtk_http_test_request_t *pReq_data = (rtk_http_test_request_t *)msg_data;

				EVENT("========== Finish http %s test ========== ", (msg_no==RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH) ? "download" : "upload");
				EVENT("connection_number %u ", pReq_data->connection_number);
				EVENT("client_mac %pM ", pReq_data->client_mac.octet);
				EVENT("server_mac %pM ", pReq_data->server_mac.octet);
				if(pReq_data->isIPv4OrIpv6==0)
				{
					EVENT("client_ipv4 %pI4h ", &pReq_data->client_ip.ipv4_addr);
					EVENT("server_ipv4 %pI4h ", &pReq_data->server_ip.ipv4_addr);
				}
				else
				{
					EVENT("client_ipv6 %pI6c ", &pReq_data->client_ip.ipv6_addr);
					EVENT("server_ipv6 %pI6c ", &pReq_data->server_ip.ipv6_addr);
				}
				EVENT("client_l4port %u(0x%x) ", pReq_data->client_l4port, pReq_data->client_l4port);
				EVENT("server_l4port %u(0x%x) ", pReq_data->server_l4port, pReq_data->server_l4port);
				EVENT("ldpid %u ", pReq_data->ldpid);
				EVENT("dma_aft_idx %d ", pReq_data->dma_aft_idx);	
				EVENT("[PON] sid %d ", pReq_data->streamId.sid);
				EVENT("[PON] ldpid %d cos %d flowId %d ", pReq_data->streamId.ldpid, pReq_data->streamId.cos, pReq_data->streamId.flowid);
				EVENT("http_req_url %s ", pReq_data->http_req_url);
				EVENT("http_version %s ", pReq_data->http_version);
				EVENT("tcp_window_size %u", pReq_data->tcp_window_size);
				EVENT("tcp_mss_size %u", pReq_data->tcp_mss_size);
				if(msg_no==RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH)
					EVENT("non_congestion_mode %u", pReq_data->non_congestion_mode);
				if(msg_no==RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_FINISH)
					EVENT("upload_content_length %llu", pReq_data->upload_content_length);
				EVENT("buf_phy_addr 0x%x", pReq_data->buf_phy_addr);
				EVENT("buf_size %u(0x%x)", pReq_data->buf_size, pReq_data->buf_size);
				
				_rtk_fc_pe_http_test_free_resource();
			}		
			break;
		case RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_CNT:
		case RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_CNT:
		case RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_RESULT:
			if(*msg_size != sizeof(rtk_http_test_result_t))
			{
				WARNING("[HTTP handler][msg_no %d] msg size is not matched, remote msg_size=%d but local msg_size=%d", msg_no, *msg_size, (uint32)sizeof(rtk_http_test_result_t));
			}
			//else if(msg_no==RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_RESULT)
			if(msg_no==RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_RESULT)
			{
				rtk_http_test_result_t *pResult = (rtk_http_test_result_t *)msg_data;

				EVENT("========== Result of http latency test ========== ");
				EVENT("Latency %u.%03u ms ", (uint32)(pResult->latency_us/RT_PE_USEC_PER_MSEC),  (uint32)(pResult->latency_us%RT_PE_USEC_PER_MSEC));
				EVENT("Jitter %u.%03u ms ", (uint32)(pResult->jitter_us/RT_PE_USEC_PER_MSEC),  (uint32)(pResult->jitter_us%RT_PE_USEC_PER_MSEC));
				EVENT("pktLoss %u/%u ", pResult->pktLoss, (pResult->pktLoss+pResult->pktRcv));
			}
			else
			{
				rtk_http_test_result_t *pResult = (rtk_http_test_result_t *)msg_data;
				uint64 avg_speed_Mbps, duration_milliseconds=pResult->HTTP_EOMTime_ms-pResult->HTTP_BOMTime_ms;

				EVENT("========== Result of http %s test ========== ", (msg_no==RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH) ? "download" : "upload" );	
				EVENT("TCP_openRequestTime	%llu ms ", pResult->TCP_openRequestTime_ms);
				EVENT("TCP_openResponseTime	%llu ms ", pResult->TCP_openResponseTime_ms);
				EVENT("HTTP_ROMTime %llu ms ", pResult->HTTP_ROMTime_ms);
				EVENT("HTTP_BOMTime %llu ms ", pResult->HTTP_BOMTime_ms);
				EVENT("HTTP_EOMTime %llu ms ", pResult->HTTP_EOMTime_ms);
				EVENT("Duration(HTTP_EOMTime - HTTP_BOMTime) %llu ms ", duration_milliseconds);
				EVENT("================ Test statistic ===============");
				EVENT("PktCnt %u ", pResult->pktCnt);
				EVENT("ByteCnt(payload only) %llu ", pResult->byteCnt);
				EVENT("ByteCnt(whole packet) %llu ", pResult->byteCnt_include_pktHdr);
				avg_speed_Mbps = (pResult->byteCnt*8*1000/duration_milliseconds)/1000000;
				EVENT("Average speed(payload only) %u.%03u Gpbs ", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
				avg_speed_Mbps = (pResult->byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
				EVENT("Average speed(whole packet) %u.%03u Gpbs ", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
				EVENT("================ Total statistic ===============");
				EVENT("PktCnt %u ", pResult->total_pktCnt);
				EVENT("ByteCnt(payload only) %llu ", pResult->total_byteCnt);
				EVENT("ByteCnt(whole packet) %llu ", pResult->total_byteCnt_include_pktHdr);
				avg_speed_Mbps = (pResult->total_byteCnt*8*1000/duration_milliseconds)/1000000;
				EVENT("Average speed(payload only) %u.%03u Gpbs ", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
				avg_speed_Mbps = (pResult->total_byteCnt_include_pktHdr*8*1000/duration_milliseconds)/1000000;
				EVENT("Average speed(whole packet) %u.%03u Gpbs ", (uint32)(avg_speed_Mbps/1000), (uint32)(avg_speed_Mbps%1000));
			}
			break;		
		default:
			WARNING("[HTTP handler] unknown msg_no %d", msg_no);
			break;
	}	
	
    return 0;
}

static ca_ipc_msg_handle_t http_test_ipc_msg[RT_PE_HTTP_TEST_RETURN_CMD_MAX] = {
	{ .msg_no = RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH, 	.proc = rtk_ipc_cb_http_test_handler },
	{ .msg_no = RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_CNT, 		.proc = rtk_ipc_cb_http_test_handler },
	{ .msg_no = RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_FINISH, 		.proc = rtk_ipc_cb_http_test_handler },
	{ .msg_no = RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_CNT, 			.proc = rtk_ipc_cb_http_test_handler },
	{ .msg_no = RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_FINISH, 		.proc = rtk_ipc_cb_http_test_handler },
	{ .msg_no = RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_RESULT, 		.proc = rtk_ipc_cb_http_test_handler },
};
#endif

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
int rtk_fc_internal_crypto_set_src_desc(unsigned int first_dw, unsigned int second_dw)
{
	fc_db.pe_crypto_ps_desc_info.crypto_desc_array[fc_db.pe_crypto_ps_desc_info.crypto_array_idx].first_dw = first_dw;
	fc_db.pe_crypto_ps_desc_info.crypto_desc_array[fc_db.pe_crypto_ps_desc_info.crypto_array_idx].second_dw = second_dw;
	fc_db.pe_crypto_ps_desc_info.crypto_array_idx++;

	return 0;
}

int rtk_fc_internal_crypto_set_dst_desc(unsigned int first_dw, unsigned int second_dw, unsigned char start, unsigned char end)
{
	if (start) {
		fc_db.pe_crypto_ps_desc_info.crypto_array_idx = 0;
	}

	fc_db.pe_crypto_ps_desc_info.crypto_desc_array[fc_db.pe_crypto_ps_desc_info.crypto_array_idx].first_dw = first_dw;
	fc_db.pe_crypto_ps_desc_info.crypto_desc_array[fc_db.pe_crypto_ps_desc_info.crypto_array_idx].second_dw = second_dw;
	fc_db.pe_crypto_ps_desc_info.crypto_array_idx++;

	if (end) {
		fc_db.pe_crypto_ps_desc_info.crypto_desc_array[fc_db.pe_crypto_ps_desc_info.crypto_array_idx].first_dw = 0;
		fc_db.pe_crypto_ps_desc_info.crypto_desc_array[fc_db.pe_crypto_ps_desc_info.crypto_array_idx].second_dw = 0;
		fc_db.pe_crypto_ps_desc_info.crypto_array_idx++;
	}

	return 0;
}

void rtk_fc_internal_init_ipsec_ps_api(void)
{
	dma_addr_t paddr;

	memset(&fc_db.pe_crypto_ps_desc_info, 0, sizeof(rtk_pe_crypto_ps_desc_info_t));
	fc_db.pe_crypto_ps_desc_info.pWait_desc_done = (uint32 *)dma_alloc_coherent(&(ni_info_data.pdev->dev), (sizeof(uint32_t)), &paddr, GFP_KERNEL | GFP_DMA);
	if(fc_db.pe_crypto_ps_desc_info.pWait_desc_done)
	{
		fc_db.pe_crypto_ps_desc_info.wait_desc_done_phy_addr = (uint32_t)paddr;
		*(volatile uint32 *)fc_db.pe_crypto_ps_desc_info.pWait_desc_done = 0;
		printk("[IPSEC PS API] wait_desc_done %u(%p) wait_desc_done_phy_addr 0x%x \n", *fc_db.pe_crypto_ps_desc_info.pWait_desc_done, fc_db.pe_crypto_ps_desc_info.pWait_desc_done, fc_db.pe_crypto_ps_desc_info.wait_desc_done_phy_addr);
	}
	else
		WARNING("[IPSEC PS API] Fail to allocate crypto share memory for wait_desc_done");
	fc_db.pe_crypto_ps_desc_info.pWait_pop_done = (uint32 *)dma_alloc_coherent(&(ni_info_data.pdev->dev), (sizeof(uint32_t)), &paddr, GFP_KERNEL | GFP_DMA);	
	if(fc_db.pe_crypto_ps_desc_info.pWait_pop_done)
	{
		fc_db.pe_crypto_ps_desc_info.wait_pop_done_phy_addr = (uint32_t)paddr;
		*(volatile uint32 *)fc_db.pe_crypto_ps_desc_info.pWait_pop_done = 0;
		printk("[IPSEC PS API] wait_pop_done %u(%p) wait_pop_done_phy_addr 0x%x \n", *fc_db.pe_crypto_ps_desc_info.pWait_pop_done, fc_db.pe_crypto_ps_desc_info.pWait_pop_done, fc_db.pe_crypto_ps_desc_info.wait_pop_done_phy_addr);
	}
	else
		WARNING("[IPSEC PS API] Fail to allocate crypto share memory for wait_pop_done");

	return;
}

int rtk_fc_internal_crypto_ps_pop_done(void)
{
	*(volatile uint32 *)fc_db.pe_crypto_ps_desc_info.pWait_pop_done = 0;
	return 0;
}

int rtk_fc_internal_crypto_wait_put_desc_done(void)
{
	while(1)
	{
		if(*(volatile uint32 *)fc_db.pe_crypto_ps_desc_info.pWait_desc_done == 0)
			break;		
	}
	return 0;
}

int rtk_fc_internal_crypto_send_desc(void)
{
	unsigned int ret = 0;
	ca_ipc_pkt_t ipc_pkt;

	if (fc_db.pe_crypto_ps_desc_info.crypto_array_idx > RTK_FC_CRYPTO_MAX_IPC_DESC_NUM) {
		printk("excess CRYPTO_MAX_IPC_DESC_NUM %d\n", RTK_FC_CRYPTO_MAX_IPC_DESC_NUM);
		fc_db.pe_crypto_ps_desc_info.crypto_array_idx = 0;
		return -1;
	}

	*(volatile uint32 *)fc_db.pe_crypto_ps_desc_info.pWait_desc_done = 1;
	
	ipc_pkt.session_id = CA_IPC_SESSION_CRYPTO_ENGINE;
	ipc_pkt.dst_cpu_id = fc_db.controlFuc.func_on_pe_cpu_num[RT_PE_FUNC_NUM_CRYPTO] + CA_IPC_CPU_PE0;
	ipc_pkt.priority = 0;
	ipc_pkt.msg_no = RT_PE_CRYPTO_ENGINE_CMD_PS_SEND_DESC;
	ipc_pkt.msg_data = &fc_db.pe_crypto_ps_desc_info.crypto_desc_array[0];
	ipc_pkt.msg_size = sizeof(rtk_fc_crypto_desc) * fc_db.pe_crypto_ps_desc_info.crypto_array_idx;
	ret = ca_ipc_msg_async_send(&ipc_pkt);
	if (ret != CA_E_OK) {
		*(volatile uint32 *)fc_db.pe_crypto_ps_desc_info.pWait_desc_done = 0;
		return -1;
	}
	
	//push done then clear fc_db.pe_crypto_ps_desc_info.crypto_array_idx
	fc_db.pe_crypto_ps_desc_info.crypto_array_idx = 0;

	return 0;
}

int rtk_fc_internal_crypto_register_hw_crypto_ready_cnt_callback(rtk_fc_crypto_hw_crypto_ready_cnt_callback cb_func)
{
	fc_db.pe_crypto_ps_desc_info.crypto_hw_crypto_ready_cnt_cb_func = cb_func;
	return 0;
}
EXPORT_SYMBOL(rtk_fc_internal_crypto_set_src_desc);
EXPORT_SYMBOL(rtk_fc_internal_crypto_set_dst_desc);
EXPORT_SYMBOL(rtk_fc_internal_crypto_ps_pop_done);
EXPORT_SYMBOL(rtk_fc_internal_crypto_send_desc);
EXPORT_SYMBOL(rtk_fc_internal_crypto_wait_put_desc_done);
EXPORT_SYMBOL(rtk_fc_internal_crypto_register_hw_crypto_ready_cnt_callback);

static int rtk_ipc_cb_crypto_handler(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id, const void *msg_data, ca_uint16_t *msg_size)
{
	unsigned int hw_ok_cnt = 0;

	switch (msg_no)
	{
		case RT_PE_CRYPTO_ENGINE_RETURN_CMD_FINISH:
			if(*msg_size != sizeof(rt_pe_crypto_request_t))
			{
				WARNING("[PE CRYPTO handler][msg_no %d] msg size is not matched, remote msg_size=%d but local msg_size=%d", msg_no, *msg_size, (uint32)sizeof(rt_pe_crypto_request_t));
			}
			{
				rt_pe_crypto_request_t *pReq_data = (rt_pe_crypto_request_t *)msg_data;
				EVENT("buf_phy_addr 0x%x", pReq_data->buf_phy_addr);
				EVENT("buf_size %u(0x%x)", pReq_data->buf_size, pReq_data->buf_size);
				//free buffer
				if(fc_db.pe_crypto_pageInfo.pPages)
					_rtk_fc_page_free(&fc_db.pe_crypto_pageInfo);
				else
					WARNING("Fail to free page buffer of PE IPSEC ... due to pPages is NULL !!");
			}
			break;
		case RT_PE_CRYPTO_ENGINE_HW_CRYPTO_READY:
			hw_ok_cnt = *(uint32*)msg_data;
			fc_db.pe_crypto_ps_desc_info.crypto_hw_crypto_ready_cnt_cb_func(hw_ok_cnt);
			break;
		default:
			break;
	}
	return 0;
}

static ca_ipc_msg_handle_t crypto_ipc_msg[RT_PE_HTTP_TEST_RETURN_CMD_MAX] = {
	{ .msg_no = RT_PE_CRYPTO_ENGINE_RETURN_CMD_FINISH, 			.proc = rtk_ipc_cb_crypto_handler },
	{ .msg_no = RT_PE_CRYPTO_ENGINE_HW_CRYPTO_READY, 			.proc = rtk_ipc_cb_crypto_handler },
};
#endif

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
#ifndef CONFIG_RTK_PE_STATIC_BUFFER_RESERVE
static unsigned int RT_PE_AMSDU_OFLD_PAGE_ORDER[RT_PE_AMSDU_OFLD_TOTAL_PAGE_ORDER_NUM] = {10, 9};//4MB, 2MB
#endif
static rt_pe_amsdu_ofld_request_t rt_pe_amsdu_ofld_req[RT_PE_AMSDU_OFLD_BUF_SEC] = {0};

int _rtk_fc_wifi_amsdu_pe_offload_page_alloc(void) 
{
	unsigned int i = 0;
#ifndef CONFIG_RTK_PE_STATIC_BUFFER_RESERVE
	rtk_fc_page_t page_s; 
#endif
	
	for (i = 0; i < RT_PE_AMSDU_OFLD_BUF_SEC; i++)
	{
#ifndef CONFIG_RTK_PE_STATIC_BUFFER_RESERVE
		page_s.order = RT_PE_AMSDU_OFLD_PAGE_ORDER[i];

		if(_rtk_fc_page_alloc(&page_s) != RTK_FC_RET_OK)
		{
			WARNING("[PE AMSDU Page Alloc SEC %u] Fail to allocate page buffer !\n", i);
			return RT_PE_RET_FAIL;
		}

		rt_pe_amsdu_ofld_req[i].buf_phy_addr = page_s.buf_phy_addr;
		rt_pe_amsdu_ofld_req[i].buf_size = page_s.buf_size;
#else
		rt_pe_amsdu_ofld_req[i].buf_phy_addr = 0;
		rt_pe_amsdu_ofld_req[i].buf_size = 0;
#endif
		printk("[PE AMSDU Page Alloc SEC %u] buf_phy_addr %x\n", i, rt_pe_amsdu_ofld_req[i].buf_phy_addr);
		printk("[PE AMSDU Page Alloc SEC %u] buf_size %x\n", i, rt_pe_amsdu_ofld_req[i].buf_size);
	}
	return RT_PE_RET_OK;
}

static int rtk_ipc_cb_wfo_set_para(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id, const void *msg_data, ca_uint16_t *msg_size)
{
	unsigned int i = 0;
	rt_pe_amsdu_ofld_request_t *req_p = NULL;

	switch (msg_no) 
	{
		case RT_PE_AMSDU_OFLD_CMD_GET_PAGE:
			req_p = (rt_pe_amsdu_ofld_request_t *)msg_data;
			for (i = 0; i < RT_PE_AMSDU_OFLD_BUF_SEC; i++)
			{
				req_p[i].buf_phy_addr = rt_pe_amsdu_ofld_req[i].buf_phy_addr;
				req_p[i].buf_size = rt_pe_amsdu_ofld_req[i].buf_size;
			}
			break;
	}
	printk("get msg no %d\n", msg_no);
	return 0;
}

static ca_ipc_msg_handle_t rtk_wfo_msg[] = {
	{.msg_no = 0, .proc = rtk_ipc_cb_wfo_set_para},
};
#endif

#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_SMP)
static cpumask_t cpumask_gmac0 = {{0}};
static cpumask_t cpumask_gmac1 = {{0}};
static cpumask_t cpumask_gmac2 = {{0}};
#endif

__SRAM_FC_DATA rtk_fc_globalDatabase_t fc_db;
#if defined(CONFIG_FC_RTL9607C_SERIES)
extern uint32 dynamic_sram_desc;
rtk_fc_special_fastFwd_data_t fc_db_fastFwd_data;
#endif
rtk_fc_pathDecision_global_var_t pathDecision_var;


static int _rtk_fc_ingress_flowLearning(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo);
static int _rtk_fc_egress_flowLearning(struct rt_skbuff * rtskb, struct net_device * dev, int portMask, rtk_fc_wlan_devidx_t egrWlanDevIdx);

rtk_fc_api_module_t *RTK_FC_API_MODULE=NULL;
static int RTK_FC_TIMER_SELETC=0;
rtk_fc_timer_list_t *rtkFlowbaseHwnatHouseKeepingTimer=NULL;

fc_l2_refresh_notify rtk_fc_notify_lut_refresh=NULL;
void rtk_fc_reg_lut_notify(fc_l2_refresh_notify notify_func)
{
	rtk_fc_notify_lut_refresh = notify_func;
	return;
}

#define MODINITPRINT(comment ,arg...) \
do {\
	if(fc_db.controlFuc.module_probe_log) \
	printk(comment ,## arg);\
} while(0)

#ifdef __KERNEL__
static intrBcasterNotifier_t linkChangeNotifier = {
    .notifyType = MSG_TYPE_LINK_CHANGE,
    .notifierCb = rtk_fc_switchLinkChangeHandler,
};
#endif


static struct notifier_block rtk_fc_netif_inetaddr_notifier = {
	.notifier_call = rtk_fc_netif_inetaddr_event,
};

static struct notifier_block rtk_fc_netif_inet6addr_notifier = {
	.notifier_call = rtk_fc_netif_inet6addr_event,
};


static struct notifier_block rtk_fc_netif_netdev_notifier = {
	.notifier_call = rtk_fc_netif_netdev_event,
};

int rtk_fc_netif_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	if(RTK_FC_API_MODULE->rtk_fc_api_Netif_inetAddrEvent){
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		RTK_FC_API_MODULE->rtk_fc_api_Netif_inetAddrEvent(ptr, event);
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	}

	return NOTIFY_DONE;
}

int rtk_fc_netif_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	if(RTK_FC_API_MODULE->rtk_fc_api_Netif_inet6AddrEvent){
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		RTK_FC_API_MODULE->rtk_fc_api_Netif_inet6AddrEvent(ptr, event);
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	}

	return NOTIFY_DONE;
}


int rtk_fc_netif_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{

	if(RTK_FC_API_MODULE->rtk_fc_api_Netif_netDevEvent){
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		RTK_FC_API_MODULE->rtk_fc_api_Netif_netDevEvent(ptr, event);
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();

	}

	return NOTIFY_DONE;
}

void rtk_fc_houseKeepingTimerFunc(unsigned long task_priv)
{
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	//========================= Critical Section Start =========================//
	task_priv = RTK_FC_HELPER_TIMER_DATA_GET(task_priv);
	//TIMER("fc house keeping periodTimer TICK");
	if(RTK_FC_API_MODULE==NULL)
		goto EXIT;

	if((1<<fc_db.controlFuc.hwnat_mode) & HWNAT_MODE_SKIP_TIMER_BITMASK) 
		goto EXIT;

	switch(RTK_FC_TIMER_SELETC)
	{

		case RTK_FC_HOUSEKEEP_LUT_SELECT:
			_rtk_fc_lut_timeoutCheck();
			break;
		case RTK_FC_HOUSEKEEP_FLOW_SELECT:
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			_rtk_fc_mainHashFlowTrf_update();
#endif
			_rtk_fc_flow_timeoutCheck();
			break;
		case RTK_FC_HOUSEKEEP_DRV_EVENT_REC:
			_rtk_fc_drv_event_record_timeoutCheck();
			break;
		default:
			break;
	}

	if(RTK_FC_HOUSEKEEP_MAX_SELECT>0)
		RTK_FC_TIMER_SELETC = ((RTK_FC_TIMER_SELETC+1)%RTK_FC_HOUSEKEEP_MAX_SELECT);
EXIT:
	
	RTK_FC_HELPER_MOD_TIMER(rtkFlowbaseHwnatHouseKeepingTimer, jiffies + ((fc_db.flowSyncPeriod_unit1s*CONFIG_HZ)/(RTK_FC_HOUSEKEEP_MAX_SELECT*fc_db.flowSync_groupCnt)));

	//========================= Critical Section End =========================//
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();

	return;
}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int rtk_fc_mapet_hwfmr_add(rtk_fc_mapet_hwfmrInfo_t *fmrhwInfo)
{
	l3pe_dual_FMR_tbl_entry_t entry,getEntry;
	int i,j,aal_ret;
	int first_invalid=FAIL,extistEntryIdx=FAIL;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	int netifSwIdx=FAIL;
	int devGwMacIdx=FAIL;
	int counter;

	
	//error check
	if(fmrhwInfo->fmrPrefixLen > 128)
		return FAILED;
	if(fmrhwInfo->psidStartOffset > fmrhwInfo->psidEndOffset)
		return FAILED;


	for(i=DEVIFIDX_VALID_MIN ;i <RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev==NULL)
			continue;		
		if(strcmp(fc_db.devGwMacTbl[i].dev->name,fmrhwInfo->devName)==0)
		{
			devGwMacIdx=i;
			break;
		}
	}

	if(devGwMacIdx==FAIL)
		{WARNING("netif %s not found ",fmrhwInfo->devName); return FAIL;}

	for(i = 0; i < RTK_FC_TABLESIZE_INTF_SW; i++)
	{
		if(fc_db.netifTbl[i].intf.valid==0)
			continue;
		counter=rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[i].psIfidxStackKey,p_PScacheDev);

		for(j=0;j<counter;j++)
		{
			if(strcmp(p_PScacheDev[j]->name,fmrhwInfo->devName)==0 )
			{
				netifSwIdx = i;
				goto HITNETIF_NAME;
			}
		}
	}

HITNETIF_NAME:


	memset(&entry,0,sizeof(entry));
	memcpy(&entry.fmr_pre_addr,fmrhwInfo->fmrv6Prefix,sizeof(entry.fmr_pre_addr));
	entry.fmr_pref_len = fmrhwInfo->fmrPrefixLen;
	
	if(fmrhwInfo->draftVersion)
		entry.fmr_drf_ver = 1;

	if(fmrhwInfo->psidStartOffset == fmrhwInfo->psidEndOffset)
	{
		entry.fmr_psid_refill	=0;
		entry.fmr_psid_end_ost	=0;
		entry.fmr_psid_str_ost	=0;
	}
	else
	{
		entry.fmr_psid_refill=1;
		entry.fmr_psid_str_ost = fmrhwInfo->psidStartOffset;
		entry.fmr_psid_end_ost = fmrhwInfo->psidEndOffset;
	}
	
	for(i=0;i<L3PE_DUAL_FMR_TBL_ENTRY_MAX;i++)
	{
		memset(&getEntry,0,sizeof(getEntry));
		aal_ret=rtk_rg_l3_pe_dual_fmr_get(i, &getEntry);
	
		if(((aal_ret == ASIC_RET_ENTRYNOTRSVD) || (aal_ret == ASIC_RET_NOT_FOUND) || (aal_ret == ASIC_RET_NULL_POINTER)) && first_invalid<0){
			first_invalid=i;
			continue;
		}
		else
		{
			//TABLE("ret=%d",aal_ret);
			//TABLE("[%d] fmr_pref_len=%d fmr_psid_end_ost=%d fmr_psid_str_ost=%d",i,getEntry.fmr_pref_len,getEntry.fmr_psid_end_ost,getEntry.fmr_psid_str_ost);
			//TABLE(" 	fmr_pref_len=%d fmr_psid_end_ost=%d fmr_psid_str_ost=%d memcmp:=%d",entry.fmr_pref_len,entry.fmr_psid_end_ost,entry.fmr_psid_str_ost,(memcmp(&getEntry,&entry,sizeof(l3pe_dual_FMR_tbl_entry_t)))); 	
			if(!(memcmp(&getEntry,&entry,sizeof(l3pe_dual_FMR_tbl_entry_t))))	
			{
				TRACE("using same FMR Table[%d]",i);
				extistEntryIdx=i;
				//compare with exist FMR, if match perfectly, use it
				break;
			}
		}
	}

	if(extistEntryIdx!=FAIL)
	{
		//entry already in hardware
		fmrhwInfo->hwFmrIdx=extistEntryIdx;
		fc_db.devGwMacTbl[devGwMacIdx].mapet_fmr_hwidx = extistEntryIdx;
		return SUCCESS;
	}
	else if (first_invalid==FAIL)
	{
		//no more invalid entry 
		fmrhwInfo->hwFmrIdx=FAIL;
		TABLE("no more invalid entry ");
		return FAILED;
	}
	else
	{
		//add a new entry
		//TABLE("fmr_psid_end_ost=%d fmr_psid_str_ost=%d ",entry.fmr_psid_end_ost,entry.fmr_psid_str_ost);
		aal_ret=rtk_rg_l3_pe_dual_fmr_add(first_invalid,&entry);
		if(aal_ret)
		{
			TABLE("ERROR FMR[%d]  aal_ret=%d",first_invalid,aal_ret);
			return aal_ret;
		}
		fmrhwInfo->hwFmrIdx=first_invalid;
		fc_db.devGwMacTbl[devGwMacIdx].mapet_fmr_hwidx = first_invalid;

		if(netifSwIdx!=FAILED)		
		{		
			fc_db.netifTbl[netifSwIdx].hwFmrIdx = first_invalid;
			TABLE("ADD FMR_TABLE[%d] netifSwIdx %d",first_invalid,netifSwIdx);
		}
		return SUCCESS;
	}


}
int rtk_fc_mapet_hwfmr_del(rtk_fc_mapet_hwfmrInfo_t *fmrhwInfo)
{
	l3pe_dual_FMR_tbl_entry_t entry,getEntry;
	int i,aal_ret;
	int extistEntryIdx=FAIL;

	if(fmrhwInfo==NULL)
		return FAILED;

	if(fmrhwInfo->fmrPrefixLen==0 && fmrhwInfo->psidEndOffset==0 && fmrhwInfo->psidStartOffset==0)
	{	
		//delete by index
		return rtk_rg_l3_pe_dual_fmr_del(fmrhwInfo->hwFmrIdx);	
	}
	
	//error check
	if(fmrhwInfo->fmrPrefixLen > 128)
		return FAILED;
	if(fmrhwInfo->psidStartOffset > fmrhwInfo->psidEndOffset)
		return FAILED;
	memset(&entry,0,sizeof(entry));


	memcpy(&entry.fmr_pre_addr,fmrhwInfo->fmrv6Prefix,sizeof(entry.fmr_pre_addr));
	entry.fmr_pref_len = fmrhwInfo->fmrPrefixLen;
	
	if(fmrhwInfo->draftVersion)
		entry.fmr_drf_ver = 1;

	if(fmrhwInfo->psidStartOffset == fmrhwInfo->psidEndOffset)
	{
		entry.fmr_psid_refill	=0;
		entry.fmr_psid_end_ost	=0;
		entry.fmr_psid_str_ost	=0;
	}
	else
	{
		entry.fmr_psid_refill=1;
		entry.fmr_psid_str_ost = fmrhwInfo->psidStartOffset;
		entry.fmr_psid_end_ost = fmrhwInfo->psidEndOffset;
	}
	
	for(i=0;i<L3PE_DUAL_FMR_TBL_ENTRY_MAX;i++)
	{
		aal_ret=rtk_rg_l3_pe_dual_fmr_get(i, &getEntry);
		if(((aal_ret== ASIC_RET_ENTRYNOTRSVD) || (aal_ret == ASIC_RET_NOT_FOUND) || (aal_ret == ASIC_RET_NULL_POINTER)) ){
			continue;
		}else if(!memcmp(&getEntry,&entry,sizeof(l3pe_dual_FMR_tbl_entry_t))){
			extistEntryIdx=i;
			//compare with exist FMR, if match perfectly, use it
			break;
		}
	}

	if(extistEntryIdx!=FAIL)
	{
		return rtk_rg_l3_pe_dual_fmr_del(extistEntryIdx);
	}
	else
	{
		//not get exist entry 
		return SUCCESS;
	}

}
#endif

int rtk_fc_dual_devInfoConfig(char* devName,rtk_fc_api_dualHdrtype_t dualType,rtk_fc_dual_devInfoConfig_t *dualDevInfo)
{

	int i,j;
	int devGwMacIdx=FAIL;
	for(i=DEVIFIDX_VALID_MIN ;i <RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev==NULL)
			continue;		
		if(strcmp(fc_db.devGwMacTbl[i].dev->name,devName)==0)
		{
			devGwMacIdx=i;
			break;
		}
	}
	if(devGwMacIdx==FAIL)
		return RTK_FC_RET_ERR_INVALID_PARAM;
	if(dualType==RTK_FC_API_DUALTYPE_XLAT464)
	{
		struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
		struct net_device *devGwMac=fc_db.devGwMacTbl[devGwMacIdx].dev;
		if(dualDevInfo->xlat464_info.clat_prefixLen!=dualDevInfo->xlat464_info.plat_prefixLen)
		{
			WARNING("clat_prefixLen(%d) != xlat464_info(%d) not support yet!",dualDevInfo->xlat464_info.clat_prefixLen,dualDevInfo->xlat464_info.plat_prefixLen);
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
		if(dualDevInfo->xlat464_info.clat_prefixLen==96)
			fc_db.dualIpv6HashMask |= DUAL_IPV6_HASHMASK_PREFIX_96;
		else if(dualDevInfo->xlat464_info.clat_prefixLen==64)
			fc_db.dualIpv6HashMask |= DUAL_IPV6_HASHMASK_PREFIX_64;			
		else if(dualDevInfo->xlat464_info.clat_prefixLen==56)
			fc_db.dualIpv6HashMask |= DUAL_IPV6_HASHMASK_PREFIX_56;			
		else if(dualDevInfo->xlat464_info.clat_prefixLen==48)
			fc_db.dualIpv6HashMask |= DUAL_IPV6_HASHMASK_PREFIX_48;			
		else if(dualDevInfo->xlat464_info.clat_prefixLen==40)
			fc_db.dualIpv6HashMask |= DUAL_IPV6_HASHMASK_PREFIX_40;
		else if(dualDevInfo->xlat464_info.clat_prefixLen==32)
			fc_db.dualIpv6HashMask |= DUAL_IPV6_HASHMASK_PREFIX_32;
		else
		{
			WARNING("prefixLen(%d) not support",dualDevInfo->xlat464_info.clat_prefixLen);
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
		// sync all swNetif dual info
		fc_db.devGwMacTbl[devGwMacIdx].isXLAT464=1;

		for(i=0; i<RTK_FC_TABLESIZE_INTF_SW; i++)
		{
			if (fc_db.netifTbl[i].intf.valid)
			{
				rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[i].psIfidxStackKey,p_PScacheDev);
			}
			for(j=0;j<DEV_STACK_MAX;j++)
			if(p_PScacheDev[j]  &&  p_PScacheDev[j]==devGwMac)
			{
				fc_db.netifTbl[i].dualType=RTK_FC_DUALTYPE_XLAT464;
				fc_db.netifTbl[i].dualUniInfo.xlatInfo.Ipv6Addr_localPreifx = (fc_db.dualIpv6HashMask & (~DUAL_IPV6_HASHMASK_ALL));
				fc_db.netifTbl[i].dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx = (fc_db.dualIpv6HashMask & (~DUAL_IPV6_HASHMASK_ALL));				
				TABLE("update swNetif[%d] localPreifx:%d remotePreifx :%d",i,fc_db.netifTbl[i].dualUniInfo.xlatInfo.Ipv6Addr_localPreifx,fc_db.netifTbl[i].dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx);

				//delete flow by netif
				if(fc_db.netifTbl[i].hwIdx!=SIGNED_INVALID)
					rtk_fc_flow_delete_by_intfIdx(fc_db.netifTbl[i].hwIdx);
				//re-sync info
				fc_db.netifTbl[i].dualUniInfo_syncedDOWN=0;
				fc_db.netifTbl[i].dualUniInfo_syncedUP=0;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				fc_db.netifTbl[i].dualHdr_HWState=0;
#endif
			}
		}

	}
	else if (dualType==RTK_FC_API_DUALTYPE_MAPE || dualType==RTK_FC_API_DUALTYPE_MAPT)
	{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		rtk_fc_mapet_hwfmr_add(&dualDevInfo->mape_mapt_hwInfo);
#endif
	}


	return RTK_FC_RET_OK;
	
}



int rtk_fc_wlan_ingress_info_recovery(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{

	if(pPktHdr->fromWlan_noHWlookup){
		
		rtk_fc_pmap_t wifiPortMap;
		
		//decide src wlan device index
		RTK_FC_HELPER_WLAN_DEV_TO_DEVID(RTSKB_DEV(rtskb), &pPktHdr->igrWlanDevIdx);

		if(pPktHdr->igrWlanDevIdx==RTK_FC_WLANX_NOT_FOUND)
		{
			// unknown dev
			WARNING("unknown wifi dev %s, drop packet.", RTSKB_DEV(rtskb)->name);
			return RTK_FC_NIC_RX_STOP;
		}
		
		RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pPktHdr->igrWlanDevIdx, &wifiPortMap.macPortIdx, &wifiPortMap.macExtPortIdx);
	
		pPktHdr->ingressPort.macPortIdx = wifiPortMap.macPortIdx;
		pPktHdr->ingressPort.macExtPortIdx = wifiPortMap.macExtPortIdx;

	}
	else
	{
		//hwlookup miss and then trap from switch, we can get wlan_dev_idx from rxDesc's extspa.

		struct ethhdr *eth = (struct ethhdr *)RTSKB_DATA(rtskb);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		rtk_fc_wlan_ext_info_t wlanExtInfo;
		wlanExtInfo.macExtPort = pPktHdr->ingressPort.macExtPortIdx;
		pPktHdr->igrWlanDevIdx = rtk_fc_wlan_port_to_devid_find(pPktHdr->ingressPort.macPortIdx, wlanExtInfo, rtskb, RTSKB_SKB(rtskb), TRUE, FALSE);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		pPktHdr->igrWlanDevIdx = pPktHdr->ingressPort.macExtPortIdx;
		RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pPktHdr->igrWlanDevIdx, &pPktHdr->ingressPort.macPortIdx, &pPktHdr->ingressPort.macExtPortIdx);
#else
		rtk_fc_wlan_ext_info_t wlanExtInfo;
		wlanExtInfo.wlanDevIdx = pPktHdr->ingressPort.macExtPortIdx;
		pPktHdr->ingressPort.macExtPortIdx = RTK_FC_MAC_EXT_NONE;
		pPktHdr->igrWlanDevIdx = rtk_fc_wlan_port_to_devid_find(pPktHdr->ingressPort.macPortIdx, wlanExtInfo, rtskb, RTSKB_SKB(rtskb), TRUE, FALSE);
#endif	//CONFIG_FC_RTL8277C_SERIES || defined(CONFIG_FC_RTL9607F_SERIES)
#endif

		if(pPktHdr->igrWlanDevIdx==RTK_FC_WLANX_NOT_FOUND) {
		#if defined(CONFIG_RTK_SOC_RTL8198D)
			WIFI("fail to get wlan dev idx, spa %d extspa %d smac %pM", pPktHdr->ingressPort.macPortIdx, pPktHdr->ingressPort.macExtPortIdx, &eth->h_source[0]);
		#else
			WARNING("fail to get wlan dev idx, spa %d extspa %d smac %pM", pPktHdr->ingressPort.macPortIdx, pPktHdr->ingressPort.macExtPortIdx, &eth->h_source[0]);
		#endif
		}

	}

	if(pPktHdr->igrWlanDevIdx==RTK_FC_WLANX_NOT_FOUND)
	{
	#if defined(CONFIG_RTK_SOC_RTL8198D)
		WIFI("[Drop] strange wifi packet....can't decide wlan_dev_idx, Drop!dev=%s, spa:%d", RTSKB_DEV(rtskb)->name, pPktHdr->ingressPort.macPortIdx);
	#else
		WARNING("[Drop] strange wifi packet....can't decide wlan_dev_idx, Drop!dev=%s, spa:%d", RTSKB_DEV(rtskb)->name, pPktHdr->ingressPort.macPortIdx);
	#endif
		if(unlikely(fc_db.smpStatistic))
			atomic_inc(&fc_db.smp_statistic[FC_SMP_WIFI_RX_DROP][smp_processor_id()]);

		return RTK_FC_NIC_RX_STOP;
	}

	TRACE("wifi! wlan_dev_idx is %d, IngressMacPort=%d, IngressMacExtPort=%d", pPktHdr->igrWlanDevIdx, pPktHdr->ingressPort.macPortIdx, pPktHdr->ingressPort.macExtPortIdx);
	return RTK_FC_NIC_RX_CONTINUE;
}

int rtk_fc_fromWifiToProtocolStack(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{
	struct net_device *dev=NULL;
	int wlan_dev_idx = pPktHdr->igrWlanDevIdx;

	DEBUG("wifi dev idx %d recv pkt to ps", wlan_dev_idx);
	RTK_FC_HELPER_WLAN_DEVID_TO_DEV(wlan_dev_idx, &dev);

	if(dev){
		RTSKB_DEV(rtskb) = dev;
		RTSKB_FROMDEV(rtskb)= dev;
#if defined(CONFIG_RTK_SOC_RTL8198D)
		RTSKB_FCIGRDATA(rtskb)->wlan_type = RTK_FC_HELPER_IS_WLAN_5G(dev->name);
#endif
		if( fc_db.controlFuc.wifiSlowPathRxbyNic){
			TRACE("change skb dev to %s before continue NIC Rx func",dev->name);
			return RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS;
		}
		else{
			TRACE("change skb dev to %s before enter netif_rx",dev->name);
			return RTK_FC_NIC_RX_NETIFRX_BY_FC;	//the SKB should be received to Linux directly!
		}
	}else{
		TRACE("wifi devid: %d hardware lookup miss but dev is not ready, drop packet here", wlan_dev_idx);
		return RTK_FC_NIC_RX_STOP;
	}

}

int rtk_fc_l2Info_get(uint8 *mac, rtk_fc_mgr_l2_info_t *l2info)
{
	int ret;
	int16 lutIdx=0;

	{
		RTK_FC_HELPER_RCU_READ_LOCK();
		
		if((ret = _rtk_fc_lut_find(mac, &lutIdx)) == RTK_FC_RET_OK) {
			l2info->spa = fc_db.lutTbl[lutIdx]->spa;
			l2info->extspa = fc_db.lutTbl[lutIdx]->extspa;
			l2info->wlan_dev_idx = fc_db.lutTbl[lutIdx]->wlan_dev_idx;
			l2info->isGMAC = fc_db.lutTbl[lutIdx]->isGMAC;
			l2info->lutIdx = lutIdx;
		}
		
		RTK_FC_HELPER_RCU_READ_UNLOCK();
	}

	return ret;
}


//entrance from wifi driver rx function
int rtk_fc_fastfwd_ingress_rcv(struct rt_skbuff *rtskb)
{
	int ret;

	/*     for the packets from wifi and no need to do HW loop up (non-unicast or wifi device other than root, vap0, vap1, vap2, vap3 ),
	   go to FC ingress_flowLearning (for ingress LUT learning)*/
	{
		RTK_FC_HELPER_LOCAL_BH_DISABLE();
		RTK_FC_HELPER_RCU_READ_LOCK();
		
		//============= Critical Section Start =============//
		ret = _rtk_fc_ingress_flowLearning(rtskb, NULL);
		//============= Critical Section End =============//
		
		RTK_FC_HELPER_RCU_READ_UNLOCK();
		RTK_FC_HELPER_LOCAL_BH_ENABLE();
	}

	return ret;
}

int rtk_fc_fastfwd_egress_xmit(struct rt_skbuff *rtskb, struct net_device *dev, rtk_fc_wlan_devidx_t wlanDevIdx)
{
	rtk_fc_mac_port_idx_t macPort;
	rtk_fc_mac_ext_port_idx_t macExtPort;
	struct sk_buff *skb = RTSKB_SKB(rtskb);

	if(RTSKB_FCIGRDATA(rtskb)->isDummyPkt)
	{
		// dummy packet - free skb
		atomic_inc(&fc_db.statistic.perPortCnt_dummpPkt[RTSKB_FCIGRDATA(rtskb)->ingressPort]);
		RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(skb);
		return 0;
	}

	if (RTK_FC_RET_OK != RTK_FC_HOOK_VLAN_PASSTHROUGH_TX_HANDLE(&skb, rtskb)){
		//if failed, skb already free.
		return 0;
	}

	if((1 << fc_db.controlFuc.hwnat_mode) & HWNAT_MODE_SKIP_FC_FUNC_BITMASK) //trap to ps mode
	{
		TRACE("bypass - continue tx to wifi dev %s ... ", dev->name);
		goto WLAN_TX_WITHOUT_LEARN;
	}

	if(fc_db.skbmark[RTK_FC_SKBMARK_SKIPFCFUNC].startBit != RTK_FC_RMK_UNDEF)
	{
		if(rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SKIPFCFUNC))
		{
			TRACE("skb_mark skipFcFunc is enabled - continue tx to wifi dev %s ... ", dev->name);
			goto WLAN_TX_WITHOUT_LEARN;
		}
	}

	if(wlanDevIdx >= RTK_FC_WLANX_END_INTF) {
		WARNING("unknown Tx from wifi dev %s ... ", dev->name);
		goto WLAN_TX_WITHOUT_LEARN;
	}

	TRACE("[TX] SKB[%p] dev %s wlanDevIdx %d", skb, dev->name, wlanDevIdx);

	RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(wlanDevIdx, &macPort, &macExtPort);

	return rtk_fc_egress_flowLearning(rtskb, dev, (1<<macPort), wlanDevIdx);

WLAN_TX_WITHOUT_LEARN:

	if(wlanDevIdx <= RTK_FC_WLANX_END_INTF) {
		RTK_FC_HELPER_WLAN_HARD_START_XMIT(wlanDevIdx, skb, NULL);
	}else{

		WARNING("not handled dev %s", dev->name);
		RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(skb);
		return NETDEV_TX_BUSY;
	}


	return NETDEV_TX_OK;
}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
static int rtk_fc_wifiMcFastForward(struct sk_buff *skb, fc_rx_info_t *pRxDesc)
{
	rtk_fc_wlan_devidx_t wlan_dev_idx=RTK_FC_WLANX_NOT_FOUND;
	uint32 _rx_dst_export_mask=RXINFO_DST_EXTPMASK(pRxDesc);
	uint32 mbssidStart=0U,mbssidEnd=0U,sendMbssid=0U;
	struct sk_buff *new_skb=NULL;
	rtk_fc_mac_port_idx_t macPort;
	rtk_fc_mac_ext_port_idx_t macExtPort;
	rtk_fc_wlan_devmask_t wlanDevBitMask;

	//ingore cpu port
	_rx_dst_export_mask &= (~(0x1));

	while(_rx_dst_export_mask)
	{
		wlan_dev_idx = RTK_FC_WLANX_NOT_FOUND;
		if(RXINFO_GMAC(pRxDesc)==0U)
		{
			macPort = RTK_FC_MAC_PORT_MAINCPU;
		}
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
		else if(RXINFO_GMAC(pRxDesc)==1U)
		{
			macPort = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
		}
		else if(RXINFO_GMAC(pRxDesc)==2U)
		{
			macPort = RTK_FC_MAC_PORT_SLAVECPU;
		}
#endif
		else
		{
			WARNING("invalid cp->gmac %u", RXINFO_GMAC(pRxDesc));
			return RTK_FC_NIC_RX_STOP;
		}
		macExtPort = __ffs(_rx_dst_export_mask);		//bit 0 is for CPU extport
		
		_rx_dst_export_mask &= (~(1<<macExtPort));

		RTK_FC_HELPER_WLAN_PORT_TO_DEVID(macPort, macExtPort, &wlan_dev_idx, &wlanDevBitMask);

		if((wlan_dev_idx==RTK_FC_WLANX_MULTI_INTF) && wlanDevBitMask)
		{
			mbssidStart = __ffs64(wlanDevBitMask);
			mbssidEnd = (63 - __builtin_clzll(wlanDevBitMask));
		}else if(wlan_dev_idx == RTK_FC_WLANX_NOT_FOUND) {
			WARNING("not wlan dev binding to port %d extport %d", macPort, macExtPort);
			continue;
		}else{
			mbssidStart = mbssidEnd = wlan_dev_idx;
		}

		for(sendMbssid=mbssidStart;sendMbssid<=mbssidEnd;sendMbssid++)
		{
			if((wlan_dev_idx==RTK_FC_WLANX_MULTI_INTF) && !(wlanDevBitMask & (1LL<<sendMbssid)))
				continue;

			if(RTK_FC_HELPER_WLAN_CHECK_DEV_IS_RUNNING(sendMbssid)!= SUCCESS)
				continue;
			
			WIFI("Send to  WIFI by fast Mc forward Mbssid:%d",sendMbssid);

			RTK_FC_HOOK_PS_SKB_SKB_COPY(skb, GFP_ATOMIC, &new_skb);
			if(new_skb==NULL) goto FASTFORWARD_OUT_OF_MEM;

			RTK_FC_HELPER_WLAN_HARD_START_XMIT(sendMbssid, new_skb, pRxDesc);
		}
		
	}

	WIFI("Multicast FastForward Send End pRxDesc->rx_src_port_num=%d",RXINFO_SPA(pRxDesc));

	if(unlikely(fc_db.fwdStatistic))
	{
		atomic_inc(&fc_db.statistic.perPortCnt_MWFF_TX[RXINFO_SPA(pRxDesc)]);
	}

	return RTK_FC_NIC_RX_STOP;

FASTFORWARD_OUT_OF_MEM:
	WARNING("Wifi Fast forward out of memory");
	return RTK_FC_NIC_RX_STOP;

}

__IRAM_FC_NICTRX
static int rtk_fc_wifiUCFastForward(struct sk_buff *skb, fc_rx_info_t *pRxDesc)
{
	int  wlan_dev_idx=RTK_FC_WLANX_NOT_FOUND;
	rtk_fc_mac_port_idx_t macPort=0;
	rtk_fc_wlan_ext_info_t wlanExtInfo;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(RXINFO_GMAC(pRxDesc)==0U)
	{
		macPort = RTK_FC_MAC_PORT_MAINCPU;
	}
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	else if(RXINFO_GMAC(pRxDesc)==1U)
	{
		macPort = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
	}
	else if(RXINFO_GMAC(pRxDesc)==2U)
	{
		macPort = RTK_FC_MAC_PORT_SLAVECPU;
	}
#endif
	else {
		WARNING("known packet to gmac %d", RXINFO_GMAC(pRxDesc));
		goto ERROR_DEST;
	}

	wlanExtInfo.macExtPort = __ffs(RXINFO_DST_EXTPMASK(pRxDesc));		//bit 0 is for CPU extport

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

#if 0 // wifi FF: ldpid may be 0x10~0x0x13, and no need RXINFO_GMAC info
	macPort = RTK_FC_GMAC_TO_MACPORT(RXINFO_GMAC(pRxDesc));

	if(macPort > RTK_FC_MAC_PORT_WLAN_CPU5){
		WARNING("known packet to gmac %d", RXINFO_GMAC(pRxDesc));
		goto ERROR_DEST;
	}
#endif
	wlanExtInfo.wlanDevIdx = RXINFO_DST_EXTPORT_IDX(pRxDesc);

#endif


	wlan_dev_idx = rtk_fc_wlan_port_to_devid_find(macPort, wlanExtInfo, NULL, skb, FALSE, TRUE);

	if(unlikely(wlan_dev_idx >= RTK_FC_WLANX_END_INTF)) {
		// skb will be freed because pWifiDev is NULL
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		WARNING("wlan port to dev idx mapping is something wrong, wlanExtInfo.wlanDevIdx %d", wlanExtInfo.wlanDevIdx);
#else
		WARNING("wlan port to dev idx mapping is something wrong, port %d extport %d, wlan_dev_idx %d, spa %d, skb->len=%d", 
			macPort, wlanExtInfo.macExtPort, wlan_dev_idx, RXINFO_SPA(pRxDesc), skb->len);
#endif
		goto ERROR_DEST;
	}

	RTK_FC_HELPER_WLAN_HARD_START_XMIT(wlan_dev_idx, skb, pRxDesc);


	WIFI("Send to WIFI DEVID %d by fast forward", wlan_dev_idx);

	if(unlikely(fc_db.fwdStatistic))
	{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		if(skb->data[0]&1)
			atomic_inc(&fc_db.statistic.mcPortCnt_WIFI_FF_TX);
		else
#endif
		{
			atomic_inc(&fc_db.statistic.perPortCnt_MWFF_TX[RXINFO_SPA(pRxDesc)]);
		}
	}

	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
	
ERROR_DEST:
	RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(skb);
	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
}

#endif

__IRAM_FC_NICTRX
int rtk_fc_fastfwd_directXmit(struct sk_buff *skb, fc_rx_info_t *pRxInfo)
{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)

	// G3 HW support replicate packet and do tx one by one.
	// return rtk_fc_wifiUCFastForward(skb, pRxInfo);

	if(unlikely(fc_db.fwdStatistic))
	{
		if(skb->data[0]&1)
			atomic_inc(&fc_db.statistic.mcPortCnt_WIFI_FF_TX);
		else
		{
			atomic_inc(&fc_db.statistic.perPortCnt_MWFF_TX[RXINFO_SPA(pRxInfo)]);
		}
	}

#if defined(CONFIG_FC_QTNA_WIFI_AX)
	RXINFO_IS_WIFI_FF(pRxInfo) = 1;
#endif

	return RTK_FC_HELPER_WLAN_HARD_START_XMIT(RXINFO_DST_EXTPIDX(pRxInfo), skb, pRxInfo);
 	
#else //!CONFIG_RTK_L34_G3_PLATFORM

	uint32 dst_extp_cnt = RXINFO_DST_EXTPMASK(pRxInfo);

	RTK_FC_ONE_COUNT(dst_extp_cnt);

	if((dst_extp_cnt==1U)&& ((skb->data[0]&1)==0))
		return rtk_fc_wifiUCFastForward(skb, pRxInfo);
	else
		return rtk_fc_wifiMcFastForward(skb, pRxInfo);
	
#endif
}

#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D)
void update_gmac_sel_by_port_status(int port, int linkup);
#endif
void rtk_fc_switchLinkChangeHandler(intrBcasterMsg_t	*pMsgData)
{

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	if(pMsgData->intrType==MSG_TYPE_LINK_CHANGE && pMsgData->intrSubType==INTR_STATUS_LINKUP)
	{
		/* Link up: setup port mask */
		atomic_set(&fc_db.portLinkupMask, atomic_read(&fc_db.portLinkupMask) | (1<<pMsgData->intrBitMask));
		DEBUG("@%s, port %d linkup, linkupmask = 0x%x", __FUNCTION__, pMsgData->intrBitMask, atomic_read(&fc_db.portLinkupMask));
#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D) && !defined(CONFIG_FC_WIFI_RX_NONE)
		update_gmac_sel_by_port_status(pMsgData->intrBitMask, RT_PORT_LINKUP);
#endif
	}else if (pMsgData->intrType==MSG_TYPE_LINK_CHANGE && pMsgData->intrSubType==INTR_STATUS_LINKDOWN)
	{
		/* Link down: clear port mask */
		atomic_set(&fc_db.portLinkupMask, atomic_read(&fc_db.portLinkupMask) & ~(1<<pMsgData->intrBitMask));
		DEBUG("@%s, port %d linkdown, linkupmask = 0x%x", __FUNCTION__, pMsgData->intrBitMask, atomic_read(&fc_db.portLinkupMask));
#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D) && !defined(CONFIG_FC_WIFI_RX_NONE)
				update_gmac_sel_by_port_status(pMsgData->intrBitMask, RT_PORT_LINKDOWN);
#endif

		_rtk_fc_lut_linkdown_del(pMsgData->intrBitMask);		// delete l2 entry -> delete related flow entries.
	}

	
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && !defined(CONFIG_FC_RTL8198F_SERIES)

	if(fc_db.controlFuc.lan_queue_ability_enhance)
	{
		/* Adjust buffer threshold if only one lan port link up with 100M */
		rtk_fc_portStatusInfo_t portInfo;
		uint32 linkup_cnt = (fc_db.lanPortMask.portmask & atomic_read(&fc_db.portLinkupMask));
		uint8 profileId, find = FALSE;
		aal_l3_te_cb_comb_threshold_t voq_threshold_cfg;
		aal_l3_te_cb_comb_threshold_mask_t voq_threshold_cfg_mask;

		aal_l3_te_cb_comb_threshold_t port_threshold_cfg;
		aal_l3_te_cb_comb_threshold_mask_t port_threshold_mask;
		uint32 last_port_idx = RTK_FC_MAC_PORT4;

		linkup_cnt &= ((1<<(last_port_idx+1)) - 1);

		RTK_FC_ONE_COUNT(linkup_cnt);

		if(linkup_cnt<=2) {
			uint32 singlepid = 0;
			//__ffs((uint32)(fc_db.lanPortMask.portmask & atomic_read(&fc_db.portLinkupMask)));
			for(singlepid = 0; singlepid <= last_port_idx; singlepid++) {
				if((1<<singlepid) & (fc_db.lanPortMask.portmask & atomic_read(&fc_db.portLinkupMask))) {		
					rtk_fc_portStatus_get(singlepid, &portInfo);
					DEBUG("lan linkup with single port %d and speed is %d", singlepid, portInfo.linkSpeed);	
					if(portInfo.linkSpeed<=RTK_RG_PORT_SPEED_100M) {
						find = TRUE;
						break; //find
					}
				}
			}
		}

		if(fc_db.systemGlobal.l3te_voq_thrsh_profile_3_hth == 0) {
			// save default setting
			memset(&voq_threshold_cfg, 0, sizeof(voq_threshold_cfg));
			memset(&port_threshold_cfg, 0, sizeof(port_threshold_cfg));
			
			aal_l3_te_cb_voq_profile_sel_get(G3_DEF_DEVID, ((CA_AAL_MAX_VOQ_ID+1)/2), &profileId);
			aal_l3_te_cb_voq_threshold_profile_get(G3_DEF_DEVID, profileId, &voq_threshold_cfg);
			fc_db.systemGlobal.l3te_voq_thrsh_profile_3_hth = voq_threshold_cfg.threshold_hi ;
			
			aal_l3_te_cb_port_profile_sel_get(G3_DEF_DEVID, CA_AAL_L3_TE_NI_PORT_OFFSET, &profileId);
			aal_l3_te_cb_port_threshold_profile_get(G3_DEF_DEVID, profileId, &port_threshold_cfg);
			fc_db.systemGlobal.l3te_port_thrsh_profile_3_lth = port_threshold_cfg.threshold_lo;
		}

		if(find)
		{
			/*set voq threshold to MAX*/

			// get port 0 voq profile
			aal_l3_te_cb_voq_profile_sel_get(G3_DEF_DEVID, ((CA_AAL_MAX_VOQ_ID+1)/2), &profileId);

			memset(&voq_threshold_cfg, 0, sizeof(voq_threshold_cfg));
			memset(&voq_threshold_cfg_mask, 0, sizeof(voq_threshold_cfg_mask));
			voq_threshold_cfg.threshold_hi = 0x3fff;
			voq_threshold_cfg_mask.s.threshold_hi = 1;;
			aal_l3_te_cb_voq_threshold_profile_set(G3_DEF_DEVID, profileId, voq_threshold_cfg_mask, &voq_threshold_cfg);

			// get port 0 profile
			aal_l3_te_cb_port_profile_sel_get(G3_DEF_DEVID, CA_AAL_L3_TE_NI_PORT_OFFSET, &profileId);

			memset(&port_threshold_cfg, 0, sizeof(port_threshold_cfg));
			memset(&port_threshold_mask, 0, sizeof(port_threshold_mask));
			port_threshold_cfg.threshold_lo = 0;
			port_threshold_mask.s.threshold_lo = 1;
			aal_l3_te_cb_port_threshold_profile_set(G3_DEF_DEVID, profileId, port_threshold_mask, &port_threshold_cfg);
			
			DEBUG("adjust voq threshold to max");
		}
		else
		{
			/*set voq threshold to Initialization*/

			// get port 0 voq profile
			aal_l3_te_cb_voq_profile_sel_get(G3_DEF_DEVID, ((CA_AAL_MAX_VOQ_ID+1)/2), &profileId);

			memset(&voq_threshold_cfg, 0, sizeof(voq_threshold_cfg));
			memset(&voq_threshold_cfg_mask, 0, sizeof(voq_threshold_cfg_mask));
			voq_threshold_cfg.threshold_hi = fc_db.systemGlobal.l3te_voq_thrsh_profile_3_hth;		// assume lan use profile 3
			voq_threshold_cfg_mask.s.threshold_hi = 1;;
			aal_l3_te_cb_voq_threshold_profile_set(G3_DEF_DEVID, profileId, voq_threshold_cfg_mask, &voq_threshold_cfg);

			// get port 0 profile
			aal_l3_te_cb_port_profile_sel_get(G3_DEF_DEVID, CA_AAL_L3_TE_NI_PORT_OFFSET, &profileId);

			memset(&port_threshold_cfg, 0, sizeof(port_threshold_cfg));
			memset(&port_threshold_mask, 0, sizeof(port_threshold_mask));
			port_threshold_cfg.threshold_lo = fc_db.systemGlobal.l3te_port_thrsh_profile_3_lth;
			port_threshold_mask.s.threshold_lo = 1;
			aal_l3_te_cb_port_threshold_profile_set(G3_DEF_DEVID, profileId, port_threshold_mask, &port_threshold_cfg);
			
			DEBUG("adjust voq threshold to generic setting");
		}
	}
#endif

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
}

rtk_rg_err_code_t rtk_fc_portStatus_get(rtk_fc_mac_port_idx_t port, rtk_fc_portStatusInfo_t *portInfo)
{
	rtk_port_linkStatus_t linkStatus;
    rtk_port_speed_t linkSpeed;
    rtk_port_duplex_t linkDuplex;

	if(portInfo==NULL)
		return RT_ERR_RG_NULL_POINTER;
	if(port < 0 || port>=RTK_FC_MAC_PORT_MAX)
		return RT_ERR_RG_INVALID_PARAM;

	bzero(&linkStatus,sizeof(rtk_port_linkStatus_t));
	bzero(&linkSpeed,sizeof(rtk_port_speed_t));
	bzero(&linkDuplex,sizeof(rtk_port_duplex_t));

	if(rtk_port_link_get(port, &linkStatus)==0)
		portInfo->linkStatus = linkStatus;

	if(rtk_port_speedDuplex_get(port, &linkSpeed, &linkDuplex)==0)
	{
		portInfo->linkSpeed = linkSpeed;
		portInfo->linkDuplex = linkDuplex;
	}
	return (RT_ERR_RG_OK);
}

bool _rtk_fc_highPriFlow_check(void *pPathData, int *hitIdx)
{
	rtk_rg_asic_path3_entry_t *pFlow = pPathData;
	int i;

	for(i = 0 ; i < RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE ; i++)
	{
		if(!(fc_db.highPriFlowPattern[i].sip6_valid || fc_db.highPriFlowPattern[i].dip6_valid || fc_db.highPriFlowPattern[i].sip4_valid || fc_db.highPriFlowPattern[i].dip4_valid || fc_db.highPriFlowPattern[i].sport_valid || fc_db.highPriFlowPattern[i].dport_valid || fc_db.highPriFlowPattern[i].l4proto_valid))
			continue; // no valid pattern, do nothing

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		if((fc_db.highPriFlowPattern[i].l4proto_valid) && (((fc_db.highPriFlowPattern[i].l4proto==RT_FLOW_OP_FLOW_PATTERN_L4PROTO_TCP)?IPPROTO_TCP:IPPROTO_UDP) != pFlow->in_l4proto_num))
#else
		if((fc_db.highPriFlowPattern[i].l4proto_valid) && (fc_db.highPriFlowPattern[i].l4proto != pFlow->in_l4proto))
#endif
			continue;

		if((fc_db.highPriFlowPattern[i].sip4_valid || fc_db.highPriFlowPattern[i].dip4_valid) && (fc_db.highPriFlowPattern[i].sip6_valid || fc_db.highPriFlowPattern[i].dip6_valid))
			continue; //pattern include IPv4 and IPv6 at the same time

		if((fc_db.highPriFlowPattern[i].sip4_valid || fc_db.highPriFlowPattern[i].dip4_valid) && pFlow->in_ipv4_or_ipv6)
			continue; //pattern include SIPv4 or DIPv4 but is IPv6 flow

		if(fc_db.highPriFlowPattern[i].sip4_valid && (fc_db.highPriFlowPattern[i].sip4 != pFlow->in_src_ipv4_addr))
			continue; //pattern include SIPv4 but no hit

		if(fc_db.highPriFlowPattern[i].dip4_valid && (fc_db.highPriFlowPattern[i].dip4 != pFlow->in_dst_ipv4_addr))
			continue; //pattern include DIPv4 but no hit

		if((fc_db.highPriFlowPattern[i].sip6_valid || fc_db.highPriFlowPattern[i].dip6_valid) && !pFlow->in_ipv4_or_ipv6)
			continue; //pattern include SIPv6 or DIPv6 but is IPv4 flow

		if(fc_db.highPriFlowPattern[i].sip6_valid && (_rtk_rg_flowHashIPv6SrcAddr_get(fc_db.highPriFlowPattern[i].sip6) != pFlow->in_src_ipv6_addr_hash))
			continue; //pattern include SIPv6 but no hit

		if(fc_db.highPriFlowPattern[i].dip6_valid && (_rtk_rg_flowHashIPv6DstAddr_get(fc_db.highPriFlowPattern[i].dip6) != pFlow->in_dst_ipv6_addr_hash))
			continue; //pattern include DIPv6 but no hit

		if(fc_db.highPriFlowPattern[i].sport_valid && (fc_db.highPriFlowPattern[i].sport != pFlow->in_l4_src_port))
			continue; //pattern include SPORT but no hit

		if(fc_db.highPriFlowPattern[i].dport_valid && (fc_db.highPriFlowPattern[i].dport != pFlow->in_l4_dst_port))
			continue; //pattern include DPORT but no hit

		*hitIdx = i;
		return TRUE;
	}
	*hitIdx = SIGNED_INVALID;
	return FALSE;
}


int _rtk_fc_sw_rate_limit_drop(rt_rate_sw_rate_limit_type_t type, uint32 pktLen, uint32 igrPort, rtk_fc_wlan_devidx_t igrWlanDevIdx)
{
	rtk_fc_sw_meter_t *meterConf=NULL;
	uint32 i, j, igrPortHit=0U, igrWlanDevIdxHit=0U;

	igrPortHit = (fc_db.swRateLimit[type].rateLimit_conf.ingress_port_mask & (0x1<<igrPort)) ? 1 : 0;
	if(igrWlanDevIdx!=RTK_FC_WLANX_NOT_FOUND)
	{
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
		{
			for(j=0; j<RT_WLAN_MBSSID_MAX; j++)
			{
				if(fc_db.swRateLimit[type].rateLimit_conf.ingress_wlan_dev_mask[i] & (0x1<<j))
				{
					if(igrWlanDevIdx==rtk_fc_wlan_rtmbssid_to_devidx(i, j))
					{
						igrWlanDevIdxHit = 1U;
						break;
					}
				}
			}
		}
	}
	if(igrPortHit || igrWlanDevIdxHit)
	{
		if(0<=fc_db.swRateLimit[type].rateLimit_conf.shareMeterIdx && fc_db.swRateLimit[type].rateLimit_conf.shareMeterIdx<RTK_FC_TABLESIZE_SW_SHAREMTR)
		{
		
			RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_SHAPER_MIB, 0);
			
			meterConf = &fc_db.swMeter[fc_db.swRateLimit[type].rateLimit_conf.shareMeterIdx];
			if(time_after_eq(jiffies, meterConf->lastJiffies + RTK_FC_SW_RATE_LIMIT_JIFFIES/*unit:(RTK_FC_SW_RATE_LIMIT_JIFFIES/CONFIG_HZ)sec*/))
			{
				memset(&meterConf->meterCount, 0, sizeof(rtk_fc_meterCount_t));
				meterConf->lastJiffies = jiffies;
			}


			if(time_after_eq(jiffies, meterConf->lastJiffiesPersec + CONFIG_HZ/*unit: 1sec*/))
			{
				memset(&meterConf->meterCountPersec, 0, sizeof(rtk_fc_meterCount_t));
				meterConf->lastJiffiesPersec = jiffies;
			}

			//Check if over rate limit (Only support bit rate now)
			{
				/*rate limit bits in time period*/
				if(
					((meterConf->meterCount.byteCount+pktLen)*8 > (((meterConf->rate*1000*RTK_FC_SW_RATE_LIMIT_JIFFIES)%CONFIG_HZ)?((meterConf->rate*1000*RTK_FC_SW_RATE_LIMIT_JIFFIES)/CONFIG_HZ + 1):((meterConf->rate*1000*RTK_FC_SW_RATE_LIMIT_JIFFIES)/CONFIG_HZ))/*unit:(RTK_FC_SW_RATE_LIMIT_JIFFIES/CONFIG_HZ)sec*/) ||
					((meterConf->meterCountPersec.byteCount+pktLen)*8 > (meterConf->rate*1000)/*unit:1sec*/)
					)
				{
					TRACE("[Drop] the packet belongs to shareMeter[%d], and over its rate limit. (type=%d)", fc_db.swRateLimit[type].rateLimit_conf.shareMeterIdx, type);
					RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_SHAPER_MIB, 0U);
					return TRUE;
				}
			}

			meterConf->meterCount.byteCount += pktLen;
			meterConf->meterCount.packetCount++;

			meterConf->meterCountPersec.byteCount += pktLen;
			meterConf->meterCountPersec.packetCount++;
			
			RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_SHAPER_MIB, 0U);
		}
	}

	return FALSE;
}

__IRAM_FC_SHORTCUT
rtk_fc_nic_rx_t _rtk_fc_sw_rate_limit(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{
	uint32 i;

	for(i=0; i<RT_RATE_SW_RATE_LIMIT_TYPE_MAX; i++)
	{
		if(fc_db.swRateLimit[i].rateLimit_conf.shareMeterIdx == SIGNED_INVALID)
			continue;
		
		switch(i)
		{
			case RT_RATE_SW_RATE_LIMIT_TYPE_ARP_REQ:
				if((RTSKB_DATA(rtskb)[0]==0xff) &&
					((*(u16*)&RTSKB_DATA(rtskb)[ETHER_ADDR_LEN*2]==htons(ETH_P_ARP)) || ((*(u16*)&RTSKB_DATA(rtskb)[ETHER_ADDR_LEN*2]==htons(ETH_P_8021Q)) && (*(u16*)&RTSKB_DATA(rtskb)[(ETHER_ADDR_LEN*2)+4]==htons(ETH_P_ARP)))))
				{
					if(_rtk_fc_sw_rate_limit_drop(RT_RATE_SW_RATE_LIMIT_TYPE_ARP_REQ, RTSKB_LEN(rtskb), pPktHdr->ingressPort.macPortIdx, pPktHdr->igrWlanDevIdx))
					{
						return RTK_FC_NIC_RX_STOP;
					}
				}
				break;
			case RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS:
				if((((pPktHdr->cvh) && fc_db.swRateLimit[i].rateLimit_conf.specific_dos.ctagif && fc_db.swRateLimit[i].rateLimit_conf.specific_dos.ctagVid==pPktHdr->cvlanid) || ((pPktHdr->cvh==NULL) && fc_db.swRateLimit[i].rateLimit_conf.specific_dos.ctagif==0))
					&& (!memcmp(&fc_db.swRateLimit[i].rateLimit_conf.specific_dos.ingressDmac[0], &pPktHdr->eth->h_dest[0], ETHER_ADDR_LEN))
					&& ((pPktHdr->iph) && pPktHdr->iph->daddr==fc_db.swRateLimit[i].rateLimit_conf.specific_dos.ingressDip)
					&& (((pPktHdr->tcph) && fc_db.swRateLimit[i].rateLimit_conf.specific_dos.isTcp) || ((pPktHdr->udph) && fc_db.swRateLimit[i].rateLimit_conf.specific_dos.isTcp==0))
					&& ((RTSKB_LEN(rtskb)>=fc_db.swRateLimit[i].rateLimit_conf.specific_dos.pktLenStart) && (RTSKB_LEN(rtskb)<=fc_db.swRateLimit[i].rateLimit_conf.specific_dos.pktLenEnd)))
				{
					if(_rtk_fc_sw_rate_limit_drop(RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS, RTSKB_LEN(rtskb), pPktHdr->ingressPort.macPortIdx, pPktHdr->igrWlanDevIdx))
					{
						return RTK_FC_NIC_RX_STOP;
					}
				}
				break;
			case RT_RATE_SW_RATE_LIMIT_TYPE_TCP_SYN_ONLY:
				if((pPktHdr->tcph) && pPktHdr->tcph->syn && !(pPktHdr->tcph->fin  |  pPktHdr->tcph->rst  |  pPktHdr->tcph->psh  |  pPktHdr->tcph->ack  |  pPktHdr->tcph->urg  |  pPktHdr->tcph->ece | pPktHdr->tcph->cwr))
				{
					if(_rtk_fc_sw_rate_limit_drop(RT_RATE_SW_RATE_LIMIT_TYPE_TCP_SYN_ONLY, RTSKB_LEN(rtskb), pPktHdr->ingressPort.macPortIdx, pPktHdr->igrWlanDevIdx))
					{
						return RTK_FC_NIC_RX_STOP;
					}
				}
				break;
			case RT_RATE_SW_RATE_LIMIT_TYPE_UNKNOWN_UC:
				if(pPktHdr->dmacL2Idx==SIGNED_INVALID && pPktHdr->isMulticast==FALSE)
				{
					TRACE("UUC sw rate limit check");
					if(_rtk_fc_sw_rate_limit_drop(RT_RATE_SW_RATE_LIMIT_TYPE_UNKNOWN_UC, RTSKB_LEN(rtskb), pPktHdr->ingressPort.macPortIdx, pPktHdr->igrWlanDevIdx))
					{
						return RTK_FC_NIC_RX_STOP;
					}
				}
				/* fall through */
			default:
				break;
		}
	}

	return RTK_FC_NIC_RX_CONTINUE;
}

static rtk_fc_ret_t _rtk_fc_flowIgnoreCheck(rtk_fc_pktHdr_t *pPktHdr)
{

	if(pPktHdr->eth==NULL)
	{
		return RTK_FC_RET_LRN_NULL_POINTER;
	}

	/*
		Ignore reserved DMAC address
		SMAC[0]&0x1==1
		SIP Multicast
		DMac 01-80-C2-XX-XX-XX / FF-XX-XX-XX-XX-XXX
		DIP 224.0.0.x
		SSDP 239.255.255.250
		DIPv6 FF01/FF02/FF05 reserved , Permit FFXE scpoe

	*/
	if( pPktHdr->igmph)
	{
		return RTK_FC_RET_LRN_SKIP_IGMP_MLD;
	}

	if(pPktHdr->icmp6h && !(pPktHdr->icmp6h->icmp6_type==128 || pPktHdr->icmp6h->icmp6_type==129 ) )
	{
		return RTK_FC_RET_LRN_SKIP_ICMPv6;
	}

	if(pPktHdr->eth->h_source[0]&0x1)
	{
		return RTK_FC_RET_LRN_NOT_SUPPORT;
	}

	if((pPktHdr->iph  && FLOW_IPV4_MULTICAST_ADDRESS(ntohl(pPktHdr->iph->saddr))) ||
		(pPktHdr->ip6h && FLOW_IPV6_MULTICAST_ADDRESS(pPktHdr->ip6h->saddr.s6_addr32[0])))
	{
		return RTK_FC_RET_LRN_NOT_SUPPORT;
	}

	if(pPktHdr->iph && FLOW_MULTICAST_RESV_IPV4_ALL(ntohl(pPktHdr->iph->daddr)))
	{
		return RTK_FC_RET_LRN_SKIP_MCPKT;
	}

	if(pPktHdr->ip6h && (FLOW_MULTICAST_RESV_IPV6_ALL(pPktHdr->ip6h->daddr.s6_addr32[0]) /*|| FLOW_MULTICAST6_DATA_NOT_FFXE(pPktHdr->ip6h->daddr.s6_addr32[0]) */))
	{
		return RTK_FC_RET_LRN_SKIP_MCPKT;
	}

	if(pPktHdr->dmacToGatewayMAC && !pPktHdr->iph && !pPktHdr->ip6h)
	{
		// e.g. arp, pppoe discovery
		return RTK_FC_RET_LRN_SKIP_LOCALINOUT;
	}

	if((pPktHdr->udph!=NULL) && ((pPktHdr->udph->source==htons(67) && pPktHdr->udph->dest==htons(68)) || 
		(pPktHdr->udph->source==htons(68) && pPktHdr->udph->dest==htons(67))))
	{
		// skip dhcp request/reply packet.
		return RTK_FC_RET_LRN_NOT_SUPPORT;
	}
/*
	if(pPktHdr->iph && (pPktHdr->iph->protocol == IPPROTO_ICMP)) {
		// skip icmp
		return RTK_FC_RET_LRN_SKIP_NOL4;
	}
*/
	return RTK_FC_RET_OK;
}

__IRAM_FC_SHORTCUT
static rtk_fc_ret_t _rtk_fc_cpuRsnPreProcess(rtk_fc_pktHdr_t *pPktHdr)
{

	/* bypass CPU reason? */

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	uint8 reason = pPktHdr->reason;

	switch(reason)
	{
		//============= L34 =============//
		case CPU_REASON_TCPFLAG:
			return RTK_FC_RET_OK;

		case CPU_REASON_WOTCPUDP:
			if((pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP) && (pPktHdr->tunnelInfo.pptp.outerFlowReady==FALSE))
			{
				return RTK_FC_RET_OK;
			}
			else if(pPktHdr->dualHdrType==RTK_FC_DUALTYPE_IP4INIP6)
			{
				return RTK_FC_RET_OK;
			}
			// support l3 only routing/nat shortcut
			if(pPktHdr->iph || pPktHdr->ip6h)
				return RTK_FC_RET_OK;
			break;

		case CPU_REASON_FLOWMISS:
			// if flow was saved in shortcut, we need to take care following routing packets.
			if(pPktHdr->dmacToGatewayMAC && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_IP4INIP6)
			{
				//support dslite only dowstream direction
				return RTK_FC_RET_OK;
			}

			// CPU_REASON_IPEXT
			if(pPktHdr->dmacToGatewayMAC &&
				(	(pPktHdr->outer_iph && (pPktHdr->outer_iph->ihl!=5)) ||
					(pPktHdr->iph && (pPktHdr->iph->ihl!=5)) ||
					(pPktHdr->ip6h && (pPktHdr->ip6h->nexthdr!=IPPROTO_IPIP) && (pPktHdr->ip6h->nexthdr!=IPPROTO_TCP) && (pPktHdr->ip6h->nexthdr!=IPPROTO_UDP))
				)
			){
				TRACE("EXTHDR, trap to PS");
				return RTK_FC_RET_LRN_NOT_SUPPORT;
			}
			// CPU_REASON_TTL
			if(pPktHdr->dmacToGatewayMAC &&
				(	(pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE && ((pPktHdr->iph && pPktHdr->iph->ttl<2) || (pPktHdr->ip6h && pPktHdr->ip6h->hop_limit<2))) ||
					(((pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP)||(pPktHdr->dualHdrType==RTK_FC_DUALTYPE_L2TP))&& ((pPktHdr->iph && pPktHdr->iph->ttl<2) || (pPktHdr->ip6h && pPktHdr->ip6h->hop_limit<2)))
				)
			){
				TRACE("TTL<2, trap to PS");
				return RTK_FC_RET_LRN_NOT_SUPPORT;
			}
			// CPU_REASON_MTU => check after finding egress netif in shortcut entry

			break;

		//============= L2 =============//
		#if 0	// DA learning at egress phase, so ingress learning should not be blocked.
		case CPU_REASON_UNKNOWN_DA:
			return RTK_FC_RET_LRN_SKIP_FLOODING;
		#endif
		#if 0   // ACL trap: do shortcut check but skip shortcut forwarding, do egress flow learning
		case CPU_REASON_ACL:
			return RTK_FC_RET_LRN_SKIP_CTRLPKT;
		#endif
		default:
			break;
	}

	// Following cases trappd by Hw were also not supported in FC driver. Handled by PS.
	if((reason != CPU_REASON_FRAG) && (reason < CPU_REASON_L34_FWD) && (reason > CPU_REASON_FLOWMISS)){
		return RTK_FC_RET_LRN_NOT_SUPPORT;
	}
#endif

	if (pPktHdr->isFragment) {
		if (fc_db.controlFuc.ip_frag_shortcut) {
			if (pPktHdr->ip6h || (pPktHdr->iph && pPktHdr->iph->protocol != IPPROTO_UDP))
				return RTK_FC_RET_LRN_SKIP_FRAG;
		}
		else {
			return RTK_FC_RET_LRN_SKIP_FRAG;
		}
	}

	return RTK_FC_RET_OK;
}

static rtk_fc_ret_t _rtk_fc_ingress_dataCaching(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{
	rtk_fc_ingress_data_t *pFcIngressData;
	int i;
	
	if(pPktHdr == NULL || pPktHdr->eth == NULL) {
		WARNING("pPktHdr or pPktHdr->eth is NULL!\n");
		return RTK_FC_RET_ERR_NULL_POINTER;
	}

	//allocate new skb, swap skb, enqueue old_skb back to queue, continue to protocol stack
	if(RTSKB_RECYCLABLE(rtskb)) {
		struct sk_buff *new_skb=NULL;
		RTSKB_FCIGRDATA(rtskb)->doLearning=0U;	//omit copy ingressData when swap.
		//printk("RTSKB_FCIGRDATA(rtskb)->doLearning=%d\n",RTSKB_FCIGRDATA(rtskb)->doLearning);
		RTK_FC_HOOK_PS_SKB_SWAP_RECYCLE_SKB(RTSKB_SKB(rtskb), &new_skb);
		if(new_skb){//printk("the recyclable is %d\n",new_skb->recyclable);
#if 0//defined(CONFIG_RTK_FC_IPSEC_FASTFWD)		
			if(pPktHdr->ipsec_sc_info.skb_recovery_for_hwLookup)
			{
				new_skb->sp = RTSKB_SKB(rtskb)->sp;
				new_skb->dev = RTSKB_DEV(rtskb);
			}
#endif			
			RTK_FC_HOOK_CONVERTER_SKB(new_skb, rtskb);
		}else{
			WARNING("fail to get new skb..drop!\n");
			return RTK_FC_RET_ERR_EXHAUST_SKB;
		}
	}

	pFcIngressData = RTSKB_FCIGRDATA(rtskb);

	memset(pFcIngressData,0,sizeof(rtk_fc_ingress_data_t));
	memcpy(pFcIngressData->sa,pPktHdr->eth->h_source,ETH_ALEN);
	memcpy(pFcIngressData->da,pPktHdr->eth->h_dest,ETH_ALEN);

	if(((1<<pPktHdr->ingressPort.macPortIdx)&fc_db.wanPortMask.portmask)!=0)
		pFcIngressData->isDownStream=1;
	if(fc_db.wanPortWifidevMask && (fc_db.wanPortWifidevMask & (1LL << pPktHdr->igrWlanDevIdx)))
		pFcIngressData->isDownStream=1;

	//if packet from wlan ,skb->dev not need update
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	if(pPktHdr->ipsec_sc_info.skb_recovery_for_hwLookup)
	{
		pFcIngressData->ipsec_hwlookup = 1;
		IPSEC("No need to update skb->dev, pFcIngressData->isDownStream = %d",pFcIngressData->isDownStream);
	}
	else 
#endif		
	{
		if(pPktHdr->igrWlanDevIdx==RTK_FC_WLANX_NOT_FOUND)
		    RTK_FC_HELPER_DECIDE_RX_DEV_BY_SPA(pPktHdr->ingressPort.macPortIdx, &RTSKB_DEV(rtskb));
		else
		    RTK_FC_HELPER_WLAN_DEVID_TO_DEV(pPktHdr->igrWlanDevIdx, &RTSKB_DEV(rtskb));

	}

	if(RTSKB_DEV(rtskb))
	{
		for(i=DEVIFIDX_VALID_MIN;i<RTK_FC_DEV_GW_MAC_TBL;i++)
		{
			if(fc_db.devGwMacTbl[i].dev==RTSKB_DEV(rtskb))
				break;
		}
		if(i>=RTK_FC_DEV_GW_MAC_TBL)
		{
			TRACE("Error SrcDev netif Idx i=%d,dev is %s \n",i,RTSKB_DEV(rtskb)->name);
			pFcIngressData->SrcDevDevGwMacIdx = DEVIFIDX_INVALID_MIN;
		}
		else
			pFcIngressData->SrcDevDevGwMacIdx = i;
	}

	pFcIngressData->doLearning = TRUE;

	pFcIngressData->outerHdrUnhit = pPktHdr->outerHdrMiss;
	pFcIngressData->flowHashIdx = pPktHdr->flowHashIdx;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	pFcIngressData->crc32 = pPktHdr->crc32;
#endif

	pFcIngressData->isDAGatewayMAC = pPktHdr->dmacToGatewayMAC & 0x1U;
	pFcIngressData->isDualHeader = (pPktHdr->dualHdrType) ? TRUE : FALSE;

	pFcIngressData->ingressPort = pPktHdr->ingressPort.macPortIdx;
	pFcIngressData->wlan_dev_idx = pPktHdr->igrWlanDevIdx;

	pFcIngressData->ingressDualFailed = pPktHdr->dualFailed;

	if(pPktHdr->svh)
	{
		pFcIngressData->srcSVlanTpid_sel = pPktHdr->stpid_sel;
		pFcIngressData->srcSVlanId = pPktHdr->svlanid;
		pFcIngressData->srcSVlanPri = pPktHdr->svlanpri;
		pFcIngressData->srcSVlanDei = pPktHdr->svlandei;
		pFcIngressData->ingressTagif |= SVLAN_TAGIF;
	}
	if(pPktHdr->outer_svh)
	{
		/*
		pFcIngressData->outer_srcSVlanTpid_sel = pPktHdr->outer_stpid_sel;
		pFcIngressData->outer_srcSVlanId = (ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_VID_MASK);
		pFcIngressData->outer_srcSVlanPri =((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT);
		pFcIngressData->outer_srcSVlanDei =((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT);
		*/
		pFcIngressData->ingressTagif |= OUTER_SVLAN_TAGIF;

	}

	if(pPktHdr->cvh)
	{
		pFcIngressData->srcCVlanId = pPktHdr->cvlanid;
		pFcIngressData->srcCVlanPri = pPktHdr->cvlanpri;
		pFcIngressData->srcCVlanCfi = pPktHdr->cvlancfi;
		pFcIngressData->ingressTagif |= CVLAN_TAGIF;
	}
	if(pPktHdr->outer_cvh)
	{
	/*
		pFcIngressData->outer_srcCVlanId = (ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK);
		pFcIngressData->outer_srcCVlanPri = ((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT);
		pFcIngressData->outer_srcCVlanCfi =((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT);
	*/
		pFcIngressData->ingressTagif |= OUTER_CVLAN_TAGIF;
	}
	if(pPktHdr->ppph)
	{
		pFcIngressData->sessionId = ntohs(pPktHdr->ppph->sid);
		pFcIngressData->ingressTagif |= PPPOE_TAGIF;
	}
	if(pPktHdr->outer_ppph)
	{
		pFcIngressData->outer_sessionId = ntohs(pPktHdr->outer_ppph->sid);
		pFcIngressData->ingressTagif |= OUTER_PPPOE_TAGIF;
	}

	pFcIngressData->dualPattenHashKey = pPktHdr->dualHdrHashKey;
	// save dual header tagif
	if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP){
		pFcIngressData->ingressTagif|= PPTP_TAGIF;
		//For Recording Call id, Protocol stack might change call id when down stream, so we need to record ingress call id
		// to know whether PS has changed call id or not
		if(pPktHdr->tunnelInfo.pptp.pCallId)
		{
			pFcIngressData->ingressCallid = pPktHdr->tunnelInfo.pptp.grecallid;
		}else
			pFcIngressData->ingressCallid = -1;
	}
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP)
		pFcIngressData->ingressTagif|= L2TP_TAGIF;
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IP4INIP6)
		pFcIngressData->ingressTagif|= IP4_IN_IP6;
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_6RD)
		pFcIngressData->ingressTagif|= V6RD_TAGIF;
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IPSEC)
	{
		pFcIngressData->ingressTagif|= IPSEC_ESP_TAGIF;
		
		pFcIngressData->ipsec_info.ingress_spi		= (pPktHdr->esph)?ntohl(pPktHdr->esph->spi):0;
		pFcIngressData->ipsec_info.ingress_seq_num 	= (pPktHdr->esph)?ntohl(pPktHdr->esph->seq_no):0;
	}
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN){
		pFcIngressData->ingressTagif|= VXLAN_TAGIF;

		pFcIngressData->vxlan_info.outer_tag_len = pPktHdr->vxlan_info.outerTag_off;
		pFcIngressData->vxlan_info.outer_isV6	= pPktHdr->vxlan_info.outer_isV6;
#if defined(CONFIG_FC_RTL9607C_SERIES)||defined(CONFIG_FC_CA8277B_SERIES)
		if(pPktHdr->outer_udph)
			pFcIngressData->vxlan_info.outer_srcPort	= ntohs(pPktHdr->outer_udph->source);
#endif
	}
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_GRE_ETH_BR){
		pFcIngressData->ingressTagif|= GRE_ETH_BR_TAGIF;
	}

	pFcIngressData->l4protol = pPktHdr->l4protol;
	if(pFcIngressData->outerHdrUnhit)
	{
		TRACE("ingress data for outer header");
		if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP){
			pFcIngressData->srcIp = ntohl(pPktHdr->outer_iph->saddr);
			pFcIngressData->dstIp = ntohl(pPktHdr->outer_iph->daddr);
			pFcIngressData->ingressTagif|= IPV4_TAGIF;
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP){
			pFcIngressData->srcIp = ntohl(pPktHdr->outer_iph->saddr);
			pFcIngressData->dstIp = ntohl(pPktHdr->outer_iph->daddr);
			pFcIngressData->ingressTagif|= IPV4_TAGIF;

			pFcIngressData->srcPort = ntohs(pPktHdr->outer_udph->source);
			pFcIngressData->dstPort = ntohs(pPktHdr->outer_udph->dest);
			pFcIngressData->ingressTagif|= UDP_TAGIF;
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IP4INIP6)
		{
			pFcIngressData->srcIp = pPktHdr->ipv6Sip_hash;
			pFcIngressData->dstIp = pPktHdr->ipv6Dip_hash;
			pFcIngressData->ingressTagif|= IPV6_TAGIF;
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_6RD){
			pFcIngressData->srcIp = ntohl(pPktHdr->outer_iph->saddr);
			pFcIngressData->dstIp = ntohl(pPktHdr->outer_iph->daddr);
			pFcIngressData->ingressTagif|= IPV4_TAGIF;
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN)
		{

			if(pPktHdr->iph)
			{
				pFcIngressData->srcIp = ntohl(pPktHdr->iph->saddr);
				pFcIngressData->dstIp = ntohl(pPktHdr->iph->daddr);
				pFcIngressData->ingressTagif|= IPV4_TAGIF;
			}
			
			if(pPktHdr->tcph)
			{
				pFcIngressData->srcPort = ntohs(pPktHdr->tcph->source);
				pFcIngressData->dstPort = ntohs(pPktHdr->tcph->dest);
				pFcIngressData->ingressTagif|= TCP_TAGIF;
			}
			else if(pPktHdr->udph)
			{
				pFcIngressData->srcPort = ntohs(pPktHdr->udph->source);
				pFcIngressData->dstPort = ntohs(pPktHdr->udph->dest);
				pFcIngressData->ingressTagif|= UDP_TAGIF;
			}		
		}

	}
	else
	{
		TRACE("ingress data for single header or inner in dual case");		
		if(pPktHdr->iph)
		{
			pFcIngressData->srcIp = ntohl(pPktHdr->iph->saddr);
			pFcIngressData->dstIp = ntohl(pPktHdr->iph->daddr);
			pFcIngressData->ingressTagif|= IPV4_TAGIF;
			pFcIngressData->ipId = ntohs(pPktHdr->iph->id);
			pFcIngressData->ipFragFlag = pPktHdr->ipFragFlag;
		}
		else if(pPktHdr->ip6h)
		{
			pFcIngressData->srcIp = pPktHdr->ipv6Sip_hash;
			pFcIngressData->dstIp = pPktHdr->ipv6Dip_hash;
			pFcIngressData->ingressTagif|= IPV6_TAGIF;									
		}
		if(pPktHdr->ip6h)
		{
			if( ((1<<pPktHdr->ingressPort.macPortIdx) & fc_db.wanPortMask.portmask) || 
				((pPktHdr->igrWlanDevIdx!=RTK_FC_WLANX_NOT_FOUND) && (fc_db.wanPortWifidevMask & (1LL << pPktHdr->igrWlanDevIdx))) 
			) // From wan, maybe it's downstream
			{
				memcpy(&pFcIngressData->ingress_v6ip_info, &pPktHdr->ip6h->daddr, sizeof(struct in6_addr));
				DEBUG("ingress v6 dip : %pI6c",&pFcIngressData->ingress_v6ip_info.s6_addr32[0]);
			}
			else// From lan, maybe it's upstream
			{
				memcpy(&pFcIngressData->ingress_v6ip_info, &pPktHdr->ip6h->saddr, sizeof(struct in6_addr));
				DEBUG("ingress v6 sip : %pI6c",&pFcIngressData->ingress_v6ip_info.s6_addr32[0]);
			}
		}

		if(pPktHdr->tcph)
		{
			pFcIngressData->srcPort = ntohs(pPktHdr->tcph->source);
			pFcIngressData->dstPort = ntohs(pPktHdr->tcph->dest);
			pFcIngressData->ingressTagif|= TCP_TAGIF;
		}
		else if(pPktHdr->udph)
		{
			pFcIngressData->srcPort = ntohs(pPktHdr->udph->source);
			pFcIngressData->dstPort = ntohs(pPktHdr->udph->dest);
			pFcIngressData->ingressTagif|= UDP_TAGIF;
		}else if(pPktHdr->esph)
		{
			pFcIngressData->ingressTagif|= IPSEC_ESP_TAGIF;
		}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		else
		{
			// pure L3
			pFcIngressData->srcPort = pPktHdr->pureIp_l4_info.src_port;
			pFcIngressData->dstPort = pPktHdr->pureIp_l4_info.dst_port;
		}
#endif

	}

	if(pPktHdr->ip6h)
	{
		pFcIngressData->v6tos = (((pPktHdr->ip6h->priority<<4)&0xf0)|((pPktHdr->ip6h->flow_lbl[0]>>4)&0xf));
	}
	else if(pPktHdr->iph)
	{
		pFcIngressData->v4tos = pPktHdr->iph->tos;
	}
	if(pPktHdr && ((pPktHdr->icmph)|| (pPktHdr->icmp6h))  )
	{	
		pFcIngressData->ingressTagif|= ICMP_TAGIF;
		if(pPktHdr->icmph && (pPktHdr->icmph->type == 8 || pPktHdr->icmph->type == 0) ) //v4 request/reply
		{
			pFcIngressData->icmp_id = pPktHdr->icmph->un.echo.id;
		}
		else if(pPktHdr->icmp6h && (pPktHdr->icmp6h->icmp6_type == 128 || pPktHdr->icmp6h->icmp6_type == 129) ) // v6 request/reply
		{
			pFcIngressData->icmp_id = pPktHdr->icmp6h->icmp6_identifier;
		}
	}
	
	pFcIngressData->isHitSwFwdedAclTrapPri = pPktHdr->isHitSwFwdedAclTrapPri;
	pFcIngressData->ponStreamId = pPktHdr->ponstreamid;

	TRACE("prepare inbound info in skb ... flowHashIdx = %d", pFcIngressData->flowHashIdx);
	return RTK_FC_RET_OK;
}

static rtk_fc_nic_rx_t _rtk_fc_ingress_slowPath(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, fc_rx_info_t *pRxInfo)
{
	int ret = RTK_FC_NIC_RX_CONTINUE;

	//Don't move it, this preparation is for sa/flow learning.
	_rtk_fc_learning_preparation(rtskb, pPktHdr);

	_rtk_fc_igmp_mld_ingressLan_learning(pPktHdr, pPktHdr->ingressPort, pPktHdr->igrWlanDevIdx);

	//Retrieve wifi device before enter PS if needed
	if(pPktHdr->igrWlanDevIdx!=RTK_FC_WLANX_NOT_FOUND && pPktHdr->fromWlan_noHWlookup==0 && RXINFO_REASON(pRxInfo)!=CPU_REASON_L34_FWD)
		ret = rtk_fc_fromWifiToProtocolStack(rtskb, pPktHdr);	// do rtk_fc_sw_shaper inside

	if(RTSKB_DEV(rtskb))
		TRACE("continue rx to ps ... dev %s", RTSKB_DEV(rtskb)->name);

	if(unlikely((fc_db.filterLevel&fc_db.debugLevel) && (fc_db.filterLevel&FC_DEBUG_LEVEL_TOPSTRACE)))
	{
		RTSKB_FCIGRDATA(rtskb)->traceFilterMatchShow = fc_db.tracefilterShow ;
		TRACE("to Ps traceFilterMatchShow:%d", RTSKB_FCIGRDATA(rtskb)->traceFilterMatchShow);
	}
	
	//update counter
	if(unlikely(fc_db.smpStatistic))
		atomic_inc(&fc_db.smp_statistic[FC_SMP_TO_PS_TX][smp_processor_id()]);
	if(unlikely(fc_db.fwdStatistic))
	{
		if(RTSKB_DATA(rtskb)[0]==0xff)
			atomic_inc(&fc_db.statistic.perPortCnt_ToPS_broadcast[pPktHdr->ingressPort.macPortIdx]);
		else if(RTSKB_DATA(rtskb)[0]&0x1)
			atomic_inc(&fc_db.statistic.perPortCnt_ToPS_multicast[pPktHdr->ingressPort.macPortIdx]);
		else
			atomic_inc(&fc_db.statistic.perPortCnt_ToPS_unicast[pPktHdr->ingressPort.macPortIdx]);
	}

	return ret;
}

void rtk_fc_egress_portDecision(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, int pmask, rtk_fc_wlan_devidx_t wlanDevIdx)
{
	// ingress info
	pPktHdr->igrWlanDevIdx = RTSKB_FCIGRDATA(rtskb)->wlan_dev_idx;

	if(pPktHdr->igrWlanDevIdx < RTK_FC_WLANX_END_INTF) {
		RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pPktHdr->igrWlanDevIdx, &pPktHdr->ingressPort.macPortIdx , &pPktHdr->ingressPort.macExtPortIdx);
			// do error handling if not found ??
	}else {

		pPktHdr->ingressPort.macPortIdx = RTSKB_FCIGRDATA(rtskb)->ingressPort;
		pPktHdr->ingressPort.macExtPortIdx = RTK_FC_MAC_EXT_CPU;
	}

	// egress info
	pPktHdr->egrWlanDevIdx = wlanDevIdx;

	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) {
		RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pPktHdr->egrWlanDevIdx, &pPktHdr->egressPort.macPortIdx , &pPktHdr->egressPort.macExtPortIdx);
			// do error handling if not found ??
	}else {

		pPktHdr->egressPort.macPortIdx = __ffs(pmask);		// 0x2 = 0b0010, __ffs(0x2) = 1
		pPktHdr->egressPort.macExtPortIdx = RTK_FC_MAC_EXT_NONE;
	}

}

__IRAM_FC_SHORTCUT
static rtk_fc_ret_t rtk_fc_ingress_portDecision(rtk_fc_pktHdr_t *pPktHdr, fc_rx_info_t *pRxDesc, struct rt_skbuff *rtskb)
{

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	int16 l2Idx;
	struct ethhdr *eth = (struct ethhdr *)RTSKB_DATA(rtskb);
	if(RXINFO_REASON(pRxDesc) == CPU_REASON_FLOW_COPY)
	{
		if((eth->h_dest[0]&1) && (_rtk_fc_lut_find(eth->h_source, &l2Idx)==RTK_FC_RET_OK))
		{
		
			pPktHdr->ingressPort.macPortIdx = RXINFO_SPA(pRxDesc) =  fc_db.lutTbl[l2Idx]->spa;
			if(fc_db.lutTbl[l2Idx]->wlan_dev_idx < RTK_FC_WLANX_END_INTF)
			{
				pPktHdr->ingressPort.macExtPortIdx = fc_db.lutTbl[l2Idx]->wlan_dev_idx;
			}
			else
				RXINFO_CPU_EXTSPA(pRxDesc) = 0;		

			if(pPktHdr->ingressPort.macPortIdx == RTK_FC_MAC_PORT_PON)
			{
				FIXME("how to recover streamid !?");
			}
			RTK_FC_HELPER_DECIDE_RX_DEV_BY_SPA(pPktHdr->ingressPort.macPortIdx, &RTSKB_DEV(rtskb));
			DEBUG("change copy to cpu entry pRxDesc to spa[%d] dev[%s]",fc_db.lutTbl[l2Idx]->spa,RTSKB_DEV(rtskb)->name);
		}
	}
#endif

	pPktHdr->ingressPort.macPortIdx = RXINFO_SPA(pRxDesc);
	
	if(pPktHdr->ingressPort.macPortIdx == RTK_FC_MAC_PORT_PON)
		pPktHdr->ponstreamid = RXINFO_PON_SID(pRxDesc);

#if defined(CONFIG_FC_RTL8277C_SERIES)
	else
	{
		/*
		   8277C only, 9607F HW fixed.
		   MTU check failed: flow hit (mdata_sw_id is changed)
		   - mdata_sw_id may be rx (streamId or wlanIdx) or tx (wlanIdx) info
		   - recover Wifi RX wlanId by LUT.
		*/

		if(RXINFO_REASON(pRxDesc) == CPU_REASON_MTU)
		{
			// recover wifi rx wlanIdx by Lut
			if(_rtk_fc_lut_find(eth->h_source, &l2Idx)==RTK_FC_RET_OK)
			{
				DEBUG("HW MTU check failed!!! recover wlan_dev_idx (%d) by LUT", fc_db.lutTbl[l2Idx]->wlan_dev_idx);
				if(fc_db.lutTbl[l2Idx]->wlan_dev_idx < RTK_FC_WLANX_END_INTF)
				{
					pPktHdr->ingressPort.macExtPortIdx = fc_db.lutTbl[l2Idx]->wlan_dev_idx;
				}
				else
					RXINFO_CPU_EXTSPA(pRxDesc) = 0;
			}
			else
			{
				DEBUG("[ERROR] HW MTU check failed but DMAC LUT lookup miss, can not recover rx SW_ID. DROP it!!! ");
				return RTK_FC_RET_DROP_MTU_FAIL_NO_SW_ID_IFNO;
			}
		}
		else if (RXINFO_CPU_EXTSPA(pRxDesc))	//not from port 7 and include extspa - wlan dev idx
			pPktHdr->ingressPort.macExtPortIdx = RXINFO_CPU_EXTSPA(pRxDesc);
	}
#elif defined(CONFIG_FC_RTL9607F_SERIES)
	else if (RXINFO_CPU_EXTSPA(pRxDesc))	//not from port 7 and include extspa - wlan dev idx
			pPktHdr->ingressPort.macExtPortIdx = RXINFO_CPU_EXTSPA(pRxDesc);
#else
	if (((1<<pPktHdr->ingressPort.macPortIdx) & RTK_FC_ALL_MAC_CPU_PORTMASK))
		pPktHdr->ingressPort.macExtPortIdx = RXINFO_CPU_EXTSPA(pRxDesc);
#endif
	return RTK_FC_RET_OK;

}
#if defined(CONFIG_FC_RTL9607C_SERIES)
void _rtk_fc_nptv6_preTXdesc_set(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb )
{
	if(pPktHdr)
	{
		fc_tx_info_t *ptxInfo;
		uint8 ip_summed;
		uint32 portMask = 1<<pPktHdr->egressPort.macPortIdx;
		ptxInfo = &fc_db.txDesc;
		
		memset(ptxInfo, 0, sizeof(fc_tx_info_t));

		TXINFO_EOR(ptxInfo) 		= 0U;
		TXINFO_FS(ptxInfo)			= 1U;
		TXINFO_LS(ptxInfo)			= 1U;
		RTK_FC_HOOK_PS_SKB_IP_SUMMED_GET(RTSKB_SKB(rtskb), &ip_summed);
		if(ip_summed != CHECKSUM_NONE) {
			
			// CHECKSUM_NONE/Tx: The skb was already checksummed by the protocol, or a checksum is not required.
			TXINFO_IPCS(ptxInfo)			= 1U;
			TXINFO_L4CS(ptxInfo)			= 1U;
		}
		TXINFO_CRC(ptxInfo) 		= 1U;
		TXINFO_DATA_LEN(ptxInfo)	= RTSKB_LEN(rtskb);
		TXINFO_CPUTAG(ptxInfo)		= 1U;
		TXINFO_TPID_SEL(ptxInfo)	= 1U;
		



		// TPID SELECTION
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if(pPktHdr->remarkDec.svlan_tpid_from_skbmark== RTK_FC_TPID_FROM_SKBMARK_USE_STAG_1)
		{
			TXINFO_TPID_SEL(ptxInfo)	= 1;
		}
#endif
		// SVLAN RELATED SETTING
		
		TXINFO_SVLAN_ACT(ptxInfo)	= (pPktHdr->svh)?0x3U:0x2U;
		
			
		TXINFO_SVLAN_VIDL(ptxInfo)	= (pPktHdr->svlanid&0xffU);
		TXINFO_SVLAN_PRIO(ptxInfo)	= pPktHdr->svlanpri;
		TXINFO_SVLAN_VIDH(ptxInfo)	= ((pPktHdr->svlanid&0xf00U)>>8);
		TXINFO_STAG_AWARE(ptxInfo)	= 1U;



		// CVLAN RELATED SETTING
		TXINFO_CVLAN_ACT(ptxInfo)	=  (pPktHdr->cvlanid)?0x3U:0x2U;

		TXINFO_CVLAN_VIDL(ptxInfo)	= (pPktHdr->cvlanid&0xffU);
		TXINFO_CVLAN_PRIO(ptxInfo)	= pPktHdr->cvlanpri;
		TXINFO_CVLAN_VIDH(ptxInfo)	= ((pPktHdr->cvlanid&0xf00U)>>8);

		
		TXINFO_SVLAN_CFI(ptxInfo)		= 0U;
		TXINFO_CVLAN_CFI(ptxInfo)		= 0U;
		TXINFO_TX_PORTMSK(ptxInfo)		= portMask;
		//printk("TXINFO_TX_PORTMSK(ptxInfo) = %x\n",portMask);

		TXINFO_ASPRI(ptxInfo)		= 1U;
		TXINFO_CPUTAG_PRI(ptxInfo)	= pPktHdr->remarkDec.outputQid;
		TXINFO_KEEP(ptxInfo)		= 0U;
		TXINFO_DISLRN(ptxInfo)		= 1U;
		if(portMask & fc_db.wanPortMask.portmask){
			TXINFO_CPUTAG_PSEL(ptxInfo) 	= pPktHdr->remarkDec.streamId_en;
			TXINFO_TX_DST_STREAM_ID(ptxInfo)= pPktHdr->remarkDec.streamId;
		}
		TXINFO_L34_KEEP(ptxInfo)		= 0U;
		TXINFO_GMAC_ID(ptxInfo) 		= 0U;
		TXINFO_EXTSPA(ptxInfo)			= 0U;
		TXINFO_TX_PPPOE_ACT(ptxInfo)	= 0U;
		TXINFO_TX_PPPOE_IDX(ptxInfo)	= 0U;

	}

}
void _rtk_fc_nptv6_acceleration_mechanism_sramMode_check(rtk_fc_pktHdr_t *pPktHdr, 
																	   rtk_rg_asic_path5_entry_t *pPath5Entry, 
																	   int32 indMacIdx, 
																	   int direction, 
																	   struct rt_skbuff *rtskb, 
																	   int *pDmacL2Idx)
{
	int i, freeIdx=FAIL, first_freeIdx=FAIL, matchIdx=FAIL;
	struct rtl8686_hwnat_customized_entry customized_entry={0};

	if(direction==RTK_FC_V6NAT_DIRECTION_UPSTREAM) // outbound
	{
		uint32 ipv6_hdr_len=40U, pppoeOffset=((pPktHdr->ppph)?8U:0U);
		uint8 ipv6_hdr_offset = 14+pppoeOffset;
		rtk_mac_t upstream_smac; // wan gmac
		rtk_mac_t upstream_dmac; // remote mac
		rtk_ipv6_addr_t upstream_sipv6; 
		rtk_ipv6_addr_t upstream_dipv6;
		uint8 upstream_isTcp;


		if(pPktHdr->ip6h)
		{	
			memcpy(upstream_sipv6.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
			memcpy(upstream_dipv6.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
		}
		else
		{
			WARNING("No ip6h!?");
			return;
		}
		memcpy(upstream_smac.octet, pPktHdr->eth->h_source, ETHER_ADDR_LEN);
		memcpy(upstream_dmac.octet, pPktHdr->eth->h_dest, ETHER_ADDR_LEN);
		upstream_isTcp = (pPktHdr->tcph)?1U:0U;
		
		//WARNING("[NPTv6]upstream_sipv6.ipv6_addr =  %pI6c pPktHdr->ip6h->saddr.in6_u.u6_addr8 = %pI6c", &upstream_sipv6.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8);
		//WARNING("[NPTv6]upstream_dipv6.ipv6_addr =  %pI6c pPktHdr->ip6h->daddr.in6_u.u6_addr8 = %pI6c", &upstream_dipv6.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8);
#if 1//defined(CONFIG_CMCC)//CMCC		
		if((fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum==0) || ((pPktHdr->ingressPort.macPortIdx & 0x1)==0)) //1 Src port number is even
#elif defined(CONFIG_YUEME)

		if((fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum==0) || (0<=pPktHdr->ingressPort.macPortIdx   && pPktHdr->ingressPort.macPortIdx <=2 ) ) //1 port number is 0~2
#endif
		{
			for(i=0; i<MAX_NPTV6_ACC_UPSTREAM_NIC_FLOW_NUM; i++)
			{
				if(fc_db.controlFuc.hwnat_customize)
				{
					if(i<fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].preallocated_outer_length)
					{
						if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].isTcp!=upstream_isTcp
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].sipv6.ipv6_addr, upstream_sipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].dipv6.ipv6_addr, upstream_dipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].smac.octet, upstream_smac.octet, ETHER_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].dmac.octet, upstream_dmac.octet, ETHER_ADDR_LEN))
							continue;

						matchIdx = i;
						break;
					}
				}
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].vaild==0)
				{
					if(first_freeIdx==FAIL)
						first_freeIdx = i;

					continue;
				}
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx] && 
					pPktHdr->ip6h &&
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN) && 
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].ipv6_addr.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN))
				{
					if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].ipv6_hdr_offset!=ipv6_hdr_offset)
						continue;

					matchIdx = i;
					break;
				}
			}
			
			if(matchIdx!=FAIL)
			{
				TRACE("Match nptv6 upstream acc idx[%d]", matchIdx);
				freeIdx = matchIdx;
			}
			else if(first_freeIdx!=FAIL)
			{
				TRACE("Find free nptv6 upstream acc idx[%d]", first_freeIdx);
				freeIdx = first_freeIdx;
			}
			if(freeIdx!=FAIL)
 			{
				rtk_fc_pktHdr_t tmp_pktHdr;
				struct ethhdr tmp_eth;
				int ret = 0;
				int32  virtual_indMacIdx = -1;
				int16 lutIdx = 0;
				if(pPktHdr->ip6h)
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].ipv6_addr.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN);
				else
				{
					WARNING("lut table[%d] is NULL!!",pPktHdr->dmacL2Idx);
					return;
				}
				tmp_pktHdr.eth = &tmp_eth;
				memcpy(&tmp_pktHdr.eth->h_dest[0], fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr,6);
				tmp_pktHdr.eth->h_dest[0] = 0x0U;
				tmp_pktHdr.eth->h_dest[1] = 0x0U;
				tmp_pktHdr.eth->h_dest[2] = 0x5eU;
				tmp_pktHdr.eth->h_dest[3] = freeIdx;

				if(fc_db_fastFwd_data.nptv6_acc.upstreamExtport == FC_EXT_MAC10_PORT1)
				{	
					//FC_EXT_MAC10_PORT1
					tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
					tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT1;
				}
			
				
				tmp_pktHdr.cvlanid = 0U;
				tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
				if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
				{
					DEBUG("[NPTv6]LUT Learned before, no need to learn again.");
					ret = RTK_FC_RET_OK;
				}
				else
				{

					if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
												FALSE, TRUE, &lutIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) 
					{
						WARNING("LUT DA learning is fail, ret = 0x%x", ret);
					}

				}
				
				
				if(ret==RTK_FC_RET_OK)
				{
					fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx = lutIdx;
					ret = rtk_fc_indMac_idx_get((int16)fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx, &virtual_indMacIdx);
					//if(fc_db.indMacTbl[virtual_indMacIdx].valid)
					//	fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
					if(ret == RTK_FC_RET_OK)
					{
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].vaild = 1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].ipv6_hdr_offset = ipv6_hdr_offset;
						//fill desc
						memcpy(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo, &fc_db.txDesc, sizeof(struct tx_info));
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.gmac_id=fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=RTK_RG_CPUTAG_PPPOEACT_KEEP;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.tx_tx_cvlan_action=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.tx_tx_svlan_action=0;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.ipcs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.l4cs=0U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.fs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.ls=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.crc=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts2.bit.cputag=1U;
						
						if(fc_db.controlFuc.hwnat_customize)
						{
							if(freeIdx < fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum)
							{
								if(dynamic_sram_desc)
								{
									if(pPktHdr->ip6h)
									{	
										// Check whether outer flow can be added.
										//if(_rtk_rg_flowHashIPv6SrcAddr_get(upstream_sipv6.ipv6_addr) != _rtk_rg_flowHashIPv6SrcAddr_get(pPktHdr->ip6h->saddr.in6_u.u6_addr8))
										if(_rtk_rg_flowHashIPv6SrcAddr_get(upstream_sipv6.ipv6_addr) != RTSKB_FCIGRDATA(rtskb)->srcIp)
										{
											memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo, 0, sizeof(struct tx_info));
											fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.gmac_id = fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
											fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.extspa = fc_db_fastFwd_data.nptv6_acc.upstreamExtport; 
											fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.stag_aware = 1;
										}
										else
										{
											printk("\033[1;33;40m[Notice] NPTv6 upstream flows(SIP/EXT_SIP) are conflict, please change prefix of related IPv6 domain or can not do cvlan HW offload. \033[0m\n");
										}

									}
								}
								if(matchIdx==FAIL && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer_length==0)
								{
									customized_entry.valid=TRUE;
									customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
									customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum - freeIdx;
									customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.upstreamTxRingNum - freeIdx;
									customized_entry.txPreLen=14+ipv6_hdr_len+2;
									customized_entry.txInfo_addr_offset_v1=RX_OFFSET;
									customized_entry.txInfo_addr_offset_v2=RX_OFFSET+customized_entry.txPreLen+pppoeOffset;
									customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo;
									customized_entry.type=CUSTOMIZE_TYPE_NPTV6_UP;
									customized_entry.rx_ext_pmsk=FC_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK;

									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].sipv6.ipv6_addr, upstream_sipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].dipv6.ipv6_addr, upstream_dipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].smac.octet, upstream_smac.octet, ETHER_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].dmac.octet, upstream_dmac.octet, ETHER_ADDR_LEN);
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].isTcp = upstream_isTcp;
									
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer_length = customized_entry.txPreLen;				
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer, RTSKB_DATA(rtskb), 12);
									*(uint16 *)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer + 12) = htons(0x86dd);

									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer+14, &RTSKB_DATA(rtskb)[pPktHdr->l3Offset], ipv6_hdr_len);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer+14+ipv6_hdr_len, &RTSKB_DATA(rtskb)[pPktHdr->l4Offset], 2);

									fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer_length;

									if (unlikely(fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length > MAX_NPTV6_OUTER_LEN))
										fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length = MAX_NPTV6_OUTER_LEN;

									memcpy(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[freeIdx].preallocated_outer, fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length);
							
									DEBUG("txPreLen=%d txInfo_addr_offset_v1=%d txInfo_addr_offset_v2=%d", customized_entry.txPreLen, customized_entry.txInfo_addr_offset_v1, customized_entry.txInfo_addr_offset_v2);
									
									DUMP_PACKET(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length, "Upstream Even Sort Port");

									re8686_customized_rx_and_tx(customized_entry, NULL, _rtk_fc_nptv6_customized_txPrepare, nic_rx_skb_NPTv6FastForward, (pppoeOffset)?_rtk_fc_nptv6_pppoe_customized_TxHook:_rtk_fc_nptv6_ipoe_customized_TxHook);
								}
							}
							else
							{
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
							}
						}
						else
						{
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
						}
						
						pPktHdr->nptv6_acceleration_upstream_idx = freeIdx;
						
						//For Adding flow, use fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[pPktHdr->nptv6_acceleration_upstream_idx].virtual_dmacL2Idx when adding flow
						*pDmacL2Idx = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx; //For Adding flow, use fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[pPktHdr->nptv6_acceleration_upstream_idx].virtual_dmacL2Idx when adding flow
						pPath5Entry->out_dmac_idx = virtual_indMacIdx;
						pPath5Entry->out_ctag_format_act = TRUE;
						pPath5Entry->out_cvid_format_act = FALSE;
						pPath5Entry->out_cpri_format_act = FALSE;
						pPath5Entry->out_stag_format_act = TRUE;
						pPath5Entry->out_svid_format_act = FALSE;
						pPath5Entry->out_spri_format_act = FALSE;
						pPath5Entry->out_user_pri_act = TRUE;
						pPath5Entry->out_user_priority = (freeIdx < fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum) ? (fc_db_fastFwd_data.nptv6_acc.upstreamPriority-freeIdx) : fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg;
						TRACE("Accelerate nptv6 upstream[%d] by indMacIdx[%d]", pPktHdr->nptv6_acceleration_upstream_idx, virtual_indMacIdx);
						//assert_ok(RTK_RG_ASIC_INDIRECTMACTABLE_DEL(indMacIdx));
						
						//if(fc_db.indMacTbl[indMacIdx].valid)
						//	fc_db.indMacTbl[indMacIdx].indMacRefCount--;
					}
					else
					{
						TRACE("[Fail to add flow] Fail to add indirect mac table for nptv6 acceleration");
						
						//assert_ok((pf.rtk_rg_macEntry_del)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx)); 			
						if(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx])
						{
							RTK_FC_LUT_DEL(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx], FALSE);
						}
					}
				}
				else
				{
					WARNING("LUT DA learning is fail, ret = 0x%x", ret);
				}
			}
		}
#if 1//defined(CONFIG_CMCC) //CMCC
		else    //1 Src port number is odd
#elif defined(CONFIG_YUEME)

		else if(pPktHdr->ingressPort.macPortIdx == 3)	//1 Src port number is port 3
#endif
		{
			for(i=0; i<MAX_NPTV6_ACC_UPSTREAM0_NIC_FLOW_NUM; i++)
			{
				if(fc_db.controlFuc.hwnat_customize)
				{
					if(i<fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].preallocated_outer_length)
					{
						if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].isTcp!=upstream_isTcp
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].sipv6.ipv6_addr, upstream_sipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].dipv6.ipv6_addr, upstream_dipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].smac.octet, upstream_smac.octet, ETHER_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].dmac.octet, upstream_dmac.octet, ETHER_ADDR_LEN))
							continue;

						matchIdx = i;
						break;
					}
				}
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[i].vaild==0)
				{
					if(first_freeIdx==FAIL)
						first_freeIdx = i;

					continue;
				}
				if( fc_db.lutTbl[pPktHdr->dmacL2Idx] &&
					pPktHdr->ip6h &&
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[i].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN) &&
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[i].ipv6_addr.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN))
				{
					if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[i].ipv6_hdr_offset!=ipv6_hdr_offset)
						continue;

					matchIdx = i;
					break;
				}
			}
			if(matchIdx!=FAIL)
			{
				TRACE("Match nptv6 upstream0 acc idx[%d]", matchIdx);
				freeIdx = matchIdx;
			}
			else if(first_freeIdx!=FAIL)
			{
				TRACE("Find free nptv6 upstream0 acc idx[%d]", first_freeIdx);
				freeIdx = first_freeIdx;
			}
			if(freeIdx!=FAIL)
			{
				rtk_fc_pktHdr_t tmp_pktHdr;
				struct ethhdr tmp_eth;
				int ret = 0;
				int virtual_indMacIdx = -1;
				int16 lutIdx = 0;

				if(pPktHdr->ip6h)
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].ipv6_addr.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN);

				if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN);
				else
				{
					WARNING("lut table[%d] is NULL!!",pPktHdr->dmacL2Idx);
					return;
				}
				tmp_pktHdr.eth = &tmp_eth;
				memcpy(&tmp_pktHdr.eth->h_dest[0], fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr,6);

				tmp_pktHdr.eth->h_dest[0] = 0x0U;
				tmp_pktHdr.eth->h_dest[1] = 0x1U;
				tmp_pktHdr.eth->h_dest[2] = 0x5eU;
				tmp_pktHdr.eth->h_dest[3] = freeIdx;

				if(fc_db_fastFwd_data.nptv6_acc.upstream0Extport == FC_EXT_MAC7_PORT1)
				{	
					//FC_EXT_MAC10_PORT1
					tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
					tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT1;
				}
				tmp_pktHdr.cvlanid = 0U;
				tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;


				if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
 			    {
 					DEBUG("[NPTv6]LUT Learned before, no need to learn again.");
					ret = RTK_FC_RET_OK;
 			    }
				else
				{
					if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
												FALSE, TRUE, &lutIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) 
					{
						WARNING("LUT DA learning is fail, ret = 0x%x", ret);
					}

				}

				if(ret==RTK_FC_RET_OK)
				{
					fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].virtual_dmacL2Idx = lutIdx;
					ret = rtk_fc_indMac_idx_get((int16)fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].virtual_dmacL2Idx, &virtual_indMacIdx);
					//if(fc_db.indMacTbl[virtual_indMacIdx].valid)
					//	fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
					if(ret == RTK_FC_RET_OK)
					{
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].vaild = 1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].ipv6_hdr_offset = ipv6_hdr_offset;
						//fill desc
						memcpy(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo, &fc_db.txDesc, sizeof(struct tx_info));
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.gmac_id=fc_db_fastFwd_data.nptv6_acc.upstream0Gmac;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_action=RTK_RG_CPUTAG_PPPOEACT_KEEP;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.tx_tx_cvlan_action=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.tx_tx_svlan_action=0;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts1.bit.ipcs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts1.bit.l4cs=0U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts1.bit.fs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts1.bit.ls=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts1.bit.crc=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts2.bit.cputag=1U;

						if(fc_db.controlFuc.hwnat_customize)
						{
							
							if(freeIdx < fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum)
							{
								if(dynamic_sram_desc)
								{	
									if(pPktHdr->ip6h)
									{	
										if(_rtk_rg_flowHashIPv6SrcAddr_get(upstream_sipv6.ipv6_addr) != RTSKB_FCIGRDATA(rtskb)->srcIp)
										{
											memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo, 0, sizeof(struct tx_info));
											fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.gmac_id = fc_db_fastFwd_data.nptv6_acc.upstream0Gmac;
											fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.extspa = fc_db_fastFwd_data.nptv6_acc.upstream0Extport; 
											fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts1.bit.stag_aware = 1;
										}
										else
										{
											printk("\033[1;33;40m[Notice] NPTv6 upstream0 flows(SIP/EXT_SIP) are conflict, please change prefix of related IPv6 domain or can not do cvlan HW offload. \033[0m\n");
										}
										
									}else
										WARNING("No ip6h!!!");
#if 0 										
									if(fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2)
									{	
										//int trace_filter_need_to_lock = 0;
										if(fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[pPktHdr->ingressPort.macPortIdx]>=0)
										{
											
#if 1//defined(CONFIG_CMCC) //CMCC
#if 0 

											if(ASICDRIVERVER == 0x3)
											{
												rate = (freeIdx==0) ? MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE: MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE_FOR_RL6831;
											}
											else
												rate = (freeIdx==0) ? MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE: MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE;
#endif											
#elif defined(CONFIG_YUEME)
											uint32 rate = 0;

											rate =  MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE;
											DEBUG("Set port %d acl rate: %d, shareMeter idx:%d", pPktHdr->ingressPort.macPortIdx,rate,fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[pPktHdr->ingressPort.macPortIdx]);

											rtskb->nptv6_setMeterFlag = 1;
											fc_db.nptv6_setMeterRate = rate;
											fc_db.nptv6_setMeter_macPortIdx = pPktHdr->ingressPort.macPortIdx;
#endif

										
										}
									}
#endif									
								}
								if(matchIdx==FAIL && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer_length==0)
								{
									customized_entry.valid=TRUE;
									customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.upstream0Gmac;
									customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.upstream0RxRingNum - freeIdx;
									customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.upstream0TxRingNum - freeIdx;
									customized_entry.txPreLen=14+ipv6_hdr_len+2;
									customized_entry.txInfo_addr_offset_v1=RX_OFFSET;
									customized_entry.txInfo_addr_offset_v2=RX_OFFSET+customized_entry.txPreLen+pppoeOffset;
									customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo;
									customized_entry.type=CUSTOMIZE_TYPE_NPTV6_UP;
									customized_entry.rx_ext_pmsk=FC_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK;

									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].sipv6.ipv6_addr, upstream_sipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].dipv6.ipv6_addr, upstream_dipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].smac.octet, upstream_smac.octet, ETHER_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].dmac.octet, upstream_dmac.octet, ETHER_ADDR_LEN);
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].isTcp = upstream_isTcp;
									
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer_length = customized_entry.txPreLen;				
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer, RTSKB_DATA(rtskb), 12);
									*(uint16 *)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer + 12) = htons(0x86dd);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer+14, &RTSKB_DATA(rtskb)[pPktHdr->l3Offset], ipv6_hdr_len);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer+14+ipv6_hdr_len, &RTSKB_DATA(rtskb)[pPktHdr->l4Offset], 2);

									fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer_length;

									if (unlikely(fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length > MAX_NPTV6_OUTER_LEN))
										fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length = MAX_NPTV6_OUTER_LEN;

									memcpy(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[freeIdx].preallocated_outer, fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length);
								
									DEBUG("txPreLen=%d txInfo_addr_offset_v1=%d txInfo_addr_offset_v2=%d", customized_entry.txPreLen, customized_entry.txInfo_addr_offset_v1, customized_entry.txInfo_addr_offset_v2);
									
									DUMP_PACKET(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length, "Upstream Odd Source Port");

									re8686_customized_rx_and_tx(customized_entry, NULL, _rtk_fc_nptv6_customized_txPrepare, nic_rx_skb_NPTv6FastForward, (pppoeOffset)?_rtk_fc_nptv6_pppoe_customized_TxHook:_rtk_fc_nptv6_ipoe_customized_TxHook);
								}
							}
							else
							{
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
							}
						}
						else
						{
							
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
						}
						pPktHdr->nptv6_acceleration_upstream0_idx = freeIdx;
						*pDmacL2Idx = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].virtual_dmacL2Idx;
						pPath5Entry->out_dmac_idx = virtual_indMacIdx;
						pPath5Entry->out_ctag_format_act = TRUE;
						pPath5Entry->out_cvid_format_act = FALSE;
						pPath5Entry->out_cpri_format_act = FALSE;
						pPath5Entry->out_stag_format_act = TRUE;
						pPath5Entry->out_svid_format_act = FALSE;
						pPath5Entry->out_spri_format_act = FALSE;
						pPath5Entry->out_user_pri_act = TRUE;
						pPath5Entry->out_user_priority = (freeIdx < fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum) ? (fc_db_fastFwd_data.nptv6_acc.upstream0Priority-freeIdx) : fc_db_fastFwd_data.nptv6_acc.upstream0Priority_rg;
						TRACE("Accelerate nptv6 upstream0[%d] by indMacIdx[%d]", pPktHdr->nptv6_acceleration_upstream0_idx, pPath5Entry->out_dmac_idx);
						//assert_ok(RTK_RG_ASIC_INDIRECTMACTABLE_DEL(indMacIdx));
						//if(fc_db.indMacTbl[indMacIdx].valid)
						//	fc_db.indMacTbl[indMacIdx].indMacRefCount--;
					}
					else
					{
						TRACE("[Fail to add flow] Fail to add indirect mac table for nptv6 acceleration");
						//assert_ok((pf.rtk_rg_macEntry_del)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].virtual_dmacL2Idx));
						if(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].virtual_dmacL2Idx])
						{
							RTK_FC_LUT_DEL(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[freeIdx].virtual_dmacL2Idx], FALSE);
						}
					}
				}
				else
				{
					WARNING("LUT DA learning is fail, ret = 0x%x", ret);
				}
			}
			
		}	
	}
	else if(direction==RTK_FC_V6NAT_DIRECTION_DOWNSTREAM) // inbound
	{
		uint32 ipv6_hdr_len=40U;
		uint8 ipv6_hdr_offset = 14U;
		rtk_mac_t downstream_smac; // wan gmac
		rtk_mac_t downstream_dmac; // remote mac
		rtk_ipv6_addr_t downstream_sipv6; 
		rtk_ipv6_addr_t downstream_dipv6;
		uint8 downstream_isTcp;

		if(pPktHdr->ip6h)
		{
			memcpy(downstream_sipv6.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
			memcpy(downstream_dipv6.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN);

		}else{
			WARNING("No ip6h!?");
			return;
		}
		memcpy(downstream_smac.octet, pPktHdr->eth->h_source, ETHER_ADDR_LEN);
		memcpy(downstream_dmac.octet, pPktHdr->eth->h_dest, ETHER_ADDR_LEN);
		downstream_isTcp = (pPktHdr->tcph)?1U:0U;
		//WARNING("[NPTv6]downstream_sipv6.ipv6_addr =	%pI6c pPktHdr->ip6h->saddr.in6_u.u6_addr8 = %pI6c", &downstream_sipv6.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8);
		//WARNING("[NPTv6]downstream_dipv6.ipv6_addr =	%pI6c pPktHdr->ip6h->daddr.in6_u.u6_addr8 = %pI6c", &downstream_sipv6.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8);
		if((fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum==0U) || (fc_db.lutTbl[pPktHdr->dmacL2Idx] && (fc_db.lutTbl[pPktHdr->dmacL2Idx]->spa & 0x1U)==0U)) //1 Dst port number is even
		{
			for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM_NIC_FLOW_NUM; i++)
			{
				if(fc_db.controlFuc.hwnat_customize)
				{
					if(i<fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].preallocated_outer_length)
					{
						if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].isTcp!=downstream_isTcp
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].sipv6.ipv6_addr, downstream_sipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].dipv6.ipv6_addr, downstream_dipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].smac.octet, downstream_smac.octet, ETHER_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].dmac.octet, downstream_dmac.octet, ETHER_ADDR_LEN))
							continue;

						matchIdx = i;
						break;
					}
				}
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].vaild==0)
				{
					if(first_freeIdx==FAIL)
						first_freeIdx = i;

					continue;
				}
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx] &&
					pPktHdr->ip6h &&
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN) &&
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].ipv6_addr.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN))
				{
					if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].ipv6_hdr_offset!=ipv6_hdr_offset)
						continue;

					matchIdx = i;
					break;
				}
			}
			if(matchIdx!=FAIL)
			{
				TRACE("Match nptv6 downstream acc idx[%d]", matchIdx);
				freeIdx = matchIdx;
			}
			else if(first_freeIdx!=FAIL)
			{
				TRACE("Find free nptv6 downstream acc idx[%d]", first_freeIdx);
				freeIdx = first_freeIdx;
			}
			if(freeIdx!=FAIL)
			{
				rtk_fc_pktHdr_t tmp_pktHdr;
				struct ethhdr tmp_eth;
				int ret = 0;
				int32  virtual_indMacIdx = -1;
				int16 lutIdx = 0;
				if(pPktHdr->ip6h)
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].ipv6_addr.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN);

				tmp_pktHdr.eth = &tmp_eth;
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
					memcpy(&tmp_pktHdr.eth->h_dest[0], fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr,6);
				tmp_pktHdr.eth->h_dest[0] = 0x0U;
				tmp_pktHdr.eth->h_dest[1] = 0x0U;
				tmp_pktHdr.eth->h_dest[2] = 0x5eU;
				tmp_pktHdr.eth->h_dest[3] = freeIdx;
				if(fc_db_fastFwd_data.nptv6_acc.downstreamExtport == FC_EXT_MAC9_PORT2)
				{
					tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
					tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT2;
				}
				else if(fc_db_fastFwd_data.nptv6_acc.downstreamExtport == FC_EXT_MAC7_PORT2)
				{
					tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
					tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT2;
				}
				tmp_pktHdr.cvlanid = 0U;
				tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
				
				if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
 			    {
 					DEBUG("[NPTv6]LUT Learned before, no need to learn again.");
					ret = RTK_FC_RET_OK;
 			    }
				else
				{
					if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
												FALSE, TRUE, &lutIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
						WARNING("LUT DA learning is fail, ret = 0x%x", ret);
					}

				}

				if(ret==RTK_FC_RET_OK)
				{
					fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx = lutIdx;
					ret = rtk_fc_indMac_idx_get((int16)fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx, &virtual_indMacIdx);
					//if(fc_db.indMacTbl[virtual_indMacIdx].valid)
					//	fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
					if(ret == RTK_FC_RET_OK)
					{
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].vaild = 1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].ipv6_hdr_offset = ipv6_hdr_offset;
						//fill desc
						memcpy(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo, &fc_db.txDesc, sizeof(struct tx_info));
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.gmac_id=fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=RTK_RG_CPUTAG_PPPOEACT_KEEP;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.tx_tx_cvlan_action=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.tx_tx_svlan_action=0;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.ipcs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.l4cs=0U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.fs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.ls=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.crc=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts2.bit.cputag=1U;

						if(fc_db.controlFuc.hwnat_customize)
						{
							if(freeIdx < fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum)
							{
								if(dynamic_sram_desc)
								{
									if(_rtk_rg_flowHashIPv6DstAddr_get(downstream_dipv6.ipv6_addr) != RTSKB_FCIGRDATA(rtskb)->dstIp)
									{
										memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo, 0, sizeof(struct tx_info));
										fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.gmac_id = fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
										fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.extspa = fc_db_fastFwd_data.nptv6_acc.downstreamExtport; 
										fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.stag_aware = 1;
									}
									else
									{
										printk("\033[1;33;40m[Notice] NPTv6 downstream flows(DIP/EXT_DIP) are conflict, please change prefix of related IPv6 domain or can not do cvlan HW offload. \033[0m\n");
									}
								}
								if(matchIdx==FAIL && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer_length==0)
								{
									customized_entry.valid=TRUE;
									customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
									customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum - freeIdx;
									customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.downstreamTxRingNum - freeIdx;
									customized_entry.txPreLen=14+ipv6_hdr_len+2;
									customized_entry.txInfo_addr_offset_v1=RX_OFFSET;
									customized_entry.txInfo_addr_offset_v2=RX_OFFSET+customized_entry.txPreLen;
									customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo;
									customized_entry.type=CUSTOMIZE_TYPE_NPTV6_DOWN;
									customized_entry.rx_ext_pmsk=FC_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK;

									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].sipv6.ipv6_addr, downstream_sipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].dipv6.ipv6_addr, downstream_dipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].smac.octet, downstream_smac.octet, ETHER_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].dmac.octet, downstream_dmac.octet, ETHER_ADDR_LEN);
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].isTcp = downstream_isTcp;
									
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer_length = customized_entry.txPreLen; 					
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer, RTSKB_DATA(rtskb), 12);
									*(uint16 *)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer + 12) = htons(0x86dd);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer+14, &RTSKB_DATA(rtskb)[pPktHdr->l3Offset], ipv6_hdr_len);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer+14+ipv6_hdr_len, &RTSKB_DATA(rtskb)[pPktHdr->l4Offset], 2);

									fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer_length;
									
									memcpy(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[freeIdx].preallocated_outer, MAX_NPTV6_OUTER_LEN);
								
									DEBUG("txPreLen=%d txInfo_addr_offset_v1=%d txInfo_addr_offset_v2=%d", customized_entry.txPreLen, customized_entry.txInfo_addr_offset_v1, customized_entry.txInfo_addr_offset_v2);
									
									DUMP_PACKET(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length, "DownStream NPTv6 SortPort even");

									re8686_customized_rx_and_tx(customized_entry, NULL, _rtk_fc_nptv6_customized_txPrepare, nic_rx_skb_NPTv6FastForward, _rtk_fc_nptv6_ipoe_customized_TxHook);
								}
							}
							else
							{
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
							}
						}
						else
						{
						
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
						}
						
						pPktHdr->nptv6_acceleration_downstream_idx = freeIdx;
						*pDmacL2Idx = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx; // change to cpu port
						pPath5Entry->out_dmac_idx = virtual_indMacIdx;
						pPath5Entry->in_dst_ipv6_addr_hash = RTSKB_FCIGRDATA(rtskb)->dstIp;//_rtk_rg_flowHashIPv6DstAddr_get(pPktHdr->ip6h->daddr.in6_u.u6_addr8);
						pPath5Entry->out_ctag_format_act = TRUE;
						pPath5Entry->out_cvid_format_act = FALSE;
						pPath5Entry->out_cpri_format_act = FALSE;
						pPath5Entry->out_stag_format_act = TRUE;
						pPath5Entry->out_svid_format_act = FALSE;
						pPath5Entry->out_spri_format_act = FALSE;
						pPath5Entry->out_user_pri_act = TRUE;
						pPath5Entry->out_user_priority = (freeIdx < fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum) ? (fc_db_fastFwd_data.nptv6_acc.downstreamPriority-freeIdx) : fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg;
						TRACE("Accelerate nptv6 downstream[%d] by indMacIdx[%d]", pPktHdr->nptv6_acceleration_downstream_idx, pPath5Entry->out_dmac_idx);

						//assert_ok(RTK_RG_ASIC_INDIRECTMACTABLE_DEL(indMacIdx));
						
						//if(fc_db.indMacTbl[indMacIdx].valid)
						//	fc_db.indMacTbl[indMacIdx].indMacRefCount--;
					}
					else
					{
						TRACE("[Fail to add flow] Fail to add indirect mac table for nptv6 acceleration");
						//assert_ok((pf.rtk_rg_macEntry_del)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx));
						if(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx])
						{
							RTK_FC_LUT_DEL(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx], FALSE);
						}
					}
				}
				else
				{
					WARNING("LUT DA learning is fail, ret = 0x%x", ret);
				}
			}
		}
		else //1 Dst port number is odd
		{
			for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM0_NIC_FLOW_NUM; i++)
			{
				if(fc_db.controlFuc.hwnat_customize)
				{
					if(i<fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].preallocated_outer_length)
					{
						if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].isTcp!=downstream_isTcp
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].sipv6.ipv6_addr, downstream_sipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].dipv6.ipv6_addr, downstream_dipv6.ipv6_addr, IPV6_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].smac.octet, downstream_smac.octet, ETHER_ADDR_LEN)
							|| memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].dmac.octet, downstream_dmac.octet, ETHER_ADDR_LEN))
							continue;

						matchIdx = i;
						break;
					}
				}
				
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[i].vaild==0)
				{
					if(first_freeIdx==FAIL)
						first_freeIdx = i;

					continue;
				}
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx] &&
					pPktHdr->ip6h &&
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[i].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN) && 
					!memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[i].ipv6_addr.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN))
				{
					if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[i].ipv6_hdr_offset!=ipv6_hdr_offset)
						continue;

					matchIdx = i;
					break;
				}
			}
			if(matchIdx!=FAIL)
			{
				TRACE("Match nptv6 downstream0 acc idx[%d]", matchIdx);
				freeIdx = matchIdx;
			}
			else if(first_freeIdx!=FAIL)
			{
				TRACE("Find free nptv6 downstream0 acc idx[%d]", first_freeIdx);
				freeIdx = first_freeIdx;
			}
			if(freeIdx!=FAIL)
			{
				rtk_fc_pktHdr_t tmp_pktHdr;
				struct ethhdr tmp_eth;
				int ret = 0;
				int32 virtual_indMacIdx = -1;
				int16 lutIdx = 0;

				if(pPktHdr->ip6h)
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].ipv6_addr.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
					memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN);

				tmp_pktHdr.eth = &tmp_eth;
				if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
					memcpy(&tmp_pktHdr.eth->h_dest[0], fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr,6);
				
				tmp_pktHdr.eth->h_dest[0] = 0x0U;
				tmp_pktHdr.eth->h_dest[1] = 0x1U;
				tmp_pktHdr.eth->h_dest[2] = 0x5eU;
				tmp_pktHdr.eth->h_dest[3] = freeIdx;

				if(fc_db_fastFwd_data.nptv6_acc.downstream0Extport == FC_EXT_MAC9_PORT2)
				{
					tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
					tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT2;
				}
				tmp_pktHdr.cvlanid = 0U;
				tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

				if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
 			    {
 					DEBUG("[NPTv6]LUT Learned before, no need to learn again.");
					ret = RTK_FC_RET_OK;
 			    }
				else
 			    {
					if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
												FALSE, TRUE, &lutIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
						WARNING("LUT DA learning is fail, ret = 0x%x", ret);
					}

				}

				if(ret==RTK_FC_RET_OK)
				{
					fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].virtual_dmacL2Idx = lutIdx;
					ret = rtk_fc_indMac_idx_get((int16)fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].virtual_dmacL2Idx, &virtual_indMacIdx);
					//if(fc_db.indMacTbl[virtual_indMacIdx].valid)
					//	fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
					if(ret == RTK_FC_RET_OK)
					{
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].vaild = 1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].ipv6_hdr_offset = ipv6_hdr_offset;
						//fill desc
						memcpy(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo, &fc_db.txDesc, sizeof(struct tx_info));
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.gmac_id=fc_db_fastFwd_data.nptv6_acc.downstream0Gmac;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_action=RTK_RG_CPUTAG_PPPOEACT_KEEP;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.tx_tx_cvlan_action=0;
						//fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.tx_tx_svlan_action=0;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts1.bit.ipcs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts1.bit.l4cs=0U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts1.bit.fs=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts1.bit.ls=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts1.bit.crc=1U;
						fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts2.bit.cputag=1U;

						if(fc_db.controlFuc.hwnat_customize)
						{	
							if(freeIdx < fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum)
							{
								if(dynamic_sram_desc)
								{
									if(_rtk_rg_flowHashIPv6DstAddr_get(downstream_dipv6.ipv6_addr) != RTSKB_FCIGRDATA(rtskb)->dstIp)
									{
										memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo, 0, sizeof(struct tx_info));
										fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.gmac_id = fc_db_fastFwd_data.nptv6_acc.downstream0Gmac;
										fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.extspa = fc_db_fastFwd_data.nptv6_acc.downstream0Extport; 
										fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts1.bit.stag_aware = 1;
									}
									else
									{
										printk("\033[1;33;40m[Notice] NPTv6 downstream0 flows(DIP/EXT_DIP) are conflict, please change prefix of related IPv6 domain or can not do cvlan HW offload. \033[0m\n");
									}
								}
								if(matchIdx==FAIL && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer_length==0)
								{
									customized_entry.valid=TRUE;
									customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.downstream0Gmac;
									customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.downstream0RxRingNum - freeIdx;
									customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.downstream0TxRingNum - freeIdx;
									customized_entry.txPreLen=14+ipv6_hdr_len+2;
									customized_entry.txInfo_addr_offset_v1=RX_OFFSET;
									customized_entry.txInfo_addr_offset_v2=RX_OFFSET+customized_entry.txPreLen;
									customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo;
									customized_entry.type=CUSTOMIZE_TYPE_NPTV6_DOWN;
									customized_entry.rx_ext_pmsk=FC_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK;

									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].sipv6.ipv6_addr, downstream_sipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].dipv6.ipv6_addr, downstream_dipv6.ipv6_addr, IPV6_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].smac.octet, downstream_smac.octet, ETHER_ADDR_LEN);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].dmac.octet, downstream_dmac.octet, ETHER_ADDR_LEN);
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].isTcp = downstream_isTcp;
									
									fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer_length = customized_entry.txPreLen;						
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer, RTSKB_DATA(rtskb), 12);
									*(uint16 *)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer + 12) = htons(0x86dd);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer+14, &RTSKB_DATA(rtskb)[pPktHdr->l3Offset], ipv6_hdr_len);
									memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer+14+ipv6_hdr_len, &RTSKB_DATA(rtskb)[pPktHdr->l4Offset], 2);

									fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer_length;
									memcpy(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[freeIdx].preallocated_outer, MAX_NPTV6_OUTER_LEN);
									
									DEBUG("txPreLen=%d txInfo_addr_offset_v1=%d txInfo_addr_offset_v2=%d", customized_entry.txPreLen, customized_entry.txInfo_addr_offset_v1, customized_entry.txInfo_addr_offset_v2);

									DUMP_PACKET(fc_db_fastFwd_data.nptv6_acc.preallocated_outer, fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length, "Down Strean src port odd");

									re8686_customized_rx_and_tx(customized_entry, NULL, _rtk_fc_nptv6_customized_txPrepare, nic_rx_skb_NPTv6FastForward, _rtk_fc_nptv6_ipoe_customized_TxHook);
								}
							}
							else
							{
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
								fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
							}

						}
						else
						{
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
							fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;

						}
						
						pPktHdr->nptv6_acceleration_downstream0_idx = freeIdx;
						*pDmacL2Idx = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].virtual_dmacL2Idx;
						pPath5Entry->out_dmac_idx = virtual_indMacIdx;
						pPath5Entry->in_dst_ipv6_addr_hash = RTSKB_FCIGRDATA(rtskb)->dstIp;//_rtk_rg_flowHashIPv6DstAddr_get(pPktHdr->ip6h->daddr.in6_u.u6_addr8);
						pPath5Entry->out_ctag_format_act = TRUE;
						pPath5Entry->out_cvid_format_act = FALSE;
						pPath5Entry->out_cpri_format_act = FALSE;
						pPath5Entry->out_stag_format_act = TRUE;
						pPath5Entry->out_svid_format_act = FALSE;
						pPath5Entry->out_spri_format_act = FALSE;
						pPath5Entry->out_user_pri_act = TRUE;
						pPath5Entry->out_user_priority = (freeIdx < fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum) ? (fc_db_fastFwd_data.nptv6_acc.downstream0Priority-freeIdx) : fc_db_fastFwd_data.nptv6_acc.downstream0Priority_rg;
						TRACE("Accelerate nptv6 downstream0[%d] by indMacIdx[%d]", pPktHdr->nptv6_acceleration_downstream0_idx, pPath5Entry->out_dmac_idx);
					//	assert_ok(RTK_RG_ASIC_INDIRECTMACTABLE_DEL(indMacIdx));
						//if(fc_db.indMacTbl[indMacIdx].valid)
						//	fc_db.indMacTbl[indMacIdx].indMacRefCount--;
					}
					else
					{
						TRACE("[Fail to add flow] Fail to add indirect mac table for nptv6 acceleration");
						//assert_ok((pf.rtk_rg_macEntry_del)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].virtual_dmacL2Idx));
						if(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].virtual_dmacL2Idx])
						{
							RTK_FC_LUT_DEL(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[freeIdx].virtual_dmacL2Idx], FALSE);
						}
					}
				}
				else
				{
					WARNING("LUT DA learning is fail, ret = 0x%x", ret);
				}
				
			}
		}
	}
	return;
			
}
int _rtk_fc_nptv6_acceleration_mechanism_dramMode_check(rtk_fc_pktHdr_t *pPktHdr, 
																	  rtk_rg_asic_path5_entry_t *pPath5Entry, 
																	  int32 indMacIdx, 
																	  int direction, 
																	  struct rt_skbuff *rtskb, 
																	  int *pDmacL2Idx)
{
   int i, freeIdx=FAIL, first_freeIdx=FAIL, matchIdx=FAIL;
   int outerFlow_collision=0;
   struct tx_info hwlookup_txInfo;
   struct rtl8686_hwnat_customized_entry customized_entry={0};


   if(direction==RTK_FC_V6NAT_DIRECTION_UPSTREAM)
   {
	   if(pPktHdr->ip6h && RTSKB_FCIGRDATA(rtskb)->srcIp == _rtk_rg_flowHashIPv6SrcAddr_get(pPktHdr->ip6h->saddr.in6_u.u6_addr8))
	   {
		   printk("[NPTv6][US]outer flow Collision!\n");
		   outerFlow_collision = 1;
		   return FAILED;
	   }
   }
   else if(direction==RTK_FC_V6NAT_DIRECTION_DOWNSTREAM)
   {

	   if(pPktHdr->ip6h && RTSKB_FCIGRDATA(rtskb)->dstIp == _rtk_rg_flowHashIPv6DstAddr_get(pPktHdr->ip6h->daddr.in6_u.u6_addr8))
	   {
		   printk("[NPTv6][DS]outer flow Collision!\n");
		   outerFlow_collision = 1;
		   return FAILED;
	   }
   }
   

   if(direction==RTK_FC_V6NAT_DIRECTION_UPSTREAM) // outbound
   {
	   uint32 ipv6_hdr_len=40U, pppoeOffset=((pPktHdr->ppph)?8U:0U);
	   uint8 ipv6_hdr_offset = 14+pppoeOffset;

	   for(i=0; i<MAX_NPTV6_ACC_UPSTREAM_SIZE; i++)
	   {
		   if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].vaild==0)
		   {
			   if(first_freeIdx==FAIL)
				   first_freeIdx = i;

			   continue;
		   }
		   if( fc_db.lutTbl[pPktHdr->dmacL2Idx] &&
			   pPktHdr->ip6h &&
			   !memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN)
			   && !memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].ipv6_addr.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN)
			   && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].ipv6_hdr_offset==ipv6_hdr_offset)
		   {
			   matchIdx = i;
			   break;
		   }
	   }
	   if(matchIdx!=FAIL)
	   {
		   TRACE("Match nptv6 upstream acc idx[%d]", matchIdx);
		   freeIdx = matchIdx;
	   }
	   else if(first_freeIdx!=FAIL)
	   {
		   TRACE("Find free nptv6 upstream acc idx[%d]", first_freeIdx);
		   freeIdx = first_freeIdx;
	   }

	   if(freeIdx == FAIL)
	   {
			 TRACE("Can't Find free nptv6 upstream acc free index, go swOnly");	
			  return FAILED;
	   }
	   if(freeIdx!=FAIL)
	   {   
		   rtk_fc_pktHdr_t tmp_pktHdr;
		   struct ethhdr tmp_eth;
		   int ret = 0;
		   int32 virtual_indMacIdx = -1;
		   int16 lutIdx = 0;
		   if(pPktHdr->ip6h)
			   memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].ipv6_addr.ipv6_addr, pPktHdr->ip6h->saddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
		   if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
			   memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN);

		   tmp_pktHdr.eth = &tmp_eth;
		   if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
			   memcpy(&tmp_pktHdr.eth->h_dest[0], fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr,6);

		   tmp_pktHdr.eth->h_dest[0] = 0x0U;
		   tmp_pktHdr.eth->h_dest[1] = 0x0U;
		   tmp_pktHdr.eth->h_dest[2] = 0x5eU;
		   tmp_pktHdr.eth->h_dest[3] = freeIdx;

		   if(fc_db_fastFwd_data.nptv6_acc.upstreamExtport == FC_EXT_MAC10_PORT1)
		   {
			   tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
			   tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT1;
		   }
		   tmp_pktHdr.cvlanid = 0U;
		   tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

		   if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
		   {
				DEBUG("[NPTv6]LUT Learned before, no need to learn again.");
				ret = RTK_FC_RET_OK;
		   }
		   else
		   {
		   
			   if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
										   FALSE, TRUE, &lutIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
				   WARNING("LUT DA learning is fail, ret = 0x%x", ret);
			   }

		   }

									   
		   if(ret==RTK_FC_RET_OK)
		   {
			   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx = lutIdx;
			   ret = rtk_fc_indMac_idx_get((int16)fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx, &virtual_indMacIdx);
				   //if(fc_db.indMacTbl[virtual_indMacIdx].valid)
				   //  fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
			   if(ret == RTK_FC_RET_OK)
			   {
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].vaild = 1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].ipv6_hdr_offset = ipv6_hdr_offset;
				   //fill desc
				   memcpy(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo, &fc_db.txDesc, sizeof(struct tx_info));
				   //printk("(%d):freeIdx = %d TXINFO_TX_PORTMSK(ptxInfo) = %x\n", __LINE__, freeIdx, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts2.bit.tx_portmask);
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.gmac_id=fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
				   //fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.tx_tx_cvlan_action=0;
				   //fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.tx_tx_svlan_action=0;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.ipcs=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.l4cs=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.fs=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.ls=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts1.bit.crc=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts2.bit.cputag=1U;
				   //if(outerFlow_collision)
				   //		fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.l34_keep=1U;

				   if(fc_db.controlFuc.hwnat_customize)
				   {
					   if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[0].preallocated_outer_length==0U)
					   {
						   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[0].preallocated_outer_length = 1U; // for valid bit

						   //for HWLOOKUP
						   memset(&hwlookup_txInfo, 0, sizeof(struct tx_info));
						   hwlookup_txInfo.opts3.bit.gmac_id = fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
						   hwlookup_txInfo.opts3.bit.extspa = fc_db_fastFwd_data.nptv6_acc.upstreamExtport; 
						   hwlookup_txInfo.opts1.bit.stag_aware = 1;
							   
						   customized_entry.valid=TRUE;
						   customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
						   customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum;
						   customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.upstreamTxRingNum;
						   customized_entry.txPreLen=14+ipv6_hdr_len;  // dram mode don't care
						   customized_entry.txInfo_addr_offset_v1=RX_OFFSET;
						   customized_entry.txInfo_addr_offset_v2=RX_OFFSET+customized_entry.txPreLen+pppoeOffset; // dram mode don't care
						   customized_entry.pCustomized_txInfo=(struct tx_info *)&hwlookup_txInfo;
						   customized_entry.type=CUSTOMIZE_TYPE_NPTV6_UP;
						   customized_entry.rx_ext_pmsk=FC_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK;

						   re8686_customized_rx_and_tx(customized_entry, NULL, NULL, nic_rx_skb_NPTv6FastForward, NULL);
					   }
				   }
				   

				   pPktHdr->nptv6_acceleration_upstream_idx = freeIdx;
				   *pDmacL2Idx = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx;
				   pPath5Entry->out_dmac_idx = virtual_indMacIdx;
				   pPath5Entry->out_ctag_format_act = TRUE;
				   pPath5Entry->out_cvid_format_act = FALSE;
				   pPath5Entry->out_cpri_format_act = FALSE;
				   pPath5Entry->out_stag_format_act = TRUE;
				   pPath5Entry->out_svid_format_act = FALSE;
				   pPath5Entry->out_spri_format_act = FALSE;
				   pPath5Entry->out_user_pri_act = TRUE;
				   pPath5Entry->out_user_priority = /*(outerFlow_collision) ? fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg:*/fc_db_fastFwd_data.nptv6_acc.upstreamPriority;
				   TRACE("Accelerate nptv6 upstream[%d] by indMacIdx[%d]", pPktHdr->nptv6_acceleration_upstream_idx, pPath5Entry->out_dmac_idx);
				   //assert_ok(RTK_RG_ASIC_INDIRECTMACTABLE_DEL(indMacIdx));
				   //if(fc_db.indMacTbl[indMacIdx].valid)
				   //	   fc_db.indMacTbl[indMacIdx].indMacRefCount--;
			   }
			   else
			   {
				   TRACE("[Fail to add flow] Fail to add indirect mac table for nptv6 acceleration");
				   //assert_ok((pf.rtk_rg_macEntry_del)(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx));
				   if(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx])
				   {
					   RTK_FC_LUT_DEL(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].virtual_dmacL2Idx], FALSE);
				   }
			   }
		   }
		   else
		   {
			   WARNING("LUT DA learning is fail, ret = 0x%x", ret);
			   return FAILED;
		   }
		   
	   }   
   }
   else if(direction==RTK_FC_V6NAT_DIRECTION_DOWNSTREAM) // inbound
   {
	   uint32 ipv6_hdr_len=40U;
	   uint8 ipv6_hdr_offset = 14U;
	   
	   for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM_SIZE; i++)
	   {
		   if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].vaild==0)
		   {
			   if(first_freeIdx==FAIL)
				   first_freeIdx = i;

			   continue;
		   }
		   if( fc_db.lutTbl[pPktHdr->dmacL2Idx] &&
			   pPktHdr->ip6h &&
			   !memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN)
			   && !memcmp(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].ipv6_addr.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN)
			   && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].ipv6_hdr_offset==ipv6_hdr_offset)
		   {
			   matchIdx = i;
			   break;
		   }
	   }
	   if(matchIdx!=FAIL)
	   {
		   TRACE("Match nptv6 downstream acc idx[%d]", matchIdx);
		   freeIdx = matchIdx;
	   }
	   else if(first_freeIdx!=FAIL)
	   {
		   TRACE("Find free nptv6 downstream acc idx[%d]", first_freeIdx);
		   freeIdx = first_freeIdx;
	   }
	   

	   if(freeIdx == FAIL)
	   {
			 TRACE("Can't Find free nptv6 upstream acc free index, go swOnly");	
			  return FAILED;
	   }
	   
	   if(freeIdx!=FAIL)
	   {
		   rtk_fc_pktHdr_t tmp_pktHdr;
		   struct ethhdr tmp_eth;
		   int ret = 0;
		   int32 virtual_indMacIdx = -1;
		   int16 lutIdx = 0;

		   if(pPktHdr->ip6h)
			   memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].ipv6_addr.ipv6_addr, pPktHdr->ip6h->daddr.in6_u.u6_addr8, IPV6_ADDR_LEN);
		   if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
			   memcpy(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].dmac.octet, fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr, ETHER_ADDR_LEN);

		   tmp_pktHdr.eth = &tmp_eth;
		   if(fc_db.lutTbl[pPktHdr->dmacL2Idx])
			   memcpy(&tmp_pktHdr.eth->h_dest[0], fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr,6);
		   
		   tmp_pktHdr.eth->h_dest[0] = 0x0U;
		   tmp_pktHdr.eth->h_dest[1] = 0x0U;
		   tmp_pktHdr.eth->h_dest[2] = 0x5eU;
		   tmp_pktHdr.eth->h_dest[3] = freeIdx;

		   if(fc_db_fastFwd_data.nptv6_acc.downstreamExtport == FC_EXT_MAC9_PORT2)
		   {
			   tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
			   tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT2;
		   }
		   else if(fc_db_fastFwd_data.nptv6_acc.downstreamExtport == FC_EXT_MAC7_PORT2)
		   {
			   tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
			   tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT2;
		   }

		   tmp_pktHdr.cvlanid = 0U;
		   tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
		   if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
		   {
				DEBUG("[NPTv6]LUT Learned before, no need to learn again.");
				ret = RTK_FC_RET_OK;
		   }
		   else
		   {
			   if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
										   FALSE, TRUE, &lutIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
				   WARNING("LUT DA learning is fail, ret = 0x%x", ret);
			   }

		   }
									   
		   
		   if(ret==RTK_FC_RET_OK)
		   {
			   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx = lutIdx;
			   ret = rtk_fc_indMac_idx_get((int16)fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx, &virtual_indMacIdx);
				   //if(fc_db.indMacTbl[virtual_indMacIdx].valid)
				   //  fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
			   if(ret==RTK_FC_RET_OK)
			   {
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].vaild = 1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].ipv6_hdr_offset = ipv6_hdr_offset;
				   //fill desc
				   memcpy(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo, &fc_db.txDesc, sizeof(struct tx_info));
				   //printk("(%d):freeIdx = %d TXINFO_TX_PORTMSK(ptxInfo) = %x fc_db.txDesc = %x\n", __LINE__, freeIdx, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts2.bit.tx_portmask, fc_db.txDesc.opts2.bit.tx_portmask);
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.gmac_id=fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_action=0U;//RTK_RG_CPUTAG_PPPOEACT_KEEP;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts3.bit.tx_pppoe_idx=0U;
				   //fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.tx_tx_cvlan_action=0;
				   //fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.tx_tx_svlan_action=0;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.ipcs=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.l4cs=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.fs=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.ls=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts1.bit.crc=1U;
				   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo.opts2.bit.cputag=1U;
				   //if(outerFlow_collision)
					//	fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[freeIdx].txInfo.opts3.bit.l34_keep=1U;

				   if(fc_db.controlFuc.hwnat_customize)
				   {
					   if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[0].preallocated_outer_length==0U)
					   {
						   fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[0].preallocated_outer_length = 1U; // for valid bit

						   //for HWLOOKUP
						   memset(&hwlookup_txInfo, 0, sizeof(struct tx_info));
						   hwlookup_txInfo.opts3.bit.gmac_id = fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
						   hwlookup_txInfo.opts3.bit.extspa = fc_db_fastFwd_data.nptv6_acc.downstreamExtport; 
						   hwlookup_txInfo.opts1.bit.stag_aware = 1;
						   
						   customized_entry.valid=TRUE;
						   customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
						   customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum;
						   customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.downstreamTxRingNum;
						   customized_entry.txPreLen=14+ipv6_hdr_len;  // dram mode don't care
						   customized_entry.txInfo_addr_offset_v1=RX_OFFSET;
						   customized_entry.txInfo_addr_offset_v2=RX_OFFSET+customized_entry.txPreLen; // dram mode don't care
						   customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].txInfo;
						   customized_entry.type=CUSTOMIZE_TYPE_NPTV6_DOWN;
						   customized_entry.rx_ext_pmsk=FC_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK;

						   re8686_customized_rx_and_tx(customized_entry, NULL, NULL, nic_rx_skb_NPTv6FastForward, NULL);
					   }

				   }
				   pPktHdr->nptv6_acceleration_downstream_idx = freeIdx;
				   *pDmacL2Idx = fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx;
				   pPath5Entry->out_dmac_idx = virtual_indMacIdx;
				   pPath5Entry->in_dst_ipv6_addr_hash = RTSKB_FCIGRDATA(rtskb)->dstIp;//_rtk_rg_flowHashIPv6DstAddr_get(pPktHdr->ip6h->daddr.in6_u.u6_addr8);
				   pPath5Entry->out_ctag_format_act = TRUE;
				   pPath5Entry->out_cvid_format_act = FALSE;
				   pPath5Entry->out_cpri_format_act = FALSE;
				   pPath5Entry->out_stag_format_act = TRUE;
				   pPath5Entry->out_svid_format_act = FALSE;
				   pPath5Entry->out_spri_format_act = FALSE;
				   pPath5Entry->out_user_pri_act = TRUE;
				   pPath5Entry->out_user_priority = /*(outerFlow_collision) ?  fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg:*/fc_db_fastFwd_data.nptv6_acc.downstreamPriority;
				   
				   TRACE("Accelerate nptv6 downstream[%d] by indMacIdx[%d]", pPktHdr->nptv6_acceleration_downstream_idx, pPath5Entry->out_dmac_idx);
				   //assert_ok(RTK_RG_ASIC_INDIRECTMACTABLE_DEL(indMacIdx));
				   //if(fc_db.indMacTbl[indMacIdx].valid)
				   //	   fc_db.indMacTbl[indMacIdx].indMacRefCount--;
			   }
			   else
			   {
				   TRACE("[Fail to add flow] Fail to add indirect mac table for nptv6 acceleration");
			   
				   if(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx])
				   {
					   RTK_FC_LUT_DEL(fc_db.lutTbl[fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[freeIdx].virtual_dmacL2Idx], FALSE);
				   }
			   }
		   }
		   else
		   {
			   WARNING("LUT DA learning is fail, ret = 0x%x", ret);
			   return FAILED;
		   }
		   
	   }
   }

   return SUCCESS;
}

#endif


int rtk_fc_get_ipv6_nat_indirectMap_ref(rtk_fc_pktHdr_t *pPktHdr, uint32 *i6NatIndirectIndex)
{
	int i;
	if(pPktHdr->ip6h==NULL)
		return FAILED;
	
	for(i = RTK_FC_I6NAT_TABLE_START_INDEX ; i < RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE; i ++)
	{
		//Find Old same info Entry
		if( atomic_read(&fc_db.ipv6_nat_mappingTbl[i].refCount)!=0 )
		{
			if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && 
				(!memcmp(&fc_db.ipv6_nat_mappingTbl[i].addr.s6_addr32[0], &pPktHdr->ip6h->saddr.s6_addr32[0], sizeof(struct in6_addr))) 
			   )
			{				
				DEBUG("[NAT66/NPT][US]Find Match Entry (%d)!! Add reference count !",i);
				*i6NatIndirectIndex = i;
				atomic_inc(&fc_db.ipv6_nat_mappingTbl[i].refCount);
				return SUCCESS;					
			}
			else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM && 
				    (!memcmp(&fc_db.ipv6_nat_mappingTbl[i].addr.s6_addr32[0], &pPktHdr->ip6h->daddr.s6_addr32[0], sizeof(struct in6_addr)))  
				   )
			{
				DEBUG("[NAT66/NPT][DS]Find Match Entry (%d)!! Add reference count !",i);
				*i6NatIndirectIndex = i;
				
				atomic_inc(&fc_db.ipv6_nat_mappingTbl[i].refCount);
				return SUCCESS;
				
			}

			continue;
			
		}
		//Find New Entry or replace old one ( refcount = 0)
		if(atomic_read(&fc_db.ipv6_nat_mappingTbl[i].refCount)==0)
		{
			
			
			if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM )
			{
				int tmp_off = 0; // l2: 14, double fake vlan: 8
				DEBUG("[NAT66/NPT][US]Find Entry (%d)!!",i);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)	
				tmp_off +=14 + 4; // 07C use ctag to store flow index
#elif defined(CONFIG_FC_CAG3_SERIES)		
				tmp_off +=14 + 8; // 8277 use stag+ctag to store flow index
#endif
				fc_db.ipv6_nat_mappingTbl[i].addr = pPktHdr->ip6h->saddr;
				atomic_inc(&fc_db.ipv6_nat_mappingTbl[i].refCount);
				
				if(pPktHdr->ppph)
					tmp_off+=8;
				
				fc_db.ipv6_nat_mappingTbl[i].oriCVID = pPktHdr->cvlanid;
				fc_db.ipv6_nat_mappingTbl[i].oriCPRI = pPktHdr->cvlanpri;
				fc_db.ipv6_nat_mappingTbl[i].oriSVID = pPktHdr->svlanid;
				fc_db.ipv6_nat_mappingTbl[i].oriSPRI = pPktHdr->svlanpri;
				fc_db.ipv6_nat_mappingTbl[i].direction = RTK_FC_V6NAT_DIRECTION_UPSTREAM;
				*i6NatIndirectIndex = i;
				
				return SUCCESS;
				
					
			}
			else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM )
			{
				int tmp_off = 0; // l2: 14, double fake vlan: 8
				
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)	
				tmp_off +=14 + 4; // 07C use ctag to store flow index
#elif defined(CONFIG_FC_CAG3_SERIES)		
				tmp_off +=14 + 8; // 8277 use stag+ctag to store flow index
#endif
				DEBUG("[NAT66/NPT][DS]Find Entry (%d)!!",i);
				fc_db.ipv6_nat_mappingTbl[i].addr = pPktHdr->ip6h->daddr;
				atomic_inc(&fc_db.ipv6_nat_mappingTbl[i].refCount);
				
				if(pPktHdr->ppph)
					tmp_off+=8;
				
				fc_db.ipv6_nat_mappingTbl[i].oriCVID = pPktHdr->cvlanid;
				fc_db.ipv6_nat_mappingTbl[i].oriCPRI = pPktHdr->cvlanpri;
				fc_db.ipv6_nat_mappingTbl[i].oriSVID = pPktHdr->svlanid;
				fc_db.ipv6_nat_mappingTbl[i].oriSPRI = pPktHdr->svlanpri;
				fc_db.ipv6_nat_mappingTbl[i].direction = RTK_FC_V6NAT_DIRECTION_DOWNSTREAM;

				*i6NatIndirectIndex = i;
				
				return SUCCESS;
			}
		}
		if(i == RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE-1)
		{
			WARNING("ipv6 nat mapping table FULL!!\n");
			return FAILED;
		}
	}
	return FAILED;
		
}

int _rtk_fc_get_ipv6_mapt_indirectMap_ref(struct in6_addr *addr, uint32 *i6MaptIndirectIndex)
{
	int i,first_invalid=-1;

	for(i = RTK_FC_I6MAPT_TABLE_START_INDEX ; i < RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE; i ++){
		//Find Old same info Entry
		if(atomic_read(&fc_db.ipv6_mapt_mappingTbl[i].refCount)!=0){
			if(!fc_db.ipv6_mapt_mappingTbl[i].isHash && !memcmp(&fc_db.ipv6_mapt_mappingTbl[i].addr.s6_addr32[0], &addr->s6_addr32[0], sizeof(struct in6_addr))){
				DEBUG("[IPV6 MAPT/XLAT]Find Match Entry (%d)!! Add reference count !",i);
				*i6MaptIndirectIndex = i;
				atomic_inc(&fc_db.ipv6_mapt_mappingTbl[i].refCount);
				return SUCCESS;					
			}
		}
		else if(first_invalid<0)
			first_invalid=i;
	}

	if(first_invalid==FAIL){
		WARNING("ipv6 mapt/xlat mapping table FULL!!\n");
		return FAILED;
	}

	fc_db.ipv6_mapt_mappingTbl[first_invalid].isHash = 0U;
	fc_db.ipv6_mapt_mappingTbl[first_invalid].addr = *addr;
	atomic_inc(&fc_db.ipv6_mapt_mappingTbl[first_invalid].refCount);
	*i6MaptIndirectIndex = first_invalid;

	return SUCCESS;
}

int _rtk_fc_get_ipv6Hash_mapt_indirectMap_ref(uint32 hashValue, uint32 *i6MaptIndirectIndex)
{
	int i,first_invalid=-1;

	for(i = RTK_FC_I6MAPT_TABLE_START_INDEX ; i < RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE; i ++){
		//Find Old same info Entry
		if(atomic_read(&fc_db.ipv6_mapt_mappingTbl[i].refCount)!=0){
			if(fc_db.ipv6_mapt_mappingTbl[i].isHash && fc_db.ipv6_mapt_mappingTbl[i].v6_hash_value == hashValue){
				DEBUG("[IPV6 MAPT/XLAT]Find Match Entry (%d)!! Add reference count !",i);
				*i6MaptIndirectIndex = i;
				atomic_inc(&fc_db.ipv6_mapt_mappingTbl[i].refCount);
				return SUCCESS;
			}
		}
		else if(first_invalid<0)
			first_invalid=i;
	}

	if(first_invalid==FAIL){
		WARNING("ipv6 mapt/xlat mapping table FULL!!\n");
		return FAILED;
	}

	fc_db.ipv6_mapt_mappingTbl[first_invalid].isHash = 1U;
	fc_db.ipv6_mapt_mappingTbl[first_invalid].v6_hash_value = hashValue;
	atomic_inc(&fc_db.ipv6_mapt_mappingTbl[first_invalid].refCount);
	*i6MaptIndirectIndex = first_invalid;

	return SUCCESS;
}

int rtk_fc_get_ipv6_mapt_indirectMap_ref(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_maptIpv6Info_t *maptInfo)
{
	int sip_idx,dip_idx;

	if(pPktHdr==NULL || maptInfo==NULL)
		return FAILED;

	if(_rtk_fc_get_ipv6_mapt_indirectMap_ref(&pPktHdr->ip6h->saddr, &sip_idx)==SUCCESS &&
		_rtk_fc_get_ipv6_mapt_indirectMap_ref(&pPktHdr->ip6h->daddr, &dip_idx)==SUCCESS){
		maptInfo->sip_indirect_mapping_index=sip_idx;
		maptInfo->dip_indirect_mapping_index=dip_idx;
		return SUCCESS;
	}

	return FAILED;
}

int rtk_fc_get_ipv6Hash_mapt_indirectMap_ref(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_maptIpv6Info_t *maptInfo)
{
	int sip_idx,dip_idx;

	if(pFcIngressData==NULL || maptInfo==NULL)
		return FAILED;

	if(_rtk_fc_get_ipv6Hash_mapt_indirectMap_ref(pFcIngressData->srcIp, &sip_idx)==SUCCESS &&
		_rtk_fc_get_ipv6Hash_mapt_indirectMap_ref(pFcIngressData->dstIp, &dip_idx)==SUCCESS){
		maptInfo->sip_indirect_mapping_index=sip_idx;
		maptInfo->dip_indirect_mapping_index=dip_idx;
		return SUCCESS;
	}

	return FAILED;
}

#if defined(CONFIG_FC_RTL9607C_SERIES)
__IRAM_FWDENG
int rtk_fc_nicHook_rx_skb_NPTv6FastForward(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	int ret=RE8670_RX_CONTINUE;

#if 0 //#ifdef CONFIG_SMP
	++rg_db.systemGlobal.smp_statistic[RG_SMP_GMAC_RX][smp_processor_id()];
#endif
	if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism != RTK_FC_NPTV6_ACC_DISABLE)
	{
		if(((rtk_fc_rxdesc_t *)pRxInfo)->rx_reason==CPU_REASON_L34_FWD
			&& skb->data[0]==0x0
			&& skb->data[2]==0x5e)
		{
			((rtk_fc_rxdesc_t *)pRxInfo)->rx_gmac = cp->gmac;
			ret = _rtk_fc_NPTv6FastForward(skb->data, (rtk_fc_rxdesc_t *)pRxInfo, skb);
			if (ret !=RE8670_RX_CONTINUE)
				return ret;
		}
	}

	return ret;
}

__IRAM_FC_NICTRX
int _rtk_fc_NPTv6FastForward(char *skbData, rtk_fc_rxdesc_t *pRxDesc, struct sk_buff *skb)
{
	rtk_fc_nptv6_acceleration_t *pNptv6_acceleration;
	uint8 idx;
	
	//printk("rx_gmac=%d rx_dst_port_mask=0x%x rx_internal_priority=%d \n", pRxDesc->rx_gmac, pRxDesc->rx_dst_port_mask, pRxDesc->rx_internal_priority);
	//upstream	
	if(pRxDesc->rx_gmac==fc_db_fastFwd_data.nptv6_acc.upstreamGmac
		&& pRxDesc->rx_dst_port_mask==FC_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK
		&& ((skb==NULL && pRxDesc->rx_internal_priority==fc_db_fastFwd_data.nptv6_acc.upstreamPriority)
				|| (skb && pRxDesc->rx_internal_priority==fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg)))
	{	
		idx = skbData[3];
		if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[idx].vaild)
			pNptv6_acceleration = &fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[idx];
		else
			return RE8670_RX_CONTINUE;

		if(skb)
		{
			//modify dmac	
			*(u32 *)skbData = *(u32 *)pNptv6_acceleration->dmac.octet;
			//modify sipv6
			memcpy(&skbData[pNptv6_acceleration->ipv6_hdr_offset+8], pNptv6_acceleration->ipv6_addr.ipv6_addr, IPV6_ADDR_LEN);

			re8686_send_with_txInfo(skb, &pNptv6_acceleration->txInfo, 0);

			if(fc_db.fwdStatistic)
				atomic_inc(&fc_db.statistic.perPortCnt_NPTv6_FF_TX[pRxDesc->rx_src_port_num]);
		}
		else
		{
			//modify dmac
			FC_UNCACHE_DW(skbData) = *(u32 *)pNptv6_acceleration->dmac.octet;
			//modify sipv6
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+8) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[0]);
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+12) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[4]);
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+16) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[8]);
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+20) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[12]);

			if(fc_db.fwdStatistic)
				atomic_inc(&fc_db.statistic.perPortCnt_NPTv6_NIC_FF_TX[pRxDesc->rx_src_port_num]);
		}

		return RE8670_RX_STOP_SKBNOFREE;		
	}
	//downstream
	if(pRxDesc->rx_gmac==fc_db_fastFwd_data.nptv6_acc.downstreamGmac
		&& pRxDesc->rx_dst_port_mask==FC_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK
		&& ((skb==NULL && pRxDesc->rx_internal_priority==fc_db_fastFwd_data.nptv6_acc.downstreamPriority)
				|| (skb && pRxDesc->rx_internal_priority==fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg)))
	{
		idx = skbData[3];
		if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[idx].vaild)
			pNptv6_acceleration = &fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[idx];
		else
			return RE8670_RX_CONTINUE;

		if(skb)
		{
			//modify dmac			
			*(u32 *)skbData = *(u32 *)pNptv6_acceleration->dmac.octet;
			//modify dipv6
			memcpy(&skbData[pNptv6_acceleration->ipv6_hdr_offset+24], pNptv6_acceleration->ipv6_addr.ipv6_addr, IPV6_ADDR_LEN);

			re8686_send_with_txInfo(skb, &pNptv6_acceleration->txInfo, 0);

			if(fc_db.fwdStatistic)
				atomic_inc(&fc_db.statistic.perPortCnt_NPTv6_FF_TX[pRxDesc->rx_src_port_num]);
		}
		else
		{
			//modify dmac
			FC_UNCACHE_DW(skbData) = *(u32 *)pNptv6_acceleration->dmac.octet;
			//modify dipv6
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+24) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[0]);
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+28) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[4]);
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+32) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[8]);
			FC_UNCACHE_DW(skbData+pNptv6_acceleration->ipv6_hdr_offset+36) = *(u32 *)(&pNptv6_acceleration->ipv6_addr.ipv6_addr[12]);

			if(fc_db.fwdStatistic)
				atomic_inc(&fc_db.statistic.perPortCnt_NPTv6_NIC_FF_TX[pRxDesc->rx_src_port_num]);
		}
		
		return RE8670_RX_STOP_SKBNOFREE;		
	}

	return RE8670_RX_CONTINUE;
}

__IRAM_FC_NICTRX
int _rtk_fc_nptv6_ipoe_customized_TxHook(struct re_private *cp, struct tx_info *pTxInfo, int len)
{
	//DA, SA, etherType, IPv6 hdr
	*(unsigned short *)(pTxInfo->addr+14/*DA, SA, etherType*/+4/*ipv6 payload len*/) = htons(len-54);

	//if(rg_db.systemGlobal.fwdStatistic)
		//rg_db.systemGlobal.statistic.perPortCnt_NPTv6_NIC_FF_TX[pRxInfo->opts3.bit.src_port_num]++;

	return len-54-2;
}

__IRAM_FC_NICTRX
int _rtk_fc_nptv6_pppoe_customized_TxHook(struct re_private *cp, struct tx_info *pTxInfo, int len)
{
	//DA, SA, etherType, pppoe, IPv6 hdr
	*(unsigned short *)(pTxInfo->addr+14/*DA, SA, etherType*/+4/*ipv6 payload len*/) = htons(len-62);

	//if(rg_db.systemGlobal.fwdStatistic)
		//rg_db.systemGlobal.statistic.perPortCnt_NPTv6_NIC_FF_TX[pRxInfo->opts3.bit.src_port_num]++;

	return len-62-2;
}

__IRAM_FC_NICTRX
int nic_rx_skb_NPTv6FastForward(struct re_private *cp, struct rx_info *pRxInfo, int len)
{
	char *skbData=(char *)(pRxInfo->addr+2);//dma offset
		
	if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism != RTK_FC_NPTV6_ACC_DISABLE)
	{
		//dump_packet(skbData, len, "nic_rx_skb_NPTv6FastForward");
		((rtk_fc_rxdesc_t *)pRxInfo)->rx_gmac = cp->gmac;
		_rtk_fc_NPTv6FastForward(skbData, (rtk_fc_rxdesc_t *)pRxInfo, NULL);
	}

	return len;
}

int _rtk_fc_nptv6_customized_txPrepare(struct re_private *cp, struct tx_info *pTxInfo)
{
	unsigned char *pData=(unsigned char *)pTxInfo->addr;

	if(fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length>MAX_HWNAT_CUSTOMIZED_TX_HDR_BUFFER_SIZE)
		WARNING("preallocated_outer_length=%d is out of range (>%d)", fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length, MAX_HWNAT_CUSTOMIZED_TX_HDR_BUFFER_SIZE);
	memcpy(pData, fc_db_fastFwd_data.nptv6_acc.preallocated_outer, (fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length>MAX_HWNAT_CUSTOMIZED_TX_HDR_BUFFER_SIZE)?MAX_HWNAT_CUSTOMIZED_TX_HDR_BUFFER_SIZE:fc_db_fastFwd_data.nptv6_acc.preallocated_outer_length);

	return 0;
}

void rtk_fc_nptv6_check_flow_canBeAdded(uint8 *flow_can_add, int direction,
														 void *pPathData, int inner_flowHashIdx, int outer_flowHashIdx,
														 rtk_fc_g3IgrExtraInfo_t * pG3IgrExtraInfo ,
												 		 rtk_fc_pktHdr_t *pPktHdr, 
												 	 	 struct rt_skbuff *rtskb,
														 int igrNetifIdx, 
														 int egrNetifIdx,
														  rtk_fc_igrExtraInfo_t *extraInfo,
														  struct rt_nfconn *rtct,
														  int *entry_exist)
{
	int canAdd = 1;
	int flowIndex = 0, ret = 0;
	
	rtk_rg_asic_path3_entry_t *pFlowPath3;
	rtk_rg_asic_path1_entry_t tmp_flowEntry;
	
	pFlowPath3 = (rtk_rg_asic_path3_entry_t *)&tmp_flowEntry;
	
	ret = _rtk_fc_flow_freeEntry_get(&flowIndex, inner_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)pPathData, 0, 0, 0, 0, 0, pG3IgrExtraInfo, 0);
	if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
	{
		DEBUG("[NPTv6]inner flow exist! Do nothing");
		*entry_exist =1;
	}
	else if(ret !=RTK_FC_RET_OK )
	{
		DEBUG("[NPTv6]inner flow get entry failed!! Cannot add outer flow!ret = %x",ret);
		canAdd = 0;
	}
	else if(flowIndex >= fc_db.flowHwTableSize)
	{
		DEBUG("[NPTv6]inner flow is sw flow!! Cannot add outer flow!");
		canAdd = 0;
	}
	rtk_fc_nptv6_fastFwd_outerFlow_prepare(pFlowPath3,
											outer_flowHashIdx, 
											pPktHdr, 
											rtskb, 
											igrNetifIdx, 
											egrNetifIdx,
											extraInfo,
											pG3IgrExtraInfo,
											rtct);
	pPktHdr->nptv6_outerFlow =0U; // reset to 0

	ret = _rtk_fc_flow_freeEntry_get(&flowIndex, outer_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)(&tmp_flowEntry), 0, 0, 0, 0, 0, pG3IgrExtraInfo, 0);
	if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
	{
		DEBUG("[NPTv6]Outer flow exist! Do nothing");
		*entry_exist =1;
	}
	else if(ret !=RTK_FC_RET_OK )
	{
		DEBUG("[NPTv6]Outer flow get entry failed!! Cannot add outer flow!ret = %x", ret);
		canAdd = 0;
	}
	else if(flowIndex >= fc_db.flowHwTableSize)
	{
		DEBUG("[NPTv6]Outer flow is sw flow!! Cannot add outer flow!");
		canAdd = 0;
	}
	if(canAdd == 0)
		*flow_can_add = 0U;
	
}


int _rtk_fc_get_nptv6_flowInfo_idx(int *idx)
{
	int i;

	// 0: reserved
	for(i = 1 ; i < MAX_NPTV6_ACC_UPSTREAM_SIZE+MAX_NPTV6_ACC_DOWNSTREAM_SIZE; i++)
	{
		if(fc_db.nptv6_flow_info[i].isSet==0U)
		{
			*idx = i;
			return SUCCESS;
		}
	}
	return FAILED;
}
int rtk_fc_nptv6_fastFwd_outerFlow_prepare(rtk_rg_asic_path3_entry_t *pFlowPath3,
															 int outer_flow_hash_index, 
															 rtk_fc_pktHdr_t *pPktHdr, 
															 struct rt_skbuff *rtskb, 
															 int srcNetifIdx, 
															 int egrNetifIdx,
															 rtk_fc_igrExtraInfo_t *extraInfo,
															 rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo,
															 struct rt_nfconn *rtct)
{


	
	int in_l4proto = pPktHdr->tcph?1:0;

	

	pPktHdr->nptv6_outerFlow =1U;
	{
		memset(pFlowPath3, 0, sizeof(rtk_rg_asic_path3_entry_t));
		pFlowPath3->in_path = FB_PATH_34;	//path 3
		pFlowPath3->valid = TRUE;
		pFlowPath3->in_multiple_act = FALSE;
		pFlowPath3->in_ipv4_or_ipv6 = TRUE;
		
		pFlowPath3->in_src_ipv6_addr_hash = _rtk_rg_flowHashIPv6SrcAddr_get(pPktHdr->ip6h->saddr.s6_addr);
		pFlowPath3->in_dst_ipv6_addr_hash = _rtk_rg_flowHashIPv6DstAddr_get(pPktHdr->ip6h->daddr.s6_addr);
		pFlowPath3->in_l4_src_port = (in_l4proto)?ntohs(pPktHdr->tcph->source):ntohs(pPktHdr->udph->source);
		pFlowPath3->in_l4_dst_port = (in_l4proto)?ntohs(pPktHdr->tcph->dest):ntohs(pPktHdr->udph->dest);
		pFlowPath3->in_l4proto = in_l4proto;
		pFlowPath3->in_intf_idx = srcNetifIdx;
		
		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM) // for down stream carry vlan to lan
			pFlowPath3->in_stagif = (pPktHdr->svh)?TRUE:FALSE;
		else
			pFlowPath3->in_stagif = FALSE;
		
		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)// for down stream carry vlan to lan
		{
			pFlowPath3->in_ctagif = (pPktHdr->cvh)?TRUE:FALSE;
			pFlowPath3->in_cvlan_pri = (pPktHdr->cvh)?pPktHdr->cvlanpri:0U;

		}
		else
			pFlowPath3->in_ctagif = FALSE;
		if(dynamic_sram_desc)
			pFlowPath3->in_pppoeif = FALSE;
		else
			pFlowPath3->in_pppoeif = (pPktHdr->ppph) ? TRUE : FALSE;


		pFlowPath3->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS];
		pFlowPath3->in_tos = (((pPktHdr->ip6h->priority<<4)&0xf0)|((pPktHdr->ip6h->flow_lbl[0]>>4)&0xf));

		//-------------------------egress action-------------------------

		pFlowPath3->out_intf_idx = egrNetifIdx;
			
		pFlowPath3->out_portmask = 1 << (pPktHdr->egressPort.macPortIdx);
		pFlowPath3->out_ext_portmask_idx = 0U;
		
		pFlowPath3->out_smac_trans = ((pPktHdr->fwdType == RTK_FC_FWDTYPE_ROUTING)||(pPktHdr->fwdType == RTK_FC_FWDTYPE_NAPT))? TRUE:FALSE;
		pFlowPath3->out_dmac_trans = 0U;
		DEBUG("Setting nptv6 outer flow, svid = %d, cvid = %d",pPktHdr->svlanid, pPktHdr->cvlanid);
		//svlan
		pFlowPath3->out_egress_svid_act = (pPktHdr->svh) ? TRUE : FALSE;
		pFlowPath3->out_svlan_id = (pPktHdr->svh) ? pPktHdr->svlanid : 0U;
		pFlowPath3->out_stag_format_act = TRUE;
		pFlowPath3->out_svid_format_act = ((pPktHdr->svh))?TRUE:FALSE;
		pFlowPath3->out_spri_format_act = ((pPktHdr->svh))?TRUE:FALSE;
		pFlowPath3->out_spri = (pPktHdr->svh) ? pPktHdr->svlanpri : 0U;
		//cvlan
		pFlowPath3->out_egress_cvid_act = TRUE;
		pFlowPath3->out_cvlan_id = (pPktHdr->cvh) ? pPktHdr->cvlanid : 0U;
		pFlowPath3->out_ctag_format_act = TRUE;
		pFlowPath3->out_cvid_format_act = ((pPktHdr->cvh))?TRUE:FALSE;
		pFlowPath3->out_cpri_format_act = ((pPktHdr->cvh))?TRUE:FALSE;

		pFlowPath3->out_cpri = (pPktHdr->cvh) ? pPktHdr->cvlanpri : 0U;

		//other
		pFlowPath3->out_multiple_act = FALSE;
		if(pPktHdr->remarkDec.outputQid)
			pFlowPath3->out_user_pri_act = TRUE;
		else
			pFlowPath3->out_user_pri_act = FALSE;
		pFlowPath3->out_user_priority = pPktHdr->remarkDec.outputQid;
		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS])
		{
			// remarking dscp only when dscp as flow key
			pFlowPath3->out_dscp_act = TRUE;
			pFlowPath3->out_dscp = (((pPktHdr->ip6h->priority<<4) | (pPktHdr->ip6h->flow_lbl[0]>>4))>>2) & 0x3f;
		}

		pFlowPath3->out_stream_idx_act = pPktHdr->remarkDec.streamId_en;
		pFlowPath3->out_stream_idx = pPktHdr->remarkDec.streamId;

		pFlowPath3->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
		pFlowPath3->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

		pFlowPath3->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
		pFlowPath3->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;

		pFlowPath3->out_extra_tag_index = 0U;
		pFlowPath3->out_egress_port_to_vid_act = FALSE;
		pFlowPath3->out_drop = 0U;
		pFlowPath3->lock = (pFlowPath3->out_share_meter_act) ? TRUE: FALSE;
		
	}
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

int _rtk_fc_dynamic_prehashPtn_deleteMcFlow(void)
{
	return SUCCESS;
}


int _rtk_fc_dynamic_prehashPtn_moveMcFlow(uint32 sip_ptn,uint32 dip_ptn,uint32 sport_ptn,uint32 dport_ptn)
{
	return SUCCESS;
}

int _rtk_fc_dynamic_calculate_prehashPtn(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_forwardingType_t fwdType)
{
	uint8 *tmp_flowTable=NULL;
	uint32 upstreamHashIndex = 0U, downstreamHashIndex = 0U;
	uint32 param1_us=0U, param2_us=0U, param3_us=0U, param4_us=0U;
	uint32 param1_ds=0U, param2_ds=0U, param3_ds=0U, param4_ds=0U;
	uint32 extraItem_us = 0U;
	uint32 extraItem_ds = 0U;
	uint8 tos = 0U, in_l4proto = 0U;
	uint32 sip_ptn, dip_ptn, sport_ptn = 0x0U, dport_ptn = 0x0U;
	rtk_fc_dynamic_prehash_info_t dynamic_prehash_info;
	int min_collision = fc_db.flowHwTableSize;
	unsigned long int msec2jiffies=0UL;
	int i = 0;
	int igrDSCP = 0;
	uint32 sport_ptn_start = 0U,sport_ptn_end = 0U,dport_ptn_start = 0U , dport_ptn_end = 0U;
	int loop_cnt = 0;
	
	tmp_flowTable = RTK_FC_HELPER_FC_KMALLOC(sizeof(uint8)*fc_db.flowHwTableSize, GFP_ATOMIC);
	// Back up original setting

	{
		// downstream
		// Reverse one: sip = egress dip, dip = egress sip
		tos = fc_db.dynamic_prehash_5tuple_info.ds_tos;
		in_l4proto = fc_db.dynamic_prehash_5tuple_info.ds_in_l4proto;
		
		param1_ds = fc_db.dynamic_prehash_5tuple_info.ds_sport; // should be 1024
		param2_ds = fc_db.dynamic_prehash_5tuple_info.ds_dport; // should be 1024
		param3_ds = fc_db.dynamic_prehash_5tuple_info.ds_sip;
		param4_ds = fc_db.dynamic_prehash_5tuple_info.ds_dip; 
		
		{
			
			extraItem_ds = 0U;
			igrDSCP = tos >> 2;
			
			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_SVID]==DISABLED){
				extraItem_ds |= (fc_db.dynamic_prehash_5tuple_info.ds_svid<<12);
				//DEBUG("flow hash with SVID %d", igrSVID);
			}else
				extraItem_ds &= 0x000fffU;
				
			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_CVID]==DISABLED){
				extraItem_ds |= fc_db.dynamic_prehash_5tuple_info.ds_cvid;
				//DEBUG("flow hash with CVID %d", igrCVID);
			}else
				extraItem_ds &= 0xfff000U;

			extraItem_ds = (in_l4proto<<23) ^ extraItem_ds;

			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_DSCP]==DISABLED){
				extraItem_ds = (((igrDSCP >> 2)&0x3f)<<3) ^ extraItem_ds;
				//DEBUG("flow hash with DSCP %d", igrDSCP);
			}

			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_CPRI]==DISABLED){
				extraItem_ds = (fc_db.dynamic_prehash_5tuple_info.ds_cpri&0x7) ^ extraItem_ds;
				//DEBUG("flow hash with CPRI %d", igrCPRI);
			}

		}
		
		// upstream
		
		tos = fc_db.dynamic_prehash_5tuple_info.us_tos;
		in_l4proto = fc_db.dynamic_prehash_5tuple_info.us_in_l4proto;
		param1_us = fc_db.dynamic_prehash_5tuple_info.us_sport;
		param2_us = fc_db.dynamic_prehash_5tuple_info.us_dport;
		param3_us = fc_db.dynamic_prehash_5tuple_info.us_sip;
		param4_us = fc_db.dynamic_prehash_5tuple_info.us_dip;
		{
			extraItem_us = 0U;
			igrDSCP = tos >> 2;
			
			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_SVID]==DISABLED){
				extraItem_us |= (fc_db.dynamic_prehash_5tuple_info.us_svid<<12);
				//DEBUG("flow hash with SVID %d", igrSVID);
			}else
				extraItem_us &= 0x000fffU;
				
			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_CVID]==DISABLED){
				extraItem_us |= fc_db.dynamic_prehash_5tuple_info.us_cvid;
				//DEBUG("flow hash with CVID %d", igrCVID);
			}else
				extraItem_us &= 0xfff000U;

			extraItem_us = (in_l4proto<<23) ^ extraItem_us;

			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_DSCP]==DISABLED){
				extraItem_us = (((igrDSCP >> 2)&0x3f)<<3) ^ extraItem_us;
				//DEBUG("flow hash with DSCP %d", igrDSCP);
			}

			if(fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_CPRI]==DISABLED){
				extraItem_us = (fc_db.dynamic_prehash_5tuple_info.us_cpri&0x7) ^ extraItem_us;
				//DEBUG("flow hash with CPRI %d", igrCPRI);
			}

		}
		
		
	}
	/*
	if(fwdType == RTK_FC_FWDTYPE_BRIDGE) // Bridge need full run
	{
		sport_ptn_start = 0x0;
		sport_ptn_end = 0xf0000;
		dport_ptn_start = 0x0;
		dport_ptn_end = 0xf0000;
	}
	else
	{
		sport_ptn_start = 0x0;
		sport_ptn_end = 0x0;
		dport_ptn_start = 0x0;
		dport_ptn_end = 0x0;

	}
	*/
	sport_ptn_start = 0x0U;
	sport_ptn_end = 0xf0000U;
	dport_ptn_start = 0x0U;
	dport_ptn_end = 0xf0000U;
	memset(&dynamic_prehash_info, 0 , sizeof(dynamic_prehash_info));
	EVENT("Before calculate: jiffies = %lu\n",jiffies);
	for(sport_ptn = sport_ptn_start; sport_ptn <= sport_ptn_end ; sport_ptn +=0x10000U)
	{
		for(dport_ptn = dport_ptn_start; dport_ptn <= dport_ptn_end ; dport_ptn +=0x10000U)
		{
			for(sip_ptn = 0x800000U;sip_ptn <=0xf00000U; sip_ptn +=0x100000U)
		    {

		        for(dip_ptn = 0x800000U;dip_ptn <=0xf00000U; dip_ptn +=0x100000U)
		        {
		    		int find = 1;
					int total_collision = 0;
					int ip_counter = 0;
		            memset(tmp_flowTable, 0 , sizeof(uint8)*fc_db.flowHwTableSize);
					for(i = 0; i < 4000 ; i++)
					{
						upstreamHashIndex = _rtk_fc_sw_flowHashIndexStep1_get_with_prehashPattern(param1_us+i, param2_us+i, param3_us+ip_counter, param4_us, extraItem_us, sip_ptn, dip_ptn, sport_ptn, dport_ptn);

						if(tmp_flowTable[upstreamHashIndex]!=0)
							total_collision++;
						tmp_flowTable[upstreamHashIndex]++;
						ip_counter+=1;
						if(ip_counter==10)
							ip_counter = 0;

					}
					
					for(i = 0; i < 4000  ; i++)
					{
						downstreamHashIndex = _rtk_fc_sw_flowHashIndexStep1_get_with_prehashPattern(param1_ds+i, param2_ds+i, param3_ds, param4_ds, extraItem_ds, sip_ptn, dip_ptn, sport_ptn, dport_ptn);

						if(tmp_flowTable[downstreamHashIndex]!=0)
							total_collision++;
						
						tmp_flowTable[downstreamHashIndex]++;
						
					}
					
					if(min_collision > total_collision)
					{
						min_collision = total_collision;
						dynamic_prehash_info.collision = min_collision;
						dynamic_prehash_info.dip_ptn= dip_ptn;
						dynamic_prehash_info.sip_ptn= sip_ptn;
						dynamic_prehash_info.dport_ptn= dport_ptn;
						dynamic_prehash_info.sport_ptn= sport_ptn;
					}
					for( i = 0 ; i < fc_db.flowHwTableSize ; i++)
		            {
		                if(tmp_flowTable[i]>=2)
		                {
		                   find = 0;
						   break;
		                }
						
		            }
					loop_cnt ++;// for statistic
					if(find==1)
					{
						
						EVENT("Find No collision algo with sip: %x dip: %x remoteIP: %x sport: %d dport:%d\n", fc_db.dynamic_prehash_5tuple_info.us_sip
																					        					  , fc_db.dynamic_prehash_5tuple_info.us_dip
																					        					  , fc_db.dynamic_prehash_5tuple_info.ds_sip
																					        					  , fc_db.dynamic_prehash_5tuple_info.us_sport
																					        					  , fc_db.dynamic_prehash_5tuple_info.us_dport);
						EVENT("sip_pattern: %x dip_pattern: %x sport_pattern: %x dport_pattern: %x\n",sip_ptn, dip_ptn, sport_ptn, dport_ptn);
						if(sport_ptn!= fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_SPORT] ||
							dport_ptn!= fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_DPORT] ||
							sip_ptn!= fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_SIP] ||
							dip_ptn!= fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_DIP])
						{

							//Set HWNAT to 0
							fc_db.controlFuc.hwnat_mode = RT_FLOW_HWNAT_MODE_DIS_ACC;
							RTK_FC_HELPER_MGR_HWNAT_MODE_SET(RT_FLOW_HWNAT_MODE_DIS_ACC);

							//creat new multicast flow 
							_rtk_fc_dynamic_prehashPtn_moveMcFlow(sip_ptn,dip_ptn,sport_ptn,dport_ptn);
							
							rtk_fc_flow_flush_forHashChange();//clear all flow

							//delete old multicast flow
							_rtk_fc_dynamic_prehashPtn_deleteMcFlow();

							// Set pattern
							
							RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SPORT,  sport_ptn);
							RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DPORT, dport_ptn);
							RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SIP, sip_ptn);
							RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DIP, dip_ptn);
							
							fc_db.dynamic_calculate_prehash_isSet = 1U;
							EVENT("[found]Return: jiffies = %lu loop_cnt = %d\n",jiffies, loop_cnt);

							//rtk_fc_user_pipe_cmd("sleep 3");
						
							EVENT("[RTNL JOB] Start a Event Timer");
						
							RTK_FC_HELPER_MSECS_TO_JIFFIES(RTK_FC_DYNAMIC_PREHASHPTN_EVENT_TICK_MSECONDMS, &msec2jiffies);
						
							RTK_FC_HELPER_MOD_TIMER(fc_db.dynamic_prehashPtn_eventTimer, jiffies+msec2jiffies);

							
							//rtk_fc_user_pipe_cmd("echo 1 > /proc/fc/ctrl/hwnat");
							if(tmp_flowTable)  RTK_FC_HELPER_FC_KFREE(tmp_flowTable, sizeof(uint8)*fc_db.flowHwTableSize);
							return RTK_FC_DYNAMIC_PREHASHPTH_SKIP_FLOW_LEARNING;

						}
						else if(sport_ptn== fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_SPORT] &&
							dport_ptn== fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_DPORT] &&
							sip_ptn== fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_SIP] &&
							dip_ptn== fc_db.systemGlobal.preHashPtn[FB_PREHASH_PTN_DIP]
						)
						{
							EVENT("Already set! Do nothing.\n");
							if(tmp_flowTable)  RTK_FC_HELPER_FC_KFREE(tmp_flowTable, sizeof(uint8)*fc_db.flowHwTableSize);
							return RTK_FC_DYNAMIC_PREHASHPTH_KEEP_FLOW_LEARNING;
						}
						
					}

		        }
			}
		}
	}
	
		
	

	
	EVENT("[Can't find]Return: jiffies = %lu loop_cnt = %d\n",jiffies, loop_cnt);
	
	{
		EVENT("Use least collision pattern (total %d collision)! dip %x sip %x sport %x dport %x",min_collision,
			 																					  dynamic_prehash_info.dip_ptn,
																			 					  dynamic_prehash_info.sip_ptn,
																								  dynamic_prehash_info.dport_ptn,
																			 					  dynamic_prehash_info.sport_ptn);
			
		 //Set HWNAT to 0
		 fc_db.controlFuc.hwnat_mode = RT_FLOW_HWNAT_MODE_DIS_ACC;
		 RTK_FC_HELPER_MGR_HWNAT_MODE_SET(RT_FLOW_HWNAT_MODE_DIS_ACC);
				 
		 //creat new multicast flow 
		 _rtk_fc_dynamic_prehashPtn_moveMcFlow(dynamic_prehash_info.sip_ptn,dynamic_prehash_info.dip_ptn,dynamic_prehash_info.sport_ptn,dynamic_prehash_info.dport_ptn);
		 
		 rtk_fc_flow_flush_forHashChange();//clear all flow
		 
		 //delete old multicast flow
		 _rtk_fc_dynamic_prehashPtn_deleteMcFlow();


		 // Set pattern
		 
		 RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SPORT,  dynamic_prehash_info.sport_ptn);
		 RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DPORT, dynamic_prehash_info.dport_ptn);
		 RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SIP, dynamic_prehash_info.sip_ptn);
		 RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DIP, dynamic_prehash_info.dip_ptn);

		 fc_db.dynamic_calculate_prehash_isSet = 1U;
		 
		 EVENT("[found]Return: jiffies = %lu loop_cnt = %d\n",jiffies, loop_cnt);

		 //rtk_fc_user_pipe_cmd("sleep 3");
	 
		 EVENT("[RTNL JOB] Start a Event Timer");
	 
		 RTK_FC_HELPER_MSECS_TO_JIFFIES(RTK_FC_DYNAMIC_PREHASHPTN_EVENT_TICK_MSECONDMS, &msec2jiffies);
	 
		 RTK_FC_HELPER_MOD_TIMER(fc_db.dynamic_prehashPtn_eventTimer, jiffies+msec2jiffies);
			
			
			
	}
	if(tmp_flowTable)  RTK_FC_HELPER_FC_KFREE(tmp_flowTable, sizeof(uint8)*fc_db.flowHwTableSize);
	return RTK_FC_DYNAMIC_PREHASHPTH_SKIP_FLOW_LEARNING;

	
}


int _rtk_fc_dynamic_set_prehashPtn_process(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_forwardingType_t fwdType)
{
	int ret = SUCCESS;
	
	{
		if(fc_db.dynamic_calculate_prehash_isSet ==1 && time_after_eq(jiffies, fc_db.dynamic_calculate_prehash_jiffies + fc_db.dynamic_calculate_prehash_timeout_sec))
		{
			EVENT("After 60 second, re-open dynamic calculate prehash ptn process.");
			fc_db.dynamic_calculate_prehash_isSet = 0U;
			fc_db.dynamic_calculate_prehash_count = 0U;
		}
		else if(fc_db.dynamic_calculate_prehash_isSet ==1U)
			return SUCCESS;
		
		if(fc_db.dynamic_calculate_prehash_isSet ==0U )
		{
//NEW_ONE:
			if(fc_db.dynamic_calculate_prehash_count  == 0U )
			{
				if(pFcIngressData->srcPort - pFcIngressData->dstPort!=0)
				{
					DEBUG("port diff not 0, return success and do nothing.");
					return SUCCESS;
				}
				fc_db.dynamic_calculate_prehash_port_diff = pFcIngressData->srcPort - pFcIngressData->dstPort;
				
				
				//Record the first info
				if(pPktHdr->iph == NULL && pPktHdr->ip6h == NULL)
				{
					DEBUG("No ip/ip6 header, do nothing.");
					return SUCCESS;
				}
				// downstream info
				fc_db.dynamic_prehash_5tuple_info.ds_tos = (pPktHdr->iph) ?pPktHdr->iph->tos:(((pPktHdr->ip6h->priority<<4)&0xf0)|((pPktHdr->ip6h->flow_lbl[0]>>4)&0xf));
				fc_db.dynamic_prehash_5tuple_info.ds_in_l4proto = (pPktHdr->tcph)?1U:0U;
				fc_db.dynamic_prehash_5tuple_info.ds_sport = pFcIngressData->srcPort;
				fc_db.dynamic_prehash_5tuple_info.ds_dport = pFcIngressData->dstPort;
				fc_db.dynamic_prehash_5tuple_info.ds_sip = (pPktHdr->iph)?ntohl(pPktHdr->iph->daddr) : pPktHdr->ipv6Dip_hash;
				fc_db.dynamic_prehash_5tuple_info.ds_dip = (pPktHdr->iph)?ntohl(pPktHdr->iph->saddr) : pPktHdr->ipv6Sip_hash; 
				fc_db.dynamic_prehash_5tuple_info.ds_svid = pPktHdr->svlanid;
				fc_db.dynamic_prehash_5tuple_info.ds_cvid = pPktHdr->cvlanid;
				fc_db.dynamic_prehash_5tuple_info.ds_cpri = pPktHdr->cvlanpri;

				//upstream info
				fc_db.dynamic_prehash_5tuple_info.us_tos = (pPktHdr->iph) ?pFcIngressData->v4tos:pFcIngressData->v6tos;
				fc_db.dynamic_prehash_5tuple_info.us_in_l4proto = (pPktHdr->tcph)?1U:0U;
				fc_db.dynamic_prehash_5tuple_info.us_sport = pFcIngressData->srcPort;
				fc_db.dynamic_prehash_5tuple_info.us_dport = pFcIngressData->dstPort;
				fc_db.dynamic_prehash_5tuple_info.us_sip = pFcIngressData->srcIp;
				fc_db.dynamic_prehash_5tuple_info.us_dip = pFcIngressData->dstIp;
				fc_db.dynamic_prehash_5tuple_info.us_svid = pFcIngressData->srcSVlanId;
				fc_db.dynamic_prehash_5tuple_info.us_cvid = pFcIngressData->srcCVlanId;
				fc_db.dynamic_prehash_5tuple_info.us_cpri = pFcIngressData->srcCVlanPri;
			}
			/*
			TMP("port_diff = %d, Igrsip = %x, Igrdip = %x, EgrSip = %x EgrDip = %x",fc_db.dynamic_calculate_prehash_port_diff,
						   															  fc_db.dynamic_prehash_5tuple_info.us_sip,
																 					  fc_db.dynamic_prehash_5tuple_info.us_dip,
																 					  fc_db.dynamic_prehash_5tuple_info.ds_sip,
																 					  fc_db.dynamic_prehash_5tuple_info.ds_dip);

     		  */
			if((fc_db.dynamic_calculate_prehash_port_diff == pFcIngressData->srcPort - pFcIngressData->dstPort) &&
				(fc_db.dynamic_prehash_5tuple_info.us_sip == pFcIngressData->srcIp) &&
				(fc_db.dynamic_prehash_5tuple_info.us_dip == pFcIngressData->dstIp) &&
				(fc_db.dynamic_prehash_5tuple_info.ds_sip == ((pPktHdr->iph)?ntohl(pPktHdr->iph->daddr) : pPktHdr->ipv6Dip_hash)) &&
				(fc_db.dynamic_prehash_5tuple_info.ds_dip == ((pPktHdr->iph)?ntohl(pPktHdr->iph->saddr) : pPktHdr->ipv6Sip_hash))
				
			)
			{
				if(fc_db.dynamic_calculate_prehash_count<20U)
				{
					EVENT("Same pattern!dynamic_calculate_prehash_count  plus 1");
					fc_db.dynamic_calculate_prehash_count+=1U;

				}

			}

		}

		
		if(fc_db.dynamic_calculate_prehash_count == 20U && fc_db.dynamic_calculate_prehash_isSet ==0U)
		{
			ret = _rtk_fc_dynamic_calculate_prehashPtn(pPktHdr, pFcIngressData, fwdType);	
		}
			
	}

	
	return ret;
}
#endif


#if defined(CONFIG_RTK_L34_G3_PLATFORM)

int rtk_fc_syncToHw_G3updateGroupHash(int32 swFlowIdx,uint32 h_idx,uint32 mceIdx,int32 defOuterTagTCI,uint16 igrIntf)
{
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	aal_hash_action_t action = {0};
	rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowIdx=NULL;

	if(pAbstrSwFlowEt->g3KeyHwPriv.hwacc)
	{
		
		mcSwFlowIdx = rtk_fc_igmp_mcExtraFlowIdxTbl_find(swFlowIdx);
		if(mcSwFlowIdx==NULL){IGMP("can't get mcflowIdxTbl"); return FAIL;}


#if defined(CONFIG_FC_G3_G3LITE_SERIES)	
{
		action.mac_da_vld = 1;
		if(!mcSwFlowIdx->isIpv6) 
		{
			action.mac_da_5 = 0x01;
			action.mac_da_4 = 0x00;
			action.mac_da_3 = 0x5E;
			action.mac_da_2 = (mcSwFlowIdx->multicastAddress[0] & 0x7F0000) >> 16;
			action.mac_da_1 = (mcSwFlowIdx->multicastAddress[0] & 0x00FF00) >> 8;
			action.mac_da_0 = (mcSwFlowIdx->multicastAddress[0] & 0x0000FF);
		}
		else
		{
			action.mac_da_5 = 0x33;
			action.mac_da_4 = 0x33;
			action.mac_da_3 = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0xFF000000) >> 24;
			action.mac_da_2 = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0x00FF0000) >> 16;
			action.mac_da_1 = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0x0000FF00) >> 8;
			action.mac_da_0 = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0x000000FF);
		}

		if(defOuterTagTCI==FAIL)
		{
			//remove all vlan tag in hash
			action.vlan_cnt = 0;
			action.vlan_vld = 1;
		}
		else
		{
			//always give a default top vlan
			action.vlan_cnt = 1;
			action.vlan_vld = 1;
			action.top_tpid_enc = 1;
			action.top_dei_sel = 1; //using headeri  (keep ori)
			action.top_vid = (defOuterTagTCI&VLAN_VID_MASK);
			action.top_802_1p_sel=2;	//from fib
			action.top_802_1p = (defOuterTagTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
		}

		//assign mce_index		
		action.mc=1;
		action.mcgid = mceIdx;
		action.dpid_vld = 1;
		action.permit = 1;
		action.dpid_pri = 1;	 
	
		action.pppoe_set = 1;
		action.pppoe_vld = 0;	
		
		action.chk_msk_ptr = hash_mask_multicast_forward_idx;
		
		if (CA_E_OK != aal_hash_action_update(0, &action, TUPLE_TYPE_FWD, hash_mask_multicast_forward_idx, l3fe_main_hash_profile_mc, h_idx))
		{
			IGMP("Hash update failed \n");
		}
		
}
#elif defined(CONFIG_FC_CA8277B_SERIES)
{
		//set mac index table at first
		idx_intf_mac_entry_t mac_entry;
		ca_uint8_t mac_da[6];
		int da_rslt_idx = 0;
		int ret;
		memset(&mac_entry, 0, sizeof(mac_entry));
		if(!mcSwFlowIdx->isIpv6) {
			mac_da[0] = 0x01;
			mac_da[1] = 0x00;
			mac_da[2] = 0x5E;
			mac_da[3] = (mcSwFlowIdx->multicastAddress[0] & 0x7F0000) >> 16;
			mac_da[4] = (mcSwFlowIdx->multicastAddress[0] & 0x00FF00) >> 8;
			mac_da[5] = (mcSwFlowIdx->multicastAddress[0] & 0x0000FF);
		}
		else
		{
			mac_da[0] = 0x33;
			mac_da[1] = 0x33;
			mac_da[2] = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0xFF000000) >> 24;
			mac_da[3] = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0x00FF0000) >> 16;
			mac_da[4] = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0x0000FF00) >> 8;
			mac_da[5] = (ntohl(mcSwFlowIdx->multicastAddress[3]) & 0x000000FF);
		}
		array_reverse_copy(mac_entry.mac_da, mac_da, 6);
		
		/*find old entry */
		ret = aal_entry_get(AAL_TABLE_IDX_INTF_MAC, &mac_entry, &da_rslt_idx);
		if(ret != CA_E_OK) {
			return CA_E_ERROR;
		}
		action.mac_da_idx = da_rslt_idx;
		action.mac_da_idx_vld = 1;

		if(defOuterTagTCI==FAIL)
		{
			//remove all vlan tag in hash
			action.vlan_cnt = 0;
			action.vlan_vld = 1;
		}
		else
		{
			//always give a default top vlan
			action.vlan_cnt = 1;
			action.vlan_vld = 1;
			action.top_tpid_enc = 1;
			action.top_dei_sel = 1; //using headeri  (keep ori)
			action.top_vid = (defOuterTagTCI&VLAN_VID_MASK);
			action.top_802_1p_sel=2;	//from fib
			action.top_802_1p = (defOuterTagTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
		}

		//assign mce_index		
		action.mc=1;
		action.mcgid = mceIdx;
		action.dpid_vld = 1;
		action.permit = 1;
		action.dpid_pri = 1;	 

		action.pppoe_set = 1;
		action.pppoe_vld = 0;

		//interface mib
		action.mdata_w_vld_1 = 0xF;
		action.mdata_w_1 = (igrIntf+RTK_FC_MC_HW_NETIF_IDXSHIFT)<<8; // egress interface fix to 0
		
		action.chk_msk_ptr = hash_mask_multicast_forward_idx;
		
		if (CA_E_OK != aal_hash_action_update(0, &action, TUPLE_TYPE_FWD, hash_mask_multicast_forward_idx, l3fe_main_hash_profile_mc, h_idx))
		{
			IGMP("Hash update failed \n");
		}
		
}
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
{
	int32 aal_ret=CA_E_ERROR;
	rtk_fc_table_mcGroupTbl_t* pGroupTbl=NULL;

	pGroupTbl = rtk_fc_mc_findKnownGroupEntry(mcSwFlowIdx->isIpv6,mcSwFlowIdx->multicastAddress);

	if(pGroupTbl && pGroupTbl->hwIdx2!=FAIL)
	{
		action.mac_da_idx_vld=1;
		action.mac_da_idx = pGroupTbl->hwIdx2;
	}
	else
		WARNING("pGroupTbl not found or hwIdx2=FAIL pGroupTbl=%p  hwIdx2=%d",pGroupTbl,pGroupTbl?pGroupTbl->hwIdx2:FAIL);

	if(defOuterTagTCI==FAIL)
	{
		//remove all vlan tag in hash
		action.vlan_cnt1 = 0;
		action.vlan_vld1 = 1;
	}
	else
		WARNING("\n Error Config FixMe\n");

	//assign mce_index		
	action.mc=1;
	action.mcgid = mceIdx;
	action.dpid_vld = 1;
	action.permit = 1;
	action.dpid_pri = 1;	 

	action.pppoe_set1 = 1;
	action.pppoe_vld1 = 0;

	//netif mib
	action.pol_vld1 = 1;
	action.pol_en1 = 1;
	action.pol_id1 = igrIntf + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB; // care ingress interface idx only
#if 0
	action.igr_l3_if_idx1 = igrIntf;// ingress interface idx
	action.egr_l3_if_idx1 = 0;// fix to 0 
	action.l3_if_counter_en = 1; 
	action.l3_if_vld1 = 1;
#endif

	// SW work around for acl trap
	action.ip_ttl_zero_discard_en1 = 1; // always TRUE for ACL trap

	//smac
	action.smac_trans = 1;	 //if we don't want to change smac,set smac_trans==1 !(Wow)

	action.chk_msk_ptr = rtk_asic_db.flow_profile_info[RTK_ASIC_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_FLOW_FORWARD_TUPLE].hash_mask_idx;

	aal_ret = aal_hash_action_update(0, &action, TUPLE_TYPE_TYPE0, rtk_asic_db.flow_profile_info[RTK_ASIC_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_FLOW_FORWARD_TUPLE].hash_mask_idx, RTK_ASIC_FLOW_PROFILE_FLOW_MC, h_idx);
	if (CA_E_OK != aal_ret)
	{
		IGMP("Hash update failed aal_ret=%d\n",aal_ret);
	}


}
#else
	WARNING("\n WARNING: FIXME IF NEW FLATFORM !!!!!!!!!!\n");
#endif
	





	}
	return RTK_FC_RET_OK;

}

int rtk_fc_syncToHw_G3addGroupFwdHash(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,uint8 actDscp_en,uint8 actDscp,uint32 *h_idx,uint32 mceIdx,int32 defOuterTagTCI,uint16 igrIntf)
{
	aal_hash_key_t key = {0};
	aal_hash_action_t action = {0};
	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);
	int32 aal_ret=CA_E_ERROR;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint16 svlan_tpid=0;
	uint8 svlan_tpid_idx=0,cvlan_tpid_idx=0;
#endif
	
	if(pPktHdr->iph==NULL && pPktHdr->ip6h==NULL)
		return CA_E_ERROR;

	if((pFcIngressData->ingressTagif&SVLAN_TAGIF ) && (pFcIngressData->ingressTagif&CVLAN_TAGIF ))
	{
		//Stag+Ctag
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		svlan_tpid = fc_db.systemGlobal.stagTPID[pFcIngressData->srcSVlanTpid_sel];
		if(aal_l3fe_pp_top_tpid_get(svlan_tpid, &svlan_tpid_idx) != AAL_E_OK)
		{
			DEBUG("Unknown TPID 0x%x", svlan_tpid);
			return ASIC_RET_NOT_SUPPORT;
		}
		if(aal_l3fe_pp_top_tpid_get(CVLAN_TPID, &cvlan_tpid_idx) != AAL_E_OK)
		{
			DEBUG("Unknown TPID 0x%x", CVLAN_TPID);
			return ASIC_RET_NOT_SUPPORT;
		}
		key.top_tpid_enc = svlan_tpid_idx;
		key.inner_tpid_enc = cvlan_tpid_idx;
		key.vlan_cnt=2;
#endif
		key.top_vid = pFcIngressData->srcSVlanId;
		key.top_8021p = pFcIngressData->srcSVlanPri;
		key.top_dei = pFcIngressData->srcSVlanDei;

		key.inner_vid = pFcIngressData->srcCVlanId;
		key.inner_8021p = pFcIngressData->srcCVlanPri;
		key.inner_dei = pFcIngressData->srcCVlanCfi;
	}
	else if(pFcIngressData->ingressTagif&SVLAN_TAGIF )
	{
		//only Stag
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		svlan_tpid = fc_db.systemGlobal.stagTPID[pFcIngressData->srcSVlanTpid_sel];
		if(aal_l3fe_pp_top_tpid_get(svlan_tpid, &svlan_tpid_idx) != AAL_E_OK)
		{
			DEBUG("Unknown TPID 0x%x", svlan_tpid);
			return ASIC_RET_NOT_SUPPORT;
		}
		key.top_tpid_enc = svlan_tpid_idx;
		key.vlan_cnt=1;
#endif

		key.top_vid = pFcIngressData->srcSVlanId;
		key.top_8021p = pFcIngressData->srcSVlanPri;
		key.top_dei = pFcIngressData->srcSVlanDei;

	}
	else if(pFcIngressData->ingressTagif&CVLAN_TAGIF )
	{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		//only Ctag
		if(aal_l3fe_pp_top_tpid_get(CVLAN_TPID, &cvlan_tpid_idx) != AAL_E_OK)
		{
			DEBUG("Unknown TPID 0x%x", CVLAN_TPID);
			return ASIC_RET_NOT_SUPPORT;
		}
		key.top_tpid_enc = cvlan_tpid_idx;
		key.vlan_cnt=1;
#endif		
		key.top_vid = pFcIngressData->srcCVlanId;
		key.top_8021p = pFcIngressData->srcCVlanPri;
		key.top_dei = pFcIngressData->srcCVlanCfi;

	}
	else
	{
		//untag
		key.top_vid=0;
		key.inner_vid=0;
	}
	
	if(pFcIngressData->ingressPort == RTK_FC_MAC_PORT_PON)
		key.mdata = pFcIngressData->ponStreamId;

	if(((1 << pFcIngressData->ingressPort) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK) && pPktHdr->igrWlanDevIdx!=RTK_FC_WLANX_NOT_FOUND)
		key.mdata = pPktHdr->igrWlanDevIdx;


	key.ip_vld=1;
	if(pPktHdr->iph)
	{
		key.ip_ver=0;
		key.ip_da_0 = pFcIngressData->dstIp;
		key.ip_da_1 = 0;
		key.ip_da_2 = 0;
		key.ip_da_3 = 0;
		
		key.ip_sa_0 = pFcIngressData->srcIp;
		key.ip_sa_1 = 0;
		key.ip_sa_2 = 0;
		key.ip_sa_3 = 0;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		key.ip_dscp = (pFcIngressData->v4tos>>2)&0x3f;
		key.ip_ecn = (pFcIngressData->v4tos)&0x3;
#endif
	}
	else if(pPktHdr->ip6h)
	{
		key.ip_ver=1;
		key.ip_da_0 = ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]);
		key.ip_da_1 = ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[2]);
		key.ip_da_2 = ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[1]);
		key.ip_da_3 = ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[0]);
		
		key.ip_sa_0 = ntohl(pPktHdr->ip6h->saddr.in6_u.u6_addr32[3]);
		key.ip_sa_1 = ntohl(pPktHdr->ip6h->saddr.in6_u.u6_addr32[2]);
		key.ip_sa_2 = ntohl(pPktHdr->ip6h->saddr.in6_u.u6_addr32[1]);
		key.ip_sa_3 = ntohl(pPktHdr->ip6h->saddr.in6_u.u6_addr32[0]);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		key.ip_dscp = (pFcIngressData->v6tos>>2)&0x3f;
		key.ip_ecn = (pFcIngressData->v6tos)&0x3;
#endif		
	}
		
#if defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_G3LITE)
{
	action.mac_da_vld = 1;
	if (pPktHdr->iph) 
	{
		action.mac_da_5 = 0x01;
		action.mac_da_4 = 0x00;
		action.mac_da_3 = 0x5E;
		action.mac_da_2 = (pFcIngressData->dstIp & 0x7F0000) >> 16;
		action.mac_da_1 = (pFcIngressData->dstIp & 0x00FF00) >> 8;
		action.mac_da_0 = (pFcIngressData->dstIp & 0x0000FF);
	}
	else if(pPktHdr->ip6h)
	{
		action.mac_da_5 = 0x33;
		action.mac_da_4 = 0x33;
		action.mac_da_3 = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0xFF000000) >> 24;
		action.mac_da_2 = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0x00FF0000) >> 16;
		action.mac_da_1 = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0x0000FF00) >> 8;
		action.mac_da_0 = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0x000000FF);
	}

	if(defOuterTagTCI==FAIL)
	{
		//remove all vlan tag in hash
		action.vlan_cnt = 0;
		action.vlan_vld = 1;
	}
	else
	{
		//always give a default top vlan
		action.vlan_cnt = 1;
		action.vlan_vld = 1;
		action.top_tpid_enc = 1;
		action.top_dei_sel = 1;	//using headeri  (keep ori)
		action.top_vid = (defOuterTagTCI&VLAN_VID_MASK);
		action.top_802_1p_sel=2;	//from fib
		action.top_802_1p = (defOuterTagTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
	}
	//assign mce_index		
	action.mc=1;
	action.mcgid = mceIdx;
	action.dpid_vld = 1;
	action.permit = 1;
	action.dpid_pri = 1;	 

	action.pppoe_set = 1;
	action.pppoe_vld = 0;	

	action.chk_msk_ptr = hash_mask_multicast_forward_idx;
	aal_ret = aal_hash_add(0, &key, &action, TUPLE_TYPE_FWD, HASH_AGING_STATIC, hash_mask_multicast_forward_idx, l3fe_main_hash_profile_mc, h_idx);
	if (CA_E_OK != aal_ret)
	{
		IGMP("Hash add failed \n");
	}	
}
#elif defined(CONFIG_FC_CA8277B_SERIES)
{
	//set mac index table at first
	idx_intf_mac_entry_t mac_entry;
	ca_uint8_t mac_da[6];
	int da_rslt_idx = 0;
	int ret;
	memset(&mac_entry, 0, sizeof(mac_entry));
	if(pPktHdr->iph) {
		mac_da[0] = 0x01;
		mac_da[1] = 0x00;
		mac_da[2] = 0x5E;
		mac_da[3] = (pFcIngressData->dstIp & 0x7F0000) >> 16;
		mac_da[4] = (pFcIngressData->dstIp & 0x00FF00) >> 8;
		mac_da[5] = (pFcIngressData->dstIp & 0x0000FF);
	}
	else if(pPktHdr->ip6h)
	{
		mac_da[0] = 0x33;
		mac_da[1] = 0x33;
		mac_da[2] = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0xFF000000) >> 24;
		mac_da[3] = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0x00FF0000) >> 16;
		mac_da[4] = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0x0000FF00) >> 8;
		mac_da[5] = (ntohl(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]) & 0x000000FF);
	}
	array_reverse_copy(mac_entry.mac_da, mac_da, 6);
	/*auto delete in aal hash action delete AAL_TABLE_HASH_HASH_ACTION */
	ret = aal_entry_add(AAL_TABLE_IDX_INTF_MAC, &mac_entry, &da_rslt_idx);
	if(ret != CA_E_OK) {
		return CA_E_ERROR;
	}
	action.mac_da_idx = da_rslt_idx;
	action.mac_da_idx_vld = 1;


	if(defOuterTagTCI==FAIL)
	{
		//remove all vlan tag in hash
		action.vlan_cnt = 0;
		action.vlan_vld = 1;
	}
	else
	{
		//always give a default top vlan
		action.vlan_cnt = 1;
		action.vlan_vld = 1;
		action.top_tpid_enc = 1;
		action.top_dei_sel = 1;	//using headeri  (keep ori)
		action.top_vid = (defOuterTagTCI&VLAN_VID_MASK);
		action.top_802_1p_sel=2;	//from fib
		action.top_802_1p = (defOuterTagTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
	}
	//assign mce_index		
	action.mc=1;
	action.mcgid = mceIdx;
	action.dpid_vld = 1;
	action.permit = 1;
	action.dpid_pri = 1;	 

	action.pppoe_set = 1;
	action.pppoe_vld = 0;

	//interface mib
	action.mdata_w_vld_1 = 0xF;
	action.mdata_w_1 = (igrIntf+RTK_FC_MC_HW_NETIF_IDXSHIFT)<<8; // egress interface fix to 0

	action.ip_dscp_update_en = actDscp_en;
	action.ip_dscp = actDscp ;

	action.chk_msk_ptr = hash_mask_multicast_forward_idx;

	aal_ret = aal_hash_add(0, &key, &action, TUPLE_TYPE_FWD, HASH_AGING_STATIC, hash_mask_multicast_forward_idx, l3fe_main_hash_profile_mc, h_idx);
	if (CA_E_OK != aal_ret)
	{
		IGMP("Hash add failed \n");
	}

}
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
{
	uint32 gip[4];
	uint32 isipv6=0;
	rtk_fc_table_mcGroupTbl_t* pGroupTbl=NULL;
	if(pPktHdr->iph)
	{
		gip[0]=pFcIngressData->dstIp;
	}
	else
	{
		isipv6=1;
		gip[0]=(pPktHdr->ip6h->daddr.in6_u.u6_addr32[0]);
		gip[1]=(pPktHdr->ip6h->daddr.in6_u.u6_addr32[1]);
		gip[2]=(pPktHdr->ip6h->daddr.in6_u.u6_addr32[2]);
		gip[3]=(pPktHdr->ip6h->daddr.in6_u.u6_addr32[3]);
	}
	pGroupTbl = rtk_fc_mc_findKnownGroupEntry(isipv6,gip);

	if(pGroupTbl && pGroupTbl->hwIdx2!=FAIL)
	{
		action.mac_da_idx_vld=1;
		action.mac_da_idx = pGroupTbl->hwIdx2;
	}
	else
		WARNING("pGroupTbl not found or hwIdx2=FAIL pGroupTbl=%p  hwIdx2=%d",pGroupTbl,pGroupTbl?pGroupTbl->hwIdx2:FAIL);

	if(defOuterTagTCI==FAIL)
	{
		//remove all vlan tag in hash
		action.vlan_cnt1 = 0;
		action.vlan_vld1 = 1;
	}
	else
		WARNING("\n Error Config FixMe\n");

	//assign mce_index		
	action.mc=1;
	action.mcgid = mceIdx;
	action.dpid_vld = 1;
	action.permit = 1;
	action.dpid_pri = 1;	 

	action.pppoe_set1 = 1;
	action.pppoe_vld1 = 0;

	//netif mib
	action.pol_vld1 = 1;
	action.pol_en1 = 1;
	action.pol_id1 = igrIntf + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB; // care ingress interface idx only

	action.ip_dscp_update_en1 = actDscp_en;
	action.ip_dscp1 = actDscp ;

	//flowmib
	if(!fc_db.controlFuc.flow_meter_mib_conf_dependence)	
	{
		action.pol3_id_enable1 = pPktHdr->remarkDec.mibIdx_en;
		action.pol3_id1 = pPktHdr->remarkDec.mibIdx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;	
	}

	// SW work around for acl trap
	action.ip_ttl_zero_discard_en1 = 1; // always TRUE for ACL trap

	//smac
	action.smac_trans = 1;	 //if we don't want to change smac,set smac_trans==1 !(Wow)


	action.chk_msk_ptr1 = rtk_asic_db.flow_profile_info[RTK_ASIC_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_FLOW_FORWARD_TUPLE].hash_mask_idx;
	aal_ret = aal_hash_add(0, &key, &action, TUPLE_TYPE_TYPE0, HASH_AGING_START, rtk_asic_db.flow_profile_info[RTK_ASIC_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_FLOW_FORWARD_TUPLE].hash_mask_idx, RTK_ASIC_FLOW_PROFILE_FLOW_MC, h_idx);
	if (CA_E_OK != aal_ret)
	{
		IGMP("Hash add failed aal_ret=%d\n",aal_ret);
	}

}
#else
	WARNING("\n WARNING: FIXME IF NEW FLATFORM !!!!!!!!!!\n");
#endif



	return RTK_FC_RET_OK;
}
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_abstrXPONHw_del(uint32 hwIdx)
{
	uint32 indMacIdx;
	rtk_fc_tableFlow_t *pFlowTable;
	
	if(hwIdx >= fc_db.flowHwTableSize){WARNING("hwIdx >=fc_db.flowHwTableSize"); return RTK_FC_RET_ERR;}

	pFlowTable = &fc_db.flowTbl[hwIdx];

	TABLE("delete hwflowIdx[%d]",hwIdx);

	//decrease extidx reference count
	if( ((fc_db.flowTbl[hwIdx].pFlowEntry->path1.in_path == FB_PATH_12) || (fc_db.flowTbl[hwIdx].pFlowEntry->path1.in_path == FB_PATH_34))
		&& (fc_db.flowTbl[hwIdx].pFlowEntry->path1.out_ext_portmask_idx!=0U) )
	{
		ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(fc_db.flowTbl[hwIdx].pFlowEntry->path1.out_ext_portmask_idx),RTK_FC_RET_OK);
	}

	// decrease indmac reference count
	if((((fc_db.flowTbl[hwIdx].pFlowEntry->path1.in_path == FB_PATH_12) || (fc_db.flowTbl[hwIdx].pFlowEntry->path1.in_path == FB_PATH_34))
			&& (fc_db.flowTbl[hwIdx].pFlowEntry->path1.in_multiple_act != 1)
			&& (fc_db.flowTbl[hwIdx].pFlowEntry->path1.out_dmac_trans || fc_db.flowTbl[hwIdx].pFlowEntry->path1.out_uc_lut_lookup)) 	// path 1, 3
		|| (fc_db.flowTbl[hwIdx].pFlowEntry->path1.in_path == FB_PATH_5)														// path 5
		)
	{
		indMacIdx = fc_db.flowTbl[hwIdx].pFlowEntry->path1.out_dmac_idx;
		if(fc_db.indMacTbl[indMacIdx].valid)
			fc_db.indMacTbl[indMacIdx].indMacRefCount--;
		else{
			FIXME("indMacIdx was wrong?");
		}
	}


	if(hwIdx < fc_db.flowHwTableSize)
	{
		fc_db.fb_hwFlow_validBitsArray[(hwIdx >> 5)] &= ~(0x1 << (hwIdx&0x1f));
		// flow Tcam list
		if(rgpro_db.fbMode==FB_MODE_4K && RTK_FC_TABLESIZE_FLOWSRAM<=hwIdx)
		{
			assert_ok(_rtk_fc_flowTcamListDel(hwIdx));
		}
	}

	ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_DEL(hwIdx), RTK_FC_RET_OK);

	// initialize sw field of hwflow
	pFlowTable->cachedCt = NULL;
	//--------------- RESET: init value which is 0	   -------------------------------------------------------------------------------//
	memset((uint8 *)pFlowTable + offsetof(rtk_fc_tableFlow_t, cachedCt) , 0 , (sizeof(rtk_fc_tableFlow_t)- offsetof(rtk_fc_tableFlow_t, cachedCt) ) );
	//------------------------------------------------------------------------------------------------------------------------------//
	
	//----------------RESET: init value which is not 0 -------------------------------------------------------------------------------//
	pFlowTable->loopbackRevFlowIdx = SIGNED_INVALID;		// shared info
	pFlowTable->lutIgrSaIdx = SIGNED_INVALID;
	pFlowTable->lutEgrDaIdx = SIGNED_INVALID;
	
	pFlowTable->pAbstrSwFlowEt=NULL; //we free pSwFlowEt in software reference not hardward referece
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
	if(pFlowTable->dummyPkt) bzero(pFlowTable->dummyPkt, sizeof(rtk_fc_igrDummyData_t));
#endif
	//------------------------------------------------------------------------------------------------------------------------------//
	return SUCCESS;

}
#endif


int rtk_fc_abstrSwSyncToHw(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int32 swFlowIdx)
{
	int32 hwacc=0;

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	hwacc = fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->g3KeyHwPriv.hwacc;
#else
	hwacc = fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->xponKeyHwPriv.hwacc;
#endif

	if( hwacc && pPktHdr->abstrSwFlowFlooding)
	{
		//TRACE("rtk_fc_abstrSwSyncToHw_update");
		rtk_fc_abstrSwSyncToHw_update(swFlowIdx);
	}
	else
	{
		//TRACE("rtk_fc_abstrSwSyncToHw_add");
		rtk_fc_abstrSwSyncToHw_add(rtskb, pPktHdr,swFlowIdx);
	}

	return RTK_FC_RET_OK;
}


#if defined(CONFIG_RTK_L34_G3_PLATFORM)

#define TAG_ALL_UNTAG 		0
#define TAG_ALL_SAME_TAG 	1 
#define TAG_MULTI_DIFF_TAG	2

typedef enum rtk_fc_mctop_vlanMode_e
{
	MCFIB_ONLY_SVALN_MODIFY= 0,
	MCFIB_ONLY_CVALN_MODIFY= 1,
	MCFIB_BOTH_SCVLAN_MODIFY= 2,
	MCFIB_ERROR= 3,

}rtk_fc_mctop_vlanMode_t;

/*
	CTAG				TAG_ALL_UNTAG				TAG_ALL_SAME_TAG				TAG_MULTI_DIFF_TAG
STAG

TAG_ALL_UNTAG			mcfib.nop					if(egrC)mcfib.push-Ctag			if(egrC)mcfib.push-Ctag
						hash untag					else(egrunC) mcfib.nop			else(egrunC) mcfib.nop
													hash untag						hash untag

TAG_ALL_SAME_TAG		mcfib.push-Stag				mcfib.push-Stag & 				not support
						hash untag					hash default a C
													

TAG_MULTI_DIFF_TAG		if(egrS)mcfib.push-Stag		if(egrS)mcfib.push-Stag			not support
(include 1tag 1untag)	else(egrunS) mcfib.nop		&& hash default a C
						hash untag					else(egrunS) mcfib.nop
													

STAG=TAG_ALL_SAME_TAG / CTAG=TAG_ALL_SAME_TAG  <==> STAG=TAG_ALL_UNTAG / CTAG=TAG_ALL_SAME_TAG
need readd all port by new mode and call aal_hw_mcgid_free old member
													
*/
int rtk_fc_abstrSwSyncToHw_mcDecision(int32 swFlowIdx,int32 *defaultCtafTCI,rtk_fc_mctop_vlanMode_t *vlanMode)
{
#if defined(CONFIG_FC_G3_G3LITE_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)
	rtk_fc_abstrSwFlowActionField_entry_t *pActField,actField;
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	
	int32 stagAction=TAG_ALL_UNTAG,svlanTCI=FAIL,untagSvlanAddOne=FAIL;
	int32 ctagAction=TAG_ALL_UNTAG,cvlanTCI=FAIL,untagCvlanAddOne=FAIL;

	//this conuter is not real action counter we just need to identity action >=2
	int32 stagActionCnt=0,ctagActionCnt=0;

	pActField=&actField;
	*defaultCtafTCI=FAIL;

	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		
		rtk_fc_parseAbstrSwActionField(pAbstrSwFlowEt->swFlowKey.bits.isIpv6,&pSwFlowAction->swFlowAction,pActField);

		if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
		{
			if(svlanTCI==FAIL)
			{
				svlanTCI=*pActField->pSvlanTCI;
				stagActionCnt++;
			}
			
			if(svlanTCI != *pActField->pSvlanTCI)
				stagActionCnt++;
		}
		else
		{
			if(untagSvlanAddOne==FAIL)
			{
				stagActionCnt++;
				untagSvlanAddOne=1;
			}
		}
		
		if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
		{
			if(cvlanTCI==FAIL)
			{
				cvlanTCI=*pActField->pCvlanTCI;
				ctagActionCnt++;
			}

			if(cvlanTCI != *pActField->pCvlanTCI)
				ctagActionCnt++;
		}
		else
		{
			if(untagCvlanAddOne==FAIL)
			{
				ctagActionCnt++;
				untagCvlanAddOne=1;
			}
		}
	}

	if(untagSvlanAddOne==1 && stagActionCnt==1)
		stagAction=TAG_ALL_UNTAG;
	else if( stagActionCnt==1)
		stagAction=TAG_ALL_SAME_TAG;
	else
		stagAction=TAG_MULTI_DIFF_TAG;

	if(untagCvlanAddOne==1 && ctagActionCnt==1)
		ctagAction=TAG_ALL_UNTAG;
	else if( ctagActionCnt==1)
		ctagAction=TAG_ALL_SAME_TAG;
	else
		ctagAction=TAG_MULTI_DIFF_TAG;

//not support check
#if defined(CONFIG_FC_G3_G3LITE_SERIES) 
	if(ctagAction >=TAG_ALL_SAME_TAG && stagAction>=TAG_ALL_SAME_TAG)
		return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
#elif defined(CONFIG_FC_CA8277B_SERIES)
	if(ctagAction >=TAG_MULTI_DIFF_TAG && stagAction>=TAG_ALL_SAME_TAG)
		return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
#endif

	if(stagAction==TAG_ALL_UNTAG)
	{
		*vlanMode=MCFIB_ONLY_CVALN_MODIFY;
		*defaultCtafTCI=FAIL;
	}
	else
	{
		//outer stag modify and inner single ctag by hash
		*vlanMode=MCFIB_ONLY_SVALN_MODIFY;
		*defaultCtafTCI=cvlanTCI;
	}
#elif  defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	*vlanMode=MCFIB_BOTH_SCVLAN_MODIFY;
	*defaultCtafTCI=FAIL;
#endif

	
	return RTK_FC_RET_OK;

}



int rtk_fc_delete_mc_macId(int swMacid)
{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)	

	int _swMacid;

	if( (RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START <= swMacid) && (swMacid<=RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END))
		return SUCCESS;

	_swMacid = swMacid - (RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END+1);

	if(_swMacid < 0 ||  (WLAN_MC_TO_UC_MAC_POOL_MAX) <= _swMacid)
		return FAILED;
	if(fc_db.mcToUcMacId[_swMacid].valid==0)
		return FAILED;

	fc_db.mcToUcMacId[_swMacid].refCnt--;
	
	if(fc_db.mcToUcMacId[_swMacid].refCnt ==0 )
	{
		if(fc_db.mcToUcMacId[_swMacid].lutIdx!=FAIL && fc_db.lutTbl[fc_db.mcToUcMacId[_swMacid].lutIdx])
		{
			if(fc_db.lutTbl[fc_db.mcToUcMacId[_swMacid].lutIdx]->wifiMacId == swMacid)
				fc_db.lutTbl[fc_db.mcToUcMacId[_swMacid].lutIdx]->wifiMacId=0;
		}

		fc_db.mcToUcMacId[_swMacid].lutIdx=FAIL;
		fc_db.mcToUcMacId[_swMacid].valid=0;
	}
	return SUCCESS;
#endif
#endif
	return FAILED;
}



int rtk_fc_get_free_macId(int *swMacid,int lutIdx)
{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)	

	int i;
	int fistInvalid=SIGNED_INVALID;

	for(i=0;i<WLAN_MC_TO_UC_MAC_POOL_MAX;i++)
	{
		if(fc_db.mcToUcMacId[i].valid==0 && fistInvalid==SIGNED_INVALID)
		{
			fistInvalid = i;
			break;
		}
	}
	if(fistInvalid==SIGNED_INVALID)
		return FAILED;

	fc_db.mcToUcMacId[fistInvalid].valid=1;
	fc_db.mcToUcMacId[fistInvalid].refCnt=1;
	fc_db.mcToUcMacId[fistInvalid].lutIdx=lutIdx;
	*swMacid = fistInvalid+(RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END+1);
	return SUCCESS;

#endif
#endif
	return FAILED;
}

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
/* refill ldpid and wifi_flowid*/
int rtk_fc_abstrSwSyncToHw_WifiMcToUc(rtk_fc_mcExtraSwFlowIdx_entry_t* p_mcSwFlowIdx,uint8* dmac,portForwardActionVlanExtend_t *pfwdAction,uint32 *amsdu_h_idx,uint32 *swMacId,uint32 updateFlow)
{
	int ret;
	int mac_id=0;
	int16 lutIdx;
	bool toPE_AMSDU=false;
	uint32 flow_idx;
	rtk_fc_pmap_t portmap;
	rtk_fc_tableFlowEntry_t flowPathEntry;
	rtk_rg_asic_pathMc_wifi_amsdu_tx_entry_t *flowPath_mc_wifi_amsdu_tx = &flowPathEntry.pathMc_wifi_amsdu_tx;
	rtk_rg_asic_flow_hash_crc_t flow_hash_crc;
	rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
	rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;
	rtk_fc_table_mcGroupTbl_t* pGroupTbl=NULL;
	uint8 wifi_pri = 0;
	int16 hashKeyMacId = 0;
	rtk_fc_wifi_tx_amsdu_queueid_t amsdu_queue = RTK_FC_WIFI_TX_AMSDU_DISABLE;

	*swMacId = G3_FLOWIDX_INVALID;

	if(pfwdAction->cos_en)
	{
		wifi_pri = fc_db.controlFuc.wifi_tx_qos_mapping[pfwdAction->cos%8];
		amsdu_queue = fc_db.controlFuc.wifi_tx_asmdu_queue_mapping[pfwdAction->cos%8];
	}

	if(_rtk_fc_lut_find(dmac,&lutIdx)==RTK_FC_RET_OK)
	{
		mac_id = fc_db.lutTbl[lutIdx]->wifiMacId ;

		if(mac_id<RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START)
		{
			//mac_id==0 invalid
			toPE_AMSDU=FALSE;
			//sw_macid update lut macid to swmacid
			ret = rtk_fc_get_free_macId(&mac_id,lutIdx);
			if(ret)
				{IGMP("no free macId"); return FAILED;}
			fc_db.lutTbl[lutIdx]->wifiMacId = mac_id;
		}
		else if(( RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START <= mac_id) && (mac_id <= fc_db.wifi_amsdu_pe_offload_sta_count))
		{
			if(fc_db.pe_offload_wifi_amsdu_en && amsdu_queue!=RTK_FC_WIFI_TX_AMSDU_DISABLE)
			{
				//default enable AMSDU
				toPE_AMSDU=TRUE;

				//amsdu macid just using lut macid
				if(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].valid==0)	
					{IGMP("macId invalid"); return FAILED;}
				/* mac_id base power saving */
				if(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info.power_saving)
					toPE_AMSDU=FALSE;
			}
			else
				toPE_AMSDU=FALSE;	
		}
		else
		{
			//softward MacId
			fc_db.mcToUcMacId[mac_id-(RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END+1)].refCnt++;;
			IGMP("softward macid :%d add mcToUcMacId[%d] refCnt:%d",mac_id,mac_id-(RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END+1),fc_db.mcToUcMacId[mac_id-(RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END+1)].refCnt);
			toPE_AMSDU=FALSE;
		}
	}
	else
	{
		IGMP("mc to uc Mac not found");
		return FAILED;
	}



	pGroupTbl = rtk_fc_mc_findKnownGroupEntry(p_mcSwFlowIdx->isIpv6,p_mcSwFlowIdx->multicastAddress);

	if(pGroupTbl==NULL ||  pGroupTbl->hwIdx2==FAIL)
	{
		WARNING("pGroupTbl not found or hwIdx2=FAIL pGroupTbl=%p  hwIdx2=%d",pGroupTbl,pGroupTbl?pGroupTbl->hwIdx2:FAIL);
		return FAILED;
	}

	hashKeyMacId = (mac_id);

	bzero(flowPath_mc_wifi_amsdu_tx, sizeof(rtk_rg_asic_pathMc_wifi_amsdu_tx_entry_t));
	flowPath_mc_wifi_amsdu_tx->valid = TRUE;
	flowPath_mc_wifi_amsdu_tx->in_path = FB_PATH_MC_WIFI_AMSDU_TX;
	flowPath_mc_wifi_amsdu_tx->in_wifi_mac_id =hashKeyMacId;
	flowPath_mc_wifi_amsdu_tx->in_dmac_lut_idx = pGroupTbl->hwIdx2;
	flowPath_mc_wifi_amsdu_tx->out_dmac_lut_act = TRUE;
	flowPath_mc_wifi_amsdu_tx->out_dmac_lut_idx = lutIdx;
	flowPath_mc_wifi_amsdu_tx->lock = TRUE;


	if(toPE_AMSDU)
	{
		uint8 ldpid,cos;
		if(_rtk_fc_wifi_amsdu_pe_offload_voq_info_get(mac_id,&ldpid,&cos)==RTK_FC_RET_OK)
		{
			flowPath_mc_wifi_amsdu_tx->out_cos_act = TRUE;
			flowPath_mc_wifi_amsdu_tx->out_cos = cos;
			flowPath_mc_wifi_amsdu_tx->out_ldpid_act = TRUE;
			flowPath_mc_wifi_amsdu_tx->out_ldpid = ldpid;

			if(fc_db.wifi_amsdu_pe_offload_mode == RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64)
			{
				flowPath_mc_wifi_amsdu_tx->out_cos+=amsdu_queue;
			}

#if defined(CONFIG_FC_RTL9607F_SERIES)
			/* trans ethIIto SNAP format*/
			flowPath_mc_wifi_amsdu_tx->out_l2format_act_vld = TRUE;
			flowPath_mc_wifi_amsdu_tx->out_l2format_act = RTK_ASIC_L2_FORMAT_ACT_SNAP;
#endif
			flowPath_mc_wifi_amsdu_tx->out_wifi_devId_macId_act = TRUE;
			flowPath_mc_wifi_amsdu_tx->out_wifi_devId_macId = wifi_pri ;
		}
		else
		{
			TRACE("Get _rtk_fc_wifi_amsdu_pe_offload_voq_info_get failed");
			return RT_ERR_RG_FAILED;
		}
	}
	else
	{
		// to arm take orinal cos and wifi_flow_id
		flowPath_mc_wifi_amsdu_tx->out_cos_act = TRUE;
		flowPath_mc_wifi_amsdu_tx->out_cos =  pfwdAction->cos;
	
		flowPath_mc_wifi_amsdu_tx->out_ldpid_act = TRUE;
		flowPath_mc_wifi_amsdu_tx->out_ldpid = pfwdAction->ldpid ;

		flowPath_mc_wifi_amsdu_tx->out_wifi_devId_macId_act = TRUE;
		flowPath_mc_wifi_amsdu_tx->out_wifi_devId_macId = pfwdAction->wifi_flowid ;
	}



	if(_rtk_fc_flow_hashIndex(flowPathEntry, NULL, &flow_hash_crc) != RTK_FC_RET_OK)
	{
			WARNING("Get flow CRC by flowPathEntry failed");
					return RT_ERR_RG_FAILED;
	}

	//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo
	_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
	pG3IgrExtraInfo = &g3IgrExtraInfo;
	pG3IgrExtraInfo->crc16 = flow_hash_crc.crc16;
	pG3IgrExtraInfo->crc32 = flow_hash_crc.crc32;

	portmap.macPortIdx = RTK_FC_MAC_PORT_MC_PORT;


	//we need delete flow for add (for th same flow_idx case)
	if(updateFlow)
		rtk_fc_flow_delete(*amsdu_h_idx);

	ret = rtk_fc_flow_add(&flow_idx, pG3IgrExtraInfo->crc16, flowPath_mc_wifi_amsdu_tx, NULL,
													NULL, FALSE, portmap, FALSE, TRUE,
													pG3IgrExtraInfo, NULL);

	//MC_FIB lookback to L3FE
	pfwdAction->ldpid = RTK_FC_MAC_PORT_L3_LAN;
	pfwdAction->wifi_flowid = hashKeyMacId;
	*amsdu_h_idx = flow_idx;
	*swMacId = mac_id;

	if(toPE_AMSDU)
		TABLE("Multicast AMSDU_TO_PE Add Hash:[%d] l3feOffload Mac:%pM mac_id:%d",flow_idx,fc_db.lutTbl[lutIdx]->l2Addr,mac_id);
	else
		TABLE("Multicast WIFIFF_TO_CPU Add Hash:[%d] l3feOffload Mac:%pM mac_id:%d",flow_idx,fc_db.lutTbl[lutIdx]->l2Addr,mac_id);
	
	return SUCCESS;
}
#endif

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

int rtk_fc_abstrG3MctoUc_handle(rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid,uint8	*pDmac,portForwardActionVlanExtend_t *pActField,rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowIdx,rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction)
{
	int ret=RTK_FC_RET_OK;

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)	
	if(pSwFlowAction->swFlowAction.bits.dmacTrans)
	{
		ca_uint32_t h_idx_amsdu=G3_FLOWIDX_INVALID;
		ca_uint32_t swMacId=G3_FLOWIDX_INVALID;
		if(pLdpid->g3PortPriv.hwFibHashAcc)
		{
			h_idx_amsdu = pLdpid->g3PortPriv.hwFibHashIdx;
			ret=rtk_fc_abstrSwSyncToHw_WifiMcToUc(mcSwFlowIdx,pDmac,pActField,&h_idx_amsdu,&swMacId,TRUE);
			if(ret)
				{IGMP("DmacTransNotSupport"); return ret;}						
		}
		else
		{
			ret=rtk_fc_abstrSwSyncToHw_WifiMcToUc(mcSwFlowIdx,pDmac,pActField,&h_idx_amsdu,&swMacId,FALSE);
			if(ret)
				{IGMP("DmacTransNotSupport"); return ret;}
		}
		pLdpid->g3PortPriv.hwFibHashAcc_tmp=1;
		pLdpid->g3PortPriv.hwFibHashIdx_tmp = h_idx_amsdu;
		if(swMacId!=G3_FLOWIDX_INVALID)
		{
			pLdpid->g3PortPriv.swMacIdEn_tmp=1;
			pLdpid->g3PortPriv.swMacId_tmp=swMacId;
		}					
	}
#endif
	return ret;

}
#endif

/*

if fib change update mutlciast mcgid and mc_fib 

*/
int rtk_fc_abstrSwSyncToHw_g3McReFlush(int32 swFlowIdx)
{
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowActionField_entry_t *pActField,actField;
	rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowIdx=NULL;
	ca_uint32_t h_idx;
	ca_uint16_t ori_swMcgid=CA_UINT16_INVALID,swMcgid=CA_UINT16_INVALID;
	ca_uint16_t newHwMcgid=CA_UINT16_INVALID,oriHwMcgid=CA_UINT16_INVALID;
	ca_uint16_t fistActIgrIntf=CA_UINT16_INVALID;
	int32 mainHashDefaultTAGTCI=FAIL;
	rtk_fc_mctop_vlanMode_t fibVlanMode=MCFIB_ERROR;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	portForwardActionVlanExtend_t fwdAction;
#else
	portForwardAction_t fwdAction;
#endif

	int ret;

	pActField=&actField;

	if(FLOW_INFO_IS_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
	if(!pAbstrSwFlowEt->g3KeyHwPriv.hwacc)
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;

	mcSwFlowIdx = rtk_fc_igmp_mcExtraFlowIdxTbl_find(swFlowIdx);
	if(mcSwFlowIdx==NULL)
		{IGMP("can't get mcflowIdxTbl"); goto G3_REUPDATE_FAIL_SWONLY;}


	h_idx=pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx;
	ori_swMcgid=pAbstrSwFlowEt->g3KeyHwPriv.swMcgid;
	oriHwMcgid=pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid;

	//=============add mce ===============
	//setup1. get a sw_mcgid 
	//we using new swMcgid every times
	if(aal_sw_mcgid_alloc(&swMcgid)!=AAL_E_OK)
	{
		IGMP("aal_sw_mcgid_alloc Fail");
		goto G3_REUPDATE_FAIL_SWONLY;
	}


	rtk_fc_abstrSwSyncToHw_mcDecision(swFlowIdx,&mainHashDefaultTAGTCI,&fibVlanMode);

	//setup2.using sw_mcgid to add port action  
	bzero(&fwdAction,sizeof(fwdAction));

	if(pAbstrSwFlowEt->copy2cpu)
	{
		rtk_fc_abstrSwFlowPattenField_entry_t field;
		rtk_fc_parseAbstrSwPattenField(pAbstrSwFlowEt->swFlowKey.bits.isIpv6,&pAbstrSwFlowEt->swFlowKey,&field);

		IGMP("SET A COPY2CPU MCFIB");
		//copy packet is original
		fwdAction.ldpid = RTK_FC_MAC_PORT_WLAN_CPU0;
		fwdAction.wifi_flowid = RTK_FC_COPY2CPU_INTF;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

		if(pAbstrSwFlowEt->swFlowKey.bits.stagif)
		{
			fwdAction.svlanId_cmd	 = ACTION_VLANTAG_BYTBL;
			fwdAction.svlanPri_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.svlanDei_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.svlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.svlan_tpid = (*field.pSvlanTPID);
			fwdAction.svlanId = ((*field.pSvlanTCI)&VLAN_VID_MASK);
			fwdAction.svlanPri = ((*field.pSvlanTCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			fwdAction.svlanDei =   ((*field.pSvlanTCI)&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
		}
		if(pAbstrSwFlowEt->swFlowKey.bits.ctagif)
		{
			fwdAction.cvlanId_cmd	 = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlanPri_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlanDei_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlan_tpid =  (*field.pCvlanTPID);
			fwdAction.cvlanId = ((*field.pCvlanTCI)&VLAN_VID_MASK);
			fwdAction.cvlanPri = ((*field.pCvlanTCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			fwdAction.cvlanDei = (  (*field.pCvlanTCI)&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
		}
#endif
		fwdAction.src_port_filter_dis=1;
		aal_port_forward_action_add(swMcgid,&fwdAction);
	}	
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		rtk_fc_parseAbstrSwActionField((pAbstrSwFlowEt->swFlowKey.bits.isIpv6),&pSwFlowAction->swFlowAction,pActField);

		bzero(&fwdAction,sizeof(fwdAction));


		if(fistActIgrIntf==CA_UINT16_INVALID)
			fistActIgrIntf=pSwFlowAction->igrIntf;

		if(pSwFlowAction->swFlowAction.bits.smacTrans)
		{
			fwdAction.mac_sa_cmd=1;
			memcpy(fwdAction.mac_sa,pActField->pSmac,6);
		}
		if(pSwFlowAction->swFlowAction.bits.userPriCmd)
		{
			fwdAction.cos_en=1;
			fwdAction.cos = (*pActField->pUserPri)&0x7;
		}

#if defined(CONFIG_FC_G3_G3LITE_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)

		if(fibVlanMode == MCFIB_ONLY_CVALN_MODIFY)
		{
			if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
			{
				//default is untag after main hash we need push a cvlan
				fwdAction.vlan_cmd = ACTION_VLAN_PUSH;
				fwdAction.vpri_cmd = ACTION_VPRI_FROM_TABLE;
				fwdAction.dei_cmd = 1;
				fwdAction.vlan = (*pActField->pCvlanTCI&VLAN_VID_MASK);
				fwdAction.vpri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.dei =  (*pActField->pCvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
				fwdAction.topVlanType_svlan=0;
			}
			else
			{
				//default is untag after main hash 
				fwdAction.vlan_cmd = ACTION_VLAN_NOP;
			}

		}
		else if(fibVlanMode == MCFIB_ONLY_SVALN_MODIFY)
		{
			if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
			{
				//default is unstag after main hash we need push a cvlan
				fwdAction.vlan_cmd = ACTION_VLAN_PUSH;
				fwdAction.vpri_cmd = ACTION_VPRI_FROM_TABLE;
				fwdAction.dei_cmd = 1;
				fwdAction.vlan = (*pActField->pSvlanTCI&VLAN_VID_MASK);
				fwdAction.vpri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.dei =  (*pActField->pSvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
				fwdAction.topVlanType_svlan=1;
			}
			else
			{
				//default is unstag after main hash 
				fwdAction.vlan_cmd = ACTION_VLAN_NOP;
			}

		}
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(fibVlanMode == MCFIB_BOTH_SCVLAN_MODIFY)
		{
			if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
			{
				fwdAction.svlanId_cmd	 = ACTION_VLANTAG_BYTBL;
				fwdAction.svlanPri_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlanDei_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlan_tpid = (*pActField->pSvlanTpid);
				fwdAction.svlanId = (*pActField->pSvlanTCI&VLAN_VID_MASK);
				fwdAction.svlanPri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.svlanDei =   (*pActField->pSvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
			}
			else
			{
				//default is untag after main hash 
				fwdAction.svlanId_cmd = ACTION_VLAN_UNTAG;
			}
			
			if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
			{
				fwdAction.cvlanId_cmd    = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlanPri_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlanDei_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlan_tpid = (*pActField->pCvlanTpid);
				fwdAction.cvlanId = (*pActField->pCvlanTCI&VLAN_VID_MASK);
				fwdAction.cvlanPri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.cvlanDei =   (*pActField->pCvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
			}
			else
			{
				//default is untag after main hash 
				fwdAction.cvlanId_cmd = ACTION_VLAN_UNTAG;
			}

		}
#endif
		
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{

			if(pLdpid->isWlan)
			{
				rtk_fc_mac_port_idx_t macPort;
				rtk_fc_mac_ext_port_idx_t macExtPort;
				RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pLdpid->flowLdpid, &macPort, &macExtPort);
				fwdAction.ldpid = macPort;
				fwdAction.wifi_flowid = pLdpid->flowLdpid;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)					
				if(rtk_fc_abstrG3MctoUc_handle(pLdpid,pActField->pDmac,&fwdAction,mcSwFlowIdx,pSwFlowAction)!=RTK_FC_RET_OK)
					goto G3_REUPDATE_FAIL_SWONLY;
#endif
			}
			else
			{
				fwdAction.ldpid = pLdpid->flowLdpid;
				fwdAction.wifi_flowid = 0;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)					
				if(rtk_fc_abstrG3MctoUc_handle(pLdpid,pActField->pDmac,&fwdAction,mcSwFlowIdx,pSwFlowAction)!=RTK_FC_RET_OK)
					goto G3_REUPDATE_FAIL_SWONLY;
#endif				
			}
			fwdAction.src_port_filter_dis=1;
			aal_port_forward_action_add(swMcgid,&fwdAction);
			pLdpid->g3PortPriv.hwFibAcc=1;
		}
	}

	//setup3.apply to get hw_mcgid
	ret=aal_hw_mcgid_new(swMcgid,&newHwMcgid);
	if(ret)
	{
		IGMP("aal_hw_mcgid_new Error ret:%d",ret);
		goto G3_REUPDATE_FAIL_SWONLY;
	}
	//setup4.update hw_mcgid to your fucntion configuration and keep original ori_hw_mcgid
	// =============add main hash ===============
	ret=rtk_fc_syncToHw_G3updateGroupHash(swFlowIdx,h_idx,newHwMcgid,mainHashDefaultTAGTCI,fistActIgrIntf);
	if(ret)
	{
		IGMP("aal_hw_mcgid_new Error ret:%d",ret);
		goto G3_REUPDATE_FAIL_SWONLY;
	}

	IGMP("UPDATE G3Mc hashIdx=%d ori_swMcgid=%d ->new_swMcgid=%d oriHwMcgid=%d -> newHwMcgid=%d fibVlanMode=%d mainHashDefaultTAGTCI=%x"
	,h_idx,ori_swMcgid,swMcgid,oriHwMcgid,newHwMcgid,fibVlanMode,mainHashDefaultTAGTCI);
	//setup5.free ori_hw_mcgid to de-reference
	ASSERT_EQ(aal_hw_mcgid_free(ori_swMcgid,oriHwMcgid),CA_E_OK);
	//free ori_swMcgid to delete
	ASSERT_EQ(aal_sw_mcgid_free(ori_swMcgid),CA_E_OK);

	//free old ldpid hash info and restore tmp to entry
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{
			if(pLdpid->g3PortPriv.hwFibHashAcc && pLdpid->g3PortPriv.hwFibHashAcc_tmp && pLdpid->g3PortPriv.hwFibHashIdx==pLdpid->g3PortPriv.hwFibHashIdx_tmp)
			{
				pLdpid->g3PortPriv.hwFibHashAcc=0;
			}
			else if(pLdpid->g3PortPriv.hwFibHashAcc)
			{
				rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx);
				pLdpid->g3PortPriv.hwFibHashAcc=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn)
			{
				rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacId);
				pLdpid->g3PortPriv.swMacIdEn=0;
			}
			if(pLdpid->g3PortPriv.hwFibHashAcc_tmp)
			{
				pLdpid->g3PortPriv.hwFibHashAcc = pLdpid->g3PortPriv.hwFibHashAcc_tmp;
				pLdpid->g3PortPriv.hwFibHashIdx = pLdpid->g3PortPriv.hwFibHashIdx_tmp;
				pLdpid->g3PortPriv.hwFibHashAcc_tmp=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn_tmp)
			{
				pLdpid->g3PortPriv.swMacIdEn = pLdpid->g3PortPriv.swMacIdEn_tmp;
				pLdpid->g3PortPriv.swMacId =  pLdpid->g3PortPriv.swMacId_tmp;
				pLdpid->g3PortPriv.swMacIdEn_tmp=0;
			}
		}
	}


	//update swMcgid & newHwMcgid
	pAbstrSwFlowEt->g3KeyHwPriv.swMcgid = swMcgid;
	pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid = newHwMcgid;
	pAbstrSwFlowEt->g3KeyHwPriv.hwacc=1;

	return RTK_FC_RET_OK;


G3_REUPDATE_FAIL_SWONLY:

	IGMP("update fail set sofware flow only");

	if(newHwMcgid!=CA_UINT16_INVALID && (swMcgid!=CA_UINT16_INVALID))
		aal_hw_mcgid_free(swMcgid,newHwMcgid);
	if(swMcgid!=CA_UINT16_INVALID)
		aal_sw_mcgid_free(swMcgid);

	//free ldpid tmp entry
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{
			if(pLdpid->g3PortPriv.hwFibHashAcc_tmp)
			{
				rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx_tmp);
				pLdpid->g3PortPriv.hwFibHashAcc_tmp=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn_tmp)
			{				
				rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacIdEn_tmp);
				pLdpid->g3PortPriv.swMacIdEn_tmp=0;
			}
		}
	}


	rtk_fc_abstrSwSyncToHw_del(swFlowIdx);

	fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->g3KeyHwPriv.hwacc=0;

	
	return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
	
}

int rtk_fc_abstrSwSyncToHw_g3McReFlushALL(void)
{

	int i;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
	{
		list_for_each_entry(mcSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
		{
			rtk_fc_abstrSwSyncToHw_g3McReFlush(mcSwFlowId_entry->swFlowIdx);
		}
	}
	IGMP("rtk_fc_abstrSwSyncToHw_g3McReFlushALL");

	return SUCCESS;

}





/* 
	only for multicast entry.
	using for multicast join/leave update flow table
*/
//CONFIG_RTK_L34_G3_PLATFORM
int rtk_fc_abstrSwSyncToHw_update(int32 swFlowIdx)
{

	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowActionField_entry_t *pActField,actField;
	ca_uint32_t h_idx;
	ca_uint16_t ori_swMcgid=CA_UINT16_INVALID,swMcgid=CA_UINT16_INVALID;
	ca_uint16_t newHwMcgid=CA_UINT16_INVALID,oriHwMcgid=CA_UINT16_INVALID;
	ca_uint16_t fistActIgrIntf=CA_UINT16_INVALID;
	int32 mainHashDefaultTAGTCI=FAIL;
	rtk_fc_mctop_vlanMode_t fibVlanMode=MCFIB_ERROR;
	rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowIdx=NULL;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	portForwardActionVlanExtend_t fwdAction;
#else
	portForwardAction_t fwdAction;
#endif
	int ret;

	pActField=&actField;

	if(FLOW_INFO_IS_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
	if(!pAbstrSwFlowEt->g3KeyHwPriv.hwacc)
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;

	mcSwFlowIdx = rtk_fc_igmp_mcExtraFlowIdxTbl_find(swFlowIdx);
	if(mcSwFlowIdx==NULL){IGMP("can't get mcflowIdxTbl"); goto G3_UPDATE_FAIL_SWONLY;}


	h_idx=pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx;
	ori_swMcgid=pAbstrSwFlowEt->g3KeyHwPriv.swMcgid;
	oriHwMcgid=pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid;

	//=============add mce ===============
	//setup1. get a sw_mcgid 
	//we using new swMcgid every times
	if(aal_sw_mcgid_alloc(&swMcgid)!=AAL_E_OK)
	{
		IGMP("aal_sw_mcgid_alloc Fail");
		goto G3_UPDATE_FAIL_SWONLY;
	}

	bzero(&fwdAction,sizeof(fwdAction));

	if(pAbstrSwFlowEt->copy2cpu)
	{
		rtk_fc_abstrSwFlowPattenField_entry_t field;
		rtk_fc_parseAbstrSwPattenField(pAbstrSwFlowEt->swFlowKey.bits.isIpv6,&pAbstrSwFlowEt->swFlowKey,&field);

		IGMP("SET A COPY2CPU MCFIB");
		//copy packet is original
		fwdAction.ldpid = RTK_FC_MAC_PORT_WLAN_CPU0;
		fwdAction.wifi_flowid = RTK_FC_COPY2CPU_INTF;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

		if(pAbstrSwFlowEt->swFlowKey.bits.stagif)
		{
			fwdAction.svlanId_cmd	 = ACTION_VLANTAG_BYTBL;
			fwdAction.svlanPri_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.svlanDei_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.svlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
			//FIXME we can't get svlan tpid here
			fwdAction.svlan_tpid = (*field.pSvlanTPID);
			fwdAction.svlanId = ((*field.pSvlanTCI)&VLAN_VID_MASK);
			fwdAction.svlanPri = ((*field.pSvlanTCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			fwdAction.svlanDei =   ((*field.pSvlanTCI)&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
		}
		if(pAbstrSwFlowEt->swFlowKey.bits.ctagif)
		{
			fwdAction.cvlanId_cmd	 = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlanPri_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlanDei_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
			fwdAction.cvlan_tpid = (*field.pCvlanTPID);
			fwdAction.cvlanId = ((*field.pCvlanTCI)&VLAN_VID_MASK);
			fwdAction.cvlanPri = ((*field.pCvlanTCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			fwdAction.cvlanDei = (  (*field.pCvlanTCI)&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
		}
#endif
		fwdAction.src_port_filter_dis=1;
		aal_port_forward_action_add(swMcgid,&fwdAction);
	}


	//all port already sync in hardware 
	if(swMcgid==CA_UINT16_INVALID)
		return RTK_FC_RET_OK;		

	rtk_fc_abstrSwSyncToHw_mcDecision(swFlowIdx,&mainHashDefaultTAGTCI,&fibVlanMode);

	//setup2.using sw_mcgid to add port action  
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		rtk_fc_parseAbstrSwActionField((pAbstrSwFlowEt->swFlowKey.bits.isIpv6),&pSwFlowAction->swFlowAction,pActField);
		bzero(&fwdAction,sizeof(fwdAction));


		if(fistActIgrIntf==CA_UINT16_INVALID)
			fistActIgrIntf=pSwFlowAction->igrIntf;

		if(pSwFlowAction->swFlowAction.bits.smacTrans)
		{
			fwdAction.mac_sa_cmd=1;
			memcpy(fwdAction.mac_sa,pActField->pSmac,6);
		}
		if(pSwFlowAction->swFlowAction.bits.userPriCmd)
		{
			fwdAction.cos_en=1;
			fwdAction.cos = (*pActField->pUserPri)&0x7;
		}

#if defined(CONFIG_FC_G3_G3LITE_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)
		if(pSwFlowAction->swFlowAction.bits.dmacTrans)
		{
			IGMP("Not Support Dmac trans for Mc yet!");
			goto G3_UPDATE_FAIL_SWONLY;
		}

		if(fibVlanMode == MCFIB_ONLY_CVALN_MODIFY)
		{
			if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
			{
				//default is untag after main hash we need push a cvlan
				fwdAction.vlan_cmd = ACTION_VLAN_PUSH;
				fwdAction.vpri_cmd = ACTION_VPRI_FROM_TABLE;
				fwdAction.dei_cmd = 1;
				fwdAction.vlan = (*pActField->pCvlanTCI&VLAN_VID_MASK);
				fwdAction.vpri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.dei =  (*pActField->pCvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
				fwdAction.topVlanType_svlan=0;
			}
			else
			{
				//default is untag after main hash 
				fwdAction.vlan_cmd = ACTION_VLAN_NOP;
			}

		}
		else if(fibVlanMode == MCFIB_ONLY_SVALN_MODIFY)
		{
			if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
			{
				//default is unstag after main hash we need push a cvlan
				fwdAction.vlan_cmd = ACTION_VLAN_PUSH;
				fwdAction.vpri_cmd = ACTION_VPRI_FROM_TABLE;
				fwdAction.dei_cmd = 1;
				fwdAction.vlan = (*pActField->pSvlanTCI&VLAN_VID_MASK);
				fwdAction.vpri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.dei =  (*pActField->pSvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
				fwdAction.topVlanType_svlan=1;
			}
			else
			{
				//default is unstag after main hash 
				fwdAction.vlan_cmd = ACTION_VLAN_NOP;
			}

		}
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(fibVlanMode == MCFIB_BOTH_SCVLAN_MODIFY)
		{
			if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
			{
				fwdAction.svlanId_cmd	 = ACTION_VLANTAG_BYTBL;
				fwdAction.svlanPri_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlanDei_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlan_tpid =(*pActField->pSvlanTpid);
				fwdAction.svlanId = (*pActField->pSvlanTCI&VLAN_VID_MASK);
				fwdAction.svlanPri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.svlanDei =   (*pActField->pSvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
			}
			else
			{
				//default is untag after main hash 
				fwdAction.svlanId_cmd = ACTION_VLAN_UNTAG;
			}
			
			if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
			{
				fwdAction.cvlanId_cmd    = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlanPri_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlanDei_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlan_tpid = (*pActField->pCvlanTpid);
				fwdAction.cvlanId = (*pActField->pCvlanTCI&VLAN_VID_MASK);
				fwdAction.cvlanPri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.cvlanDei =   (*pActField->pCvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
			}
			else
			{
				//default is untag after main hash 
				fwdAction.cvlanId_cmd = ACTION_VLAN_UNTAG;
			}

		}
#endif
		
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{

			if(pLdpid->isWlan)
			{
				rtk_fc_mac_port_idx_t macPort;
				rtk_fc_mac_ext_port_idx_t macExtPort;
				RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pLdpid->flowLdpid, &macPort, &macExtPort);
				fwdAction.ldpid = macPort;
				fwdAction.wifi_flowid = pLdpid->flowLdpid;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)					
				if(rtk_fc_abstrG3MctoUc_handle(pLdpid,pActField->pDmac,&fwdAction,mcSwFlowIdx,pSwFlowAction)!=RTK_FC_RET_OK)
					goto G3_UPDATE_FAIL_SWONLY;
#endif			
			}
			else
			{
				fwdAction.ldpid = pLdpid->flowLdpid;
				fwdAction.wifi_flowid = 0;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)					
				if(rtk_fc_abstrG3MctoUc_handle(pLdpid,pActField->pDmac,&fwdAction,mcSwFlowIdx,pSwFlowAction)!=RTK_FC_RET_OK)
					goto G3_UPDATE_FAIL_SWONLY;
#endif				
			}
			fwdAction.src_port_filter_dis=1;
			aal_port_forward_action_add(swMcgid,&fwdAction);
			pLdpid->g3PortPriv.hwFibAcc=1;
		}
	}

	//setup3.apply to get hw_mcgid
	ret=aal_hw_mcgid_new(swMcgid,&newHwMcgid);
	if(ret)
	{
		IGMP("aal_hw_mcgid_new Error ret:%d",ret);
		goto G3_UPDATE_FAIL_SWONLY;
	}
	//setup4.update hw_mcgid to your fucntion configuration and keep original ori_hw_mcgid
	// =============add main hash ===============
	ret=rtk_fc_syncToHw_G3updateGroupHash(swFlowIdx,h_idx,newHwMcgid,mainHashDefaultTAGTCI,fistActIgrIntf);
	if(ret)
	{
		IGMP("aal_hw_mcgid_new Error ret:%d",ret);
		goto G3_UPDATE_FAIL_SWONLY;
	}

	IGMP("UPDATE G3Mc hashIdx=%d ori_swMcgid=%d ->new_swMcgid=%d oriHwMcgid=%d -> newHwMcgid=%d fibVlanMode=%d mainHashDefaultTAGTCI=%x"
	,h_idx,ori_swMcgid,swMcgid,oriHwMcgid,newHwMcgid,fibVlanMode,mainHashDefaultTAGTCI);
	//setup5.free ori_hw_mcgid to de-reference
	ASSERT_EQ(aal_hw_mcgid_free(ori_swMcgid,oriHwMcgid),CA_E_OK);
	//free ori_swMcgid to delete
	ASSERT_EQ(aal_sw_mcgid_free(ori_swMcgid),CA_E_OK);

	//free old ldpid hash info and restore tmp to entry
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{
			if(pLdpid->g3PortPriv.hwFibHashAcc && pLdpid->g3PortPriv.hwFibHashAcc_tmp && pLdpid->g3PortPriv.hwFibHashIdx==pLdpid->g3PortPriv.hwFibHashIdx_tmp)
			{
				pLdpid->g3PortPriv.hwFibHashAcc=0;
			}
			else if(pLdpid->g3PortPriv.hwFibHashAcc)
			{
				rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx);
				pLdpid->g3PortPriv.hwFibHashAcc=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn)
			{
				rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacId);
				pLdpid->g3PortPriv.swMacIdEn=0;
			}
			if(pLdpid->g3PortPriv.hwFibHashAcc_tmp)
			{
				pLdpid->g3PortPriv.hwFibHashAcc = pLdpid->g3PortPriv.hwFibHashAcc_tmp;
				pLdpid->g3PortPriv.hwFibHashIdx = pLdpid->g3PortPriv.hwFibHashIdx_tmp;
				pLdpid->g3PortPriv.hwFibHashAcc_tmp=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn_tmp)
			{
				pLdpid->g3PortPriv.swMacIdEn = pLdpid->g3PortPriv.swMacIdEn_tmp;
				pLdpid->g3PortPriv.swMacId =  pLdpid->g3PortPriv.swMacId_tmp;
				pLdpid->g3PortPriv.swMacIdEn_tmp=0;
			}			
		}
	}

	//update swMcgid & newHwMcgid
	pAbstrSwFlowEt->g3KeyHwPriv.swMcgid = swMcgid;
	pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid = newHwMcgid;
	pAbstrSwFlowEt->g3KeyHwPriv.hwacc=1;



	return RTK_FC_RET_OK;


G3_UPDATE_FAIL_SWONLY:

	IGMP("update fail set sofware flow only");

	if(newHwMcgid!=CA_UINT16_INVALID && (swMcgid!=CA_UINT16_INVALID))
		aal_hw_mcgid_free(swMcgid,newHwMcgid);
	if(swMcgid!=CA_UINT16_INVALID)
		aal_sw_mcgid_free(swMcgid);

	//free ldpid tmp entry
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)			
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{
			if(pLdpid->g3PortPriv.hwFibHashAcc_tmp)
			{
				rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx_tmp);
				pLdpid->g3PortPriv.hwFibHashAcc_tmp=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn_tmp)
			{				
				rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacIdEn_tmp);
				pLdpid->g3PortPriv.swMacIdEn_tmp=0;
			}
		}
	}
#endif

	rtk_fc_abstrSwSyncToHw_del(swFlowIdx);

	fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->g3KeyHwPriv.hwacc=0;

	
	return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
	
}


//just delete mapping hw flow 
//CONFIG_RTK_L34_G3_PLATFORM
int rtk_fc_abstrSwSyncToHw_del(int32 swFlowIdx)
{

	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	int32 hwIdx=FAIL;
	

	IGMP("clear swFlowIdx[%d] hardware",swFlowIdx);

	if(pAbstrSwFlowEt->g3KeyHwPriv.hwacc)
	{
		list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
		{	
			list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
			{
				//clear software  private field
				pLdpid->g3PortPriv.hwFibAcc=0;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)		
				if(pLdpid->g3PortPriv.hwFibHashAcc)
				{
					IGMP("delete hwFibHashIdx flow :%d",pLdpid->g3PortPriv.hwFibHashIdx);
					rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx);
					pLdpid->g3PortPriv.hwFibHashAcc=0;
				}
				if(pLdpid->g3PortPriv.swMacIdEn)
				{
					rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacId);
					pLdpid->g3PortPriv.swMacIdEn=0;
				}
#endif					
			}
		}
 		hwIdx=pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx;

		aal_hash_delete(0, pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx);
 		aal_hw_mcgid_free(pAbstrSwFlowEt->g3KeyHwPriv.swMcgid,pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid);
 		aal_sw_mcgid_free(pAbstrSwFlowEt->g3KeyHwPriv.swMcgid);
		TABLE("aal_hash_delete [%d] swMcgid[%d] hwMcgid[%d]",hwIdx,pAbstrSwFlowEt->g3KeyHwPriv.swMcgid,pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid);
 		pAbstrSwFlowEt->g3KeyHwPriv.hwacc=0;
		pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx = CA_UINT16_INVALID;
		pAbstrSwFlowEt->g3KeyHwPriv.swMcgid = CA_UINT16_INVALID;
		pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid = CA_UINT16_INVALID;
	}
	

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
{
	rtk_fc_tableFlow_t *pFlowTable;

	if(hwIdx==FAIL)
	{
		IGMP("is not hardware acc entry");
		return RTK_FC_RET_OK;
	}
		IGMP("clear hwflow[%d]",hwIdx);

	//invalid hwFlow_validBitsArray
	if(hwIdx < fc_db.flowHwTableSize)
	{
		fc_db.g3_mHashTbl_validBitsArray[(hwIdx >> 5)] &= ~(0x1 << (hwIdx&0x1f));
	}
	
	pFlowTable = &fc_db.flowTbl[hwIdx];

	pFlowTable->pFlowEntry->path1.valid = 0;

	// initialize sw field of hwflow
	pFlowTable->cachedCt = NULL;
	//--------------- RESET: init value which is 0	   -------------------------------------------------------------------------------//
	memset((uint8 *)pFlowTable + offsetof(rtk_fc_tableFlow_t, cachedCt) , 0 , (sizeof(rtk_fc_tableFlow_t)- offsetof(rtk_fc_tableFlow_t, cachedCt) ) );
	//------------------------------------------------------------------------------------------------------------------------------//
	//----------------RESET: init value which is not 0 -------------------------------------------------------------------------------//
	pFlowTable->loopbackRevFlowIdx = SIGNED_INVALID;		// shared info
	pFlowTable->lutIgrSaIdx = SIGNED_INVALID;
	pFlowTable->lutEgrDaIdx = SIGNED_INVALID;
	
	pFlowTable->pAbstrSwFlowEt=NULL; //we free pSwFlowEt in software reference not hardward referece
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
	if(pFlowTable->dummyPkt) bzero(pFlowTable->dummyPkt, sizeof(rtk_fc_igrDummyData_t));
#endif
	//------------------------------------------------------------------------------------------------------------------------------//
}
#else
	// G3 A/B version
	if(fc_db.flowTbl[swFlowIdx].mainHashIdx != G3_FLOWIDX_INVALID)
		fc_db.g3_mHashTbl_validBitsArray[(fc_db.flowTbl[swFlowIdx].mainHashIdx >> 5)] &= ~(0x1 << (fc_db.flowTbl[swFlowIdx].mainHashIdx&0x1f));
	// reset mainHashIdx after delete entry
	fc_db.flowTbl[swFlowIdx].mainHashIdx = G3_FLOWIDX_INVALID;
#endif

	
	return RTK_FC_RET_OK;

}



//CONFIG_RTK_L34_G3_PLATFORM
int rtk_fc_abstrSwSyncToHw_add(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int32 swFlowIdx)
{
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	rtk_fc_abstrSwFlowActionField_entry_t *pActField,actField;
	rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowIdx=NULL;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	portForwardActionVlanExtend_t fwdAction;
#else
	portForwardAction_t fwdAction;
#endif	

	ca_uint32_t h_idx=G3_FLOWIDX_INVALID;
	ca_uint16_t swMcgid=CA_UINT16_INVALID;
	ca_uint16_t hwMcgid=CA_UINT16_INVALID;
	ca_uint16_t fistActIgrIntf=CA_UINT16_INVALID;
	ca_uint16_t fistActDscp_en=0;
	ca_uint16_t fistActDscp=CA_UINT16_INVALID;
	int32 ret=SUCCESS;

	if(FLOW_INFO_IS_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
	if(pAbstrSwFlowEt->g3KeyHwPriv.hwacc)
		return RTK_FC_RET_ERR_ENTRY_EXIST;

	pActField=&actField;

	if(pPktHdr->isMulticast)
	{
		//hardware ability check
		int32 mainHashDefaultTAGTCI=FAIL;
		rtk_fc_mctop_vlanMode_t fibVlanMode=MCFIB_ERROR;

		mcSwFlowIdx = rtk_fc_igmp_mcExtraFlowIdxTbl_find(swFlowIdx);
		if(mcSwFlowIdx==NULL){IGMP("can't get mcflowIdxTbl"); goto G3_ADD_FAIL_SWONLY;}

		//=============add mce ===============
		//setup1. get a sw_mcgid
		if(aal_sw_mcgid_alloc(&swMcgid)!=AAL_E_OK)
			goto G3_ADD_FAIL_SWONLY;

		rtk_fc_abstrSwSyncToHw_mcDecision(swFlowIdx,&mainHashDefaultTAGTCI,&fibVlanMode);

		//setup2.using sw_mcgid to adddel port action  
		bzero(&fwdAction,sizeof(fwdAction));		
		if(pAbstrSwFlowEt->copy2cpu)
		{
			rtk_fc_abstrSwFlowPattenField_entry_t field;
			rtk_fc_parseAbstrSwPattenField(pAbstrSwFlowEt->swFlowKey.bits.isIpv6,&pAbstrSwFlowEt->swFlowKey,&field);

			IGMP("SET A COPY2CPU MCFIB");
			//copy packet is original
			fwdAction.ldpid = RTK_FC_MAC_PORT_WLAN_CPU0;
			fwdAction.wifi_flowid = RTK_FC_COPY2CPU_INTF;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			if(pAbstrSwFlowEt->swFlowKey.bits.stagif)
			{
				fwdAction.svlanId_cmd	 = ACTION_VLANTAG_BYTBL;
				fwdAction.svlanPri_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlanDei_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.svlan_tpid = (*field.pSvlanTPID);
				fwdAction.svlanId = ((*field.pSvlanTCI)&VLAN_VID_MASK);
				fwdAction.svlanPri = ((*field.pSvlanTCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.svlanDei =   ((*field.pSvlanTCI)&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
			}
			if(pAbstrSwFlowEt->swFlowKey.bits.ctagif)
			{
				fwdAction.cvlanId_cmd	 = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlanPri_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlanDei_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
				fwdAction.cvlan_tpid =  (*field.pCvlanTPID);
				fwdAction.cvlanId = ((*field.pCvlanTCI)&VLAN_VID_MASK);
				fwdAction.cvlanPri = ((*field.pCvlanTCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				fwdAction.cvlanDei =   ((*field.pCvlanTCI)&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
			}
#endif
			fwdAction.src_port_filter_dis=1;
			aal_port_forward_action_add(swMcgid,&fwdAction);
		}

		list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
		{
			if(fistActIgrIntf==CA_UINT16_INVALID)
				fistActIgrIntf=pSwFlowAction->igrIntf;
			rtk_fc_parseAbstrSwActionField((pPktHdr->ip6h) ? TRUE : FALSE,&pSwFlowAction->swFlowAction,pActField);
			bzero(&fwdAction,sizeof(fwdAction));

			if(pSwFlowAction->swFlowAction.bits.dscpCmd && fistActDscp==CA_UINT16_INVALID )
			{
				fistActDscp_en=TRUE;
				fistActDscp=(*pActField->pTos) >>2;
			}

			if(fistActDscp!=CA_UINT16_INVALID && (fistActDscp!=    (*pActField->pTos) >>2))
			{
				IGMP("Not Support egress Multi-Dscp set software only");
				goto G3_ADD_FAIL_SWONLY;
			}
			
			if(pSwFlowAction->swFlowAction.bits.smacTrans)
			{
				fwdAction.mac_sa_cmd=1;
				memcpy(fwdAction.mac_sa,pActField->pSmac,6);
			}
			if(pSwFlowAction->swFlowAction.bits.userPriCmd)
			{
				fwdAction.cos_en=1;
				fwdAction.cos = (*pActField->pUserPri)&0x7;
			}


#if defined(CONFIG_FC_G3_G3LITE_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)	
			if(pSwFlowAction->swFlowAction.bits.dmacTrans)
			{
				IGMP("Not Support Dmac trans for Mc yet!");
				goto G3_ADD_FAIL_SWONLY;
			}

			if(fibVlanMode == MCFIB_ONLY_CVALN_MODIFY)
			{
				if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
				{
					//default is untag after main hash we need push a cvlan
					fwdAction.vlan_cmd = ACTION_VLAN_PUSH;
					fwdAction.vpri_cmd = ACTION_VPRI_FROM_TABLE;
					fwdAction.dei_cmd = 1;
					fwdAction.vlan = (*pActField->pCvlanTCI&VLAN_VID_MASK);
					fwdAction.vpri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					fwdAction.dei =  (*pActField->pCvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
					fwdAction.topVlanType_svlan=0;
				}
				else
				{
					//default is untag after main hash 
					fwdAction.vlan_cmd = ACTION_VLAN_NOP;
				}
			
			}
			else if(fibVlanMode == MCFIB_ONLY_SVALN_MODIFY)
			{
				if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
				{
					//default is unstag after main hash we need push a cvlan
					fwdAction.vlan_cmd = ACTION_VLAN_PUSH;
					fwdAction.vpri_cmd = ACTION_VPRI_FROM_TABLE;
					fwdAction.dei_cmd = 1;
					fwdAction.vlan = (*pActField->pSvlanTCI&VLAN_VID_MASK);
					fwdAction.vpri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					fwdAction.dei =  (*pActField->pSvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
					fwdAction.topVlanType_svlan=1;
				}
				else
				{
					//default is unstag after main hash 
					fwdAction.vlan_cmd = ACTION_VLAN_NOP;
				}
			
			}
			else
			{
				IGMP("not support Both Vlan Modify");
				goto G3_ADD_FAIL_SWONLY;
			}
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

			if(fibVlanMode == MCFIB_BOTH_SCVLAN_MODIFY)
			{
				if(pSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
				{
					fwdAction.svlanId_cmd	 = ACTION_VLANTAG_BYTBL;
					fwdAction.svlanPri_cmd = ACTION_VLANTAG_BYTBL;
					fwdAction.svlanDei_cmd = ACTION_VLANTAG_BYTBL;
					fwdAction.svlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
					fwdAction.svlan_tpid = (*pActField->pSvlanTpid);
					fwdAction.svlanId = (*pActField->pSvlanTCI&VLAN_VID_MASK);
					fwdAction.svlanPri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					fwdAction.svlanDei =   (*pActField->pSvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
				}
				else
				{
					//default is untag after main hash 
					fwdAction.svlanId_cmd = ACTION_VLAN_UNTAG;
				}
				
				if(pSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
				{
					fwdAction.cvlanId_cmd	 = ACTION_VLANTAG_BYTBL;
					fwdAction.cvlanPri_cmd = ACTION_VLANTAG_BYTBL;
					fwdAction.cvlanDei_cmd = ACTION_VLANTAG_BYTBL;
					fwdAction.cvlan_tpid_cmd = ACTION_VLANTAG_BYTBL;
					fwdAction.cvlan_tpid = (*pActField->pCvlanTpid);
					fwdAction.cvlanId = (*pActField->pCvlanTCI&VLAN_VID_MASK);
					fwdAction.cvlanPri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					fwdAction.cvlanDei =   (*pActField->pCvlanTCI&VLAN_CFI_MASK) >>VLAN_CFI_SHIFT;
				}
				else
				{
					//default is untag after main hash 
					fwdAction.cvlanId_cmd = ACTION_VLAN_UNTAG;
				}

			}
#else
			if(pSwFlowAction->swFlowAction.bits.dmacTrans)
			{
				dmacTransNotSupport=1;
				IGMP("Not Support Dmac trans for Mc yet!");
				goto G3_ADD_FAIL_SWONLY;
			}
			WARNING("G3Series New Chip FIXME");
#endif
			
			list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
			{
				if(pLdpid->isWlan)
				{
					rtk_fc_mac_port_idx_t macPort;
					rtk_fc_mac_ext_port_idx_t macExtPort;
					RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(pLdpid->flowLdpid, &macPort, &macExtPort);
					fwdAction.ldpid = macPort;
					fwdAction.wifi_flowid = pLdpid->flowLdpid;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)					
					if(rtk_fc_abstrG3MctoUc_handle(pLdpid,pActField->pDmac,&fwdAction,mcSwFlowIdx,pSwFlowAction)!=RTK_FC_RET_OK)
						goto G3_ADD_FAIL_SWONLY;
#endif

				}
				else
				{
					
					fwdAction.ldpid = pLdpid->flowLdpid;
					fwdAction.wifi_flowid = 0;
					if(pLdpid->flowLdpid==RTK_FC_MAC_PORT_PON      && (pSwFlowAction->swFlowAction.bits.ponStreamIdCmd))
					{
						if(fc_db.streamidTbl[*pActField->pPonStreaId].valid)
						{
							fwdAction.cos_en=1;
							fwdAction.cos=fc_db.streamidTbl[*pActField->pPonStreaId].cos;
							fwdAction.wifi_flowid = fc_db.streamidTbl[*pActField->pPonStreaId].gemid;
							fwdAction.ldpid = fc_db.streamidTbl[*pActField->pPonStreaId].ldpid;
						}
					}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)					
					if(rtk_fc_abstrG3MctoUc_handle(pLdpid,pActField->pDmac,&fwdAction,mcSwFlowIdx,pSwFlowAction)!=RTK_FC_RET_OK)
						goto G3_ADD_FAIL_SWONLY;
#endif					
				}
				fwdAction.src_port_filter_dis=1;
				ret = aal_port_forward_action_add(swMcgid,&fwdAction);
				
				if(ret)
						{IGMP("aal_port_forward_action_add Fail"); goto G3_ADD_FAIL_SWONLY;}
						
				pLdpid->g3PortPriv.hwFibAcc=1;
				if(pLdpid->g3PortPriv.hwFibHashAcc_tmp)
				{
					pLdpid->g3PortPriv.hwFibHashAcc = pLdpid->g3PortPriv.hwFibHashAcc_tmp;
					pLdpid->g3PortPriv.hwFibHashIdx = pLdpid->g3PortPriv.hwFibHashIdx_tmp;
					pLdpid->g3PortPriv.hwFibHashAcc_tmp=0;
				}
				if(pLdpid->g3PortPriv.swMacIdEn_tmp)
				{
					pLdpid->g3PortPriv.swMacIdEn = pLdpid->g3PortPriv.swMacIdEn_tmp;
					pLdpid->g3PortPriv.swMacId =  pLdpid->g3PortPriv.swMacId_tmp;
					pLdpid->g3PortPriv.swMacIdEn_tmp=0;
				}								
			}
		}

		//setup3.apply to get hw_mcgid
		ret = aal_hw_mcgid_new(swMcgid,&hwMcgid);
		if(ret)
			{IGMP("aal_hw_mcgid_new Fail"); goto G3_ADD_FAIL_SWONLY;}



		//setup4.update hw_mcgid to your fucntion configuration and keep original ori_hw_mcgid
		// =============add main hash ===============
		ret=rtk_fc_syncToHw_G3addGroupFwdHash(rtskb,pPktHdr,fistActDscp_en,fistActDscp,&h_idx,hwMcgid,mainHashDefaultTAGTCI,fistActIgrIntf);
		if(ret)
			goto G3_ADD_FAIL_SWONLY;
		
		pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx=h_idx;
		pAbstrSwFlowEt->g3KeyHwPriv.swMcgid = swMcgid;
		pAbstrSwFlowEt->g3KeyHwPriv.hwMcgid = hwMcgid;
		pAbstrSwFlowEt->g3KeyHwPriv.hwacc=1;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(h_idx!=G3_FLOWIDX_INVALID)
		{
			// ocupy flow HW entry
			fc_db.flowTbl[h_idx].pFlowEntry->path1.valid=TRUE;
			fc_db.flowTbl[h_idx].candidateState=CANDIDATE_STATE_READY;
			fc_db.g3_mHashTbl_validBitsArray[(h_idx >> 5)] |= (0x1 << (h_idx&0x1f));
			fc_db.flowTbl[h_idx].pAbstrSwFlowEt=pAbstrSwFlowEt; //just for refer to swEntry
			FLOW_INFO_SET(&fc_db.flowTbl[h_idx], FLOW_INFO_STATIC_ENTRY, 1);
			FLOW_INFO_SET(&fc_db.flowTbl[h_idx], FLOW_INFO_MC_ENTRY, 1);
			fc_db.flowTbl[h_idx].crc16 = fc_db.flowTbl[swFlowIdx].crc16;
			fc_db.flowTbl[h_idx].crc32 = fc_db.flowTbl[swFlowIdx].crc32;
		}
		else
		{
			goto G3_ADD_FAIL_SWONLY;
		}
#else
		//for traffic timeout
		if(h_idx!=G3_FLOWIDX_INVALID)
		{
			fc_db.flowTbl[swFlowIdx].mainHashIdx = h_idx;
			fc_db.flowTbl[swFlowIdx].candidateState=CANDIDATE_STATE_READY;
			fc_db.g3_mHashTbl_validBitsArray[(h_idx >> 5)] |= (0x1 << (h_idx&0x1f));
		}
		else
		{
			fc_db.flowTbl[swFlowIdx].mainHashIdx=G3_FLOWIDX_INVALID;
			goto G3_ADD_FAIL_SWONLY;
		}
#endif
		IGMP("ADD G3Mc hashIdx=%d swMcgid=%d hwMcgid=%d fibVlanMode=%d mainHashDefaultTAGTCI=%x"
		,h_idx,swMcgid,hwMcgid,fibVlanMode,mainHashDefaultTAGTCI);
		
		return RTK_FC_RET_OK;

	}
	else
	{
		WARNING("not multicast not support yet");
		return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
	}

G3_ADD_FAIL_SWONLY:

	FLOW_INFO_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY, 1);

	fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->g3KeyHwPriv.hwacc=0;

	if(hwMcgid!=CA_UINT16_INVALID && (swMcgid!=CA_UINT16_INVALID))
		aal_hw_mcgid_free(swMcgid,hwMcgid);
	if(swMcgid!=CA_UINT16_INVALID)
		aal_sw_mcgid_free(swMcgid);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	// reset after delete entry
	if(h_idx!=G3_FLOWIDX_INVALID)
	{
		// release flow HW entry
		fc_db.flowTbl[h_idx].pFlowEntry->path1.valid=FALSE;
		fc_db.flowTbl[h_idx].candidateState=CANDIDATE_STATE_NONE;
		fc_db.g3_mHashTbl_validBitsArray[(h_idx >> 5)] &= ~(0x1 << (h_idx&0x1f));
		fc_db.flowTbl[h_idx].pAbstrSwFlowEt=NULL;
		pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx = CA_UINT16_INVALID;
		aal_hash_delete(0, h_idx);
	}
#else
	// reset mainHashIdx after delete entry
	if(h_idx!=G3_FLOWIDX_INVALID)
	{
		fc_db.g3_mHashTbl_validBitsArray[(h_idx >> 5)] &= ~(0x1 << (h_idx&0x1f));
		fc_db.flowTbl[swFlowIdx].mainHashIdx = G3_FLOWIDX_INVALID;
		pAbstrSwFlowEt->g3KeyHwPriv.hwHashIdx = CA_UINT16_INVALID;
		aal_hash_delete(0, h_idx);
	}
#endif

	//free ldpid hash info and restore tmp to entry
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{
			if(pLdpid->g3PortPriv.hwFibHashAcc)
			{
				rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx);
				pLdpid->g3PortPriv.hwFibHashAcc=0;
			}
			if(pLdpid->g3PortPriv.swMacIdEn)
			{
				rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacId);
				pLdpid->g3PortPriv.swMacIdEn=0;
			}
		}
	}


	fc_db.flowTbl[swFlowIdx].ingressSaHostPolIdx = SIGNED_INVALID;
	fc_db.flowTbl[swFlowIdx].egressDaHostPolIdx = SIGNED_INVALID;
	

	return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;	
}
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

/* 
	only for multicast/bc entry.
	just update portmask/extpmsk and vlan action
*/

//CONFIG_RTK_L34_XPON_PLATFORM
int rtk_fc_abstrSwSyncToHw_update(int32 swFlowIdx)
{

	int ret;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	uint32 actionCount=0U;
	uint32 pmsk=0U,p135Pmsk=0U,p24Pmsk=0U,newPmsk=0U,oriP135Pmsk=0U,oriP24Pmsk=0U;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL,*pP135SwFlowAction=NULL,*pP24SwFlowAction=NULL,*newSwFlowAction=NULL;
	rtk_fc_wlan_devmask_t wlanMsk=0ULL,p135_wlanMsk = 0ULL, p24_wlanMsk = 0ULL,new_wlanMsk = 0ULL;
	rtk_fc_abstrSwFlowActionField_entry_t *p135ActField=NULL,actField,*p24ActField=NULL,secActField,*pNewActField=NULL,newActField;
	rtk_fc_ext_port_mask_t p135Extpmsk = 0, p24Extpmsk = 0, newExtpmsk = 0;
	int32 oriP135ExtPmskHwIdx=FAIL,oriP24ExtPmskHwIdx=FAIL,p135ExtPmskHwIdx=FAIL,p24ExtPmskHwIdx=FAIL;
	rtk_rg_asic_path1_entry_t p13Data;
	rtk_rg_asic_path2_entry_t p24Data;
	int32 path13Idx=FAIL,path24Idx=FAIL;
	int32 oriP13MutiAct=0;

	if(FLOW_INFO_IS_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
	if(!pAbstrSwFlowEt->xponKeyHwPriv.hwacc)
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;


	p135ActField=&actField;
	p24ActField=&secActField;
	pNewActField=&newActField;


	//--------------For hardward not support behavior---------------------------------------------------
	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		pmsk=0U;
		wlanMsk=0ULL;

		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{

			if(pLdpid->isWlan)
			{
				if(wlanMsk& ((rtk_fc_wlan_devmask_t)1<<pLdpid->flowLdpid))
					goto UPDATE_FAIL_SWONLY;
				else
					wlanMsk|=((rtk_fc_wlan_devmask_t)1<<pLdpid->flowLdpid);
			}
			else
			{
				if(pmsk& (1<<pLdpid->flowLdpid))
					goto UPDATE_FAIL_SWONLY;
				else
					pmsk|=(1<<pLdpid->flowLdpid);
			}
			
		}
		actionCount++;

		
		if(actionCount <= 2U)
		{
			if(pSwFlowAction->xponActPriv.hwacc)
			{
				if(pSwFlowAction->xponActPriv.isPath24==0U)
				{
					pP135SwFlowAction = pSwFlowAction;
					p135Pmsk = pmsk;
					p135_wlanMsk = wlanMsk;
					RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(p135_wlanMsk, &p135Extpmsk);
					rtk_fc_parseAbstrSwActionField((pAbstrSwFlowEt->swFlowKey.bits.isIpv6) ? TRUE : FALSE,&pSwFlowAction->swFlowAction,p135ActField);
					TRACE("Path135 update to p135Pmsk=%x p135Extpmsk=%x",p135Pmsk,p135Extpmsk);
				}
				else
				{
					pP24SwFlowAction = pSwFlowAction;
					p24Pmsk = pmsk;
					p24_wlanMsk = wlanMsk;
					RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(p24_wlanMsk, &p24Extpmsk);
					rtk_fc_parseAbstrSwActionField((pAbstrSwFlowEt->swFlowKey.bits.isIpv6) ? TRUE : FALSE,&pSwFlowAction->swFlowAction,p24ActField);
					TRACE("Path24 update to p24Pmsk=%x p24Extpmsk=%x",p24Pmsk,p24Extpmsk);					
				}
			}
			else
			{
				newSwFlowAction = pSwFlowAction;
				newPmsk=pmsk;
				new_wlanMsk = wlanMsk;
				TRACE("new action add");
				RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(new_wlanMsk, &newExtpmsk);
				rtk_fc_parseAbstrSwActionField((pAbstrSwFlowEt->swFlowKey.bits.isIpv6) ? TRUE : FALSE,&pSwFlowAction->swFlowAction,pNewActField);
			}
 		}
		else
		{
			TRACE("actionCount > 3 not support");
			goto UPDATE_FAIL_SWONLY;
		}
	}

	// check error
	if(pP135SwFlowAction && pP24SwFlowAction==NULL && newSwFlowAction)
	{
		//newFlowAction put in path24
		p24Pmsk=newPmsk;
		p24Extpmsk=newExtpmsk;
	}
	else if (pP135SwFlowAction==NULL && pP24SwFlowAction && newSwFlowAction)
	{
		//newFlowAction put in path13
		p135Pmsk=newPmsk;
		p135Extpmsk=newExtpmsk;
	}
	else if(pP135SwFlowAction==NULL && pP24SwFlowAction==NULL)
	{
		WARNING(" flow update Error"); goto UPDATE_FAIL_SWONLY;
	}


	//forward a copy to cpu 
	if((p135Extpmsk||p24Extpmsk) && pAbstrSwFlowEt->copy2cpu)
	{
		//not support hardware accelerate
		TRACE("Not support copy2cpu and send to wifi!");		
		goto UPDATE_FAIL_SWONLY;
	}
	else if(p135Extpmsk==0 && pAbstrSwFlowEt->copy2cpu)
	{
		p135Pmsk|= (1<<RTK_FC_MAC_PORT_CPU);
		IGMP("Copy2CPU flow");
	}


	if (((p135Pmsk&p24Pmsk) || (p135Extpmsk&p24Extpmsk)))
		goto UPDATE_FAIL_SWONLY;
	if(p135Pmsk==0U && p24Pmsk==0U && p135Extpmsk==0U && p24Extpmsk==0U)
		{IGMP("Multicast flow update to pmsk zero delete hw entry"); goto UPDATE_FAIL_SWONLY;}
	else if( p24Pmsk==0U &&  p24Extpmsk==0U && pP24SwFlowAction)
	{
		//delete path4 entry if no need to send
		if(pP24SwFlowAction->xponActPriv.hwacc)
			rtk_fc_abstrXPONHw_del(pP24SwFlowAction->xponActPriv.hwIdx);
		pP24SwFlowAction->xponActPriv.hwacc=0U;
		TABLE("p24Pmsk && p24ExtPmskHwIdx zero delete hwflow[%d]",pP24SwFlowAction->xponActPriv.hwIdx);
		pP24SwFlowAction=NULL;
	}
	


	//===============start update hardware entry===============

	path13Idx=pAbstrSwFlowEt->xponKeyHwPriv.flowP13hwIdx;

	memcpy(&p13Data, &fc_db.flowTbl[path13Idx].pFlowEntry->path3, sizeof(rtk_rg_asic_path3_entry_t));
	_rtk_fc_sharing_image_flow_structure_convert((rtk_rg_asic_path1_entry_t *)&p13Data);	


	if(pP135SwFlowAction)
	{
		if( fc_db.extPortTbl[p13Data.out_ext_portmask_idx].extPortEnt.extpmask != p135Extpmsk)
		{
			oriP135ExtPmskHwIdx = p13Data.out_ext_portmask_idx;
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(p135Extpmsk,&p135ExtPmskHwIdx)!=RTK_FC_RET_OK)
			{
				WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
			}
			p13Data.out_ext_portmask_idx =	p135ExtPmskHwIdx;
		
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(oriP135ExtPmskHwIdx)!=RTK_FC_RET_OK)
			{
				WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
			}
		}
		oriP135Pmsk=p13Data.out_portmask;
		p13Data.out_portmask = p135Pmsk;

		oriP13MutiAct = p13Data.out_multiple_act;
		if(pP24SwFlowAction || newSwFlowAction)
			p13Data.out_multiple_act=TRUE;
		else
			p13Data.out_multiple_act=FALSE;

		if(oriP135Pmsk!=p135Pmsk || oriP135ExtPmskHwIdx!=p135ExtPmskHwIdx || oriP13MutiAct!=p13Data.out_multiple_act)
		{
			ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path13Idx, &p13Data, NULL, 0), RTK_FC_RET_OK);
			TABLE("path13Idx[%d] update to out_portmask=%x->%x out_ext_portmask_idx=%d->%d(-1:no-change) out_multiple_act=%d->%d",path13Idx,oriP135Pmsk,p135Pmsk,oriP135ExtPmskHwIdx,p135ExtPmskHwIdx,oriP13MutiAct,p13Data.out_multiple_act);
		}
	}
	else if(pP135SwFlowAction==NULL && pP24SwFlowAction && newSwFlowAction)
	{
		IGMP("use a path3(useless) ation for newSwFlowAction ");
		p13Data.out_smac_trans = newSwFlowAction->swFlowAction.bits.smacTrans;

		if(newSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_UNTAG)
		{
			p13Data.out_svlan_id = 0U;
			p13Data.out_svid_format_act = FALSE;
			p13Data.out_spri_format_act = FALSE;
			p13Data.out_spri = 0U;
		}
		else if (newSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_TAG)
		{
			p13Data.out_svid_format_act = TRUE;
			p13Data.out_spri_format_act = TRUE;
			p13Data.out_spri = (*pNewActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			p13Data.out_svlan_id = (*pNewActField->pSvlanTCI&VLAN_VID_MASK);
		}
		
		//cvlan
		p13Data.out_egress_cvid_act = TRUE;
		p13Data.out_ctag_format_act = TRUE;
		if(newSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_UNTAG)
		{
			p13Data.out_cvlan_id = 0U;
			p13Data.out_cvid_format_act = FALSE;
			p13Data.out_cpri_format_act = FALSE;
			p13Data.out_cpri = 0U;
		}
		else if (newSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_TAG)
		{
			p13Data.out_cvid_format_act = TRUE;
			p13Data.out_cpri_format_act = TRUE;
			p13Data.out_cpri = (*pNewActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			p13Data.out_cvlan_id = (*pNewActField->pCvlanTCI&VLAN_VID_MASK);
		}

		p13Data.out_dscp_act = newSwFlowAction->swFlowAction.bits.dscpCmd;
		p13Data.out_dscp = (*pNewActField->pTos) >>2 ;

		//setting Path4 PortMask and ExtPmsk	
		if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(newExtpmsk,&p135ExtPmskHwIdx)!=RTK_FC_RET_OK)
		{
			WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
			goto UPDATE_FAIL_SWONLY;
		}

		p13Data.out_portmask = p135Pmsk;
		p13Data.out_ext_portmask_idx = p135ExtPmskHwIdx;
		p13Data.out_multiple_act=TRUE;
		
		ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path13Idx, &p13Data, NULL, 0), RTK_FC_RET_OK);
		TABLE("path13Idx[%d] using to out_portmask=%x out_ext_portmask_idx=%d ",path13Idx,p135Pmsk,p135ExtPmskHwIdx);

		newSwFlowAction->xponActPriv.hwacc=1U;
		newSwFlowAction->xponActPriv.hwIdx=path13Idx;
		newSwFlowAction->xponActPriv.isPath24=0U;
		

	}

	
	if(pP24SwFlowAction)
	{
		path24Idx=pP24SwFlowAction->xponActPriv.hwIdx;
		memcpy(&p24Data, &fc_db.flowTbl[path24Idx].pFlowEntry->path4, sizeof(rtk_rg_asic_path4_entry_t));
		_rtk_fc_sharing_image_flow_structure_convert((rtk_rg_asic_path1_entry_t *)&p24Data);

		if( fc_db.extPortTbl[p24Data.out_ext_portmask_idx].extPortEnt.extpmask != p24Extpmsk)
		{
			oriP24ExtPmskHwIdx = p24Data.out_ext_portmask_idx;
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(p24Extpmsk,&p24ExtPmskHwIdx)!=RTK_FC_RET_OK)
			{
				WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
				goto UPDATE_FAIL_SWONLY;
			}
			p24Data.out_ext_portmask_idx =	p24ExtPmskHwIdx;
		
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(oriP24ExtPmskHwIdx)!=RTK_FC_RET_OK)
			{
				WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
				goto UPDATE_FAIL_SWONLY;
			}
		}
		oriP24Pmsk=p24Data.out_portmask;
		p24Data.out_portmask = p24Pmsk;

		if(oriP24Pmsk!=p24Pmsk || oriP24ExtPmskHwIdx!=p24ExtPmskHwIdx)
		{
			TABLE("path24Idx[%d] update to out_portmask=%x->%x out_ext_portmask_idx=%d->%d ",path24Idx,oriP24Pmsk,p24Pmsk,oriP24ExtPmskHwIdx,p24ExtPmskHwIdx);
			ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path24Idx, &p24Data, NULL, 0), RTK_FC_RET_OK);
		}
		
	}
	else if (pP135SwFlowAction && pP24SwFlowAction==NULL && newSwFlowAction)
	{
		//add new action for path4
		memcpy(&p24Data, &p13Data, sizeof(rtk_rg_asic_path4_entry_t));
		//init path4 parameter
		
		p24Data.out_smac_trans = newSwFlowAction->swFlowAction.bits.smacTrans;
		p24Data.in_multiple_act =TRUE;

		if(newSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_UNTAG)
		{
			p24Data.out_svlan_id = 0U;
			p24Data.out_svid_format_act = FALSE;
			p24Data.out_spri_format_act = FALSE;
			p24Data.out_spri = 0U;
		}
		else if (newSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_TAG)
		{
			p24Data.out_svid_format_act = TRUE;
			p24Data.out_spri_format_act = TRUE;
			p24Data.out_spri = (*pNewActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			p24Data.out_svlan_id = (*pNewActField->pSvlanTCI&VLAN_VID_MASK);
		}
		
		//cvlan
		p24Data.out_egress_cvid_act = TRUE;
		p24Data.out_ctag_format_act = TRUE;
		if(newSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_UNTAG)
		{
			p24Data.out_cvlan_id = 0U;
			p24Data.out_cvid_format_act = FALSE;
			p24Data.out_cpri_format_act = FALSE;
			p24Data.out_cpri = 0U;
		}
		else if (newSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_TAG)
		{
			p24Data.out_cvid_format_act = TRUE;
			p24Data.out_cpri_format_act = TRUE;
			p24Data.out_cpri = (*pNewActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
			p24Data.out_cvlan_id = (*pNewActField->pCvlanTCI&VLAN_VID_MASK);
		}

		p24Data.out_dscp_act = newSwFlowAction->swFlowAction.bits.dscpCmd;
		p24Data.out_dscp = (*pNewActField->pTos) >>2 ;

		//setting Path4 PortMask and ExtPmsk	
		if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(newExtpmsk,&p24ExtPmskHwIdx)!=RTK_FC_RET_OK)
		{
			WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
			goto UPDATE_FAIL_SWONLY;
		}

		p24Data.out_portmask = p24Pmsk;
		p24Data.out_ext_portmask_idx = p24ExtPmskHwIdx;

		ret=rtk_fc_abstrHwflow_freeEntry_get(&path24Idx, _rtk_fc_sw_flowHashIndexStep2_get(path13Idx), *(rtk_fc_tableFlowEntry_t *)&p24Data, TRUE, TRUE,TRUE ); 
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			goto UPDATE_FAIL_SWONLY;

		ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path24Idx, &p24Data, NULL, 0), RTK_FC_RET_OK);
		TABLE("flowtlb[%d] add new-multi action  out_portmask=%x out_ext_portmask_idx=%x",path24Idx,p24Pmsk,p24ExtPmskHwIdx);

		newSwFlowAction->xponActPriv.hwacc=1U;
		newSwFlowAction->xponActPriv.hwIdx=path24Idx;
		newSwFlowAction->xponActPriv.isPath24=1U;
		
		//just for refer to swEntry
		fc_db.flowTbl[path24Idx].pAbstrSwFlowEt =pAbstrSwFlowEt; 
	}

	return RTK_FC_RET_OK;
	
UPDATE_FAIL_SWONLY:

	IGMP("update fail set sofware flow only and delete hardware");

	if(p135ExtPmskHwIdx!=FAIL)
		RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(p135ExtPmskHwIdx);
	
	if(p24ExtPmskHwIdx!=FAIL)
		RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(p24ExtPmskHwIdx);

	//clear all hw setting delete flow 
	rtk_fc_abstrSwSyncToHw_del(swFlowIdx);
	
	FLOW_INFO_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY, 1);
	if(pP135SwFlowAction)
		pP135SwFlowAction->xponActPriv.hwacc=0U;
	if(pP24SwFlowAction)
		pP24SwFlowAction->xponActPriv.hwacc=0U;
	
	fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->xponKeyHwPriv.hwacc=0U;

	return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;

}

//just delete mapping hw flow
//CONFIG_RTK_L34_XPON_PLATFORM
int rtk_fc_abstrSwSyncToHw_del(int32 swFlowIdx)
{

	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;

	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		if(pSwFlowAction->xponActPriv.hwacc)
		{
			//delete outside forloop
			if(pSwFlowAction->xponActPriv.hwIdx == pAbstrSwFlowEt->xponKeyHwPriv.flowP13hwIdx)
				continue;

			rtk_fc_abstrXPONHw_del(pSwFlowAction->xponActPriv.hwIdx);
			pSwFlowAction->xponActPriv.hwacc=0U;
		}
	}
	rtk_fc_abstrXPONHw_del(pAbstrSwFlowEt->xponKeyHwPriv.flowP13hwIdx);

	pAbstrSwFlowEt->xponKeyHwPriv.hwacc=0U;

	return RTK_FC_RET_OK;

}


//CONFIG_RTK_L34_XPON_PLATFORM
int rtk_fc_abstrSwSyncToHw_add(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int32 swFlowIdx)
{
	uint8 isIpv6 = 0U, l4Protocol = 0U, igrCVlanTagif = 0U, igrSVlanTagif = 0U, igrPPPoETagif = 0U;
	uint32 sip = 0U, dip = 0U;
	uint16 sport = 0U, dport = 0U, igrSVID = 0U, igrCVID = 0U;
	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);
	rtk_fc_abstrSwFlowActionField_entry_t *pActField,actField,*pSecActField,secActField;
	rtk_rg_asic_path1_entry_t flowEntry,flowEntryMultiAction;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL,*pFirstSwFlowAction=NULL,*pSecondSwFlowAction=NULL;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt;
	uint32 actionCount=0U;
	uint32 pmsk=0U,firstPmsk=0U,secondPmsk=0U;
	int32 _FirstExtPmskHwIdx=FAIL,_SecondExtPmskHwIdx=FAIL;
	rtk_fc_wlan_devmask_t wlanMsk=0ULL,first_wlanMsk = 0ULL, second_wlanMsk = 0ULL;
	rtk_fc_ext_port_mask_t firstExtpmsk = 0, secondExtpmsk = 0;
	int32 firstActHwIdx=FAIL,secondActHwIdx=FAIL;
	uint8 notTcam=FALSE,forceGet=FALSE;
	int32 ret;

	if(FLOW_INFO_IS_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;

	if(pAbstrSwFlowEt->xponKeyHwPriv.hwacc)
		return RTK_FC_RET_ERR_ENTRY_EXIST;

	isIpv6=pAbstrSwFlowEt->swFlowKey.bits.isIpv6;

	pActField=&actField;
	pSecActField=&secActField;

	//--------------For hardward not support behavior---------------------------------------------------

	list_for_each_entry(pSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		pmsk=0U;
		wlanMsk=0ULL;
		list_for_each_entry(pLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
		{

			if(pLdpid->isWlan)
			{
				if(wlanMsk& (((rtk_fc_wlan_devmask_t)1)<<pLdpid->flowLdpid))
					goto ADD_FAIL_SWONLY;
				else
					wlanMsk|=(((rtk_fc_wlan_devmask_t)1)<<pLdpid->flowLdpid);
			}
			else
			{
				if(pmsk& (1<<pLdpid->flowLdpid))
					goto ADD_FAIL_SWONLY;
				else
					pmsk|=(1<<pLdpid->flowLdpid);
			}
		}
		actionCount++;

		if(actionCount==1U)
		{
			rtk_fc_parseAbstrSwActionField((pPktHdr->ip6h) ? TRUE : FALSE,&pSwFlowAction->swFlowAction,pActField);
			pFirstSwFlowAction=pSwFlowAction;
			firstPmsk=pmsk;
			first_wlanMsk=wlanMsk;
			RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(first_wlanMsk, &firstExtpmsk);
		}
		else if(actionCount==2U)
		{
			rtk_fc_parseAbstrSwActionField((pPktHdr->ip6h) ? TRUE : FALSE,&pSwFlowAction->swFlowAction,pSecActField);
			pSecondSwFlowAction=pSwFlowAction;
			secondPmsk=pmsk;
			second_wlanMsk=wlanMsk;
			RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(second_wlanMsk, &secondExtpmsk);
		}
		else if (actionCount>=3U)
		{
			goto ADD_FAIL_SWONLY;
		}
	}


	//forward a copy to cpu 
	if((firstExtpmsk||secondExtpmsk) && pAbstrSwFlowEt->copy2cpu)
	{
		//not support hardware accelerate
		TRACE("Not support copy2cpu and send to wifi!");		
		goto ADD_FAIL_SWONLY;
	}
	else if(firstExtpmsk==0 && pAbstrSwFlowEt->copy2cpu)
	{
		firstPmsk|= (1<<RTK_FC_MAC_PORT_CPU);
	}


	if(pFirstSwFlowAction->swFlowAction.bits.smacTrans && 
		pPktHdr && 
		(pPktHdr->iph || pPktHdr->ip6h) && (pPktHdr->tcph || pPktHdr->udph) &&
		!(pFcIngressData->isDualHeader && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE ) && //Not down stream dual header
		!(!pFcIngressData->isDualHeader && pPktHdr->dualHdrType!=RTK_FC_DUALTYPE_NONE)  //Not up stream dual header
	)
	{
		dport =(pPktHdr->tcph)?ntohs(pPktHdr->tcph->dest):ntohs(pPktHdr->udph->dest);
		sport =(pPktHdr->tcph)?ntohs(pPktHdr->tcph->source):ntohs(pPktHdr->udph->source); 
		sip = (pPktHdr->iph)?ntohl(pPktHdr->iph->saddr):(pPktHdr->ipv6Sip_hash);
		dip = (pPktHdr->iph)?ntohl(pPktHdr->iph->daddr):(pPktHdr->ipv6Dip_hash);
		

		if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM &&  (dip != pFcIngressData->dstIp))
		{
			TRACE("Not support upstream change dip!");
			goto ADD_FAIL_SWONLY;
		}
		else if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM &&  (dport != pFcIngressData->dstPort))
		{
			TRACE("Not support upstream change dport!");
			goto ADD_FAIL_SWONLY;
		}
		else if(pPktHdr->direction== RTK_FC_FLOW_DIRECTION_DOWNSTREAM && (sip != pFcIngressData->srcIp))
		{
			TRACE("Not support downstream change sip!");
			goto ADD_FAIL_SWONLY;
		}
		else if(pPktHdr->direction== RTK_FC_FLOW_DIRECTION_DOWNSTREAM && (sport != pFcIngressData->srcPort))
		{
			TRACE("Not support downstream change sport!");
			goto ADD_FAIL_SWONLY;
		}
		
	}

	if ((firstPmsk&secondPmsk) || (firstExtpmsk&secondExtpmsk))
		goto ADD_FAIL_SWONLY;


	//Hardware not support DEI/CFI remarking
	if((pFcIngressData->ingressTagif & SVLAN_TAGIF) && pFirstSwFlowAction->swFlowAction.bits.stagCmd==SWFLOW_EGACT_TAG)
	{
		if(pFcIngressData->srcSVlanDei != (*pActField->pSvlanTCI&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT)
			goto ADD_FAIL_SWONLY;
	}

	if((pFcIngressData->ingressTagif & CVLAN_TAGIF) && pFirstSwFlowAction->swFlowAction.bits.ctagCmd==SWFLOW_EGACT_TAG)
	{
		if(pFcIngressData->srcCVlanCfi != (*pActField->pCvlanTCI&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT)
			goto ADD_FAIL_SWONLY;
	}

	//Hardware not support TOS/ECN remarking
	if(pFirstSwFlowAction->swFlowAction.bits.ecnCmd)
	{
		uint8  ipTos=0U;
		if(!isIpv6)
		{
			ipTos =  (*pActField->pTos);
			if((ipTos&0x3)!= (pFcIngressData->v4tos&0x3))
			{
				TRACE("ECN remarking not support");
				goto ADD_FAIL_SWONLY;
			}
		}
		else 
		{
			ipTos =  (*pActField->pTos);
			if((ipTos&0x3)!= (pFcIngressData->v6tos&0x3))
			{
				TRACE("ECN remarking not support");
				goto ADD_FAIL_SWONLY;
			}
			
		}
	}

	//--------------------------------------------------------------------------------------------------


	bzero(&flowEntry,sizeof(flowEntry));

	// VLAN
	if(pFcIngressData->ingressTagif & SVLAN_TAGIF)
		igrSVlanTagif = TRUE;
	if(pFcIngressData->ingressTagif & CVLAN_TAGIF)
		igrCVlanTagif = TRUE;
	if(pFcIngressData->ingressTagif & PPPOE_TAGIF)
		igrPPPoETagif = TRUE;

	igrSVID = igrSVlanTagif ? pFcIngressData->srcSVlanId : 0;
	igrCVID = igrCVlanTagif ? pFcIngressData->srcCVlanId : 0;

	l4Protocol = pPktHdr->tcph ? TRUE : FALSE;


	//path12 bridge_5tuple_flow_accelerate_by_2tuple or non-l4 packets
	if( fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple ||
	   (pPktHdr->tcph==NULL && pPktHdr->udph==NULL))
	{
		if( pPktHdr->abstrSwFlowFlooding )
		{
			rtk_rg_asic_path1_entry_t *pFlowPath1=(rtk_rg_asic_path1_entry_t*)&flowEntry;
			rtk_rg_asic_path2_entry_t *pFlowPath2=(rtk_rg_asic_path2_entry_t*)&flowEntryMultiAction;
			uint32 ethHwIdx;
			uint8 bcMac[6]={0xff,0xff,0xff,0xff,0xff,0xff};
			
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(firstExtpmsk,&_FirstExtPmskHwIdx)!=RTK_FC_RET_OK)
				return RTK_FC_RET_ERR;

			//ingress patten
			pFlowPath1->valid = TRUE;
			pFlowPath1->in_path = FB_PATH_12;
			pFlowPath1->in_multiple_act = FALSE;
			pFlowPath1->in_smac_lut_idx = pPktHdr->smacL2Idx;

			if (memcmp(&pPktHdr->eth->h_dest[0],bcMac,sizeof(bcMac))==0)
			{
				if(fc_db.bcMacIdx==FAIL){WARNING("BC Mac Not ready!");return RTK_FC_RET_ERR;}
				pFlowPath1->in_dmac_lut_idx = HW_BC_IDX;
			}
			else
			{
				if(pPktHdr->dmacL2Idx==FAIL){WARNING("Mac Not ready!");return RTK_FC_RET_ERR;}
				pFlowPath1->in_dmac_lut_idx = pPktHdr->dmacL2Idx;
			}
			
			pFlowPath1->in_svlan_id = igrSVID;
			pFlowPath1->in_cvlan_id = igrCVID;
			pFlowPath1->in_pppoe_sid = pFcIngressData->sessionId;
			pFlowPath1->in_spa_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_SPA];
			pFlowPath1->in_spa = pPktHdr->ingressPort.macPortIdx;
			pFlowPath1->in_ext_spa = pPktHdr->ingressPort.macExtPortIdx;


			if(RTK_RG_ASIC_ETHTYPE_GET_HW_IDX(pPktHdr->ethtype, &ethHwIdx)!=RTK_FC_RET_OK)
				return FAILED;
			else
				pFlowPath1->in_protocol = ethHwIdx;

			if((1<<pFlowPath1->in_spa) & fc_db.wanPortMask.portmask)
			{
			
				pFlowPath1->in_out_stream_idx_check_act = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_STREAM_IDX];
				pFlowPath1->in_out_stream_idx = pFcIngressData->ponStreamId;
			}
			else if(pFirstSwFlowAction->swFlowAction.bits.ponStreamIdCmd)
			{
				pFlowPath1->in_out_stream_idx_check_act = 1U;
				pFlowPath1->in_out_stream_idx = *pActField->pPonStreaId;
			}
			else
			{
				pFlowPath1->in_out_stream_idx_check_act = 0U;
				pFlowPath1->in_out_stream_idx = 0U;
			}

			pFlowPath1->in_intf_idx = pFirstSwFlowAction->igrIntf;
			pFlowPath1->in_stagif = igrSVlanTagif;
			pFlowPath1->in_ctagif = igrCVlanTagif;
			pFlowPath1->in_pppoeif = igrPPPoETagif;
			if(igrPPPoETagif)
				pFlowPath1->in_pppoe_sid_check = pFcIngressData->isDAGatewayMAC;
			pFlowPath1->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS];
			pFlowPath1->in_tos = isIpv6 ? pFcIngressData->v6tos : pFcIngressData->v4tos;
			pFlowPath1->in_cvlan_pri = pFcIngressData->srcCVlanPri;


			//-------------------------egress action-------------------------
			pFlowPath1->out_intf_idx = pFirstSwFlowAction->egrIntf;
			pFlowPath1->out_portmask = firstPmsk;
			pFlowPath1->out_ext_portmask_idx = _FirstExtPmskHwIdx;
			pFlowPath1->out_smac_trans = pFirstSwFlowAction->swFlowAction.bits.smacTrans;
			pFlowPath1->out_dmac_trans = 0U; //multicast dmac hardware will autoTrans to 01-00-5e-/33-33-
			
			//svlan
			pFlowPath1->out_egress_svid_act = TRUE;
			pFlowPath1->out_stag_format_act = TRUE;
			if(pFirstSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_UNTAG)
			{
				pFlowPath1->out_svlan_id = 0U;
				pFlowPath1->out_svid_format_act = FALSE;
				pFlowPath1->out_spri_format_act = FALSE;
				pFlowPath1->out_spri = 0U;
			}
			else if (pFirstSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_TAG)
			{
				pFlowPath1->out_svid_format_act = TRUE;
				pFlowPath1->out_spri_format_act = TRUE;
				pFlowPath1->out_spri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				pFlowPath1->out_svlan_id = (*pActField->pSvlanTCI&VLAN_VID_MASK);
			}
			
			//cvlan
			pFlowPath1->out_egress_cvid_act = TRUE;
			pFlowPath1->out_ctag_format_act = TRUE;
			if(pFirstSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_UNTAG)
			{
				pFlowPath1->out_cvlan_id = 0U;
				pFlowPath1->out_cvid_format_act = FALSE;
				pFlowPath1->out_cpri_format_act = FALSE;
				pFlowPath1->out_cpri = 0U;
			}
			else if (pFirstSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_TAG)
			{
				pFlowPath1->out_cvid_format_act = TRUE;
				pFlowPath1->out_cpri_format_act = TRUE;
				pFlowPath1->out_cpri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				pFlowPath1->out_cvlan_id = (*pActField->pCvlanTCI&VLAN_VID_MASK);
			}
			
			//other
			if(pSecondSwFlowAction)
				pFlowPath1->out_multiple_act = TRUE;


			if( pFirstSwFlowAction->swFlowAction.bits.userPriCmd)
			{
				pFlowPath1->out_user_pri_act = TRUE;
				pFlowPath1->out_user_priority = (*pActField->pUserPri)&0x7U;
			}
			else
				pFlowPath1->out_user_pri_act = FALSE;


			pFlowPath1->out_dscp_act = pFirstSwFlowAction->swFlowAction.bits.dscpCmd;
			pFlowPath1->out_dscp = (*pActField->pTos) >>2 ;
			
			pFlowPath1->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
			pFlowPath1->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;
			
			pFlowPath1->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
			pFlowPath1->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;


			if(pSecondSwFlowAction)
			{
				memcpy(pFlowPath2,pFlowPath1,sizeof(*pFlowPath2));
				//init path4 parameter
				pFlowPath2->out_smac_trans = pSecondSwFlowAction->swFlowAction.bits.smacTrans;
				pFlowPath2->in_multiple_act =TRUE;

				
				if(pSecondSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_UNTAG)
				{
					pFlowPath2->out_svlan_id = 0U;
					pFlowPath2->out_svid_format_act = FALSE;
					pFlowPath2->out_spri_format_act = FALSE;
					pFlowPath2->out_spri = 0U;
				}
				else if (pSecondSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_TAG)
				{
					pFlowPath2->out_svid_format_act = TRUE;
					pFlowPath2->out_spri_format_act = TRUE;
					pFlowPath2->out_spri = (*pSecActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					pFlowPath2->out_svlan_id = (*pSecActField->pSvlanTCI&VLAN_VID_MASK);
				}
				
				//cvlan
				pFlowPath2->out_egress_cvid_act = TRUE;
				pFlowPath2->out_ctag_format_act = TRUE;
				if(pSecondSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_UNTAG)
				{
					pFlowPath2->out_cvlan_id = 0U;
					pFlowPath2->out_cvid_format_act = FALSE;
					pFlowPath2->out_cpri_format_act = FALSE;
					pFlowPath2->out_cpri = 0U;
				}
				else if (pSecondSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_TAG)
				{
					pFlowPath2->out_cvid_format_act = TRUE;
					pFlowPath2->out_cpri_format_act = TRUE;
					pFlowPath2->out_cpri = (*pSecActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					pFlowPath2->out_cvlan_id = (*pSecActField->pCvlanTCI&VLAN_VID_MASK);
				}

				pFlowPath2->out_dscp_act = pSecondSwFlowAction->swFlowAction.bits.dscpCmd;
				pFlowPath2->out_dscp = (*pSecActField->pTos) >>2 ;

				
				//setting Path4 PortMask and ExtPmsk	
				if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(secondExtpmsk,&_SecondExtPmskHwIdx)!=RTK_FC_RET_OK)
				 goto ADD_FAIL_SWONLY;
				
				pFlowPath2->out_portmask = secondPmsk;
				pFlowPath2->out_ext_portmask_idx = _SecondExtPmskHwIdx;


			}

			forceGet=TRUE;
			notTcam = TRUE;

		}
		else
		{
			TRACE("we only support flooding type pakcets");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}


	}
	else if(pPktHdr->tcph || pPktHdr->udph) //paht34 only support tcp/udp accerelate
	{
		if(pPktHdr->isMulticast)
		{
			rtk_rg_asic_path3_entry_t *pFlowPath3=(rtk_rg_asic_path3_entry_t*)&flowEntry;
			rtk_rg_asic_path4_entry_t *pFlowPath4=(rtk_rg_asic_path4_entry_t*)&flowEntryMultiAction;
			
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(firstExtpmsk,&_FirstExtPmskHwIdx)!=RTK_FC_RET_OK)
				return RTK_FC_RET_ERR;

			//ingress patten
			pFlowPath3->valid = TRUE;
			pFlowPath3->in_path = FB_PATH_34;
			pFlowPath3->in_multiple_act = FALSE;
			pFlowPath3->in_ipv4_or_ipv6 = isIpv6;
			pFlowPath3->in_src_ipv4_addr =  pFcIngressData->srcIp;
			pFlowPath3->in_dst_ipv4_addr =  pFcIngressData->dstIp;
			pFlowPath3->in_l4_src_port = pFcIngressData->srcPort;
			pFlowPath3->in_l4_dst_port = pFcIngressData->dstPort;
			pFlowPath3->in_l4proto = l4Protocol;						//tcp: 1, udp: 0

			pFlowPath3->in_intf_idx = pFirstSwFlowAction->igrIntf;
			pFlowPath3->in_stagif = igrSVlanTagif;
			pFlowPath3->in_ctagif = igrCVlanTagif;
			pFlowPath3->in_pppoeif = igrPPPoETagif;
			if(igrPPPoETagif)
				pFlowPath3->in_pppoe_sid_check = pFcIngressData->isDAGatewayMAC;
			pFlowPath3->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS];
			pFlowPath3->in_tos = isIpv6 ? pFcIngressData->v6tos : pFcIngressData->v4tos;
			pFlowPath3->in_cvlan_pri = pFcIngressData->srcCVlanPri;

			//-------------------------egress action-------------------------
			pFlowPath3->out_intf_idx = pFirstSwFlowAction->egrIntf;
			pFlowPath3->out_portmask = firstPmsk;
			pFlowPath3->out_ext_portmask_idx = _FirstExtPmskHwIdx;
			pFlowPath3->out_smac_trans = pFirstSwFlowAction->swFlowAction.bits.smacTrans;
			pFlowPath3->out_dmac_trans = 0U; //multicast dmac hardware will autoTrans to 01-00-5e-/33-33-
			
			//svlan
			pFlowPath3->out_egress_svid_act = TRUE;
			pFlowPath3->out_stag_format_act = TRUE;
			if(pFirstSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_UNTAG)
			{
				pFlowPath3->out_svlan_id = 0U;
				pFlowPath3->out_svid_format_act = FALSE;
				pFlowPath3->out_spri_format_act = FALSE;
				pFlowPath3->out_spri = 0U;
			}
			else if (pFirstSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_TAG)
			{
				pFlowPath3->out_svid_format_act = TRUE;
				pFlowPath3->out_spri_format_act = TRUE;
				pFlowPath3->out_spri = (*pActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				pFlowPath3->out_svlan_id = (*pActField->pSvlanTCI&VLAN_VID_MASK);
			}
			
			//cvlan
			pFlowPath3->out_egress_cvid_act = TRUE;
			pFlowPath3->out_ctag_format_act = TRUE;
			if(pFirstSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_UNTAG)
			{
				pFlowPath3->out_cvlan_id = 0U;
				pFlowPath3->out_cvid_format_act = FALSE;
				pFlowPath3->out_cpri_format_act = FALSE;
				pFlowPath3->out_cpri = 0U;
			}
			else if (pFirstSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_TAG)
			{
				pFlowPath3->out_cvid_format_act = TRUE;
				pFlowPath3->out_cpri_format_act = TRUE;
				pFlowPath3->out_cpri = (*pActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
				pFlowPath3->out_cvlan_id = (*pActField->pCvlanTCI&VLAN_VID_MASK);
			}
			
			//other
			if(pSecondSwFlowAction)
				pFlowPath3->out_multiple_act = TRUE;


			if( pFirstSwFlowAction->swFlowAction.bits.userPriCmd)
			{
				pFlowPath3->out_user_pri_act = TRUE;
				pFlowPath3->out_user_priority = (*pActField->pUserPri)&0x7U;
			}
			else
				pFlowPath3->out_user_pri_act = FALSE;


			pFlowPath3->out_dscp_act = pFirstSwFlowAction->swFlowAction.bits.dscpCmd;
			pFlowPath3->out_dscp = (*pActField->pTos) >>2 ;
						

			if((1<<pFcIngressData->ingressPort) & fc_db.wanPortMask.portmask)
			{
			
				pFlowPath3->out_stream_idx_act = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH345_STREAM_IDX];
				pFlowPath3->out_stream_idx = pFcIngressData->ponStreamId;
			}
			else if(pFirstSwFlowAction->swFlowAction.bits.ponStreamIdCmd)
			{
				pFlowPath3->out_stream_idx_act = 1U;
				pFlowPath3->out_stream_idx = *pActField->pPonStreaId;
			}
			else
			{
				pFlowPath3->out_stream_idx_act = 0U;
				pFlowPath3->out_stream_idx = 0U;
			}

			
			pFlowPath3->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
			pFlowPath3->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;
			
			pFlowPath3->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
			pFlowPath3->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;


			if(pSecondSwFlowAction)
			{
				memcpy(pFlowPath4,pFlowPath3,sizeof(*pFlowPath4));
				//init path4 parameter
				pFlowPath4->out_smac_trans = pSecondSwFlowAction->swFlowAction.bits.smacTrans;
				pFlowPath4->in_multiple_act =TRUE;

				
				if(pSecondSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_UNTAG)
				{
					pFlowPath4->out_svlan_id = 0U;
					pFlowPath4->out_svid_format_act = FALSE;
					pFlowPath4->out_spri_format_act = FALSE;
					pFlowPath4->out_spri = 0U;
				}
				else if (pSecondSwFlowAction->swFlowAction.bits.stagCmd == SWFLOW_EGACT_TAG)
				{
					pFlowPath4->out_svid_format_act = TRUE;
					pFlowPath4->out_spri_format_act = TRUE;
					pFlowPath4->out_spri = (*pSecActField->pSvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					pFlowPath4->out_svlan_id = (*pSecActField->pSvlanTCI&VLAN_VID_MASK);
				}
				
				//cvlan
				pFlowPath4->out_egress_cvid_act = TRUE;
				pFlowPath4->out_ctag_format_act = TRUE;
				if(pSecondSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_UNTAG)
				{
					pFlowPath4->out_cvlan_id = 0U;
					pFlowPath4->out_cvid_format_act = FALSE;
					pFlowPath4->out_cpri_format_act = FALSE;
					pFlowPath4->out_cpri = 0U;
				}
				else if (pSecondSwFlowAction->swFlowAction.bits.ctagCmd == SWFLOW_EGACT_TAG)
				{
					pFlowPath4->out_cvid_format_act = TRUE;
					pFlowPath4->out_cpri_format_act = TRUE;
					pFlowPath4->out_cpri = (*pSecActField->pCvlanTCI&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
					pFlowPath4->out_cvlan_id = (*pSecActField->pCvlanTCI&VLAN_VID_MASK);
				}

				pFlowPath4->out_dscp_act = pSecondSwFlowAction->swFlowAction.bits.dscpCmd;
				pFlowPath4->out_dscp = (*pSecActField->pTos) >>2 ;

				
				//setting Path4 PortMask and ExtPmsk	
				if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(secondExtpmsk,&_SecondExtPmskHwIdx)!=RTK_FC_RET_OK)
				 goto ADD_FAIL_SWONLY;
				
				pFlowPath4->out_portmask = secondPmsk;
				pFlowPath4->out_ext_portmask_idx = _SecondExtPmskHwIdx;


			}

			forceGet=TRUE;
			notTcam = TRUE;

		}
		else
		{
			TRACE("path35 unicast send to pLdpid->flowLdpid or unknow uc/l2mc flooding not support yet!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
	}
	else
	{
		TRACE("Error not support yet!");
		return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
	}


	ret=rtk_fc_abstrHwflow_freeEntry_get(&firstActHwIdx, (pFcIngressData->flowHashIdx), *(rtk_fc_tableFlowEntry_t *)&flowEntry, pPktHdr->isMulticast, notTcam,forceGet);
	if(ret==RTK_FC_RET_ERR_FLOW_FULL)
		goto ADD_FAIL_SWONLY;
	if(pSecondSwFlowAction)
	{
		ret=rtk_fc_abstrHwflow_freeEntry_get(&secondActHwIdx, _rtk_fc_sw_flowHashIndexStep2_get(firstActHwIdx), *(rtk_fc_tableFlowEntry_t *)&flowEntryMultiAction, pPktHdr->isMulticast, notTcam,forceGet);	
		IGMP("secondActHwIdx=%d ret=%d",secondActHwIdx,ret);
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			goto ADD_FAIL_SWONLY;
	}

	//add hardware 
	ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&firstActHwIdx, (void *)&flowEntry, NULL, 0), RTK_FC_RET_OK);
	TABLE("add hardware flow to [%d]",firstActHwIdx);
	//fill hwprivate field
	pFirstSwFlowAction->xponActPriv.hwacc=1U;
	pFirstSwFlowAction->xponActPriv.hwIdx=firstActHwIdx;
	pFirstSwFlowAction->xponActPriv.isPath24=0U;
	
	//just for refer to swEntry
	fc_db.flowTbl[firstActHwIdx].pAbstrSwFlowEt =pAbstrSwFlowEt; 
	FLOW_INFO_SET(&fc_db.flowTbl[firstActHwIdx], FLOW_INFO_STATIC_ENTRY, 1);
	FLOW_INFO_SET(&fc_db.flowTbl[firstActHwIdx], FLOW_INFO_MC_ENTRY, 1);		

	//traifc entry
	fc_db.fb_hwFlow_validBitsArray[(firstActHwIdx >> 5)] |= (0x1 << (firstActHwIdx&0x1f));
	fc_db.flowTbl[firstActHwIdx].candidateState=CANDIDATE_STATE_READY;
	fc_db.flowTbl[firstActHwIdx].lutEgrDaIdx = fc_db.flowTbl[swFlowIdx].lutEgrDaIdx;
	fc_db.flowTbl[firstActHwIdx].lutIgrSaIdx = fc_db.flowTbl[swFlowIdx].lutIgrSaIdx;

	if(pSecondSwFlowAction)
	{
		//add hardware 
		ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&secondActHwIdx, (void *)&flowEntryMultiAction, NULL, 0), RTK_FC_RET_OK);
		TABLE("add hardware flow to second-act [%d]",secondActHwIdx);
		//fill hwprivate field
		pSecondSwFlowAction->xponActPriv.hwacc=1U;
		pSecondSwFlowAction->xponActPriv.hwIdx=secondActHwIdx;
		pSecondSwFlowAction->xponActPriv.isPath24=1U;
		
		//just for refer to swEntry
		fc_db.flowTbl[secondActHwIdx].pAbstrSwFlowEt =pAbstrSwFlowEt; 
		FLOW_INFO_SET(&fc_db.flowTbl[secondActHwIdx], FLOW_INFO_STATIC_ENTRY, 1);
		FLOW_INFO_SET(&fc_db.flowTbl[secondActHwIdx], FLOW_INFO_MC_ENTRY, 1);

		fc_db.fb_hwFlow_validBitsArray[(secondActHwIdx >> 5)] |= (0x1 << (secondActHwIdx&0x1f));
		fc_db.flowTbl[secondActHwIdx].candidateState=CANDIDATE_STATE_READY;
		fc_db.flowTbl[secondActHwIdx].lutEgrDaIdx = fc_db.flowTbl[swFlowIdx].lutEgrDaIdx;
		fc_db.flowTbl[secondActHwIdx].lutIgrSaIdx = fc_db.flowTbl[swFlowIdx].lutIgrSaIdx;		
	}

	pAbstrSwFlowEt->xponKeyHwPriv.hwacc=1U;
	pAbstrSwFlowEt->xponKeyHwPriv.flowP13hwIdx=firstActHwIdx;

	return RTK_FC_RET_OK;

ADD_FAIL_SWONLY:
	TRACE("RTK_FC_RET_ERR_CHIP_NOT_SUPPORT sofware flow only");

	if(_FirstExtPmskHwIdx!=FAIL)
		RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(_FirstExtPmskHwIdx);

	if(_SecondExtPmskHwIdx!=FAIL)
		RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(_SecondExtPmskHwIdx);

	FLOW_INFO_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY, 1);
	if(pFirstSwFlowAction)
		pFirstSwFlowAction->xponActPriv.hwacc=0U;
	if(pSecondSwFlowAction)
		pSecondSwFlowAction->xponActPriv.hwacc=0U;
	fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->xponKeyHwPriv.hwacc=0U;

	return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;


}
#endif



int rtk_fc_egress_abstrSwFlowDecision(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{
	int i,ret=RTK_FC_RET_ERR;
	int flowTypeSel=FAIL;
	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);
	int32 swFlowIdx=FAIL;
	uint32 forceGet=0U;
	rtk_fc_abstrSwFlowPattern_entry_t swFlowPattern;
	rtk_fc_abstrSwFlowActionList_entry_t *pTmpSwFlowAction=NULL,*pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowAction_entry_t action;
	int isWlan=0,ldpid=FAIL;
	rtk_fc_abstrSwFlowLdpid_entry_t *pTmpLdpid=NULL,*pLdpid=NULL;
	int8 addSwOnly=FALSE;
	int32 indMacIdx=FAIL,staticEntry=0,isKnownMulticast=0;
	int32 psFloodfwdAcc=pPktHdr->remarkDec.psFloodfwdAcc;
	rtk_fc_igrExtraInfo_t extraInfo;
 	bool if_verify_fdb = TRUE;
	int  saIdx = pPktHdr->smacL2Idx ;
	int16 igrNetifIdx = SIGNED_INVALID, egrNetifIdx = SIGNED_INVALID;
	int16 swIgrNetifIdx = SIGNED_INVALID, swEgrNetifIdx = SIGNED_INVALID;
	rtk_fc_forwardingType_t fwdType = pPktHdr->fwdType;

	rtk_fc_mcExtraSwFlowIdx_entry_t *mcEctSwFlowIdxTbl=NULL;
	uint8 PortChangeSync2Hw=0U;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	int filterStackingCnt=0;
	bool isIpv6=0;
	//Known IPMC 
	if(pPktHdr->isMulticast &&  (!psFloodfwdAcc))
		isKnownMulticast=TRUE;

	if(pPktHdr->iph)
		isIpv6=0;
	else if (pPktHdr->ip6h)
		isIpv6=1;

	if(isKnownMulticast)
	{
		staticEntry=1;
		//check user/auto mode config port
		if(pPktHdr->mcPortInConfig==0)
		{
			IGMP("mcPortInConfig=0 ingore abstrflow add");
			goto ABSTRADD_FAIL;
		}

	}


	// Netif
	if(pPktHdr->remarkDec.swIntfIdx_assign_en)
	{
		if(pPktHdr->remarkDec.swIntfIdx_assign_inOut == 0U)
		{
			swIgrNetifIdx = pPktHdr->remarkDec.swIntfIdx_assign_idx;
			TRACE("decide igrNetifIdx:%d by skbmark", swIgrNetifIdx);
		}
		else
		{
			swEgrNetifIdx = pPktHdr->remarkDec.swIntfIdx_assign_idx;
			TRACE("decide igrNetifIdx:%d by skbmark", swEgrNetifIdx);
		}
	}

	if(pPktHdr->dstDev)
	{

		TRACE("[Before] flow was forwarded to %s (%s)", pPktHdr->dstDev->name, (fwdType > RTK_FC_FWDTYPE_BRIDGE)?(fwdType==RTK_FC_FWDTYPE_ROUTING?"routing":"napt"):"bridge");

		if(swIgrNetifIdx == SIGNED_INVALID)
			swIgrNetifIdx = rtk_fc_getIngressSwNetifIdx(pPktHdr, rtskb->skb, fwdType, pPktHdr->direction);
		if(swEgrNetifIdx == SIGNED_INVALID)
			swEgrNetifIdx = rtk_fc_getEgressSwNetifIdx(pPktHdr, rtskb->skb, fwdType, pPktHdr->direction);

		if(swIgrNetifIdx==SIGNED_INVALID || swEgrNetifIdx==SIGNED_INVALID)
		{
			WARNING("[Fail to add flow] igrNetifIdx=%d egrNetifIdx=%d",swIgrNetifIdx, swEgrNetifIdx);
			goto ABSTRADD_FAIL;
		}
		else
			TRACE("[After] flow was forwarded from netifIdx %d to %d", swIgrNetifIdx, swEgrNetifIdx);
	}
	else
	{
		WARNING("no dev");
		goto ABSTRADD_FAIL;
	}
	if(fc_db.netifTbl[swIgrNetifIdx].hwIdx==SIGNED_INVALID || fc_db.netifTbl[swEgrNetifIdx].hwIdx==SIGNED_INVALID)
	{
		DEBUG("Hardware igrNetifIdxHW:%d  egrNetifIdxHW:%d not ready",fc_db.netifTbl[swIgrNetifIdx].hwIdx,fc_db.netifTbl[swEgrNetifIdx].hwIdx);
		goto ABSTRADD_FAIL;
	}
	igrNetifIdx = fc_db.netifTbl[swIgrNetifIdx].hwIdx;
	egrNetifIdx =  fc_db.netifTbl[swEgrNetifIdx].hwIdx;



	//2 SA find/learn
 	if(pPktHdr->abstrSwFlowFlooding) 				
		if_verify_fdb = FALSE;			// skip FDB/neighbor check for multicast server, the l2 entry will be configured as static after adding flow.
	if(fc_db.controlFuc.flow_skipAllPsTracking || fc_db.controlFuc.l2_skipPsTracking) if_verify_fdb = FALSE;			// force skip	

	if((saIdx == SIGNED_INVALID) || 																// SA needs to be learned (l2_macEgrLearning==1)
		(fc_db.lutTbl[saIdx] && (fc_db.lutTbl[saIdx]->isSync2PsEntry==FALSE) && (if_verify_fdb==TRUE))) {	// SA needs to be updated (l2_macEgrLearning==0)

		if((ret = _rtk_fc_lut_learning(pFcIngressData->sa, pPktHdr->ingressPort, pFcIngressData->srcCVlanId, pPktHdr->igrWlanDevIdx,
										if_verify_fdb, FALSE, &pPktHdr->smacL2Idx, pPktHdr,LUT_DIR_SA_LEARNING)) != RTK_FC_RET_OK)
		{
			TRACE("LUT SA learning is fail, ret = 0x%x", ret);
			goto ABSTRADD_FAIL;
		}
		saIdx = pPktHdr->smacL2Idx;
		if((pPktHdr->smacL2Idx != SIGNED_INVALID) && fc_db.lutTbl[pPktHdr->smacL2Idx])
			pPktHdr->smacHostPolIdx = fc_db.lutTbl[pPktHdr->smacL2Idx]->hostPolIdx;
	}

	if(unlikely((pFcIngressData->flowHashIdx < 0) || (pFcIngressData->flowHashIdx >= fc_db.flowSwTableSize)))
	{
		TRACE("Flow hash index[%d] is invalid", pFcIngressData->flowHashIdx);
		goto ABSTRADD_FAIL;
	}
	


	//2 DA find/learn
	if((pPktHdr->eth->h_dest[0]&1)==0)
	{
		uint16 vid;
#if defined(CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT)	
		if(fc_db.controlFuc.external_switch_en && pPktHdr->egressPort.macPortIdx == CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT)
		{
			vid = pPktHdr->cvlanid;
			pPktHdr->egressPort.macExtPortIdx = pPktHdr->svlanid - fc_db.external_switch_vlan_start;
		}
		else
#endif	
		{
			if(pPktHdr->svlanid!=0)
				vid = pPktHdr->svlanid;
			else
				vid = pPktHdr->cvlanid;
		}	
		// un-learned DA or needs to sync PS. Alway go to lut learning function to check port moving.
		if((ret = _rtk_fc_lut_learning(pPktHdr->eth->h_dest, pPktHdr->egressPort, vid, pPktHdr->egrWlanDevIdx,
										if_verify_fdb, FALSE, &pPktHdr->dmacL2Idx, pPktHdr,LUT_DIR_DA_LEARNING)) != RTK_FC_RET_OK) {
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
		}
		if((pPktHdr->dmacL2Idx != SIGNED_INVALID) && fc_db.lutTbl[pPktHdr->dmacL2Idx])
			pPktHdr->dmacHostPolIdx = fc_db.lutTbl[pPktHdr->dmacL2Idx]->hostPolIdx;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		// forbidden if PON_mode & to_WAN & !sid
		if(fc_db.systemGlobal.pon_sidTbl_en && 
			((pPktHdr->dmacL2Idx != SIGNED_INVALID) && fc_db.lutTbl[pPktHdr->dmacL2Idx]) && (fc_db.lutTbl[pPktHdr->dmacL2Idx]->spa == RTK_FC_MAC_PORT_PON) &&
			!pPktHdr->remarkDec.streamId_en) 
		{
			WARNING("egress to %s with DMAC %pM spa %d and no streamid assign",  pPktHdr->dstDev->name, &fc_db.lutTbl[pPktHdr->dmacL2Idx]->l2Addr[0], RTK_FC_MAC_PORT_PON);
			goto ABSTRADD_FAIL;
		}
#endif
	}


	//============= start create abstrSwflow =============

	for(i=ABSTR_SWFLOW_TYPE_SIZE-1;i>=0;i--)
	{
		if(!fc_db.abstrSwFlowType[i].valid)
			continue;		
		if(rtk_fc_abstrSwFlowPatternGenBySkb(pPktHdr,rtskb,&swFlowPattern,i)==SUCCESS)
		{
			//select first high pri flow type template
			flowTypeSel=i;
			break;
		}
	}
	if(flowTypeSel==FAIL)
	{
		TRACE("flowTypeSel fail skip learning");
		goto ABSTRADD_FAIL;
	}

	//alloc entry or get same entry
	ret =rtk_fc_abstrSwflow_freeEntry_get(&swFlowIdx,pFcIngressData->flowHashIdx,forceGet,&swFlowPattern);
	if(ret==RTK_FC_RET_ERR_FLOW_LRU)
	{
		if(unlikely(fc_db.fwdStatistic))
		{
			atomic_inc(&fc_db.statistic.perPortCnt_FlowLRU[pPktHdr->ingressPort.macPortIdx]);
		}
	}
	else if(ret==RTK_FC_RET_ERR_FLOW_FULL)
	{
		goto ABSTRADD_FAIL;
	}
	if(swFlowIdx==FAIL)
		goto ABSTRADD_FAIL;

	
	if(ret==RTK_FC_RET_ERR_FLOW_LRU || ret==RTK_FC_RET_OK)
	{
		bzero(&extraInfo,sizeof(extraInfo));
		extraInfo.lutEgrDaIdx = pPktHdr->dmacL2Idx;
		extraInfo.lutIgrSaIdx = pPktHdr->smacL2Idx;
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
		if(fc_db.controlFuc.flow_sessionAutoExtend && pFcIngressData->isDualHeader!=TRUE){				// dummy packet is not support for dual header
			rtk_fc_igrDummyData_t tmpData;
			bzero(&tmpData, sizeof(tmpData));
			extraInfo.dummyPkt = &tmpData;
			rtk_fc_flow_skbDataReasm(pFcIngressData, pPktHdr, (uint8 *)extraInfo.dummyPkt);
		}
#endif
		if(!staticEntry)
		{
			if(pPktHdr->remarkDec.swShaper_en)
			{
			   // if hw meter disable, force adding to sw flow
			   if(pPktHdr->remarkDec.meterIdx_en == 0U) 
				   addSwOnly = TRUE;
			   extraInfo.swShaperEn |= (1<<SWSHAPER_TYPE_FLOW);
			}
			if((pPktHdr->smacHostPolIdx != SIGNED_INVALID) && fc_db.hostPoliceTable[pPktHdr->smacHostPolIdx].ingress_limit_by_swShaper)
			{
			   addSwOnly = TRUE; // SW shaper for host policing
			   extraInfo.swShaperEn |= (1<<SWSHAPER_TYPE_SMAC_HOST);
			}
		
			if((pPktHdr->dmacHostPolIdx != SIGNED_INVALID) && fc_db.hostPoliceTable[pPktHdr->dmacHostPolIdx].egress_limit_by_swShaper)
			{
			   addSwOnly = TRUE; // SW shaper for host policing
			   extraInfo.swShaperEn |= (1<<SWSHAPER_TYPE_DMAC_HOST);
			}
		}

		
		ret = _rtk_fc_flow_swField_set(swFlowIdx, pFcIngressData->flowHashIdx, NULL, &extraInfo, indMacIdx, addSwOnly, staticEntry,isKnownMulticast, NULL, pPktHdr);
		if(ret==RTK_FC_RET_OK)
		{
			TABLE("Add %s flow entry [%d],","Sw", swFlowIdx);
		}
		else
		{
			WARNING("unexpect setting error");
			return FAILED;
		}

		fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->swFlowIdx=swFlowIdx;
		fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->copy2cpu = pPktHdr->abstrFlowCopy2cpu;
		fc_db.flowTbl[swFlowIdx].candidateState = CANDIDATE_STATE_READY;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		fc_db.flowTbl[swFlowIdx].crc16 = pFcIngressData->flowHashIdx;
		fc_db.flowTbl[swFlowIdx].crc32 = pFcIngressData->crc32;
#endif

		if(isKnownMulticast)
		{
			mcEctSwFlowIdxTbl=rtk_fc_igmp_mcExtraFlowIdxTbl_add(pFcIngressData,pPktHdr,rtskb,swFlowIdx,pPktHdr->mcIsGrpDftFwd);
			if(mcEctSwFlowIdxTbl==NULL)
				goto ABSTRADD_FAIL;
		}
		
	}

	

	ret = rtk_fc_abstrSwFlowActGenBySkb(pPktHdr, rtskb,&action);
	if (ret!=RTK_FC_RET_OK)
	{
		TRACE("rtk_fc_abstrSwFlowActGenBySkb Fail skiplearning ret=%d",ret);
		goto ABSTRADD_FAIL;
	}
			
	//*search same action in data base
	list_for_each_entry(pTmpSwFlowAction, &fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
	{
		if(rtk_fc_abstrSwFlowActionCompare(isIpv6,&pTmpSwFlowAction->swFlowAction,&action)==TRUE)
		{
			pSwFlowAction=pTmpSwFlowAction;
			TRACE("found old same action entry");
			break;
		}
	}

	if(pSwFlowAction==NULL)
	{
		TRACE("alloc new action");
		pSwFlowAction = rtk_fc_abstrSwFlowActionAlloc(&action);
		if(pSwFlowAction==NULL)
			goto ABSTRADD_FAIL;
		//list add to swFlowActionHdr
		list_add_tail(&pSwFlowAction->swFlowActionList,&fc_db.flowTbl[swFlowIdx].pAbstrSwFlowEt->swFlowActionHdr);
	}
	pSwFlowAction->igrIntf = igrNetifIdx;
	pSwFlowAction->egrIntf = egrNetifIdx;

	pSwFlowAction->flowMibEn = pPktHdr->remarkDec.mibIdx_en;
	pSwFlowAction->flowMibIdx =  pPktHdr->remarkDec.mibIdx;;

	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) 
	{
		isWlan = 1;
		ldpid = pPktHdr->egrWlanDevIdx;
	}
	else 
	{
		ldpid = pPktHdr->egressPort.macPortIdx;
	}

	for(i=0 ; i<DEV_STACK_MAX ;i++)
	{
		p_PScacheDev[i] = rtk_fc_getDev_by_fcDevIdx(pPktHdr->logicalOutDevIfidx[i]);

		if(p_PScacheDev[i])
			filterStackingCnt++;
	}

	//*search same lidid
	list_for_each_entry(pTmpLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
	{
		int devIfidx;
		if(filterStackingCnt)
			devIfidx = p_PScacheDev[filterStackingCnt-1]->ifindex;		// the final device
		else
			devIfidx = pPktHdr->dstDev->ifindex;
			
		if( pTmpLdpid->isWlan==isWlan && pTmpLdpid->flowLdpid==ldpid &&  pTmpLdpid->devIfIdx==devIfidx)
		{
			TRACE("found old pLdpid action entry");
			pLdpid=pTmpLdpid;
			break;
		}
	}

	//*alloc ldpid struct
	if(pLdpid==NULL)
	{
		TRACE("alloc new Ldpid");
		pLdpid = rtk_fc_abstrSwFlowLdpidAlloc();
		if(pLdpid==NULL)
			goto ABSTRADD_FAIL;
		pLdpid->isWlan = isWlan;
		pLdpid->flowLdpid = ldpid;

		//we want get logical dest device
		if(filterStackingCnt)
			pLdpid->devIfIdx =  p_PScacheDev[filterStackingCnt-1]->ifindex;		// the final device
		else
			pLdpid->devIfIdx =  pPktHdr->dstDev->ifindex;

		//list add to ldpidListHdr
		list_add_tail(&pLdpid->ldpidList,&pSwFlowAction->ldpidListHdr);
		PortChangeSync2Hw=1;
	}

{
	int tagsLen = 0;				// assume ingress with no any tags,  get how many 4bytes we needed to +/- according to egress skb len.

	if(pPktHdr->svh) tagsLen-=4;
	if(pPktHdr->cvh) tagsLen-=4;
	if(pPktHdr->ppph) tagsLen-=8;
	if(pFcIngressData->ingressTagif & SVLAN_TAGIF) tagsLen+=4;
	if(pFcIngressData->ingressTagif & CVLAN_TAGIF) tagsLen+=4;
	if(pFcIngressData->ingressTagif & PPPOE_TAGIF) tagsLen+=8;

	pPktHdr->skbLen_ingress = RTSKB_LEN(rtskb) + tagsLen;
	pPktHdr->flowIdx = swFlowIdx; // for later flow mib update
}


	if( fc_db.controlFuc.hwnat_mode == RT_FLOW_HWNAT_MODE_SW_ONLY || FLOW_INFO_IS_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
		addSwOnly = TRUE;
	if(pPktHdr->isHitSwFwdedAclTrapPri) 
		addSwOnly=TRUE;


	if(addSwOnly)
	{
		TRACE("flow is software only entry stop sync to hardware");
		FLOW_INFO_SET(&fc_db.flowTbl[swFlowIdx], FLOW_INFO_SOFTWARE_ONLY, TRUE);
	}
	else
	{
		if(PortChangeSync2Hw)
		{
			TRACE("start to sync to hardware");
			rtk_fc_abstrSwSyncToHw(rtskb,pPktHdr,swFlowIdx);
			if(isKnownMulticast)
			{
				rtk_fc_igmp_setupmcDummyPktDetectorTimer();
			}
		}
	}

	
	return SUCCESS;

ABSTRADD_FAIL:


	if(swFlowIdx!=FAIL)
	{
		ASSERT_EQ(rtk_fc_flow_delete(swFlowIdx), RTK_FC_RET_OK);
		rtk_fc_igmp_mcExtraFlowIdxTbl_del(swFlowIdx);
	}

	return FAIL;
	
}




#if defined(CONFIG_FC_RTL9607C_SERIES)
static inline int RTK_FC_NPTV6_SRAM_DRAM_CHECK(rtk_fc_pktHdr_t *pPktHdr, bool swOnly, bool isL4)
{
	if(fc_db.controlFuc.special_fast_forward_mode==1U && 
	   fc_db.controlFuc.hwnat_customize==1U &&
	   pPktHdr->isV6_NAT &&
	   swOnly !=1 &&
	   (dynamic_sram_desc==1U || dynamic_sram_desc==0U) && 
	   isL4 == TRUE
	 )
	 	return TRUE;
	else
		return FALSE;
}

static inline int RTK_FC_NPTV6_HWLOOKUP_FLOW_CHECK(rtk_fc_pktHdr_t *pPktHdr, bool swOnly, uint8 flow_can_add, bool isL4)
{
#if defined(CONFIG_FC_RTL9607C_SERIES)
	if( fc_db.controlFuc.special_fast_forward_mode==1U&&
		fc_db.controlFuc.hwnat_customize &&
		fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2 &&
		fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple==0 &&
		pPktHdr->isV6_NAT ==1U &&
		swOnly !=1 &&
		flow_can_add == 1U && 
		isL4 == TRUE
	)
		return TRUE;
	else if(fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple==1)
	{
		TABLE("Please disable bridge_5tuple_flow_accelerate_by_2tuple to set special fast forward.");
		return FALSE;
	}
	else
		return FALSE;
#else
	return FALSE;
#endif

}

static int _rtk_fc_nptv6_hwlookup_flow_processing(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, 
														  int igrNetifIdx, int egrNetifIdx, rtk_fc_igrExtraInfo_t *pExtraInfo, struct rt_nfconn *rtct, uint32 flowIdx,
														  rtk_rg_asic_path5_entry_t *tmp_pFlowPath5, rtk_fc_g3IgrExtraInfo_t *tmp_pG3IgrExtraInfo,
														  rtk_fc_ingress_data_t *pFcIngressData)
{
	int ret = 0;
	int outer_flow_index=-1;
	int outer_flow_hash_index =-1;
	rtk_fc_pktHdr_t tmp_outer_pktHdr;
	rtk_rg_asic_path3_entry_t *pFlowPath3;
	rtk_rg_asic_path1_entry_t tmp_flowEntry;
	int sw_flow_index=0;

	pFlowPath3 = (rtk_rg_asic_path3_entry_t *)&tmp_flowEntry;


	memcpy(&tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));

	tmp_outer_pktHdr.svh = NULL;
	tmp_outer_pktHdr.cvh = NULL;
	tmp_outer_pktHdr.ppph = NULL;
	tmp_outer_pktHdr.dmacToGatewayMAC = 0U;
	  
	outer_flow_hash_index = _rtk_fc_flow_hashIndex_byPktHdr(&tmp_outer_pktHdr);
	pG3IgrExtraInfo->nptv6_hwlookup_flow =1;

	DEBUG("[NPTv6]Outer flow hash = %d",outer_flow_hash_index);
	if(ret == RTK_FC_RET_OK && flowIdx <fc_db.flowHwTableSize)
	{
		//NPTv6 inner add to hw flow is done, time to add outer flow.
	  	rtk_fc_nptv6_fastFwd_outerFlow_prepare(pFlowPath3,
										  outer_flow_hash_index, 
										  pPktHdr, 
										  rtskb, 
										  igrNetifIdx, 
										  egrNetifIdx,
										  pExtraInfo,
										  pG3IgrExtraInfo,
										  rtct);
	  	ret = rtk_fc_flow_add(&outer_flow_index, outer_flow_hash_index, &tmp_flowEntry, NULL,
											  pExtraInfo, 0, pPktHdr->ingressPort, 0, 1, pG3IgrExtraInfo, pPktHdr);
	  
	  	if(ret!=RTK_FC_RET_OK)
	  	{
		  	if(ret==RTK_FC_RET_ERR_FLOW_FULL)
				TRACE("[Fail to add flow] Flow table is full.");
		  	else
			  	WARNING("[Fail to add flow] ret=%d", ret);
		  	ret = -1;
	  	}
	  	else
	  	{
		  	DEBUG("[NPTv6]Outer flow index = %d",outer_flow_hash_index);
	  	}
		}
	else
	 	DEBUG("[NPTv6]inner collision!Sw only!");

	if(dynamic_sram_desc==0U && tmp_pG3IgrExtraInfo && tmp_pFlowPath5)
	{
	
		tmp_pG3IgrExtraInfo->nptv6_info_index = pG3IgrExtraInfo->nptv6_info_index;
		pPktHdr->nptv6_outerFlow = 0U;
		DEBUG("tmp_pG3IgrExtraInfo->nptv6_info_index: %d",tmp_pG3IgrExtraInfo->nptv6_info_index);
		//Add another sw-flow for tcp trap
		ret = rtk_fc_flow_add(&sw_flow_index, pFcIngressData->flowHashIdx, (rtk_rg_asic_path1_entry_t *)tmp_pFlowPath5, NULL,
													pExtraInfo, 0, pPktHdr->ingressPort, 1, 1, tmp_pG3IgrExtraInfo, pPktHdr);

		if(ret!=RTK_FC_RET_OK)
		{
			if(ret==RTK_FC_RET_ERR_FLOW_FULL)
				TRACE("[NPTv6][SW FLOW]FLOW TABLE is FULL");
			else
				WARNING("[NPTv6][SW FLOW]ret=%d", ret);
		}
		else
		{
			DEBUG("[NPTv6]SW flow index = %d",sw_flow_index);
			
			// if(tmp_pFlowPath5)  RTK_FC_HELPER_FC_KFREE(tmp_pFlowPath5, sizeof(rtk_rg_asic_path5_entry_t));
			// if(tmp_pG3IgrExtraInfo) RTK_FC_HELPER_FC_KFREE(tmp_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
		}


	}
	
	return ret;
}


static void _rtk_fc_nptv6_sram_dram_preprocessing(uint8 *flow_can_add, rtk_rg_asic_path1_entry_t *flowEntry, rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb,
													int flowHashIdx, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int igrNetifIdx, int egrNetifIdx, 
													rtk_fc_igrExtraInfo_t *extraInfo, struct rt_nfconn *rtct, bool *swOnly, uint8 egrSVlanTagif,
													rtk_rg_asic_path5_entry_t *tmp_pFlowPath5, rtk_fc_g3IgrExtraInfo_t *tmp_pG3IgrExtraInfo)
{
	  int direction;
	  rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)flowEntry;
	  int indMacIdx = pFlowPath5->out_dmac_idx;
	  int dmacIdx = -1;
	  int tmp_outer_hashIdx =-1;
	  rtk_fc_pktHdr_t tmp_outer_pktHdr;
	  int flow_exist = 0;
	  int ret;

	  // For checking whether two flows can be added---------------------------------------
	  memcpy(&tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));
	  
	  tmp_outer_pktHdr.svh = NULL;
	  tmp_outer_pktHdr.cvh = NULL;
	  tmp_outer_pktHdr.ppph = NULL;
	  tmp_outer_pktHdr.dmacToGatewayMAC = 0U;
		  
	  tmp_outer_hashIdx = _rtk_fc_flow_hashIndex_byPktHdr(&tmp_outer_pktHdr);
	  
	  if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		  direction = RTK_FC_V6NAT_DIRECTION_UPSTREAM;
	  else
		  direction = RTK_FC_V6NAT_DIRECTION_DOWNSTREAM;
	  
	  rtk_fc_nptv6_check_flow_canBeAdded(flow_can_add, direction, &flowEntry, flowHashIdx, tmp_outer_hashIdx,
													   pG3IgrExtraInfo ,
													   pPktHdr, 
													   rtskb, igrNetifIdx, egrNetifIdx, extraInfo, rtct, &flow_exist);
	  // --------------------------------------------------------------------------------------
	  if(flow_can_add==0)
	  {
		  *swOnly = TRUE;
	  }
	  else if(flow_exist)
	  {
		  DEBUG("[NPTv6]Flow exist, by-pass checking sram/dram mechanism.");
	  }
	  else
	  {
		  	_rtk_fc_nptv6_preTXdesc_set(pPktHdr, rtskb );
	  		if(dynamic_sram_desc==0)
			{	
				// tmp_pFlowPath5 = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_rg_asic_path5_entry_t), GFP_ATOMIC);
				// tmp_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
				memcpy(tmp_pFlowPath5, pFlowPath5, sizeof(rtk_rg_asic_path5_entry_t));
				memcpy(tmp_pG3IgrExtraInfo, pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
			}
			if(dynamic_sram_desc)
			{
				DEBUG("NPTv6 SRAM MODE!!");
				_rtk_fc_nptv6_acceleration_mechanism_sramMode_check(pPktHdr, pFlowPath5, indMacIdx, direction, rtskb, &dmacIdx);
			}
			else
			{
				DEBUG("NPTv6 DRAM MODE!!");
				ret = _rtk_fc_nptv6_acceleration_mechanism_dramMode_check(pPktHdr, pFlowPath5, indMacIdx, direction, rtskb, &dmacIdx);
				if(ret != SUCCESS)
				{	 
					*swOnly = TRUE;
					return;
				}
			}

			if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			{
				pFlowPath5->valid = TRUE; // Add to hw
				// Force untag
				pFlowPath5->out_egress_svid_act = egrSVlanTagif ? TRUE : FALSE;
				pFlowPath5->out_svlan_id = 0U;
				pFlowPath5->out_stag_format_act = TRUE;
				pFlowPath5->out_svid_format_act = FALSE;
				pFlowPath5->out_spri_format_act = FALSE;
				pFlowPath5->out_spri = 0U;
				  
				//cvlan
				pFlowPath5->out_egress_cvid_act = TRUE;
				pFlowPath5->out_cvlan_id =  0U;
				pFlowPath5->out_ctag_format_act = TRUE;
				pFlowPath5->out_cvid_format_act = FALSE;
				pFlowPath5->out_cpri_format_act = FALSE;
				pFlowPath5->out_cpri = 0U;
			}
			else
			{
				  
				pFlowPath5->valid = TRUE; // Add to hw
			}
	  }

}
#endif

void _rtk_fc_flow_wan_downstream_process(rtk_fc_igrExtraInfo_t *extraInfo, rtk_rg_asic_path1_entry_t *flowEntry, rtk_fc_g3IgrExtraInfo_t *g3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, uint32 *flowIdx,
												  rtk_fc_ingress_data_t *pFcIngressData, struct rt_nfconn *rtct, bool swOnly, uint8 staticEntry, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo)
{
	uint32 revflowIdx = FAILED;
	int revRet=RTK_FC_RET_ERR, revHashIdx;
	rtk_rg_asic_path1_entry_t RevFlowEntry;
	rtk_fc_igrExtraInfo_t revExtraInfo;
	rtk_fc_g3IgrExtraInfo_t revG3IgrExtraInfo;
	int ret = 0;
	
	_rtk_fc_flow_wan_downstream_loopback_flow_prepare(flowEntry, &RevFlowEntry, &revHashIdx, extraInfo, &revExtraInfo, g3IgrExtraInfo, &revG3IgrExtraInfo, pPktHdr);
	ret = rtk_fc_flow_add(flowIdx, pFcIngressData->flowHashIdx, flowEntry, rtct,
							extraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, swOnly, staticEntry, pG3IgrExtraInfo, pPktHdr);
	if((ret==RTK_FC_RET_OK) && (revHashIdx != SIGNED_INVALID))
	{
		//add loopback reverse flow
		TRACE("wanDsLoopback_en is enabled, add its reverse flow");
		revRet = rtk_fc_flow_add(&revflowIdx, revHashIdx, &RevFlowEntry, NULL,
							&revExtraInfo, pPktHdr->isMulticast, pPktHdr->egressPort/*egress port is reverse as ingress port*/, 
							swOnly, 1 /*static*/, &revG3IgrExtraInfo, NULL/*pPktHdr only for v6 NAT*/);
		if(revRet == RTK_FC_RET_OK) {
			fc_db.flowTbl[*flowIdx].protoCtrl = FLOW_PROTO_CTRL_LOOPBACK_ACC;
			fc_db.flowTbl[*flowIdx].loopbackRevFlowIdx = revflowIdx;
		}
	}
}
												  
int	_rtk_fc_nptv6_nat66_check(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *pFcIngressData, uint8 *isNPT)
{
	int i,ret;
	
	for(i = DEVIFIDX_VALID_MIN; i< RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev==NULL)
			continue;

		if(!pPktHdr)return FAILED;
		if(!pPktHdr->ip6h)return FAILED;

		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			ret = RTK_FC_HELPER_NETDEV_COMPARE_V6ADDR_BY_V6ADDR(fc_db.devGwMacTbl[i].dev, &pPktHdr->ip6h->saddr);
		else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
			ret = RTK_FC_HELPER_NETDEV_COMPARE_V6ADDR_BY_V6ADDR(fc_db.devGwMacTbl[i].dev, &pFcIngressData->ingress_v6ip_info);
		else
			return FAILED;
		
		if(ret == SUCCESS)
		{
			*isNPT = 0U;
			return SUCCESS;
		}
		else
			*isNPT = 1U;
	}

	return SUCCESS;
}


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

int _rtk_fc_nptv6_prefixIdx_get(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr)
{
	hash_npt6_prefix_addr_entry_t prefix;
	uint32_t prefix_idx;
	int aal_ret;
	uint32 v6ip[4];
	if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
	 	//Upstream
	 	v6ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.in6_u.u6_addr32[0]);
		v6ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.in6_u.u6_addr32[1]);
		v6ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.in6_u.u6_addr32[2]);
		v6ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Saddr.in6_u.u6_addr32[3]);

		memset(&prefix, 0, sizeof(prefix));
		prefix.ipv6_prefix = ((uint64_t)v6ip[0] << 32) | v6ip[1];
		prefix.ipv6_prefix &= (0xffffffffffffffff - ((1 << (64-pG3IgrExtraInfo->naptv6_info.wan_prefix_len))-1));
		TRACE("[NPTv6]wan_prefix= 0x%llx, wan_prefix_len = %d", prefix.ipv6_prefix, pG3IgrExtraInfo->naptv6_info.wan_prefix_len);

		aal_ret = aal_hash_npt6_prefix_addr_entry_add(0, &prefix, &prefix_idx);
		if(aal_ret)
		{
			WARNING("add lan prefix entry failed, ret = %d", aal_ret);
			return aal_ret;
		}
		DEBUG("[NPTv6][US]prefix_idx=  %d",prefix_idx);
		pG3IgrExtraInfo->naptv6_info.prefix_idx = prefix_idx;
	}
	else
	{
		//Downstream
		
		v6ip[0] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.in6_u.u6_addr32[0]);
		v6ip[1] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.in6_u.u6_addr32[1]);
		v6ip[2] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.in6_u.u6_addr32[2]);
		v6ip[3] = ntohl(*(uint32*)&pG3IgrExtraInfo->ipv6_Daddr.in6_u.u6_addr32[3]);

		memset(&prefix, 0, sizeof(prefix));
		prefix.ipv6_prefix = ((uint64_t)v6ip[0] << 32) | v6ip[1];
		prefix.ipv6_prefix &= (0xffffffffffffffff - ((1 << (64-pG3IgrExtraInfo->naptv6_info.lan_prefix_len))-1));
		TRACE("[NPTv6]lan_prefix= 0x%llx, wan_prefix_len = %d", prefix.ipv6_prefix, pG3IgrExtraInfo->naptv6_info.lan_prefix_len);

		aal_ret = aal_hash_npt6_prefix_addr_entry_add(0, &prefix, &prefix_idx);
		if(aal_ret)
		{
			WARNING("add wan prefix entry failed, ret = %d", aal_ret);
			return aal_ret;
		}
		DEBUG("[NPTv6][US]prefix_idx=  %d",prefix_idx);
		pG3IgrExtraInfo->naptv6_info.prefix_idx = prefix_idx;
	}
	
	return SUCCESS;
}
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

int rtk_fc_store_flow_info_by_ct(struct rt_nfconn *rtct, uint32 flow_index, rtk_fc_pktHdr_t *pPktHdr, int flow_hash_index)
{
	uint32 ct_hash = 0U, find = 0U;
	rtk_fc_ct_hash_info_t *ct_hash_info;
	rtk_fc_ct_hash_info_t *pTmp_ct_hash_list, *pTmp_next_ct_hash_list;
		
	if(rtct ==NULL)
		return FAILED;

	if(rtct->ct == NULL)
		return FAILED;


	DEBUG("(uint32)(uintptr_t)rtct->ct = %x", (uint32)(uintptr_t)rtct->ct);
	ct_hash = (((uint32)(uintptr_t)rtct->ct) >> 12)&0xff; // should be 0~255

	DEBUG("ct_hash = %d", ct_hash);

	if(!list_empty(&fc_db.ct_hash_info_ListHead[ct_hash]))
	{
		list_for_each_entry_safe(pTmp_ct_hash_list, pTmp_next_ct_hash_list, &fc_db.ct_hash_info_ListHead[ct_hash], ct_hash_list)	//just return the first entry right behind of head
		{
			if(pTmp_ct_hash_list->ct == rtct->ct)
			{
				DEBUG("Find!!");
				if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
				{
					DEBUG("[DS]Cached upstream flow index = %d ct = %p",pTmp_ct_hash_list->upstream_flow_index, pTmp_ct_hash_list->ct);
					pTmp_ct_hash_list->downstream_flow_index = flow_index;
					pTmp_ct_hash_list->downstream_flow_hash_index = flow_hash_index;
					find = 1U;
				}
				else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
				{
					DEBUG("[US]Cached downstream flow index = %d ct = %p",pTmp_ct_hash_list->downstream_flow_index, pTmp_ct_hash_list->ct);
					pTmp_ct_hash_list->upstream_flow_index = flow_index;
					pTmp_ct_hash_list->upstream_flow_hash_index = flow_hash_index;
					find = 1U;
				}
				else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_LANBRIDGE)
				{
					DEBUG("[BRIDGE]Cached upstream flow index = %d, downstream flow index = %d ,ct = %p",pTmp_ct_hash_list->upstream_flow_index,pTmp_ct_hash_list->downstream_flow_index, pTmp_ct_hash_list->ct);
					
					if(pTmp_ct_hash_list->upstream_flow_index !=0 && pTmp_ct_hash_list->upstream_flow_index != flow_index)
					{
						pTmp_ct_hash_list->downstream_flow_index =flow_index;
						pTmp_ct_hash_list->downstream_flow_hash_index =flow_hash_index;
					}
					find = 1U;
					//ct_hash_info->ct = rtct->ct;
				}
				break;
			}
		}
	}

	if(find == 0U)
	{
		ct_hash_info = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_ct_hash_info_t), GFP_ATOMIC);
		memset(ct_hash_info,0,sizeof(rtk_fc_ct_hash_info_t));
		INIT_LIST_HEAD(&ct_hash_info->ct_hash_list);
		
		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
		{
			ct_hash_info->downstream_flow_index = flow_index;
			ct_hash_info->downstream_flow_hash_index = flow_hash_index;
			ct_hash_info->ct = rtct->ct;
			DEBUG("DOWNSTREAM! flow_index = %d",flow_index);
		}
		else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			ct_hash_info->upstream_flow_index = flow_index;
			ct_hash_info->upstream_flow_hash_index = flow_hash_index;
			ct_hash_info->ct = rtct->ct;
			DEBUG("UPSTREAM! flow_index = %d",flow_index);
		}
		else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_LANBRIDGE)
		{
			
			ct_hash_info->upstream_flow_index = flow_index;
			ct_hash_info->upstream_flow_hash_index = flow_hash_index;
			ct_hash_info->ct = rtct->ct;
			DEBUG("BRIDGE! flow_index = %d",flow_index);
		}
		list_add_tail(&ct_hash_info->ct_hash_list, &fc_db.ct_hash_info_ListHead[ct_hash]);
	}
	return SUCCESS;
	
}
#endif
int _rtk_fc_flow_ct_decision(struct rt_nfconn *rtct,  rtk_fc_pktHdr_t *pPktHdr, rt_tcp_conntrack_state_t *rt_tcp_state, bool *swOnly)
{
	uint8 protonum = 0U;
	if(rtct->ct)
	{

		RTK_FC_HOOK_PS_CT_PROTONUM_GET(rtct->ct, &protonum);
		RTK_FC_HELPER_PS_CT_RT_TCP_STATE_GET(rtct->ct, rt_tcp_state);

		DEBUG("ct status : 0x%lx use %d", rtct->status, atomic_read(&rtct->ct->ct_general.use));	// refer to ip_conntrack_status (Nf_conntrack_common.h)
		// TCP state control
		if((pPktHdr->tcph) && (protonum == IPPROTO_TCP))
		{
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
			if(!fc_db.tcp_in_window_shortcut_check)
#endif
				RTK_FC_HELPER_PS_CT_TCP_LIBERAL_SET(rtct->ct);

			if(((*rt_tcp_state>=RT_TCP_CONNTRACK_FIN_WAIT) && (*rt_tcp_state<=RT_TCP_CONNTRACK_CLOSE)) || (*rt_tcp_state==RT_TCP_CONNTRACK_MAX))
			{
				//TCP state is going to be closed, not add to flow table
				TRACE("TCP connection is going to be closed, skip learning");
				return FAILED;
			}

		}

		// Helper control
		if(!pPktHdr->remarkDec.skip_fc_alg_check)
		{
			struct nf_conntrack_helper *helper=NULL;

			RTK_FC_HELPER_RCU_READ_LOCK();
			RTK_FC_HOOK_PS_CT_HELPER_EXIST_CHECK(rtct->ct, &helper);
			if (helper != NULL){
				if(strlen(helper->name)){
					if(RTK_FC_HOOK_PS_CT_HELPER_IS_CONENAT_CHECK(helper)==SUCCESS)
						TRACE("connection is processed by nf_conntrack %s helper function, continue learning", helper->name);
					else{
						TRACE("connection is processed by nf_conntrack %s helper function, skip learning", helper->name);
						RTK_FC_HELPER_RCU_READ_UNLOCK();
						return FAILED;
					}
				}
				else{
					TRACE("connection is processed by nf_conntrack helper function, skip learning");
					RTK_FC_HELPER_RCU_READ_UNLOCK();
					return FAILED;
				}
			}
			RTK_FC_HELPER_RCU_READ_UNLOCK();
		}
		else
		{
			TRACE("Skip FC Alg check! But sw flow only!");
			*swOnly = 1;
		}

	}
	return SUCCESS;
}

int _rtk_fc_dualHeader_g3ExtraInfo_setting(rtk_fc_dualHdrtype_t dual_passthrough_type, rtk_rg_asic_path1_entry_t *flowEntry, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, uint8 isNAPT, int igrNetifIdx, int egrNetifIdx, bool *swOnly)
{
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)	
	bool isL3 = ((pPktHdr->iph) || (pPktHdr->ip6h)) ? TRUE : FALSE;
	bool isL4 = ((pPktHdr->tcph) || (pPktHdr->udph)) ? TRUE : FALSE;
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	//struct net_device *vxlanDev=NULL;
	//int vxlanNetifIdx = 0;
	int ret = 0;
	
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
	pG3IgrExtraInfo->ipsec_hook_table_index = -1;
#endif

	if(unlikely((dual_passthrough_type!=RTK_FC_DUALTYPE_NONE) && (flowEntry->in_path == FB_PATH_5)))
	{
		if(dual_passthrough_type == RTK_FC_DUALTYPE_PPTP)
		{
			/*
			PPTP passthrough:
			bridge: no L4 header, path1
			routing: 
				- ApolloPro SW only, path5		(HW not support path5 with no L4 header)
			NAPT: SW only, path 5		(HW not support path5 with no L4 header. Otherwise, downstream flow need SW mapping internal IP/PORT with callID)
			*/
			TRACE("PPTP PASSTHROUGH!");
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			if(isNAPT)
			{
				*swOnly = TRUE;
				if(pFcIngressData->ingressCallid != pPktHdr->tunnelInfo.pptp.grecallid)
					DEBUG("[GRE] Call Id has been modified! Ingress call id: %d, Call id from Protocol stack: %d",pFcIngressData->ingressCallid, pPktHdr->tunnelInfo.pptp.grecallid);
			
				pG3IgrExtraInfo->dualPtFlowMapping_info.dual_type = RTK_FC_DUALTYPE_PPTP;
				pG3IgrExtraInfo->dualPtFlowMapping_info.pptp_flowMapping.in_gre_call_id = pFcIngressData->ingressCallid;	
				pG3IgrExtraInfo->dualPtFlowMapping_info.pptp_flowMapping.out_gre_call_id = pPktHdr->tunnelInfo.pptp.grecallid;
					
			}
#else
			
			*swOnly = TRUE;
			if(pFcIngressData->ingressCallid != pPktHdr->tunnelInfo.pptp.grecallid)
				DEBUG("[GRE] Call Id has been modified! Ingress call id: %d, Call id from Protocol stack: %d",pFcIngressData->ingressCallid, pPktHdr->tunnelInfo.pptp.grecallid);
			
			pG3IgrExtraInfo->dualPtFlowMapping_info.dual_type = RTK_FC_DUALTYPE_PPTP;
			pG3IgrExtraInfo->dualPtFlowMapping_info.pptp_flowMapping.in_gre_call_id = pFcIngressData->ingressCallid;	
			pG3IgrExtraInfo->dualPtFlowMapping_info.pptp_flowMapping.out_gre_call_id = pPktHdr->tunnelInfo.pptp.grecallid;
#endif
			
		}
		else if(dual_passthrough_type == RTK_FC_DUALTYPE_L2TP)
		{
			
			/*
			L2TP passthrough:
			bridge: path3
			routing: HW, path5
			NAPT: SW only, path 5	(downstream flow need SW mapping internal IP/PORT with tunnelID/sessionID)
			*/
			if(isNAPT)
			{
				*swOnly = TRUE;
				pG3IgrExtraInfo->dualPtFlowMapping_info.dual_type = RTK_FC_DUALTYPE_L2TP;
				pG3IgrExtraInfo->dualPtFlowMapping_info.l2tp_flowMapping.in_l2tp_session_id = pPktHdr->tunnelInfo.l2tp.sessionid;
				pG3IgrExtraInfo->dualPtFlowMapping_info.l2tp_flowMapping.in_l2tp_tunnel_id = pPktHdr->tunnelInfo.l2tp.tunnelid;
					
				
			}
		}
		else if(dual_passthrough_type == RTK_FC_DUALTYPE_IPSEC)
		{
			/*
			IPsec passthrough:
			bridge: no L4 header, path1
			routing: SW only, path5		(HW not support path5 with no L4 header)
			NAPT: SW only, path 5		(HW not support path5 with no L4 header)
			*/
			*swOnly = TRUE;
			pG3IgrExtraInfo->dualPtFlowMapping_info.dual_type = RTK_FC_DUALTYPE_IPSEC;
			pG3IgrExtraInfo->dualPtFlowMapping_info.ipsec_flowMapping.esp_spi = pPktHdr->esph->spi;		
		}
	}

	if(pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IPSEC )
	{
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		if(fc_db.controlFuc.ipsec_en_shortCut)
		{
			int index = -1;
			int ingress_tcp = (pFcIngressData->ingressTagif & TCP_TAGIF)? TRUE : FALSE;
			int ingress_udp = (pFcIngressData->ingressTagif & UDP_TAGIF)? TRUE : FALSE;
			if((ingress_tcp ==FALSE && ingress_udp == FALSE )) 
			{
				DEBUG("Not support non-tcp/non-udp IPSEC Endpoint upstream !!");
				return FAILED;
			}
			if(pFcIngressData->dual_hdr_cnt > 0)
			{
				DEBUG("Not support dual stacking IPSEC Endpoint upstream (dual header stack: %d)!!",pFcIngressData->dual_hdr_cnt);
				return FAILED;
			}
			RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
			ret = _rtk_fc_esp_policy_check(&index, pFcIngressData, pPktHdr, pG3IgrExtraInfo, 1);
			RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
			if(ret == SUCCESS && index != -1) 
			{
				int i ;
				struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
				int filterStackingCnt = 0;
				
				IPSEC("ESP upstream match fc_db.fc_ipsec_info[%d]! do learning shortCut!",index);
				RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
				ret = _rtk_fc_esp_setting(index, pFcIngressData, pPktHdr, pG3IgrExtraInfo, swOnly,1);
				if(fc_db.controlFuc.ipsec_pe_offload == 1)
				{
					IPSEC("[PE offload] Encrypt saInfo_idx %u pe_crypto_sw_id=0x%x", index, fc_db.fc_ipsec_info[index].pe_crypto_sw_id);
					if(fc_db.fc_ipsec_info[index].pe_crypto_sw_id == 0) //invalid
					{
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)					
						rtk_fc_pe_crypto_conn_info_prepare(RTK_FC_IPSEC_DIR_ENCRYT, pFcIngressData, pPktHdr);
#endif
					}
					if(fc_db.fc_ipsec_info[index].pe_crypto_sw_id) //valid
					{
						rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)flowEntry;
						
						IPSEC("[PE offload encrypt] pe_crypto_sw_id=0x%x", fc_db.fc_ipsec_info[index].pe_crypto_sw_id);
						pFlowPath5->valid = TRUE;
						pFlowPath5->out_l4_act = FALSE;
						pG3IgrExtraInfo->ipsec_pe_offload = 1;
						*swOnly = FALSE;
					}
				}
				RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
				for(i=0 ; i<DEV_STACK_MAX ;i++)
				{
					p_PScacheDev[i] = rtk_fc_getDev_by_fcDevIdx(pPktHdr->logicalOutDevIfidx[i]);
					if(p_PScacheDev[i])
						filterStackingCnt++;
				}
				pPktHdr->ipsec_sc_info.devGwMacIdx = pPktHdr->logicalOutDevIfidx[filterStackingCnt-1];
				return ret;
			}

		}
#endif
		DEBUG("Not support IPSEC Endpoint upstream !!");
		return FAILED;
	}
	else if(pFcIngressData->isDualHeader && (pFcIngressData->ingressTagif & IPSEC_ESP_TAGIF) && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE)
	{
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		if(fc_db.controlFuc.ipsec_en_shortCut)
		{

			int index = -1;
			bool hasL4 = ((pPktHdr->tcph) || (pPktHdr->udph)) ? TRUE : FALSE;

			if(hasL4 == FALSE)
			{
				DEBUG("Not support non-tcp/non-udp IPSEC Endpoint downstream !!");
				return FAILED;
			}
			
			RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
			ret = _rtk_fc_esp_policy_check(&index, pFcIngressData, pPktHdr, pG3IgrExtraInfo, 0);
			RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
			
			if(ret == SUCCESS && index != -1)
			{
				int i ;
				struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
				int filterStackingCnt = 0;
				
				IPSEC("ESP downstream match fc_db.fc_ipsec_info[%d]! do learning shortCut!",index);
				RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
				ret = _rtk_fc_esp_setting(index, pFcIngressData, pPktHdr, pG3IgrExtraInfo, swOnly,0);
				if(fc_db.controlFuc.ipsec_pe_offload == 1)
				{
					IPSEC("[PE offload] Decrypt saInfo_idx %u pe_crypto_sw_id=0x%x", index, fc_db.fc_ipsec_info[index].pe_crypto_sw_id);
					if(fc_db.fc_ipsec_info[index].pe_crypto_sw_id == 0) //invalid
					{
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)					
						rtk_fc_pe_crypto_conn_info_prepare(RTK_FC_IPSEC_DIR_DECRYT, pFcIngressData, pPktHdr);
#endif
					}
					if(fc_db.fc_ipsec_info[index].pe_crypto_sw_id) //valid
					{
						rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)flowEntry;
						
						IPSEC("[PE offload decrypt] pe_crypto_sw_id=0x%x", fc_db.fc_ipsec_info[index].pe_crypto_sw_id);
						pFlowPath5->valid = TRUE;
						pFlowPath5->out_l4_act = FALSE;
						pG3IgrExtraInfo->ipsec_pe_offload = 1;
						*swOnly = FALSE;
					}
				}
				RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
				
				for(i=0 ; i<DEV_STACK_MAX ;i++)
				{
					p_PScacheDev[i] = rtk_fc_getDev_by_fcDevIdx(pPktHdr->logicalInDevIfidx[i]);
					if(p_PScacheDev[i])
						filterStackingCnt++;
				}
				pPktHdr->ipsec_sc_info.devGwMacIdx = pPktHdr->logicalInDevIfidx[filterStackingCnt-1];
				/*Decrypt should always un-tag to cpu*/
				flowEntry->out_cvid_format_act = FALSE;
				flowEntry->out_cpri_format_act = FALSE;
				flowEntry->out_svid_format_act = FALSE;
				flowEntry->out_spri_format_act = FALSE;
				return ret;
			}
		}
#endif
		DEBUG("Not support IPSEC Endpoint downstream !!");
		return FAILED;

	}

	if((pFcIngressData->ingressTagif & ICMP_TAGIF) &&  (pPktHdr->icmph || pPktHdr->icmp6h)  )
	{
		pG3IgrExtraInfo->ingress_icmp_valid = TRUE;
		pG3IgrExtraInfo->ingress_icmp_id = pFcIngressData->icmp_id;
	}

	if(pFcIngressData->isDualHeader && pFcIngressData->ingressTagif & VXLAN_TAGIF && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE )
	{
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)	
		int inner_extra_len = 0;

		//VXLAN DOWN STREAM, swonly
		if(isL3!= TRUE ||isL4 != TRUE)
		{
			DEBUG("Not support non-l4 vxlan,go shortCut only !!");
			pPktHdr->skip_special_fastFwd = 1;
		}
		TRACE("[VXLAN]Down stream!");
		*swOnly = TRUE;
	
		pG3IgrExtraInfo->dualHdrType					= RTK_FC_DUALTYPE_VXLAN;
		pG3IgrExtraInfo->direction						= RTK_FC_FLOW_DIRECTION_DOWNSTREAM;
		pG3IgrExtraInfo->vxlan_info.ingress_intf_idx	= igrNetifIdx;
		pG3IgrExtraInfo->vxlan_info.egress_intf_idx 	= egrNetifIdx; 
		pG3IgrExtraInfo->vxlan_info.outer_tag_len 		= pFcIngressData->vxlan_info.outer_tag_len;
		
		if(pFcIngressData->ingressTagif & SVLAN_TAGIF)
		{
			pG3IgrExtraInfo->vxlan_info.ingress_inner_stag = 1;
			inner_extra_len+=4;
		}
		if(pFcIngressData->ingressTagif & CVLAN_TAGIF)
		{
			pG3IgrExtraInfo->vxlan_info.ingress_inner_ctag = 1;
			inner_extra_len+=4;
		}
		if(pFcIngressData->ingressTagif & PPPOE_TAGIF)
		{
			pG3IgrExtraInfo->vxlan_info.ingress_inner_pppoeTag = 1;
			inner_extra_len+=8;
		}

		pPktHdr->vxlan_info.inner_extra_len = inner_extra_len;
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		TRACE("[VXLAN]Down stream!");
		if(pFcIngressData->vxlan_info.outer_isV6 && fc_db.controlFuc.en_vxlan_v6_hwacc == 0)
		{
			*swOnly = TRUE;
		}
		ret = rtk_fc_vxlan_8277C_downstream_flow_setting(pG3IgrExtraInfo, igrNetifIdx, egrNetifIdx, pFcIngressData, pPktHdr, *swOnly);
		if(ret!=RTK_FC_RET_OK)
		{	
			WARNING("VXLAN Downstream flow setting add! ret = %x",ret);
			return FAILED;
		}

#endif
	}
	else if( pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN) 
	{
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)		
		//VXLAN UPSTREAM STREAM, swonly
		if(isL3!= TRUE ||isL4 != TRUE)
		{
			DEBUG("Not support non-l4 vxlan, go shortCut only!!");
			pPktHdr->skip_special_fastFwd = 1;
		}
		TRACE("[VXLAN]Up stream!");
		*swOnly = TRUE;
		pG3IgrExtraInfo->dualHdrType				= RTK_FC_DUALTYPE_VXLAN;
		pG3IgrExtraInfo->direction					= RTK_FC_FLOW_DIRECTION_UPSTREAM;
		pG3IgrExtraInfo->vxlan_info.outer_tag_len 	= pPktHdr->vxlan_info.outerTag_off;
		pG3IgrExtraInfo->vxlan_info.ingress_intf_idx	= igrNetifIdx;
		pG3IgrExtraInfo->vxlan_info.egress_intf_idx 	= egrNetifIdx; 
		pG3IgrExtraInfo->vxlan_info.outer_srcPort 		= ntohs(pPktHdr->outer_udph->source);
		//pG3IgrExtraInfo->vxlan_info.vni				= ntohl(pPktHdr->vxlan_info.vxlanHdr->vx_vni)>>8;
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

		if(pPktHdr->outer_ip6h && fc_db.controlFuc.en_vxlan_v6_hwacc == 0)
		{
			*swOnly = TRUE;
		}
		ret = rtk_fc_vxlan_8277C_upstream_flow_setting(pG3IgrExtraInfo, igrNetifIdx, egrNetifIdx, pPktHdr, *swOnly);
		if(ret!=RTK_FC_RET_OK)
		{	
			WARNING("VXLAN upstream flow setting add! ret = %x",ret);
			return FAILED;
		}

#endif
	}
	return SUCCESS;
}
int _rtk_fc_extraInfo_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_igrExtraInfo_t *pExtraInfo, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flowPath_param_t *flowPath_param, rtk_fc_ingress_data_t *pFcIngressData, bool *swOnly, uint32 in_path, uint32 in_pppoeif, bool if_egr_tos_remark)
{
	int ret;
		
	_rtk_fc_g3IgrExtraInfo_init(pG3IgrExtraInfo);
	bzero(pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	bzero(pExtraInfo, sizeof(rtk_fc_igrExtraInfo_t));

	
	pExtraInfo->lutIgrSaIdx = flowPath_param->saIdx;
	pExtraInfo->lutEgrDaIdx = flowPath_param->daIdx;
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
	if(fc_db.controlFuc.flow_sessionAutoExtend && pFcIngressData->isDualHeader!=TRUE){				// dummy packet is not support for dual header
		rtk_fc_igrDummyData_t tmpData;
		bzero(&tmpData, sizeof(tmpData));
		pExtraInfo->dummyPkt = &tmpData;
		rtk_fc_flow_skbDataReasm(pFcIngressData, pPktHdr, (uint8 *)pExtraInfo->dummyPkt);
		pExtraInfo->srcDev = pFcIngressData->srcDev;
	}
#endif
	if(flowPath_param->isMAPT || flowPath_param->para_isXLAT)
		pExtraInfo->pFcIngressData=pFcIngressData;	//for MAP-T, used in _rtk_fc_flow_swField_set

	/*
	 * sw shaper patch!
	 *	- force adding to sw flow
	 *	- make sure flow->out_share_meter_idx is assigned from pPktHdr->remarkDec
	 */

	 if(pPktHdr->remarkDec.swShaper_en)
	 {
		// if hw meter disable, force adding to sw flow
		if(pPktHdr->remarkDec.meterIdx_en == 0U) 
			*swOnly = TRUE;
		pExtraInfo->swShaperEn |= (1<<SWSHAPER_TYPE_FLOW);
	 }
	 if((pPktHdr->smacHostPolIdx != SIGNED_INVALID) && fc_db.hostPoliceTable[pPktHdr->smacHostPolIdx].ingress_limit_by_swShaper)
	 {
		*swOnly = TRUE; // SW shaper for host policing
		pExtraInfo->swShaperEn |= (1<<SWSHAPER_TYPE_SMAC_HOST);
	 }

	 if((pPktHdr->dmacHostPolIdx != SIGNED_INVALID) && fc_db.hostPoliceTable[pPktHdr->dmacHostPolIdx].egress_limit_by_swShaper)
	 {
		*swOnly = TRUE; // SW shaper for host policing
		pExtraInfo->swShaperEn |= (1<<SWSHAPER_TYPE_DMAC_HOST);
	 }
	 
	//prepare info that needed for adding G3 HW entry
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	pG3IgrExtraInfo->crc16 = pFcIngressData->flowHashIdx;
	pG3IgrExtraInfo->crc32 = pFcIngressData->crc32;
#endif
	pG3IgrExtraInfo->lutIgrSaIdx = pExtraInfo->lutIgrSaIdx;
	pG3IgrExtraInfo->lutEgrDaIdx = pExtraInfo->lutEgrDaIdx;
	pG3IgrExtraInfo->svlan_id = flowPath_param->igrSVID;
	pG3IgrExtraInfo->cvlan_id = flowPath_param->igrCVID;
	//prepare info that needed for adding G3 HW entry
	if((in_path==FB_PATH_34)||(in_path==FB_PATH_5)){
		if(!flowPath_param->isIpv6 || flowPath_param->isMAPT || flowPath_param->para_isXLAT){
			pG3IgrExtraInfo->ipv6_Saddr.s6_addr32[0] = pFcIngressData->srcIp;
			pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[0] = pFcIngressData->dstIp;
		}else{
			memcpy(&pG3IgrExtraInfo->ipv6_Saddr, &pPktHdr->ip6h->saddr, sizeof(struct in6_addr));
			memcpy(&pG3IgrExtraInfo->ipv6_Daddr, &pPktHdr->ip6h->daddr, sizeof(struct in6_addr));
		}
	}
		//check if the packet hit host policing

	if(pPktHdr->isV6_NAT)
	{
		uint32 wan_prefix_len;
		uint32 lan_prefix_len;
		uint8 isNPT;
		//NPTv6-TODO:Should check is NPTv6 or NAT66
		ret = _rtk_fc_nptv6_nat66_check(pPktHdr, pFcIngressData, &isNPT);
		if(ret == FAILED)
		{
			WARNING("Check nptv6/nat66 failed, direction: %d",pPktHdr->direction);
			return FAILED;
		}
		
		if(isNPT)
		{
			
			wan_prefix_len = pFcIngressData->nptv6_info.wan_prefix_len;
			lan_prefix_len = pFcIngressData->nptv6_info.lan_prefix_len;
		
			pG3IgrExtraInfo->naptv6_info.wan_prefix_len = wan_prefix_len;
			pG3IgrExtraInfo->naptv6_info.lan_prefix_len = lan_prefix_len;
			pG3IgrExtraInfo->naptv6_info.isNPTv6  = 1U;

			DEBUG("[NPTv6]wan prefix len = %d, lan prefix len = %d", wan_prefix_len, lan_prefix_len);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			_rtk_fc_nptv6_prefixIdx_get(pG3IgrExtraInfo, pPktHdr);
#endif				
		}
		else
		{
			
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)		
			if(fc_db.controlFuc.if_hash_fib_384_bits)
			{
				DEBUG("[NAT66]HW support NAT 66! ");
			}
			else
#endif			
			{
				//NAT66 (If need hw flow, 384 bit mode should turn on)
				*swOnly = TRUE;
				DEBUG("[NAT66]SW flow only!");
			}
		}
	}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	if((pPktHdr->fwdType == RTK_FC_FWDTYPE_NATLOOPBACK) &&
		pPktHdr->iph && (pPktHdr->tcph || pPktHdr->udph) &&
		(in_path==FB_PATH_5) && flowPath_param->isNAPT)
	{
		pG3IgrExtraInfo->natloopback_act_info.is_ipv4_natloopback = TRUE;
		if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			// outbound, get additional dip and dport
			pG3IgrExtraInfo->natloopback_act_info.l3_ip = ntohl(pPktHdr->iph->daddr);
			pG3IgrExtraInfo->natloopback_act_info.l4_port = pPktHdr->tcph ? (ntohs(pPktHdr->tcph->dest)) : (ntohs(pPktHdr->udph->dest));
		}else
		{
			// inbound,  get additional sip and sport
			pG3IgrExtraInfo->natloopback_act_info.l3_ip = ntohl(pPktHdr->iph->saddr);
			pG3IgrExtraInfo->natloopback_act_info.l4_port = pPktHdr->tcph ? (ntohs(pPktHdr->tcph->source)) : (ntohs(pPktHdr->udph->source));
		}
	}
#endif

	if(flowPath_param->saIdx != FAIL)
		pG3IgrExtraInfo->ingressSaHostPolIdx = fc_db.lutTbl[flowPath_param->saIdx]->hostPolIdx;
	if(flowPath_param->daIdx != FAIL)
		pG3IgrExtraInfo->egressDaHostPolIdx = fc_db.lutTbl[flowPath_param->daIdx]->hostPolIdx;
	pG3IgrExtraInfo->direction = pPktHdr->direction;

	if(((in_path==FB_PATH_34)||(in_path==FB_PATH_5)) && (in_pppoeif)){
		pG3IgrExtraInfo->pppoe_session_id = pFcIngressData->sessionId;
	}

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI])
		pG3IgrExtraInfo->svlan_pri = pFcIngressData->srcSVlanPri;

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI])
	{
		pG3IgrExtraInfo->igr_svlan_dei = pFcIngressData->srcSVlanDei;
		pG3IgrExtraInfo->igr_cvlan_cfi = pFcIngressData->srcCVlanCfi;
	}
	pG3IgrExtraInfo->egr_svlan_dei = pPktHdr->svlandei;
	pG3IgrExtraInfo->egr_cvlan_cfi = pPktHdr->cvlancfi;
	pG3IgrExtraInfo->igr_svlan_tpid_sel = pFcIngressData->srcSVlanTpid_sel;
	pG3IgrExtraInfo->egr_svlan_tpid_sel = pPktHdr->stpid_sel;
	if(if_egr_tos_remark)
	{
		pG3IgrExtraInfo->egr_tos_ecn_remark = TRUE;
		pG3IgrExtraInfo->egr_tos_ecn = flowPath_param->egr_tos&0x3;
	}
	pG3IgrExtraInfo->pon_stream_id = pFcIngressData->ponStreamId; // if not from PON, FcIngressData->ponStreamId will be 0.
	
	//pG3IgrExtraInfo = &g3IgrExtraInfo;
	return SUCCESS;
}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
uint32 _rtk_fc_wlan_tx_lookup_gmac_decision_by_port(uint32 spa)
{
	uint32 gmac = 0;


	if(fc_db.controlFuc.wifi_tx_gmac_sel[spa] == RTK_FC_WIFI_GMAC_RR) {
		gmac = fc_db.controlFuc.wifi_tx_gmac_sel_current++%fc_db.controlFuc.wifi_tx_gmac_trunking_num;
	}else if(fc_db.controlFuc.wifi_tx_gmac_sel[spa] == RTK_FC_WIFI_GMAC_TBD) {
		gmac = fc_db.controlFuc.wifi_tx_gmac_sel[spa] = (fc_db.controlFuc.wifi_tx_gmac_sel_spa++) % fc_db.controlFuc.wifi_tx_gmac_trunking_num;
	}else {
		gmac = fc_db.controlFuc.wifi_tx_gmac_sel[spa];
	}

	return gmac;
}

uint32 _rtk_fc_wlan_tx_lookup_gmac(uint32 spa)
{
	uint32 gmac = 0;
	
	if(fc_db.controlFuc.wifi_tx_gmac_auto_sel_en) {
		return _rtk_fc_wlan_tx_lookup_gmac_decision_by_port(spa);
	}
	else {
		gmac = fc_db.controlFuc.wifi_tx_gmac_sel_current++%fc_db.controlFuc.wifi_tx_gmac_trunking_num;
		return gmac;
	}
}
#endif

int _rtk_fc_flowEntry_path5_setting(rtk_rg_asic_path1_entry_t *flowEntry, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_flowPath_param_t *path5_param, bool *swOnly)
{
	// ======================== path 5 ========================
	rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)flowEntry;
	bool isL4 = ((pPktHdr->tcph) || (pPktHdr->udph)) ? TRUE : FALSE;
	uint8 l4Protocol=0U;
	
	uint32 sip = 0U, dip = 0U;
	uint16 sport = 0U, dport = 0U;
	uint16 egrDstPort = 0U;
	int idx = 0;
	int flowMibEn= 0, flowMibIdx = 0;
	int highPriFlowPatternIdx;
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	int ret = 0;
#endif


	l4Protocol = pFcIngressData->l4protol;

	//For ESP downstream TCP 
	if(pFcIngressData->isDualHeader && (pFcIngressData->ingressTagif & IPSEC_ESP_TAGIF) && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE)
	{
		l4Protocol = pPktHdr->l4protol;
	}
	
	if(pPktHdr->ip6h)
	{
		TRACE("Ingress v6 %s: %pI6c, egress v6 %s:%pI6c memcmp:%d", (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?"sip":"dip", 
								&pFcIngressData->ingress_v6ip_info.s6_addr32[0],
								(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?"sip":"dip",
								(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?&pPktHdr->ip6h->saddr.s6_addr32[0]:&pPktHdr->ip6h->daddr.s6_addr32[0],
								memcmp(&pFcIngressData->ingress_v6ip_info.s6_addr32[0], &pPktHdr->ip6h->saddr.s6_addr32[0], sizeof(struct in6_addr)));
		
	}

	if(path5_param->isNAPT && pathDecision_var.flowPath_param.isIpv6 && !(path5_param->isMAPT || path5_param->para_isXLAT)){
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		//Don't sw-only
#elif defined(CONFIG_FC_RTL9607C_SERIES)
		//Don't sw-only
		TRACE("dynamic_sram_desc = %d fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism = %d",dynamic_sram_desc, fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism);
		if( (dynamic_sram_desc!=1 && dynamic_sram_desc!=0)|| (dynamic_sram_desc==0 && fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == 0) )
			*swOnly = TRUE;

		if(pPktHdr->igrWlanDevIdx != RTK_FC_WLANX_NOT_FOUND || pPktHdr->egrWlanDevIdx != RTK_FC_WLANX_NOT_FOUND)
			*swOnly = TRUE;
#else
		*swOnly = TRUE;
#endif
		pPktHdr->isV6_NAT = TRUE; 
		TRACE("NPTv6/NAT66 find, swOnly = %d", *swOnly);

		
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(pPktHdr->stpid_sel) *swOnly = TRUE; // HW SVLAN action supports svlan_tpid_sel 0 only
	else if((!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI]) && (path5_param->igrSVlanTagif)) *swOnly = TRUE; // HW not support SPRI as flow key
#endif


	// prepare ip port info
	sip = pFcIngressData->srcIp;
	dip = pFcIngressData->dstIp;


	sport = pFcIngressData->srcPort;
	dport = pFcIngressData->dstPort;
	
	if(isL4){
		if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			// outbound
			egrDstPort = (l4Protocol == IPPROTO_TCP) ? (pPktHdr->tcph ? (ntohs(pPktHdr->tcph->source)): 0)
													: (pPktHdr->udph ? (ntohs(pPktHdr->udph->source)) : 0);
		}else
		{
			// inbound
			egrDstPort = (l4Protocol == IPPROTO_TCP) ? (pPktHdr->tcph ? (ntohs(pPktHdr->tcph->dest)) : 0)
													: (pPktHdr->udph ? (ntohs(pPktHdr->udph->dest)) : 0);
		}
	}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	else
	{
		// udp-lite, RDPv1, RDPv2: should not change port
		if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)
			egrDstPort = sport;// outbound
		else
			egrDstPort = dport;// inbound
	}
#endif

	{
		//Outbound or Inbound
		bzero(pFlowPath5, sizeof(*pFlowPath5));
		pFlowPath5->valid = TRUE;
		pFlowPath5->in_path = FB_PATH_5;
		pFlowPath5->in_ipv4_or_ipv6 = pathDecision_var.flowPath_param.isIpv6;
		pFlowPath5->in_src_ipv4_addr = sip;
		if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM || !path5_param->isNAPT)
			pFlowPath5->in_dst_ipv4_addr = dip;
		else if(((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM) || (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_LANBRIDGE)) && path5_param->isNAPT)
		{
			if(pPktHdr->iph!=NULL){
				pFlowPath5->out_dst_ipv4_addr = ntohl(pPktHdr->iph->daddr);
				if(path5_param->isMAPT || path5_param->para_isXLAT)pFlowPath5->in_src_ipv4_addr = ntohl(pPktHdr->iph->saddr);
			}else if(pPktHdr->iph == NULL && pPktHdr->ip6h!=NULL) 
			{
				// Down stream ipv6 dnat
				// Fill pFcIngressData->dstIp(v6 dip hash) to out_dst_ipv4_addr so search flow in short-cut can 
				// Find appropriate flow entry.
				//
				pFlowPath5->out_dst_ipv4_addr = dip;
				if(*swOnly)
					pFlowPath5->out_dst_ipv4_addr = dip;
			}
		}

		pFlowPath5->in_l4_src_port = sport;
		pFlowPath5->in_l4_dst_port = dport;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		pFlowPath5->in_l4proto_num = l4Protocol;
#else
		pFlowPath5->in_l4proto = (l4Protocol == IPPROTO_TCP)?1:0;
#endif
		pFlowPath5->in_intf_idx = path5_param->igrNetifIdx;
		pFlowPath5->in_stagif = path5_param->igrSVlanTagif;
		pFlowPath5->in_ctagif = path5_param->igrCVlanTagif;
		pFlowPath5->in_pppoeif = path5_param->igrPPPoETagif;
		pFlowPath5->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS];
		pFlowPath5->in_tos = path5_param->igr_tos;
		pFlowPath5->in_cvlan_pri = pFcIngressData->srcCVlanPri;

		//-------------------------egress action-------------------------

		pFlowPath5->out_l4_act = path5_param->isNAPT;														// Routing: 0, NAPT: 1

		if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			pFlowPath5->out_l4_direction =1U;													// outbound: 1, inbound: 0 (routing don't care)
		}else
		{
			pFlowPath5->out_l4_direction =0U;
		}
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

		if(pPktHdr->isV6_NAT)
			pFlowPath5->out_l4_act = 0U; // v6 nat should not edit sip and dip
#endif			
		pFlowPath5->out_l4_port = egrDstPort;													// don't care if routing

		pFlowPath5->out_intf_idx = path5_param->egrNetifIdx;

#if defined(CONFIG_FC_RTL9607C_SERIES)
		if(fc_db.controlFuc.wifi_tx_gmac_auto_sel_en &&
			path5_param->daIdx != SIGNED_INVALID && 
				fc_db.lutTbl[path5_param->daIdx]->duplicateEntry_exist &&
				fc_db.lutTbl[path5_param->daIdx]->wlan_dev_idx < RTK_FC_WLANX_END_INTF) {
			// wifi sta and duplicate lut entry exist
			int16 lutIdx = path5_param->daIdx;
			uint32 fid = _rtk_fc_wlan_tx_lookup_gmac(pPktHdr->ingressPort.macPortIdx);		// 0,1,2

			if(fid == RTK_FC_WIFI_FID_0) {
				// do nothing, gmac 0
				ret = RTK_FC_RET_OK;
			}else {
				// find gmac 1 or gmac 2
				ret = _rtk_fc_lut_find_fid(&fc_db.lutTbl[path5_param->daIdx]->l2Addr[0], fid, &lutIdx);
			}
			if(ret == RTK_FC_RET_OK) {
				path5_param->daIdx = lutIdx;
			}

		}
#endif

#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)) // 8277C/9607F no need indMac, pFlowPath5->out_dmac_idx is meaningless
		if((ret = rtk_fc_indMac_idx_get(path5_param->daIdx, &idx))){
			WARNING("indmac get fail, daidx = %d", path5_param->daIdx);
			return FAILED;
		}	
		barrier();
#endif
		pFlowPath5->out_dmac_idx = idx;

		//svlan
		pFlowPath5->out_egress_svid_act = path5_param->egrSVlanTagif ? TRUE : FALSE;

		pFlowPath5->out_svlan_id =  (path5_param->egrSVlanTagif ?(pPktHdr->svh?pPktHdr->svlanid : 0):0 );

		pFlowPath5->out_stag_format_act = TRUE;
		pFlowPath5->out_svid_format_act = (path5_param->egrSVlanTagif)?TRUE:FALSE;
		pFlowPath5->out_spri_format_act = (path5_param->egrSVlanTagif)?TRUE:FALSE;
		pFlowPath5->out_spri = (path5_param->egrSVlanTagif ?(pPktHdr->svh?pPktHdr->svlanpri : 0):0 );

		//cvlan
		pFlowPath5->out_egress_cvid_act = TRUE;

	#if defined(CONFIG_FC_RTL8198F_SERIES)
		RTK_FC_HOOK_83XX_QOS_SET_VLAN_TAG(&path5_param->egrCVlanTagif, &path5_param->egrCVID, pPktHdr->direction, RTSKB_SKB(rtskb));
	#endif
		
		pFlowPath5->out_cvlan_id =  path5_param->egrCVID;
		pFlowPath5->out_ctag_format_act = TRUE;
		pFlowPath5->out_cvid_format_act = (path5_param->egrCVlanTagif)?TRUE:FALSE;
		pFlowPath5->out_cpri_format_act = (path5_param->egrCVlanTagif)?TRUE:FALSE;
		pFlowPath5->out_cpri = (path5_param->egrCVlanTagif ?(pPktHdr->cvh?pPktHdr->cvlanpri : 0):0 );


	#if defined(CONFIG_FC_RTL8198F_SERIES)
	{
		int qos_flag = RTK_FC_HOOK_GET_83XX_QOS_STATUS();
		if((qos_flag != RTK_FC_RET_ERR) && (qos_flag > 0))
		 	pFlowPath5->out_cpri = pPktHdr->remarkDec.outputQid;
	}
	#endif

		//other
		if(pPktHdr->remarkDec.outputQid)
			pFlowPath5->out_user_pri_act = TRUE;
		else
			pFlowPath5->out_user_pri_act = FALSE;
		
		pFlowPath5->out_user_priority = pPktHdr->remarkDec.outputQid;

		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS])
		{
			// remarking dscp only when dscp as flow key
			pFlowPath5->out_dscp_act = TRUE;
			pFlowPath5->out_dscp = (path5_param->egr_tos>>2) & 0x3f;
		}

		pFlowPath5->out_stream_idx_act = pPktHdr->remarkDec.streamId_en;
		pFlowPath5->out_stream_idx = pPktHdr->remarkDec.streamId;

		pFlowPath5->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
		pFlowPath5->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

		pFlowPath5->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
		pFlowPath5->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;

		pFlowPath5->out_extra_tag_index = path5_param->tunnelExtraTagIdx;

		pFlowPath5->out_egress_port_to_vid_act = FALSE;
		pFlowPath5->out_drop = 0U;
		pFlowPath5->lock = (pFlowPath5->out_share_meter_act) ? TRUE: FALSE;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		//sync fmr idx to hw setting
		if(pPktHdr->pkt_fmr_idx != SIGNED_INVALID && pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			TRACE("flow set fmr table fmr_idx=%d",pPktHdr->pkt_fmr_idx);
			pFlowPath5->out_fmr_idx_act = TRUE;
			pFlowPath5->out_fmr_idx = pPktHdr->pkt_fmr_idx;
		}
#endif

		flowMibEn = pFlowPath5->out_flow_counter_act;
		flowMibIdx = pFlowPath5->out_flow_counter_idx;

		DEBUG("Egress dmac index: %d\n", pFlowPath5->out_dmac_idx);
	}
#if defined(CONFIG_FC_RTL9607C_SERIES)	
	if(fc_db.controlFuc.special_fast_forward_mode && dynamic_sram_desc ==0)
	{
		if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF && (pFlowPath5->out_user_priority == 5 || pFlowPath5->out_user_priority == 4 ) )
		{
			WARNING("DRAM mode wifi packet try to set priority 5 or 4! change to 3");
			pFlowPath5->out_user_priority = 3;
		}
	}
#endif

	if(_rtk_fc_highPriFlow_check(pFlowPath5, &highPriFlowPatternIdx))
	{
		TRACE("hit highPriFlowPattern[%d]", highPriFlowPatternIdx);
		pFlowPath5->lock = TRUE; //high priority flow
	}
#if !defined(CONFIG_RTK_L34_G3_PLATFORM)
	if(!isL4) {
		// ! (tcp || udp)
		TRACE("Not TCP or UDP -> Sw flow only!");
		*swOnly = TRUE;
	}
#endif
	if((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM))
	{
		pPktHdr->isP5DS = 1;
		pPktHdr->path5_downStream_flow_bucket_idx = pFlowPath5->out_dst_ipv4_addr&0xff;
	}
			

	if(pFlowPath5->in_ipv4_or_ipv6==0){
		if(isL4)
			TRACE("[IPv4][%s][%s] Src=%d.%d.%d.%d(%d) Dest=%d.%d.%d.%d(%d) L4Proto=0x%02x(%s)\n", (pFlowPath5->out_l4_act)?"L34":"L3", (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?"US":"DS",
				(sip>>24)&0xff, (sip>>16)&0xff, (sip>>8)&0xff, (sip)&0xff, sport,
				(dip>>24)&0xff, (dip>>16)&0xff, (dip>>8)&0xff, (dip)&0xff, dport,
				l4Protocol, (l4Protocol == IPPROTO_TCP)?"TCP":((l4Protocol == IPPROTO_UDP)?"UDP":"Others"));
		else
			TRACE("[IPv4][%s][%s] Src=%d.%d.%d.%d(%d) Dest=%d.%d.%d.%d(%d) [NOT TCP/UDP]\n", (pFlowPath5->out_l4_act)?"L34":"L3", (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?"US":"DS",
				(sip>>24)&0xff, (sip>>16)&0xff, (sip>>8)&0xff, (sip)&0xff, sport,
				(dip>>24)&0xff, (dip>>16)&0xff, (dip>>8)&0xff, (dip)&0xff, dport);
	}
	else
	{
		if(isL4)
			TRACE("[IPv6][L3][%s] Src=0x%x(%d) Dest=0x%x(%d) L4Proto=0x%02x%s\n", (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?"US":"DS",
				sip, sport,
				dip, dport,
				l4Protocol, (l4Protocol == IPPROTO_TCP)?"TCP":((l4Protocol == IPPROTO_UDP)?"UDP":"Others"));
		else
			TRACE("[IPv6][L3][%s] Src=0x%x(%d) Dest=0x%x(%d) [NOT TCP/UDP]\n", (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM)?"US":"DS",
				sip, sport,
				dip, dport);
	}

	return SUCCESS;
}

int _rtk_fc_flowEntry_path34_setting(rtk_rg_asic_path1_entry_t *flowEntry, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_flowPath_param_t *path3_param , uint32 *pExtPMaskIdx)
{
	// ========================  ip header bridge flow ========================
	rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)flowEntry;
	rtk_fc_ext_port_mask_t extpmsk = 0;
	uint8 l4Protocol=0U;
	int flowMibEn= 0, flowMibIdx = 0;

	int highPriFlowPatternIdx;

	l4Protocol = pFcIngressData->l4protol;

	pFlowPath3->valid = TRUE;
	pFlowPath3->in_path = FB_PATH_34;
	pFlowPath3->in_multiple_act = FALSE;
	pFlowPath3->in_ipv4_or_ipv6 = pathDecision_var.flowPath_param.isIpv6;
	pFlowPath3->in_src_ipv4_addr =  pFcIngressData->srcIp;
	pFlowPath3->in_dst_ipv4_addr =  pFcIngressData->dstIp;
	pFlowPath3->in_l4_src_port = pFcIngressData->srcPort;		//PPTP passthrough, pFcIngressData->srcPort = 0
	pFlowPath3->in_l4_dst_port = pFcIngressData->dstPort;		//PPTP passthrouh, pFcIngressData->dstPort = 0
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	pFlowPath3->in_l4proto_num = l4Protocol;
#else
	pFlowPath3->in_l4proto = (l4Protocol == IPPROTO_TCP)?1:0;					//tcp: 1, udp: 0
#endif
	pFlowPath3->in_intf_idx = path3_param->igrNetifIdx;
	pFlowPath3->in_stagif = path3_param->igrSVlanTagif;
	pFlowPath3->in_ctagif = path3_param->igrCVlanTagif;
	pFlowPath3->in_pppoeif = path3_param->igrPPPoETagif;
	pFlowPath3->in_pppoe_sid_check = FALSE;				//2Note: for pppoe passthrough, it must turn off!!
	pFlowPath3->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS];
	pFlowPath3->in_tos = path3_param->igr_tos;
	pFlowPath3->in_cvlan_pri = pFcIngressData->srcCVlanPri;

	//-------------------------egress action-------------------------

	pFlowPath3->out_uc_lut_lookup = FALSE;		// tx port refer to portmask and ext_portmask_idx
	pFlowPath3->out_intf_idx = path3_param->egrNetifIdx;
	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) {
		RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK((1LL<<pPktHdr->egrWlanDevIdx), &extpmsk);
#if defined(CONFIG_FC_RTL9607C_SERIES)
		if(fc_db.controlFuc.wifi_tx_gmac_auto_sel_en && 
			path3_param->daIdx != SIGNED_INVALID && 
			fc_db.lutTbl[path3_param->daIdx]->duplicateEntry_exist && 
			fc_db.lutTbl[path3_param->daIdx]->wlan_dev_idx < RTK_FC_WLANX_END_INTF) {
			// extpmsk bit shift
			extpmsk <<= (6*(_rtk_fc_wlan_tx_lookup_gmac(pPktHdr->ingressPort.macPortIdx)));
		}
#endif
		if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(extpmsk, pExtPMaskIdx)!=RTK_FC_RET_OK)
			return FAILED;
		
		pFlowPath3->out_portmask = 0U;
		pFlowPath3->out_ext_portmask_idx = *pExtPMaskIdx;

	} else {	
		pFlowPath3->out_portmask = 1 << (pPktHdr->egressPort.macPortIdx);
		pFlowPath3->out_ext_portmask_idx = 0U;
	}
	pFlowPath3->out_smac_trans = path3_param->smacTrans;
	pFlowPath3->out_dmac_trans = 0U;
	//svlan
	pFlowPath3->out_egress_svid_act = path3_param->egrSVlanTagif ? TRUE : FALSE;
	pFlowPath3->out_svlan_id = path3_param->egrSVlanTagif ? pPktHdr->svlanid : 0U;
	pFlowPath3->out_stag_format_act = TRUE;
	pFlowPath3->out_svid_format_act = (path3_param->egrSVlanTagif)?TRUE:FALSE;
	pFlowPath3->out_spri_format_act = (path3_param->egrSVlanTagif)?TRUE:FALSE;
	pFlowPath3->out_spri = path3_param->egrSVlanTagif ? pPktHdr->svlanpri : 0U;
	//cvlan
	pFlowPath3->out_egress_cvid_act = TRUE;
	pFlowPath3->out_cvlan_id = path3_param->egrCVID;
	pFlowPath3->out_ctag_format_act = TRUE;
	pFlowPath3->out_cvid_format_act = (path3_param->egrCVlanTagif)?TRUE:FALSE;
	pFlowPath3->out_cpri_format_act = (path3_param->egrCVlanTagif)?TRUE:FALSE;
	pFlowPath3->out_cpri = path3_param->egrCVlanTagif ? pPktHdr->cvlanpri : 0U;

	//other
	pFlowPath3->out_multiple_act = FALSE;
	if(pPktHdr->remarkDec.outputQid)
		pFlowPath3->out_user_pri_act = TRUE;
	else
		pFlowPath3->out_user_pri_act = FALSE;
	pFlowPath3->out_user_priority = pPktHdr->remarkDec.outputQid;
	if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS])
	{
		// remarking dscp only when dscp as flow key
		pFlowPath3->out_dscp_act = TRUE;
		pFlowPath3->out_dscp = (path3_param->egr_tos>>2) & 0x3f;
	}

	pFlowPath3->out_stream_idx_act = pPktHdr->remarkDec.streamId_en;
	pFlowPath3->out_stream_idx = pPktHdr->remarkDec.streamId;

	pFlowPath3->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
	pFlowPath3->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

	pFlowPath3->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
	pFlowPath3->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;

	pFlowPath3->out_extra_tag_index = path3_param->tunnelExtraTagIdx;
	pFlowPath3->out_egress_port_to_vid_act = FALSE;
	pFlowPath3->out_drop = 0U;
	pFlowPath3->lock = (pFlowPath3->out_share_meter_act) ? TRUE: FALSE;

	flowMibEn = pFlowPath3->out_flow_counter_act;
	flowMibIdx = pFlowPath3->out_flow_counter_idx;
#if defined(CONFIG_FC_RTL9607C_SERIES)	
	if(fc_db.controlFuc.special_fast_forward_mode && dynamic_sram_desc ==0)
	{
		if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF && (pFlowPath3->out_user_priority == 5 || pFlowPath3->out_user_priority == 4 ) )
		{
			WARNING("DRAM mode wifi packet try to set priority 5 or 4! change to 3");
			pFlowPath3->out_user_priority = 3;
		}
	}
#endif
	if(_rtk_fc_highPriFlow_check(pFlowPath3, &highPriFlowPatternIdx))
	{
		TRACE("hit highPriFlowPattern[%d]", highPriFlowPatternIdx);
		pFlowPath3->lock = TRUE; //high priority flow
	}

	if(pFlowPath3->in_ipv4_or_ipv6==0)
	{
		TRACE("[IPv4][L2] Src=%d.%d.%d.%d(%d) Dest=%d.%d.%d.%d(%d) L4Proto=0x%02x(%s)\n",
			(pFlowPath3->in_src_ipv4_addr>>24)&0xff, (pFlowPath3->in_src_ipv4_addr>>16)&0xff, (pFlowPath3->in_src_ipv4_addr>>8)&0xff, (pFlowPath3->in_src_ipv4_addr)&0xff, pFlowPath3->in_l4_src_port,
			(pFlowPath3->in_dst_ipv4_addr>>24)&0xff, (pFlowPath3->in_dst_ipv4_addr>>16)&0xff, (pFlowPath3->in_dst_ipv4_addr>>8)&0xff, (pFlowPath3->in_dst_ipv4_addr)&0xff, pFlowPath3->in_l4_dst_port,
			l4Protocol, (l4Protocol == IPPROTO_TCP)?"TCP":((l4Protocol == IPPROTO_UDP)?"UDP":"Others"));
	}
	else
	{
		TRACE("[IPv6][L2] Src=0x%x(%d) Dest=0x%x(%d) L4Proto=0x%02x%s\n",
			pFlowPath3->in_src_ipv4_addr, pFlowPath3->in_l4_src_port,
			pFlowPath3->in_dst_ipv4_addr, pFlowPath3->in_l4_dst_port,
			l4Protocol, (l4Protocol == IPPROTO_TCP)?"TCP":((l4Protocol == IPPROTO_UDP)?"UDP":"Others"));
	}

	return SUCCESS;
}

int _rtk_fc_flowEntry_path12_setting(rtk_rg_asic_path1_entry_t *flowEntry, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_flowPath_param_t *path1_param,struct rt_nfconn *rtct, uint32 *pExtPMaskIdx)
{
	// ======================== path 1/2 ========================
	rtk_rg_asic_path1_entry_t *pFlowPath1 = (rtk_rg_asic_path1_entry_t *)flowEntry;
	rtk_fc_ext_port_mask_t extpmsk = 0;
	uint32 idx=0U;
	int flowMibEn= 0, flowMibIdx = 0;
	bool egrToPonPort;

	rtct->ct = NULL;

	pFlowPath1->valid = TRUE;
	pFlowPath1->in_path = FB_PATH_12;
	pFlowPath1->in_multiple_act = FALSE;

	pFlowPath1->in_intf_idx = path1_param->igrNetifIdx;
	pFlowPath1->in_stagif = path1_param->igrSVlanTagif;
	pFlowPath1->in_ctagif = path1_param->igrCVlanTagif;
	pFlowPath1->in_pppoeif = path1_param->igrPPPoETagif;
	pFlowPath1->in_pppoe_sid_check = TRUE;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	pFlowPath1->in_iphdrif = (pPktHdr->iph || pPktHdr->ip6h)?TRUE:FALSE;
#endif
	pFlowPath1->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS];
	pFlowPath1->in_tos = path1_param->igr_tos;
	pFlowPath1->in_smac_lut_idx = path1_param->saIdx;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)	
	if(path1_param->daIdx==fc_db.bcMacIdx)
		pFlowPath1->in_dmac_lut_idx=HW_BC_IDX;
	else
#endif		
		pFlowPath1->in_dmac_lut_idx = path1_param->daIdx;
	pFlowPath1->in_svlan_id = path1_param->igrSVID;
	pFlowPath1->in_cvlan_id = path1_param->igrCVID;
	pFlowPath1->in_pppoe_sid = pFcIngressData->sessionId;
	pFlowPath1->in_spa_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_SPA];
	pFlowPath1->in_spa = pPktHdr->ingressPort.macPortIdx;
	pFlowPath1->in_ext_spa = pPktHdr->ingressPort.macExtPortIdx;
#if defined(CONFIG_FC_WIFI_RX_HASH_SUPPORT) || defined(CONFIG_FC_WIFI_RX_GMAC_AUTO_SEL_SUPPORT)
	if(fc_db.lutTbl[path1_param->saIdx]->wlan_dev_idx < RTK_FC_WLANX_END_INTF)
		pFlowPath1->in_spa_check = FALSE;					// wifi spa may be 7,9,10
#endif	
	pFlowPath1->in_cvlan_pri = pFcIngressData->srcCVlanPri;

	//-------------------------egress action-------------------------
	pFlowPath1->out_uc_lut_lookup = FALSE;		// tx port refer to portmask and ext_portmask_idx
	pFlowPath1->out_intf_idx = path1_param->egrNetifIdx;	
	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) {
		RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK((1LL<<pPktHdr->egrWlanDevIdx), &extpmsk);
#if defined(CONFIG_FC_RTL9607C_SERIES)
		if(fc_db.controlFuc.wifi_tx_gmac_auto_sel_en &&
			path1_param->daIdx != SIGNED_INVALID && 
			fc_db.lutTbl[path1_param->daIdx]->duplicateEntry_exist &&
			fc_db.lutTbl[path1_param->daIdx]->wlan_dev_idx < RTK_FC_WLANX_END_INTF) {
			// extpmsk bit shift
			extpmsk <<= (6*(_rtk_fc_wlan_tx_lookup_gmac(pPktHdr->ingressPort.macPortIdx)));
		}
#endif
		if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(extpmsk, pExtPMaskIdx)!=RTK_FC_RET_OK)
			return FAILED;
		
		pFlowPath1->out_portmask = 0U;
		pFlowPath1->out_ext_portmask_idx = *pExtPMaskIdx;
	} else {
		pFlowPath1->out_portmask = 1 << (pPktHdr->egressPort.macPortIdx);
		pFlowPath1->out_ext_portmask_idx = 0U;
	}
	pFlowPath1->out_smac_trans = path1_param->smacTrans;
	pFlowPath1->out_dmac_trans = 0;
	//svlan
	pFlowPath1->out_egress_svid_act = path1_param->egrSVlanTagif ? TRUE : FALSE;
	pFlowPath1->out_svlan_id = path1_param->egrSVlanTagif ? pPktHdr->svlanid : 0U;
	pFlowPath1->out_stag_format_act = TRUE;
	pFlowPath1->out_svid_format_act = (path1_param->egrSVlanTagif)?TRUE:FALSE;
	pFlowPath1->out_spri_format_act = (path1_param->egrSVlanTagif)?TRUE:FALSE;
	pFlowPath1->out_spri =path1_param-> egrSVlanTagif ? pPktHdr->svlanpri : 0U;
	//cvlan
	pFlowPath1->out_egress_cvid_act = TRUE;
	pFlowPath1->out_cvlan_id = path1_param->egrCVID;
	pFlowPath1->out_ctag_format_act = TRUE;
	pFlowPath1->out_cvid_format_act = (path1_param->egrCVlanTagif)?TRUE:FALSE;
	pFlowPath1->out_cpri_format_act = (path1_param->egrCVlanTagif)?TRUE:FALSE;
	pFlowPath1->out_cpri = path1_param->egrCVlanTagif ? pPktHdr->cvlanpri : 0U;

	//other
	pFlowPath1->out_multiple_act = FALSE;
	if(pPktHdr->remarkDec.outputQid)
		pFlowPath1->out_user_pri_act = TRUE;
	else
		pFlowPath1->out_user_pri_act = FALSE;
	pFlowPath1->out_user_priority = pPktHdr->remarkDec.outputQid;
#if 1 // path 1 always disable dscp action
	if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS])
	{
		// remarking dscp only when dscp as flow key
		pFlowPath1->out_dscp_act = TRUE;
		pFlowPath1->out_dscp = (path1_param->egr_tos>>2)&0x3f;
	}
#endif
	pFlowPath1->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
	pFlowPath1->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

	pFlowPath1->out_extra_tag_index = path1_param->tunnelExtraTagIdx;
	pFlowPath1->out_egress_port_to_vid_act = FALSE;
	pFlowPath1->out_drop = 0U;
	pFlowPath1->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
	pFlowPath1->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;
	pFlowPath1->lock = (pFlowPath1->out_share_meter_act) ? TRUE: FALSE;

	flowMibEn = pFlowPath1->out_flow_counter_act;
	flowMibIdx = pFlowPath1->out_flow_counter_idx;

	//If ingress port is PON port, the in_stream_check should be enabled. If egress port is PON port, the out_stream_idx_act should be enabled.
	//in_stream_check and out_stream_idx_act share the same field in PATH 1 entry.
	egrToPonPort = FALSE;
	if(pFlowPath1->out_uc_lut_lookup)
	{
		if((1<<fc_db.lutTbl[path1_param->daIdx]->spa) & fc_db.wanPortMask.portmask)
			egrToPonPort = TRUE;

	}
	else
	{	// txport refer to out_portmask of flow entry
		if(pFlowPath1->out_portmask & fc_db.wanPortMask.portmask)
			egrToPonPort = TRUE;
	}
	if((1<<pFlowPath1->in_spa) & fc_db.wanPortMask.portmask)
	{

		pFlowPath1->in_out_stream_idx_check_act = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_STREAM_IDX];
		pFlowPath1->in_out_stream_idx = pFcIngressData->ponStreamId;
	}
	else if(egrToPonPort)
	{
		pFlowPath1->in_out_stream_idx_check_act = pPktHdr->remarkDec.streamId_en;
		pFlowPath1->in_out_stream_idx = pPktHdr->remarkDec.streamId;
	}
	else
	{
		pFlowPath1->in_out_stream_idx_check_act = 0U;
		pFlowPath1->in_out_stream_idx = 0U;
	}
	
#if defined(CONFIG_FC_RTL9607C_SERIES)	
	if(fc_db.controlFuc.special_fast_forward_mode && dynamic_sram_desc ==0)
	{
		if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF && (pFlowPath1->out_user_priority == 5 || pFlowPath1->out_user_priority == 4 ) )
		{
			WARNING("DRAM mode wifi packet try to set priority 5 or 4! change to 3");
			pFlowPath1->out_user_priority = 3;
		}
	}
#endif

	if(RTK_RG_ASIC_ETHTYPE_GET_HW_IDX(pPktHdr->ethtype, &idx)!=RTK_FC_RET_OK)
		return FAILED;
	else
		pFlowPath1->in_protocol = idx;
	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(pFlowPath1->in_dmac_lut_idx==HW_BC_IDX && pFlowPath1->in_smac_lut_idx<RTK_FC_TABLESIZE_LUT)
	{
		TRACE("[Non IP/TCP/UDP][L2] SA=%02x:%02x:%02x:%02x:%02x:%02x DA=ff:ff:ff:ff:ff:ff\n",
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[0],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[1],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[2],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[3],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[4],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[5]);
	}
	else
#endif		
	if(pFlowPath1->in_smac_lut_idx<RTK_FC_TABLESIZE_LUT && pFlowPath1->in_dmac_lut_idx<RTK_FC_TABLESIZE_LUT){
		TRACE("[Non IP/TCP/UDP][L2] SA=%02x:%02x:%02x:%02x:%02x:%02x DA=%02x:%02x:%02x:%02x:%02x:%02x\n",
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[0],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[1],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[2],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[3],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[4],
			fc_db.lutTbl[pFlowPath1->in_smac_lut_idx]->l2Addr[5],
			fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr[0],
			fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr[1],
			fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr[2],
			fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr[3],
			fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr[4],
			fc_db.lutTbl[pFlowPath1->in_dmac_lut_idx]->l2Addr[5]);
	}

	return SUCCESS;
}

rtk_fc_ret_t _rtk_fc_pathDecision_var_prepare(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *pFcIngressData, bool *if_egr_tos_remark)
{
	uint8 egrInsertV6Hdr_4in6 = 0U/*dslite & mape*/, is6RD = 0U;

	bzero(&pathDecision_var, sizeof(pathDecision_var));

	pathDecision_var.flowPath_param.saIdx = pPktHdr->smacL2Idx;
	pathDecision_var.flowPath_param.daIdx = pPktHdr->dmacL2Idx;
	pathDecision_var.flowPath_param.smacTrans = (pPktHdr->fwdType >= RTK_FC_FWDTYPE_ROUTING_BASED)? TRUE:FALSE;
	pathDecision_var.flowPath_param.isIpv6 = (pPktHdr->ip6h) ? TRUE : FALSE;

	if(!pFcIngressData->isDualHeader)
	{
		if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IP4INIP6) // single -> dual && it is ds-lite packet: the IPv6 header is added at egress
		{
			pathDecision_var.flowPath_param.isIpv6 = FALSE;
			egrInsertV6Hdr_4in6 = TRUE;
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_6RD) // single -> dual && it is 6rd packet: the IPv4 header is added at egress
		{
			pathDecision_var.flowPath_param.isIpv6 = TRUE;
			is6RD = TRUE;
		}
	}

	if(pFcIngressData->isDAGatewayMAC && !pPktHdr->isMulticast && ((pPktHdr->iph) || (pPktHdr->ip6h)))
	{
		// l34 routing / napt
		if((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM) && pPktHdr->iph && (ntohl(pPktHdr->iph->saddr) != pFcIngressData->srcIp) && (!is6RD)) // for ipv4 up-stream
			pathDecision_var.flowPath_param.isNAPT = TRUE;
		else if ((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM) && pPktHdr->ip6h &&  memcmp(&pFcIngressData->ingress_v6ip_info.s6_addr32[0], &pPktHdr->ip6h->saddr.s6_addr32[0], sizeof(struct in6_addr)) && (!egrInsertV6Hdr_4in6)) // for ipv6 up-stream
			pathDecision_var.flowPath_param.isNAPT = TRUE;

		if((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM) && pPktHdr->iph && (ntohl(pPktHdr->iph->daddr) != pFcIngressData->dstIp)) // for ipv4 down-stream
			pathDecision_var.flowPath_param.isNAPT = TRUE;
		else if((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM) && pPktHdr->ip6h && memcmp(&pFcIngressData->ingress_v6ip_info.s6_addr32[0], &pPktHdr->ip6h->daddr.s6_addr32[0], sizeof(struct in6_addr)))// for ipv6 down-stream
			pathDecision_var.flowPath_param.isNAPT = TRUE;

		if(pPktHdr->fwdType == RTK_FC_FWDTYPE_NATLOOPBACK)
			pathDecision_var.flowPath_param.isNAPT = TRUE;
	}

	if( (!pFcIngressData->isDualHeader && (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE || pPktHdr->dualHdrType==RTK_FC_DUALTYPE_MAPT) && pPktHdr->isMAPT))
	{
		pathDecision_var.flowPath_param.isMAPT = TRUE;
	}
	else if (!pFcIngressData->isDualHeader && (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE || pPktHdr->dualHdrType==RTK_FC_DUALTYPE_XLAT464) && pPktHdr->isXLAT)
	{
		pathDecision_var.flowPath_param.para_isXLAT = TRUE;
	}
	pathDecision_var.flowPath_param.igr_tos = (pFcIngressData->ingressTagif & IPV4_TAGIF)?pFcIngressData->v4tos:((pFcIngressData->ingressTagif & IPV6_TAGIF)?pFcIngressData->v6tos:0);
	pathDecision_var.flowPath_param.egr_tos = (pPktHdr->iph)?pPktHdr->iph->tos:((pPktHdr->ip6h)?(((pPktHdr->ip6h->priority<<4)&0xf0)|((pPktHdr->ip6h->flow_lbl[0]>>4)&0xf)):0);
	if((pathDecision_var.flowPath_param.igr_tos & 0x3) != (pathDecision_var.flowPath_param.egr_tos & 0x3))
		*if_egr_tos_remark = TRUE;

	// VLAN
	if(pFcIngressData->ingressTagif & SVLAN_TAGIF)
		pathDecision_var.flowPath_param.igrSVlanTagif = TRUE;
	if(pFcIngressData->ingressTagif & CVLAN_TAGIF)
		pathDecision_var.flowPath_param.igrCVlanTagif = TRUE;
	if(pFcIngressData->ingressTagif & PPPOE_TAGIF)
		pathDecision_var.flowPath_param.igrPPPoETagif = TRUE;

	pathDecision_var.flowPath_param.igrSVID = pathDecision_var.flowPath_param.igrSVlanTagif ? pFcIngressData->srcSVlanId : 0;
	pathDecision_var.flowPath_param.igrCVID = pathDecision_var.flowPath_param.igrCVlanTagif ? pFcIngressData->srcCVlanId : 0;
	pathDecision_var.flowPath_param.egrSVlanTagif = (pPktHdr->svh ? TRUE : FALSE);
	pathDecision_var.flowPath_param.egrCVlanTagif = (pPktHdr->cvh ? TRUE : FALSE);
	pathDecision_var.flowPath_param.egrCVID = pathDecision_var.flowPath_param.egrCVlanTagif ? (pPktHdr->cvlanid ? pPktHdr->cvlanid : 0):0 ;

	return RTK_FC_RET_OK;
}

rtk_fc_ret_t _rtk_fc_hardware_not_support_check(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *pFcIngressData, bool if_egr_tos_remark)
{
	bool isL3 = ((pPktHdr->iph) || (pPktHdr->ip6h)) ? TRUE : FALSE;

#if !defined(CONFIG_RTK_L34_G3_PLATFORM)
	if((pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE) && (pPktHdr->fwdType ==RTK_FC_FWDTYPE_NAPT) && !((pPktHdr->tcph) || (pPktHdr->udph)))
	{
		TRACE("NAPT but not TCP/UDP/DUAL_PASSTHROUGH - skip");
		return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
	}
#endif

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	if((pPktHdr->fwdType == RTK_FC_FWDTYPE_NATLOOPBACK) && (!fc_db.controlFuc.if_hash_fib_384_bits))
#else
	if(pPktHdr->fwdType == RTK_FC_FWDTYPE_NATLOOPBACK)
#endif
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; //support in shortcut only

	if(pathDecision_var.flowPath_param.smacTrans && !isL3) {
			TRACE("Not support direction %d without L3 hdr!", pPktHdr->direction);
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
	}

	if(pathDecision_var.flowPath_param.smacTrans && 
		(pPktHdr->iph || pPktHdr->ip6h) && (pPktHdr->tcph || pPktHdr->udph) &&
		!(pFcIngressData->isDualHeader && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE ) && //Not down stream dual header
		!(!pFcIngressData->isDualHeader && pPktHdr->dualHdrType!=RTK_FC_DUALTYPE_NONE) &&//Not up stream dual header
		(pPktHdr->dual_passthrough_type != RTK_FC_DUALTYPE_PPTP)						// pptp passthrough FcIngressData is from inner
	)
	{
		uint16 dport =(pPktHdr->tcph)?ntohs(pPktHdr->tcph->dest):ntohs(pPktHdr->udph->dest);
		uint16 sport =(pPktHdr->tcph)?ntohs(pPktHdr->tcph->source):ntohs(pPktHdr->udph->source);
		uint32 sip = (pPktHdr->iph)?ntohl(pPktHdr->iph->saddr):(pPktHdr->ipv6Sip_hash);
		uint32 dip = (pPktHdr->iph)?ntohl(pPktHdr->iph->daddr):(pPktHdr->ipv6Dip_hash);

		if(!pFcIngressData->isDualHeader && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE && (pPktHdr->isMAPT || pPktHdr->isXLAT))
		{
			TRACE("support MAP-T translation, continue.");
		}
		else if(pPktHdr->fwdType == RTK_FC_FWDTYPE_NATLOOPBACK)
		{
			TRACE("support NAT Loopback translation, continue.");
		}
		else if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM &&  (dip != pFcIngressData->dstIp))
		{
			TRACE("Not support upstream change dip!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
		else if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM &&  (dport != pFcIngressData->dstPort))
		{
			TRACE("Not support upstream change dport!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
		else if(pPktHdr->direction== RTK_FC_FLOW_DIRECTION_DOWNSTREAM && (sip != pFcIngressData->srcIp))
		{
			TRACE("Not support downstream change sip!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
		else if(pPktHdr->direction== RTK_FC_FLOW_DIRECTION_DOWNSTREAM && (sport != pFcIngressData->srcPort))
		{
			TRACE("Not support downstream change sport!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
	}

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_LANBRIDGE )
	{
		uint32 sip = (pPktHdr->iph)?ntohl(pPktHdr->iph->saddr):(pPktHdr->ipv6Sip_hash);
		uint32 dip = (pPktHdr->iph)?ntohl(pPktHdr->iph->daddr):(pPktHdr->ipv6Dip_hash);

		if((sip != pFcIngressData->srcIp))
		{
			TRACE("Not support bridge change sip!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
		else if((dip != pFcIngressData->dstIp))
		{
			TRACE("Not support bridge change dip!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
	}

	//2 TCP state control
	if(pPktHdr->tcph) {
		// fin or rst => control pkt do nothing
		if((pPktHdr->tcph->fin==1) || (pPktHdr->tcph->rst==1)) {

			TRACE("TCP control packet no ct, skip learning");
			return RTK_FC_RET_LRN_FAIL;
		}
	}

	if(pFcIngressData->ingressTagif & ICMP_TAGIF  )
	{
		if(pPktHdr->dual_passthrough_type != RTK_FC_DUALTYPE_NONE || pPktHdr->isMAPT || pPktHdr->isXLAT ||
		   (pFcIngressData->isDualHeader && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE) || //dual header down stream
		   (!pFcIngressData->isDualHeader && pPktHdr->dualHdrType!=RTK_FC_DUALTYPE_NONE) || //dual header up stream 
		   (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM && pPktHdr->ip6h && memcmp(&pFcIngressData->ingress_v6ip_info.s6_addr32[0], &pPktHdr->ip6h->saddr.s6_addr32[0], sizeof(struct in6_addr)) ) || //nptv6 upstream
		   (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM && pPktHdr->ip6h && memcmp(&pFcIngressData->ingress_v6ip_info.s6_addr32[0], &pPktHdr->ip6h->daddr.s6_addr32[0], sizeof(struct in6_addr)) ) //nptv6 downstream
		)
		{
			DEBUG("Not Support dual header/NPTv6/MAP-T/dual passthrough ICMP acceleration!");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}
	}

	/* ========== Then, check if flow is supported in sw only. ==========*/

	if(fc_db.controlFuc.hwnat_mode == RT_FLOW_HWNAT_MODE_SW_ONLY) //sw only mode
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;

	if(fc_db.controlFuc.hwnat_mode == RT_FLOW_HWNAT_MODE_DIS_UPSTREAM && pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		return RTK_FC_RET_LRN_FAIL;

	if(pFcIngressData->ingressTagif & ICMP_TAGIF)
	{
		if(pFcIngressData->isDAGatewayMAC) {
			TRACE("ICMP!! Keep learning, swOnly");
			return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; //support in shortcut only
		}else{
			TRACE("ICMP!! disable learning");
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT; 		//disable acceleration
		}
	}

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	if(pPktHdr->tcph && fc_db.tcp_in_window_shortcut_check)
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
#endif

	if(if_egr_tos_remark)
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;

	if (pFcIngressData->ipFragFlag) {
		if (pFcIngressData->ipFragFlag & RTK_FC_IP_FIRST_FRAG) {
			if (pPktHdr->udph && pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM && pFcIngressData->dstPort != ntohs(pPktHdr->udph->dest)) {
				FRAGMENT("first ip frag packet, iph id=0x%x! but change dport, disable learning!", pFcIngressData->ipId);
				return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT; 		//disable acceleration
			}

			FRAGMENT("first ip frag packet, flowIdx=0x%x, iph id=0x%x!", pFcIngressData->flowHashIdx, pFcIngressData->ipId);
			return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
		}
		else {
			FRAGMENT("second and later ip frag packet, iph id=0x%x!, disable learning!", pFcIngressData->ipId);
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT; 		//disable acceleration
		}
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(pPktHdr->stpid_sel)
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; // HW SVLAN action supports svlan_tpid_sel 0 only

	if((!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI]) && (pFcIngressData->ingressTagif & SVLAN_TAGIF))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; // HW not support SPRI as flow key
#endif

#if !defined(CONFIG_FC_RTL9607F_SERIES)
	if(pFcIngressData->isDualHeader && (pFcIngressData->ingressTagif & GRE_ETH_BR_TAGIF) && (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE))
	{
		//GRE transparent ethertype bridge downstream
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; //support in shortcut only
	}
	else if((pFcIngressData->isDualHeader == FALSE) && (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_GRE_ETH_BR))
	{
		//GRE transparent ethertype bridge upstream
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; //support in shortcut only
	}
#endif

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_RTL9607F_SERIES)
	//do nothing, support HW for PPTP
#elif defined(CONFIG_FC_RTL8277C_SERIES)
	if((pFcIngressData->ingressTagif & PPTP_TAGIF) && (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP))
	{
		//do nothing, support HW for PPTP passthrough
	}
	else if((pFcIngressData->ingressTagif & PPTP_TAGIF) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; // make PPTP flow add to shortcut only
#else
	if((pFcIngressData->ingressTagif & PPTP_TAGIF) && (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP))
	{
		//do nothing, support HW acceleartion for PPTP passthrough
	}
	else if((pFcIngressData->ingressTagif & L2TP_TAGIF) && (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_L2TP))
	{
		//do nothing, support HW acceleartion for L2TP passthrough
	}
	else if((pFcIngressData->ingressTagif & V6RD_TAGIF) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_6RD) ||
		(pFcIngressData->ingressTagif & IP4_IN_IP6) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_IP4INIP6) ||
		(pFcIngressData->ingressTagif & PPTP_TAGIF) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP) ||
		(pFcIngressData->ingressTagif & L2TP_TAGIF) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_L2TP))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; // make dual header flow add to shortcut only
#endif
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(fc_db.controlFuc.dsliteHwAcceleration_disable &&
		((pFcIngressData->ingressTagif & IP4_IN_IP6) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_IP4INIP6)))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; // make ds-lite flow add to shortcut only when fc_db.controlFuc.dsliteHwAcceleration_disable is enabled.
	if( fc_db.controlFuc.dsliteV6TosFromV4     && 
		((pFcIngressData->ingressTagif & IP4_IN_IP6) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_IP4INIP6)) &&
		pFcIngressData->v4tos!=0 )
	{
		// we only do hardware accelerate v4Tos==0 other v4Tos need forward by shortcut
		DSLITE(" dsliteV6TosFromV4 enable and v4tos!=0 software shortvut only entry ");
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
	}

	if((pFcIngressData->ingressTagif & V6RD_TAGIF) || (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_6RD))
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION; // make 6RD flow add to shortcut only.
#endif

#if defined(CONFIG_FC_RTL9607C_SERIES)
	// For VXLAN/NPTv6 nic acceleration:
	// If VXLAN/NPTv6 nic acceleration is set, set wifi to shortCut only to prevent wrong data path
	if( fc_db.controlFuc.special_fast_forward_mode==1 &&
		(pPktHdr->igrWlanDevIdx != RTK_FC_WLANX_NOT_FOUND || pPktHdr->egrWlanDevIdx != RTK_FC_WLANX_NOT_FOUND)
	)
	{
		int i;
		int special_fastFwd_isSet =0;
		for(i=0; i<MAX_NPTV6_ACC_UPSTREAM_SIZE+MAX_NPTV6_ACC_DOWNSTREAM_SIZE; i++)
		{
			if(fc_db.nptv6_flow_info[i].isSet==1)
			{
				special_fastFwd_isSet =1;
				break;
			}
		}
		for(i =0; i<4; i++)
		{
			if(fc_db.vxlan_upStream_record[i].isSet ==1 || fc_db.vxlan_downStream_record[i].isSet ==1)
			{
				special_fastFwd_isSet =1;
				break;
			}
		}

		if(special_fastFwd_isSet)
		{
			return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
		}
	}
#endif

	if (pPktHdr->force_to_bridge_flow) {
		TRACE("mac clone bridge forwarding!! Keep learning , swOnly");
		return RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION;
	}

	return RTK_FC_RET_OK;
}

int rtk_fc_egress_pathDecision(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{
	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);
	struct rt_nfconn *pRt_ct = NULL;
	//uint8 protonum = 0U;
	uint32 flowIdx = FAILED ,reverseFlowIdx=FAILED,flowIdxMultiAction = FAILED ;
	rtk_fc_forwardingType_t fwdType = pPktHdr->fwdType;
	bool isL3 = ((pPktHdr->iph) || (pPktHdr->ip6h)) ? TRUE : FALSE;
	bool isL4 = ((pPktHdr->tcph) || (pPktHdr->udph)) ? TRUE : FALSE;
	uint32 _extPmskIdx=0U;
	bool swOnly, bypass_flowDelay = FALSE;
	bool pppoeGatewayL2Idx_updateByDa = FALSE, pppoeGatewayL2Idx_updateBySa = FALSE; //if is needed to update lut index of pppoe gateway mac
	bool if_verify_fdb = TRUE;
	bool wanDsLoopback_en = FALSE; // wan downstream enable, support  bridge only
	int16 igrNetifIdx = SIGNED_INVALID, egrNetifIdx = SIGNED_INVALID;	//hw
	int16 swIgrNetifIdx = SIGNED_INVALID, swEgrNetifIdx = SIGNED_INVALID;	//sw
	int outer_cvid= 0;
	int16  outer_daIdx=0;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	int flowMibEn = 0, flowMibIdx = FAIL;
#endif

#if defined(CONFIG_FC_RTL9607C_SERIES)
	uint8 flow_can_add = 1U;
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	bool isMcPassthrough = (!pPktHdr->isMulticast && ((pPktHdr->iph && FLOW_IPV4_MULTICAST_ADDRESS(ntohl(pPktHdr->iph->daddr))) ||(pPktHdr->ip6h && FLOW_IPV6_MULTICAST_ADDRESS(ntohl(pPktHdr->ip6h->daddr.s6_addr32[0])))))?TRUE:FALSE;
#endif
	bool if_egr_tos_remark = FALSE;
	rt_tcp_conntrack_state_t rt_tcp_state;

	
	int ret=RTK_FC_RET_ERR;

	swOnly = FALSE;

	//2 passthrough control
	if((pFcIngressData->ingressTagif & PPTP_TAGIF) && (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP))
	{
		/*
		PPTP passthrough:
		bridge: no L4 header, path1
		routing: SW only, path5		(HW not support path5 with no L4 header)
		NAPT: SW only, path 5		(HW not support path5 with no L4 header. Otherwise, downstream flow need SW mapping internal IP/PORT with callID)
		*/
		if(pFcIngressData->isDAGatewayMAC)
		{
			pPktHdr->dual_passthrough_type = RTK_FC_DUALTYPE_PPTP; // support PPTP passthrough
			pPktHdr->iph = pPktHdr->outer_iph;
			pPktHdr->ip6h = NULL;
            pPktHdr->udph = NULL;
			pPktHdr->tcph = NULL;							//inner L4 header may be TCP 
			pPktHdr->icmph = NULL;
			isL4 = FALSE;									//PPTP => pure IP routing/NAPT
			pPktHdr->l4protol = pPktHdr->outer_l4proto;
		}
	
		TRACE("PPTP passthrough");
	}
	else if((pFcIngressData->ingressTagif & L2TP_TAGIF) && (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP))
	{
		/*
		L2TP passthrough:
		bridge: path3
		routing: HW, path5
		NAPT: SW only, path 5	(downstream flow need SW mapping internal IP/PORT with tunnelID/sessionID)
		*/
		pPktHdr->dual_passthrough_type = RTK_FC_DUALTYPE_L2TP; // support L2TP passthrough
		pPktHdr->iph = pPktHdr->outer_iph;
		pPktHdr->ip6h = NULL;
        pPktHdr->udph = pPktHdr->outer_udph;
        pPktHdr->tcph = NULL;                                                   //inner L4 header may be TCP
        pPktHdr->icmph = NULL;
		isL4 = TRUE;									//For inner non-TCP/UDP
		pPktHdr->l4protol = pPktHdr->outer_l4proto;
		TRACE("L2TP passthrough");
	}
	else if((pFcIngressData->ingressTagif & IPSEC_ESP_TAGIF) && (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IPSEC))
	{
		if(pFcIngressData->isDAGatewayMAC)
		{
			pPktHdr->dual_passthrough_type = RTK_FC_DUALTYPE_IPSEC;
			TRACE("IPsec passthrough");
		}
	}

	//--------------For pathDecision_var prepare-------------------------------------------------------
	_rtk_fc_pathDecision_var_prepare(pPktHdr, pFcIngressData, &if_egr_tos_remark);
	pRt_ct = &pathDecision_var.rtct;
	//--------------------------------------------------------------------------------------------------
	//--------------For hardware not support behavior---------------------------------------------------
	ret = _rtk_fc_hardware_not_support_check(pPktHdr, pFcIngressData, if_egr_tos_remark);

	if(ret ==RTK_FC_RET_OK) {
		// hw support
	}else if(ret == RTK_FC_RET_LRN_DISABLE_HW_ACCELERATION)
		swOnly = TRUE;
	else
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	
	//--------------------------------------------------------------------------------------------------

//3FLOWPATH_DECISION:

	//2 ct / conntrack control
	// decide the ct refered by flow entry, it may be NULL or come from netfilter hook.
	if((ret = rtk_fc_flow_ct_decision(rtskb, pPktHdr, &pRt_ct)) != RTK_FC_RET_OK) {
		//WARNING("detect CT exist but dying state, skip flow learning");
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}
	ret = _rtk_fc_flow_ct_decision(pRt_ct, pPktHdr, &rt_tcp_state, &swOnly);
	if(ret == FAILED)
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;

	rtk_fc_internal_vid_fdb_find(pPktHdr, rtskb->skb);


	if((pFcIngressData->ingressTagif & PPPOE_TAGIF) && pFcIngressData->isDAGatewayMAC)
		pppoeGatewayL2Idx_updateBySa = TRUE;
	if(pPktHdr->ppph && pFcIngressData->isDAGatewayMAC)
		pppoeGatewayL2Idx_updateByDa = TRUE;

	// Lut - only unicast need check saIdx/daIdx


	//2 SA find/learn
	if_verify_fdb = pppoeGatewayL2Idx_updateBySa?FALSE:TRUE; 				// PPPoE gateway MAC will not exist in FDB/neighbor, thus no need to verify FDB/neighbor
	if(fc_db.controlFuc.flow_skipAllPsTracking || fc_db.controlFuc.l2_skipPsTracking) if_verify_fdb = FALSE;			// force skip
	if(pFcIngressData->isDualHeader 
		&& pFcIngressData->ingressTagif & VXLAN_TAGIF 
		&& pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE) if_verify_fdb = FALSE;
	if((pathDecision_var.flowPath_param.saIdx == SIGNED_INVALID) || 																// SA needs to be learned (l2_macEgrLearning==1)
		(fc_db.lutTbl[pathDecision_var.flowPath_param.saIdx] && (fc_db.lutTbl[pathDecision_var.flowPath_param.saIdx]->isSync2PsEntry==FALSE) && (if_verify_fdb==TRUE))) {	// SA needs to be updated (l2_macEgrLearning==0)

		if((ret = _rtk_fc_lut_learning(pFcIngressData->sa, pPktHdr->ingressPort, pFcIngressData->srcCVlanId, pPktHdr->igrWlanDevIdx,
										if_verify_fdb, FALSE, &pPktHdr->smacL2Idx, pPktHdr,LUT_DIR_SA_LEARNING)) != RTK_FC_RET_OK) {
			TRACE("LUT SA learning is fail, ret = 0x%x", ret);
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
		}
		pathDecision_var.flowPath_param.saIdx = pPktHdr->smacL2Idx;
		if((pPktHdr->smacL2Idx != SIGNED_INVALID) && fc_db.lutTbl[pPktHdr->smacL2Idx])
			pPktHdr->smacHostPolIdx = fc_db.lutTbl[pPktHdr->smacL2Idx]->hostPolIdx;
	}

	//2 DA find/learn
	if_verify_fdb = pppoeGatewayL2Idx_updateByDa?FALSE:TRUE; 				// PPPoE gateway MAC will not exist in FDB/neighbor, thus no need to verify FDB/neighbor
	if(fc_db.controlFuc.flow_skipAllPsTracking || fc_db.controlFuc.l2_skipPsTracking) if_verify_fdb = FALSE;			// force skip
	

	if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN )
	{
		// VXLAN remote lut set to static in case of avoiding be delete by RTNL DEL-NEIGH
		if((ret = _rtk_fc_lut_learning(pPktHdr->eth->h_dest, pPktHdr->egressPort, pPktHdr->cvlanid, pPktHdr->egrWlanDevIdx,
										if_verify_fdb, FALSE, &pPktHdr->dmacL2Idx,NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
		}
	}
	else
	{
		uint16 vid;
		// check unknownDA/uuc flooding to prevent incorrect mac learning
	#if defined(CONFIG_RTK_SOC_RTL8198D)
		if (pFcIngressData->skbCloned)
	#else
		if((fwdType == RTK_FC_FWDTYPE_BRIDGE) && 
			pFcIngressData->skbCloned)
	#endif
		{
			rtk_fc_fdb_entry_t fc_fdb;

			if( (_rtk_fc_lookup_fdb_force(pPktHdr->eth->h_dest, pPktHdr->cvlanid, &fc_fdb)==FALSE)) {
				// fdb and Neighbor(v4/v6) are all not found. Skip learning
				if(pPktHdr->outDev_internal_vid_fdb_find == TRUE)
					TRACE("Find by pvid!");
				else
				{
					TRACE("bridge packet may flooding by PS, skip learning");
					goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
				}
			}
#if defined(CONFIG_RTL_SMUX_DEV)
			else {
				rtk_fc_realdev_t realdev;
				if((RTK_FC_HOOK_PS_DEV_GET_REALDEV(fc_fdb.br_port_netdev, &realdev) == RTK_FC_RET_OK) &&
					realdev.rdev == pPktHdr->dstDev) {
					// find
					DEBUG("bridge packet is cloned by PS, DA fdb is belong to %s(%s) matching with current egress dev %s", 
						fc_fdb.br_port_netdev->name, realdev.rdev->name, pPktHdr->dstDev->name);
					
				}else {
					TRACE("bridge packet is cloned by PS, DA fdb is belong to %s(%s) but current egress dev is %s, skip learning", 
						fc_fdb.br_port_netdev->name, realdev.rdev->name, pPktHdr->dstDev->name);
					goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
				}
			}
#endif
		}

			
#if defined(CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT)	
			if(fc_db.controlFuc.external_switch_en && pPktHdr->egressPort.macPortIdx == CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT)
			{

				vid = pPktHdr->cvlanid;
				
				pPktHdr->egressPort.macExtPortIdx = pPktHdr->svlanid - fc_db.external_switch_vlan_start;
			}
			else
#endif	
			{
				if(pPktHdr->svlanid!=0)
					vid = pPktHdr->svlanid;
				else
					vid = pPktHdr->cvlanid;
			}
		// un-learned DA or needs to sync PS. Alway go to lut learning function to check port moving.
		if((ret = _rtk_fc_lut_learning(pPktHdr->eth->h_dest, pPktHdr->egressPort, vid, pPktHdr->egrWlanDevIdx,
										if_verify_fdb, FALSE, &pPktHdr->dmacL2Idx, pPktHdr,LUT_DIR_DA_LEARNING)) != RTK_FC_RET_OK) {
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
		}
		
	}

	/* ========== Check if flow is not supported first. ==========*/
	if(unlikely((pFcIngressData->flowHashIdx < 0) || (pFcIngressData->flowHashIdx >= fc_db.flowSwTableSize)))
	{
		TRACE("Flow hash index[%d] is invalid", pFcIngressData->flowHashIdx);
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}
	
	pathDecision_var.flowPath_param.daIdx = pPktHdr->dmacL2Idx;
	if((pPktHdr->dmacL2Idx != SIGNED_INVALID) && fc_db.lutTbl[pPktHdr->dmacL2Idx])
		pPktHdr->dmacHostPolIdx = fc_db.lutTbl[pPktHdr->dmacL2Idx]->hostPolIdx;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	// forbidden if PON_mode & to_WAN & !sid
	if(fc_db.systemGlobal.pon_sidTbl_en && 
		((pPktHdr->dmacL2Idx != SIGNED_INVALID) && fc_db.lutTbl[pPktHdr->dmacL2Idx]) && (fc_db.lutTbl[pathDecision_var.flowPath_param.daIdx]->spa == RTK_FC_MAC_PORT_PON) &&
		!pPktHdr->remarkDec.streamId_en) {
		WARNING("egress to %s with DMAC %pM spa %d and no streamid assign",  pPktHdr->dstDev->name, &fc_db.lutTbl[pathDecision_var.flowPath_param.daIdx]->l2Addr[0], RTK_FC_MAC_PORT_PON);
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}
#endif

	if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN ) // upstream
	{
		
		TRACE("[VXLAN]Now learning outer da");
		if(pPktHdr->outer_cvh)
			outer_cvid = (ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK);

		if(_rtk_fc_lut_find(pPktHdr->outer_eth->h_dest, &outer_daIdx) == RTK_FC_RET_OK)
		{
			DEBUG("[VXLAN]outer da LUT Learned before, no need to learn again.");
		}
		else if((ret = _rtk_fc_lut_learning(pPktHdr->outer_eth->h_dest, pPktHdr->egressPort, outer_cvid, pPktHdr->egrWlanDevIdx,
										FALSE, FALSE, &outer_daIdx, NULL,LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) 
		{
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
		}
		//pPktHdr->outer_dmacL2Idx = outer_daIdx;
		

	}


	//3 Netif decision
	if(pPktHdr->remarkDec.swIntfIdx_assign_en)
	{
		if(pPktHdr->remarkDec.swIntfIdx_assign_inOut == 0U)
		{
			swIgrNetifIdx = pPktHdr->remarkDec.swIntfIdx_assign_idx;
			TRACE("decide igrNetifIdx:%d by skbmark", swIgrNetifIdx);
		}
		else
		{
			swEgrNetifIdx = pPktHdr->remarkDec.swIntfIdx_assign_idx;
			TRACE("decide igrNetifIdx:%d by skbmark", swEgrNetifIdx);
		}
	}


	if(pPktHdr->dstDev)
	{

		TRACE("[Before] flow was forwarded to %s (%s) dir=%s", pPktHdr->dstDev->name, (fwdType > RTK_FC_FWDTYPE_BRIDGE)?(fwdType==RTK_FC_FWDTYPE_ROUTING?"routing":"napt"):"bridge",pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM?"UPSTREAM":"DOWNSTREAM");


		if(swIgrNetifIdx == SIGNED_INVALID)
			swIgrNetifIdx = rtk_fc_getIngressSwNetifIdx(pPktHdr, rtskb->skb, fwdType, pPktHdr->direction);
		if(swEgrNetifIdx == SIGNED_INVALID)
			swEgrNetifIdx = rtk_fc_getEgressSwNetifIdx(pPktHdr, rtskb->skb, fwdType, pPktHdr->direction);

		if(swIgrNetifIdx==SIGNED_INVALID || swEgrNetifIdx==SIGNED_INVALID)
		{
			TRACE("[Fail to add flow] swIgrNetifIdx=%d swEgrNetifIdx=%d",swIgrNetifIdx, swEgrNetifIdx);
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
		}
		else
			TRACE("[After] flow was forwarded from swIgrNetifIdx:%d to swEgrNetifIdx:%d", swIgrNetifIdx, swEgrNetifIdx);
	}
	else
	{
		WARNING("no dev");
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}
	if(fc_db.netifTbl[swIgrNetifIdx].hwIdx==SIGNED_INVALID || fc_db.netifTbl[swEgrNetifIdx].hwIdx==SIGNED_INVALID)
	{
		DEBUG("Hardware igrNetifIdxHW:%d  egrNetifIdxHW:%d not ready",fc_db.netifTbl[swIgrNetifIdx].hwIdx,fc_db.netifTbl[swEgrNetifIdx].hwIdx);
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}
	if((pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM && fc_db.netifTbl[swEgrNetifIdx].dualType!=RTK_FC_DUALTYPE_NONE && fc_db.netifTbl[swEgrNetifIdx].dualUniInfo_syncedUP==0) ||
	   (pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM && fc_db.netifTbl[swIgrNetifIdx].dualType!=RTK_FC_DUALTYPE_NONE && fc_db.netifTbl[swIgrNetifIdx].dualUniInfo_syncedDOWN==0))
	{
		TRACE("netif dualInfo not ready skip learning");
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}

	
	igrNetifIdx = fc_db.netifTbl[swIgrNetifIdx].hwIdx;
	egrNetifIdx =  fc_db.netifTbl[swEgrNetifIdx].hwIdx;
	pathDecision_var.flowPath_param.igrNetifIdx=igrNetifIdx;
	pathDecision_var.flowPath_param.egrNetifIdx=egrNetifIdx;

	if(fc_db.netifTbl[swEgrNetifIdx].dualType != RTK_FC_DUALTYPE_NONE && pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && fc_db.netifTbl[swIgrNetifIdx].outerHdrExtratagIdx!=SIGNED_INVALID) 
		pathDecision_var.flowPath_param.tunnelExtraTagIdx=fc_db.netifTbl[swIgrNetifIdx].outerHdrExtratagIdx;

	// Find ExtraTag index if egress with Dual Hdr
	if(fc_db.netifTbl[swEgrNetifIdx].dualType != RTK_FC_DUALTYPE_NONE && pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM) 
	{
		DEBUG("[Dual] EgrIf dual header type %d  %s", fc_db.netifTbl[swEgrNetifIdx].dualType,name_of_dualHdrType[fc_db.netifTbl[swEgrNetifIdx].dualType]);

		if(pFcIngressData->dual_hdr_cnt > 0 && fc_db.netifTbl[swEgrNetifIdx].dualType != RTK_FC_DUALTYPE_VXLAN)
		{
			TRACE("[Dual] Not support multiple dual header format, dual cnt:%d", pFcIngressData->dual_hdr_cnt);
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
		}
		ret = rtk_fc_dual_hw_setting(pPktHdr, rtskb->skb, egrNetifIdx, &pathDecision_var.flowPath_param.tunnelExtraTagIdx);

		if(ret != SUCCESS)
		{	
			if(fc_db.netifTbl[swEgrNetifIdx].dualType == RTK_FC_DUALTYPE_MAPT || (fc_db.netifTbl[swEgrNetifIdx].dualType == RTK_FC_DUALTYPE_XLAT464))
			{
				//mapt software support
				swOnly = TRUE;
				pathDecision_var.flowPath_param.isIpv6 = FALSE;	//upstream match flow before transform=>v4
			}
			else
			{
				TRACE("[Dual] Fail to get extra tag index, ret = %d", ret);
				goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
			}
		}else{
			if(fc_db.netifTbl[swEgrNetifIdx].dualType == RTK_FC_DUALTYPE_MAPT|| (fc_db.netifTbl[swEgrNetifIdx].dualType == RTK_FC_DUALTYPE_XLAT464))
				pathDecision_var.flowPath_param.isIpv6 = FALSE; //upstream match flow before transform=>v4
		}
	}
	else if(fc_db.netifTbl[swIgrNetifIdx].dualType != RTK_FC_DUALTYPE_NONE && pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM) 
	{
		uint32 tunnelExtraTagIdx=0;
		DEBUG("[Dual] IgrIf dual header type %d  %s", fc_db.netifTbl[swIgrNetifIdx].dualType,name_of_dualHdrType[fc_db.netifTbl[swIgrNetifIdx].dualType]);
		ret = rtk_fc_dual_hw_setting(pPktHdr, rtskb->skb ,igrNetifIdx, &tunnelExtraTagIdx);
			
		if(ret != SUCCESS)
		{	
			if(fc_db.netifTbl[swIgrNetifIdx].dualType == RTK_FC_DUALTYPE_MAPT|| (fc_db.netifTbl[swIgrNetifIdx].dualType == RTK_FC_DUALTYPE_XLAT464))
			{
				//mapt software support
				swOnly = TRUE;
				pathDecision_var.flowPath_param.isIpv6 = TRUE;	// single -> single && it is mapt packet: if ip6h, upstream, otherwise downstream
			}
			else
			{
				TRACE("[Dual] Fail to get extra tag index, ret = %d", ret);
				goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
			}
		}else{
			if(fc_db.netifTbl[swIgrNetifIdx].dualType == RTK_FC_DUALTYPE_MAPT || (fc_db.netifTbl[swIgrNetifIdx].dualType == RTK_FC_DUALTYPE_XLAT464))
				pathDecision_var.flowPath_param.isIpv6 = FALSE; //downstream match flow AFTER transform=>v4
		}
	}

	//should after rtk_fc_dual_hw_setting
	if(pFcIngressData->outerHdrUnhit && pPktHdr->direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM && fc_db.netifTbl[swIgrNetifIdx].dualType!=RTK_FC_DUALTYPE_NONE )
	{
		TRACE("downstream ingress interface is dual-type(%d) but unhit outerHdr skip flow learning",fc_db.netifTbl[swIgrNetifIdx].dualType);
		goto RETURN_FAILED_WITHOUT_FLOW_DELETE;
	}

	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	//----------------------For dynamic calculate hash pattern---------------------------------------------------------------------------------------

	if(fc_db.controlFuc.dynamic_calculate_prehash_pattern &&
		pPktHdr &&
		pFcIngressData &&
		(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM  || pPktHdr->direction==RTK_FC_FLOW_DIRECTION_LANBRIDGE)&&
		pPktHdr->isMulticast==0 &&
		rgpro_db.fbMode == FB_MODE_32K
	)
	{
		//RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_DYNAMIC_PREHASH, 0);
		ret = _rtk_fc_dynamic_set_prehashPtn_process(pPktHdr,pFcIngressData, pPktHdr->fwdType);
		//RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_DYNAMIC_PREHASH, 0);
		if(ret == RTK_FC_DYNAMIC_PREHASHPTH_FAILED)
			DEBUG("Find Hash Fail");
		else if(ret == RTK_FC_DYNAMIC_PREHASHPTH_SKIP_FLOW_LEARNING)
			goto RETURN_FAILED_WITHOUT_FLOW_DELETE;

	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
#endif

	//3 === Flow table critical section START - protected by flow group lock ===
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_FLOW, pFcIngressData->flowHashIdx);
#if defined(CONFIG_FC_RTL8277C_SERIES) && RTK_FC_TABLESIZE_OVERFLOW_FLOW
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_FLOW_OVERFLOW, 0);
#endif


	if(pFcIngressData->isDAGatewayMAC && !pPktHdr->isMulticast && isL3) 
	{
		ret = _rtk_fc_flowEntry_path5_setting(&pathDecision_var.flowEntry, pFcIngressData, pPktHdr, &pathDecision_var.flowPath_param, &swOnly);
		if(ret != SUCCESS)
			goto FLOW_ADD_ERROR;
	}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	else if(isL3 && !isMcPassthrough &&
		(pPktHdr->isMulticast || !fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple || (pFcIngressData->isDualHeader && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE)/*dual header down stream*/))
#else
	else if(isL3 && isL4 &&  !fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple)
#endif
	{
		ret = _rtk_fc_flowEntry_path34_setting(&pathDecision_var.flowEntry, pFcIngressData, pPktHdr, &pathDecision_var.flowPath_param, &_extPmskIdx);
		if(ret != SUCCESS)
			goto FLOW_ADD_ERROR;
	}
	else
	{
		ret = _rtk_fc_flowEntry_path12_setting(&pathDecision_var.flowEntry, pFcIngressData, pPktHdr, &pathDecision_var.flowPath_param, pRt_ct, &_extPmskIdx);
		if(ret != SUCCESS)
			goto FLOW_ADD_ERROR;
	
	}
	if(pPktHdr->remarkDec.wanDsLoopback_en && ((pathDecision_var.flowEntry.in_path == FB_PATH_12) || (pathDecision_var.flowEntry.in_path == FB_PATH_34)) && !(pPktHdr->isMulticast) && !(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF))
		wanDsLoopback_en = TRUE;
	
#if defined(CONFIG_RTK_L34_CANDIDATE_FLOW)
	if(((pPktHdr->tcph==NULL) && (!pFcIngressData->isDualHeader) && (wanDsLoopback_en == FALSE))|| pPktHdr->isV6_NAT  
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)		
		|| (fc_db.controlFuc.special_fast_forward_mode==1 && (pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN && fc_db.vxlan_us_fastForward_num< RTK_FC_VXLAN_US_FF_NUM_MAX)) //VXLAN upstream fast forward
#endif
		){ // udp or other non-tcp: delay to HWFWD - start canditate state control (Dual header packet: not delay to HWFWD)
		pathDecision_var.flowEntry.valid = FALSE;
		
		pathDecision_var.flowEntryMultiAction.valid = FALSE;
		TRACE("[Candidata] next pkt will be handled by SW");
	}

	if (fc_db.controlFuc.flow_delay_hit_num > 0) {
		bool isTCP = FALSE;
				
		if((pathDecision_var.flowEntry.in_path == FB_PATH_34) || (pathDecision_var.flowEntry.in_path == FB_PATH_5)) {
			isTCP = (pPktHdr->l4protol ==  IPPROTO_TCP) ? TRUE : FALSE;
		}

		if((isTCP && fc_db.controlFuc.flow_delay_tcp_dis) || 
			(!isTCP && fc_db.controlFuc.flow_delay_other_dis)) {
			bypass_flowDelay = TRUE;
		}

		if(!bypass_flowDelay)
			pathDecision_var.flowEntry.valid = FALSE;
	}

	if(pPktHdr->tcph && pRt_ct->ct){

		switch(fc_db.controlFuc.flow_tcp_learning_state)
		{
			case TCP_LEARN_SYN_SENT:
				if(rt_tcp_state<RT_TCP_CONNTRACK_SYN_SENT) {
					TRACE("RT TCP ct state is %s but FC TCP learning state is SYN_SENT", rt_tcp_conntrack_names[rt_tcp_state]);
					pathDecision_var.flowEntry.valid = FALSE;
				}
				break;
			case TCP_LEARN_SYN_RECV:
				if(rt_tcp_state<RT_TCP_CONNTRACK_SYN_RECV) {
					TRACE("RT TCP ct state is %s but FC TCP learning state is SYN_RECV", rt_tcp_conntrack_names[rt_tcp_state]);
					pathDecision_var.flowEntry.valid = FALSE;
				}
				break;
			case TCP_LEARN_ESTABLISHED:
				if(rt_tcp_state<RT_TCP_CONNTRACK_ESTABLISHED) {
					TRACE("RT TCP ct state is %s but FC TCP learning state is ESTABLISHED", rt_tcp_conntrack_names[rt_tcp_state]);
					pathDecision_var.flowEntry.valid = FALSE;
				}
				break;
			default:
				WARNING("tcp learning state %d is not support", fc_db.controlFuc.flow_tcp_learning_state);
				goto FLOW_ADD_ERROR;
				break;
		}
		if(pathDecision_var.flowEntry.valid == FALSE) {
			TRACE("[Candidate] next pkt will be handled by SW");
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	if((pathDecision_var.flowEntry.valid == TRUE) && fc_db.lutTbl[pPktHdr->dmacL2Idx] &&
		(RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START <= fc_db.lutTbl[pPktHdr->dmacL2Idx]->wifiMacId) &&
		(fc_db.lutTbl[pPktHdr->dmacL2Idx]->wifiMacId <= RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END) &&
		fc_db.wifi_amsdu_pe_offload_mac_id_tbl[fc_db.lutTbl[pPktHdr->dmacL2Idx]->wifiMacId].sta_info.power_saving)
	{
		TRACE("[Candidate][TO AMSDU STA] lutEgrDa hit amsdu_pe_offload mac_id %d and its power_saving mode is 1, should not update to HW", fc_db.lutTbl[pPktHdr->dmacL2Idx]->wifiMacId);
		pathDecision_var.flowEntry.valid = FALSE;
	}
#endif
#endif
	// extra info collection: lutidx and dummy pkt buffer... etc
	{
		
		
		ret = _rtk_fc_extraInfo_setting(&pathDecision_var.g3IgrExtraInfo, &pathDecision_var.extraInfo, pPktHdr, &pathDecision_var.flowPath_param,pFcIngressData, &swOnly, pathDecision_var.flowEntry.in_path, pathDecision_var.flowEntry.in_pppoeif, if_egr_tos_remark);
		if(ret == FAILED)
		{
			WARNING("_rtk_fc_extraInfo_setting set failed.");
			goto FLOW_ADD_ERROR;
		}
		if(bypass_flowDelay)
			pathDecision_var.g3IgrExtraInfo.bypass_flowDelay = TRUE;
	}
	
	ret = _rtk_fc_dualHeader_g3ExtraInfo_setting(pPktHdr->dual_passthrough_type, &pathDecision_var.flowEntry, &pathDecision_var.g3IgrExtraInfo, pFcIngressData, pPktHdr, pathDecision_var.flowPath_param.isNAPT, pathDecision_var.flowPath_param.igrNetifIdx, pathDecision_var.flowPath_param.egrNetifIdx, &swOnly);
	if(ret == FAILED)
	{
		DEBUG("_rtk_fc_dualHeader_g3ExtraInfo_setting set failed.");
		goto FLOW_ADD_ERROR;
	}
	
	
	

	
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	{
		rtk_rg_asic_path1_entry_t *pFlow = &pathDecision_var.flowEntry;
		if(pFlow->in_path != FB_PATH_6)
		{
			if(fc_db.controlFuc.flow_meter_mib_conf_dependence)
			{
				TRACE("flow_meter_mib_conf_dependence is enabled, flow mib configuration depends on flow meter configuration");
				if(pFlow->out_share_meter_act)
				{
					pFlow->out_flow_counter_act = TRUE;
					pFlow->out_flow_counter_idx = pFlow->out_share_meter_idx;
				}
				else if(pathDecision_var.extraInfo.swShaperEn & (1<<SWSHAPER_TYPE_FLOW))
				{
					pFlow->out_flow_counter_act = TRUE;
					pFlow->out_flow_counter_idx = pFlow->out_share_meter_idx;
				}
				else
					pFlow->out_flow_counter_act = FALSE;

				flowMibEn = pFlow->out_flow_counter_act;
				flowMibIdx = pFlow->out_flow_counter_idx;
			}
		}
	}
#endif

#if defined(CONFIG_FC_RTL9607C_SERIES)
	if(RTK_FC_NPTV6_SRAM_DRAM_CHECK(pPktHdr, swOnly, isL4 ))
	{
		_rtk_fc_nptv6_sram_dram_preprocessing(&flow_can_add, &pathDecision_var.flowEntry, pPktHdr, rtskb, pFcIngressData->flowHashIdx, &pathDecision_var.g3IgrExtraInfo, pathDecision_var.flowPath_param.igrNetifIdx,
											  pathDecision_var.flowPath_param.egrNetifIdx, &pathDecision_var.extraInfo, pRt_ct, &swOnly, pathDecision_var.flowPath_param.egrSVlanTagif, &pathDecision_var.tmp_pFlowPath5, &pathDecision_var.tmp_pG3IgrExtraInfo);
	}
#endif

	if (unlikely(!fc_db.controlFuc.ip_frag_shortcut && pFcIngressData->ipFragFlag))
	{
		FRAGMENT("detect invalid egress ip frag packet, iph id=0x%x!", pFcIngressData->ipId);
		goto FLOW_ADD_ERROR;
	}

	if(unlikely(wanDsLoopback_en))
	{
		_rtk_fc_flow_wan_downstream_process(&pathDecision_var.extraInfo, &pathDecision_var.flowEntry, &pathDecision_var.g3IgrExtraInfo, pPktHdr, &flowIdx, pFcIngressData, pRt_ct, swOnly, 0, &pathDecision_var.g3IgrExtraInfo);
	}
	else
	{
#if defined(CONFIG_RTK_SOC_RTL8198D)
		// swOnly may be changed here
		if (rtk_fc_flow_limit_check(&swOnly, pathDecision_var.flowEntry.in_path) != SUCCESS) {
			goto FLOW_ADD_ERROR;
		}
#endif

		//Original flow add
		ret = rtk_fc_flow_add(&flowIdx, pFcIngressData->flowHashIdx, &pathDecision_var.flowEntry, pRt_ct,
								&pathDecision_var.extraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, swOnly, 0, &pathDecision_var.g3IgrExtraInfo, pPktHdr);
		if(ret != RTK_FC_RET_OK)
		{
			if (ret == RTK_FC_RET_ERR_FLOW_FULL)
				TRACE("flow add failed: flow table is full");
			else
				WARNING("flow add failed, ret = 0x%x",ret);
			goto FLOW_ADD_ERROR;
		}
		
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_fc_store_flow_info_by_ct(pRt_ct, flowIdx, pPktHdr, pFcIngressData->flowHashIdx);
#endif
#if defined(CONFIG_FC_RTL9607C_SERIES) 

	/*
		NPTv6 HWLOOKUP Flow, Only valid when 9607C
	*/
	if(RTK_FC_NPTV6_HWLOOKUP_FLOW_CHECK(pPktHdr, swOnly, flow_can_add, isL4))
	{
		_rtk_fc_nptv6_hwlookup_flow_processing(&pathDecision_var.g3IgrExtraInfo, pPktHdr, rtskb, pathDecision_var.flowPath_param.igrNetifIdx, pathDecision_var.flowPath_param.egrNetifIdx, 
											   &pathDecision_var.extraInfo, pRt_ct, flowIdx, &pathDecision_var.tmp_pFlowPath5, &pathDecision_var.tmp_pG3IgrExtraInfo, pFcIngressData);
	}
#endif	
	
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)
	//----------------9607C/8277B special fastFwd VXLAN RELATED FUNTIONS------------------------------------------------------------------------------------------
	ret = _rtk_fc_vxlan_processing(pFcIngressData, pPktHdr, &pathDecision_var.g3IgrExtraInfo, pathDecision_var.flowPath_param.egrNetifIdx, &pathDecision_var.flowEntry, rtskb, pathDecision_var.flowPath_param.igrNetifIdx, pRt_ct, &pathDecision_var.extraInfo, outer_daIdx, flowIdx);

	if(ret & FC_VXLAN_FLOW_BYPASS_VXLAN_ADD)
	{
		DEBUG("VXLAN Bypass adding process! ret = 0x%x",ret);
		ret = RTK_FC_RET_OK;
	}
	//-------------------------------------------------------------------------------------------------------------------------------
#endif


	//cache MAP-E tunnel IPv4 address
	if((ret == RTK_FC_RET_OK))
	{
		if (fc_db.netifTbl[swEgrNetifIdx].dualType   == RTK_FC_DUALTYPE_IP4INIP6 && fc_db.netifTbl[swEgrNetifIdx].dualUniInfo.ip4Inip6_info.isMape   )
		{	//MAP-E: single->dual
			fc_db.flowTbl[flowIdx].protoCtrl = FLOW_PROTO_CTRL_MAPE_ACC;
			fc_db.flowTbl[flowIdx].mapeInfo.mape_tun_ipv4_addr = ntohl(pPktHdr->iph->saddr);
			DEBUG("[MAP-E][upstream] mape_tun_ipv4_addr=0x%08x",fc_db.flowTbl[flowIdx].mapeInfo.mape_tun_ipv4_addr);
		}
		else if (fc_db.netifTbl[swIgrNetifIdx].dualType   == RTK_FC_DUALTYPE_IP4INIP6 && fc_db.netifTbl[swEgrNetifIdx].dualUniInfo.ip4Inip6_info.isMape ) 
		{	//MAP-E: dual->single
			fc_db.flowTbl[flowIdx].protoCtrl = FLOW_PROTO_CTRL_MAPE_ACC;
			fc_db.flowTbl[flowIdx].mapeInfo.mape_tun_ipv4_addr = pFcIngressData->dstIp;
			DEBUG("[MAP-E][downstream] mape_tun_ipv4_addr=0x%08x",fc_db.flowTbl[flowIdx].mapeInfo.mape_tun_ipv4_addr);
		}
	}
	
	if((ret==RTK_FC_RET_OK) && fc_db.controlFuc.flow_tcp_bidirection_learning && 
		(pathDecision_var.flowEntry.in_path == FB_PATH_5) && (pathDecision_var.flowPath_param.tunnelExtraTagIdx==0)  && (!pathDecision_var.flowPath_param.isIpv6) && (!(pathDecision_var.flowPath_param.isMAPT || pathDecision_var.flowPath_param.para_isXLAT)) && (pPktHdr->tcph)
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		&& (!pathDecision_var.g3IgrExtraInfo.natloopback_act_info.is_ipv4_natloopback)
#endif
		) {
		// support ipv4 single header tcp bidirection learning
		
		ret = rtk_fc_flow_reverse_add(&reverseFlowIdx, pFcIngressData->flowHashIdx, &pathDecision_var.flowEntry, pRt_ct,
							&pathDecision_var.extraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, swOnly, 0, &pathDecision_var.g3IgrExtraInfo);

		if((ret==RTK_FC_RET_OK) && pRt_ct->ct && (rt_tcp_state<RT_TCP_CONNTRACK_ESTABLISHED)) {
			// tcp bidirectional acceleration is ready
			RTK_FC_HELPER_SPIN_LOCK_BH(pRt_ct->lock);

			DEBUG("update tcp state to SYN_RECV");

			// state update
			rt_tcp_state = RT_TCP_CONNTRACK_ESTABLISHED;
			RTK_FC_HELPER_PS_CT_RT_TCP_STATE_SET(pRt_ct->ct, rt_tcp_state);

			RTK_FC_HELPER_SPIN_UNLOCK_BH(pRt_ct->lock);

		}
	}

	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] flowent idx=%d, ret=%d", pFcIngressData->flowHashIdx, ret);
		goto FLOW_ADD_ERROR;
	}
	else
	{
		TRACE("Add %s flow entry [%d]", (flowIdx<fc_db.flowHwTableSize)?"Hw":"Sw", flowIdx);

		// for mib update
		{
			int tagsLen = 0;				// assume ingress with no any tags,  get how many 4bytes we needed to +/- according to egress skb len.

			if(pPktHdr->svh) tagsLen-=4;
			if(pPktHdr->cvh) tagsLen-=4;
			if(pPktHdr->ppph) tagsLen-=8;
			if(pFcIngressData->ingressTagif & SVLAN_TAGIF) tagsLen+=4;
			if(pFcIngressData->ingressTagif & CVLAN_TAGIF) tagsLen+=4;
			if(pFcIngressData->ingressTagif & PPPOE_TAGIF) tagsLen+=8;

			pPktHdr->skbLen_ingress = RTSKB_LEN(rtskb) + tagsLen;
			pPktHdr->flowIdx = flowIdx; // for later flow mib update
		}

	}
	
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_FLOW, pFcIngressData->flowHashIdx);
#if defined(CONFIG_FC_RTL8277C_SERIES) && RTK_FC_TABLESIZE_OVERFLOW_FLOW
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_FLOW_OVERFLOW, 0);
#endif

	//3 === Flow table critical section END - protected by flow group lock ===	


	if(pPktHdr->egressFlowAddComplete == 0) {	// somehow skip flow adding but return SUCCESS, e.g. flow exist.
		if(_extPmskIdx)
			RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(_extPmskIdx);
	}

	return RTK_FC_RET_OK;


FLOW_ADD_ERROR:

	if(_extPmskIdx)
		RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(_extPmskIdx);

	if(flowIdx!=FAILED)
		rtk_fc_flow_delete(flowIdx);
	if(reverseFlowIdx!=FAILED)
		rtk_fc_flow_delete(reverseFlowIdx);
	if(flowIdxMultiAction!=FAILED)
		rtk_fc_flow_delete(flowIdxMultiAction);

	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_FLOW, pFcIngressData->flowHashIdx);
#if defined(CONFIG_FC_RTL8277C_SERIES) && RTK_FC_TABLESIZE_OVERFLOW_FLOW
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_FLOW_OVERFLOW, 0);
#endif

	//3 === Flow table critical section END - protected by flow group lock ===

	return RTK_FC_RET_ERR;

RETURN_FAILED_WITHOUT_FLOW_DELETE:

	return FAILED;
}

int rtk_fc_egress_skbMarkSetting(struct rt_skbuff *rtskb, rtk_fc_skbmarkTarget_t target)
{

		int startBit, len, mark1or2;
		uint32 mark_value;

		
		startBit = fc_db.skbmark[target].startBit;
		len      = fc_db.skbmark[target].len;
		mark1or2 = fc_db.skbmark[target].mark1or2;

		mark_value = (mark1or2==0)?(RTSKB_MARK(rtskb)>>startBit)&((1<<len)-1):(uint32)(RTSKB_MARK2(rtskb)>>startBit)&((1LL<<len)-1);

		return mark_value;
}

static void rtk_fc_egress_skbMarkInspect(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int portMask)
{
	/* skb->mark may be remarked in mangle table for specific purpose */
	
	//3remarking func from skb mark value
	// Skip caching / force fwd by ps
	if(fc_db.skbmark[RTK_FC_SKBMARK_FWDBYPS].startBit != FAILED)
		pPktHdr->remarkDec.fwdByPS = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_FWDBYPS);

	// Queue id
	if(fc_db.skbmark[RTK_FC_SKBMARK_QID].startBit != FAILED)
		pPktHdr->remarkDec.outputQid = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_QID);

	//WAN dowmstream loopback enable
	if((fc_db.skbmark[RTK_FC_SKBMARK_WANDSLOOPBACK_EN].startBit != FAILED) && ((1 << RTSKB_FCIGRDATA(rtskb)->ingressPort) & fc_db.wanPortMask.portmask) && (fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].startBit!=FAILED))
	{
		pPktHdr->remarkDec.wanDsLoopback_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_WANDSLOOPBACK_EN);
		if(pPktHdr->remarkDec.wanDsLoopback_en){
			pPktHdr->remarkDec.streamId = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_STREAMID);
			if(fc_db.controlFuc.epon_llid_format){
				pPktHdr->remarkDec.streamId <<= 4;
				pPktHdr->remarkDec.streamId |= pPktHdr->remarkDec.outputQid;
			}
		}
		TRACE("from skb->mark, wanDsLoopback_en = %d, upstream sid = %d", pPktHdr->remarkDec.wanDsLoopback_en, pPktHdr->remarkDec.streamId);
	}

	// Stream id
	if((portMask & fc_db.wanPortMask.portmask) &&
		(fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN].startBit != FAILED) && (fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].startBit!=FAILED))
	{
		pPktHdr->remarkDec.streamId_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_STREAMID_EN);
		if(pPktHdr->remarkDec.streamId_en){
			pPktHdr->remarkDec.streamId = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_STREAMID);
			if(fc_db.controlFuc.epon_llid_format){
				pPktHdr->remarkDec.streamId <<= 4;
				pPktHdr->remarkDec.streamId |= pPktHdr->remarkDec.outputQid;
			}
			TRACE("from skb->mark, sid = %d", pPktHdr->remarkDec.streamId);
		}
	}

	// Meter idx
	if(fc_db.skbmark[RTK_FC_SKBMARK_METERIDX].startBit != FAILED)
	{
		if(fc_db.skbmark[RTK_FC_SKBMARK_METERIDX_EN].startBit != FAILED) {
			pPktHdr->remarkDec.meterIdx_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_METERIDX_EN);
			if(pPktHdr->remarkDec.meterIdx_en)
				pPktHdr->remarkDec.meterIdx = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_METERIDX);
		}

		// swShaper idx
		if(fc_db.skbmark[RTK_FC_SKBMARK_SWSHAPER_EN].startBit != FAILED)
		{
			pPktHdr->remarkDec.swShaper_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SWSHAPER_EN);
			if(pPktHdr->remarkDec.swShaper_en)
				pPktHdr->remarkDec.meterIdx = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_METERIDX);
		}
	}
	//Mib idx
	if((fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX_EN].startBit != FAILED) && (fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX].startBit != FAILED))
	{
		pPktHdr->remarkDec.mibIdx_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_MIBIDX_EN);
		if(pPktHdr->remarkDec.mibIdx_en )
		{
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && defined(CONFIG_FC_CAG3_SERIES)
			if(pPktHdr->remarkDec.meterIdx_en)
			{
				WARNING("Meter and Mib can not be set at the same time. Only meter action of this flow takes effect!\n");
				pPktHdr->remarkDec.mibIdx_en = 0;
			}
			else
#endif
			{
				pPktHdr->remarkDec.mibIdx = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_MIBIDX);
			}
		}
	}

	{
		if(fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ACTION].startBit != FAILED)
		{
			pPktHdr->remarkDec.cvlan_action_from_skbmark = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_CVLAN_ACTION);
		}

		if(fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ACTION].startBit != FAILED)
		{
			pPktHdr->remarkDec.svlan_action_from_skbmark = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SVLAN_ACTION);
		}

	}
	{
		if((fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_EN].startBit != FAILED) && (fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_INOUT].startBit != FAILED) && (fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_IDX].startBit != FAILED))
		{
			pPktHdr->remarkDec.swIntfIdx_assign_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_EN);
			pPktHdr->remarkDec.swIntfIdx_assign_inOut = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_INOUT);
			pPktHdr->remarkDec.swIntfIdx_assign_idx = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_IDX);
		}
	}

	{
		if((fc_db.skbmark[RTK_FC_SKBMARK_SKIP_FC_ALG_CHECK].startBit != RTK_FC_RMK_UNDEF) )
		{
			pPktHdr->remarkDec.skip_fc_alg_check = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_SKIP_FC_ALG_CHECK);
			TRACE("pPktHdr->remarkDec.skip_fc_alg_check  = %d\n",pPktHdr->remarkDec.skip_fc_alg_check );
		}
	}

	{
		if((fc_db.skbmark[RTK_FC_SKBMARK_FLOW_CACHE_MIB_EN].startBit != RTK_FC_RMK_UNDEF) )
		{
			pPktHdr->remarkDec.flow_cache_mib_en = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_FLOW_CACHE_MIB_EN);
		}
	}

	{
		if((fc_db.skbmark[RTK_FC_SKBMARK_PSFLOODFWDACC].startBit != RTK_FC_RMK_UNDEF) )
		{
			pPktHdr->remarkDec.psFloodfwdAcc= rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_PSFLOODFWDACC);
			TRACE("pPktHdr->remarkDec.psFloodfwdAcc  = %d\n",pPktHdr->remarkDec.psFloodfwdAcc );
		}
	}

	{
		if((fc_db.skbmark[RTK_FC_SKBMARK_MAPET_FMR].startBit != RTK_FC_RMK_UNDEF) )
		{
			pPktHdr->remarkDec.mapet_fmr = rtk_fc_egress_skbMarkSetting(rtskb, RTK_FC_SKBMARK_MAPET_FMR);
			TRACE("pPktHdr->remarkDec.mapet_fmr  = %d\n",pPktHdr->remarkDec.mapet_fmr );
		}
	}

	return;
}


static int _rtk_fc_egress_flowLearning(struct rt_skbuff *rtskb, struct net_device *dev, int portMask, rtk_fc_wlan_devidx_t egrWlanDevIdx)
{
	uint8 glbLocked = FALSE;
	rtk_fc_ret_t ret = RTK_FC_RET_OK;
	rtk_fc_table_mcConfigTbl_t *p_alloc_userconf=NULL;
	rtk_fc_table_mcConfigTbl_t *p_userconf=NULL;
	int32 skbTotalNeedSendCnt=1,skbHandleCnt=0;
	struct rt_skbuff *handle_rtskb=rtskb;
	struct rt_skbuff *handle_rtskb_bk=NULL;
	struct rt_skbuff _handle_rtskb_bk;
	struct sk_buff *handle_skb=NULL;
	struct sk_buff *handle_skb_bk=NULL;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	unsigned short isIpv6;
	unsigned int groupip[4];
	unsigned int sourceip[4];
	unsigned int sourceport = 0;
	unsigned int destPort = 0;
#endif

	rtk_fc_pktHdr_t *pPktHdr=&fc_db.smp_pktHdr[smp_processor_id()];
	memset(pPktHdr,0,sizeof(*pPktHdr));

COPY_SKBPKT:
	skbHandleCnt++;
	//if only one skb handle just using orinal rtskb , init skbTotalNeedSendCnt before this point
	if(pPktHdr->isMulticast && skbTotalNeedSendCnt>1)
	{
		if(skbHandleCnt==skbTotalNeedSendCnt)
		{
			//last Handle packet using bk skb
			IGMP("MC2UC last Handle packet using backup skb");
			handle_rtskb=handle_rtskb_bk;
		}
		else
		{
			//using  handle_rtskb_bk alloc new skb to handle_rtskb
			IGMP("MC2UC using  handle_rtskb_bk alloc new skb to handle_rtskb");
			RTK_FC_HOOK_PS_SKB_SKB_COPY(RTSKB_SKB(handle_rtskb_bk), GFP_ATOMIC, &handle_skb);
			RTK_FC_HOOK_CONVERTER_SKB(handle_skb, handle_rtskb);
		}
		rtk_fc_mc2ucPktModify(handle_rtskb,pPktHdr,skbHandleCnt,p_userconf);
		memset(pPktHdr,0,sizeof(*pPktHdr));		
	}


	if(unlikely(fc_db.fwdStatistic))
	{
		if(RTSKB_FCIGRDATA(handle_rtskb)->isNotLocalOut)
		{
			if(RTSKB_DATA(handle_rtskb)[0]==0xff)
				atomic_inc(&fc_db.statistic.perPortCnt_FromPS_broadcast[RTSKB_FCIGRDATA(handle_rtskb)->ingressPort]);
			else if(RTSKB_DATA(handle_rtskb)[0]&0x1)
				atomic_inc(&fc_db.statistic.perPortCnt_FromPS_multicast[RTSKB_FCIGRDATA(handle_rtskb)->ingressPort]);
			else
				atomic_inc(&fc_db.statistic.perPortCnt_FromPS_unicast[RTSKB_FCIGRDATA(handle_rtskb)->ingressPort]);
		}
		else
		{
			atomic_inc(&fc_db.statistic.perPortCnt_LocalOut[RTK_FC_MAC_PORT_MAINCPU]);
		}
	}
	// init pPktHdr
	_rtk_fc_pktHdr_init(pPktHdr, handle_rtskb);

	//3 1. Remarking info inspection MUST do remark checking before any egress_tx
	rtk_fc_egress_skbMarkInspect(handle_rtskb, pPktHdr, portMask);

	if(unlikely(fc_db.rtk_fc_init==0))
		goto EGRESS_TX;

	// unknown destination port
	if(unlikely(portMask == 0))
	{
		TRACE("Error: unknown dest portmask 0x%x...", portMask);
		goto EGRESS_TX;
	}
	
	if(RTSKB_FCIGRDATA(handle_rtskb)->isNotLocalOut) {
//		pktHdr.reason = RTSKB_FCIGRDATA(rtskb)->reason;
		rtk_fc_egress_portDecision(pPktHdr, handle_rtskb, portMask, egrWlanDevIdx);
	}
	else {
		pPktHdr->ingressPort.macPortIdx = RTK_FC_MAC_PORT_MAINCPU;
	}

	//3 2. Parse and inspect packet header.
	ret = _rtk_fc_pktHdr_inspect(handle_rtskb,pPktHdr,portMask, NULL);
	
	
	if(unlikely(ret != RTK_FC_RET_OK))
	{
		TRACE("Skb is strange! skip learning!");
		goto EGRESS_TX;
	}

	/* multicast process */
	{
		if(pPktHdr->isMulticast && skbHandleCnt==1 )
		{
			if(p_alloc_userconf==NULL)
			{
				p_alloc_userconf = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_table_mcConfigTbl_t), GFP_ATOMIC);
				memset(p_alloc_userconf,0,sizeof(*p_alloc_userconf));
				p_userconf = rtk_fc_egress_mcConfigGetByPkt(rtskb,pPktHdr,p_alloc_userconf);
			}
			//wifi MctoUc need init skbTotalNeedSendCnt
			if(fc_db.controlFuc.mc2uc_en && (rtskb->skb->data[0]&0x1))
				rtk_fc_mc2ucCopyCnt_get(handle_rtskb,pPktHdr,&skbTotalNeedSendCnt,p_userconf);
		}
		//if only one skb handle just using orinal rtskb , init skbTotalNeedSendCnt before this point
		if(pPktHdr->isMulticast && skbTotalNeedSendCnt>1 && skbHandleCnt==1)
		{
			//copy a backup skb and using orinal rtskb
			IGMP("MC2UC COPY a BACKUP SKB and Send orinal packet");
			handle_rtskb_bk=&_handle_rtskb_bk;
			RTK_FC_HOOK_PS_SKB_SKB_COPY(RTSKB_SKB(handle_rtskb), GFP_ATOMIC, &handle_skb_bk);
			RTK_FC_HOOK_CONVERTER_SKB(handle_skb_bk, handle_rtskb_bk);
		}
		if(pPktHdr->isMulticast)
		{
			rtk_fc_egress_mcCheckPortInConfig(handle_rtskb,pPktHdr,p_userconf);	
		}
		if(pPktHdr->isMulticast && skbHandleCnt==1 && (rtskb->skb->data[0]&0x1))
		{
			//trans first orignal packet
			rtk_fc_mc2ucPktModify(handle_rtskb,pPktHdr,skbHandleCnt,p_userconf);
		}
	}
	
	_rtk_fc_trace_log_processing(handle_rtskb, NULL, pPktHdr, RTK_FC_TRACEFILTER_EGRESS);

	
	ret = _rtk_fc_lut_learning_processing(handle_rtskb, portMask, pPktHdr);
		
	if(unlikely(ret & RTK_FC_RET_DROP)){
		//RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		goto DROP;
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	glbLocked = TRUE;
	
#if defined(CONFIG_RTK_SOC_RTL8198D)
	if ((RTSKB_DATA(handle_rtskb)[0] & 1) == 0 && RTK_FC_HELPER_GET_EXT_FLOW_MIB_CONTROL()) {
		uint32 ext_flowmib_idx;
		bool is_uplink = (pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM) ? TRUE : FALSE;
		int8 is_wlan_sta = 0;

		if (pPktHdr->igrWlanDevIdx < RTK_FC_WLANX_END_INTF && pPktHdr->egrWlanDevIdx  < RTK_FC_WLANX_END_INTF) {
			TRACE("[ext flow mib] do not count wlan (%d) to wlan (%d) traffic", pPktHdr->igrWlanDevIdx, pPktHdr->egrWlanDevIdx);
		}
		else if (pPktHdr->igrWlanDevIdx < RTK_FC_WLANX_END_INTF && (is_wlan_sta = RTSKB_FCIGRDATA(handle_rtskb)->wlan_type) < 0) {
			WARNING("igrWlanDevIdx %d %d is neither 5g (%s) or 2g (%s)", pPktHdr->igrWlanDevIdx, RTSKB_FCIGRDATA(handle_rtskb)->wlan_type, RTK_FC_WLAN_NAME_5G, RTK_FC_WLAN_NAME_2G);
		}
		else if (pPktHdr->egrWlanDevIdx  < RTK_FC_WLANX_END_INTF && (is_wlan_sta = RTK_FC_HELPER_IS_WLAN_5G(RTSKB_DEV(handle_rtskb)->name)) < 0) {
			WARNING("egrWlanDevIdx %d is neither 5g (%s) or 2g (%s)", pPktHdr->egrWlanDevIdx, RTK_FC_WLAN_NAME_5G, RTK_FC_WLAN_NAME_2G);
		}
		else {
			// set traffic hw flow mib idx
			if (RTK_FC_HELPER_SKB_EGRESS_COUNT_IN_EXT_FLOW_MIB(RTSKB_SKB(handle_rtskb), is_wlan_sta, is_uplink, &ext_flowmib_idx) == SUCCESS) {
				if (ext_flowmib_idx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
					pPktHdr->remarkDec.mibIdx_en	= 1;
					pPktHdr->remarkDec.mibIdx 	= is_uplink ? RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(ext_flowmib_idx) : RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(ext_flowmib_idx);
				}
			}
		}
	}
#endif

	if(unlikely(ret != RTK_FC_RET_OK))
	{

		TRACE("Skip flow learning");
		goto EGRESS_TX;
	}
	else
	{

#if defined(CONFIG_RTK_SOC_RTL8198D)
		if(pPktHdr->isMulticast&&(pPktHdr->igmph==NULL)&&(pPktHdr->icmp6h==NULL))
		{
			memset(sourceip,0 ,sizeof(sourceip));
			memset(groupip, 0,sizeof(groupip));
			
			if(RTK_FC_HELPER_CHECK_USER_GROUP(RTSKB_DATA(handle_rtskb)) == 1)
			{
				//pskb = RTSKB_DATA(rtskb);

				sourceport = RTSKB_FCIGRDATA(handle_rtskb)->srcPort;
				destPort = RTSKB_FCIGRDATA(handle_rtskb)->dstPort;

				
				if(pPktHdr->iph)
				{
					isIpv6 = FALSE;
					groupip[0] = pPktHdr->iph->daddr;
					sourceip[0] = RTSKB_FCIGRDATA(handle_rtskb)->srcIp;
				}
				else if(pPktHdr->ip6h)
				{
					isIpv6 = TRUE;
					memcpy(groupip, pPktHdr->ip6h->daddr.s6_addr32, sizeof(pPktHdr->ip6h->daddr.s6_addr32));
					memcpy(sourceip,pPktHdr->ip6h->saddr.s6_addr32,sizeof(sourceip));
				}
				
				//printk("--destPort = %d, sourcePort=%d,,isIpv6 =%d,groupip is %x\n",destPort,sourceport,isIpv6,groupip[0]);

				if(RTK_FC_HELPER_CHECK_USER_GROUP_FLOW(groupip,isIpv6,sourceip,sourceport,destPort) == 1)
				{
					//should not be forward.
					ret=RTK_FC_RET_DROP;
					goto DROP;
				}
		
			}
		}
#endif 

		
		DEBUG("Igr port %d/%d wlanidx %d Egr port %d/%d wlanidx %d", 
			pPktHdr->ingressPort.macPortIdx, pPktHdr->ingressPort.macExtPortIdx, pPktHdr->igrWlanDevIdx,
														pPktHdr->egressPort.macPortIdx, pPktHdr->egressPort.macExtPortIdx, pPktHdr->egrWlanDevIdx);
	}

	//3 4. Record fwd_statistic info
	if(unlikely(fc_db.fwdStatistic))
		_rtk_fc_record_egress_statistic(handle_rtskb, pPktHdr);

	//For Non-pppoe igmp/mld
	
	if(pPktHdr->remarkDec.fwdByPS && !RTSKB_FCIGRDATA(handle_rtskb)->mcDummyPkt)
	{
		TRACE("FORCE fwd by PS..., skip learning");
		goto EGRESS_TX;
	}

	// no ingress pattern.
	if(!RTSKB_FCIGRDATA(handle_rtskb)->doLearning)
	{
		TRACE("No inbound info, skip learning flow...");
		goto EGRESS_TX;
	}
	if( (RTSKB_FCIGRDATA(handle_rtskb)->ingressDualFailed + pPktHdr->dualFailed) ==1)
	{
		
		TRACE("Dual Failed! skip learning flow... ingress dual fail[%d] egress dual fail[%d]",RTSKB_FCIGRDATA(handle_rtskb)->ingressDualFailed,pPktHdr->dualFailed);
		goto EGRESS_TX;

	}

	if((RTSKB_FCIGRDATA(rtskb)->ingressTagif & L2TP_TAGIF) && (pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE) && RTSKB_FCIGRDATA(rtskb)->dual_hdr_cnt > 0)
	{
		// for VXLAN over l2tp, skip learning
		TRACE("VXLAN over l2tp downstream!Skip learning! dual hdr cnt(%d)",RTSKB_FCIGRDATA(rtskb)->dual_hdr_cnt);
		goto EGRESS_TX;
		
		
	}


	//3 5. Cache flow ingress patterns and egress actions
	if(pPktHdr->abstrSwFlowSearch)
	{
		rtk_fc_egress_abstrSwFlowDecision(handle_rtskb, pPktHdr);
	}
	else
	{
		rtk_fc_egress_pathDecision(handle_rtskb, pPktHdr);	// path1~5 flow learning
	}
	
	if(pPktHdr->isMulticast  &&RTSKB_FCIGRDATA(handle_rtskb)->mcDummyPkt )
	{
		IGMP("Drop multicast DummyPkt and Free skb");
		atomic_inc(&fc_db.statistic.perPortCnt_dummpPkt[RTSKB_FCIGRDATA(handle_rtskb)->ingressPort]);
		ret = RTK_FC_RET_DROP_DUMMY;
		goto DROP;
	}


EGRESS_TX:
	
	if(glbLocked) {
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#if 0//defined(CONFIG_FC_RTL9607C_SERIES)		
		if(rtskb->nptv6_setMeterFlag==1)
		{
			rt_rate_shareMeterRate_set(fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[fc_db.nptv6_setMeter_macPortIdx], fc_db.nptv6_setMeterRate);
		}
		
#endif		
		glbLocked = FALSE;
	}
	
	// tunnel: outer header IPID/GRESEQ/GREACK sync (fwd 1st packt, localout)
	if((pPktHdr->eth!=NULL && (RTSKB_FCIGRDATA(handle_rtskb)->isDAGatewayMAC || (pPktHdr->smacEgrChange==TRUE))) || (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_GRE_ETH_BR) || (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IPSEC)){
		// packet handled by FC: check isDAGatewayMAC to find routing pkts.
		// packet handled by PS: check sa trans to find rounting and local out pkts.
		rtk_fc_egress_tunnelInfoSync(handle_rtskb, pPktHdr);
	}

	TRACE("direct Tx to dev %s port: 0x%x wlandevidx %d", RTSKB_DEV(handle_rtskb)?RTSKB_DEV(handle_rtskb)->name:"NULL", portMask, egrWlanDevIdx);
	// nic/wifi tx
	ret = _rtk_fc_flowEgressTx(handle_rtskb, pPktHdr, portMask, egrWlanDevIdx);

DROP:	

	if(glbLocked) {
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		glbLocked = FALSE;
	}

	if(ret&RTK_FC_RET_DROP){
		if(unlikely(fc_db.fwdStatistic))
		{
			atomic_inc(&fc_db.statistic.perPortCnt_Drop[RTSKB_FCIGRDATA(handle_rtskb)->ingressPort][(ret&~(RTK_FC_RET_DROP))]);
		}
		RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(RTSKB_SKB(handle_rtskb));
	}
	
	RTK_FC_TRACE_FILTER_UNLOCK_CHECK();

	if(pPktHdr->isMulticast && skbTotalNeedSendCnt>1 && (skbHandleCnt < skbTotalNeedSendCnt))
		goto COPY_SKBPKT;

	if(p_alloc_userconf)
		RTK_FC_HELPER_FC_KFREE(p_alloc_userconf,sizeof(*p_alloc_userconf));

	return 0;
}

#define IN_MULTICAST6_RESV1_FFO1(addr)   	 ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF010000)



__IRAM_FC_SHORTCUT
static int _rtk_fc_ingress_flowLearning(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo)
{
	rtk_fc_pktHdr_t pktHdr={0};
	bool wlan_hwlookup_miss = FALSE;
	fc_rx_info_t rxInfoFromWLAN;
	rtk_fc_toPsReason_t toPsReason = 0;

	rtk_fc_nic_rx_t nic_rx_ret = RTK_FC_NIC_RX_CONTINUE;
	rtk_fc_ret_t ret = RTK_FC_RET_OK;
	bool skip_data_caching = FALSE;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
    bool do_IPSEC_recovery = FALSE;
#endif

	
	// Init pPktHdr
	_rtk_fc_pktHdr_init(&pktHdr, rtskb);

	if(unlikely(pRxInfo==NULL))//from Wifi
	{
		pRxInfo = &rxInfoFromWLAN;
		memset(pRxInfo, 0, sizeof(fc_rx_info_t));

		pktHdr.fromWlan_noHWlookup=1U;
	}else {
	
		if(unlikely(fc_db.smpStatistic))
			atomic_inc(&fc_db.smp_statistic[FC_SMP_FC_RX][smp_processor_id()]);
	}

	/*** cpu reason ***/
	pktHdr.reason= RXINFO_REASON(pRxInfo);
	if((pktHdr.reason == CPU_REASON_ACL) && (fc_db.controlFuc.flow_swFwded_aclTrapPri_mask & (1 << RXINFO_INT_PRI(pRxInfo))))
		pktHdr.isHitSwFwdedAclTrapPri = TRUE;
	/****************/
		
	ret = rtk_fc_ingress_portDecision(&pktHdr, pRxInfo, rtskb);	// rename as "ingress rx info to pkthdr" for all rx info?
	if(unlikely(ret & RTK_FC_RET_DROP)){
		goto DROP;
	}

	if(RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo))
		wlan_hwlookup_miss = TRUE;

#if defined(CONFIG_RTK_SOC_RTL8198D)
	if(!(fc_db.wanPortMask.portmask & (1ULL<<pktHdr.ingressPort.macPortIdx))){
		//ethernet
		RTK_FC_HOOK_LINUX_VLAN_RX_HANDLE(rtskb, pktHdr.ingressPort.macPortIdx);
	}
#endif

	if(unlikely(
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
		RXINFO_SPA(pRxInfo)!=RTK_FC_IPSEC_HWLOOKUP_MACPORT &&
#endif			
		(pktHdr.fromWlan_noHWlookup || (wlan_hwlookup_miss
#if defined(CONFIG_FC_RTL8198F_SERIES)
		&& (!fc_db.controlFuc.pe_fc_enable || (pktHdr.ingressPort.macPortIdx != fc_db.controlFuc.pe_port))
#endif
		))))
	{
		if(rtk_fc_wlan_ingress_info_recovery(rtskb,&pktHdr)!=RTK_FC_NIC_RX_CONTINUE)
			goto DROP;
	}
	
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)		
	else if( RXINFO_DST_EXTPIDX(pRxInfo) >0 && RXINFO_SPA(pRxInfo)==RTK_FC_IPSEC_HWLOOKUP_MACPORT)
	{
		IPSEC("IPSEC hwlookup miss, will use ipsec table[%d] to do skb recovery", RXINFO_DST_EXTPIDX(pRxInfo) - RTK_FC_IPSEC_SWID_BASE);
		do_IPSEC_recovery = TRUE;
		pktHdr.ipsec_sc_info.skb_recovery_for_hwLookup = 1;
		pktHdr.ingressPort.macPortIdx = __ffs(fc_db.wanPortMask.portmask);
		
		if(unlikely(fc_db.fwdStatistic))
		{
			atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_decrypt_hwlookup_miss[pktHdr.ingressPort.macPortIdx]);
		}
	}
#endif

	if(unlikely(fc_db.fwdStatistic))
	{
		if(RTSKB_DATA(rtskb)[0]==0xff)
			atomic_inc(&fc_db.statistic.perPortCnt_broadcast[pktHdr.ingressPort.macPortIdx]);
		else if(RTSKB_DATA(rtskb)[0]&0x1)
			atomic_inc(&fc_db.statistic.perPortCnt_multicast[pktHdr.ingressPort.macPortIdx]);
		else
			atomic_inc(&fc_db.statistic.perPortCnt_unicast[pktHdr.ingressPort.macPortIdx]);
	}

	if(unlikely(fc_db.rtk_fc_init==0U))
		goto fwd_to_ps;

	if(IS_COPY2CPU_PKT(pRxInfo))
	{
		TOPS("skip learning because COPY to CPU spa=%d",RXINFO_SPA(pRxInfo));
		toPsReason |= FC_DBUG_TOPSREASON_COPY2CPU;
		//DUMP_PACKET(RTSKB_DATA(rtskb), (RTSKB_LEN(rtskb)-RTSKB_DATA_LEN(rtskb)),"[Ingress]");
		goto fwd_to_ps;
	}
	

#if defined(CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH) && (CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH > 5)
	if(fc_db.controlFuc.swAckDelay_en)
	{
		ret = _rtk_fc_ackDelay_check(rtskb, pRxInfo, &pktHdr);
		if(ret == RTK_FC_RET_DROP){
			return RTK_FC_NIC_RX_STOP_SKBNOFREERE;
		}
	}
#endif

	/*
	 * Ingress processing
	 */
#if defined(FC_F_SHORTCUT_EARLYCHECK)
	if(fc_db.controlFuc.shortcut_earlycheck_en)
	{
		 ret = _rtk_fc_shortCut_earlyCheck(rtskb, pRxInfo, &pktHdr);
		if(ret== RTK_FC_RET_LRN_SHORTCUT_TX){
			goto fwd_by_shortcut;
		}
		RTK_FC_TRACE_FILTER_UNLOCK_CHECK();
	}
#endif

	//1. Parse and inspect packet header.
	ret = _rtk_fc_pktHdr_inspect(rtskb, &pktHdr, 0, pRxInfo);

	if(unlikely(ret != RTK_FC_RET_OK))
	{
		DEBUG("skip learning because pktHdr parsing fail, return value: 0x%x", ret);
		toPsReason |= FC_DBUG_TOPSREASON_PKTPARSE_FAIL;
		goto fwd_to_ps;
	}


	 _rtk_fc_trace_log_processing(rtskb, pRxInfo, &pktHdr, RTK_FC_TRACEFILTER_INGRESS);


	if(unlikely(fc_db.controlFuc.gemFilter_conf_en))
		rtk_fc_sw_gemmib_filtering_ingress(rtskb, &pktHdr);

//3 	1.2. bypass specific ethertype, e.g. 802.1x(0x888E).
	if(unlikely(fc_db.systemGlobal.bypass_ethertypeProto_bitmap)) {
		uint32 bitmap = fc_db.systemGlobal.bypass_ethertypeProto_bitmap;
		int curbit = 0;
		do{
			curbit = __ffs(bitmap);
			
			if(curbit>=RTK_FC_ETHERTYPE_BYPASS_NUM)
				break;
			
			if(unlikely(fc_db.systemGlobal.bypass_ethertypeProto[curbit] == pktHdr.ethtype_proto)){
				TRACE("skip fc processing: hit ethertype[%d] 0x%x ", curbit, pktHdr.ethtype_proto);
				toPsReason |= FC_DBUG_TOPSREASON_BYPASS_ETHTYPE;
				goto fwd_to_ps;
			}
			bitmap &= ~(1<<curbit);
		}while(bitmap);
	}
	
	 
	ret = _rtk_fc_lut_learning_processing(rtskb, 0, &pktHdr);

	if(unlikely(ret & RTK_FC_RET_DROP)){
		goto DROP;
	}
	if(unlikely(ret == RTK_FC_RET_LRN_SKIP_UNKNOWNSA)){
		toPsReason |= FC_DBUG_TOPSREASON_UNKNOWN_SMAC;		
		goto fwd_to_ps;
	}

	 //2 1.3. Check sw rate limit
	 if(unlikely(fc_db.controlFuc.swRateLimit_en)) {
		if((_rtk_fc_sw_rate_limit(rtskb, &pktHdr) != RTK_FC_NIC_RX_CONTINUE)) {
				ret = RTK_FC_RET_DROP_SW_RATE_LIMIT;
				goto DROP;
		}
	}

	//3 2.1 Record fwd_statistic info
	if(unlikely(fc_db.fwdStatistic))
		_rtk_fc_record_ingress_statistic(&pktHdr);

	
	//3 3.1 CPU reason process for not support case
	if(unlikely((ret = _rtk_fc_cpuRsnPreProcess(&pktHdr))!= RTK_FC_RET_OK))
	{
		DEBUG("skip learning because of cpu reason %d, return value: 0x%x", RXINFO_REASON(pRxInfo), ret);
		toPsReason |= FC_DBUG_TOPSREASON_CPU_REASON;
		goto fwd_to_ps;
	}
	
	if (fc_db.controlFuc.ip_frag_shortcut && pktHdr.ipFragFlag)
	{
		uint8 tops = FALSE;
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
		
		if ((rtk_fc_negative_ipFrag_cache_find(&pktHdr)) == RTK_FC_RET_OK) {
			FRAGMENT("disorder fragment packet iph id=0x%x, trap to PS.", ntohs(pktHdr.iph->id));
			tops = TRUE;
		} else {
		
			if ((rtk_fc_ipFrag_cache_find(&pktHdr)) != RTK_FC_RET_OK) {
				if (pktHdr.ipFragFlag & RTK_FC_IP_FIRST_FRAG) {
					if ((ret = rtk_fc_ipFrag_cache_add(&pktHdr)) != RTK_FC_RET_OK) {
						FRAGMENT("add frag cache error 0x%x with iph id=0x%x.", ret, ntohs(pktHdr.iph->id));
						tops = TRUE;
					}
				}
				else {
					if ((ret = rtk_fc_negative_ipFrag_cache_add(&pktHdr)) != RTK_FC_RET_OK)
						FRAGMENT("add negative frag cache error 0x%x with iph id=0x%x, trap to PS.", ret, ntohs(pktHdr.iph->id));
					else 
						FRAGMENT("add negative frag cache with iph id=0x%x, trap to PS.", ntohs(pktHdr.iph->id));
					tops = TRUE;
				}
			}
		}
		
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		
		if(tops) goto fwd_to_ps;
	}

	//3 4. Calculate flowHashIdx
	if(likely((ret = _rtk_fc_flow_hashIndex_get(&pktHdr, pRxInfo,rtskb->skb))==RTK_FC_RET_OK))
	{	//3
		if (pktHdr.ipFragFlag) {
			rtk_fc_ipFrag_linkList_t *pfrag_cache;

			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

			pfrag_cache = rtk_fc_ipfrag_shortcut_find_cache(&pktHdr);

			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

			if (!pfrag_cache) {
				FRAGMENT("frag with iph id=0x%x frag_off=0x%x, but frag cache not exists, skip shortcut", 
					ntohs(pktHdr.iph->id), ntohs(pktHdr.iph->frag_off));
				goto fwd_to_ps;
			}

			FRAGMENT("frag with iph id=0x%x get hashindex=0x%x", ntohs(pktHdr.iph->id), pktHdr.flowHashIdx);
		}
		
		//3 5. ShortCut checking (ASAP, so do ignorecheck later)
		

		if(unlikely(pktHdr.abstrSwFlowSearch)) {
			
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
			{
				ret = _rtk_fc_abstrSwShortCutCheck(rtskb, &pktHdr);
			}
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		} else		
			ret = _rtk_fc_shortCutCheck(rtskb, pRxInfo, &pktHdr);

		if (pktHdr.ipFragFlag) {
			if (ret== RTK_FC_RET_LRN_SHORTCUT_TX) {
				// delete ip frag cache here
				if (pktHdr.cacheIpFragInfo.hash_idx_valid) {
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					rtk_fc_ipfrag_ingress_del_cache(&pktHdr);
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
				}

				if(unlikely(fc_db.fwdStatistic))
					atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipFrag[pktHdr.ingressPort.macPortIdx]);
			}
			else {
				FRAGMENT("frag with iph id=0x%x  frag_off=0x%x shortcut fail=0x%x", ntohs(pktHdr.iph->id), ntohs(pktHdr.iph->frag_off), ret);

				if(unlikely(fc_db.fwdStatistic))
					atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipFrag_fail[pktHdr.ingressPort.macPortIdx]);
			}
		}

		if(ret== RTK_FC_RET_LRN_SHORTCUT_TX){ // hit short cut, skb was transmitted already.
			goto fwd_by_shortcut;
		}
		
		if(ret&RTK_FC_RET_DROP){
			TRACE("Drop packet because of ret code 0x%x", ret);
			goto DROP;
		}else { 
			if(ret == RTK_FC_RET_LRN_SKIP_SHORTCUT)		// hit shortcut but need ps to handle it, skip learning
			{
				toPsReason |= FC_DBUG_TOPSREASON_SKIP_SHORTCUT;
				goto fwd_to_ps;
			}
			if(ret == RTK_FC_RET_LRN_DISABLE_SW_ACCELERATION){
				TRACE("Disabling sw acceleration mode (HW only mode). Forward to ps and not caching ingress data for egress learning.");
				skip_data_caching = TRUE;
				toPsReason |= FC_DBUG_TOPSREASON_DIS_SWFWD;
				goto fwd_to_ps;
			}
			if(ret == RTK_FC_RET_LRN_TRAP_TO_PS){
				TRACE("Foward to PS and do not learn flow again because of shortcut dpi");
				skip_data_caching = TRUE;
				toPsReason |= FC_DBUG_TOPSREASON_SHORTCUT_DPI;
				goto fwd_to_ps;
			}
			if(ret == RTK_FC_RET_LRN_SKIP_CTRLPKT)
			{
				TRACE("Hit shortcut but trap by ACL, skip shortcut forwarding and skip flow learning");
				skip_data_caching = TRUE;
				toPsReason |= FC_DBUG_TOPSREASON_CPU_REASON;
				goto fwd_to_ps;
			}
			toPsReason |= FC_DBUG_TOPSREASON_SHORTCUT_MISS;
		}
	}else {
		toPsReason |= FC_DBUG_TOPSREASON_FLOWIDX_FAIL;
		DEBUG("unknow sa/da or flowHashIdx=FAIL(%d), skip shortcut check.",pktHdr.flowHashIdx);

		//dump_packet(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "hash error");
	}
	
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	if(do_IPSEC_recovery)
	{
		RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
		ret = _rtk_fc_ipsec_hwlookup_recovery(rtskb, pRxInfo, &pktHdr);
		RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
		if(ret==FAILED)
			goto fwd_to_ps;	
	}
#endif
	


	//3 6. Ignore Learning to Hw (Pattern/DMac/reservedAddress)
	if((ret = _rtk_fc_flowIgnoreCheck(&pktHdr))!= RTK_FC_RET_OK)
		DEBUG("skip learning, return value: 0x%x", ret);
	else if((ret = _rtk_fc_ingress_dataCaching(rtskb, &pktHdr))!= RTK_FC_RET_OK) {	//Snapshot ingress data, check shortcut and duplicated flow entry.
		DEBUG("flow decision fail, return value: 0x%x", ret);
		if(unlikely(ret==RTK_FC_RET_ERR_EXHAUST_SKB))goto exhaust_skb_drop;
	}

	//3 7. Slowpath
fwd_to_ps:

	if(unlikely((skip_data_caching==FALSE) && (RTSKB_FCIGRDATA(rtskb)->doLearning==FALSE))) {
		if((pktHdr.icmp6h || pktHdr.igmph) || (fc_db.wanAccessLimit.accessLimitNumber>=0) || (fc_db.controlFuc.gemFilter_conf_en)) {
			/* for 
			  *   1. igmp snooping get ingress cvid
			  *   2. wanAccessLimit cache smac/sip
			 */
			ret=_rtk_fc_ingress_dataCaching(rtskb, &pktHdr);
			if(unlikely(ret==RTK_FC_RET_ERR_EXHAUST_SKB))goto exhaust_skb_drop;
			RTSKB_FCIGRDATA(rtskb)->doLearning = FALSE;
		}
	}

	if(unlikely(fc_db.debugLevel & FC_DEBUG_LEVEL_TOPS))
		_rtk_fc_displayInfo_toPs(rtskb, &pktHdr, pRxInfo, toPsReason);

	nic_rx_ret = _rtk_fc_ingress_slowPath(rtskb, &pktHdr, pRxInfo);

	RTK_FC_TRACE_FILTER_UNLOCK_CHECK();

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	if(pktHdr.ipsec_sc_info.skb_recovery_for_hwLookup)
	{
		IPSEC("New device: %s",RTSKB_DEV(rtskb)->name);
		return RTK_FC_NIC_RX_IPSEC_RELEARN;
	}
	else
#endif
	{

		return nic_rx_ret;
	}
fwd_by_shortcut:
exhaust_skb_drop:

	RTK_FC_TRACE_FILTER_UNLOCK_CHECK();	

	return RTK_FC_NIC_RX_STOP_SKBNOFREERE;

DROP:	// Drop packet by NIC
	if(unlikely(fc_db.fwdStatistic) && (ret&RTK_FC_RET_DROP))
	{
		atomic_inc(&fc_db.statistic.perPortCnt_Drop[pktHdr.ingressPort.macPortIdx][ret & ~(RTK_FC_RET_DROP)]);
		// DO NOT FREE SKB
	}
	
	RTK_FC_TRACE_FILTER_UNLOCK_CHECK();

	return RTK_FC_NIC_RX_STOP;
}


/**
 *	rtk_fc_egress_flowCaching - ApolloPro HWNAT egress learning module
 *	@skb: ingress packet buffer
 *	@portMask: egress physical port mask
 *
 *	Called by NIC Tx driver while ASIC try to send packets.
 *	This module will compare ingress data snapshot by ingress module.
 */
int rtk_fc_egress_flowLearning(struct rt_skbuff *rtskb, struct net_device *dev, int portMask, rtk_fc_wlan_devidx_t egrWlanDevIdx)
{
	int ret;

	if(RTSKB_FCIGRDATA(rtskb)->isDummyPkt)
	{
		// dummy packet - free skb
		atomic_inc(&fc_db.statistic.perPortCnt_dummpPkt[RTSKB_FCIGRDATA(rtskb)->ingressPort]);
		RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(RTSKB_SKB(rtskb));
		return 0;
	}

	if (RTK_FC_HOOK_VLAN_PASSTHROUGH_TX_HANDLE(&(RTSKB_SKB(rtskb)), rtskb) != RTK_FC_RET_OK){
		//if failed, skb already free.
		return 0;
	}

#if defined(CONFIG_RTK_SOC_RTL8198D)
	RTK_FC_HOOK_LINUX_VLAN_TX_HANDLE(rtskb, portMask);
#endif

	if(unlikely(fc_db.smpStatistic))
		atomic_inc(&fc_db.smp_statistic[FC_SMP_FROM_PS_RX][smp_processor_id()]);


	RTK_FC_HELPER_LOCAL_BH_DISABLE();
	
	//============= Critical Section Start =============//
	ret = _rtk_fc_egress_flowLearning(rtskb, dev, portMask, egrWlanDevIdx);
	//============= Critical Section End =============//
		
	RTK_FC_HELPER_LOCAL_BH_ENABLE();


	return ret;
}

__IRAM_FC_SHORTCUT
int rtk_fc_ingress_flowLearning(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo)
{
	int ret;

	prefetch(&fc_db.controlFuc);
	prefetch(RTSKB_DATA(rtskb));
	
	RTK_FC_HELPER_LOCAL_BH_DISABLE();
	
	RTK_FC_HELPER_RCU_READ_LOCK();
	
	//============= Critical Section Start =============//
	ret = _rtk_fc_ingress_flowLearning(rtskb, pRxInfo);
	//============= Critical Section End =============//
	
	RTK_FC_HELPER_RCU_READ_UNLOCK();
	
	RTK_FC_HELPER_LOCAL_BH_ENABLE();
	
	return ret;
}

int rtk_fc_coreDBInfo_get(rtk_fc_coreDB_ops_t opCode, rtk_fc_coreDB_info_t* pOpData)
{
	int ret = SUCCESS;
	
	if(pOpData == NULL)
	{
		WARNING("opcode %d but NULL data!", opCode);
		return RTK_FC_RET_ERR_NULL_POINTER;
	}
	
	switch(opCode)
	{
		
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
		case RTK_FC_COREDB_OPS_STREAMID:
			memcpy(&pOpData->data_sid.entry, &fc_db.streamidTbl[pOpData->data_sid.index], sizeof(rtk_fc_tableStreamId_t));			
			break;
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		case RTK_FC_COREDB_OPS_TX_WO_HDR:
			pOpData->data_tx_ctrl = fc_db.controlFuc.tx_with_hdr_debug;
			break;
#endif
#endif
		default:
			WARNING("unknown DB info!");
			return RTK_FC_RET_ERR_INVALID_PARAM;
			break;
	}
	return ret;
}

int rtk_fc_helper_register(rtk_fc_helper_api_t type, void *api)
{

	//printk("[HWNAT] register helper type[%d] api\n", type);

	switch(type)
	{
	case FC_HELPER_TYPE_CONVERTER:
		fc_db.hook_func.converter = (rtk_fc_converter_api_t *)api;
		break;
	case FC_HELPER_TYPE_PS:
		fc_db.hook_func.ps = (rtk_fc_ps_api_t *)api;
		break;
	case FC_HELPER_TYPE_MGR:
		fc_db.hook_func.mgr = (rtk_fc_mgr_api_t *)api;
		break;
	case FC_HELPER_TYPE_MULTI_WAN:
		fc_db.hook_func.rtk_multi_wan = (rtk_fc_multi_wan_api_t *)api;
		break;
	case FC_HELPER_TYPE_VLAN:
		fc_db.hook_func.rtk_vlan = (rtk_fc_vlan_api_t *)api;
		break;
	case FC_HELPER_TYPE_WLAN:
		fc_db.hook_func.wlan = (rtk_fc_wlan_api_t *)api;
		break;
	case FC_HELPER_TYPE_RT_API:
		fc_db.hook_func.rt_help_api = (rtk_fc_rt_helper_api_t * )api;
		break;
	case FC_HELPER_TYPE_EXT_PHY_QOS:
		fc_db.hook_func.rtk_ext_phy_qos = (rtk_ext_phy_qos_t *)api;
		break;
	case FC_HELPER_TYPE_ALL_MAC_PORTMASK:
		fc_db.hook_func.rtk_all_mac_portmask = (rtk_fc_init_all_mac_portmask_t *)api;
		break;
#if defined(CONFIG_FC_RTL8198F_SERIES)
	case FC_HELPER_TYPE_83XX_QOS:
		fc_db.hook_func.rtk83xxqos = (rtk_fc_83xx_qos_api_t *)api;
		break;
	case FC_HELPER_TYPE_8367R_RELAY_MC:
		fc_db.hook_func.rtk_8367r_relay_mc = (rtk_fc_8367r_relay_mc_api_t *)api;
		break;
#endif
	case FC_HELPER_TYPE_MC:
		fc_db.hook_func.rtk_mcast_api = (rtk_fc_mcast_api_t *)api;
		break;
#if defined(CONFIG_RTK_SOC_RTL8198D)|| defined(CONFIG_RTL8198XB_SERIES)
	case FC_HELPER_TYPE_EXT_FLOW_MIB:
		fc_db.hook_func.rtk_ext_flow_mib = (rtk_fc_ext_flow_mib_t *)api;
		break;
#endif
	case FC_HELPER_TYPE_IPFRAG:
		fc_db.hook_func.rtk_ipfrag = (rtk_fc_ipfrag_helper_t *)api;
		break;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	case FC_HELPER_TYPE_TCP:
		fc_db.hook_func.rtk_fc_tcp_helper = (rtk_fc_tcp_helper_t *)api;
		break;
#endif
	case FC_HELPER_TYPE_END:
		//printk("[HWNAT] FC helper register are compeled\n");
		fc_db.rtk_fc_mgr_init = TRUE;
		rtk_fc_param_init();
		break;
	default:
		break;
	}
/*
	if(type == (FC_HELPER_TYPE_END-1))
		fc_db.rtk_fc_mgr_init = TRUE;
*/
	return SUCCESS;
}

int rtk_fc_helper_unregister(rtk_fc_helper_api_t type )
{

	printk("[HWNAT] unregister helper type[%d] api\n", type);

	switch(type)
	{
	case FC_HELPER_TYPE_CONVERTER:
		fc_db.hook_func.converter = NULL;
		break;
	case FC_HELPER_TYPE_PS:
		fc_db.hook_func.ps = NULL;
		break;
	case FC_HELPER_TYPE_MULTI_WAN:
		fc_db.hook_func.rtk_multi_wan = NULL;
		break;
	case FC_HELPER_TYPE_VLAN:
		fc_db.hook_func.rtk_vlan = NULL;
		break;
	case FC_HELPER_TYPE_WLAN:
		fc_db.hook_func.wlan = NULL;
		break;
	case FC_HELPER_TYPE_RT_API:
		fc_db.hook_func.rt_help_api = NULL;
		break;
	case FC_HELPER_TYPE_EXT_PHY_QOS:
		fc_db.hook_func.rtk_ext_phy_qos = NULL;
		break;
	case FC_HELPER_TYPE_ALL_MAC_PORTMASK:
		fc_db.hook_func.rtk_all_mac_portmask = NULL;
		break;
#if defined(CONFIG_FC_RTL8198F_SERIES)
	case FC_HELPER_TYPE_83XX_QOS:
		fc_db.hook_func.rtk83xxqos = NULL;
		break;
	case FC_HELPER_TYPE_8367R_RELAY_MC:
		fc_db.hook_func.rtk_8367r_relay_mc = NULL;
		break;
#endif
	case FC_HELPER_TYPE_MC:
		fc_db.hook_func.rtk_mcast_api = NULL;
		break;

#if defined(CONFIG_RTK_SOC_RTL8198D)
	case FC_HELPER_TYPE_EXT_FLOW_MIB:
		fc_db.hook_func.rtk_ext_flow_mib = NULL;
		break;
#endif
	case FC_HELPER_TYPE_IPFRAG:
		fc_db.hook_func.rtk_ipfrag = NULL;
		break;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	case FC_HELPER_TYPE_TCP:
		fc_db.hook_func.rtk_fc_tcp_helper = NULL;
		break;
#endif
	default:
		break;
	}

	return SUCCESS;
}

static int rtk_fc_api_module_register(rtk_fc_api_module_t* module)
{
	//printk("Enter %s \n",__func__);

	if(RTK_FC_API_MODULE!=NULL || module->rtk_fc_api_init==NULL)
	{
		printk("Error, the hwnat module %s has already been registered.\n",RTK_FC_API_MODULE->rtk_fc_api_name);
		return -1;
	}

	if(module->rtk_fc_api_init()==0 )
	{
		RTK_FC_API_MODULE = module;
		//printk("Register hwnat module %s successfully\n",RTK_FC_API_MODULE->rtk_fc_api_name);
		return 0;
	}

	return -1;
}
void _rtk_fc_skbmark_setting_init(void)
{
	fc_db.skbmark[RTK_FC_SKBMARK_QID].startBit 				= RTK_FC_RMK_QID_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_QID].len 					= RTK_FC_RMK_QID_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_QID].mark1or2 				= 0;                             //0: use mark1; 1: use mark 2
#if defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTK_SOC_RTL8198D)
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].startBit			= RTK_FC_RMK_UNDEF;
#else
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].startBit			= RTK_FC_RMK_STREAMID_BIT_DEF;
#endif
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].len 				= RTK_FC_RMK_STREAMID_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID].mark1or2 		= 0;							 //0: use mark1; 1: use mark 2
#if defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTK_SOC_RTL8198D)
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN].startBit		= RTK_FC_RMK_UNDEF;
#else
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN].startBit 		= RTK_FC_RMK_STREAMID_EN_BIT_DEF;
#endif
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN].len 			= RTK_FC_RMK_STREAMID_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN].mark1or2 		= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_METERIDX].startBit 		= RTK_FC_RMK_METERIDX_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_METERIDX].len 				= RTK_FC_RMK_METERIDX_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_METERIDX].mark1or2 		= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_METERIDX_EN].startBit 		= RTK_FC_RMK_UNDEF;				//RTK_FC_RMK_METERIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_METERIDX_EN].len 			= RTK_FC_RMK_METERIDX_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_METERIDX_EN].mark1or2 		= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_SWSHAPER_EN].startBit 		= RTK_FC_RMK_UNDEF;				//RTK_FC_RMK_METERIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWSHAPER_EN].len 			= RTK_FC_RMK_SWSHAPER_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWSHAPER_EN].mark1or2 		= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_FWDBYPS].startBit 			= RTK_FC_RMK_UNDEF;				 //RTK_FC_RMK_FWDBYPS_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_FWDBYPS].len 				= RTK_FC_RMK_FWDBYPS_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_FWDBYPS].mark1or2 			= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX].startBit 			= RTK_FC_RMK_MIBIDX_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX].len 				= RTK_FC_RMK_MIBIDX_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX].mark1or2 			= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX_EN].startBit 		= RTK_FC_RMK_UNDEF;				//RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX_EN].len 			= RTK_FC_RMK_MIBIDX_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_MIBIDX_EN].mark1or2 		= 0;							 //0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_SKIPFCFUNC].startBit		= RTK_FC_RMK_UNDEF;				//RTK_FC_RMK_SKIPFCEGRESSFUNC_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SKIPFCFUNC].len 			= RTK_FC_RMK_SKIPFCEGRESSFUNC_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SKIPFCFUNC].mark1or2		= 0;							//0: use mark1; 1: use mark 2
	fc_db.skbmark[RTK_FC_SKBMARK_PSFLOODFWDACC].startBit	= RTK_FC_RMK_UNDEF; 	 
	fc_db.skbmark[RTK_FC_SKBMARK_PSFLOODFWDACC].len			= RTK_FC_RMK_PSFLOODFWDACC_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_PSFLOODFWDACC].mark1or2	= 0;
	fc_db.skbmark[RTK_FC_SKBMARK_MAPET_FMR].startBit		= RTK_FC_RMK_UNDEF;
	fc_db.skbmark[RTK_FC_SKBMARK_MAPET_FMR].len				= RTK_FC_RMK_MAPET_FMR_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_MAPET_FMR].mark1or2		= 0;
	
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ID].startBit			= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ID].len 				= RTK_FC_RMK_CVLAN_ID_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ID].mark1or2			= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_PRI].startBit		= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_PRI].len 			= RTK_FC_RMK_CVLAN_PRI_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_PRI].mark1or2		= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ACTION].startBit	    = RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ACTION].len 			= RTK_FC_RMK_CVLAN_ACTION_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_CVLAN_ACTION].mark1or2		= 1;

	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ID].startBit			= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ID].len 				= RTK_FC_RMK_SVLAN_ID_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ID].mark1or2			= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_PRI].startBit		= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_PRI].len 			= RTK_FC_RMK_SVLAN_PRI_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_PRI].mark1or2		= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ACTION].startBit	    = RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ACTION].len 			= RTK_FC_RMK_SVLAN_ACTION_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_ACTION].mark1or2		= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_TPID].startBit	    = RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_MIBIDX_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_TPID].len 			= RTK_FC_RMK_SVLAN_TPID_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SVLAN_TPID].mark1or2		= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_WANDSLOOPBACK_EN].startBit	= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_WANDSLOOPBACK_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_WANDSLOOPBACK_EN].len		= RTK_FC_RMK_WANDSLOOPBACK_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_WANDSLOOPBACK_EN].mark1or2	= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_EN].startBit	= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_SWINTFIDX_ASSIGN_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_EN].len		= RTK_FC_RMK_SWINTFIDX_ASSIGN_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_EN].mark1or2	= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_INOUT].startBit	= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_SWINTFIDX_ASSIGN_INOUT_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_INOUT].len		= RTK_FC_RMK_SWINTFIDX_ASSIGN_INOUT_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_INOUT].mark1or2	= 1;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_IDX].startBit	= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_SWINTFIDX_ASSIGN_IDX_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_IDX].len		= RTK_FC_RMK_SWINTFIDX_ASSIGN_IDX_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_IDX].mark1or2	= 1;
	
	fc_db.skbmark[RTK_FC_SKBMARK_SKIP_FC_ALG_CHECK].startBit	= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_SKIP_FC_ALG_CHECK_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SKIP_FC_ALG_CHECK].len		= RTK_FC_RMK_SKIP_FC_ALG_CHECK_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_SKIP_FC_ALG_CHECK].mark1or2	= 1;

	fc_db.skbmark[RTK_FC_SKBMARK_FLOW_CACHE_MIB_EN].startBit	= RTK_FC_RMK_UNDEF; 	 //RTK_FC_RMK_FLOW_CACHE_MIB_EN_BIT_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_FLOW_CACHE_MIB_EN].len		= RTK_FC_RMK_FLOW_CACHE_MIB_EN_LEN_DEF;
	fc_db.skbmark[RTK_FC_SKBMARK_FLOW_CACHE_MIB_EN].mark1or2	= 1;
	
	RTK_FC_HELPER_MGR_SKBMARK_CONF_SET(FC_MGR_SKBMARK_QID, fc_db.skbmark[RTK_FC_SKBMARK_QID]);
	RTK_FC_HELPER_MGR_SKBMARK_CONF_SET(FC_MGR_SKBMARK_STREAMID, fc_db.skbmark[RTK_FC_SKBMARK_STREAMID]);
	RTK_FC_HELPER_MGR_SKBMARK_CONF_SET(FC_MGR_SKBMARK_STREAMID_EN, fc_db.skbmark[RTK_FC_SKBMARK_STREAMID_EN]);
	RTK_FC_HELPER_MGR_SKBMARK_CONF_SET(FC_MGR_SKBMARK_SKIPFCFUNC, fc_db.skbmark[RTK_FC_SKBMARK_SKIPFCFUNC]);
}
#if defined(CONFIG_FC_RTL9607C_SERIES)	
void _rtk_fc_nptv6_acceleration_mechanism_set(int value)
{
	int i;
	
	//Reset nptv6 acceleration mechanism
	
	for(i=0; i<MAX_NPTV6_ACC_UPSTREAM_SIZE; i++)
		if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].vaild)
			_rtk_fc_lut_staticEntry_del(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream[i].virtual_dmacL2Idx, FALSE);
	for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM_SIZE; i++)
		if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].vaild)
			_rtk_fc_lut_staticEntry_del(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream[i].virtual_dmacL2Idx, FALSE);
	for(i=0; i<MAX_NPTV6_ACC_UPSTREAM0_SIZE; i++)
		if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[i].vaild)
			_rtk_fc_lut_staticEntry_del(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_upstream0[i].virtual_dmacL2Idx, FALSE);
	for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM0_SIZE; i++)
		if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[i].vaild)
			_rtk_fc_lut_staticEntry_del(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_downstream0[i].virtual_dmacL2Idx, FALSE);
		
	assert_ok(rtk_fc_flow_flush_all());
	
	for(i=0; i<RTK_FC_TABLESIZE_LUT; i++)
	{
		if(fc_db.lutTbl[i] != NULL)
		{
			if(fc_db.lutTbl[i]->l2Addr[0] == 0x0 && fc_db.lutTbl[i]->l2Addr[2] == 0x5e)
			{
				if(fc_db.lutTbl[i]->isStatic)
					_rtk_fc_lut_staticEntry_del(fc_db.lutTbl[i]->lutIdx, FALSE);
				else
					RTK_FC_LUT_DEL(fc_db.lutTbl[i], TRUE);
			}
		}	
	}
	memset(&fc_db_fastFwd_data.nptv6_acc, 0, sizeof(rtk_fc_nptv6_acceleration_data_t));
	

//#if !defined(CONFIG_GMAC2_USABLE)
	if(fc_db.controlFuc.gmac2_usable==0)
	{
		printk("\033[1;33;41m[WARNING] Forcibly disable nptv6 acceleration mechanism, due to CONFIG_GMAC2_USABLE is disabled !!!\033[0m\n");
		fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism = RTK_FC_NPTV6_ACC_DISABLE;
		return ;
	}
//#endif	
#if !defined(CONFIG_GMAC1_USABLE)
	printk("\033[1;33;41m[WARNING] Forcibly disable nptv6 acceleration mechanism, due to CONFIG_GMAC1_USABLE is disabled !!!\033[0m\n");
	fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism = RTK_FC_NPTV6_ACC_DISABLE;
	return ;
#endif

	if(fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2==1U)
	{
		//upstream
#if 1//defined(CONFIG_CMCC)//CMCC
		fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum = (dynamic_sram_desc)?2U:1U;
#elif defined(CONFIG_YUEME) //CTC
		fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum = (dynamic_sram_desc)?3:1;
#endif
		fc_db_fastFwd_data.nptv6_acc.upstreamExtport = FC_EXT_MAC10_PORT1;//RTK_RG_MAC10_EXT_PORT1;
		fc_db_fastFwd_data.nptv6_acc.upstreamCpuport = RTK_FC_MAC_PORT_MASTERCPU_CORE1;//RTK_RG_PORT_MASTERCPU_CORE1; //10
		fc_db_fastFwd_data.nptv6_acc.upstreamGmac = 1U;
		fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum = 4U;
		fc_db_fastFwd_data.nptv6_acc.upstreamTxRingNum = 3U;
		fc_db_fastFwd_data.nptv6_acc.upstreamPriority = fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum+2; //6
		
		if(fc_db.controlFuc.hwnat_customize==1U)
			fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.upstreamPriority - fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum;
		else
			fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.upstreamPriority; //6
		//upstream0
#if 1//defined(CONFIG_CMCC)//CMCC
		fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum = (dynamic_sram_desc)?2U:0U;
#elif defined(CONFIG_YUEME)

		fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum = (dynamic_sram_desc)?1:0;
#endif

		fc_db_fastFwd_data.nptv6_acc.upstream0Extport = FC_EXT_MAC7_PORT1;//RTK_RG_MAC7_EXT_PORT1;
		fc_db_fastFwd_data.nptv6_acc.upstream0Cpuport = RTK_FC_MAC_PORT_SLAVECPU; //7
		fc_db_fastFwd_data.nptv6_acc.upstream0Gmac = 2U;
		fc_db_fastFwd_data.nptv6_acc.upstream0RxRingNum = 4U;
		fc_db_fastFwd_data.nptv6_acc.upstream0TxRingNum = 3U;
		fc_db_fastFwd_data.nptv6_acc.upstream0Priority = fc_db_fastFwd_data.nptv6_acc.upstream0RxRingNum+2; //6
		
		if(fc_db.controlFuc.hwnat_customize)
			fc_db_fastFwd_data.nptv6_acc.upstream0Priority_rg = fc_db_fastFwd_data.nptv6_acc.upstream0Priority - fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum;
		else
			fc_db_fastFwd_data.nptv6_acc.upstream0Priority_rg = fc_db_fastFwd_data.nptv6_acc.upstream0Priority; //6
		//downstream
		fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum = (dynamic_sram_desc)?4U:1U;
		fc_db_fastFwd_data.nptv6_acc.downstreamExtport = (dynamic_sram_desc)?FC_EXT_MAC9_PORT2:FC_EXT_MAC7_PORT2;//RTK_RG_EXT_PORT2:RTK_RG_MAC7_EXT_PORT2;
		fc_db_fastFwd_data.nptv6_acc.downstreamCpuport = (dynamic_sram_desc)?RTK_FC_MAC_PORT_MASTERCPU_CORE0:RTK_FC_MAC_PORT_SLAVECPU; //9 or 7
		fc_db_fastFwd_data.nptv6_acc.downstreamGmac = (dynamic_sram_desc)?0U:2U;
		fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum = 4U;
		fc_db_fastFwd_data.nptv6_acc.downstreamTxRingNum = 3U;
		fc_db_fastFwd_data.nptv6_acc.downstreamPriority = fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum+2; //6

		if(fc_db.controlFuc.hwnat_customize)		
			fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.downstreamPriority - fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum;
		else
			fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.downstreamPriority; //6

		//downstream0
		fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum = 0U;
		fc_db_fastFwd_data.nptv6_acc.downstream0Cpuport = RTK_FC_MAC_PORT_MAX;
	}
	else
	{
		
		//upstream
		fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum = (dynamic_sram_desc)?4U:1U;
		fc_db_fastFwd_data.nptv6_acc.upstreamExtport = FC_EXT_MAC10_PORT1;//RTK_RG_MAC10_EXT_PORT1;
		fc_db_fastFwd_data.nptv6_acc.upstreamCpuport = RTK_FC_MAC_PORT_MASTERCPU_CORE1; //10
		fc_db_fastFwd_data.nptv6_acc.upstreamGmac = 1U;
		fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum = 4U;
		fc_db_fastFwd_data.nptv6_acc.upstreamTxRingNum = 3U;
		fc_db_fastFwd_data.nptv6_acc.upstreamPriority = fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum+2; //6
		if(fc_db.controlFuc.hwnat_customize)	
			fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.upstreamPriority - fc_db_fastFwd_data.nptv6_acc.upstreamNicFlowNum;
		else
			fc_db_fastFwd_data.nptv6_acc.upstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.upstreamPriority; //6
		
		//upstream0
		fc_db_fastFwd_data.nptv6_acc.upstream0NicFlowNum = 0U;
		fc_db_fastFwd_data.nptv6_acc.upstream0Cpuport = RTK_FC_MAC_PORT_MAX;
		
		//downstream
		fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum = (dynamic_sram_desc)?2U:1U;
		fc_db_fastFwd_data.nptv6_acc.downstreamExtport = FC_EXT_MAC7_PORT2;//RTK_RG_MAC7_EXT_PORT2;
		fc_db_fastFwd_data.nptv6_acc.downstreamCpuport = RTK_FC_MAC_PORT_SLAVECPU; //7
		fc_db_fastFwd_data.nptv6_acc.downstreamGmac = 2U;
		fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum = 4U;
		fc_db_fastFwd_data.nptv6_acc.downstreamTxRingNum = 3U;
		fc_db_fastFwd_data.nptv6_acc.downstreamPriority = fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum+2; //6
		if(fc_db.controlFuc.hwnat_customize)		
			fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.downstreamPriority - fc_db_fastFwd_data.nptv6_acc.downstreamNicFlowNum;
		else
			fc_db_fastFwd_data.nptv6_acc.downstreamPriority_rg = fc_db_fastFwd_data.nptv6_acc.downstreamPriority; //6
		
		//downstream0
		fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum = (dynamic_sram_desc)?2U:0U;
		fc_db_fastFwd_data.nptv6_acc.downstream0Extport = FC_EXT_MAC9_PORT2;//RTK_RG_EXT_PORT2;
		fc_db_fastFwd_data.nptv6_acc.downstream0Cpuport = RTK_FC_MAC_PORT_MASTERCPU_CORE0; //9
		fc_db_fastFwd_data.nptv6_acc.downstream0Gmac = 0U;
		fc_db_fastFwd_data.nptv6_acc.downstream0RxRingNum = 4U;
		fc_db_fastFwd_data.nptv6_acc.downstream0TxRingNum = 3U;
		fc_db_fastFwd_data.nptv6_acc.downstream0Priority = fc_db_fastFwd_data.nptv6_acc.downstream0RxRingNum+2; //6
		if(fc_db.controlFuc.hwnat_customize)		
			fc_db_fastFwd_data.nptv6_acc.downstream0Priority_rg = fc_db_fastFwd_data.nptv6_acc.downstream0Priority - fc_db_fastFwd_data.nptv6_acc.downstream0NicFlowNum;
		else
			fc_db_fastFwd_data.nptv6_acc.downstream0Priority_rg = fc_db_fastFwd_data.nptv6_acc.downstream0Priority; //6

	}
	

	
	
	fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism = value;
	if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism > 0)
	{
		printk("\033[1;33;41mNPTv6 mode: %d \033[0m\n",fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism);
		if(fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2==1U)
		{
			if(dynamic_sram_desc==1)
			{
				uint32 rate=MAX_NPTV6_ACC_UPSTREAM_BIT_RATE;
				rtk_fc_enable_t ifgInclude=RTK_FC_ENABLED;
#if 0				
				if(ASICDRIVERVER == 0x3)
				{
					//Bus Priority Switch Arbiter
					REG32(0xb8001a20) = 0xa54a5555;
					REG32(0xb8001a24) = 0x2aa5552a;
					REG32(0xb8001a28) = 0x52a52aa2;
					REG32(0xb8001a2c) = 0x55a2aa55;
					regValue = REG32(0xb800101c);
					REG32(0xb800101c) = regValue ;

				}
#endif					
				//for downstream
				//assert_ok(rtk_rate_portIgrBandwidthCtrlRate_set(RTK_FC_MAC_PORT_PON, MAX_NPTV6_ACC_DOWNSTREAM_BIT_RATE));
				//assert_ok(rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(RTK_FC_MAC_PORT_PON, ifgInclude));
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == RTK_FC_NPTV6_ACC_ENABLE_FOR_4LAN_PON_WAN)
				{
					assert_ok(rtk_rate_portIgrBandwidthCtrlRate_set(RTK_FC_MAC_PORT_PON, MAX_NPTV6_ACC_DOWNSTREAM_BIT_RATE));
					assert_ok(rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(RTK_FC_MAC_PORT_PON, ifgInclude));
				
				}
				else if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == RTK_FC_NPTV6_ACC_ENABLE_FOR_2LAN_PON_WAN)
				{
					assert_ok(rtk_rate_portIgrBandwidthCtrlRate_set(RTK_FC_MAC_PORT_PON, MAX_NPTV6_ACC_2LAN_PON_WAN_DOWNSTREAM_BIT_RATE));
					assert_ok(rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(RTK_FC_MAC_PORT_PON, ifgInclude));
				
				}

				for(i=0; i<4; i++)
				{
					//_rtk_rg_aclAndCfReservedRuleDel(RTK_FC_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN0_ASSIGN_SHARE_METER+i);
					if(fc_db.meterIdx_fromLan[i] >= 0)
					{
						//rt_rate_meter_mapping_t meterMapping;
						rt_acl_filterAndQos_t acl_parameter;
						///int tmp_index;
						if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == RTK_FC_NPTV6_ACC_ENABLE_FOR_4LAN_PON_WAN)
						{
							if(ASICDRIVERVER == 0x3) // RL6831, 9607C-VB 
							{
								if(i == 0)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE_FOR_RL6831;
								else if (i==1)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE;
								else if(i == 2)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE_FOR_RL6831;
								else if(i == 3)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE_FOR_RL6831;

							}
							else
							{
								if(i == 0)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE;
								else if (i==1)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE;
								else if(i == 2)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE;
								else if(i == 3)
										rate = MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE;

							}

						}
						else if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == RTK_FC_NPTV6_ACC_ENABLE_FOR_2LAN_PON_WAN)
						{
							if(i == 0)
									rate = MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN0_BIT_RATE;
 							else if (i==1)
									rate = MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN1_BIT_RATE;
 							else if(i == 2)
									rate = MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN2_BIT_RATE;
 							else if(i == 3)
									rate = MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN3_BIT_RATE;

						}
						else if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == RTK_FC_NPTV6_ACC_ENABLE_FOR_2LAN_ETHERNET_WAN)
 						{
							
						
 							if(i == 0)
									rate = MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN0_BIT_RATE;
 							else if (i==1)
									rate = MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN1_BIT_RATE;
 							else if(i == 2)
									rate = MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN2_BIT_RATE;
 							else if(i == 3)
									rate = MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN3_BIT_RATE;
 
							if(fc_db.wanPortMask.portmask & (1<<i) )
							{
								printk("ethernet WAN (port: %d) setting!\n",i);
								rate = MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_DOWNSTREAM_BIT_RATE;
								
							}
						}
						

						
						
						
						
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();

						if(fc_db.meterIdx_fromLan[i] == 0)
						{
							printk("First time to set acl meter!\n");
							// rt_rate add to get index
							rt_rate_shareMeterType_add(RT_METER_TYPE_ACL, &fc_db.meterIdx_fromLan[i]);
							//WARNING("fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[%d] = %d rate = %d",i, fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[i],rate);
							rt_rate_shareMeterRate_set(fc_db.meterIdx_fromLan[i], rate);
						
							rt_rate_shareMeterBucket_set(fc_db.meterIdx_fromLan[i], 3000);
					

						}
						else
						{
							printk("Second time to set acl meter! re-use index\n");
							rt_rate_shareMeterRate_set(fc_db.meterIdx_fromLan[i], rate);
							rt_rate_shareMeterBucket_set(fc_db.meterIdx_fromLan[i], 3000);

						}

						
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
						//RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[i], &meterMapping);

						//WARNING("ACL meter sw index: %d",fc_db_fastFwd_data.nptv6_acc.meterIdx_fromLan[i]);

						if(fc_db.meterACLIdx_fromLan_isSet[i] == 0)
						{
	
							memset(&acl_parameter, 0, sizeof(rt_acl_filterAndQos_t));

							acl_parameter.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT;
							acl_parameter.ingress_port_mask = 1<<i;
							
							acl_parameter.action_fields |= RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT;
							acl_parameter.action_meter_group_share_meter_index = fc_db.meterIdx_fromLan[i];

							acl_parameter.acl_weight = 65535;

							//acl_add_result = rt_acl_filterAndQos_add(acl_parameter, &acl_index);
							_rtk_fc_aclFilterAndQos_add(&acl_parameter, &fc_db.meterACLIdx_fromLan[i]);
							fc_db.meterACLIdx_fromLan_isSet[i] = 1;
						}

						
						//_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN0_ASSIGN_SHARE_METER+i, &nptv6_acc_upstream_assign_share_meter);
					}
				}
			}	

		}
		
		{
			rtk_qos_pri2queue_t pri2queue;
			pri2queue.pri2queue[0] = 0U;
			pri2queue.pri2queue[1] = 1U;
			pri2queue.pri2queue[2] = 2U;
			pri2queue.pri2queue[3] = 6U;
			pri2queue.pri2queue[4] = 6U;
			pri2queue.pri2queue[5] = 6U;
			pri2queue.pri2queue[6] = 6U;
			pri2queue.pri2queue[7] = 7U;
			assert_ok(rtk_qos_priMap_set(1U, &pri2queue));
			
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, 1));
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, 1));
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_SLAVECPU, 1));
			
		}
		printk("nptv6_acceleration_mechanism enabled, ready to set cpu affinity\n");
		if(dynamic_sram_desc>0U)
		{
#ifdef CONFIG_SMP
#if CONFIG_NR_CPUS == 4
			cpumask_gmac0.bits[0]=0x8;  //CPU 3
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC0, cpu port 9

			cpumask_gmac1.bits[0]=0x2;  //CPU 1
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC1, cpu port 10
			
//#if defined(CONFIG_GMAC2_USABLE)
			if(fc_db.controlFuc.gmac2_usable==1)
			{
				//cpumask_gmac2.bits[0]=0x4;  //CPU 2
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_mechanism == RTK_FC_NPTV6_ACC_ENABLE_FOR_2LAN_ETHERNET_WAN)
					cpumask_gmac2.bits[0]=0x1;  //CPU 0
				else
					cpumask_gmac2.bits[0]=0x4;  //CPU 2
				RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC2, cpu port 7
			}
//#endif
#elif CONFIG_NR_CPUS == 2
			cpumask_gmac0.bits[0]=0x1;  //CPU 0
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC0, cpu port 9

			cpumask_gmac1.bits[0]=0x2;  //CPU 1
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC1, cpu port 10
			
//#if defined(CONFIG_GMAC2_USABLE)
			if(fc_db.controlFuc.gmac2_usable==1)
			{
				cpumask_gmac2.bits[0]=0x1;  //CPU 0
				RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC2, cpu port 7
			}
//#endif
#endif
#endif
		}
		else
		{
			{
				cpumask_gmac0.bits[0]=0x2;  //CPU 1
				RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9
		
				cpumask_gmac1.bits[0]=0x2;  //CPU 1
				RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
			}
		}
	}
	else
	{
		if(fc_db.controlFuc.hwnat_customize)
		{
			//upstream
			for(i=0; i<MAX_NPTV6_ACC_UPSTREAM_NIC_FLOW_NUM; i++)
			{
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i].preallocated_outer_length)
				{
					struct rtl8686_hwnat_customized_entry customized_entry={0};
					customized_entry.type=CUSTOMIZE_TYPE_NPTV6_UP;
					customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.upstreamRxRingNum - i;
					customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.upstreamTxRingNum - i;
					customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.upstreamGmac;
					re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
				}
				memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream[i], 0, sizeof(rtk_fc_nptv6_acceleration_nic_flow_t));
			}
			
			//downstream
			for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM_NIC_FLOW_NUM; i++)
			{
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i].preallocated_outer_length)
				{
					struct rtl8686_hwnat_customized_entry customized_entry={0};
					customized_entry.type=CUSTOMIZE_TYPE_NPTV6_DOWN;
					customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.downstreamRxRingNum - i;
					customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.downstreamTxRingNum - i;
					customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.downstreamGmac;
					re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
				}
				memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream[i], 0, sizeof(rtk_fc_nptv6_acceleration_nic_flow_t));
			}

			//upstream0
			for(i=0; i<MAX_NPTV6_ACC_UPSTREAM0_NIC_FLOW_NUM; i++)
			{
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i].preallocated_outer_length)
				{
					struct rtl8686_hwnat_customized_entry customized_entry={0};
					customized_entry.type=CUSTOMIZE_TYPE_NPTV6_UP;
					customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.upstream0RxRingNum - i;
					customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.upstream0TxRingNum - i;
					customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.upstream0Gmac;
					re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
				}
				memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_upstream0[i], 0, sizeof(rtk_fc_nptv6_acceleration_nic_flow_t));
			}

			//downstream0
			for(i=0; i<MAX_NPTV6_ACC_DOWNSTREAM0_NIC_FLOW_NUM; i++)
			{
				if(fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i].preallocated_outer_length)
				{
					struct rtl8686_hwnat_customized_entry customized_entry={0};
					customized_entry.type=CUSTOMIZE_TYPE_NPTV6_DOWN;
					customized_entry.rxRingNum=fc_db_fastFwd_data.nptv6_acc.downstream0RxRingNum - i;
					customized_entry.txRingNum=fc_db_fastFwd_data.nptv6_acc.downstream0TxRingNum - i;
					customized_entry.gmac=fc_db_fastFwd_data.nptv6_acc.downstream0Gmac;
					re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
				}
				memset(&fc_db_fastFwd_data.nptv6_acc.nptv6_acceleration_nic_flow_downstream0[i], 0, sizeof(rtk_fc_nptv6_acceleration_nic_flow_t));
			}
		}
		

		if(fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2==1U)
		{
			if(dynamic_sram_desc)
			{
				//for downstream
				assert_ok(rtk_rate_portIgrBandwidthCtrlRate_set(RTK_FC_MAC_PORT_PON, 4194296));
				assert_ok(rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(RTK_FC_MAC_PORT_PON, DISABLED));

			}	

		}
		{
			rtk_qos_pri2queue_t pri2queue;
			pri2queue.pri2queue[0] = 0;
			pri2queue.pri2queue[1] = 0;
			pri2queue.pri2queue[2] = 0;
			pri2queue.pri2queue[3] = 0;
			pri2queue.pri2queue[4] = 0;
			pri2queue.pri2queue[5] = 0;
			pri2queue.pri2queue[6] = 0;
			pri2queue.pri2queue[7] = 0;
			assert_ok(rtk_qos_priMap_set(1, &pri2queue));
			
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, 3));
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, 3));
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_SLAVECPU, 3));
		
		}
		
		printk("nptv6_acceleration_mechanism disabled, ready to set cpu affinity\n");
		//reset to default init value
#ifdef CONFIG_SMP
		cpumask_gmac0.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9

		cpumask_gmac1.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
#endif	
	}
	

}


#endif
 

static const u8 mix_mode_hash_padding[64] __attribute__((aligned(32))) =
{
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
void rtk_fc_wifi_tx_mac_sel_init(void)
{
	int spa = 0;

	for(spa = RTK_FC_MAC_PORT0 ; spa < RTK_FC_MAC_PORT_MAX; spa++) {
#if defined(CONFIG_FC_WIFI_TX_GMAC_RR_SUPPORT)
		fc_db.controlFuc.wifi_tx_gmac_sel[spa] = RTK_FC_WIFI_GMAC_RR;
#else
		if(spa < RTK_FC_MAC_PORT_PON)
			fc_db.controlFuc.wifi_tx_gmac_sel[spa] = RTK_FC_WIFI_GMAC_TBD;
		else
			fc_db.controlFuc.wifi_tx_gmac_sel[spa] = RTK_FC_WIFI_GMAC_RR;
#endif
	}
}
#endif

int rtk_fc_param_init(void)
{
	int i;
	rtk_fc_api_module_t *module=NULL;
	int REAL_BCAM_SIZE;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_port_macAbility_t cpuAbility;
#endif
#if defined(CONFIG_FC_RTL8198F_SERIES)
	unsigned char ca_ne_port_id;
	rtk_ne_reg_write(0x00007FFF, NI_HV_GLB_INTF_RST_CONFIG);
#endif
	
#if defined(CONFIG_APOLLOPRO_FPGA) || defined(CONFIG_APOLLO_FPGA_PHY_TEST)
	REAL_BCAM_SIZE = 4;
#else
#if defined(CONFIG_RTK_L34_G3_PLATFORM)	
	REAL_BCAM_SIZE = 32;
#else
	REAL_BCAM_SIZE = ((ASIC_CHIP_ID==RTL9607C_CHIP_ID) ? 64 : 8);
#endif
#endif

	//printk("Enter %s \n",__func__);
	fc_db.debugLevel = FC_DEBUG_LEVEL_WARN;
	fc_db.filterLevel = 0;
	fc_db.traceFilterRuleMask=0x1U;
	fc_db.all_mac_portmask_without_cpu = RTK_FC_HELPER_INIT_ALL_MAC_PORTMASK_WITHOUT_CPU();
	//printk("RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU: 0x%x\n", RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU);

#if defined(CONFIG_FC_RTL8198F_SERIES)
	ca_ne_port_id = ca_ni_get_wan_port_id();
	fc_db.wanPortMask.portmask = (1ULL<<ca_ne_port_id);
#else
	fc_db.wanPortMask.portmask = (1ULL<<RTK_FC_MAC_PORT_PON);
	RTK_FC_HELPER_MGR_WAN_PORT_MASK_SET((1ULL<<RTK_FC_MAC_PORT_PON));
#endif
	fc_db.wanPortWifidevMask = 0ULL;
	fc_db.lanPortMask.portmask = (RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU& (~(fc_db.wanPortMask.portmask)));
	fc_db.cpuPortMask.portmask = RTK_FC_ALL_MAC_CPU_PORTMASK;

#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_fc_flow_limit_init();
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	fc_db.tcp_in_window_shortcut_check = 0U;
	fc_db.tcp_in_window_shortcut_fail_action = RTK_FC_TCP_IN_WINDOW_FAIL_FREE_SKB;
#endif

	/* PROC file */
	rtk_fc_proc_init();
	
	//========================= Critical Section Start =========================//
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	fc_db.controlFuc.port4_hsgmii_en = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_PORT4_HSGMII_EN);
#endif
	fc_db.controlFuc.loopbackMode_is_enabled = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_LOOPBACK_MODE);
	fc_db.controlFuc.module_probe_log = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_PROBE_LOG);
	fc_db.controlFuc.special_fast_forward_mode = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_SPECIAL_FAST_FORWARD);
	fc_db.controlFuc.hwnat_customize = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_HWNAT_CUSTOMIZE);
	fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2 = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_HWNAT_CUSTOMIZE_NPTV6_SRAM_ACC_V2);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	fc_db.controlFuc.gmac2_usable = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_GMAC2_USABLE);
	fc_db.controlFuc.wifi_tx_trap_hash = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GLB_CONFIG_WIFI_TX_TRAP_HASH);
	fc_db.controlFuc.wifi_tx_gmac_trunking_num = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GLB_CONFIG_WIFI_GMAC_TRUNKING_NUM);
	fc_db.controlFuc.wifi_tx_gmac_auto_sel_en = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GLB_CONFIG_WIFI_TX_GMAC_AUTO_SEL);
	rtk_fc_wifi_tx_mac_sel_init();
#endif
	fc_db.controlFuc.mem_leak_debug = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_MEMLEAK_DEBUG);
	fc_db.controlFuc.external_switch_en = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_EXTERNAL_SWITCH_ENABLE);
	fc_db.controlFuc.external_switch_port_offset = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_EXTERNAL_SWITCH_PORT_OFFSET);
	fc_db.controlFuc.vlan_filtering = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_VLAN_FILTERING);
	fc_db.controlFuc.ipsec_fastFwd = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_IPSEC_FASTFWD);
	
	MODINITPRINT("fc_db.controlFuc.external_switch_en = %d external_switch_port_offset = %d\n",fc_db.controlFuc.external_switch_en,fc_db.controlFuc.external_switch_port_offset);
	MODINITPRINT("fc_db.controlFuc.mem_leak_debug = %d fc_db.controlFuc.vlan_filtering = %d\n",fc_db.controlFuc.mem_leak_debug,fc_db.controlFuc.vlan_filtering);
	
	MODINITPRINT("fc_db.controlFuc.special_fast_forward_mode = %d fc_db.controlFuc.hwnat_customize = %d fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2 = %d\n",fc_db.controlFuc.special_fast_forward_mode, fc_db.controlFuc.hwnat_customize, fc_db.controlFuc.hwnat_customize_nptv6_sram_acc_v2);
	
	fc_db.controlFuc.sw_per_flow_mib = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_PER_SW_FLOW_MIB);
	RTK_FC_HELPER_SPIN_LOCK_INIT(&fc_db.lock_memDebug);
	INIT_LIST_HEAD(&fc_db.memAlloc);

	/* HWNAT API Init. */
	module = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_api_module_t), GFP_ATOMIC);

	if(module==NULL)
	{
		printk("Allocate hwnat module failed.\n");
#ifdef CONFIG_FC_RTL8198F_SERIES
		rtk_ne_reg_write(0, NI_HV_GLB_INTF_RST_CONFIG);
#endif
		return -1;
	}
	memset(module,0,sizeof(rtk_fc_api_module_t));
	strcpy(module->rtk_fc_api_name,RTK_FC_API_MODULE_NAME);
	module->rtk_fc_api_init = NULL;


#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	module->rtk_fc_api_init = rtk_fc_rtl9607c_API_init;
	/* Netif callback func */
	module->rtk_fc_api_Netif_inetAddrEvent = rtk_fc_eventHandler_inetAddr;
	module->rtk_fc_api_Netif_inet6AddrEvent = rtk_fc_eventHandler_inet6Addr;
	module->rtk_fc_api_Netif_netDevEvent = rtk_fc_eventHandler_netDev;
	/* LUT SA learning callback func */
	module->rtk_fc_api_lut_add = rtk_fc_rtl9607c_API_lut_add;
	module->rtk_fc_api_lut_del = rtk_fc_rtl9607c_API_lut_del;
	module->rtk_fc_api_lut_query = rtk_fc_rtl9607c_API_lut_query;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	module->rtk_fc_api_init = rtk_fc_ca_API_init;
	module->rtk_fc_api_exit = rtk_fc_pf_exit;
	/* Netif callback func */
	module->rtk_fc_api_Netif_inetAddrEvent = rtk_fc_eventHandler_inetAddr;
	module->rtk_fc_api_Netif_inet6AddrEvent = rtk_fc_eventHandler_inet6Addr;
	module->rtk_fc_api_Netif_netDevEvent = rtk_fc_eventHandler_netDev;
	/* LUT SA learning callback func */
	module->rtk_fc_api_lut_add = rtk_fc_ca_API_lut_add;
	module->rtk_fc_api_lut_del = rtk_fc_ca_API_lut_del;
	module->rtk_fc_api_lut_query = rtk_fc_ca_API_lut_query;
#endif

	if(rtk_fc_api_module_register(module)!=0)
	{
		WARNING("Register hwnat module %s failed.\n",module->rtk_fc_api_name);
#ifdef CONFIG_FC_RTL8198F_SERIES
		rtk_ne_reg_write(0, NI_HV_GLB_INTF_RST_CONFIG);
#endif
		//RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
		RTK_FC_HELPER_FC_KFREE(module, sizeof(rtk_fc_api_module_t));
		return -1;
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();

	fc_db.vxlan_us_fastForward_wan_intf_id = -1;
	fc_db.bcMacIdx = FAIL;

#if defined(CONFIG_FC_CA8277B_SERIES)

	for(i = 0 ; i < 4 ; i ++)
	{
		fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[i]		= CA_UINT32_INVALID ;
		fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[i]		= CA_UINT32_INVALID ;
		fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[i]	= CA_UINT32_INVALID ;
		fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[i]	= CA_UINT32_INVALID ;

	}
	fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[0] = 0x14;
	fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[1] = 0x15;
	fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[2] = 0x16;
	fc_db.vxlan_fastFwd_RoundRobin_cpuPort_array[3] = 0x17;
#endif	
	

	fc_db.shortcut_flow_count = RTK_FC_DEFAULT_SHORTCUT_FLOW_SIZE;	//this can not be called in _rtk_fc_flowDataBase_init() (_rtk_fc_flowDataBase_init() will be called when shortcut_flow_count or flow_mode changing)
	_rtk_fc_flowDataBase_init();
	rtk_fc_dualInfoInit();

	rtk_fc_mape_dst6Info_init();
	
	_rtk_fc_skbmark_setting_init();

	_rtk_fc_igmp_mld_module_init();

	rtk_fc_abstrSwFlowModule_init();
	//should after rtk_fc_api_module_register(),(rtk_fc_rtl9607c_API_init) we will reference default  RTK_RG_ASIC_GLOBALSTATE_SET and fc_db.systemGlobal.flowCheckState
	rtk_fc_abstrSwFlowPatternSet();

	/* LUT pool */
	_rtk_fc_lutEntry_pool_init();
	for(i=0;i<RTK_FC_LUT_BUCKET_SIZE;i++) {
		INIT_LIST_HEAD(&fc_db.lut_hash_list_head[i]);
		INIT_LIST_HEAD(&fc_db.lut_quickhash_list_head[i]);
	}

	/* LUT CAM Init */
//#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	//init software lut cam free link list
	INIT_LIST_HEAD(&fc_db.lutCam_freeListHead);
	//init software lut cam head table
	for(i=0;i<RTK_FC_LUT_BUCKET_SIZE;i++)
		INIT_LIST_HEAD(&fc_db.lutCam_hashListHead[i]);

	for(i=0;i<REAL_BCAM_SIZE;i++)
	{
		INIT_LIST_HEAD(&fc_db.lutCamList[i].lut_list);
		fc_db.lutCamList[i].idx=i+RTK_FC_TABLESIZE_LUT-LUTTABLE_BCAM_SIZE;

		//add free list to free list head
		list_add_tail(&fc_db.lutCamList[i].lut_list,&fc_db.lutCam_freeListHead);
	}
//#endif
	//init vlan group mac limist mac head
	for(i=0;i<RTK_FC_MAX_LUT_HW_HASH_HEAD;i++)
		INIT_LIST_HEAD(&fc_db.vlanGroupMACLimit_macHead[i]);

	for(i = 0 ; i < RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE; i ++)
	{
		bzero(&fc_db.ipv6_nat_mappingTbl[i].addr, sizeof(struct in6_addr));
		atomic_set(&fc_db.ipv6_nat_mappingTbl[i].refCount, 0);
	}

	for(i = 0 ; i < RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE; i ++)
	{
		bzero(&fc_db.ipv6_mapt_mappingTbl[i].addr, sizeof(struct in6_addr));
		atomic_set(&fc_db.ipv6_mapt_mappingTbl[i].refCount, 0);
	}
 
 
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
	{
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#else // support lspid_map table
{
#if 1
		fc_db.lspid_map_idx_decrypt_hwlookup = NITX_LSPID_MAP_ARM_IPSEC_DECRYPT_HWLOOKUP;
		fc_db.lspid_map_idx_pe_first_encrypt_hwlookup = NITX_LSPID_MAP_ARM_IPSEC_PE_FIRST_ENCRYPT_HWLOOKUP;
#else
		ca_status_t ret = CA_E_OK;
		aal_ni_dma_lso_lspid_map_tbl_cfg_t lspid_map_cfg;

		memset(&lspid_map_cfg, 0, sizeof(aal_ni_dma_lso_lspid_map_tbl_cfg_t));
		lspid_map_cfg.en_flag = 1;
		lspid_map_cfg.lspid = RTK_FC_IPSEC_HWLOOKUP_MACPORT;
		ret = aal_ni_dma_lso_add_lspid_map_tbl(&fc_db.lspid_map_idx_decrypt_hwlookup, &lspid_map_cfg);
		if(ret != CA_E_OK)
		{
			WARNING("[IPSEC] Fail to add lspid map for decrypt pkt ... Forcibly use lspid map index 0");
			fc_db.lspid_map_idx_decrypt_hwlookup = 0;
		}
		
		memset(&lspid_map_cfg, 0, sizeof(aal_ni_dma_lso_lspid_map_tbl_cfg_t));
		lspid_map_cfg.en_flag = 1;
		lspid_map_cfg.lspid = RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID;
		ret = aal_ni_dma_lso_add_lspid_map_tbl(&fc_db.lspid_map_idx_pe_first_encrypt_hwlookup, &lspid_map_cfg);
		if(ret != CA_E_OK)
		{
			WARNING("[IPSEC] Fail to add lspid map for pe first encrypt pkt ... Forcibly use lspid map index 0");
			fc_db.lspid_map_idx_pe_first_encrypt_hwlookup = 0;
		}
#endif
}
#endif
		for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i ++)
		{
			bzero(&fc_db.fc_ipsec_info[i], sizeof(rtk_fc_ipsec_xfrm_info_t));
			fc_db.fc_ipsec_info[i].key_index=0xff;
			fc_db.fc_ipsec_info[i].hash_key_index=0xff;
		
		}
		for(i = 0; i < RTK_FC_SPEC_HSAH_PADDING_MAX_NUM ; i++)
		{
			uint32 encrypt_data_len;

			encrypt_data_len = (i+1)*16;
			rtk_fc_mix_mode_calculate_hash_padding(1, encrypt_data_len, 8, i);
			rtk_fc_mix_mode_hmacMD5_calculate_hash_padding(1, encrypt_data_len, 8, i);
		}
		for(i = 0; i < RTK_FC_SPEC_HSAH_PADDING_MAX_NUM ; i++)
		{
			uint32 encrypt_data_len;

			encrypt_data_len = (i+1)*8;
			rtk_fc_des_mix_mode_calculate_hash_padding(1, encrypt_data_len, 8, i);
			rtk_fc_des_mix_mode_hmacMD5_calculate_hash_padding(1, encrypt_data_len, 8, i);
		}
		fc_db.ipsec_addr.ipsec_hash_padding_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 256, &fc_db.ipsec_addr.ipsec_hash_padding_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.ipsec_hash_padding_ptr, 0, 256);

		fc_db.ipsec_addr.ipsec_crypt_padding_ptr =dma_alloc_coherent(&(ni_info_data.pdev->dev), 96, &fc_db.ipsec_addr.ipsec_crypt_padding_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.ipsec_crypt_padding_ptr, 0, 96);

		fc_db.ipsec_addr.ipsec_icv_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 20, &fc_db.ipsec_addr.icv_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.ipsec_icv_ptr, 0, 20);

		fc_db.ipsec_addr.ipsec_icv_padding_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 20, &fc_db.ipsec_addr.ipsec_icv_padding_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.ipsec_icv_padding_ptr, 0, 20);

		fc_db.ipsec_addr.iv_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 16, &fc_db.ipsec_addr.iv_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.iv_ptr, 0, 16);

		fc_db.ipsec_addr.aad_padding_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 16, &fc_db.ipsec_addr.aad_padding_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.aad_padding_ptr, 0, 16);

		fc_db.ipsec_addr.aad_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 20, &fc_db.ipsec_addr.aad_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.aad_ptr, 0, 20);

		fc_db.ipsec_addr.cmd_setting_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), sizeof(rtk_fc_cryptoEngine_command_t), &fc_db.ipsec_addr.cmd_setting_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.cmd_setting_ptr, 0, sizeof(rtk_fc_cryptoEngine_command_t));

		fc_db.ipsec_addr.plain_text_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 2048, &fc_db.ipsec_addr.plain_text_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.plain_text_ptr, 0, 2048);

		fc_db.ipsec_addr.mix_mode_cmd_iv_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), sizeof(rtk_fc_cryptoEngine_command_t)+16, &fc_db.ipsec_addr.mix_mode_cmd_iv_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.mix_mode_cmd_iv_ptr, 0, sizeof(rtk_fc_cryptoEngine_command_t)+16);
		
		fc_db.ipsec_addr.key_ptr = dma_alloc_coherent(&(ni_info_data.pdev->dev), 48, &fc_db.ipsec_addr.key_dma, GFP_ATOMIC);
		memset(fc_db.ipsec_addr.key_ptr, 0, 48);
		ipsec_kernel_hook_func.ipsec_saInfo_add 				= _rtk_fc_ipsec_saInfo_add;
		ipsec_kernel_hook_func.ipsec_saInfo_delete				= _rtk_fc_ipsec_saInfo_delete;
		ipsec_kernel_hook_func.ipsec_saInfo_soft_delete 		= _rtk_fc_ipsec_saInfo_soft_delete;
		ipsec_kernel_hook_func.ipsec_callback_get_hwnatMode 	= _rtk_fc_ipsec_get_hwnatMode;
		ipsec_kernel_hook_func.ipsec_callback_shortCut_en 		= _rtk_fc_ipsec_get_shortCut_en;
		ipsec_kernel_hook_func.ipsec_callback_supported_mode 	= _rtk_fc_ipsec_get_supported_mode;
		ipsec_kernel_hook_func.ipsec_callback_table_ready		= _rtk_fc_ipsec_table_ready;

		for(i = 0; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i++)
		{
			INIT_LIST_HEAD(&fc_db.ipsec_swFlow_list[i]);
		}
		
	}
#endif
	for(i = 0; i < RTK_FC_PATH5_DS_FLOWLIST_BUCKET_MAX_NUM ; i++)
	{
		INIT_LIST_HEAD(&fc_db.path5_downStream_flowList[i]);
	}



	fc_db.controlFuc.pppoe_connectionAutoExtend = TRUE;
	fc_db.controlFuc.pppoe_connectionAutoExtend_timer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.controlFuc.pppoe_connectionAutoExtend_timer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.controlFuc.pppoe_connectionAutoExtend_timer);
	RTK_FC_HELPER_INIT_TIMER(fc_db.controlFuc.pppoe_connectionAutoExtend_timer, _rtk_fc_pppoeConnectionAutoExtend_task);
	RTK_FC_HELPER_SETUP_TIMER(fc_db.controlFuc.pppoe_connectionAutoExtend_timer, _rtk_fc_pppoeConnectionAutoExtend_task, (unsigned long)NULL);
	RTK_FC_HELPER_MOD_TIMER(fc_db.controlFuc.pppoe_connectionAutoExtend_timer, jiffies + (fc_db.controlFuc.pppoe_connectionAutoExtend_period_unit1s* CONFIG_HZ));
 
	for(i = 0 ; i < FLOWDELTRACE_MAX ; i++)
	{
		fc_db.flow_del_trace_index[i] = SIGNED_INVALID;
	}

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	for(i = 0 ; i < RTK_FC_TABLESIZE_DMAAFTACTION ; i++)
	{
		fc_db.dmaAftActionTbl[i].last_jiffies_when_deleted = jiffies;
	}	
#endif

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	for(i = 0; i < RTK_FC_CT_HASH_INFO_BUCKET_SIZE; i++)
	{
		INIT_LIST_HEAD(&fc_db.ct_hash_info_ListHead[i]);
	}
#endif	
	fc_db.controlFuc.house_keeping_flow_delete_immediatly = 1U;

	/* get init port status */
	for (i = 0; i< RTK_FC_MAC_PORT_MAX; i++){
		rtk_fc_portStatusInfo_t portInfo;

		if(fc_db.systemGlobal.phyPortStatus & 1ULL<<i)
		{
			bzero(&portInfo, sizeof(portInfo));
			rtk_fc_portStatus_get(i, &portInfo);
			atomic_set(&fc_db.portLinkupMask, atomic_read(&fc_db.portLinkupMask) | (portInfo.linkStatus<<i));
		}
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	/*QoS inetrnal-priority and queue mapping*/
	{
		rtk_qos_pri2queue_t rtk_pri2qid;
		rtk_qos_priSelWeight_t rtk_priSelWeight;

		//initialize priority selection group[0] (fc will disable flow user priority action)
		bzero(&rtk_priSelWeight, sizeof(rtk_priSelWeight));
		rtk_priSelWeight.weight_of_acl = 7U;
		rtk_priSelWeight.weight_of_portBased = 3U;
		if(rtk_qos_priSelGroup_set(0U, &rtk_priSelWeight))
			WARNING("[PRI SEL] init priority selection weight to group 0 failed!!!\n");

		for(i=0;i<RTK_FC_MAC_PORT_MAX;i++){
			if(RTK_QOS_PORTPRISELGROUP_SET(i, 0))
				WARNING("[PRI SEL] init per port priority select as group 0 failed!!!\n");
			if(RTK_QOS_PORTPRI_SET(i, 0))
				WARNING("[PRI SEL] reset port based priority as 0 failed!!!\n");

			//per port internal priority to QID mapping
			if(RTK_QOS_PORTPRIMAP_SET(i, 3))
				WARNING("init interPri and Queue Mapping failed!!!\n");
		}
		RTK_FC_HELPER_SET_EXT_PHY_QOS_PORTPRIMAP();

		//set group[3]: internal<=>queue mapping. rg only use group[3]
		rtk_pri2qid.pri2queue[0]=0U;
		rtk_pri2qid.pri2queue[1]=1U;
		rtk_pri2qid.pri2queue[2]=2U;
		rtk_pri2qid.pri2queue[3]=3U;
		rtk_pri2qid.pri2queue[4]=4U;
		rtk_pri2qid.pri2queue[5]=5U;
		rtk_pri2qid.pri2queue[6]=6U;
		rtk_pri2qid.pri2queue[7]=7U;
		if(rtk_qos_priMap_set(3U, &rtk_pri2qid))
			WARNING("init interPri and Queue Mapping failed!!!\n");
	}
	/*Set trap priority of unmatched flow and disable force trapping CPU priority*/
	/*If fc_db.controlFuc.prevent_control_packet_drop is enabled, set trap priority of unmatched flow to 3. (The packets will be received by ring 1)*/
	/*Otherwise set trap priority of unmatched flow to the default value 1. (The packets will be received by ring 0)*/
	if(fc_db.controlFuc.prevent_control_packet_drop == ENABLE)
		rtk_rg_asic_unmatchedCpuPriority_set(RTK_FC_UNMATCH_FLOW_TRAP_PRI);
	else
		rtk_rg_asic_unmatchedCpuPriority_set(RTK_FC_UNMATCH_FLOW_TRAP_PRI_DEFAULT);
	rtk_rg_asic_trapCpuPriority_set(DISABLE, 0);

	/*If fc_db.controlFuc.prevent_control_packet_drop is enabled, ENABLED CPU port flow, otherwise DISABLED.*/
	if(RTK_PORT_MACFORCEABILITY_GET(RTK_FC_MAC_PORT_MAINCPU,&cpuAbility))WARNING("Set CPU port flow control failed!!!\n");
	if(fc_db.controlFuc.prevent_control_packet_drop == ENABLE)
	{
		cpuAbility.txFc=ENABLED;
		cpuAbility.rxFc=ENABLED;
	}
	else
	{
		cpuAbility.txFc=DISABLED;
		cpuAbility.rxFc=DISABLED;
	}
	if(RTK_PORT_MACFORCEABILITY_SET(RTK_FC_MAC_PORT_MAINCPU,cpuAbility))WARNING("Set CPU port flow control failed!!!\n");
	if(RTK_PORT_MACFORCEABILITYSTATE_SET(RTK_FC_MAC_PORT_MAINCPU,ENABLED))WARNING("Set CPU port flow control failed!!!\n");

	if(RTK_PORT_MACFORCEABILITY_GET(RTK_FC_MAC_PORT_LASTCPU,&cpuAbility))WARNING("Set CPU port flow control failed!!!\n");
	if(fc_db.controlFuc.prevent_control_packet_drop == ENABLE)
	{
		cpuAbility.txFc=ENABLED;
		cpuAbility.rxFc=ENABLED;
	}
	else
	{
		cpuAbility.txFc=DISABLED;
		cpuAbility.rxFc=DISABLED;
	}
	if(RTK_PORT_MACFORCEABILITY_SET(RTK_FC_MAC_PORT_LASTCPU,cpuAbility))WARNING("Set CPU port flow control failed!!!\n");
	if(RTK_PORT_MACFORCEABILITYSTATE_SET(RTK_FC_MAC_PORT_LASTCPU,ENABLED))WARNING("Set CPU port flow control failed!!!\n");



	/*Enable ring 1 of GMAC9 flow control and disable other ones if fc_db.controlFuc.prevent_control_packet_drop is enabled. (disabled rings will not sent out pause frames)*/
	re8686_set_flow_control(0, 0, DISABLE);
	if(fc_db.controlFuc.prevent_control_packet_drop == ENABLE)
		re8686_set_flow_control(0, 1, ENABLE);
	else
		re8686_set_flow_control(0, 1, DISABLE);
	re8686_set_flow_control(0, 2, DISABLE);
	re8686_set_flow_control(0, 3, DISABLE);
	re8686_set_flow_control(0, 4, DISABLE);
	re8686_set_flow_control(0, 5, DISABLE);
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	if(CHIP_ID_9607C) {
		re8686_set_flow_control(1, 0, DISABLE);
		re8686_set_flow_control(1, 1, DISABLE);
		re8686_set_flow_control(1, 2, DISABLE);
		re8686_set_flow_control(1, 3, DISABLE);
		re8686_set_flow_control(1, 4, DISABLE);
		re8686_set_flow_control(1, 5, DISABLE);
#ifdef CONFIG_GMAC2_USABLE
		re8686_set_flow_control(2, 0, DISABLE);
		re8686_set_flow_control(2, 1, DISABLE);
		re8686_set_flow_control(2, 2, DISABLE);
		re8686_set_flow_control(2, 3, DISABLE);
		re8686_set_flow_control(2, 4, DISABLE);
		re8686_set_flow_control(2, 5, DISABLE);
#endif
	}
#endif

	/*If fc_db.controlFuc.prevent_control_packet_drop is enabled, turn on NIC suspend mechanism*/
	if(fc_db.controlFuc.prevent_control_packet_drop == ENABLE)
		re8686_set_pauseBySw(ENABLE);
	else
		re8686_set_pauseBySw(DISABLE);

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	// none

#if defined(CONFIG_FC_RTL8198F_SERIES)
	atomic_set(&fc_db.portLinkupMask, atomic_read(&fc_db.portLinkupMask) | RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU);
#else
	atomic_set(&fc_db.portLinkupMask, atomic_read(&fc_db.portLinkupMask) | (1<<RTK_FC_MAC_PORT_PON));
#endif
#endif

	rtk_fc_rt_mapper_api_init();

	// ACL
	rtk_fc_aclInit();
	


#if defined(CONFIG_FC_RTL9607C_SERIES) 
		memset(&fc_db_fastFwd_data, 0, sizeof(rtk_fc_special_fastFwd_data_t));
		if(fc_db.controlFuc.special_fast_forward_mode==1U)
		{
			_rtk_fc_vxlan_acceleration_mechanism_set(1);
			_rtk_fc_nptv6_acceleration_mechanism_set(1);
		}
	
		fc_db.vxlan_accelerate_upstreamL2Idx 						= -1;
		fc_db.vxlan_accelerate_extra_upstreamL2Idx				 	= -1;
		fc_db.vxlan_accelerate_downstreamL2Idx 						= -1;
		fc_db.vxlan_accelerate_extra_downstreamL2Idx 				= -1;
		fc_db.vxlan_rt_meter_index 									= -1;
		fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter 	= -1;

		fc_db.dynamic_calculate_prehash_timeout_sec = 60U;
		//RTK_FC_HELPER_SPIN_LOCK_INIT(&fc_db.dynamic_lock);

#endif

	{
		int j;
		for(i=0;i<FC_SMP_JOBS_TYPE_MAX;i++)
			for(j=0;j<NR_CPUS;j++)
				atomic_set(&fc_db.smp_statistic[i][j], 0);
	}
	RTK_FC_HELPER_MGR_SYSTEM_POST_INIT();
	RTK_FC_HELPER_WLAN_SYSTEM_POST_INIT();

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	for(i = 0 ; i < RT_RATE_HOSTPOLICING_TABLE_SIZE ; i++)
	{
		fc_db.hostPoliceTable[i].loggingRx_policerIdx = FAIL;
		fc_db.hostPoliceTable[i].loggingTx_policerIdx = FAIL;
		fc_db.hostPoliceTable[i].l2ClsIndex.idx_ingress= FAIL;
		fc_db.hostPoliceTable[i].l2ClsIndex.idx_egress= FAIL;
	}
#if defined(CONFIG_FC_CAG3_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)
	fc_db.controlFuc.flow_meter_mib_conf_dependence = ENABLED; // the mib configuration depends on meter configuration for every flow
#else
	fc_db.controlFuc.flow_meter_mib_conf_dependence = DISABLED; // the mib configuration depends on meter configuration for every flow
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	fc_db.controlFuc.hw_l3l4_error_pkt_check = ENABLED; // ASIC init default enable hw_l3l4_error_pkt_check
#endif
#endif

	for(i=0; i<RT_RATE_SW_RATE_LIMIT_TYPE_MAX; i++)
	{
		fc_db.swRateLimit[i].rateLimit_conf.shareMeterIdx = FAIL;
	}

	//initial pure SW meter setting
	for(i = 0 ; i < RTK_FC_TABLESIZE_SW_SHAREMTR ; i++)
	{
		fc_db.swMeter[i].rate = METER_RATE_MAX;
		fc_db.swMeter[i].ifgInclude = DISABLED;
		fc_db.swMeter[i].lastJiffies = jiffies;
		fc_db.swMeter[i].lastJiffiesPersec = jiffies;
		memset(&fc_db.swMeter[i].meterCount, 0, sizeof(fc_db.swMeter[i].meterCount));
		memset(&fc_db.swMeter[i].meterCountPersec, 0, sizeof(fc_db.swMeter[i].meterCountPersec));
	}

	//initial l34 meter setting
	for(i = 0 ; i < RTK_FC_TABLESIZE_FBMTR; i++)
	{
		fc_db.l34Meter[i].rate = METER_RATE_MAX;
		fc_db.l34Meter[i].ifgInclude = DISABLED;
		fc_db.l34Meter[i].lastJiffies = jiffies;
		fc_db.l34Meter[i].lastJiffiesPersec = jiffies;
	}

	// init sw shaping control
	for(i = 0; i< RTK_FC_TABLESIZE_SW_SHAPING; i++){
		fc_db.shapingCtrl[i].kicktxtimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
		if(RTK_FC_HELPER_TIMER_PENDING(fc_db.shapingCtrl[i].kicktxtimer))
			RTK_FC_HELPER_DEL_TIMER(fc_db.shapingCtrl[i].kicktxtimer);
		RTK_FC_HELPER_INIT_TIMER(fc_db.shapingCtrl[i].kicktxtimer, rtk_fc_sw_shaper_kicktx_timerfunc);
		RTK_FC_HELPER_SETUP_TIMER(fc_db.shapingCtrl[i].kicktxtimer,rtk_fc_sw_shaper_kicktx_timerfunc,(unsigned long)i);
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	fc_db.controlFuc.flow_miss_meter_idx = SIGNED_INVALID;
#endif

#if defined(CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH) && (CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH > 5)
	// Init sw ackDelay list
	{
		// Init sw ackDelay free list head
		INIT_LIST_HEAD(&fc_db.ackDelay_freeListHead);

		// Init sw ackDelay hash list head
		if(fc_db.ackDelay_hashListHead) RTK_FC_HELPER_FC_KFREE(fc_db.ackDelay_hashListHead,sizeof(struct list_head) * RTK_FC_ACKDELAY_HASH_SIZE);
		fc_db.ackDelay_hashListHead = RTK_FC_HELPER_FC_KMALLOC(sizeof(struct list_head) * RTK_FC_ACKDELAY_HASH_SIZE, GFP_ATOMIC);
		for(i=0;i<RTK_FC_ACKDELAY_HASH_SIZE;i++)
			INIT_LIST_HEAD(&fc_db.ackDelay_hashListHead[i]);

		// Init sw ackDelay list
		if(fc_db.ackDelayList) RTK_FC_HELPER_FC_KFREE(fc_db.ackDelayList,sizeof(rtk_fc_ackDelay_linkList_t) * RTK_FC_ACKDELAY_ENTRY_SIZE);
		fc_db.ackDelayList = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_ackDelay_linkList_t) * RTK_FC_ACKDELAY_ENTRY_SIZE, GFP_ATOMIC);
		for(i=0; i<RTK_FC_ACKDELAY_ENTRY_SIZE; i++) {
			fc_db.ackDelayList[i].list_idx = i;
			INIT_LIST_HEAD(&fc_db.ackDelayList[i].entry_list);

			//add free node to free list head
			list_add_tail(&fc_db.ackDelayList[i].entry_list, &fc_db.ackDelay_freeListHead);

			//timer per entry
			fc_db.ackDelayList[i].kicktxTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
			if(RTK_FC_HELPER_TIMER_PENDING(fc_db.ackDelayList[i].kicktxTimer))
				RTK_FC_HELPER_DEL_TIMER(fc_db.ackDelayList[i].kicktxTimer);
			RTK_FC_HELPER_INIT_TIMER(fc_db.ackDelayList[i].kicktxTimer, rtk_fc_sw_ackDelay_kicktx_timerfunc);
			RTK_FC_HELPER_SETUP_TIMER(fc_db.ackDelayList[i].kicktxTimer,rtk_fc_sw_ackDelay_kicktx_timerfunc,(unsigned long)i);
		}
	}
#endif

	// initial mac learning limit
	for(i=0; i<RTK_FC_MAC_PORT_MAX; i++)
	{
		fc_db.macAddrLearningLimit[i].learningLimit_conf.learningLimitNumber = SIGNED_INVALID;
		atomic_set(&fc_db.macAddrLearningLimit[i].learningCount, 0);
	}
	for(i=0; i<RTK_FC_WLANX_END_INTF; i++)
	{
		fc_db.wlanMacAddrLearningLimit[i].learningLimit_conf.learningLimitNumber = SIGNED_INVALID;
		atomic_set(&fc_db.wlanMacAddrLearningLimit[i].learningCount, 0);
	}
	fc_db.macAddr_portGroup.limitCfg.learningLimit_conf.learningLimitNumber = SIGNED_INVALID;

	//initial wan access limit structure
	fc_db.wanAccessLimit.accessLimitNumber = FAIL;
	atomic_set(&fc_db.wanAccessLimit.learningCount, 0);
	fc_db.wanAccessLimit.neighbor_probe_timer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	for(i=0; i<RTK_FC_WAN_ACCESS_IP_BUCKET_SIZE; i++)
		INIT_LIST_HEAD(&fc_db.wanAccessLimitIP_head[i]);

	fc_db.rtk_fc_init = 1;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	fc_db.controlFuc.wifi_flow_ctrl_detect_timer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
#endif
	fc_db.controlFuc.hwnat_mode = RT_FLOW_HWNAT_MODE_ORIGINAL; //0: disable acceleration, 1(default): enable acceleration, 2: disable hw acc(sw only), 3: disable sw acc (hw only)
	RTK_FC_HELPER_MGR_HWNAT_MODE_SET(RT_FLOW_HWNAT_MODE_ORIGINAL);

#if defined(CONFIG_FC_RTL8198F_SERIES)
	fc_db.controlFuc.pe_fc_enable = ca_ni_get_pe_fc_enable();
	if (fc_db.controlFuc.pe_fc_enable)
		fc_db.controlFuc.pe_port = ca_ni_get_pe_port();
	printk("fc_db.controlFuc.pe_fc_enable=%d fc_db.controlFuc.pe_port=%d\n",
		fc_db.controlFuc.pe_fc_enable, fc_db.controlFuc.pe_port);
#endif

	fc_db.controlFuc.flow_tcp_learning_state = TCP_LEARN_SYN_RECV;

	fc_db.controlFuc.ip_frag_shortcut = 1U;

#if defined(FC_F_SHORTCUT_EARLYCHECK)
	fc_db.controlFuc.shortcut_earlycheck_en = 2;
#endif

#if defined(CONFIG_RTK_FC_DRV_EVENT_RECORD_VERSION) && (CONFIG_RTK_FC_DRV_EVENT_RECORD_VERSION > 0)
	fc_db.controlFuc.event_rec_ctrl = 1;
#endif

	fc_db.controlFuc.wifi_tx_qos_enable = TRUE;
	for(i = 0 ; i < 8 ; i++)
		fc_db.controlFuc.wifi_tx_qos_mapping[i] = i;

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	{
		fc_db.wifi_amsdu_pe_offload_mode = RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP64;
		_rtk_fc_wifi_amsdu_pe_offload_init(TRUE);
		fc_db.controlFuc.mc2uc_en=1;
	}
#endif

	//========================= Critical Section End =========================//

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	
#if 0 //defined(CONFIG_RTK_FC_IPSEC_FASTFWD) // move to use proc
	{
		__rtk_fc_ipsec_init();
	}
#endif

	/* House keeping timer */
	rtkFlowbaseHwnatHouseKeepingTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	if(RTK_FC_HELPER_TIMER_PENDING(rtkFlowbaseHwnatHouseKeepingTimer))
		RTK_FC_HELPER_DEL_TIMER(rtkFlowbaseHwnatHouseKeepingTimer);
	RTK_FC_HELPER_INIT_TIMER(rtkFlowbaseHwnatHouseKeepingTimer, rtk_fc_houseKeepingTimerFunc);
	RTK_FC_HELPER_SETUP_TIMER(rtkFlowbaseHwnatHouseKeepingTimer,rtk_fc_houseKeepingTimerFunc,(unsigned long)NULL);
	RTK_FC_HELPER_MOD_TIMER(rtkFlowbaseHwnatHouseKeepingTimer, jiffies+(RTK_FC_TIMER_TICK_SECOND*CONFIG_HZ));

	rtk_fc_rtnetlink_register_notifier();

	/*rtnetlink time init*/
	fc_db.rtnlJobs.rtnetlinkEventTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.rtnlJobs.rtnetlinkEventTimer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.rtnlJobs.rtnetlinkEventTimer);
	RTK_FC_HELPER_INIT_TIMER(fc_db.rtnlJobs.rtnetlinkEventTimer, rtk_fc_rtnetlink_timerFunc);
	RTK_FC_HELPER_SETUP_TIMER(fc_db.rtnlJobs.rtnetlinkEventTimer,rtk_fc_rtnetlink_timerFunc,(unsigned long)NULL);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	/*Dynamic re-calculate prehash pattern Timer*/
	fc_db.dynamic_prehashPtn_eventTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.dynamic_prehashPtn_eventTimer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.dynamic_prehashPtn_eventTimer);
	RTK_FC_HELPER_INIT_TIMER(fc_db.dynamic_prehashPtn_eventTimer, rtk_fc_dynamicPrehashPtn_timerFunc);
	RTK_FC_HELPER_SETUP_TIMER(fc_db.dynamic_prehashPtn_eventTimer,rtk_fc_dynamicPrehashPtn_timerFunc, (unsigned long)NULL);
#endif


	/* register Link change handler */
	intr_bcaster_notifier_cb_register(&linkChangeNotifier);

	/* Register notifier */
	register_inetaddr_notifier(&rtk_fc_netif_inetaddr_notifier);
	register_inet6addr_notifier(&rtk_fc_netif_inet6addr_notifier);
	register_netdevice_notifier(&rtk_fc_netif_netdev_notifier);

#if defined(CONFIG_RTK_L34_G3_PLATFORM) && defined(CONFIG_RG_G3_WAN_PORT_INDEX)
	// change dev-port mapping
	if(CONFIG_RG_G3_WAN_PORT_INDEX != AAL_PPORT_NI7)
		rtk_fc_user_pipe_cmd("echo %d nas0 > proc/driver/cortina/ni/dev_port_mapping", CONFIG_RG_G3_WAN_PORT_INDEX);
#endif

	MODINITPRINT("RTK FleetConntrack Driver - core module init\n");
	MODINITPRINT(" - version%d: %s (%s) %s\n", FC_INTERNAL_VERSION, FC_GIT_VERSION, FC_GIT_SHA1, FC_GIT_SHA1_DATE);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	MODINITPRINT(" - mem usage %d KB (malloc:%d  fc_db:%d pro_db:%d)\n", (fc_db.memUsage+sizeof(fc_db)+sizeof(rgpro_db))/1024, fc_db.memUsage,  sizeof(fc_db), sizeof(rgpro_db));
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	MODINITPRINT(" - mem usage %d KB (malloc:%d  fc_db:%d)\n", (int)(fc_db.memUsage+sizeof(fc_db))/1024, fc_db.memUsage,  (int)sizeof(fc_db));
#endif

#ifdef CONFIG_FC_RTL8198F_SERIES
	rtk_ne_reg_write(0, NI_HV_GLB_INTF_RST_CONFIG);
#endif

	for(i=0; i<RT_PE_FUNC_NUM_MAX; i++)
		fc_db.controlFuc.func_on_pe_cpu_num[i] = 1; //all pe functions run on pe1 by default
#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
{
	ca_status_t ret;

	//http
	ret = ca_ipc_msg_handle_register(CA_IPC_SESSION_HTTP_TEST, http_test_ipc_msg, RT_PE_HTTP_TEST_RETURN_CMD_MAX);
	if(ret==CA_E_OK || ret==CA_E_EXISTS)
		MODINITPRINT("[Http test] Success to register ipc msg handler \n");
	else
	{
		WARNING("[Http test] Fail to register ipc msg handler ... ret=%d\n", ret);
	}

	//set dma aft to invalid
	fc_db.http_test_dma_aft_idx = FAIL;

	for(i=0; i<MAX_PE_HTTP_CONNECTION_NUM; i++)
		fc_db.http_test_cls_idx[i] = FAIL;

	//set dma lso backpressure to invalid
	fc_db.http_test_dma_lso_regValue[0] = FAIL;
	//set dma lso shaper to invalid
	fc_db.http_test_dma_lso_regValue[4] = FAIL;
	
	memset(&fc_db.pe_http_pageInfo, 0, sizeof(rtk_fc_page_t));
	fc_db.pe_http_pageInfo.order = RT_PE_HTTP_TEST_PAGE_ORDER;
}
#endif
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
{
	ca_status_t ret;
	
	//crypto
	ret = ca_ipc_msg_handle_register(CA_IPC_SESSION_CRYPTO_ENGINE, crypto_ipc_msg, RT_PE_CRYPTO_ENGINE_RETURN_CMD_MAX);
	if(ret==CA_E_OK || ret==CA_E_EXISTS)
		MODINITPRINT("[Crypto] Success to register ipc msg handler \n");
	else
	{
		WARNING("[Crypto] Fail to register ipc msg handler ... ret=%d\n", ret);
	}
	for(i=0; i<MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM; i++)
		fc_db.pe_encrypt_info[i].fc_saInfo_idx = FAIL;
	for(i=0; i<MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM; i++)
		fc_db.pe_decrypt_info[i].fc_saInfo_idx = FAIL;

	memset(&fc_db.pe_crypto_pageInfo, 0, sizeof(rtk_fc_page_t));
	fc_db.pe_crypto_pageInfo.order = RT_PE_IPSEC_PAGE_ORDER;
	//init ipsec ps api
	rtk_fc_internal_init_ipsec_ps_api();
}
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	//allocate memory for pe amsdu ofld
#ifndef CONFIG_RTK_FC_WIFI_DRIVER_OFFLOAD_BY_PE
	_rtk_fc_wifi_amsdu_pe_offload_page_alloc();
#endif
	{
		ca_status_t ret;
		ca_uint16_t msg_count = 0;

		msg_count = sizeof(rtk_wfo_msg) / sizeof(rtk_wfo_msg[0]);
		
		ret = ca_ipc_msg_handle_register(CA_IPC_SESSION_WFO, rtk_wfo_msg, msg_count);
		if(ret==CA_E_OK || ret==CA_E_EXISTS)
			MODINITPRINT("[rtk wfo] Success to register ipc msg handler \n");
		else
		{
			WARNING("[rtk wfo] Fail to register ipc msg handler ... ret=%d\n", ret);
		}

		fc_db.pe_offload_wifi_amsdu_en = 1;
	}
#endif
	return 0;
}

int rtk_fc_timer_list_exit(void)
{
	int i;

	for(i = 0; i < RTK_FC_TABLESIZE_SW_SHAPING; i++)
	{
		if(fc_db.shapingCtrl[i].kicktxtimer)
		{
			RTK_FC_HELPER_MGR_TIMER_LIST_KFREE(fc_db.shapingCtrl[i].kicktxtimer);
			fc_db.shapingCtrl[i].kicktxtimer = NULL;
		}
	}
	if(fc_db.rtnlJobs.rtnetlinkEventTimer)
	{
		RTK_FC_HELPER_MGR_TIMER_LIST_KFREE(fc_db.rtnlJobs.rtnetlinkEventTimer);
		fc_db.rtnlJobs.rtnetlinkEventTimer = NULL;
	}
	if(fc_db.wanAccessLimit.neighbor_probe_timer)
	{
		RTK_FC_HELPER_MGR_TIMER_LIST_KFREE(fc_db.wanAccessLimit.neighbor_probe_timer);
		fc_db.wanAccessLimit.neighbor_probe_timer = NULL;
	}
	if(fc_db.controlFuc.pppoe_connectionAutoExtend_timer)
	{
		RTK_FC_HELPER_MGR_TIMER_LIST_KFREE(fc_db.controlFuc.pppoe_connectionAutoExtend_timer);
		fc_db.controlFuc.pppoe_connectionAutoExtend_timer = NULL;
	}
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(fc_db.controlFuc.wifi_flow_ctrl_detect_timer)
	{
		RTK_FC_HELPER_MGR_TIMER_LIST_KFREE(fc_db.controlFuc.wifi_flow_ctrl_detect_timer);
		fc_db.controlFuc.wifi_flow_ctrl_detect_timer = NULL;
	}
#endif

	if(fc_db.igmpDummyPktDetectorTimer.igmpTimer)
		RTK_FC_HELPER_FC_KFREE(fc_db.igmpDummyPktDetectorTimer.igmpTimer, sizeof(struct timer_list));

	if(fc_db.igmpKernelSyncTimerEvent.igmpTimer)
		RTK_FC_HELPER_FC_KFREE(fc_db.igmpKernelSyncTimerEvent.igmpTimer, sizeof(struct timer_list));
	if(fc_db.igmpKernelSyncTimerPeriod.igmpTimer)
		RTK_FC_HELPER_FC_KFREE(fc_db.igmpKernelSyncTimerPeriod.igmpTimer, sizeof(struct timer_list));

	return 0;
}

__init int rtk_fc_core_init(void)
{
	memset(&fc_db,0,sizeof(rtk_fc_globalDatabase_t));
	

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	// get chip id and have correct port definition.
	rtk_rg_asic_fb_init();
	_rtk_fc_sharing_image_port_define();
#endif

	// all init are moved to rtk_fc_param_init() for waiting manager module helper function ready.

	return 0;
}


__exit
void rtk_fc_core_exit(void)
{
	printk("Enter %s \n",__func__);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	//========================= Critical Section Start =========================//
	
	if(RTK_FC_HELPER_TIMER_PENDING(rtkFlowbaseHwnatHouseKeepingTimer))
		RTK_FC_HELPER_DEL_TIMER(rtkFlowbaseHwnatHouseKeepingTimer);

	
	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.rtnlJobs.rtnetlinkEventTimer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.rtnlJobs.rtnetlinkEventTimer);

	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.controlFuc.pppoe_connectionAutoExtend_timer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.controlFuc.pppoe_connectionAutoExtend_timer);


#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.controlFuc.wifi_flow_ctrl_detect_timer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.controlFuc.wifi_flow_ctrl_detect_timer);
#endif
	rtk_fc_timer_list_exit();

#if 0 // cheney
	/*unregister NIC rx handler*/
	if(drv_nic_unregister_rxhook(RTK_FC_ALL_MAC_PORTMASK, RE8686_RXPRI_RG, rtk_fc_ingress_flowLearning))
		WARNING("FleetConntrack driver was fail to unreigster nic Rx funciton!");

#if defined(CONFIG_RTK_L34_G3_PLATFORM) && defined(CONFIG_RTK_NIC_TX_HOOK)
	if(nic_txhook_exit() != SUCCESS)
		WARNING("FleetConntrack driver was fail to unreigster nic Tx funciton!");
#endif
#endif

	/*unregister Wifi rx handler*/
	// TODO:

	if(RTK_FC_API_MODULE->rtk_fc_api_exit)
		RTK_FC_API_MODULE->rtk_fc_api_exit();

	/* unregister Link change handler */
	intr_bcaster_notifier_cb_unregister(&linkChangeNotifier);

	rtk_fc_proc_exit();

	rtk_fc_rtnetlink_unregister_notifier();

	//========================= Critical Section End =========================//

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();


	unregister_inetaddr_notifier(&rtk_fc_netif_inetaddr_notifier);
	unregister_inet6addr_notifier(&rtk_fc_netif_inet6addr_notifier);
	unregister_netdevice_notifier(&rtk_fc_netif_netdev_notifier);

	return;
}

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_set(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr)
{
	uint i;
	int ret = RTK_FC_RET_OK;
	rtk_fc_wifi_amsdu_pe_offload_sta_info_t new_sta_info = {0};
	rtk_fc_wifi_amsdu_pe_offload_sta_info_update_mask_t sta_info_update_msk = 0;
	bool if_macAddr_exist = FALSE;
	bool if_update_staInfo_only = FALSE;
	rtk_fc_wlan_devidx_t wlan_dev_idx = RTK_FC_WLANX_NOT_FOUND;

	if((mac_id < RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START) || (mac_id > fc_db.wifi_amsdu_pe_offload_sta_count))
	{
		WARNING("Invalid mac_id %u, it should be %u to %u", mac_id, RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START, fc_db.wifi_amsdu_pe_offload_sta_count);
		return RTK_FC_RET_ERR_INVALID_PARAM;
	}
	if(!mac_addr)
	{
		WARNING("NULL mac_addr");
		return RTK_FC_RET_ERR_NULL_POINTER;
	}

	if(sta_conf_sel.p_wifi_dev)
	{
		RTK_FC_HELPER_WLAN_DEV_TO_DEVID(sta_conf.p_wifi_dev, &wlan_dev_idx);

		if(wlan_dev_idx == RTK_FC_WLANX_NOT_FOUND)
		{
			WARNING("Get wlan_dev_idx by p_wifi_dev %p failed, set mac_id without p_wifi_dev info", sta_conf.p_wifi_dev);
			sta_conf_sel.p_wifi_dev = FALSE;
		}
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();

	// if duplicate
	if(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].valid)
	{
		if(memcmp(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr, mac_addr, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr)))
		{
			/* mac_id is already in use by other STA */
			WARNING("Entry exists: mac_id[%u]: %pM", mac_id, fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr);
			ret =  RTK_FC_RET_ERR_ENTRY_EXIST;
			goto UNLOCK_WITHOUT_UPDATE;
		}
		else
		{
			/* mac_id and mac_address mapping no change */
			if_macAddr_exist = TRUE;

			//get original sat_info
			memcpy(&new_sta_info, &fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info, sizeof(new_sta_info));
			//update to new sat_info
			{
				if((sta_conf_sel.power_saving) && (sta_conf.power_saving != fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info.power_saving))
				{
					sta_info_update_msk |= WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_POWER_SAVING;
					new_sta_info.power_saving = sta_conf.power_saving;
				}
				if((sta_conf_sel.p_wifi_dev) && (sta_conf.p_wifi_dev != fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info.p_wifi_dev))
				{
					sta_info_update_msk |= WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_P_WLAN_DEV;
					new_sta_info.p_wifi_dev = sta_conf.p_wifi_dev;
				}
#if defined(CONFIG_REALTEK_IPC2RCPU)
				if((sta_conf_sel.amsdu_pkt_size) && (sta_conf.amsdu_pkt_size != fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info.amsdu_pkt_size))
				{
					sta_info_update_msk |= WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_AMSDU_PKT_SIZE;
					new_sta_info.amsdu_pkt_size = sta_conf.amsdu_pkt_size;
				}
#endif
			}

			if(sta_info_update_msk)
			{
				if_update_staInfo_only = TRUE;
				goto UNLOCK_WITH_STA_INFO_UPDATE; // update sta_info
			}
			else
				goto UNLOCK_WITHOUT_UPDATE; //no change
		}
	}

	for(i = RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START ; i < fc_db.wifi_amsdu_pe_offload_sta_count ; i++)
	{
		if((i == mac_id) || (!fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].valid))
			continue;

		if(!memcmp(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].l2Addr, mac_addr, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].l2Addr)))
		{
			/* mac address is mapping to other mac_id, clear this entry */
			if_macAddr_exist = TRUE;

			//get original sat_info first
			memcpy(&new_sta_info, &fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].sta_info, sizeof(new_sta_info));
			fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].valid = 0;
			memset(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].l2Addr, 0, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].l2Addr));
			memset(&fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].sta_info, 0, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].sta_info));
			break;
		}
	}

	/*update to new sat_info*/
	{
#if defined(CONFIG_REALTEK_IPC2RCPU)
		// new add mac address entry, set to default value
		if(if_macAddr_exist == FALSE)
			new_sta_info.amsdu_pkt_size = RTK_WIFI_DFT_AMSDU_PKT_SIZE;
#endif
		// update if value is specified
		{
			if(sta_conf_sel.power_saving)
				new_sta_info.power_saving = sta_conf.power_saving;

			if(sta_conf_sel.p_wifi_dev)
			{
				new_sta_info.p_wifi_dev = sta_conf.p_wifi_dev;
				sta_info_update_msk |= WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_P_WLAN_DEV;// need to inform PE
			}
#if defined(CONFIG_REALTEK_IPC2RCPU)
			if(sta_conf_sel.amsdu_pkt_size)
				new_sta_info.amsdu_pkt_size = sta_conf.amsdu_pkt_size;

			sta_info_update_msk |= WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_AMSDU_PKT_SIZE; // need to inform PE, always update (default value)
#endif
		}
	}

	// Set entry
	fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].valid = TRUE;
	memcpy(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr, mac_addr, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr));

UNLOCK_WITH_STA_INFO_UPDATE:
	memcpy(&fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info, &new_sta_info, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].sta_info));
	if(if_update_staInfo_only)
		_rtk_fc_lutStaInfoUpdate(mac_addr, mac_id, wlan_dev_idx, sta_info_update_msk); // update sta_info only, no need to flush related flow entry
	else
	{
		_rtk_fc_lutStaInfoUpdate(mac_addr, mac_id, wlan_dev_idx, sta_info_update_msk); // update WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_AMSDU_PKT_SIZE
		_rtk_fc_lutMacIdIdxUpdate(mac_addr); // update mac address and mac_id mapping
	}

UNLOCK_WITHOUT_UPDATE:
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();

	return ret;
}

int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_del(unsigned int mac_id)
{
	unsigned char ori_l2Addr[ETH_ALEN];

	if((mac_id < RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START) || (mac_id > RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END))
	{
		WARNING("Invalid mac_id %u, it should be %u to %u", mac_id, RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START, RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_END);
		return RTK_FC_RET_ERR_INVALID_PARAM;
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	if(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].valid)
	{
		memcpy(ori_l2Addr, fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr, sizeof(ori_l2Addr));
		// clear entry
		fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].valid = FALSE;
		memset(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr, 0, sizeof(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[mac_id].l2Addr));
		_rtk_fc_lutMacIdIdxUpdate(ori_l2Addr);
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	return RTK_FC_RET_OK;
}

int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flush(void)
{
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	_rtk_fc_wifi_amsdu_pe_offload_init(FALSE);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	return RTK_FC_RET_OK;
}

int rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flow_flush(void) {
	
	uint i;
	int16 lutIdx;
	
	for(i = RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_RANGE_START ; i < RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_SIZE ; i++)
	{
		if(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].valid)
		{
			_rtk_fc_lut_find(fc_db.wifi_amsdu_pe_offload_mac_id_tbl[i].l2Addr, &lutIdx);
			rtk_fc_flow_delete_by_l2Idx(lutIdx);
		}
	}
	return RTK_FC_RET_OK;
}

extern void ca_ni_wait_tx_amsdu_pe_ofld_done(void);
int rtk_fc_internal_wifi_amsdu_pe_offload_en(rtk_pe_amsdu_mode_t mode, int pe_offload_wifi_amsdu_en) {
	
	int pre_pe_offload_wifi_amsdu_en = fc_db.pe_offload_wifi_amsdu_en;

	//protect
	if (pe_offload_wifi_amsdu_en != 0 && pe_offload_wifi_amsdu_en != 1) {
		//input not 0/1 error
		return RTK_FC_RET_ERR;
	}

	if (pre_pe_offload_wifi_amsdu_en == pe_offload_wifi_amsdu_en) {
		//status no change
		return RTK_FC_RET_OK;
	}

	//enable / disable learning
	fc_db.pe_offload_wifi_amsdu_en = pe_offload_wifi_amsdu_en;

	//1 -> 0 clear mac id flows
	//if (pre_pe_offload_wifi_amsdu_en == 1 && fc_db.pe_offload_wifi_amsdu_en == 0) {
		//clear mac id flows
		//rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flow_flush();
	//}
	/* 0 -> 1 clear mac id flows from normal wifi path
	 * 1 -> 0 clear mac id flows from pe offload wifi path */
	rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flow_flush();

	//process mode
	if (mode == RTK_PE_AMSDU_MODE_FUNC_SWITCHING) {
		//1 -> 0 clear mac id flows
		if (pre_pe_offload_wifi_amsdu_en == 1 && fc_db.pe_offload_wifi_amsdu_en == 0) {
			//rx all pkts from voq and rx ring
			ca_ni_wait_tx_amsdu_pe_ofld_done();
		}
	} 

    return RTK_FC_RET_OK;
}

int rtk_fc_internal_wifi_amsdu_pe_offload_mode_set(rtk_fc_wifi_amsdu_pe_offload_mode_t mode)
{
	if(mode >= RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_END)
		return RTK_FC_RET_ERR_INVALID_PARAM;

	if(mode != fc_db.wifi_amsdu_pe_offload_mode)
	{
		//mode change
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();

		fc_db.wifi_amsdu_pe_offload_mode = mode;
		_rtk_fc_wifi_amsdu_pe_offload_init(TRUE);

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	}
	return RTK_FC_RET_OK;
}

int rtk_fc_internal_wifi_amsdu_pe_offload_mode_get(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode)
{
	*mode = fc_db.wifi_amsdu_pe_offload_mode;
	return RTK_FC_RET_OK;
}

int rtk_fc_internal_wifi_dev_attr_set(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_wifi_dev_attr_t ori_attr, rtk_fc_wifi_dev_attr_t new_attr)
{	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();

	rtk_fc_flow_delete_by_wlanIdx(wlanDevIdx);
	
	if(ori_attr.client_mode == FALSE && new_attr.client_mode == TRUE) {
		fc_db.wanPortWifidevMask |= (1ULL << wlanDevIdx);
	}
	if(ori_attr.client_mode == TRUE && new_attr.client_mode == FALSE) {
		fc_db.wanPortWifidevMask &= ~(1ULL << wlanDevIdx);
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
		
	return RTK_FC_RET_OK;
}

int rtk_fc_internal_amsdu_pe_offload_mc2uc(bool enable)
{	
	rtk_fc_multicast_flush_patten patten={0};
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();

	if(fc_db.controlFuc.mc2uc_en != enable)
	{
		rtk_fc_multicast_flowFlush_strategy(MC_FLWO_FLUSH_BY_DMAC_TRANS_ACTION,&patten);
	}
	fc_db.controlFuc.mc2uc_en=enable;
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
		
	return RTK_FC_RET_OK;
}


int rtk_fc_internal_wifi_client_mode_cb_register(rtk_fc_wifi_callback pfunc)
{
	return RTK_FC_RET_OK;
}

EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_set);
EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_del);
EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flush);
EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flow_flush);
EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_en);
EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_mode_set);
EXPORT_SYMBOL(rtk_fc_internal_wifi_amsdu_pe_offload_mode_get);
EXPORT_SYMBOL(rtk_fc_internal_wifi_dev_attr_set);
EXPORT_SYMBOL(rtk_fc_internal_amsdu_pe_offload_mc2uc);

#endif

module_init(rtk_fc_core_init);
module_exit(rtk_fc_core_exit);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
EXPORT_SYMBOL(rtk_fc_mapet_hwfmr_add);
EXPORT_SYMBOL(rtk_fc_mapet_hwfmr_del);
#endif
EXPORT_SYMBOL(rtk_fc_helper_register);
EXPORT_SYMBOL(rtk_fc_helper_unregister);
EXPORT_SYMBOL(rtk_fc_ingress_flowLearning);
EXPORT_SYMBOL(rtk_fc_egress_flowLearning);
EXPORT_SYMBOL(rtk_fc_fastfwd_egress_xmit);
EXPORT_SYMBOL(rtk_fc_fastfwd_ingress_rcv);
EXPORT_SYMBOL(rtk_fc_fastfwd_directXmit);
EXPORT_SYMBOL(rtk_fc_coreDBInfo_get);
EXPORT_SYMBOL(rtk_fc_reg_lut_notify);
EXPORT_SYMBOL(rtk_fc_l2Info_get);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_DESCRIPTION("HWNAT - FleetConntrack Driver");

