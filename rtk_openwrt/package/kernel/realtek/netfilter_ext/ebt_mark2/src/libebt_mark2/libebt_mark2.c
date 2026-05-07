/* ebt_mark
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2002, September 2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "ebtables_u.h"
#include <linux/netfilter_bridge/ebt_mark2_t.h>

static int mark2_supplied;

#define MARK2_TARGET  '1'
#define MARK2_SETMARK '2'
#define MARK2_ORMARK  '3'
#define MARK2_ANDMARK '4'
#define MARK2_XORMARK '5'
static struct option opts[] =
{
	{ "mark2-target" , required_argument, 0, MARK2_TARGET },
	/* an oldtime messup, we should have always used the scheme
	 * <extension-name>-<option> */
	{ "set-mark2"    , required_argument, 0, MARK2_SETMARK },
	{ "mark2-set"    , required_argument, 0, MARK2_SETMARK },
	{ "mark2-or"     , required_argument, 0, MARK2_ORMARK  },
	{ "mark2-and"    , required_argument, 0, MARK2_ANDMARK },
	{ "mark2-xor"    , required_argument, 0, MARK2_XORMARK },
	{ 0 }
};

static void print_help()
{
	printf(
	"mark2 target options:\n"
	" --mark2-set value     : Set nfmark2 value\n"
	" --mark2-or  value     : Or nfmark2 with value (nfmark2 |= value)\n"
	" --mark2-and value     : And nfmark2 with value (nfmark2 &= value)\n"
	" --mark2-xor value     : Xor nfmark2 with value (nfmark2 ^= value)\n"
	" --mark2-target target : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_mark2_t_info *mark2info =
	   (struct ebt_mark2_t_info *)target->data;

	mark2info->target = EBT_CONTINUE;//ql modify from EBT_ACCEPT to EBT_CONTINUE
	mark2info->mark2 = 0;
	mark2_supplied = 0;
}

#define OPT_MARK2_TARGET   0x01
#define OPT_MARK2_SETMARK  0x02
#define OPT_MARK2_ORMARK   0x04
#define OPT_MARK2_ANDMARK  0x08
#define OPT_MARK2_XORMARK  0x10
static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_mark2_t_info *mark2info =
	   (struct ebt_mark2_t_info *)(*target)->data;
	char *end;

	switch (c) {
	case MARK2_TARGET:
		{ int tmp;
		ebt_check_option2(flags, OPT_MARK2_TARGET);
		if (FILL_TARGET(optarg, tmp))
			ebt_print_error2("Illegal --mark2-target target");
		/* the 4 lsb are left to designate the target */
		mark2info->target = (mark2info->target & ~EBT_VERDICT_BITS) | (tmp & EBT_VERDICT_BITS);
		}
		return 1;
	case MARK2_SETMARK:
		ebt_check_option2(flags, OPT_MARK2_SETMARK);
		if (*flags & (OPT_MARK2_ORMARK|OPT_MARK2_ANDMARK|OPT_MARK2_XORMARK))
			ebt_print_error2("--mark2-set cannot be used together with specific --mark2 option");
                break;
	case MARK2_ORMARK:
		ebt_check_option2(flags, OPT_MARK2_ORMARK);
		if (*flags & (OPT_MARK2_SETMARK|OPT_MARK2_ANDMARK|OPT_MARK2_XORMARK))
			ebt_print_error2("--mark2-or cannot be used together with specific --mark2 option");
		mark2info->target = (mark2info->target & EBT_VERDICT_BITS) | MARK2_OR_VALUE;
                break;
	case MARK2_ANDMARK:
		ebt_check_option2(flags, OPT_MARK2_ANDMARK);
		if (*flags & (OPT_MARK2_SETMARK|OPT_MARK2_ORMARK|OPT_MARK2_XORMARK))
			ebt_print_error2("--mark2-and cannot be used together with specific --mark2 option");
		mark2info->target = (mark2info->target & EBT_VERDICT_BITS) | MARK2_AND_VALUE;
                break;
	case MARK2_XORMARK:
		ebt_check_option2(flags, OPT_MARK2_XORMARK);
		if (*flags & (OPT_MARK2_SETMARK|OPT_MARK2_ANDMARK|OPT_MARK2_ORMARK))
			ebt_print_error2("--mark2-xor cannot be used together with specific --mark2 option");
		mark2info->target = (mark2info->target & EBT_VERDICT_BITS) | MARK2_XOR_VALUE;
                break;
	 default:
		return 0;
	}
	/* mutual code */
	mark2info->mark2 = strtoull(optarg, &end, 0);
	if (*end != '\0' || end == optarg)
		ebt_print_error2("Bad MARK2 value '%s'", optarg);
	mark2_supplied = 1;
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_mark2_t_info *mark2info =
	   (struct ebt_mark2_t_info *)target->data;

	if (time == 0 && mark2_supplied == 0) {
		ebt_print_error("No mark2 value supplied");
	} else if (BASE_CHAIN && (mark2info->target|~EBT_VERDICT_BITS) == EBT_RETURN)
		ebt_print_error("--mark2-target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_mark2_t_info *mark2info =
	   (struct ebt_mark2_t_info *)target->data;
	int tmp;

	tmp = mark2info->target & ~EBT_VERDICT_BITS;
	if (tmp == MARK2_SET_VALUE)
		printf("--mark2-set");
	else if (tmp == MARK2_OR_VALUE)
		printf("--mark2-or");
	else if (tmp == MARK2_XOR_VALUE)
		printf("--mark2-xor");
	else if (tmp == MARK2_AND_VALUE)
		printf("--mark2-and");
	else
		ebt_print_error("oops, unknown mark2 action, try a later version of ebtables");
	printf(" 0x%llx", mark2info->mark2);
	tmp = mark2info->target | ~EBT_VERDICT_BITS;
	printf(" --mark2-target %s", TARGET_NAME(tmp));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_mark2_t_info *mark2info1 =
	   (struct ebt_mark2_t_info *)t1->data;
	struct ebt_mark2_t_info *mark2info2 =
	   (struct ebt_mark2_t_info *)t2->data;

	return mark2info1->target == mark2info2->target &&
	   mark2info1->mark2 == mark2info2->mark2;
}

static struct ebt_u_target mark2_target =
{
	.name		= EBT_MARK2_TARGET,
	.size		= sizeof(struct ebt_mark2_t_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

__attribute__((constructor)) static void extension_init(void)
{
	ebt_register_target(&mark2_target);
}

