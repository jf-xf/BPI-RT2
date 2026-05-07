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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (星期五, 03 五月 2013) $
 *
 * Purpose : Definition of TRAP API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration for traping packet to CPU
 *           (2) RMA
 *           (3) User defined RMA
 *           (4) System-wise management frame
 *           (5) System-wise user defined management frame
 *           (6) Per port user defined management frame
 *           (7) Packet with special flag or option
 *           (8) CFM and OAM packet
 *
 */

#ifndef __DAL_RTL8277C_TRAP_H__
#define __DAL_RTL8277C_TRAP_H__


/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtk/epon.h> 
#include <rtk/rt/rt_trap.h> 



/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
/* Function Name:
 *      dal_rtl8277c_trap_init
 * Description:
 *      Initial the trap module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
dal_rtl8277c_trap_init(void);

/* Function Name:
 *      dal_rtl8277c_trap_oamPduAction_get
 * Description:
 *      Get forwarding action of trapped oam PDU on specified port.
 * Input:
 *      None.
 * Output:
 *      pAction - pointer to forwarding action of trapped oam PDU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_oamPduAction_get(rtk_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_oamPduAction_set
 * Description:
 *      Set forwarding action of trapped oam PDU on specified port.
 * Input:
 *      action - forwarding action of trapped oam PDU
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_oamPduAction_set(rtk_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_omciPduAction_get
 * Description:
 *      Get forwarding action of trapped omci PDU on specified port.
 * Input:
 *      None.
 * Output:
 *      pAction - pointer to forwarding action of trapped omci PDU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_omciAction_get(rtk_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_omciAction_set
 * Description:
 *      Set forwarding action of trapped omci PDU on specified port.
 * Input:
 *      action - forwarding action of trapped omci PDU
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_omciAction_set(rtk_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portIgmpMldCtrlPktAction_get
 * Description:
 *      Get forwarding action of trapped IGMP/MLD on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of trapped IGMP/MLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portIgmpMldPktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portIgmpMldCtrlPktAction_set
 * Description:
 *      Set forwarding action of trapped IGMP/MLD on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped IGMP/MLD
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portIgmpMldPktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portDhcpPktAction_get
 * Description:
 *      Get forwarding action of trapped DHCP on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of DHCP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portDhcpPktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portDhcpPktAction_set
 * Description:
 *      Set forwarding action of trapped DHCP on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped DHCP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portDhcpPktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portPppoePktAction_get
 * Description:
 *      Get forwarding action of trapped PPPoE on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of trapped PPPoE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portPppoePktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portPppoePktAction_set
 * Description:
 *      Set forwarding action of trapped PPPoE on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped PPPoE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portPppoePktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portStpPktAction_get
 * Description:
 *      Get forwarding action of trapped STP on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of trapped STP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portStpPktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portStpPktAction_set
 * Description:
 *      Set forwarding action of trapped STP on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped STP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portStpPktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portHostPktAction_get
 * Description:
 *      Get forwarding action of trapped host packet on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      mac - host mac to be trap
 *      pAction - pointer to forwarding action of trapped host packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portHostPktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portHostPktAction_set
 * Description:
 *      Set forwarding action of trapped host packet on specified port.
 * Input:
 *      action - forwarding action of trapped host packet
 *      port - specified port
 *      mac - host mac to be trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portHostPktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portEthertypePktAction_get
 * Description:
 *      Get forwarding action of trapped special ethertype packet on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of trapped special ethertype packet
 *      ethertype - special ethertype to trap
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portEthertypePktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portEthertypePktAction_set
 * Description:
 *      Set forwarding action of trapped special ethertype packet on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped special ethertype packet
 *      ethertype - special ethertype to trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portEthertypePktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_portArpPktAction_get
 * Description:
 *      Get forwarding action of trapped ARP on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of trapped ARP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portArpPktAction_get(rt_port_t port, rt_action_t *pAction);

/* Function Name:
 *      dal_rtl8277c_trap_portArpPktAction_set
 * Description:
 *      Set forwarding action of trapped ARP on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped ARP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_rtl8277c_trap_portArpPktAction_set(rt_port_t port, rt_action_t action);

/* Function Name:
 *      dal_rtl8277c_trap_pattern_get
 * Description:
 *      Set the trapped pettern setting
 * Input:
 *      type - pattern type
 * Output:
 *      pattern - pattern data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 */
extern int32
dal_rtl8277c_trap_pattern_get(rt_trap_pattern_type_t type, rt_trap_pattern_t *pattern);

/* Function Name:
 *      dal_rtl8277c_trap_pattern_set
 * Description:
 *      Set the trapped pettern setting
 * Input:
 *      type - pattern type
 *      pattern - pattern data
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 */
extern int32
dal_rtl8277c_trap_pattern_set(rt_trap_pattern_type_t type, rt_trap_pattern_t pattern);


#endif /* __DAL_RTL8277C_TRAP_H__ */
