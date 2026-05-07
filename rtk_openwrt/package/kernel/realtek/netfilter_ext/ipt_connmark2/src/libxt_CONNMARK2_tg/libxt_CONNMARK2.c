/* Shared library add-on to iptables to add CONNMARK target support.
 *
 * (C) 2002,2004 MARA Systems AB <http://www.marasystems.com>
 * by Henrik Nordstrom <hno@marasystems.com>
 *
 * Version 1.1
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_CONNMARK2.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

struct xt_connmark2_target_info {
	unsigned long long mark2;
	unsigned long long mask2;
	uint8_t mode;
};

enum {
	O_SET_MARK2 = 0,
	O_SAVE_MARK2,
	O_RESTORE_MARK2,
	O_AND_MARK2,
	O_OR_MARK2,
	O_XOR_MARK2,
	O_SET_XMARK2,
	O_CTMASK2,
	O_NFMASK2,
	O_MASK2,
	F_SET_MARK2     = 1 << O_SET_MARK2,
	F_SAVE_MARK2    = 1 << O_SAVE_MARK2,
	F_RESTORE_MARK2 = 1 << O_RESTORE_MARK2,
	F_AND_MARK2     = 1 << O_AND_MARK2,
	F_OR_MARK2      = 1 << O_OR_MARK2,
	F_XOR_MARK2     = 1 << O_XOR_MARK2,
	F_SET_XMARK2    = 1 << O_SET_XMARK2,
	F_CTMASK2       = 1 << O_CTMASK2,
	F_NFMASK2       = 1 << O_NFMASK2,
	F_MASK2         = 1 << O_MASK2,
	F_OP_ANY2       = F_SET_MARK2 | F_SAVE_MARK2 | F_RESTORE_MARK2 |
	                 F_AND_MARK2 | F_OR_MARK2 | F_XOR_MARK2 | F_SET_XMARK2,
};

static void CONNMARK2_help(void)
{
	printf(
"CONNMARK2 target options:\n"
"  --set-mark2 value[/mask]       Set conntrack mark value\n"
"  --save-mark2 [--mask2 mask]     Save the packet nfmark in the connection\n"
"  --restore-mark2 [--mask2 mask]  Restore saved nfmark value\n");
}

#define s struct xt_connmark2_target_info
static const struct xt_option_entry CONNMARK2_opts[] = {
	{.name = "set-mark2", .id = O_SET_MARK2, .type = XTTYPE_STRING,
	 .excl = F_OP_ANY2},
	{.name = "save-mark2", .id = O_SAVE_MARK2, .type = XTTYPE_NONE,
	 .excl = F_OP_ANY2},
	{.name = "restore-mark2", .id = O_RESTORE_MARK2, .type = XTTYPE_NONE,
	 .excl = F_OP_ANY2},
	{.name = "mask2", .id = O_MASK2, .type = XTTYPE_UINT64},
	XTOPT_TABLEEND,
};
#undef s

#define s struct xt_connmark2_tginfo1
static const struct xt_option_entry connmark2_tg_opts[] = {
	{.name = "set-xmark2", .id = O_SET_XMARK2, .type = XTTYPE_STRING,
	 .excl = F_OP_ANY2},
	{.name = "set-mark2", .id = O_SET_MARK2, .type = XTTYPE_STRING,
	 .excl = F_OP_ANY2},
	{.name = "and-mark2", .id = O_AND_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_OP_ANY2},
	{.name = "or-mark2", .id = O_OR_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_OP_ANY2},
	{.name = "xor-mark2", .id = O_XOR_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_OP_ANY2},
	{.name = "save-mark2", .id = O_SAVE_MARK2, .type = XTTYPE_NONE,
	 .excl = F_OP_ANY2},
	{.name = "restore-mark2", .id = O_RESTORE_MARK2, .type = XTTYPE_NONE,
	 .excl = F_OP_ANY2},
	{.name = "ctmask2", .id = O_CTMASK2, .type = XTTYPE_UINT64,
	 .excl = F_MASK2, .flags = XTOPT_PUT, XTOPT_POINTER(s, ctmask2)},
	{.name = "nfmask2", .id = O_NFMASK2, .type = XTTYPE_UINT64,
	 .excl = F_MASK2, .flags = XTOPT_PUT, XTOPT_POINTER(s, nfmask2)},
	{.name = "mask2", .id = O_MASK2, .type = XTTYPE_UINT64,
	 .excl = F_CTMASK2 | F_NFMASK2},
	XTOPT_TABLEEND,
};
#undef s

static void connmark2_tg_help(void)
{
	printf(
"CONNMARK2 target options:\n"
"  --set-xmark2 value[/ctmask]    Zero mask bits and XOR ctmark with value\n"
"  --save-mark2 [--ctmask2 mask] [--nfmask mask]\n"
"                                Copy ctmark to nfmark using masks\n"
"  --restore-mark2 [--ctmask2 mask] [--nfmask mask]\n"
"                                Copy nfmark to ctmark using masks\n"
"  --set-mark2 value[/mask]       Set conntrack mark value\n"
"  --save-mark2 [--mask2 mask]     Save the packet nfmark in the connection\n"
"  --restore-mark2 [--mask2 mask]  Restore saved nfmark value\n"
"  --and-mark2 value              Binary AND the ctmark with bits\n"
"  --or-mark2 value               Binary OR  the ctmark with bits\n"
"  --xor-mark2 value              Binary XOR the ctmark with bits\n"
);
}

static void connmark2_tg_init(struct xt_entry_target *target)
{
	struct xt_connmark2_tginfo1 *info = (void *)target->data;

	/*
	 * Need these defaults for --save-mark/--restore-mark if no
	 * --ctmark or --nfmask is given.
	 */
	info->ctmask2 = UINT64_MAX;
	info->nfmask2 = UINT64_MAX;
}

static int _parse_mark2_val(struct xt_option_call *cb, unsigned long long *pmark2, unsigned long long *pmask2)
{
	uintmax_t lmax = UINT64_MAX;
	unsigned long long mark2 = 0, mask2 = ~0U;
	char *end;
	
	if (!xtables_strtoul(cb->arg, &end, &mark2, 0, lmax))
		   xt_params->exit_err(PARAMETER_PROBLEM,
				   "%s: bad mark2 value for option \"--%s\", "
				   "or out of range.\n",
				   cb->ext_name, cb->entry->name);
	if (*end == '/' &&
	   !xtables_strtoul(end + 1, &end, &mask2, 0, lmax))
		   xt_params->exit_err(PARAMETER_PROBLEM,
				   "%s: bad mask value for option \"--%s\", "
				   "or out of range.\n",
				   cb->ext_name, cb->entry->name);
	if (*end != '\0')
		   xt_params->exit_err(PARAMETER_PROBLEM,
				   "%s: trailing garbage after value "
				   "for option \"--%s\".\n",
				   cb->ext_name, cb->entry->name);
	if(pmark2) *pmark2 = mark2;
	if(pmask2) *pmask2 = mask2;
	return 0;
}

static void CONNMARK2_parse(struct xt_option_call *cb)
{
	unsigned long long mark2 = 0, mask2 = ~0U;
	struct xt_connmark2_target_info *markinfo = cb->data;

	xtables_option_parse(cb);
	
	switch (cb->entry->id) {
	case O_SET_MARK2:
		_parse_mark2_val(cb, &mark2, &mask2);
		markinfo->mode = XT_CONNMARK2_SET;
		markinfo->mark2 = mark2;
		markinfo->mask2 = mask2;
		break;
	case O_SAVE_MARK2:
		markinfo->mode = XT_CONNMARK2_SAVE;
		break;
	case O_RESTORE_MARK2:
		markinfo->mode = XT_CONNMARK2_RESTORE;
		break;
	case O_MASK2:
		markinfo->mask2 = cb->val.u64;
		break;
	}
}

static void connmark2_tg_parse(struct xt_option_call *cb)
{
	unsigned long long mark2 = 0, mask2 = ~0U;
	struct xt_connmark2_tginfo1 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SET_XMARK2:
		_parse_mark2_val(cb, &mark2, &mask2);
		info->mode   = XT_CONNMARK2_SET;
		info->ctmark2 = mark2;
		info->ctmask2 = mask2;
		break;
	case O_SET_MARK2:
		_parse_mark2_val(cb, &mark2, &mask2);
		info->mode   = XT_CONNMARK2_SET;
		info->ctmark2 = mark2;
		info->ctmask2 = mark2 | mask2;
		break;
	case O_AND_MARK2:
		info->mode   = XT_CONNMARK2_SET;
		info->ctmark2 = 0;
		info->ctmask2 = ~cb->val.u64;
		break;
	case O_OR_MARK2:
		info->mode   = XT_CONNMARK2_SET;
		info->ctmark2 = cb->val.u64;
		info->ctmask2 = cb->val.u64;
		break;
	case O_XOR_MARK2:
		info->mode   = XT_CONNMARK2_SET;
		info->ctmark2 = cb->val.u64;
		info->ctmask2 = 0;
		break;
	case O_SAVE_MARK2:
		info->mode = XT_CONNMARK2_SAVE;
		break;
	case O_RESTORE_MARK2:
		info->mode = XT_CONNMARK2_RESTORE;
		break;
	case O_MASK2:
		info->nfmask2 = info->ctmask2 = cb->val.u64;
		break;
	}
}

static void connmark2_tg_check(struct xt_fcheck_call *cb)
{
	if (!(cb->xflags & F_OP_ANY2))
		xtables_error(PARAMETER_PROBLEM,
		           "CONNMARK2 target: No operation specified");
}

static void
print_mark2(unsigned long long mark2)
{
	printf("0x%llx", mark2);
}

static void
print_mask2(const char *text, unsigned long long mask2)
{
	if (mask2 != 0xffffffffffffffffULL)
		printf("%s0x%llx", text, mask2);
}

static void CONNMARK2_print(const void *ip,
                           const struct xt_entry_target *target, int numeric)
{
	const struct xt_connmark2_target_info *mark2info =
		(const struct xt_connmark2_target_info *)target->data;
	switch (mark2info->mode) {
	case XT_CONNMARK2_SET:
	    printf(" CONNMARK2 set ");
	    print_mark2(mark2info->mark2);
	    print_mask2("/", mark2info->mask2);
	    break;
	case XT_CONNMARK2_SAVE:
	    printf(" CONNMARK2 save ");
	    print_mask2("mask2 ", mark2info->mask2);
	    break;
	case XT_CONNMARK2_RESTORE:
	    printf(" CONNMARK2 restore ");
	    print_mask2("mask2 ", mark2info->mask2);
	    break;
	default:
	    printf(" ERROR: UNKNOWN CONNMARK2 MODE");
	    break;
	}
}

static void
connmark2_tg_print(const void *ip, const struct xt_entry_target *target,
                  int numeric)
{
	const struct xt_connmark2_tginfo1 *info = (const void *)target->data;

	switch (info->mode) {
	case XT_CONNMARK2_SET:
		if (info->ctmark2 == 0)
			printf(" CONNMARK2 and 0x%llx",
			       (unsigned long long)(uint64_t)~info->ctmask2);
		else if (info->ctmark2 == info->ctmask2)
			printf(" CONNMARK2 or 0x%llx", info->ctmark2);
		else if (info->ctmask2 == 0)
			printf(" CONNMARK2 xor 0x%llx", info->ctmark2);
		else if (info->ctmask2 == 0xFFFFFFFFFFFFFFFFUL)
			printf(" CONNMARK2 set 0x%llx", info->ctmark2);
		else
			printf(" CONNMARK2 xset 0x%llx/0x%llx",
			       info->ctmark2, info->ctmask2);
		break;
	case XT_CONNMARK2_SAVE:
		if (info->nfmask2 == UINT64_MAX && info->ctmask2 == UINT64_MAX)
			printf(" CONNMARK2 save");
		else if (info->nfmask2 == info->ctmask2)
			printf(" CONNMARK2 save mask2 0x%llx", info->nfmask2);
		else
			printf(" CONNMARK2 save nfmask2 0x%llx ctmask2 ~0x%llx",
			       info->nfmask2, info->ctmask2);
		break;
	case XT_CONNMARK2_RESTORE:
		if (info->ctmask2 == UINT64_MAX && info->nfmask2 == UINT64_MAX)
			printf(" CONNMARK2 restore");
		else if (info->ctmask2 == info->nfmask2)
			printf(" CONNMARK2 restore mask2 0x%llx", info->ctmask2);
		else
			printf(" CONNMARK2 restore ctmask2 0x%llx nfmask ~0x%llx",
			       info->ctmask2, info->nfmask2);
		break;

	default:
		printf(" ERROR: UNKNOWN CONNMARK2 MODE");
		break;
	}
}

static void CONNMARK2_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_connmark2_target_info *markinfo =
		(const struct xt_connmark2_target_info *)target->data;

	switch (markinfo->mode) {
	case XT_CONNMARK2_SET:
	    printf(" --set-mark2 ");
	    print_mark2(markinfo->mark2);
	    print_mask2("/", markinfo->mask2);
	    break;
	case XT_CONNMARK2_SAVE:
	    printf(" --save-mark2 ");
	    print_mask2("--mask2 ", markinfo->mask2);
	    break;
	case XT_CONNMARK2_RESTORE:
	    printf(" --restore-mark2 ");
	    print_mask2("--mask2 ", markinfo->mask2);
	    break;
	default:
	    printf(" ERROR: UNKNOWN CONNMARK2 MODE");
	    break;
	}
}

static void CONNMARK2_init(struct xt_entry_target *t)
{
	struct xt_connmark2_target_info *markinfo
		= (struct xt_connmark2_target_info *)t->data;

	markinfo->mask2 = 0xffffffffffffffffULL;
}

static void
connmark2_tg_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_connmark2_tginfo1 *info = (const void *)target->data;

	switch (info->mode) {
	case XT_CONNMARK2_SET:
		printf(" --set-xmark2 0x%llx/0x%llx", info->ctmark2, info->ctmask2);
		break;
	case XT_CONNMARK2_SAVE:
		printf(" --save-mark2 --nfmask2 0x%llx --ctmask2 0x%llx",
		       info->nfmask2, info->ctmask2);
		break;
	case XT_CONNMARK2_RESTORE:
		printf(" --restore-mark2 --nfmask2 0x%llx --ctmask2 0x%llx",
		       info->nfmask2, info->ctmask2);
		break;
	default:
		printf(" ERROR: UNKNOWN CONNMARK2 MODE");
		break;
	}
}

static struct xtables_target connmark2_tg_reg[] = {
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "CONNMARK2",
		.revision      = 0,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_connmark2_target_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_connmark2_target_info)),
		.help          = CONNMARK2_help,
		.init          = CONNMARK2_init,
		.print         = CONNMARK2_print,
		.save          = CONNMARK2_save,
		.x6_parse      = CONNMARK2_parse,
		.x6_fcheck     = connmark2_tg_check,
		.x6_options    = CONNMARK2_opts,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "CONNMARK2",
		.revision      = 1,
		.family        = NFPROTO_UNSPEC,
		.size          = XT_ALIGN(sizeof(struct xt_connmark2_tginfo1)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_connmark2_tginfo1)),
		.help          = connmark2_tg_help,
		.init          = connmark2_tg_init,
		.print         = connmark2_tg_print,
		.save          = connmark2_tg_save,
		.x6_parse      = connmark2_tg_parse,
		.x6_fcheck     = connmark2_tg_check,
		.x6_options    = connmark2_tg_opts,
	},
};

void _init(void)
{
	xtables_register_targets(connmark2_tg_reg, ARRAY_SIZE(connmark2_tg_reg));
}
