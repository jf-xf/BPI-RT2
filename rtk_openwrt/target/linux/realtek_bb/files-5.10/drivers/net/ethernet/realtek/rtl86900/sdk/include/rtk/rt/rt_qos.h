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
 * $Revision: 42383 $
 * $Date: 2013-08-28 14:10:44 +0800 (Wed, 28 Aug 2013) $
 *
 * Purpose : Definition of QoS API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Ingress Priority Decision
 *           (2) Egress Remarking
 *           (3) Queue Scheduling
 *           (4) Congestion avoidance
 */

#ifndef __RT_QOS_H__
#define __RT_QOS_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/rt/rt_port.h>

/*
 * Symbol Definition
 */
#define RT_MAX_DCSP_NUM 64 


/*
 * Data Declaration
 */

/* Queue weights structure */
typedef struct rt_qos_queue_weights_s
{
    uint32 weights[RTK_MAX_NUM_OF_QUEUE]; /*Weight of QoS scheduling and weight = 0 means strict*/
} rt_qos_queue_weights_t;

/* DSCP to P-bit structure */
typedef struct rt_qos_dscp2Pbit_s
{
    uint8 dscp[RT_MAX_DCSP_NUM]; /*Dot1p mapping value for dscp 0~63*/
} rt_qos_dscp2Pbit_t;

/*
 * Function Declaration
 */

/* Function Name:
 *      rt_qos_init
 * Description:
 *      Configure QoS initial settings
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED    - Failed
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 *
 *      The initialization does the following actions:
 *      - set input bandwidth control parameters to default values
 *      - set priority decision parameters
 *      - set scheduling parameters
 *      - disable port remark ability
 *      - CPU port init 8 using prioroty to queue mapping index 0
 *      - Other port init 1 queue using prioroty to queue mapping index 1
 */
extern int32
rt_qos_init(void);

/* Module Name    : QoS              */
/* Sub-module Name: Queue scheduling */

/* Function Name:
 *      rt_qos_schedulingQueue_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      port      - port id
 * Output:
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *      If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
extern int32
rt_qos_schedulingQueue_get(rt_port_t port, rt_qos_queue_weights_t *pQweights);

/* Function Name:
 *      rt_qos_schedulingQueue_set
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      port      - port id
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Note:
 *      The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *      If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
extern int32
rt_qos_schedulingQueue_set(rt_port_t port, rt_qos_queue_weights_t *pQweights);



/* Function Name:
 *      rt_qos_dscp2pbit_get
 * Description:
 *      Set the dscp to P-bit mapping table
 * Input:
 *      None.
 * Output:
 *      pDscp2PbitTbl - the array of Dscp to P-bit mapping
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None.
 */
extern int32
rt_qos_dscp2pbit_get(rt_qos_dscp2Pbit_t *pDscp2PbitTbl);


/* Function Name:
 *      rt_qos_dscp2pbit_set
 * Description:
 *      Set the dscp to P-bit mapping table
 * Input:
 *      pDscp2PbitTbl - the array of Dscp to P-bit mapping
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      support DSCP 0~63 mapping to 802.1P priority 0~7
 */
int32
extern rt_qos_dscp2pbit_set(rt_qos_dscp2Pbit_t *pDscp2PbitTbl);



#endif /* __RT_QOS_H__ */
