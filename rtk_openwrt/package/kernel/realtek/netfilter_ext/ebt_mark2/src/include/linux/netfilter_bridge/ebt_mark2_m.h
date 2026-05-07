#ifndef __LINUX_BRIDGE_EBT_MARK2_M_H
#define __LINUX_BRIDGE_EBT_MARK2_M_H

#include <linux/types.h>

#define EBT_MARK2_AND 0x01
#define EBT_MARK2_OR 0x02
#define EBT_MARK2_MASK (EBT_MARK2_AND | EBT_MARK2_OR)
struct ebt_mark2_m_info {
	unsigned long long mark2, mask2;
	__u8 invert;
	__u8 bitmask;
};
#define EBT_MARK2_MATCH "mark2_m"

#endif
