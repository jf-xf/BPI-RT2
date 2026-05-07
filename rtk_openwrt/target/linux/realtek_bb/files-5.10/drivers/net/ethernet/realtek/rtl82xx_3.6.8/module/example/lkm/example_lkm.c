/*
 * Copyright (C) 2009-2015 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 71708 $
 * $Date: 2016-09-19 11:31:17 +0800 (Mon, 19 Sep 2016) $
 *
 * Purpose : Create stacking linux kernel module
 *
 */

/*
 * Include Files
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>


#include <example/src/test_phyApi.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
static int __init example_lkm_init(void)
{

    printk("A LKM Example, and could use to test other lkm\n");

    call_all_phy_api();

    return 0;
}

static void __exit example_lkm_exit(void)
{
    printk("Exit RTK Stacking Protocol....OK\n");
}

module_init(example_lkm_init);
module_exit(example_lkm_exit);

MODULE_DESCRIPTION("module lkm example");
MODULE_LICENSE("GPL");





