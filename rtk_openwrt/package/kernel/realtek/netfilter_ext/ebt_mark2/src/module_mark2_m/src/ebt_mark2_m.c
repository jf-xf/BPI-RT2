/*
 *  ebt_mark_m
 *
 *	Authors:
 *	Bart De Schuymer <bdschuym@pandora.be>
 *
 *  July, 2002
 *
 */
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_mark2_m.h>

static bool
ebt_mark2_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct ebt_mark2_m_info *info = par->matchinfo;

	if (info->bitmask & EBT_MARK2_OR)
		return !!(skb->mark2 & info->mask2) ^ info->invert;
	return ((skb->mark2 & info->mask2) == info->mark2) ^ info->invert;
}

static int ebt_mark2_mt_check(const struct xt_mtchk_param *par)
{
	const struct ebt_mark2_m_info *info = par->matchinfo;

	if (info->bitmask & ~EBT_MARK2_MASK)
		return -EINVAL;
	if ((info->bitmask & EBT_MARK2_OR) && (info->bitmask & EBT_MARK2_AND))
		return -EINVAL;
	if (!info->bitmask)
		return -EINVAL;
	return 0;
}


#ifdef CONFIG_COMPAT
struct compat_ebt_mark2_m_info {
	compat_u64 mark2, mask2;
	uint8_t invert, bitmask;
};

static void mark2_mt_compat_from_user(void *dst, const void *src)
{
	const struct compat_ebt_mark2_m_info *user = src;
	struct ebt_mark2_m_info *kern = dst;

	kern->mark2 = user->mark2;
	kern->mask2 = user->mask2;
	kern->invert = user->invert;
	kern->bitmask = user->bitmask;
}

static int mark2_mt_compat_to_user(void __user *dst, const void *src)
{
	struct compat_ebt_mark2_m_info __user *user = dst;
	const struct ebt_mark2_m_info *kern = src;

	if (put_user(kern->mark2, &user->mark2) ||
	    put_user(kern->mask2, &user->mask2) ||
	    put_user(kern->invert, &user->invert) ||
	    put_user(kern->bitmask, &user->bitmask))
		return -EFAULT;
	return 0;
}
#endif

static struct xt_match ebt_mark2_mt_reg __read_mostly = {
	.name		= "mark2_m",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_mark2_mt,
	.checkentry	= ebt_mark2_mt_check,
	.matchsize	= sizeof(struct ebt_mark2_m_info),
#ifdef CONFIG_COMPAT
	.compatsize	= sizeof(struct compat_ebt_mark2_m_info),
	.compat_from_user = mark2_mt_compat_from_user,
	.compat_to_user	= mark2_mt_compat_to_user,
#endif
	.me		= THIS_MODULE,
};

static int __init ebt_mark2_m_init(void)
{
	return xt_register_match(&ebt_mark2_mt_reg);
}

static void __exit ebt_mark2_m_fini(void)
{
	xt_unregister_match(&ebt_mark2_mt_reg);
}

module_init(ebt_mark2_m_init);
module_exit(ebt_mark2_m_fini);
MODULE_DESCRIPTION("Ebtables: Packet mark2 match");
MODULE_LICENSE("GPL");
