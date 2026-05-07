#include <stdbool.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_mark2.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

struct xt_mark2_info {
	unsigned long long mark2, mask2;
	uint8_t invert;
};

enum {
	O_MARK2 = 0,
};

static void mark2_mt_help(void)
{
	printf(
"mark2 match options:\n"
"[!] --mark2 value[/mask]    Match nfmark2 value with optional mask\n");
}

static const struct xt_option_entry mark2_mt_opts[] = {
	{.name = "mark2", .id = O_MARK2, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	XTOPT_TABLEEND,
};

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

static void mark2_mt_parse(struct xt_option_call *cb)
{
	unsigned long long mark2 = 0, mask2 = ~0U;
	struct xt_mark2_mtinfo1 *info = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		info->invert = true;
	
	_parse_mark2_val(cb, &mark2, &mask2);
	
	info->mark2 = mark2;
	info->mask2 = mask2;
}

static void mark2_parse(struct xt_option_call *cb)
{
	unsigned long long mark2 = 0, mask2 = ~0U;
	struct xt_mark2_info *mark2info = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		mark2info->invert = 1;
	
	_parse_mark2_val(cb, &mark2, &mask2);
	
	mark2info->mark2 = mark2;
	mark2info->mask2 = mask2;
}

static void print_mark2(unsigned long long mark2, unsigned long long mask2)
{
	if (mask2 != 0xffffffffffffffffU)
		printf(" 0x%llx/0x%llx", mark2, mask2);
	else
		printf(" 0x%llx", mark2);
}

static void
mark2_mt_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mark2_mtinfo1 *info = (const void *)match->data;

	printf(" mark2 match");
	if (info->invert)
		printf(" !");
	print_mark2(info->mark2, info->mask2);
}

static void
mark2_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mark2_info *info = (const void *)match->data;

	printf(" MARK2 match");

	if (info->invert)
		printf(" !");
	
	print_mark2(info->mark2, info->mask2);
}

static void mark2_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mark2_mtinfo1 *info = (const void *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --mark2");
	print_mark2(info->mark2, info->mask2);
}

static void
mark2_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mark2_info *info = (const void *)match->data;

	if (info->invert)
		printf(" !");
	
	printf(" --mark2");
	print_mark2(info->mark2, info->mask2);
}

static struct xtables_match mark2_mt_reg[] = {
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "mark2",
		.revision      = 0,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_mark2_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark2_info)),
		.help          = mark2_mt_help,
		.print         = mark2_print,
		.save          = mark2_save,
		.x6_parse      = mark2_parse,
		.x6_options    = mark2_mt_opts,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "mark2",
		.revision      = 1,
		.family        = NFPROTO_UNSPEC,
		.size          = XT_ALIGN(sizeof(struct xt_mark2_mtinfo1)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark2_mtinfo1)),
		.help          = mark2_mt_help,
		.print         = mark2_mt_print,
		.save          = mark2_mt_save,
		.x6_parse      = mark2_mt_parse,
		.x6_options    = mark2_mt_opts,
	},
};

void _init(void)
{
#if 1 //def CONFIG_RTK_SKB_MARK2
	xtables_register_matches(mark2_mt_reg, ARRAY_SIZE(mark2_mt_reg));
#endif
}
