/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI generic OS define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI generic OS define
 */

#ifndef __OMCI_VOIP_BASIC_H__
#define __OMCI_VOIP_BASIC_H__


/*
*   Macros
*/

// VoIP Debug Messages Level
#define OMCI_VOIP_DBG_LEVEL OMCI_LOG_LEVEL_WARN

#define ODBG_SWITCH
#define P_R(fmt, args...)		printf("\x1B[31m" fmt "\x1B[0m", ## args)
#define P_G(fmt, args...)		printf("\x1B[32m" fmt "\x1B[0m", ## args)
#define P_B(fmt, args...)		printf("\x1B[34m" fmt "\x1B[0m", ## args)
#define P_Y(fmt, args...)		printf("\x1B[33m" fmt "\x1B[0m", ## args)

#ifdef  ODBG_SWITCH
#  define ODBG_R(fmt, args...)  P_R("(%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#  define ODBG_G(fmt, args...)  P_G("(%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#  define ODBG_B(fmt, args...)  P_B("(%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#  define ODBG_Y(fmt, args...)  P_Y("(%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#else
#  define ODBG_R(fmt, args...) /* not debugging: nothing */
#  define ODBG_G(fmt, args...) /* not debugging: nothing */
#  define ODBG_B(fmt, args...) /* not debugging: nothing */
#  define ODBG_Y(fmt, args...) /* not debugging: nothing */
#endif

typedef enum
{
    OMCI_VOIP_JITTER_TARGET,
    OMCI_VOIP_JITTER_MAX,
    OMCI_VOIP_HOOK_TIME_MIN,
    OMCI_VOIP_HOOK_TIME_MAX,
} omci_voip_param_type_t;

#endif
