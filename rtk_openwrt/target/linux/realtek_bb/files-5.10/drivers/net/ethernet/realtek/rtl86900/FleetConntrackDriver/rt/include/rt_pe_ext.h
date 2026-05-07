/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 68395 $
 * $Date: 2016-05-27 16:38:35 +0800 (Fri, 27 May 2016) $
 *
 * Purpose : Definition of PE API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of http test
 *
 */


#ifndef __RT_PE_EXT_H__
#define __RT_PE_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */
//crypto
#define RTK_FC_CRYPTO_MAX_IPC_DESC_NUM 25
#define RTK_FC_CRYPTO_MAX_ARRAY_SIZE 128
#define RT_PE_IPSEC_PAGE_ORDER (8) // 1MB buffer size
#define RT_PE_IPSEC_DMA_LSO_DECRYPT_HWLOOKUP_LSPID RTK_FC_IPSEC_HWLOOKUP_MACPORT
#define RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID (0x21)
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_IPSEC_CPU_PORT (0x13)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_IPSEC_CPU_PORT (0x11)
#endif
#define MAX_PE_IPSEC_HW_KEY_NUM (12)
#define MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM (4)
#define MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM (4)
#define RT_PE_IPSEC_SW_ID_DECRYPTION_BIT (0x20)
#define RT_PE_IPSEC_SW_ID_CONNECTION_IDX_MASK (0x1f)
#define RT_PE_IPSEC_SW_ID_CONNECTION_IDX_BASE (1)
#define RT_PE_IPSEC_IV_LEN (16)
#if	(MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM > 31) || (MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM > 31)
#error "PE IPSEC encrypt/decrypt connection num is overflow"
#endif
//http 
#define RT_PE_HTTP_TEST_PAGE_ORDER (8) // 1MB buffer size
#define MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM (8)
#define MAX_PE_HTTP_UPLOAD_CONNECTION_NUM (8)
#define MAX_PE_HTTP_CONNECTION_NUM ((MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM>MAX_PE_HTTP_UPLOAD_CONNECTION_NUM) ? MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM : MAX_PE_HTTP_UPLOAD_CONNECTION_NUM)
#define MAX_PE_HTTP_REQ_URL_STR_LENGTH (350)
#define MAX_PE_HTTP_VERSION_STR_LENGTH (20)
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_HTTP_TEST_CPU_PORT (0x13)
#define RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID (11)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_HTTP_TEST_CPU_PORT (0x11)
#define RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID (7)
#endif
#if defined(CONFIG_FC_CA8277B_SERIES)
#define RT_PE_HTTP_TEST_DMA_LSO_LSPID (0x20)
#else //CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES
#define RT_PE_HTTP_TEST_DMA_LSO_LSPID (0x11)
#endif
//AMSDU
#define RT_PE_AMSDU_OFLD_BUF_SEC 2
#define RT_PE_AMSDU_OFLD_TOTAL_PAGE_ORDER_NUM 10

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RT_PE_HTTP_TEST_DMA_LSO_BACKPRESSURE (1)
#endif
#define RT_PE_DMA_LSO_SHAPER_RATE_1DOT25G 	(0x0012C000)  //1200M
#define RT_PE_DMA_LSO_SHAPER_RATE_2DOT5G 	(0x00258000)  //2400M
#define RT_PE_DMA_LSO_SHAPER_RATE_10G 		(0x00992000)  //9800M

/* number of microseconds per millisecond */
#define RT_PE_USEC_PER_MSEC (1000)

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#define RT_PE_MAX_CPU_NUM (2)
#else	//CONFIG_FC_RTL9607F_SERIES
#define RT_PE_MAX_CPU_NUM (3)
#endif

/*
 * Data Declaration
 */
typedef enum rt_pe_func_num_e
{
	RT_PE_FUNC_NUM_WIFI_TX_AMSDU	= 0,
	RT_PE_FUNC_NUM_HTTP				= 1,
	RT_PE_FUNC_NUM_CRYPTO			= 2,
	RT_PE_FUNC_NUM_WFO				= 3,
	RT_PE_FUNC_NUM_MAX
} rt_pe_func_num_t;
 
typedef enum rt_pe_ret_e
{
	RT_PE_RET_FAIL				= -1,
	RT_PE_RET_OK				= 0,
	RT_PE_RET_PARAM_SIZE_ERROR	= 1,
	RT_PE_RET_EXISTED			= 2,
	RT_PE_RET_NOT_FOUND 		= 3,
} rt_pe_ret_t;

typedef enum 
{
	RT_PE_CRYPTO_EALG_AES_128	= 0,
	RT_PE_CRYPTO_EALG_AES_192	= 1,
	RT_PE_CRYPTO_EALG_AES_256	= 2,
	RT_PE_CRYPTO_EALG_DES		= 3,
	RT_PE_CRYPTO_EALG_3DES		= 4,
	RT_PE_CRYPTO_EALG_MAX
} rt_pe_crypto_ealg_t;

typedef enum 
{
	RT_PE_CRYPTO_AALG_MD5HMAC 		= 0,
	RT_PE_CRYPTO_AALG_SHA1HMAC 		= 1,
	RT_PE_CRYPTO_AALG_SHA2_224HMAC 	= 2,
	RT_PE_CRYPTO_AALG_SHA2_256HMAC 	= 3,
	RT_PE_CRYPTO_AALG_MAX
} rt_pe_crypto_aalg_t;

typedef enum rt_pe_crypto_engine_cmd_e
{
	RT_PE_CRYPTO_ENGINE_CMD_ENABLE 					= 0,
	RT_PE_CRYPTO_ENGINE_CMD_DISABLE 				= 1,
	RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_ADD 	= 2,
	RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_DEL 	= 3,
	RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_ADD 	= 4,
	RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_DEL 	= 5,
	RT_PE_CRYPTO_ENGINE_CMD_STATUS_GET			 	= 6,
	RT_PE_CRYPTO_ENGINE_CMD_PS_SEND_DESC			= 7,
	RT_PE_CRYPTO_ENGINE_CMD_MAX
} rt_pe_crypto_engine_cmd_t;

typedef enum rt_pe_amsdu_ofld_cmd_t
{
	RT_PE_AMSDU_OFLD_CMD_GET_PAGE 					= 0,
	RT_PE_AMSDU_OFLD_CMD_MAX
} rt_pe_amsdu_ofld_cmd_t;

typedef enum rt_pe_crypto_engine_return_cmd_e
{
	RT_PE_CRYPTO_ENGINE_RETURN_CMD_FINISH	= 0,
	RT_PE_CRYPTO_ENGINE_HW_CRYPTO_READY		= 1,
	RT_PE_CRYPTO_ENGINE_RETURN_CMD_MAX
} rt_pe_crypto_engine_return_cmd_t;

typedef enum rt_pe_http_test_cmd_e
{
	RT_PE_HTTP_TEST_CMD_DOWNLOAD_START = 0,
	RT_PE_HTTP_TEST_CMD_DOWNLOAD_STOP,
	RT_PE_HTTP_TEST_CMD_DOWNLOAD_GET_CNT,
	RT_PE_HTTP_TEST_CMD_UPLOAD_START,
	RT_PE_HTTP_TEST_CMD_UPLOAD_STOP,
	RT_PE_HTTP_TEST_CMD_UPLOAD_GET_CNT,
	RT_PE_HTTP_TEST_CMD_LATENCY_START,
	RT_PE_HTTP_TEST_CMD_LATENCY_STOP,
	RT_PE_HTTP_TEST_CMD_LATENCY_GET_RESULT,
	RT_PE_HTTP_TEST_CMD_MAX
} rt_pe_http_test_cmd_t;

typedef enum rt_pe_http_test_return_cmd_e
{
	RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_FINISH 	= 0,
	RT_PE_HTTP_TEST_RETURN_CMD_DOWNLOAD_CNT 	= 1,
	RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_FINISH 	= 2,
	RT_PE_HTTP_TEST_RETURN_CMD_UPLOAD_CNT 		= 3,
	RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_FINISH 	= 4,
	RT_PE_HTTP_TEST_RETURN_CMD_LATENCY_RESULT 	= 5,
	RT_PE_HTTP_TEST_RETURN_CMD_MAX
} rt_pe_http_test_return_cmd_t;

typedef enum {
	RTK_PE_HTTP_TEST_MODE_NORMAL 				= 0,
	RTK_PE_HTTP_TEST_MODE_SKIP_GET_POST 		= 1,	//skip get/post packet
	RTK_PE_HTTP_TEST_MODE_MAX
} rtk_pe_http_test_mode_t;

typedef enum {
	RTK_PE_AMSDU_MODE_DISABLE_FLOW			= 0,
	RTK_PE_AMSDU_MODE_FUNC_SWITCHING 		= 1,
	RTK_PE_AMSDU_MODE_MAX
} rtk_pe_amsdu_mode_t;

typedef struct rt_stream_id_s
{
	//tx desc for stream id
	int32 sid;		// -1 invalid
	//header_a for stream id
	uint8 ldpid; 	//tcont, -1 invalid
	uint8 cos;
	uint16 flowid; 	//gemid
} rt_stream_id_t;

typedef struct rt_pe_http_test_request_s
{
	rt_pe_http_test_cmd_t req_cmd;
		
	rtk_mac_t server_mac;
	rtk_mac_t client_mac;
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}client_ip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}server_ip;
	uint16 client_l4port;
	uint16 server_l4port;

	uint32 ldpid;
	int16 pppoe_sid;	// -1 invalid
	int16 svlan_vid;	// -1 invalid
	int16 cvlan_vid;	// -1 invalid
	int16 pon_streamId;	// -1 invalid

	uint8 http_req_url[MAX_PE_HTTP_REQ_URL_STR_LENGTH]; 	//e.g., "/garbage.php?ckSize=5000"
	uint8 http_version[MAX_PE_HTTP_VERSION_STR_LENGTH]; 	//e.g., "HTTP/1.1"

	uint8 non_congestion_mode;		// for download

	uint64 upload_content_length; 	// unit: bytes
	
	uint32 tcp_window_size; 		// unit: bytes, 0: means it uses default window size
	uint16 tcp_mss_size;			// unit: bytes, 0: means it uses default value

	uint32 connection_number;		// MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM, MAX_PE_HTTP_UPLOAD_CONNECTION_NUM

	rtk_pe_http_test_mode_t test_mode;
}rt_pe_http_test_request_t;

typedef struct rt_pe_http_test_result_s
{
	rt_pe_ret_t ret_val;
	uint32 pktCnt;
	uint64 byteCnt;					//payload only
	uint64 byteCnt_include_pktHdr;	//L2+L3+L4+payload
	uint32 total_pktCnt;
	uint64 total_byteCnt;					//payload only
	uint64 total_byteCnt_include_pktHdr;	//L2+L3+L4+payload
	//the unit of following time is milliseconds 
	uint64 TCP_openRequestTime_ms; //It's relative time and is always zero
	uint64 TCP_openResponseTime_ms;
	uint64 HTTP_ROMTime_ms; //http get
	uint64 HTTP_BOMTime_ms; //start transmission
	uint64 HTTP_EOMTime_ms; //end transmission
	uint64 latency_us;	//microsecond
	uint64 jitter_us;	//microsecond
	uint16 pktLoss;
	uint16 pktRcv;
}rt_pe_http_test_result_t;

typedef struct rt_pe_crypto_encrypt_info_s
{
	int32 key_idx;
	int32 hash_key_idx;
	rt_pe_crypto_ealg_t cipher_mode;
	rt_pe_crypto_aalg_t hash_mode;
	uint32 iv_len;
	uint32 hash_icv_len;
	
	rtk_mac_t outer_dmac;
	rtk_mac_t outer_smac;
	uint8 outer_isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}outer_sip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}outer_dip;

	uint32 esp_spi;
	uint32 esp_seq_no;
	uint8 iv[RT_PE_IPSEC_IV_LEN];
	
	uint32 ldpid;
	union{
		struct{
			int16 pppoe_sid;	// -1 invalid
			int16 svlan_vid;	// -1 invalid
			int16 cvlan_vid;	// -1 invalid
			int16 pon_streamId;	// -1 invalid
		}external_used;
		struct{
			int32 dma_aft_idx; // -1 invalid, HW offload for PPPoE sid, svlan, cvlan 
			rt_stream_id_t streamId;
		}internal_used;
	}tag_info;
}rt_pe_crypto_encrypt_info_t;

typedef struct rt_pe_crypto_decrypt_info_s
{
	int32 key_idx;
	int32 hash_key_idx;
	rt_pe_crypto_ealg_t cipher_mode;
	rt_pe_crypto_aalg_t hash_mode;
	uint32 iv_len;
	uint32 hash_icv_len;
	
	uint8 outer_isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	
	rtk_mac_t dmac;
	rtk_mac_t smac;

	uint32 hwlookup_swId;
	uint32 hwlookup_lspid;
}rt_pe_crypto_decrypt_info_t;

typedef struct rt_pe_crypto_request_s
{
	rt_pe_crypto_engine_cmd_t req_cmd;
	uint32 connection_idx;
	union{
		rt_pe_crypto_encrypt_info_t encrypt_info;
		rt_pe_crypto_decrypt_info_t decrypt_info;
	}connection;
	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
	uint32 ps_wait_desc_done_phy_addr;
	uint32 ps_wait_pop_done_phy_addr;
}rt_pe_crypto_request_t;

typedef struct rt_pe_amsdu_ofld_request_s
{
	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
} rt_pe_amsdu_ofld_request_t;

/*
 * Function Declaration
 */

/* Function Name:
 *      rt_pe_http_test
 * Description:
 *      Set configuration of pe http test
 * Input:
 *      http_test_req	- Configuration of http test request 
 * Output:
 *		pHttp_test_result
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of pe http test
 */
extern int rt_pe_http_test(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result);

#endif /* __RT_PE_EXT_H__ */
