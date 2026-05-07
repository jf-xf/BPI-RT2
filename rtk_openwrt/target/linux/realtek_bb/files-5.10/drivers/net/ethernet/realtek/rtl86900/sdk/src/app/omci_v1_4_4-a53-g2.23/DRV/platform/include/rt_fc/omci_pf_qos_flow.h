#ifndef __OMCI_PF_QOS_FLOW_H__
#define __OMCI_PF_QOS_FLOW_H__





int omci_qos_flow_init(unsigned int maxFlow);
int omci_qos_flow_dump(unsigned int maxFlow);
int omci_qos_GetUsDrvFlowId(unsigned int usLogicFlowId, unsigned int *pUsDrvFlowId);
int omci_qos_usFlow_set(unsigned int usFlowId, rt_gpon_usFlow_t *pUsFlow);
int omci_qos_usFlow_del(unsigned int usFlowId);
int omci_qos_usFlow_delAll(void);

#endif

