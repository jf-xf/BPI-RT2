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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Declaration of RLDP and RLPP API
 *
 * Feature : The file have include the following module and sub-modules
 *           1) RLDP and RLPP configuration and status
 *
 */


#ifndef __RTKSW_RLDP_H__
#define __RTKSW_RLDP_H__


/*
 * Include Files
 */


/*
 * Symbol Definition
 */
typedef enum rtksw_rldp_trigger_e
{
    RTKSW_RLDP_TRIGGER_SAMOVING = 0,
    RTKSW_RLDP_TRIGGER_PERIOD,
    RTKSW_RLDP_TRIGGER_END
} rtksw_rldp_trigger_t;

typedef enum rtksw_rldp_cmpType_e
{
    RTKSW_RLDP_CMPTYPE_MAGIC = 0,     /* Compare the RLDP with magic only */
    RTKSW_RLDP_CMPTYPE_MAGIC_ID,      /* Compare the RLDP with both magic + ID */
    RTKSW_RLDP_CMPTYPE_END
} rtksw_rldp_cmpType_t;

typedef enum rtksw_rldp_loopStatus_e
{
    RTKSW_RLDP_LOOPSTS_NONE = 0,
    RTKSW_RLDP_LOOPSTS_LOOPING,
    RTKSW_RLDP_LOOPSTS_END
} rtksw_rldp_loopStatus_t;

typedef enum rtksw_rlpp_trapType_e
{
    RTKSW_RLPP_TRAPTYPE_NONE = 0,
    RTKSW_RLPP_TRAPTYPE_CPU,
    RTKSW_RLPP_TRAPTYPE_END
} rtksw_rlpp_trapType_t;

typedef struct rtksw_rldp_config_s
{
    rtksw_enable_t        rldp_enable;
    rtksw_rldp_trigger_t trigger_mode;
    rtksw_mac_t           magic;
    rtksw_rldp_cmpType_t  compare_type;
    rtksw_uint32              interval_check; /* Checking interval for check state */
    rtksw_uint32              num_check;      /* Checking number for check state */
    rtksw_uint32              interval_loop;  /* Checking interval for loop state */
    rtksw_uint32              num_loop;       /* Checking number for loop state */
} rtksw_rldp_config_t;

typedef struct rtksw_rldp_portConfig_s
{
    rtksw_enable_t        tx_enable;
} rtksw_rldp_portConfig_t;

typedef struct rtksw_rldp_status_s
{
    rtksw_mac_t           id;
} rtksw_rldp_status_t;

typedef struct rtksw_rldp_portStatus_s
{
    rtksw_rldp_loopStatus_t   loop_status;
    rtksw_rldp_loopStatus_t   loop_enter;
    rtksw_rldp_loopStatus_t   loop_leave;
} rtksw_rldp_portStatus_t;

/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */

#define RTKSW_RLDP_INTERVAL_MAX  0xffff
#define RTKSW_RLDP_NUM_MAX       0xff


/*
 * Function Declaration
 */

/* Module Name : RLDP */


/* Function Name:
 *      rtksw_rldp_config_set
 * Description:
 *      Set RLDP module configuration
 * Input:
 *      pConfig - configuration structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_config_set(rtksw_rldp_config_t *pConfig);


/* Function Name:
 *      rtksw_rldp_config_get
 * Description:
 *      Get RLDP module configuration
 * Input:
 *      None
 * Output:
 *      pConfig - configuration structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_config_get(rtksw_rldp_config_t *pConfig);


/* Function Name:
 *      rtksw_rldp_portConfig_set
 * Description:
 *      Set per port RLDP module configuration
 * Input:
 *      port   - port number to be configured
 *      pPortConfig - per port configuration structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_portConfig_set(rtksw_port_t port, rtksw_rldp_portConfig_t *pPortConfig);


/* Function Name:
 *      rtksw_rldp_portConfig_get
 * Description:
 *      Get per port RLDP module configuration
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortConfig - per port configuration structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_portConfig_get(rtksw_port_t port, rtksw_rldp_portConfig_t *pPortConfig);


/* Function Name:
 *      rtksw_rldp_status_get
 * Description:
 *      Get RLDP module status
 * Input:
 *      None
 * Output:
 *      pStatus - status structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_status_get(rtksw_rldp_status_t *pStatus);


/* Function Name:
 *      rtksw_rldp_portStatus_get
 * Description:
 *      Get RLDP module status
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortStatus - per port status structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_portStatus_get(rtksw_port_t port, rtksw_rldp_portStatus_t *pPortStatus);


/* Function Name:
 *      rtksw_rldp_portStatus_clear
 * Description:
 *      Clear RLDP module status
 * Input:
 *      port    - port number to be clear
 *      pPortStatus - per port status structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      Clear operation effect loop_enter and loop_leave only, other field in
 *      the structure are don't care
 */
extern rtksw_api_ret_t rtksw_rldp_portStatus_set(rtksw_port_t port, rtksw_rldp_portStatus_t *pPortStatus);


/* Function Name:
 *      rtksw_rldp_portLoopPair_get
 * Description:
 *      Get RLDP port loop pairs
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortmask - per port related loop ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtksw_api_ret_t rtksw_rldp_portLoopPair_get(rtksw_port_t port, rtksw_portmask_t *pPortmask);

#endif /* __RTKSW_RLDP_H__ */

