/*
 * Copyright(c) Realtek Semiconductor Corporation, 2018
 * All rights reserved.
 *
 *
 * Purpose : RTK PHY initialize function
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Init function
 *
 */
#ifndef __PHY_INIT_H__
#define __PHY_INIT_H__

#include <common/rt_type.h>
#include <hwp/hw_profile.h>


/* Port related information */
typedef struct phy_hwp_portDescp_s
{
    uint8   mac_id;         /* port id or HWP_END
                             * A { .mac_id = HWP_END }, entry shall be added in the end of the array to indicate no more data of the array.
                             */
    uint8   phy_idx;        /* Indicate which PHY entry in phy_hwp_phyDescp_t is used by this port. */
    uint8   eth;            /* Ethernet speed type (refer to rt_port_ethType_t). */
    uint8   medi;           /* Port media type (refer to rt_port_medium_t). */
    uint8   smi;            /* which set of SMI interface (SMI bus ID)*/
    uint8   phy_addr;       /* PHYAD */
} phy_hwp_portDescp_t;

/* PHY related information */
typedef struct phy_hwp_phyDescp_s
{
    uint32  chip;           /* PHY Chip model (refer to phy_type_t) or HWP_END.
                             * A { .chip = HWP_END, } entry shall be added in the end of the array to indicate no more data of the array.
                             */
    uint8   mac_id;         /* The first port id of this PHY. For example, the RTL8264 is connected to port 0 ~ 4, then the .mac_id  = 0. */
    uint8   phy_max;        /* The MAX port number of this PHY. For examplem the RTL8264 is 4-port PHY, so this number is 4. */
} phy_hwp_phyDescp_t;


/* Information for rtk_init
 * phy_hwp_portDescp_t and phy_hwp_phyDescp_t tables are used to describe your hardware board design, especially for mapping relation between port and phy.
 *      These information is needed for rtk_init.
 *      Example :
 *          phy_hwp_portDescp_t  my_port_descp[] = {
 *                  { .mac_id =  0,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 0, .phy_addr = 0,},
 *                  { .mac_id =  1,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 0, .phy_addr = 1,},
 *                  { .mac_id =  2,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 0, .phy_addr = 2,},
 *                  { .mac_id =  3,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 0, .phy_addr = 3,},
 *                  { .mac_id =  4,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 1, .phy_addr = 4,},
 *                  { .mac_id =  5,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 1, .phy_addr = 5,},
 *                  { .mac_id =  6,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 1, .phy_addr = 6,},
 *                  { .mac_id =  7,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 1, .phy_addr = 7,},
 *                  { .mac_id =  8,  .phy_idx = 2, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 2, .phy_addr = 0,},
 *                  { .mac_id =  9,  .phy_idx = 3, .eth = HWP_XGE,   .medi = HWP_COPPER, .smi = 2, .phy_addr = 1,},
 *                  { .mac_id = HWP_END },
 *              };
 *          phy_hwp_phyDescp_t     my_phy_Descp[] = {
 *                  [0] = { .chip = RTK_PHYTYPE_RTL8264, .mac_id = 0, .phy_max = 4 },
 *                  [1] = { .chip = RTK_PHYTYPE_RTL8264, .mac_id = 4, .phy_max = 4 },
 *                  [2] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = 8, .phy_max = 1 },
 *                  [3] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = 9, .phy_max = 1 },
 *                  [4] = { .chip = HWP_END },
 *              };
 */
typedef struct rtk_phy_initInfo_s
{
    phy_hwp_portDescp_t     *port_desc;
    phy_hwp_phyDescp_t      *phy_desc;
} rtk_phy_initInfo_t;




/* Function Name:
 *      rtk_init
 * Description:
 *      Init RTK
 * Input:
 *      pInitInfo   - information for rtk phy driver init
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtk_init(rtk_phy_initInfo_t *pInitInfo);

#endif /* __PHY_INIT_H__ */

