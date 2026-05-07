/*
 *	xt_connmark - Netfilter module to operate on connection marks
 *
 *	Copyright (C) 2002,2004 MARA Systems AB <http://www.marasystems.com>
 *	by Henrik Nordstrom <hno@marasystems.com>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_connmark2.h>

MODULE_AUTHOR("Henrik Nordstrom <hno@marasystems.com>");
MODULE_DESCRIPTION("Xtables: connection mark2 operations");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_CONNMARK2");
MODULE_ALIAS("ip6t_CONNMARK2");
MODULE_ALIAS("ipt_connmark2");
MODULE_ALIAS("ip6t_connmark2");

static unsigned int
connmark2_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_connmark2_tginfo1 *info = par->targinfo;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	u_int64_t newmark2;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return XT_CONTINUE;

	switch (info->mode) {
	case XT_CONNMARK2_SET:
		newmark2 = (ct->mark2 & ~info->ctmask2) ^ info->ctmark2;
		if (ct->mark2 != newmark2) {
			ct->mark2 = newmark2;
			nf_conntrack_event_cache(IPCT_MARK, ct);
		}
		break;
	case XT_CONNMARK2_SAVE:
		newmark2 = (ct->mark2 & ~info->ctmask2) ^
		          (skb->mark2 & info->nfmask2);
		if (ct->mark2 != newmark2) {
			ct->mark2 = newmark2;
			nf_conntrack_event_cache(IPCT_MARK, ct);
		}
		break;
	case XT_CONNMARK2_RESTORE:
		newmark2 = (skb->mark2 & ~info->nfmask2) ^
		          (ct->mark2 & info->ctmask2);
		skb->mark2 = newmark2;
		break;
	}

	return XT_CONTINUE;
}

static int connmark2_tg_check(const struct xt_tgchk_param *par)
{
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	ret = nf_ct_netns_get(par->net, par->family);
#else
	ret = nf_ct_l3proto_try_module_get(par->family);
#endif
	if (ret < 0)
		pr_info("cannot load conntrack support for proto=%u\n",
			par->family);
	return ret;
}

static void connmark2_tg_destroy(const struct xt_tgdtor_param *par)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	nf_ct_netns_put(par->net, par->family);
#else
	nf_ct_l3proto_module_put(par->family);
#endif
}

static bool
connmark2_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_connmark2_mtinfo1 *info = par->matchinfo;
	enum ip_conntrack_info ctinfo;
	const struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return false;

	return ((ct->mark2 & info->mask2) == info->mark2) ^ info->invert;
}

static int connmark2_mt_check(const struct xt_mtchk_param *par)
{
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	ret = nf_ct_netns_get(par->net, par->family);
#else
	ret = nf_ct_l3proto_try_module_get(par->family);
#endif
	if (ret < 0)
		pr_info("cannot load conntrack support for proto=%u\n",
			par->family);
	return ret;
}

static void connmark2_mt_destroy(const struct xt_mtdtor_param *par)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	nf_ct_netns_put(par->net, par->family);
#else
	nf_ct_l3proto_module_put(par->family);
#endif
}

static struct xt_target connmark2_tg_reg __read_mostly = {
	.name           = "CONNMARK2",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.checkentry     = connmark2_tg_check,
	.target         = connmark2_tg,
	.targetsize     = sizeof(struct xt_connmark2_tginfo1),
	.destroy        = connmark2_tg_destroy,
	.me             = THIS_MODULE,
};

static struct xt_match connmark2_mt_reg __read_mostly = {
	.name           = "connmark2",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.checkentry     = connmark2_mt_check,
	.match          = connmark2_mt,
	.matchsize      = sizeof(struct xt_connmark2_mtinfo1),
	.destroy        = connmark2_mt_destroy,
	.me             = THIS_MODULE,
};

static int __init connmark2_mt_init(void)
{
	int ret;

	ret = xt_register_target(&connmark2_tg_reg);
	if (ret < 0)
		return ret;
	ret = xt_register_match(&connmark2_mt_reg);
	if (ret < 0) {
		xt_unregister_target(&connmark2_tg_reg);
		return ret;
	}
	return 0;
}

static void __exit connmark2_mt_exit(void)
{
	xt_unregister_match(&connmark2_mt_reg);
	xt_unregister_target(&connmark2_tg_reg);
}

module_init(connmark2_mt_init);
module_exit(connmark2_mt_exit);
