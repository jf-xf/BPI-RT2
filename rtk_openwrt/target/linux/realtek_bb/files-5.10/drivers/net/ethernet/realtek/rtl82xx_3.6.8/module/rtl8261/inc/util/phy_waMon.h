/*
 * Copyright (C) 2009-2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Revision:
 * Date: 2018-06-15
 *
 * Purpose : PHY waMon module.
 *
 * Feature : This Module is used to monitor PHY status.
 *
 */

#ifndef __PHY_WAMON_H__
#define __PHY_WAMON_H__

extern int32 dal_waMon_init(void);

/* Function Name:
 *      dal_waMon_enable
 * Description:
 *      Enable workaround monitor thread
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Note:
 *      When enable workaround monitor thread
 *
 */
extern int32 dal_waMon_enable(void);

#endif /* __PHY_WAMON_H__ */

