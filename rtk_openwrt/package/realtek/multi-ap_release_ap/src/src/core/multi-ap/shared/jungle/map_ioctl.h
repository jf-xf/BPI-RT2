/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_IOCTL_H_
#define _MAP_IOCTL_H_

#ifdef _BACKPORTS_DRIVER_

#define RTL8192CD_IOCTL_UPDATE_BSS 0x8B4A

#else // _BACKPORTS_DRIVER_

#include "apmib.h"

#ifdef _DRIVER_COMMON_STRUCTURE_
#define RTL8192CD_IOCTL_UPDATE_BSS 0x9008
#else
#define RTL8192CD_IOCTL_UPDATE_BSS 0x8BDD
#endif

#endif //_BACKPORTS_DRIVER_

#endif