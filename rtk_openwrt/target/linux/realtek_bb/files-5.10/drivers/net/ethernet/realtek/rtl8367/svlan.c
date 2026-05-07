/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in SVLAN module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <svlan.h>
#include <vlan.h>
#include <string.h>

#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtksw_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
rtksw_api_ret_t rtksw_svlan_init(void)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_init();
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_servicePort_add
 * Description:
 *      Add one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtksw_api_ret_t rtksw_svlan_servicePort_add(rtksw_port_t port)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_servicePort_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_servicePort_add(port);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_servicePort_get
 * Description:
 *      Get service ports in the specified device.
 * Input:
 *      None
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtksw_api_ret_t rtksw_svlan_servicePort_get(rtksw_portmask_t *pSvlan_portmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_servicePort_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_servicePort_get(pSvlan_portmask);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_servicePort_del
 * Description:
 *      Delete one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is removing SVLAN service port in the specified device.
 */
rtksw_api_ret_t rtksw_svlan_servicePort_del(rtksw_port_t port)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_servicePort_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_servicePort_del(port);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type.
 * Input:
 *      svlan_tag_id - Ether type of S-tag frame parsing in uplink ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
rtksw_api_ret_t rtksw_svlan_tpidEntry_set(rtksw_svlan_tpid_t svlan_tag_id)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_tpidEntry_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_tpidEntry_set(svlan_tag_id);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type setting.
 * Input:
 *      None
 * Output:
 *      pSvlan_tag_id -  Ether type of S-tag frame parsing in uplink ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtksw_api_ret_t rtksw_svlan_tpidEntry_get(rtksw_svlan_tpid_t *pSvlan_tag_id)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_tpidEntry_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_tpidEntry_get(pSvlan_tag_id);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_priorityRef_set
 * Description:
 *      Set S-VLAN upstream priority reference setting.
 * Input:
 *      ref - reference selection parameter.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The API can set the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI,
 *      - REF_PB_PRI.
 */
rtksw_api_ret_t rtksw_svlan_priorityRef_set(rtksw_svlan_pri_ref_t ref)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_priorityRef_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_priorityRef_set(ref);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_priorityRef_get
 * Description:
 *      Get S-VLAN upstream priority reference setting.
 * Input:
 *      None
 * Output:
 *      pRef - reference selection parameter.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI,
 *      - REF_PB_PRI
 */
rtksw_api_ret_t rtksw_svlan_priorityRef_get(rtksw_svlan_pri_ref_t *pRef)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_priorityRef_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_priorityRef_get(pRef);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_memberPortEntry_set
 * Description:
 *      Configure system SVLAN member content
 * Input:
 *      svid - SVLAN id
 *      psvlan_cfg - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped by default setup.
 *      - rtksw_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtksw_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtksw_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtksw_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
rtksw_api_ret_t rtksw_svlan_memberPortEntry_set(rtksw_vlan_t svid, rtksw_svlan_memberCfg_t *pSvlan_cfg)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_memberPortEntry_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_memberPortEntry_set(svid, pSvlan_cfg);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_memberPortEntry_get
 * Description:
 *      Get SVLAN member Configure.
 * Input:
 *      svid - SVLAN id
 * Output:
 *      pSvlan_cfg - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
rtksw_api_ret_t rtksw_svlan_memberPortEntry_get(rtksw_vlan_t svid, rtksw_svlan_memberCfg_t *pSvlan_cfg)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_memberPortEntry_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_memberPortEntry_get(svid, pSvlan_cfg);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_memberPortEntry_adv_set
 * Description:
 *      Configure system SVLAN member by index
 * Input:
 *      idx         - Index (0 ~ 63)
 *      psvlan_cfg  - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format by index.
 *      - rtksw_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtksw_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtksw_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtksw_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
rtksw_api_ret_t rtksw_svlan_memberPortEntry_adv_set(rtksw_uint32 idx, rtksw_svlan_memberCfg_t *pSvlan_cfg)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_memberPortEntry_adv_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_memberPortEntry_adv_set(idx, pSvlan_cfg);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_memberPortEntry_adv_get
 * Description:
 *      Get SVLAN member Configure by index.
 * Input:
 *      idx         - Index (0 ~ 63)
 * Output:
 *      pSvlan_cfg  - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
rtksw_api_ret_t rtksw_svlan_memberPortEntry_adv_get(rtksw_uint32 idx, rtksw_svlan_memberCfg_t *pSvlan_cfg)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_memberPortEntry_adv_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_memberPortEntry_adv_get(idx, pSvlan_cfg);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_defaultSvlan_set
 * Description:
 *      Configure default egress SVLAN.
 * Input:
 *      port - Source port
 *      svid - SVLAN id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_INPUT                    - Invalid input parameter.
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 * Note:
 *      The API can set port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
rtksw_api_ret_t rtksw_svlan_defaultSvlan_set(rtksw_port_t port, rtksw_vlan_t svid)
{

    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_defaultSvlan_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_defaultSvlan_set(port, svid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_defaultSvlan_get
 * Description:
 *      Get the configure default egress SVLAN.
 * Input:
 *      port - Source port
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
rtksw_api_ret_t rtksw_svlan_defaultSvlan_get(rtksw_port_t port, rtksw_vlan_t *pSvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_defaultSvlan_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_defaultSvlan_get(port, pSvid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_c2s_add
 * Description:
 *      Configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set system C2S configuration. ASIC will check upstream's VID and assign related
 *      SVID to mathed packet. There are 128 SVLAN C2S configurations.
 */
rtksw_api_ret_t rtksw_svlan_c2s_add(rtksw_vlan_t vid, rtksw_port_t src_port, rtksw_vlan_t svid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_c2s_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_c2s_add(vid, src_port, svid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_c2s_del
 * Description:
 *      Delete one C2S entry
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete system C2S configuration. There are 128 SVLAN C2S configurations.
 */
rtksw_api_ret_t rtksw_svlan_c2s_del(rtksw_vlan_t vid, rtksw_port_t src_port)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_c2s_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_c2s_del(vid, src_port);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_c2s_get
 * Description:
 *      Get configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 * Output:
 *      pSvid - SVLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *     The API can get system C2S configuration. There are 128 SVLAN C2S configurations.
 */
rtksw_api_ret_t rtksw_svlan_c2s_get(rtksw_vlan_t vid, rtksw_port_t src_port, rtksw_vlan_t *pSvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_c2s_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_c2s_get(vid, src_port, pSvid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_untag_action_set
 * Description:
 *      Configure Action of downstream UnStag packet
 * Input:
 *      action  - Action for UnStag
 *      svid    - The SVID assigned to UnStag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also supported by this API. The parameter of svid is
 *      only referenced when the action is set to UNTAG_ASSIGN
 */
rtksw_api_ret_t rtksw_svlan_untag_action_set(rtksw_svlan_untag_action_t action, rtksw_vlan_t svid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_untag_action_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_untag_action_set(action, svid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_untag_action_get
 * Description:
 *      Get Action of downstream UnStag packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for UnStag
 *      pSvid    - The SVID assigned to UnStag packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNTAG_ASSIGN
 */
rtksw_api_ret_t rtksw_svlan_untag_action_get(rtksw_svlan_untag_action_t *pAction, rtksw_vlan_t *pSvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_untag_action_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_untag_action_get(pAction, pSvid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_unmatch_action_set
 * Description:
 *      Configure Action of downstream Unmatch packet
 * Input:
 *      action  - Action for Unmatch
 *      svid    - The SVID assigned to Unmatch packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also supported by this API. The parameter od svid is
 *      only refernced when the action is set to UNMATCH_ASSIGN
 */
rtksw_api_ret_t rtksw_svlan_unmatch_action_set(rtksw_svlan_unmatch_action_t action, rtksw_vlan_t svid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_unmatch_action_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_unmatch_action_set(action, svid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_unmatch_action_get
 * Description:
 *      Get Action of downstream Unmatch packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Unmatch
 *      pSvid    - The SVID assigned to Unmatch packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNMATCH_ASSIGN
 */
rtksw_api_ret_t rtksw_svlan_unmatch_action_get(rtksw_svlan_unmatch_action_t *pAction, rtksw_vlan_t *pSvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_unmatch_action_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_unmatch_action_get(pAction, pSvid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_unassign_action_set
 * Description:
 *      Configure Action of upstream without svid assign action
 * Input:
 *      action  - Action for Un-assign
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of upstream Un-assign svid packet. If action is not
 *      trap to CPU, the port-based SVID sure be assign as system need
 */
rtksw_api_ret_t rtksw_svlan_unassign_action_set(rtksw_svlan_unassign_action_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_unassign_action_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_unassign_action_set(action);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_unassign_action_get
 * Description:
 *      Get action of upstream without svid assignment
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Un-assign
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_svlan_unassign_action_get(rtksw_svlan_unassign_action_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_unassign_action_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_unassign_action_get(pAction);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_dmac_vidsel_set
 * Description:
 *      Set DMAC CVID selection
 * Input:
 *      port    - Port
 *      enable  - state of DMAC CVID Selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can set DMAC CVID Selection state
 */
rtksw_api_ret_t rtksw_svlan_dmac_vidsel_set(rtksw_port_t port, rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_dmac_vidsel_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_dmac_vidsel_set(port, enable);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_dmac_vidsel_get
 * Description:
 *      Get DMAC CVID selection
 * Input:
 *      port    - Port
 * Output:
 *      pEnable - state of DMAC CVID Selection
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can get DMAC CVID Selection state
 */
rtksw_api_ret_t rtksw_svlan_dmac_vidsel_get(rtksw_port_t port, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_dmac_vidsel_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_dmac_vidsel_get(port, pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_ipmc2s_add
 * Description:
 *      add ip multicast address to SVLAN
 * Input:
 *      svid    - SVLAN VID
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can set IP mutlicast to SVID configuration. If upstream packet is IPv4 multicast
 *      packet and DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.
 *      There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtksw_api_ret_t rtksw_svlan_ipmc2s_add(ipaddr_t ipmc, ipaddr_t ipmcMsk,rtksw_vlan_t svid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_ipmc2s_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_ipmc2s_add(ipmc, ipmcMsk, svid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_ipmc2s_del
 * Description:
 *      delete ip multicast address to SVLAN
 * Input:
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtksw_api_ret_t rtksw_svlan_ipmc2s_del(ipaddr_t ipmc, ipaddr_t ipmcMsk)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_ipmc2s_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_ipmc2s_del(ipmc, ipmcMsk);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_ipmc2s_get
 * Description:
 *      Get ip multicast address to SVLAN
 * Input:
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can get IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtksw_api_ret_t rtksw_svlan_ipmc2s_get(ipaddr_t ipmc, ipaddr_t ipmcMsk, rtksw_vlan_t *pSvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_ipmc2s_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_ipmc2s_get(ipmc, ipmcMsk, pSvid);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_svlan_l2mc2s_add
 * Description:
 *      Add L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 *      svid    - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can set L2 Mutlicast to SVID configuration. If upstream packet is L2 multicast
 *      packet and DMAC is matched, ASIC will assign egress SVID to the packet. There are 32
 *      SVLAN multicast configurations for IP and L2 multicast.
 */
rtksw_api_ret_t rtksw_svlan_l2mc2s_add(rtksw_mac_t mac, rtksw_mac_t macMsk, rtksw_vlan_t svid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_l2mc2s_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_l2mc2s_add(mac, macMsk, svid);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_l2mc2s_del
 * Description:
 *      delete L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete Mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtksw_api_ret_t rtksw_svlan_l2mc2s_del(rtksw_mac_t mac, rtksw_mac_t macMsk)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_l2mc2s_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_l2mc2s_del(mac, macMsk);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_l2mc2s_get
 * Description:
 *      Get L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can get L2 mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtksw_api_ret_t rtksw_svlan_l2mc2s_get(rtksw_mac_t mac, rtksw_mac_t macMsk, rtksw_vlan_t *pSvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_l2mc2s_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_l2mc2s_get(mac, macMsk, pSvid);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_sp2c_add
 * Description:
 *      Add system SP2C configuration
 * Input:
 *      cvid        - VLAN ID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 *      svid        - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can add SVID & Destination Port to CVLAN configuration. The downstream frames with assigned
 *      SVID will be add C-tag with assigned CVID if the output port is the assigned destination port.
 *      There are 128 SP2C configurations.
 */
rtksw_api_ret_t rtksw_svlan_sp2c_add(rtksw_vlan_t svid, rtksw_port_t dst_port, rtksw_vlan_t cvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_sp2c_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_sp2c_add(svid, dst_port, cvid);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_sp2c_get
 * Description:
 *      Get configure system SP2C content
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      pCvid - VLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 * Note:
 *     The API can get SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
rtksw_api_ret_t rtksw_svlan_sp2c_get(rtksw_vlan_t svid, rtksw_port_t dst_port, rtksw_vlan_t *pCvid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_sp2c_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_sp2c_get(svid, dst_port, pCvid);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_sp2c_del
 * Description:
 *      Delete system SP2C configuration
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
rtksw_api_ret_t rtksw_svlan_sp2c_del(rtksw_vlan_t svid, rtksw_port_t dst_port)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_sp2c_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_sp2c_del(svid, dst_port);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_lookupType_set
 * Description:
 *      Set lookup type of SVLAN
 * Input:
 *      type        - lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      none
 */
rtksw_api_ret_t rtksw_svlan_lookupType_set(rtksw_svlan_lookupType_t type)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_lookupType_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_lookupType_set(type);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_lookupType_get
 * Description:
 *      Get lookup type of SVLAN
 * Input:
 *      pType       - lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      none
 */
rtksw_api_ret_t rtksw_svlan_lookupType_get(rtksw_svlan_lookupType_t *pType)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_lookupType_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_lookupType_get(pType);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_svlan_trapPri_set
 * Description:
 *      Set svlan trap priority
 * Input:
 *      priority - priority for trap packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_INT_PRIORITY
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_svlan_trapPri_set(rtksw_pri_t priority)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_trapPri_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_trapPri_set(priority);
    RTKSW_API_UNLOCK();

    return retVal;
}   /* end of rtksw_svlan_trapPri_set */

/* Function Name:
 *      rtksw_svlan_trapPri_get
 * Description:
 *      Get svlan trap priority
 * Input:
 *      None
 * Output:
 *      pPriority - priority for trap packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_svlan_trapPri_get(rtksw_pri_t *pPriority)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->svlan_trapPri_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->svlan_trapPri_get(pPriority);
    RTKSW_API_UNLOCK();

    return retVal;
}   /* end of rtksw_svlan_trapPri_get */


