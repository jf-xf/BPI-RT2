/*
 *	xt_mark2 - Netfilter module to match NFMARK value
 *
 *	(C) 1999-2001 Marc Boucher <marc@mbsi.ca>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_mark2.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Boucher <marc@mbsi.ca>");
MODULE_DESCRIPTION("Xtables: packet mark2 operations");
MODULE_ALIAS("ipt_mark2");
MODULE_ALIAS("ip6t_mark2");
MODULE_ALIAS("ipt_MARK2");
MODULE_ALIAS("ip6t_MARK2");
MODULE_ALIAS("arpt_MARK2");

static unsigned int
mark2_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark2_tginfo2 *info = par->targinfo;

	skb->mark2 = (skb->mark2 & ~info->mask2) ^ info->mark2;
	return XT_CONTINUE;
}

static bool
mark2_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_mark2_mtinfo1 *info = par->matchinfo;

	return ((skb->mark2 & info->mask2) == info->mark2) ^ info->invert;
}

static struct xt_target mark2_tg_reg __read_mostly = {
	.name           = "MARK2",
	.revision       = 2,
	.family         = NFPROTO_UNSPEC,
	.target         = mark2_tg,
	.targetsize     = sizeof(struct xt_mark2_tginfo2),
	.me             = THIS_MODULE,
};

static struct xt_match mark2_mt_reg __read_mostly = {
	.name           = "mark2",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.match          = mark2_mt,
	.matchsize      = sizeof(struct xt_mark2_mtinfo1),
	.me             = THIS_MODULE,
};

static int __init mark2_mt_init(void)
{
	int ret;

	ret = xt_register_target(&mark2_tg_reg);
	if (ret < 0)
		return ret;
	ret = xt_register_match(&mark2_mt_reg);
	if (ret < 0) {
		xt_unregister_target(&mark2_tg_reg);
		return ret;
	}
	return 0;
}

static void __exit mark2_mt_exit(void)
{
	xt_unregister_match(&mark2_mt_reg);
	xt_unregister_target(&mark2_tg_reg);
}

module_init(mark2_mt_init);
module_exit(mark2_mt_exit);
