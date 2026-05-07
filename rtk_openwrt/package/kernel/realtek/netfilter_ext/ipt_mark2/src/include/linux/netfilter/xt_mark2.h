#ifndef _XT_MARK2_H
#define _XT_MARK2_H

#include <linux/types.h>

struct xt_mark2_tginfo2 {
	__u64 mark2, mask2;
};

struct xt_mark2_mtinfo1 {
	__u64 mark2, mask2;
	__u8 invert;
};

#endif /*_XT_MARK2_H*/
