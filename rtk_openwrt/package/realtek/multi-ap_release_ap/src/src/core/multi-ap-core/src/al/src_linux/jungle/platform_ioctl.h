/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _PLATFORM_IOCTL_H_
#define _PLATFORM_IOCTL_H_

#ifdef _BACKPORTS_DRIVER_

#define SIOCGIWRTLGETBSSINFO 0x8B37
#define SIOCGIWRTLSTAINFO 0x8B30

#define RTL8192CD_IOCTL_UPDATE_BSS 0x8B4A
#define RTL8192CD_IOCTL_SEND_DISASSOC 0x8B4B
#define SIOCMAP_UPDATEPOLICY 0x8B4E

#define SIOCMAP_CHANNELSCAN                            0x8B58
#define SIOCMAP_CAC                                    0x8B72

#else // _BACKPORTS_DRIVER_

#include "apmib.h"

#ifdef _DRIVER_COMMON_STRUCTURE_
#define RTL8192CD_IOCTL_UPDATE_BSS 0x9008
#define RTL8192CD_IOCTL_SEND_DISASSOC 0x8B59
#define SIOCMAP_UPDATEPOLICY 0x8B58
#else
#define RTL8192CD_IOCTL_UPDATE_BSS 0x8BDD
#define RTL8192CD_IOCTL_SEND_DISASSOC 0x8BC2
#define SIOCMAP_UPDATEPOLICY 0x8BFC
#endif

#define SIOCMAP_CHANNELSCAN                            0x8B4A
#define SIOCMAP_CAC                                    0x8BCA

#endif // _BACKPORTS_DRIVER_

#endif