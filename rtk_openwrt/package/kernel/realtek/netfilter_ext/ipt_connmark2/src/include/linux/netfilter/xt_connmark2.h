#ifndef _XT_CONNMARK2_H
#define _XT_CONNMARK2_H

#include <linux/types.h>

/* Copyright (C) 2002,2004 MARA Systems AB <http://www.marasystems.com>
 * by Henrik Nordstrom <hno@marasystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

enum {
	XT_CONNMARK2_SET = 0,
	XT_CONNMARK2_SAVE,
	XT_CONNMARK2_RESTORE
};

struct xt_connmark2_tginfo1 {
	__u64 ctmark2, ctmask2, nfmask2;
	__u8 mode;
};

struct xt_connmark2_mtinfo1 {
	__u64 mark2, mask2;
	__u8 invert;
};

#endif /*_XT_CONNMARK2_H*/
