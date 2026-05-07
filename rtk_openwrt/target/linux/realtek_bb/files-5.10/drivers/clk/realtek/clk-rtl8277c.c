#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <soc/cortina/rtl8277c_registers.h>
#include <soc/cortina/cortina-soc.h>

#define to_rtl8277c_pll_clk(_hw) container_of(_hw, struct rtl8277c_pll_clk, hw)
#define to_rtl8277c_cpu_daemon(_hw) \
		container_of(_hw, struct rtl8277c_cpu_daemon, hw)
#define clk_mask(width) ((1 << width) - 1)


#define RTK_CPU_PATCH 1
#define RTK_CLK_DIV_PATCH 1
enum {
	CA_CLK_PLL,
	CA_CLK_DIVIDER,
	CA_CLK_MUX,
	CA_CLK_GATE,
	CA_CLK_FIXED_FACTOR,
	CA_CLK_PE_DAEMON,
	CA_CLK_CPU_DAEMON,
	CA_CLK_MMC_PHASE,
};

enum {
	PED_REF,
	PED_FPLL,
	PED_DIV_F,
	PED_MUX_PE,
	PED_CLKS_NUM,
};

enum {
	CPUD_REF,
	CPUD_CPLL,
	CPUD_DIV_F2C,
	CPUD_MUX_CPU,
	CPUD_DIV_CORTEX,
	CPUD_CLKS_NUM,
};

#ifdef CONFIG_ARCH_REALTEK_TAURUS
#define DIVN_BASE		3
#define DIVN_SHIFT		0
#define DIVN_WIDTH		8
#define PREDIV_BASE		2
#define PREDIV_SHIFT		12
#define PREDIV_WIDTH		2
#else
#define DIVN_BASE		3
#define DIVN_SHIFT		0
#define DIVN_WIDTH		8
#define PREDIV_BASE		2
#define PREDIV_SHIFT		10
#define PREDIV_WIDTH		2
#endif

// delay for pll lock should be more than 160us
#define MIN_DELAY_PLL_FOR_LOCK		200
#define MIN_DELAY_PLL_FOR_SWITCH	10

struct rtl8277c_clk_reg {
	void __iomem *regbase;
	spinlock_t regs_lock;	/* register access lock */
};

struct rtl8277c_clk_param {
	int type;
	u32 reg;
};

struct rtl8277c_pll_clk {
	void __iomem *reg;
	spinlock_t *lock;	/* register access lock */
	struct clk_hw hw;
};

struct rtl8277c_pll_param {
	unsigned int    pll;
	unsigned int    prediv;
	unsigned int    divn;
};

struct rtl8277c_cpu_strap_speed {
	unsigned int    pll;
	unsigned int    div;
};

struct rtl8277c_pe_speed {
	unsigned int    pll;
	unsigned int    div;
};

struct rtl8277c_cpu_daemon {
	void __iomem *reg;
	struct mutex mlock;	/* daemon lock */
	spinlock_t *lock;	/* register access lock */
	struct clk_hw hw;
	struct clk *clks[CPUD_CLKS_NUM];
};

static const struct rtl8277c_pll_param rtl8277c_pll_param_table[] = {
	{ .pll = 40, .prediv = 5, .divn = 200, },	/* CPU */
	{ .pll = 44, .prediv = 5, .divn = 220, },	/* CPU */
	{ .pll = 48, .prediv = 5, .divn = 240, },	/* CPU */
	{ .pll = 50, .prediv = 5, .divn = 250, },	/* CPU */
	{ .pll = 56, .prediv = 3, .divn = 168, },	/* CPU */
	{ .pll = 64, .prediv = 3, .divn = 192, },	/* FPLL/CPU */
	{ .pll = 72, .prediv = 3, .divn = 216, },	/* CPU */
	{ .pll = 80, .prediv = 2, .divn = 160, },	/* EPLL */
};

static const struct clk_div_table rtl8277c_sd_div_table[] = {
	{ .val = 0, .div = 40, },
	{ .val = 1, .div = 20, },
	{ .val = 2, .div = 10, },
};

static const struct rtl8277c_cpu_strap_speed rtl8277c_cpu_strap_speed_table[] = {
	{ .pll = 48, .div = 3, }, /* 400000000 */
	{ .pll = 50, .div = 2, }, /* 625000000 */
	{ .pll = 56, .div = 2, }, /* 700000000 */
	{ .pll = 64, .div = 2, }, /* 800000000 */
	{ .pll = 72, .div = 2, }, /* 900000000 */
	{ .pll = 40, .div = 1, }, /* 1000000000 */
	{ .pll = 44, .div = 1, }, /* 1100000000 */
	{ .pll = 48, .div = 1, }, /* 1200000000 */
	{ }
};

static const struct rtl8277c_pe_speed rtl8277c_pe_speed_table[] = {
	{ .pll = 64, .div = 2, }, /* 800000000 */
	{ }
};

static struct rtl8277c_clk_param pll_cpll     = {CA_CLK_PLL, GLOBAL_CPLL0};
static struct rtl8277c_clk_param pll_epll     = {CA_CLK_PLL, GLOBAL_EPLL0};
static struct rtl8277c_clk_param pll_fpll     = {CA_CLK_PLL, GLOBAL_FPLL0};
static struct rtl8277c_clk_param mux_cplldiv  = {CA_CLK_MUX, GLOBAL_CPLLDIV};
static struct rtl8277c_clk_param mux_pediv    = {CA_CLK_MUX, GLOBAL_PEDIV};
static struct rtl8277c_clk_param div_cplldiv  = {CA_CLK_DIVIDER, GLOBAL_CPLLDIV};
static struct rtl8277c_clk_param div_fplldiv  = {CA_CLK_DIVIDER, GLOBAL_PEDIV};
static struct rtl8277c_clk_param div_eplldiv  = {CA_CLK_DIVIDER, GLOBAL_EPLLDIV};
static struct rtl8277c_clk_param div_eplldiv2 = {CA_CLK_DIVIDER, GLOBAL_EPLLDIV2};
static struct rtl8277c_clk_param gate_config  = {CA_CLK_GATE, GLOBAL_GLOBAL_CONFIG};
static struct rtl8277c_clk_param fixed_factor = {CA_CLK_FIXED_FACTOR, 0};
static struct rtl8277c_clk_param pe_daemon    = {CA_CLK_PE_DAEMON, 0};
static struct rtl8277c_clk_param cpu_daemon   = {CA_CLK_CPU_DAEMON, 0};
//static struct rtl8277c_clk_param div_trcdiv   = {CA_CLK_DIVIDER, GLOBAL_CPLLDIV};

static const struct of_device_id rtl8277c_soc_clks_ids[] __initconst = {
	{ .compatible = "cortina,rtl8277c-pll-cpll-clk",
	  .data = (void *)&pll_cpll, },
	{ .compatible = "cortina,rtl8277c-pll-fpll-clk",
	  .data = (void *)&pll_fpll, },
	{ .compatible = "cortina,rtl8277c-pll-epll-clk",
	  .data = (void *)&pll_epll, },
	{ .compatible = "cortina,mux-cpu-clk",
	  .data = (void *)&mux_cplldiv, },
	{ .compatible = "cortina,mux-pe-clk",
	  .data = (void *)&mux_pediv, },
	{ .compatible = "cortina,div-cpll-clk",
	  .data = (void *)&div_cplldiv, },
	{ .compatible = "cortina,div-fpll-clk",
	  .data = (void *)&div_fplldiv, },
	{ .compatible = "cortina,div-epll-clk",
	  .data = (void *)&div_eplldiv, },
	{ .compatible = "cortina,div-epll2-clk",
	  .data = (void *)&div_eplldiv2, },
	{ .compatible = "cortina,gate-config-clk",
	  .data = (void *)&gate_config, },
	{ .compatible = "cortina,fixed-factor-clk",
	  .data = (void *)&fixed_factor, },
	{ .compatible = "cortina,rtl8277c-pe-daemon-clk",
	  .data = (void *)&pe_daemon, },
	{ .compatible = "cortina,rtl8277c-cpu-daemon-clk",
	  .data = (void *)&cpu_daemon, },
	{},
};

static bool readonly;

static unsigned long rtl8277c_pll_recalc_rate(struct clk_hw *hw,
				     unsigned long parent_rate)
{
	struct rtl8277c_pll_clk *pll_clk = to_rtl8277c_pll_clk(hw);
	u32 val, divn, prediv;
#ifdef CONFIG_ARM
	u64 result = 0;
#endif
	val = readl(pll_clk->reg) >> DIVN_SHIFT;
	val &= clk_mask(DIVN_WIDTH);
	divn = val + DIVN_BASE;

	val = readl(pll_clk->reg) >> PREDIV_SHIFT;
	val &= clk_mask(PREDIV_WIDTH);
	prediv = val + PREDIV_BASE;

#ifdef CONFIG_ARM
	result = (u64)parent_rate * (u64)divn;
	do_div(result , prediv);
	return result;
#else
	return (parent_rate * divn / prediv);
#endif
}

static long rtl8277c_pll_round_rate(struct clk_hw *hw, unsigned long rate,
			   unsigned long *parent_rate)
{
	unsigned long mult;
	int i;

	if ((rate % (*parent_rate)) != 0)
		goto NOT_SUPPORT;
	mult = rate / (*parent_rate);

	for (i = 0; i < ARRAY_SIZE(rtl8277c_pll_param_table); i++) {
		if (rtl8277c_pll_param_table[i].pll == mult)
			return rate;
	}

NOT_SUPPORT:
	return *parent_rate;
}

static int rtl8277c_pll_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct rtl8277c_pll_clk *pll_clk = to_rtl8277c_pll_clk(hw);
	unsigned long mult;
	unsigned long flags = 0;
	u32 val, divn, prediv;
	int i;

	if ((rate % parent_rate) != 0)
		return 0;

	mult = rate / parent_rate;
	prediv = 0;
	divn = 0;

	for (i = 0; i < ARRAY_SIZE(rtl8277c_pll_param_table); i++) {
		if (rtl8277c_pll_param_table[i].pll == mult) {
			prediv = rtl8277c_pll_param_table[i].prediv;
			divn = rtl8277c_pll_param_table[i].divn;
		}
	}
	if (mult != (divn / prediv))
		return 0;

	spin_lock_irqsave(pll_clk->lock, flags);
	val = readl(pll_clk->reg);
	val &= ~(clk_mask(DIVN_WIDTH) << DIVN_SHIFT);
	val |= (divn - DIVN_BASE) << DIVN_SHIFT;
	val &= ~(clk_mask(PREDIV_WIDTH) << PREDIV_SHIFT);
	val |= (prediv - PREDIV_BASE) << PREDIV_SHIFT;
	writel(val, pll_clk->reg);
	spin_unlock_irqrestore(pll_clk->lock, flags);

	return rate;
}

static const struct clk_ops rtl8277c_pll_ops = {
	.recalc_rate	= rtl8277c_pll_recalc_rate,
	.round_rate	= rtl8277c_pll_round_rate,
	.set_rate	= rtl8277c_pll_set_rate,
};

static int rtl8277c_clk_pll(struct device_node *np, struct rtl8277c_clk_reg *clk_reg,
		   struct rtl8277c_clk_param *clk_param)
{
	const char *parent_name;
	void __iomem *reg;
	struct clk_init_data init;
	struct rtl8277c_pll_clk *pll_clk;
	struct clk *soc_clk;

	parent_name = of_clk_get_parent_name(np, 0);
	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	pll_clk = kzalloc(sizeof(*pll_clk), GFP_KERNEL);
	if (!pll_clk)
		return -ENOMEM;

	init.name = np->name;
	init.ops = &rtl8277c_pll_ops;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	pll_clk->hw.init = &init;
	pll_clk->reg = reg;
	pll_clk->lock = &clk_reg->regs_lock;

	soc_clk = clk_register(NULL, &pll_clk->hw);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);
	else
		kfree(pll_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl8277c_clk_divider(struct device_node *np, struct rtl8277c_clk_reg *clk_reg,
		       struct rtl8277c_clk_param *clk_param)
{
	u8 op_shift, op_width;
#if RTK_CLK_DIV_PATCH
	u8 init_div = 0;
	u32 val;
	unsigned long flags = 0;
#endif
	const char *parent_name;
	void __iomem *reg;
	struct clk *soc_clk;


	if (of_property_read_u8(np, "operate-shift", &op_shift))
		return -EPERM;
	if (op_shift >= 32)
		return -EINVAL;
	if (of_property_read_u8(np, "operate-width", &op_width))
		return -EPERM;
	if (op_width >= 32)
		return -EINVAL;

	parent_name = of_clk_get_parent_name(np, 0);
	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

#if RTK_CLK_DIV_PATCH
	if (of_property_read_u8(np, "init-div", &init_div))
		init_div = 0;

	if (init_div) {
		spin_lock_irqsave(&clk_reg->regs_lock, flags);

		val = readl(reg);

		val &= ~(clk_mask(op_width) << op_shift);
		val |= (init_div << 1) << op_shift;

		writel(val, reg);

		spin_unlock_irqrestore(&clk_reg->regs_lock, flags);
	}
#endif

		soc_clk = clk_register_divider(NULL, np->name, parent_name, 0,
					       reg, op_shift + 1, op_width - 1,
					       CLK_DIVIDER_ONE_BASED |
					       CLK_DIVIDER_ALLOW_ZERO,
					       &clk_reg->regs_lock);

	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl8277c_clk_mux(struct device_node *np, struct rtl8277c_clk_reg *clk_reg,
		   struct rtl8277c_clk_param *clk_param)
{
	u8 op_shift, num_parents;
	void __iomem *reg;
	const char *parent_names[2];
	struct clk *soc_clk;
	int i;

	if (of_property_read_u8(np, "operate-shift", &op_shift))
		return -EPERM;
	if (op_shift >= 32)
		return -EINVAL;
	num_parents = of_clk_get_parent_count(np);
	if (num_parents != 2)
		return -EINVAL;

	for (i = 0; i < num_parents; i++)
		parent_names[i] = of_clk_get_parent_name(np, i);

	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	soc_clk = clk_register_mux(NULL, np->name, parent_names, num_parents,
				   0, reg, op_shift, 1, 0, &clk_reg->regs_lock);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl8277c_clk_gate(struct device_node *np, struct rtl8277c_clk_reg *clk_reg,
		    struct rtl8277c_clk_param *clk_param)
{
	u8 op_shift, gate_flags;
	void __iomem *reg;
	struct clk *soc_clk;

	if (of_property_read_u8(np, "operate-shift", &op_shift))
		return -EPERM;
	if (op_shift >= 32)
		return -EINVAL;

	if (of_property_read_bool(np, "active-low"))
		gate_flags = CLK_GATE_SET_TO_DISABLE;
	else
		gate_flags = 0;

	reg = clk_reg->regbase + clk_param->reg - GLOBAL_JTAG_ID;

	soc_clk = clk_register_gate(NULL, np->name, NULL, 0, reg, op_shift,
				    gate_flags, &clk_reg->regs_lock);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl8277c_clk_fixed_factor(struct device_node *np,
			    struct rtl8277c_clk_reg *clk_reg,
			    struct rtl8277c_clk_param *clk_param)
{
	unsigned int mult, div;
	const char *parent_name;
	struct clk *soc_clk;

	if (of_property_read_u32(np, "fixed-mult", &mult))
		mult = 1;
	if (of_property_read_u32(np, "fixed-div", &div))
		div = 1;

	parent_name = of_clk_get_parent_name(np, 0);

	soc_clk = clk_register_fixed_factor(NULL, np->name, parent_name, 0,
					    mult, div);
	if (!IS_ERR(soc_clk))
		of_clk_add_provider(np, of_clk_src_simple_get, soc_clk);

	return IS_ERR(soc_clk) ? PTR_ERR(soc_clk) : 0;
}

static int rtl8277c_clk_pe_daemon(struct device_node *np,
			 struct rtl8277c_clk_reg *clk_reg,
			 struct rtl8277c_clk_param *clk_param)
{
	/* keep FPLL and PEDIV untainted */

	return 0;
}
static unsigned long rtl8277c_cpu_recalc_rate(struct clk_hw *hw,
				     unsigned long parent_rate)
{
	struct rtl8277c_cpu_daemon *cpud = to_rtl8277c_cpu_daemon(hw);
	unsigned long ref_rate;
	GLOBAL_CPLLDIV_t cplldiv;
	GLOBAL_CPLL0_t cpll0;
	void __iomem *cpll0_reg;
	void __iomem *cplldiv_reg;
	u8 prediv, mult, div;
#ifdef CONFIG_ARM
	u64 recalc_rate_result = 0;
#endif
	cpll0_reg = cpud->reg + GLOBAL_CPLL0 - GLOBAL_JTAG_ID;
	cplldiv_reg = cpud->reg + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;

	cpll0.wrd = readl(cpll0_reg);
	cplldiv.wrd = readl(cplldiv_reg);
	mult = cpll0.bf.SCPU_DIV_DIVN + DIVN_BASE;
	prediv = cpll0.bf.DIV_PREDIV_SEL + PREDIV_BASE;
	div = cplldiv.bf.cortex_divsel >> 1;

	// if bypass
	if (div == 0)
		div = 1;

	ref_rate = clk_get_rate(cpud->clks[CPUD_REF]);
#ifdef CONFIG_ARM
		recalc_rate_result = (u64)ref_rate * (u64)mult;
		do_div(recalc_rate_result, prediv);
		do_div(recalc_rate_result, div);

		return (u32)recalc_rate_result;
#else
	return ref_rate * mult / prediv / div;
#endif
}

static long rtl8277c_cpu_round_rate(struct clk_hw *hw, unsigned long rate,
			   unsigned long *parent_rate)
{
	struct rtl8277c_cpu_daemon *cpud = to_rtl8277c_cpu_daemon(hw);
	unsigned long ref_rate, target;
	u32 mult, div;
	int i;

	ref_rate = clk_get_rate(cpud->clks[CPUD_REF]);
	for (i = 0; ARRAY_SIZE(rtl8277c_cpu_strap_speed_table); i++) {
		mult = rtl8277c_cpu_strap_speed_table[i].pll;
		div = rtl8277c_cpu_strap_speed_table[i].div;
		target = ref_rate * mult / div;
		if (rate == target)
			return target;
	}

	return *parent_rate;
}

static int rtl8277c_cpu_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct rtl8277c_cpu_daemon *cpud = to_rtl8277c_cpu_daemon(hw);
	GLOBAL_CPLLDIV_t cplldiv;
	GLOBAL_CPLL0_t cpll0;
	void __iomem *reg;
	unsigned long ref_rate;
	unsigned long long target;
	u32 mult, div;
	unsigned long flags = 0;
	int i;

	if (readonly)
		return 0;

	ref_rate = clk_get_rate(cpud->clks[CPUD_REF]);
	for (i = 0; ARRAY_SIZE(rtl8277c_cpu_strap_speed_table); i++) {
		mult = rtl8277c_cpu_strap_speed_table[i].pll;
		div = rtl8277c_cpu_strap_speed_table[i].div;
#ifdef CONFIG_ARM
		target = (u64)ref_rate * (u64)mult;
		do_div(target, div);
#else
		target = ref_rate * mult / div;
#endif
		if (rate == target)
			goto MATCH;
	}

	return 0;

MATCH:
	mutex_lock(&cpud->mlock);

	/* unbind daemon <--> cortex temporarily to avoid loop */
	clk_set_parent(hw->clk, NULL);

	// before setting cpll, must change clk source to fpll
	clk_set_parent(cpud->clks[CPUD_MUX_CPU], cpud->clks[CPUD_DIV_F2C]);

	// must clr RSTB before setting cpll
	spin_lock_irqsave(cpud->lock, flags);
	reg = cpud->reg + GLOBAL_CPLL0 - GLOBAL_JTAG_ID;
	cpll0.wrd = readl(reg);
	cpll0.bf.SCPU_RSTB = 0;
	writel(cpll0.wrd, reg);
	spin_unlock_irqrestore(cpud->lock, flags);

	clk_set_rate(cpud->clks[CPUD_CPLL], ref_rate * mult);

	//set override after setting cpll
	spin_lock_irqsave(cpud->lock, flags);
	reg = cpud->reg + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;
	cplldiv.wrd = readl(reg);
	cplldiv.bf.cpll_mode_override = 1;
	writel(cplldiv.wrd, reg);
	spin_unlock_irqrestore(cpud->lock, flags);

	// reset RSTB to enable cpll
	spin_lock_irqsave(cpud->lock, flags);
	reg = cpud->reg + GLOBAL_CPLL0 - GLOBAL_JTAG_ID;
	cpll0.wrd = readl(reg);
	cpll0.bf.SCPU_RSTB = 1;
	writel(cpll0.wrd, reg);
	spin_unlock_irqrestore(cpud->lock, flags);
	udelay(MIN_DELAY_PLL_FOR_LOCK);

	// set divider and override
	spin_lock_irqsave(cpud->lock, flags);
	reg = cpud->reg + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;
	cplldiv.wrd = readl(reg);
	cplldiv.bf.cortex_divsel = div << 1;
	cplldiv.bf.cpll_div_override = 1;
	writel(cplldiv.wrd, reg);
	spin_unlock_irqrestore(cpud->lock, flags);

	// swich clock source back to cpll
	clk_set_parent(cpud->clks[CPUD_MUX_CPU], cpud->clks[CPUD_CPLL]);

	// bind again
	clk_set_parent(hw->clk, cpud->clks[CPUD_DIV_CORTEX]);

	mutex_unlock(&cpud->mlock);

	pr_info("Change CPU frequency to %ld MHz\n", rate / 1000000);

	return rate;
}

static int rtl8277c_cpu_init(struct clk_hw *hw)
{
	struct rtl8277c_cpu_daemon *cpud = to_rtl8277c_cpu_daemon(hw);
	void __iomem *cpllmux_reg;

	cpllmux_reg = cpud->reg + GLOBAL_CPLLMUX - GLOBAL_JTAG_ID;

	/* magic values received from HW team		*/
	/* prevent cpu stall while changing frequency	*/
	writel(0xff000000, cpllmux_reg);
	return 0;
}

static const struct clk_ops rtl8277c_cpu_ops = {
	.recalc_rate	= rtl8277c_cpu_recalc_rate,
	.round_rate	= rtl8277c_cpu_round_rate,
	.set_rate	= rtl8277c_cpu_set_rate,
	.init		= rtl8277c_cpu_init,
};

#if defined(RTK_CPU_PATCH)
extern struct clk *cpu_daemon_clk;
#endif
static int rtl8277c_clk_cpu_daemon(struct device_node *np,
			  struct rtl8277c_clk_reg *clk_reg,
			  struct rtl8277c_clk_param *clk_param)
{
	const char *parent_names[5];
	struct clk *ref_clk;
	unsigned long ref_rate, min_rate, max_rate;
	int i;
	struct clk *cpu_clk;
#if defined(RTK_CPU_PATCH)
	void __iomem *reg;
	int strap;
	GLOBAL_CPLLDIV_t cplldiv;
	unsigned long mult, div;
	u64 rate;
#endif
	struct clk_init_data init;
	struct rtl8277c_cpu_daemon *cpud;

	readonly = of_property_read_bool(np, "read-only");

	cpud = kzalloc(sizeof(*cpud), GFP_KERNEL);
	if (!cpud)
		return -ENOMEM;

	for (i = 0; i < CPUD_CLKS_NUM; i++) {
		parent_names[i] = of_clk_get_parent_name(np, i);
		cpud->clks[i] = __clk_lookup(parent_names[i]);
	}

	ref_clk = __clk_lookup(parent_names[CPUD_REF]);
	ref_rate = clk_get_rate(ref_clk);
	min_rate = ref_rate * rtl8277c_cpu_strap_speed_table[0].pll /
			      rtl8277c_cpu_strap_speed_table[0].div;
	max_rate = ref_rate * rtl8277c_cpu_strap_speed_table[7].pll /
			      rtl8277c_cpu_strap_speed_table[7].div;
				  
#if defined(RTK_CPU_PATCH)
		reg = clk_reg->regbase + GLOBAL_CPLLDIV - GLOBAL_JTAG_ID;
		cplldiv.wrd = readl(reg);
		if(cplldiv.bf.cpll_mode_override == 1){
			printk("SOC: CPLL Clock has inited. Skip CPLL Init.");
		}else{
			reg = clk_reg->regbase + GLOBAL_STRAP - GLOBAL_JTAG_ID;
			strap = (readl(reg) >> 1) & 0x7;
			mult = rtl8277c_cpu_strap_speed_table[strap].pll;
			div = rtl8277c_cpu_strap_speed_table[strap].div;
			rate = (u64)ref_rate * (u64)mult / div;

			clk_set_rate(cpud->clks[CPUD_CPLL], ref_rate * mult);
				/* +1 for div is 3 */
			rate = clk_get_rate(cpud->clks[CPUD_MUX_CPU]) / div + 1;
			clk_set_rate(cpud->clks[CPUD_DIV_CORTEX], rate);
		}
#endif
				  
	init.name = np->name;
	init.ops = &rtl8277c_cpu_ops;
	init.parent_names = &parent_names[CPUD_DIV_CORTEX];
	init.num_parents = 1;

	mutex_init(&cpud->mlock);
	cpud->reg = clk_reg->regbase;
	cpud->lock = &clk_reg->regs_lock;
	cpud->hw.init = &init;

	cpu_clk = clk_register(NULL, &cpud->hw);
	if (!IS_ERR(cpu_clk)) {
		of_clk_add_provider(np, of_clk_src_simple_get, cpu_clk);
		clk_set_min_rate(cpu_clk, min_rate);
		clk_set_max_rate(cpu_clk, max_rate);
#if defined(RTK_CPU_PATCH)
               cpu_daemon_clk = cpu_clk;
#endif

	} else {
		kfree(cpud);
	}

	return IS_ERR(cpu_clk) ? PTR_ERR(cpu_clk) : 0;
}

static int (*rtl8277c_clk_setup_fns[])(struct device_node *np,
			      struct rtl8277c_clk_reg *clk_reg,
			      struct rtl8277c_clk_param *clk_param) = {
	[CA_CLK_PLL]          = rtl8277c_clk_pll,
	[CA_CLK_DIVIDER]      = rtl8277c_clk_divider,
	[CA_CLK_MUX]          = rtl8277c_clk_mux,
	[CA_CLK_GATE]         = rtl8277c_clk_gate,
	[CA_CLK_FIXED_FACTOR] = rtl8277c_clk_fixed_factor,
	[CA_CLK_PE_DAEMON]    = rtl8277c_clk_pe_daemon,
	[CA_CLK_CPU_DAEMON]   = rtl8277c_clk_cpu_daemon,
};

static void __init rtl8277c_soc_clks_setup(struct device_node *np)
{
	struct rtl8277c_clk_reg *clk_reg;
	struct device_node *childnp;
	const struct of_device_id *clk_id;
	struct rtl8277c_clk_param *clk_param;

	clk_reg = kzalloc(sizeof(*clk_reg), GFP_KERNEL);
	if (!clk_reg)
		return;

	clk_reg->regbase = of_iomap(np, 0);
	if (!clk_reg->regbase) {
		kfree(clk_reg);
		return;
	}
	spin_lock_init(&clk_reg->regs_lock);

	for_each_child_of_node(np, childnp) {
		clk_id = of_match_node(rtl8277c_soc_clks_ids, childnp);
		if (!clk_id)
			continue;

		clk_param = (struct rtl8277c_clk_param *)clk_id->data;
		if (rtl8277c_clk_setup_fns[clk_param->type](childnp, clk_reg,
							  clk_param))
			pr_err("rtl8277c_clk_setup_fns(%s) fail!\n", np->name);
	}
	printk("RTK SOC: soc_clks_setup finished.\n");
}

module_param(readonly, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(readonly, "CPU daemon is readonly");

CLK_OF_DECLARE(rtl8277c_soc_clks, "cortina,rtl8277c-soc-clks",
	       rtl8277c_soc_clks_setup);
