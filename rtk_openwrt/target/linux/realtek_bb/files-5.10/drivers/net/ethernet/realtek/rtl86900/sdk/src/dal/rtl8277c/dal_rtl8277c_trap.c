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



/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>
#include <rtk/trap.h>
#include <dal/rtl8277c/dal_rtl8277c.h>
#include <dal/rtl8277c/dal_rtl8277c_trap.h>
#include <osal/time.h>

#include "cortina-api/include/port.h"
#include "cortina-api/include/special_packet.h"
#include "aal_port.h"
#include "aal_l2_specpkt.h"
#include "aal_pdc.h"
/*
 * Symbol Definition
 */

#define DAL_RTL8277C_TRAP_CPU_LDPID 0x10


/*
 * Data Declaration
 */
static uint32 rtl8277c_trap_init = INIT_NOT_COMPLETED;
uint8 rtl8277c_trap_port_spb_idx_map[8] = {1,2,3,4,5,5,6,7};

/*
 * Function Declaration
 */


int32 _dal_rtl8277c_trap_action_set(aal_l2_specpkt_type_t type, rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else if(action == ACTION_FORWARD)
    {
        ctrl.spcl_fwd = 0;
    }
    else
    {
        return RT_ERR_FWD_ACTION;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], type, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}

int32 _dal_rtl8277c_trap_action_get(aal_l2_specpkt_type_t type, rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], type, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}



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
int32
dal_rtl8277c_trap_init(void)
{   
    aal_ilpb_cfg_t ilpbCfg;
    aal_ilpb_cfg_msk_t ilpbCfgMask;
    uint8 i = 0;
    /*ILPB special packet index setting*/
    for(i=0;i<sizeof(rtl8277c_trap_port_spb_idx_map);i++)
    {
        memset(&ilpbCfgMask,0,sizeof(aal_ilpb_cfg_msk_t));
        ilpbCfgMask.s.spcl_pkt_idx = 1;
        memset(&ilpbCfg,0,sizeof(aal_ilpb_cfg_t));
        ilpbCfg.spcl_pkt_idx = rtl8277c_trap_port_spb_idx_map[i];
        aal_port_ilpb_cfg_set(0, i, ilpbCfgMask, &ilpbCfg);
    }
    rtl8277c_trap_init = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_rtl8277c_trap_init */


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
int32
dal_rtl8277c_trap_oamPduAction_get(rtk_action_t *pAction)
{
    ca_status_t ret=CA_E_OK;
    aal_l2_specpkt_detect_t detect;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(rtl8277c_trap_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    /* function body */
    memset(&detect,0,sizeof(aal_l2_specpkt_detect_t));

    if((ret = aal_l2_specpkt_detect_get(0, &detect)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }


    *pAction = ((detect.udf0_detect_enable == 1) ? ACTION_TRAP2CPU : ACTION_FORWARD);
    return RT_ERR_OK;
} /* end of dal_rtl8277c_trap_oamPduAction_get */

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
int32
dal_rtl8277c_trap_oamPduAction_set(rtk_action_t action)
{
    ca_status_t ret=CA_E_OK;
    aal_ilpb_cfg_t ilpb_cfg;
    aal_ilpb_cfg_msk_t ilpb_msk;
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrl_mask;
    aal_l2_specpkt_detect_t detect;
    aal_l2_specpkt_detect_mask_t detect_mask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(rtl8277c_trap_init);

    /* function body */
    memset(&ilpb_cfg,0,sizeof(aal_ippb_cfg_t));
    memset(&ilpb_msk,0,sizeof(aal_ilpb_cfg_msk_t));
    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrl_mask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    memset(&detect,0,sizeof(aal_l2_specpkt_detect_t));
    memset(&detect_mask,0,sizeof(aal_l2_specpkt_detect_mask_t));

    switch(action)
    {
        case ACTION_FORWARD:
            detect.udf0_detect_enable = 0; 
            break;
        case ACTION_TRAP2CPU:
            detect.udf0_detect_enable = 1; 
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
            break;
    }
    
    if((ret = aal_port_ilpb_cfg_get(0, 7, &ilpb_cfg)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }

    ilpb_msk.s.spcl_pkt_idx = 1;

    if((ret = aal_port_ilpb_cfg_set(0, 0x20, ilpb_msk, &ilpb_cfg)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }

    ctrl_mask.s.ldpid = 1;
    ctrl_mask.s.pkt_no_drop = 1;
    ctrl_mask.s.spcl_fwd = 1;
    ctrl_mask.s.cos = 7;
    ctrl_mask.s.learn_dis = 1;

    ctrl.ldpid = 0x10;
    ctrl.pkt_no_drop = 1;
    ctrl.spcl_fwd = 1;
    ctrl.cos = 7;
    ctrl.learn_dis =1;

    if((ret = aal_l2_specpkt_ctrl_set(0, ilpb_cfg.spcl_pkt_idx, AAL_L2_SPECPKT_TYPE_UDF_0, ctrl_mask, &ctrl)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }

    detect_mask.s.udf0_ethtype_enable = 1;
    detect_mask.s.udf0_ethtype = 1;
    detect_mask.s.udf0_mac_mask = 1;
    detect_mask.s.udf0_mac = 1;
    detect_mask.s.udf0_detect_enable = 1;

    detect.udf0_mac[0] = 0x00;
    detect.udf0_mac[1] = 0x13;
    detect.udf0_mac[2] = 0x25;
    detect.udf0_mac[3] = 0x00;
    detect.udf0_mac[4] = 0x00;
    detect.udf0_mac[5] = 0x00;
    detect.udf0_mac_mask[0] = 0xFF;
    detect.udf0_mac_mask[1] = 0xFF;
    detect.udf0_mac_mask[2] = 0xFF;
    detect.udf0_mac_mask[3] = 0xFF;
    detect.udf0_mac_mask[4] = 0xFF;
    detect.udf0_mac_mask[5] = 0x00;
    detect.udf0_ethtype = 0xfff1;
    detect.udf0_ethtype_enable = 1;

    if((ret = aal_l2_specpkt_detect_set(0, detect_mask, &detect)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl8277c_trap_oamPduAction_set */


/* Function Name:
 *      dal_rtl8277c_trap_omciAction_get
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
int32
dal_rtl8277c_trap_omciAction_get(rtk_action_t *pAction)
{
    ca_status_t ret=CA_E_OK;
    aal_pdc_ctrl_t pdc_ctrl;

    memset(&pdc_ctrl, 0, sizeof(pdc_ctrl));
    if((ret = aal_pdc_ctrl_get(0, &pdc_ctrl)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }

    if(pdc_ctrl.omci_hp_en == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;
    
    return RT_ERR_OK;
} /* end of dal_rtl8277c_trap_omciAction_get */

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
int32
dal_rtl8277c_trap_omciAction_set(rtk_action_t action)
{
    ca_status_t ret=CA_E_OK;
    aal_pdc_pdc_map_mem_data_mask_t pdc_mask;
    aal_pdc_pdc_map_mem_data_t      pdc_cfg;
    aal_pdc_ctrl_mask_t             pdc_ctrl_mask;
    aal_pdc_ctrl_t                  pdc_ctrl;
    int gem_port;

    memset(&pdc_cfg, 0, sizeof(pdc_cfg));
    pdc_mask.wrd = ~0;

    memset(&pdc_ctrl, 0, sizeof(pdc_ctrl));
    pdc_ctrl_mask.wrd = 0;

    switch(action)
    {
        case ACTION_FORWARD:
            pdc_cfg.fe_bypass = 0;
            pdc_ctrl.omci_hp_en = 0;
            break;
        case ACTION_TRAP2CPU:
            pdc_cfg.fe_bypass = 1;
            pdc_ctrl.omci_hp_en = 1;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
            break;
    }
    pdc_cfg.ldpid     = 0x10;
    pdc_cfg.lspid     = 7;
    pdc_cfg.no_drop   = 1;
    pdc_cfg.pol_en    = 0;
    pdc_cfg.cos       = 6;
    pdc_cfg.pol_grp_id = 0;
    
    for(gem_port=0;gem_port<8;gem_port++)
    {
        pdc_cfg.pol_id = gem_port + 0x80;
        if((ret = aal_pdc_map_mem_set(0, gem_port, pdc_mask, &pdc_cfg)) != CA_E_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }

    pdc_ctrl_mask.bf.omci_hp_cos = 1;
    pdc_ctrl_mask.bf.omci_hp_ldpid = 1;
    pdc_ctrl_mask.bf.omci_hp_en = 1;

    pdc_ctrl.omci_hp_cos = 7; 
    pdc_ctrl.omci_hp_ldpid = 0x10;
    

    if((ret = aal_pdc_ctrl_set(0, pdc_ctrl_mask, &pdc_ctrl)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl8277c_trap_omciPduAction_set */


/* Function Name:
 *      dal_rtl8277c_trap_portIgmpMldPktAction_get
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
dal_rtl8277c_trap_portIgmpMldPktAction_get(rt_port_t port, rt_action_t *pAction)
{
    return _dal_rtl8277c_trap_action_get(AAL_L2_SPECPKT_TYPE_IGMP_MLD, port, pAction);
}/* end of dal_rtl8277c_trap_portIgmpMldCtrlPktAction_get */

/* Function Name:
 *      dal_rtl8277c_trap_portIgmpMldPktAction_set
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
dal_rtl8277c_trap_portIgmpMldPktAction_set(rt_port_t port, rt_action_t action)
{
    return _dal_rtl8277c_trap_action_set(AAL_L2_SPECPKT_TYPE_IGMP_MLD, port, action);
}/* end of dal_rtl8277c_trap_portIgmpMldCtrlPktAction_set */


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
dal_rtl8277c_trap_portDhcpPktAction_get(rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_DHCP, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portDhcpPktAction_get */

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
dal_rtl8277c_trap_portDhcpPktAction_set(rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else
    {
        ctrl.spcl_fwd = 0;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_DHCP, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portDhcpPktAction_set */

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
dal_rtl8277c_trap_portPppoePktAction_get(rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_PPPOE_DISC, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portPppoePktAction_get */

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
dal_rtl8277c_trap_portPppoePktAction_set(rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else
    {
        ctrl.spcl_fwd = 0;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_PPPOE_DISC, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portPppoePktAction_set */

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
dal_rtl8277c_trap_portStpPktAction_get(rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_BPDU, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portStpPktAction_get */

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
dal_rtl8277c_trap_portStpPktAction_set(rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else
    {
        ctrl.spcl_fwd = 0;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_BPDU, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portStpPktAction_set */

/* Function Name:
 *      dal_rtl8277c_trap_portHostPktAction_get
 * Description:
 *      Get forwarding action of trapped host packet on specified port.
 * Input:
 *      port - specified port
 * Output:
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
dal_rtl8277c_trap_portHostPktAction_get(rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_MYMAC, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portHostPktAction_get */

/* Function Name:
 *      dal_rtl8277c_trap_portHostPktAction_set
 * Description:
 *      Set forwarding action of trapped host packet on specified port.
 * Input:
 *      action - forwarding action of trapped host packet
 *      port - specified port
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
dal_rtl8277c_trap_portHostPktAction_set(rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else
    {
        ctrl.spcl_fwd = 0;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_MYMAC, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portHostPktAction_set */

/* Function Name:
 *      dal_rtl8277c_trap_portEthertypePktAction_get
 * Description:
 *      Get forwarding action of trapped special ethertype packet on specified port.
 * Input:
 *      port - specified port
 * Output:
 *      pAction - pointer to forwarding action of trapped special ethertype packet
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
dal_rtl8277c_trap_portEthertypePktAction_get(rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_UDF_0, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portEthertypePktAction_get */

/* Function Name:
 *      dal_rtl8277c_trap_portEthertypePktAction_set
 * Description:
 *      Set forwarding action of trapped special ethertype packet on specified port.
 * Input:
 *      port - specified port
 *      action - forwarding action of trapped special ethertype packet
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
dal_rtl8277c_trap_portEthertypePktAction_set(rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else
    {
        ctrl.spcl_fwd = 0;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_UDF_0, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portEthertypePktAction_set */

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
dal_rtl8277c_trap_portArpPktAction_get(rt_port_t port, rt_action_t *pAction)
{
    aal_l2_specpkt_ctrl_t ctrl;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));

    ret = aal_l2_specpkt_ctrl_get(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_ARP, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    if(ctrl.spcl_fwd == 1)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portArpPktAction_get */

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
dal_rtl8277c_trap_portArpPktAction_set(rt_port_t port, rt_action_t action)
{
    aal_l2_specpkt_ctrl_t ctrl;
    aal_l2_specpkt_ctrl_mask_t ctrlMask;
    ca_status_t ret=CA_E_OK;

    if(port > HAL_GET_PON_PORT())
        return RT_ERR_PORT_ID;

    memset(&ctrl,0,sizeof(aal_l2_specpkt_ctrl_t));
    memset(&ctrlMask,0,sizeof(aal_l2_specpkt_ctrl_mask_t));
    ctrlMask.s.spcl_fwd = 1;
    if(action == ACTION_TRAP2CPU)
    {
        ctrlMask.s.ldpid =1;
        ctrl.spcl_fwd = 1;
        ctrl.ldpid = DAL_RTL8277C_TRAP_CPU_LDPID;
    }
    else
    {
        ctrl.spcl_fwd = 0;
    }
    ret = aal_l2_specpkt_ctrl_set(0, rtl8277c_trap_port_spb_idx_map[port], AAL_L2_SPECPKT_TYPE_ARP, ctrlMask, &ctrl);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_portArpPktAction_set */

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
dal_rtl8277c_trap_pattern_get(rt_trap_pattern_type_t type, rt_trap_pattern_t *pattern)
{
    aal_l2_specpkt_detect_t detect;
    ca_status_t ret=CA_E_OK;

    memset(&detect,0,sizeof(aal_l2_specpkt_detect_t));

    ret = aal_l2_specpkt_detect_get(0, &detect);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;

    switch(type)
    {
        case RT_TRAP_PATTERN_TYPE_HOST_MAC:
            memcpy(&(pattern->hostMac),&(detect.my_mac),sizeof(ca_mac_addr_t));
            break;
        case RT_TRAP_PATTERN_TYPE_ETHERTYPE:
            pattern->ethertype = detect.udf0_ethtype;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_pattern_get */

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
dal_rtl8277c_trap_pattern_set(rt_trap_pattern_type_t type, rt_trap_pattern_t pattern)
{
    aal_l2_specpkt_detect_mask_t detectMask;
    aal_l2_specpkt_detect_t detect;
    ca_status_t ret=CA_E_OK;

    memset(&detect,0,sizeof(aal_l2_specpkt_detect_t));
    memset(&detectMask,0,sizeof(aal_l2_specpkt_detect_mask_t));

    switch(type)
    {
        case RT_TRAP_PATTERN_TYPE_HOST_MAC:
            detectMask.s.my_mac=1;
            detectMask.s.my_mac1=1;

            memcpy(&(detect.my_mac),&(pattern.hostMac),sizeof(ca_mac_addr_t));
            memcpy(&(detect.my_mac1),&(pattern.hostMac),sizeof(ca_mac_addr_t));
            break;
        case RT_TRAP_PATTERN_TYPE_ETHERTYPE:
            detectMask.s.udf0_detect_enable = 1;
            detectMask.s.udf0_ethtype_enable = 1;
            detectMask.s.udf0_ethtype = 1;
            detectMask.s.udf0_mac_mask = 1;

            detect.udf0_detect_enable = 1;
            detect.udf0_ethtype_enable = 1;
            memset(&detect.udf0_mac_mask,0,sizeof(ca_mac_addr_t));
            detect.udf0_ethtype = pattern.ethertype;
            break;
        default:
            return RT_ERR_FAILED;
    }

    
    
    ret = aal_l2_specpkt_detect_set(0,  detectMask, &detect);
    if(ret != CA_E_OK)
        return RT_ERR_FAILED;
    
    
    return RT_ERR_OK;
}/* end of dal_rtl8277c_trap_pattern_set */

