/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _PLATFORM_IOCTL_H_
#define _PLATFORM_IOCTL_H_

#ifdef CONFIG_RTK_PON_XDSL
#include "wifi_common.h"
#else
//#include "mib.h"
#endif

#define SIOCGIWRTLGETBSSINFO 0x8B37
#define SIOCGIWRTLSTAINFO 0x8B30

#define RTL8192CD_IOCTL_UPDATE_BSS 0x8B4A
#define RTL8192CD_IOCTL_SEND_DISASSOC 0x8B4B
#define SIOCMAP_UPDATEPOLICY 0x8B4E

#define SIOCMAP_CHANNELSCAN                            0x8B58
#define SIOCMAP_CAC                                    0x8B72

#endif