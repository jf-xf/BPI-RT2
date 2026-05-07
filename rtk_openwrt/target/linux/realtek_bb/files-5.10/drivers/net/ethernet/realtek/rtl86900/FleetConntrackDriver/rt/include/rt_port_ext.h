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
 * Purpose : Definition of Port extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Parameter definition for the port-based view
 *
 */


#ifndef __RT_PORT_EXT_H__
#define __RT_PORT_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>


/*
 * Symbol Definition
 */

#if !defined(RT_PORT_PHY_PORT_PON)
/* For Physical Port Mask */
typedef enum rt_port_phy_port_mask_e
{
        RT_PORT_PHY_PORT_PORT0_BIT = (0x1<<RT_PORT0),
        RT_PORT_PHY_PORT_PORT1_BIT = (0x1<<RT_PORT1),
        RT_PORT_PHY_PORT_PORT2_BIT = (0x1<<RT_PORT2),
        RT_PORT_PHY_PORT_PORT3_BIT = (0x1<<RT_PORT3),
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_RTL8198F_SERIES)
        RT_PORT_PHY_PORT_PORT4_BIT = (0x1<<RT_PORT4),
#endif
        RT_PORT_PHY_PORT_PON_BIT = (0x1<<RT_PORT_PON),
} rt_port_phy_port_mask_t;

typedef enum rt_port_phy_port_inedx_e
{
        RT_PORT_PHY_PORT_PORT0 = (RT_PORT0),
        RT_PORT_PHY_PORT_PORT1 = (RT_PORT1),
        RT_PORT_PHY_PORT_PORT2 = (RT_PORT2),
        RT_PORT_PHY_PORT_PORT3 = (RT_PORT3),
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_RTL8198F_SERIES)
        RT_PORT_PHY_PORT_PORT4 = (RT_PORT4),
#endif
        RT_PORT_PHY_PORT_PON = (RT_PORT_PON),
} rt_port_phy_port_index_t;
#endif

#endif /* __RT_PORT_EXT_H__ */
