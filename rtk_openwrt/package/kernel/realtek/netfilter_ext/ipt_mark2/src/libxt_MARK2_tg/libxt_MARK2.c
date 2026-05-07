#include <stdbool.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_MARK2.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

/* Version 0 */
struct xt_mark2_target_info {
	unsigned long long mark2;
};

/* Version 1 */
enum {
	XT_MARK2_SET=0,
	XT_MARK2_AND,
	XT_MARK2_OR,
};

struct xt_mark2_target_info_v1 {
	unsigned long long mark2;
	uint8_t mode;
};

enum {
	O_SET_MARK2 = 0,
	O_AND_MARK2,
	O_OR_MARK2,
	O_XOR_MARK2,
	O_SET_XMARK2,
	F_SET_MARK2  = 1 << O_SET_MARK2,
	F_AND_MARK2  = 1 << O_AND_MARK2,
	F_OR_MARK2   = 1 << O_OR_MARK2,
	F_XOR_MARK2  = 1 << O_XOR_MARK2,
	F_SET_XMARK2 = 1 << O_SET_XMARK2,
	F_ANY       = F_SET_MARK2 | F_AND_MARK2 | F_OR_MARK2 |
	              F_XOR_MARK2 | F_SET_XMARK2,
};

static void MARK2_help(void)
{
	printf(
"MARK2 target options:\n"
"  --set-mark2 value                   Set nfmark2 value\n"
"  --and-mark2 value                   Binary AND the nfmark2 with value\n"
"  --or-mark2  value                   Binary OR  the nfmark2 with value\n");
}

static const struct xt_option_entry MARK2_opts[] = {
	{.name = "set-mark2", .id = O_SET_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_ANY},
	{.name = "and-mark2", .id = O_AND_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_ANY},
	{.name = "or-mark2", .id = O_OR_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_ANY},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry mark2_tg_opts[] = {
	{.name = "set-xmark2", .id = O_SET_XMARK2, .type = XTTYPE_STRING,
	 .excl = F_ANY},
	{.name = "set-mark2", .id = O_SET_MARK2, .type = XTTYPE_STRING,
	 .excl = F_ANY},
	{.name = "and-mark2", .id = O_AND_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_ANY},
	{.name = "or-mark2", .id = O_OR_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_ANY},
	{.name = "xor-mark2", .id = O_XOR_MARK2, .type = XTTYPE_UINT64,
	 .excl = F_ANY},
	XTOPT_TABLEEND,
};

static void mark2_tg_help(void)
{
	printf(
"MARK2 target options:\n"
"  --set-xmark2 value[/mask]  Clear bits in mask and XOR value into nfmark2\n"
"  --set-mark2 value[/mask]   Clear bits in mask and OR value into nfmark2\n"
"  --and-mark2 bits           Binary AND the nfmark2 with bits\n"
"  --or-mark2 bits            Binary OR the nfmark2 with bits\n"
"  --xor-mask2 bits           Binary XOR the nfmark2 with bits\n"
"\n");
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

static void MARK2_parse_v0(struct xt_option_call *cb)
{
	unsigned long long mark2 = 0, mask2 = ~0U;
	struct xt_mark2_target_info *markinfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SET_MARK2:
		_parse_mark2_val(cb, &mark2, &mask2);
		markinfo->mark2 = mark2;
		break;
	default:
		xtables_error(PARAMETER_PROBLEM,
			   "MARK2 target: kernel too old for --%s",
			   cb->entry->name);
	}
}

static void MARK2_check(struct xt_fcheck_call *cb)
{
	if (cb->xflags == 0)
		xtables_error(PARAMETER_PROBLEM,
		           "MARK2 target: Parameter --set/and/or-mark2"
			   " is required");
}

static void MARK2_parse_v1(struct xt_option_call *cb)
{
	struct xt_mark2_target_info_v1 *mark2info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SET_MARK2:
	        mark2info->mode = XT_MARK2_SET;
		break;
	case O_AND_MARK2:
	        mark2info->mode = XT_MARK2_AND;
		break;
	case O_OR_MARK2:
	        mark2info->mode = XT_MARK2_OR;
		break;
	}
	mark2info->mark2 = cb->val.u64;
}

static void mark2_tg_parse(struct xt_option_call *cb)
{
	unsigned long long mark2 = 0, mask2 = ~0U;
	struct xt_mark2_tginfo2 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SET_XMARK2:
		_parse_mark2_val(cb, &mark2, &mask2);
		info->mark2 = mark2;
		info->mask2 = mask2;
		break;
	case O_SET_MARK2:
		_parse_mark2_val(cb, &mark2, &mask2);
		info->mark2 = mark2;
		info->mask2 = mark2 | mask2;
		break;
	case O_AND_MARK2:
		info->mark2 = 0;
		info->mask2 = ~cb->val.u64;
		break;
	case O_OR_MARK2:
		info->mark2 = info->mask2 = cb->val.u64;
		break;
	case O_XOR_MARK2:
		info->mark2 = cb->val.u64;
		info->mask2 = 0;
		break;
	}
}

static void mark2_tg_check(struct xt_fcheck_call *cb)
{
	if (cb->xflags == 0)
		xtables_error(PARAMETER_PROBLEM, "MARK2: One of the --set-xmark2, "
		           "--{and,or,xor,set}-mark2 options is required");
}

static void
print_mark2(unsigned long long mark2)
{
	printf(" 0x%llx", mark2);
}

static void MARK2_print_v0(const void *ip,
                          const struct xt_entry_target *target, int numeric)
{
	const struct xt_mark2_target_info *mark2info =
		(const struct xt_mark2_target_info *)target->data;
	printf(" MARK2 set");
	print_mark2(mark2info->mark2);
}

static void MARK2_save_v0(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_mark2_target_info *mark2info =
		(const struct xt_mark2_target_info *)target->data;

	printf(" --set-mark2");
	print_mark2(mark2info->mark2);
}

static void MARK2_print_v1(const void *ip, const struct xt_entry_target *target,
                          int numeric)
{
	const struct xt_mark2_target_info_v1 *mark2info =
		(const struct xt_mark2_target_info_v1 *)target->data;

	switch (mark2info->mode) {
	case XT_MARK2_SET:
		printf(" MARK2 set");
		break;
	case XT_MARK2_AND:
		printf(" MARK2 and");
		break;
	case XT_MARK2_OR: 
		printf(" MARK2 or");
		break;
	}
	print_mark2(mark2info->mark2);
}

static void mark2_tg_print(const void *ip, const struct xt_entry_target *target,
                          int numeric)
{
	const struct xt_mark2_tginfo2 *info = (const void *)target->data;

	if (info->mark2 == 0)
		printf(" MARK2 and 0x%llx", (unsigned long long)(uint64_t)~info->mask2);
	else if (info->mark2 == info->mask2)
		printf(" MARK2 or 0x%llx", info->mark2);
	else if (info->mask2 == 0)
		printf(" MARK2 xor 0x%llx", info->mark2);
	else if (info->mask2 == 0xffffffffffffffffU)
		printf(" MARK2 set 0x%llx", info->mark2);
	else
		printf(" MARK2 xset 0x%llx/0x%llx", info->mark2, info->mask2);
}

static void MARK2_save_v1(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_mark2_target_info_v1 *mark2info =
		(const struct xt_mark2_target_info_v1 *)target->data;

	switch (mark2info->mode) {
	case XT_MARK2_SET:
		printf(" --set-mark2");
		break;
	case XT_MARK2_AND:
		printf(" --and-mark2");
		break;
	case XT_MARK2_OR: 
		printf(" --or-mark2");
		break;
	}
	print_mark2(mark2info->mark2);
}

static void mark2_tg_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_mark2_tginfo2 *info = (const void *)target->data;

	printf(" --set-xmark2 0x%llx/0x%llx", info->mark2, info->mask2);
}

static struct xtables_target mark2_tg_reg[] = {
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "MARK2",
		.version       = XTABLES_VERSION,
		.revision      = 0,
		.size          = XT_ALIGN(sizeof(struct xt_mark2_target_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark2_target_info)),
		.help          = MARK2_help,
		.print         = MARK2_print_v0,
		.save          = MARK2_save_v0,
		.x6_parse      = MARK2_parse_v0,
		.x6_fcheck     = MARK2_check,
		.x6_options    = MARK2_opts,
	},
	{
		.family        = NFPROTO_IPV4,
		.name          = "MARK2",
		.version       = XTABLES_VERSION,
		.revision      = 1,
		.size          = XT_ALIGN(sizeof(struct xt_mark2_target_info_v1)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark2_target_info_v1)),
		.help          = MARK2_help,
		.print         = MARK2_print_v1,
		.save          = MARK2_save_v1,
		.x6_parse      = MARK2_parse_v1,
		.x6_fcheck     = MARK2_check,
		.x6_options    = MARK2_opts,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "MARK2",
		.revision      = 2,
		.family        = NFPROTO_UNSPEC,
		.size          = XT_ALIGN(sizeof(struct xt_mark2_tginfo2)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark2_tginfo2)),
		.help          = mark2_tg_help,
		.print         = mark2_tg_print,
		.save          = mark2_tg_save,
		.x6_parse      = mark2_tg_parse,
		.x6_fcheck     = mark2_tg_check,
		.x6_options    = mark2_tg_opts,
	},
};

void _init(void)
{
	xtables_register_targets(mark2_tg_reg, ARRAY_SIZE(mark2_tg_reg));
}
