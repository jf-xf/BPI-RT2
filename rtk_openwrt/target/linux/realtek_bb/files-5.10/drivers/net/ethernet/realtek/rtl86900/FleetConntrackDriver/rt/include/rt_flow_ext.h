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
 * Purpose : Definition of flow extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) flow callback functions register
 *           (2) flow operations
 *           (3) flow acceleration mode configuration
 *
 */

#ifndef __RT_FLOW_EXT_H__
#define __RT_FLOW_EXT_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

#include "rt_stat_ext.h" 				//for rt_stat_flow_mib_t


#define RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE 4
/*
 * Symbol Definition
 */
typedef enum rt_flow_callback_func_e
{
	RT_FLOW_CB_FUNC_FLOW_ADD = 0,		/* callback function when flow add			(pCbFunc type: cbFlowAdd_t*) */
	RT_FLOW_CB_FUNC_FLOW_DEL,			/* callback function when flow DEL			(pCbFunc type: cbFlowDel_t*) */
	RT_FLOW_CB_FUNC_SHORTCUT_BEFORE,	/* callback function before shortcut modify	(pCbFunc type: cbShortcut_t*) */
	RT_FLOW_CB_FUNC_SHORTCUT_AFTER,		/* callback function after shortcut modify	(pCbFunc type: cbShortcut_t*) */
	
	RT_FLOW_CB_FUNC_END,
}rt_flow_callback_func_t;

typedef enum rt_flow_extra_info_e
{
	FLOW_INFO_ALG_EXIST=(1<<0),
	FLOW_INFO_NATLOOPBACK_ENTRY=(1<<1),
	FLOW_INFO_VLAN_FILTERING_PVID_ENTRY=(1<<2),		// to refresh SMAC fdb with pvid lookup
	FLOW_INFO_FORCE_TO_BRIDGE_FLOW=(1<<3),
	FLOW_INFO_VXD_RX_ENTRY=(1<<4),						// path5 but disable smac_trans!

	FLOW_INFO_SOFTWARE_ONLY=(1<<16),
	FLOW_INFO_STATIC_ENTRY=(1<<17),
	FLOW_INFO_MC_ENTRY=(1<<18),
	FLOW_INFO_SKIP_CT=(1<<19),
	FLOW_INFO_ACL_PRI_FWD=(1<<20),
	FLOW_INFO_DUPLICATE_EXIST=(1<<21),
	FLOW_INFO_PATH5_DS=(1<<22),

}rt_flow_extra_info_t;

typedef struct rt_cbFunc_flow_info_s
{	/* callback function flow infos */
	struct nf_conn *cachedCt;
	uint32 flowIdx;
	rt_flow_extra_info_t flow_extra_info;

}rt_cbFunc_flow_info_t;

typedef enum rt_flow_callback_func_ret_e
{	/* callback function return values */
	RT_FLOW_CB_FUNC_RET_TRAP = 0x80000,
	RT_FLOW_CB_FUNC_RET_CONTINUE,	
	RT_FLOW_CB_FUNC_RET_DROP,
	RT_FLOW_CB_FUNC_RET_END,
}rt_flow_callback_func_ret_t;


/*
 * cbFlowAdd_t description:
 * Input:
 * dpi_index	- DPI index (= ct index)
 * ct			- linux ct pointer
 * flowIdx 		- FC flow table index
 * direction 		- flow direction
 * 				0: UPSTREAM, 1: DOWNSTREAM
 * 				2: LANBRIDGE, 3: NATLOOPBACK
 * acc_mode 	- acceleration mode,
 * 				0: HW acceleration, 1: SW acceleration
 * Output:
 * none.
 */
typedef int (*cbFlowAdd_t)(uint32 dpi_index, /*struct nf_conn*/void *ct, int flowIdx, int direction, int acc_mode);

/*
 * cbFlowDel_t description:
 * Input:
 * flowIdx              - FC flow table index
 * ingress_packet_count - ingress packet count
 * ingress_byte_count   - ingress byte count
 * Output:
 * none.
 */
typedef int (*cbFlowDel_t)(int flowIdx, uint32 ingress_packet_count, uint64 ingress_byte_count);

/*
 * cbShortcut_t description:
 * Input:
 * skb			- linux skb pointer
 * flowIdx 		- FC flow table index
 * Output:
 * none.
 */
typedef rt_flow_callback_func_ret_t (*cbShortcut_t)(/*struct sk_buff */void *skb, rt_cbFunc_flow_info_t flow_info);


typedef enum rt_flow_ops_e
{
	RT_FLOW_OPS_DELETE_FLOW = 0, 			/* delete flow by flow key */
	RT_FLOW_OPS_FLUSH_FLOWTABLE,			/* flush flow table, including SW and HW flows */
	RT_FLOW_OPS_GET_SW_FLOW_MIB_BY_IDX,		/* get flow mib by flow index (support in HWNAT_MODE_SW_ONLY mode only) */
	RT_FLOW_OPS_GET_FLOWGRP_MIB_BY_IDX, 	/* get flow group mib by flowmib index */
	RT_FLOW_OPS_CLEAR_ALL_MIB,				/* clear all mib, including flow mib and flow group mib */
	RT_FLOW_OPS_CLEAR_SW_FLOW_MIB_BY_IDX,	/* clear flow mib by flow index (support in HWNAT_MODE_SW_ONLY mode only) */
	RT_FLOW_OPS_CLEAR_FLOWGRP_MIB_BY_IDX,	/* clear flow group mib by flowmib index */
	RT_FLOW_OPS_SYNC_CT_STATE,				/* sync ct state to flow table immediately */
	RT_FLOW_OPS_HIPRIFLOW_PTN_ACT,			/* high prioity flow pattern action */
	
	RT_FLOW_OPS_END,
} rt_flow_ops_t;

typedef enum rt_flow_op_flow_key_type_e
{
	RT_FLOW_OP_FLOW_KEY_TYPE_INDEX = 0,	/* delete flow by index */
	RT_FLOW_OP_FLOW_KEY_TYPE_PATTERN,	/* delete flow by pattern */
	
	RT_FLOW_OP_DEL_FLOW_KEY_TYPE_END,
}rt_flow_op_flow_key_type_t;

typedef enum rt_flow_op_flow_pattern_l4proto_e
{
	RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP = 0,	/* flow pattern: L4 protocol UDP */
	RT_FLOW_OP_FLOW_PATTERN_L4PROTO_TCP,		/* flow pattern: L4 protocol TCP */
}rt_flow_op_flow_pattern_l4proto_t;

typedef enum rt_flow_op_hipriflow_ptn_act_type_e
{
	RT_FLOW_OP_HIPRIFLOW_PTN_ACT_ADD_BY_IDX = 0,	/* add a high priority flow pattern entry by index */
	RT_FLOW_OP_HIPRIFLOW_PTN_ACT_DEL_BY_IDX,		/* delete a high priority flow pattern entry by index */
	RT_FLOW_OP_HIPRIFLOW_PTN_ACT_GET_BY_IDX,		/* get a high priority flow pattern entry by index */
	RT_FLOW_OP_HIPRIFLOW_PTN_ACT_FLUSH,				/* flush high priority flow pattern table */

	RT_FLOW_OP_HIPRIFLOW_PTN_ACT_END,
}rt_flow_op_hipriflow_ptn_act_type_t;

typedef enum rt_flow_hwnat_mode_e
{
	RT_FLOW_HWNAT_MODE_DIS_ACC = 0, 		/* Disable acceleration */
	RT_FLOW_HWNAT_MODE_ORIGINAL = 1,		/* Enable acceleration */
	RT_FLOW_HWNAT_MODE_SW_ONLY = 2,			/* Disable HW acceleration */
	RT_FLOW_HWNAT_MODE_HW_ONLY = 3,			/* Disable SW acceleration */
	RT_FLOW_HWNAT_MODE_TRAP_ALL = 4,    	/* Trap all packet to CPU, and skip FC */
	RT_FLOW_HWNAT_MODE_DIS_UPSTREAM=5,		/* Disable upstream acceleration */
	RT_FLOW_HWNAT_MODE_END,
	RT_FLOW_HWNAT_MODE_SPEC_TEST = 10,		/* Special mode for testing. */
}rt_flow_hwnat_mode_t;

#define HWNAT_MODE_SKIP_TIMER_BITMASK		((1<<RT_FLOW_HWNAT_MODE_DIS_ACC) | (1<<RT_FLOW_HWNAT_MODE_TRAP_ALL | (1<<RT_FLOW_HWNAT_MODE_SPEC_TEST)))
#define HWNAT_MODE_SKIP_FC_FUNC_BITMASK	((1<<RT_FLOW_HWNAT_MODE_DIS_ACC) | (1<<RT_FLOW_HWNAT_MODE_TRAP_ALL))

/*
 * Macro Declaration
 */


/*
 * Data Declaration
 */
typedef struct rt_flow_op_flow_pattern_s
{
	uint8 l4proto_valid:1;
	uint8 sport_valid:1;
	uint8 dport_valid:1;
	uint8 sip4_valid:1;
	uint8 dip4_valid:1;
	uint8 sip6_valid:1;
	uint8 dip6_valid:1;
	uint8 l4proto:1;		/* rt_flow_op_flow_pattern_l4proto_t */
	uint8 modified_sport_valid:1;
	uint8 modified_dport_valid:1;
	uint8 modified_sip4_valid:1;
	uint8 modified_dip4_valid:1;
	uint8 mac_valid:1;
	uint8 smac_valid:1;
	uint8 dmac_valid:1;
	uint32 sport:16;
	uint32 dport:16;
	uint32 sip4;
	uint32 dip4;
	uint8 sip6[16];
	uint8 dip6[16];
	uint8 mac[6];
	uint8 smac[6];
	uint8 dmac[6];
	int16 mac_lut_idx;
	int16 smac_lut_idx;
	int16 dmac_lut_idx;
	uint32 modified_sport:16;		/* if path5.out_l4_act=1 and path5.out_l4_direction=1, modified_sport=out_l4_port; otherwise do not check */
	uint32 modified_dport:16;		/* if path5.out_l4_act=1 and path5.out_l4_direction=0, modified_dport=out_l4_port; otherwise do not check */
	uint32 modified_sip4;			/* if path5.out_l4_act=1 and path5.out_l4_direction=1, modified_sip4=gateway_ipv4_addr; otherwise do not check */
	uint32 modified_dip4;			/* if path5.out_l4_act=1 and path5.out_l4_direction=0, modified_dip4=out_dst_ipv4_addr; otherwise do not check */
}rt_flow_op_flow_pattern_t;

typedef struct rt_flow_op_flow_key_s
{
	union{
		uint32 flowIdx;
		rt_flow_op_flow_pattern_t flowPattern;
	};
}rt_flow_op_flow_key_t;

typedef struct rt_flow_key_info_s
{
	rt_flow_op_flow_key_type_t flowKeyType;
	rt_flow_op_flow_key_t flowKey;
}rt_flow_key_info_t;

typedef struct rt_flow_op_sw_flow_mib_info_s
{
	uint32 ingress_packet_count;
	uint64 ingress_byte_count;
}rt_flow_op_sw_flow_mib_info_t;

typedef struct rt_flow_op_mib_info_s
{
	uint32 index;
	union
	{
		rt_flow_op_sw_flow_mib_info_t swFlowCnt; //for SW flow mib
		rt_stat_flow_mib_t flowGrpCnt; // for group mib
	};
	
}rt_flow_op_mib_info_t;

typedef struct rt_flow_op_hiPriflow_ptn_act_s
{
	rt_flow_op_hipriflow_ptn_act_type_t hipriflow_ptn_act;
	uint32 index;
	rt_flow_op_flow_pattern_t flowPattern;
}rt_flow_op_hiPriflow_ptn_act_t;

typedef rt_flow_key_info_t rt_flow_op_data_delete_flow_t;
typedef char rt_flow_op_data_flush_flowtable_t;
typedef rt_flow_op_mib_info_t rt_flow_op_data_get_swFlow_mib_t;
typedef rt_flow_op_mib_info_t rt_flow_op_data_get_flowGrp_mib_t;
typedef char rt_flow_op_data_clear_all_mib_t;
typedef uint32 rt_flow_op_data_clear_swFlow_mib_t;
typedef uint32 rt_flow_op_data_clear_flowGrp_mib_t;
typedef char rt_flow_op_data_sync_ct_state_t;

typedef struct rt_flow_ops_data_s
{
	union{
		rt_flow_op_data_delete_flow_t data_delFlow;					//RT_FLOW_OPS_DELETE_FLOW
		rt_flow_op_data_flush_flowtable_t data_flushFlow;			//RT_FLOW_OPS_FLUSH_FLOWTABLE (meaningless)
		rt_flow_op_data_get_swFlow_mib_t data_getFlowMib;			//RT_FLOW_OPS_GET_FLOW_MIB_BY_IDX
		rt_flow_op_data_get_flowGrp_mib_t data_getFlowGrpMib;		//RT_FLOW_OPS_GET_FLOWGRP_MIB_BY_IDX
		rt_flow_op_data_clear_all_mib_t data_clearAllMib;			//RT_FLOW_OPS_CLEAR_ALL_MIB (meaningless)
		rt_flow_op_data_clear_swFlow_mib_t data_clearFlowMib;		//RT_FLOW_OPS_CLEAR_FLOW_MIB_BY_IDX
		rt_flow_op_data_clear_flowGrp_mib_t data_clearFlowGrpMib;	//RT_FLOW_OPS_CLEAR_FLOWGRP_MIB_BY_IDX
		rt_flow_op_data_sync_ct_state_t	data_syncCtState;			//RT_FLOW_OPS_SYNC_CT_STATE (meaningless)
		rt_flow_op_hiPriflow_ptn_act_t data_hiPriflowPtnAct;		//RT_FLOW_OPS_HIPRIFLOW_PTN_ACT
	};
}rt_flow_ops_data_t;


/*
 * Function Declaration
 */
 /* Function Name:
 *      rt_flow_callbackFunc_register
 * Description:
 *      register callback function
 * Input:
 *      cbFuncType	- callback function type
 *      pCbFunc		- callback function pointer
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can register callback function.
 *      Please note that data type of pCbFunc is varied according to the callback function type.
 */
extern int32
rt_flow_callbackFunc_register (
	rt_flow_callback_func_t cbFuncType,
	void* pCbFunc);

/* Function Name:
 *      rt_flow_operation_add
 * Description:
 *      make a flow operation
 * Input:
 *      flowOp	- flow oparation
 *      pOpData	- data for oparation
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can make an operation to flows.
 *      Please note that data type of pOpData is varied according to the operation.
 */
extern int32
rt_flow_operation_add(
	rt_flow_ops_t flowOp,
	rt_flow_ops_data_t* pOpData);

/* Function Name:
 *      rt_flow_hwnatMode_set
 * Description:
 *      set configuration of hwnat mode
 * Input:
 *      hwnatMode	- Configuration of hwnat mode
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can set configuration of hwnat mode
 */
extern int32
rt_flow_hwnatMode_set (
	rt_flow_hwnat_mode_t hwnatMode);

/* Function Name:
 *      rt_flow_hwnatMode_get
 * Description:
 *      get configuration of hwnat mode
 * Input:
 *      None.
 * Output:
 *      pHwnatMode	- Configuration of hwnat mode
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get configuration of hwnat mode
 */
extern int32
rt_flow_hwnatMode_get (
	rt_flow_hwnat_mode_t *pHwnatMode);

#endif /* __RT_FLOW_EXT_H__ */
