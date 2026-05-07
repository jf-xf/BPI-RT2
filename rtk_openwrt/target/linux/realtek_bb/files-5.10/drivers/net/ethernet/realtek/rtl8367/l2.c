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
 * Feature : Here is a list of all functions and variables in L2 module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <l2.h>
#include <string.h>

#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtksw_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      Initialize l2 module before calling any l2 APIs.
 */
rtksw_api_ret_t rtksw_l2_init(void)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_init();
    RTKSW_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtksw_l2_addr_add
 * Description:
 *      Add LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      pL2_data - Unicast entry parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry.
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtksw_api_ret_t rtksw_l2_addr_add(rtksw_mac_t *pMac, rtksw_l2_ucastAddr_t *pL2_data)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_addr_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_addr_add(pMac, pL2_data);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac    - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtksw_api_ret_t rtksw_l2_addr_get(rtksw_mac_t *pMac, rtksw_l2_ucastAddr_t *pL2_data)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_addr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_addr_get(pMac, pL2_data);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_addr_next_get
 * Description:
 *      Get Next LUT unicast entry.
 * Input:
 *      read_method     - The reading method.
 *      port            - The port number if the read_metohd is READMETHOD_NEXT_L2UCSPA
 *      pAddress        - The Address ID
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next unicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all entries is LUT.
 */
rtksw_api_ret_t rtksw_l2_addr_next_get(rtksw_l2_read_method_t read_method, rtksw_port_t port, rtksw_uint32 *pAddress, rtksw_l2_ucastAddr_t *pL2_data)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_addr_next_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_addr_next_get(read_method, port, pAddress, pL2_data);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      fid - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtksw_api_ret_t rtksw_l2_addr_del(rtksw_mac_t *pMac, rtksw_l2_ucastAddr_t *pL2_data)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_addr_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_addr_del(pMac, pL2_data);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_VID           - Invalid VID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced,
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtksw_api_ret_t rtksw_l2_mcastAddr_add(rtksw_l2_mcastAddr_t *pMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_mcastAddr_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_mcastAddr_add(pMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      pMcastAddr  - L2 multicast entry structure
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the multicast mac address existed in the LUT, it will return the port where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtksw_api_ret_t rtksw_l2_mcastAddr_get(rtksw_l2_mcastAddr_t *pMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_mcastAddr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_mcastAddr_get(pMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_mcastAddr_next_get
 * Description:
 *      Get Next L2 Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pMcastAddr  - L2 multicast entry structure
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next L2 multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all multicast entries is LUT.
 */
rtksw_api_ret_t rtksw_l2_mcastAddr_next_get(rtksw_uint32 *pAddress, rtksw_l2_mcastAddr_t *pMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_mcastAddr_next_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_mcastAddr_next_get(pAddress, pMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_mcastAddr_del
 * Description:
 *      Delete LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtksw_api_ret_t rtksw_l2_mcastAddr_del(rtksw_l2_mcastAddr_t *pMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_mcastAddr_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_mcastAddr_del(pMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastAddr_add
 * Description:
 *      Add Lut IP multicast entry
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to
 *      forward IP multicast frame directly without flooding.
 */
rtksw_api_ret_t rtksw_l2_ipMcastAddr_add(rtksw_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastAddr_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastAddr_add(pIpMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastAddr_get
 * Description:
 *      Get LUT IP multicast entry.
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      pIpMcastAddr    - IP Multicast entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can get Lut table of IP multicast entry.
 */
rtksw_api_ret_t rtksw_l2_ipMcastAddr_get(rtksw_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastAddr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastAddr_get(pIpMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastAddr_next_get
 * Description:
 *      Get Next IP Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pIpMcastAddr    - IP Multicast entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next IP multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all IP multicast entries is LUT.
 */
rtksw_api_ret_t rtksw_l2_ipMcastAddr_next_get(rtksw_uint32 *pAddress, rtksw_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastAddr_next_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastAddr_next_get(pAddress, pIpMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastAddr_del
 * Description:
 *      Delete a ip multicast address entry from the specified device.
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can delete a IP multicast address entry from the specified device.
 */
rtksw_api_ret_t rtksw_l2_ipMcastAddr_del(rtksw_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastAddr_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastAddr_del(pIpMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipVidMcastAddr_add
 * Description:
 *      Add Lut IP multicast+VID entry
 * Input:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_ipVidMcastAddr_add(rtksw_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipVidMcastAddr_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipVidMcastAddr_add(pIpVidMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipVidMcastAddr_get
 * Description:
 *      Get LUT IP multicast+VID entry.
 * Input:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Output:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_ipVidMcastAddr_get(rtksw_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipVidMcastAddr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipVidMcastAddr_get(pIpVidMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipVidMcastAddr_next_get
 * Description:
 *      Get Next IP Multicast+VID entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next IP multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all IP multicast entries is LUT.
 */
rtksw_api_ret_t rtksw_l2_ipVidMcastAddr_next_get(rtksw_uint32 *pAddress, rtksw_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipVidMcastAddr_next_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipVidMcastAddr_next_get(pAddress, pIpVidMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipVidMcastAddr_del
 * Description:
 *      Delete a ip multicast+VID address entry from the specified device.
 * Input:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_ipVidMcastAddr_del(rtksw_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipVidMcastAddr_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipVidMcastAddr_del(pIpVidMcastAddr);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ucastAddr_flush
 * Description:
 *      Flush L2 mac address by type in the specified device (both dynamic and static).
 * Input:
 *      pConfig - flush configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      flushByVid          - 1: Flush by VID, 0: Don't flush by VID
 *      vid                 - VID (0 ~ 4095)
 *      flushByFid          - 1: Flush by FID, 0: Don't flush by FID
 *      fid                 - FID (0 ~ 15)
 *      flushByPort         - 1: Flush by Port, 0: Don't flush by Port
 *      port                - Port ID
 *      flushByMac          - Not Supported
 *      ucastAddr           - Not Supported
 *      flushStaticAddr     - 1: Flush both Static and Dynamic entries, 0: Flush only Dynamic entries
 *      flushAddrOnAllPorts - 1: Flush VID-matched entries at all ports, 0: Flush VID-matched entries per port.
 */
rtksw_api_ret_t rtksw_l2_ucastAddr_flush(rtksw_l2_flushCfg_t *pConfig)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ucastAddr_flush)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ucastAddr_flush(pConfig);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_table_clear
 * Description:
 *      Flush all static & dynamic entries in LUT.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_table_clear(void)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_table_clear)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_table_clear();
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_table_clearStatus_get
 * Description:
 *      Get table clear status
 * Input:
 *      None
 * Output:
 *      pStatus - Clear status, 1:Busy, 0:finish
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_table_clearStatus_get(rtksw_l2_clearStatus_t *pStatus)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_table_clearStatus_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_table_clearStatus_get(pStatus);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
 *      enable - link down flush status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - RTKSW_DISABLED
 *      - RTKSW_ENABLED
 */
rtksw_api_ret_t rtksw_l2_flushLinkDownPortAddrEnable_set(rtksw_port_t port, rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_flushLinkDownPortAddrEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_flushLinkDownPortAddrEnable_set(port, enable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - link down flush status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - RTKSW_DISABLED
 *      - RTKSW_ENABLED
 */
rtksw_api_ret_t rtksw_l2_flushLinkDownPortAddrEnable_get(rtksw_port_t port, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_flushLinkDownPortAddrEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_flushLinkDownPortAddrEnable_get(port, pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port    - Port id.
 *      enable  - Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtksw_api_ret_t rtksw_l2_agingEnable_set(rtksw_port_t port, rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_agingEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_agingEnable_set(port, enable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can be used to get L2 LUT aging function per port.
 */
rtksw_api_ret_t rtksw_l2_agingEnable_get(rtksw_port_t port, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_agingEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_agingEnable_get(port, pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port    - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning)
 *      to 2112.
 */
rtksw_api_ret_t rtksw_l2_limitLearningCnt_set(rtksw_port_t port, rtksw_mac_cnt_t mac_cnt)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitLearningCnt_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitLearningCnt_set(port, mac_cnt);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
rtksw_api_ret_t rtksw_l2_limitLearningCnt_get(rtksw_port_t port, rtksw_mac_cnt_t *pMac_cnt)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitLearningCnt_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitLearningCnt_get(port, pMac_cnt);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitSystemLearningCnt_set
 * Description:
 *      Set System auto learning limit number
 * Input:
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set system ASIC auto learning limit number from 0(disable learning)
 *      to 2112.
 */
rtksw_api_ret_t rtksw_l2_limitSystemLearningCnt_set(rtksw_mac_cnt_t mac_cnt)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitSystemLearningCnt_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitSystemLearningCnt_set(mac_cnt);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitSystemLearningCnt_get
 * Description:
 *      Get System auto learning limit number
 * Input:
 *      None
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get system ASIC auto learning limit number.
 */
rtksw_api_ret_t rtksw_l2_limitSystemLearningCnt_get(rtksw_mac_cnt_t *pMac_cnt)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitSystemLearningCnt_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitSystemLearningCnt_get(pMac_cnt);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtksw_api_ret_t rtksw_l2_limitLearningCntAction_set(rtksw_port_t port, rtksw_l2_limitLearnCntAction_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitLearningCntAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitLearningCntAction_set(port, action);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtksw_api_ret_t rtksw_l2_limitLearningCntAction_get(rtksw_port_t port, rtksw_l2_limitLearnCntAction_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitLearningCntAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitLearningCntAction_get(port, pAction);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitSystemLearningCntAction_set
 * Description:
 *      Configure system auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtksw_api_ret_t rtksw_l2_limitSystemLearningCntAction_set(rtksw_l2_limitLearnCntAction_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitSystemLearningCntAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitSystemLearningCntAction_set(action);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitSystemLearningCntAction_get
 * Description:
 *      Get system auto learn over limit number action.
 * Input:
 *      None.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtksw_api_ret_t rtksw_l2_limitSystemLearningCntAction_get(rtksw_l2_limitLearnCntAction_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitSystemLearningCntAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitSystemLearningCntAction_get(pAction);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitSystemLearningCntPortMask_set
 * Description:
 *      Configure system auto learn portmask
 * Input:
 *      pPortmask - Port Mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask.
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_limitSystemLearningCntPortMask_set(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitSystemLearningCntPortMask_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitSystemLearningCntPortMask_set(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_limitSystemLearningCntPortMask_get
 * Description:
 *      get system auto learn portmask
 * Input:
 *      None
 * Output:
 *      pPortmask - Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null pointer.
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_limitSystemLearningCntPortMask_get(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_limitSystemLearningCntPortMask_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_limitSystemLearningCntPortMask_get(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
rtksw_api_ret_t rtksw_l2_learningCnt_get(rtksw_port_t port, rtksw_mac_cnt_t *pMac_cnt)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_learningCnt_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_learningCnt_get(port, pMac_cnt);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_floodPortMask_set
 * Description:
 *      Set flooding portmask
 * Input:
 *      type - flooding type.
 *      pFlood_portmask - flooding porkmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set the flooding mask.
 *      The flooding type is as following:
 *      - FLOOD_UNKNOWNDA
 *      - FLOOD_UNKNOWNMC
 *      - FLOOD_BC
 */
rtksw_api_ret_t rtksw_l2_floodPortMask_set(rtksw_l2_flood_type_t floood_type, rtksw_portmask_t *pFlood_portmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_floodPortMask_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_floodPortMask_set(floood_type, pFlood_portmask);
    RTKSW_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtksw_l2_floodPortMask_get
 * Description:
 *      Get flooding portmask
 * Input:
 *      type - flooding type.
 * Output:
 *      pFlood_portmask - flooding porkmask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the flooding mask.
 *      The flooding type is as following:
 *      - FLOOD_UNKNOWNDA
 *      - FLOOD_UNKNOWNMC
 *      - FLOOD_BC
 */
rtksw_api_ret_t rtksw_l2_floodPortMask_get(rtksw_l2_flood_type_t floood_type, rtksw_portmask_t *pFlood_portmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_floodPortMask_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_floodPortMask_get(floood_type, pFlood_portmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_localPktPermit_set
 * Description:
 *      Set permittion of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 *      permit - permittion status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid permit value.
 * Note:
 *      This API is setted to permit frame if its source port is equal to destination port.
 */
rtksw_api_ret_t rtksw_l2_localPktPermit_set(rtksw_port_t port, rtksw_enable_t permit)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_localPktPermit_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_localPktPermit_set(port, permit);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_localPktPermit_get
 * Description:
 *      Get permittion of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 * Output:
 *      pPermit - permittion status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is to get permittion status for frames if its source port is equal to destination port.
 */
rtksw_api_ret_t rtksw_l2_localPktPermit_get(rtksw_port_t port, rtksw_enable_t *pPermit)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_localPktPermit_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_localPktPermit_get(port, pPermit);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_aging_set
 * Description:
 *      Set LUT agging out speed
 * Input:
 *      aging_time - Agging out time.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can set LUT agging out period for each entry and the range is from 45s to 458s.
 */
rtksw_api_ret_t rtksw_l2_aging_set(rtksw_l2_age_time_t aging_time)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_aging_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_aging_set(aging_time);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_aging_get
 * Description:
 *      Get LUT agging out time
 * Input:
 *      None
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get LUT agging out period for each entry.
 */
rtksw_api_ret_t rtksw_l2_aging_get(rtksw_l2_age_time_t *pAging_time)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_aging_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_aging_get(pAging_time);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastAddrLookup_set
 * Description:
 *      Set Lut IP multicast lookup function
 * Input:
 *      type - Lookup type for IPMC packet.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      LOOKUP_MAC      - Lookup by MAC address
 *      LOOKUP_IP       - Lookup by IP address
 *      LOOKUP_IP_VID   - Lookup by IP address & VLAN ID
 */
rtksw_api_ret_t rtksw_l2_ipMcastAddrLookup_set(rtksw_l2_ipmc_lookup_type_t type)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastAddrLookup_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastAddrLookup_set(type);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastAddrLookup_get
 * Description:
 *      Get Lut IP multicast lookup function
 * Input:
 *      None.
 * Output:
 *      pType - Lookup type for IPMC packet.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      None.
 */
rtksw_api_ret_t rtksw_l2_ipMcastAddrLookup_get(rtksw_l2_ipmc_lookup_type_t *pType)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastAddrLookup_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastAddrLookup_get(pType);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastForwardRouterPort_set
 * Description:
 *      Set IPMC packet forward to rounter port also or not
 * Input:
 *      enabled - 1: Inlcude router port, 0, exclude router port
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_ipMcastForwardRouterPort_set(rtksw_enable_t enabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastForwardRouterPort_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastForwardRouterPort_set(enabled);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastForwardRouterPort_get
 * Description:
 *      Get IPMC packet forward to rounter port also or not
 * Input:
 *      None.
 * Output:
 *      pEnabled    - 1: Inlcude router port, 0, exclude router port
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t rtksw_l2_ipMcastForwardRouterPort_get(rtksw_enable_t *pEnabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastForwardRouterPort_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastForwardRouterPort_get(pEnabled);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastGroupEntry_add
 * Description:
 *      Add an IP Multicast entry to group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 *      pPortmask   - portmask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Add an entry to IP Multicast Group table.
 */
rtksw_api_ret_t rtksw_l2_ipMcastGroupEntry_add(ipaddr_t ip_addr, rtksw_uint32 vid, rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastGroupEntry_add)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastGroupEntry_add(ip_addr, vid, pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastGroupEntry_del
 * Description:
 *      Delete an entry from IP Multicast group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry from IP Multicast group table.
 */
rtksw_api_ret_t rtksw_l2_ipMcastGroupEntry_del(ipaddr_t ip_addr, rtksw_uint32 vid)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastGroupEntry_del)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastGroupEntry_del(ip_addr, vid);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_ipMcastGroupEntry_get
 * Description:
 *      get an entry from IP Multicast group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 * Output:
 *      pPortmask   - member port mask
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry from IP Multicast group table.
 */
rtksw_api_ret_t rtksw_l2_ipMcastGroupEntry_get(ipaddr_t ip_addr, rtksw_uint32 vid, rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_ipMcastGroupEntry_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_ipMcastGroupEntry_get(ip_addr, vid, pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_entry_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~2111.
 */
rtksw_api_ret_t rtksw_l2_entry_get(rtksw_l2_addr_table_t *pL2_entry)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_entry_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_entry_get(pL2_entry);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_lookupHitIsolationAction_set
 * Description:
 *      Set action of lookup hit & isolation.
 * Input:
 *      action          - The action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      This API is used to configure the action of packet which is lookup hit
 *      in L2 table but the destination port/portmask are not in the port isolation
 *      group.
 */
rtksw_api_ret_t rtksw_l2_lookupHitIsolationAction_set(rtksw_l2_lookupHitIsolationAction_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_lookupHitIsolationAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_lookupHitIsolationAction_set(action);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_l2_lookupHitIsolationAction_get
 * Description:
 *      Get action of lookup hit & isolation.
 * Input:
 *      None.
 * Output:
 *      pAction         - The action
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_NULL_POINTER         - Null pointer
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      This API is used to get the action of packet which is lookup hit
 *      in L2 table but the destination port/portmask are not in the port isolation
 *      group.
 */
rtksw_api_ret_t rtksw_l2_lookupHitIsolationAction_get(rtksw_l2_lookupHitIsolationAction_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->l2_lookupHitIsolationAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->l2_lookupHitIsolationAction_get(pAction);
    RTKSW_API_UNLOCK();

    return retVal;
}
