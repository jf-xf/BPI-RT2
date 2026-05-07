/*
 * Copyright (c) Cortina-Access Limited 2015.  All rights reserved.
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
/*
 * realtek-soc-a.c: Realtek SoC driver for CA serial
 *
 */

#include <soc/cortina/cortina-soc.h>
#include <soc/cortina/rtk_8277bc_pll.h>

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <soc/cortina/cortina-smc.h>
#include <soc/cortina/registers.h>

#include <linux/arm-smccc.h>

#define MHZ	(1000000)


#if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
#define IS_FPGA_BOARD 1
#else
#define IS_FPGA_BOARD 0
#endif
#define CRYPTO_DIVSEL_MASK		GENMASK(6,0)

struct proc_dir_entry *ca_proc_dir = NULL;
EXPORT_SYMBOL(ca_proc_dir);

struct ca_soc_data *ca_soc = NULL;

/* */
u64 ca_smc_call(unsigned int smc_id, unsigned long x1,unsigned long x2, \
		unsigned long x3, unsigned long x4)
{
	struct arm_smccc_res res;

	if(x1==0){
		printk("Illegal SMC call(Address 0)!\n");
		while(1);
	}

	arm_smccc_smc(smc_id, x1, x2, x3, 0, 0, 0, 0, &res);
	return res.a0;
}
EXPORT_SYMBOL(ca_smc_call);

static void __ca_clock_dump(void)
{
	if (ca_soc->cpll != 0)
		dev_info(&ca_soc->pdev->dev, "CPLL: %u.%02u MHZ\n", ca_soc->cpll / MHZ, (ca_soc->cpll / 10000) % 100);

	if (ca_soc->epll != 0)
		dev_info(&ca_soc->pdev->dev, "EPLL: %u.%02u MHZ\n", ca_soc->epll / MHZ, (ca_soc->epll / 10000) % 100);
	if (ca_soc->fpll != 0)
		dev_info(&ca_soc->pdev->dev, "FPLL: %u.%02u MHZ\n", ca_soc->fpll / MHZ, (ca_soc->fpll / 10000) % 100);

	if (ca_soc->cpu_clk != 0)
		dev_info(&ca_soc->pdev->dev, "CPU Clock: %u.%02u MHZ\n", ca_soc->cpu_clk / MHZ, (ca_soc->cpu_clk / 10000) % 100);
	if (ca_soc->core_clk != 0)
		dev_info(&ca_soc->pdev->dev, "Core Clock: %u.%02u MHZ\n", ca_soc->core_clk / MHZ, (ca_soc->core_clk / 10000) % 100);
	if (ca_soc->eaxi_clk != 0)
		dev_info(&ca_soc->pdev->dev, "EAXI Clock: %u.%02u MHZ\n", ca_soc->eaxi_clk / MHZ, (ca_soc->eaxi_clk / 10000) % 100);

	if (ca_soc->lsaxi_clk != 0)
		dev_info(&ca_soc->pdev->dev, "LSAXI Clock: %u.%02u MHZ\n", ca_soc->lsaxi_clk / MHZ, (ca_soc->lsaxi_clk / 10000) % 100);
	if (ca_soc->hsaxi_clk != 0)
		dev_info(&ca_soc->pdev->dev, "HSAXI Clock: %u.%02u MHZ\n", ca_soc->hsaxi_clk / MHZ, (ca_soc->hsaxi_clk / 10000) % 100);
	if (ca_soc->cci_clk != 0)
		dev_info(&ca_soc->pdev->dev, "CCI Clock: %u.%02u MHZ\n", ca_soc->cci_clk / MHZ, (ca_soc->cci_clk / 10000) % 100);
	if (ca_soc->crypto_clk != 0)
		dev_info(&ca_soc->pdev->dev, "Crypto Clock: %u.%02u MHZ\n", ca_soc->crypto_clk / MHZ, (ca_soc->crypto_clk / 10000) % 100);
	if (ca_soc->atb_clk != 0)
		dev_info(&ca_soc->pdev->dev, "ATB Clock: %u.%02u MHZ\n", ca_soc->atb_clk / MHZ, (ca_soc->atb_clk / 10000) % 100);
	if (ca_soc->pe_clk != 0)
		dev_info(&ca_soc->pdev->dev, "PE Clock: %u.%02u MHZ\n", ca_soc->pe_clk / MHZ, (ca_soc->pe_clk / 10000) % 100);
	if (ca_soc->peaxi_clk != 0)
		dev_info(&ca_soc->pdev->dev, "PE AXI Clock: %u.%02u MHZ\n", ca_soc->peaxi_clk / MHZ, (ca_soc->peaxi_clk / 10000) % 100);
}

#if IS_FPGA_BOARD
static void __ca_clock_get(void)
{
	uint32_t ref_clk = 25 * MHZ;

	ca_soc->cpll = ref_clk;
	ca_soc->epll = ref_clk;
	ca_soc->fpll = ref_clk;
	ca_soc->cpu_clk = ref_clk;
	ca_soc->cci_clk = ref_clk;
	ca_soc->lsaxi_clk = ref_clk;
	ca_soc->hsaxi_clk = ref_clk;
#if defined(CONFIG_ARCH_REALTEK_TAURUS)
	ca_soc->core_clk = 20 * MHZ;
#else
	ca_soc->core_clk = ref_clk;
#endif
	ca_soc->crypto_clk = ref_clk;
	ca_soc->eaxi_clk = ref_clk;
	ca_soc->atb_clk = ref_clk;
	ca_soc->pe_clk = ref_clk;
	ca_soc->peaxi_clk = ref_clk;
}
#else /* Not FPGA */
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/clk.h>
//extern struct clk *cpu_daemon_clk;
struct clk *cpu_daemon_clk = NULL;
/**************************************
 * VENUS: 8277B 
 **************************************/
static void __ca_clock_get_b(void){
	uint32_t ref_clk = 25 * MHZ;
	B_GLOBAL_CPLL0_t cpll0;
	B_GLOBAL_EPLL0_t epll0;
	B_GLOBAL_FPLL0_t fpll0;
	B_GLOBAL_CPLLDIV_t cplldiv;
	B_GLOBAL_EPLLDIV_t eplldiv;
	B_GLOBAL_EPLLDIV2_t eplldiv2;
	B_GLOBAL_PEDIV_t pediv;
	B_GLOBAL_STRAP_t strap;

	/* GLOBAL_STRAP */
	strap.wrd = readl(ca_soc->strap_base);

	/* CPLL */
	cpll0.wrd = readl(ca_soc->clk_base + OFF_CPLL0);
	if (cpll0.bf.DIV_PREDIV_BPS)
		ca_soc->cpll = ref_clk * (cpll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->cpll = ref_clk / (cpll0.bf.DIV_PREDIV_SEL + 2) * (cpll0.bf.SCPU_DIV_DIVN + 3);

	/* EPLL */
	epll0.wrd = readl(ca_soc->clk_base + OFF_EPLL0);
	if (epll0.bf.DIV_PREDIV_BPS)
		ca_soc->epll = ref_clk * (epll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->epll = ref_clk / (epll0.bf.DIV_PREDIV_SEL + 2) * (epll0.bf.SCPU_DIV_DIVN + 3);

	/* FPLL */
	fpll0.wrd = readl(ca_soc->clk_base + OFF_FPLL0);

	if (fpll0.bf.DIV_PREDIV_BPS)
		ca_soc->fpll = ref_clk * (fpll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->fpll = ref_clk / (fpll0.bf.DIV_PREDIV_SEL + 2) * (fpll0.bf.SCPU_DIV_DIVN + 3);

	/* CPLL related */
	cplldiv.wrd = readl(ca_soc->clk_base + OFF_CPLLDIV);

	if(cpu_daemon_clk){
		ca_soc->cpu_clk = clk_get_rate(cpu_daemon_clk);
	}else
	if (cplldiv.bf.cpll_div_override) {
		/* use CPLL to get CPU clock */
		if (cplldiv.bf.cf_sel) {
			if (cplldiv.bf.f2c_divsel != 0)
				ca_soc->cpu_clk = ca_soc->fpll * 2 / cplldiv.bf.f2c_divsel;
			else
				ca_soc->cpu_clk = ca_soc->fpll;
		} else {
			if (cplldiv.bf.cortex_divsel != 0)
				ca_soc->cpu_clk = ca_soc->cpll * 2 / cplldiv.bf.cortex_divsel;
			else
				ca_soc->cpu_clk = ca_soc->cpll;
		}
	} else {
		/* use STRAP pin to get CPU clock */
		switch(strap.bf.speed) {
		case 0:
			ca_soc->cpu_clk = 400 * MHZ;
			break;
		case 1:
			ca_soc->cpu_clk = 625 * MHZ;
			break;
		case 2:
			ca_soc->cpu_clk = 700 * MHZ;
			break;
		case 3:
			ca_soc->cpu_clk = 800 * MHZ;
			break;
		case 4:
			ca_soc->cpu_clk = 900 * MHZ;
			break;
		case 5:
			ca_soc->cpu_clk = 1000 * MHZ;
			break;
		case 6:
			ca_soc->cpu_clk = 1100 * MHZ;
			break;
		case 7:
			ca_soc->cpu_clk = 1200 * MHZ;
			break;
		}
	}

	/* EPLL related */
	eplldiv.wrd = readl(ca_soc->clk_base + OFF_EPLLDIV);
	eplldiv2.wrd = readl(ca_soc->clk_base + OFF_EPLLDIV2);

	ca_soc->cci_clk = ca_soc->epll/((eplldiv2.wrd >>8)/2);


	if (eplldiv.bf.lsaxi_divsel != 0)
		ca_soc->lsaxi_clk = ca_soc->epll * 2 / eplldiv.bf.lsaxi_divsel;
	else
		ca_soc->lsaxi_clk = ca_soc->epll; /* impossible */

	ca_soc->hsaxi_clk = ca_soc->lsaxi_clk << 1;

	if (eplldiv.bf.core_divsel != 0){
		ca_soc->core_clk = ca_soc->epll * 2 / eplldiv.bf.core_divsel;
	}else{
		ca_soc->core_clk = ca_soc->epll;  /* impossible */
	}

		if ((eplldiv.wrd & CRYPTO_DIVSEL_MASK) != 0){
			ca_soc->crypto_clk = ca_soc->epll * 2 / (eplldiv.wrd & CRYPTO_DIVSEL_MASK);
		}else{
			ca_soc->crypto_clk = ca_soc->epll; /* impossible */
		}

	/* FPLL related */
	pediv.wrd = readl(ca_soc->clk_base + OFF_PEDIV);
	if (pediv.bf.offload_sel) {
		if (pediv.bf.c2f_divsel != 0)
			ca_soc->pe_clk = ca_soc->cpll * 2 / pediv.bf.c2f_divsel;
		else
			ca_soc->pe_clk = ca_soc->cpll;
	} else {
		if (pediv.bf.f_divsel != 0)
			ca_soc->pe_clk = ca_soc->fpll * 2 / pediv.bf.f_divsel;
		else
			ca_soc->pe_clk = ca_soc->fpll;
	}

	if (pediv.bf.peaxi_fullspeedsel)
		ca_soc->peaxi_clk = ca_soc->pe_clk;
	else
		ca_soc->peaxi_clk = ca_soc->pe_clk / 2;
}
/**************************************
 * TAURUS: 8277C
 **************************************/
static void __ca_clock_get_c(void){
	uint32_t ref_clk = 25 * MHZ;
	C_GLOBAL_CPLL0_t cpll0;
	C_GLOBAL_EPLL0_t epll0;
	C_GLOBAL_FPLL0_t fpll0;
	C_GLOBAL_CPLLDIV_t cplldiv;
	C_GLOBAL_EPLLDIV_t eplldiv;
	C_GLOBAL_EPLLDIV2_t eplldiv2;
	C_GLOBAL_PEDIV_t pediv;
	C_GLOBAL_STRAP_t strap;

	/* GLOBAL_STRAP */
	strap.wrd = readl(ca_soc->strap_base);

	/* CPLL */
	cpll0.wrd = readl(ca_soc->clk_base + OFF_CPLL0);
	if (cpll0.bf.DIV_PREDIV_BPS)
		ca_soc->cpll = ref_clk * (cpll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->cpll = ref_clk / (cpll0.bf.DIV_PREDIV_SEL + 2) * (cpll0.bf.SCPU_DIV_DIVN + 3);

	/* EPLL */
	epll0.wrd = readl(ca_soc->clk_base + OFF_EPLL0);
	if (epll0.bf.DIV_PREDIV_BPS)
		ca_soc->epll = ref_clk * (epll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->epll = ref_clk / (epll0.bf.DIV_PREDIV_SEL + 2) * (epll0.bf.SCPU_DIV_DIVN + 3);

	/* FPLL */
	fpll0.wrd = readl(ca_soc->clk_base + OFF_FPLL0);

	if (fpll0.bf.DIV_PREDIV_BPS)
		ca_soc->fpll = ref_clk * (fpll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->fpll = ref_clk / (fpll0.bf.DIV_PREDIV_SEL + 2) * (fpll0.bf.SCPU_DIV_DIVN + 3);

	/* CPLL related */
	cplldiv.wrd = readl(ca_soc->clk_base + OFF_CPLLDIV);

	if(cpu_daemon_clk){
		ca_soc->cpu_clk = clk_get_rate(cpu_daemon_clk);
	}else
	if (cplldiv.bf.cpll_div_override) {
		/* use CPLL to get CPU clock */
		if (cplldiv.bf.cf_sel) {
			if (cplldiv.bf.f2c_divsel != 0)
				ca_soc->cpu_clk = ca_soc->fpll * 2 / cplldiv.bf.f2c_divsel;
			else
				ca_soc->cpu_clk = ca_soc->fpll;
		} else {
			if (cplldiv.bf.cortex_divsel != 0)
				ca_soc->cpu_clk = ca_soc->cpll * 2 / cplldiv.bf.cortex_divsel;
			else
				ca_soc->cpu_clk = ca_soc->cpll;
		}
	} else {
		/* use STRAP pin to get CPU clock */
		switch(strap.bf.speed) {
		case 0:
			ca_soc->cpu_clk = 400 * MHZ;
			break;
		case 1:
			ca_soc->cpu_clk = 625 * MHZ;
			break;
		case 2:
			ca_soc->cpu_clk = 700 * MHZ;
			break;
		case 3:
			ca_soc->cpu_clk = 800 * MHZ;
			break;
		case 4:
			ca_soc->cpu_clk = 900 * MHZ;
			break;
		case 5:
			ca_soc->cpu_clk = 1000 * MHZ;
			break;
		case 6:
			ca_soc->cpu_clk = 1100 * MHZ;
			break;
		case 7:
			ca_soc->cpu_clk = 1200 * MHZ;
			break;
		}
	}

	/* EPLL related */
	eplldiv.wrd = readl(ca_soc->clk_base + OFF_EPLLDIV);
	eplldiv2.wrd = readl(ca_soc->clk_base + OFF_EPLLDIV2);

	ca_soc->cci_clk = ca_soc->epll/((eplldiv2.wrd >>8)/2);


	if (eplldiv.bf.lsaxi_divsel != 0)
		ca_soc->lsaxi_clk = ca_soc->epll * 2 / eplldiv.bf.lsaxi_divsel;
	else
		ca_soc->lsaxi_clk = ca_soc->epll; /* impossible */

	ca_soc->hsaxi_clk = ca_soc->lsaxi_clk << 1;

	if (eplldiv.bf.core_divsel != 0)
		ca_soc->core_clk = ca_soc->epll * 2 / eplldiv.bf.core_divsel;
	else
		ca_soc->core_clk = ca_soc->epll;  /* impossible */


	/* FPLL related */
	pediv.wrd = readl(ca_soc->clk_base + OFF_PEDIV);
	if (pediv.bf.offload_sel) {
		if (pediv.bf.c2f_divsel != 0)
			ca_soc->pe_clk = ca_soc->cpll * 2 / pediv.bf.c2f_divsel;
		else
			ca_soc->pe_clk = ca_soc->cpll;
	} else {
		if (pediv.bf.f_divsel != 0)
			ca_soc->pe_clk = ca_soc->fpll * 2 / pediv.bf.f_divsel;
		else
			ca_soc->pe_clk = ca_soc->fpll;
	}

	if (pediv.bf.peaxi_fullspeedsel)
		ca_soc->peaxi_clk = ca_soc->pe_clk;
	else
		ca_soc->peaxi_clk = ca_soc->pe_clk / 2;
}


/**************************************
 * ELNATH(9607F)
 **************************************/
 enum CLK_TYPE {
	CPLL = 0,
	EPLL,
	FPLL,
	CPU,
	CORE,
	LSAXI,
	HSAXI,
	PE,
	PEAXI,
	CLK_TYPE_MAX,
};

u32 soc_default_clk[CLK_TYPE_MAX] = {
	1000000000,
	2000000000,
	1600000000,
	1000000000,
	 333000000,
	 200000000,
	 400000000,
	 800000000,
		 0,
};
static void __ca_clock_get_9607f(void){
	uint32_t ref_clk = 25 * MHZ;
	F_GLOBAL_CPLL0_t cpll0;
	F_GLOBAL_EPLL0_t epll0;
	F_GLOBAL_FPLL0_t fpll0;
	F_GLOBAL_CPLLDIV_t cplldiv;
	F_GLOBAL_EPLLDIV_t eplldiv;
	F_GLOBAL_EPLLDIV2_t eplldiv2;
	F_GLOBAL_PEDIV_t pediv;
	F_GLOBAL_STRAP_t strap;

	/* GLOBAL_STRAP */
	strap.wrd = readl(ca_soc->strap_base);
	if(strap.bf.xtal_sel){
		ref_clk = 40 * MHZ;
	}
	/* CPLL */
	cpll0.wrd = readl(ca_soc->clk_base + OFF_CPLL0);
	if(cplldiv.bf.cpll_mode_override){
		/* CPLL */
		cpll0.wrd = readl(ca_soc->clk_base + (F_GLOBAL_CPLL0 - F_GLOBAL_CPLLDIV));
		if (cpll0.bf.DIV_PREDIV_BPS){
			ca_soc->cpll = ref_clk * (cpll0.bf.SCPU_DIV_DIVN + 3);
		}else{
			ca_soc->cpll = ref_clk / (cpll0.bf.DIV_PREDIV_SEL + 2) * (cpll0.bf.SCPU_DIV_DIVN + 3);
		}
	}else{
			ca_soc->cpll =  soc_default_clk[CPLL];
	}
	/* EPLL */
	epll0.wrd = readl(ca_soc->clk_base + OFF_EPLL0);
	if (epll0.bf.DIV_PREDIV_BPS)
		ca_soc->epll = ref_clk * (epll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->epll = ref_clk / (epll0.bf.DIV_PREDIV_SEL + 2) * (epll0.bf.SCPU_DIV_DIVN + 3);

	/* FPLL */
	fpll0.wrd = readl(ca_soc->clk_base + OFF_FPLL0);

	if (fpll0.bf.DIV_PREDIV_BPS)
		ca_soc->fpll = ref_clk * (fpll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->fpll = ref_clk / (fpll0.bf.DIV_PREDIV_SEL + 2) * (fpll0.bf.SCPU_DIV_DIVN + 3);

	/* CPLL related */
	cplldiv.wrd = readl(ca_soc->clk_base + OFF_CPLLDIV);

	if(cpu_daemon_clk){
		ca_soc->cpu_clk = clk_get_rate(cpu_daemon_clk);
	}else
	if (cplldiv.bf.cpll_div_override) {
		/* use CPLL to get CPU clock */
		if (cplldiv.bf.cf_sel) {
			if (cplldiv.bf.f2c_divsel != 0)
				ca_soc->cpu_clk = ca_soc->fpll * 2 / cplldiv.bf.f2c_divsel;
			else
				ca_soc->cpu_clk = ca_soc->fpll;
		} else {
			if (cplldiv.bf.cortex_divsel != 0)
				ca_soc->cpu_clk = ca_soc->cpll * 2 / cplldiv.bf.cortex_divsel;
			else
				ca_soc->cpu_clk = ca_soc->cpll;
		}
	} else {
		/* use STRAP pin to get CPU clock */
		switch(strap.bf.speed) {
		case 0:
			ca_soc->cpu_clk = 500 * MHZ;
			break;
		case 1:
			ca_soc->cpu_clk = 600 * MHZ;
			break;
		case 2:
			ca_soc->cpu_clk = 1000 * MHZ;
			break;
		case 3:
			ca_soc->cpu_clk = 1200 * MHZ;
			break;
		}
	}

	/* EPLL related */
	eplldiv.wrd = readl(ca_soc->clk_base + OFF_EPLLDIV);
	eplldiv2.wrd = readl(ca_soc->clk_base + OFF_EPLLDIV2);

	/* 9607F no CCI */
	ca_soc->cci_clk = 0;


	if (eplldiv.bf.lsaxi_divsel != 0)
		ca_soc->lsaxi_clk = ca_soc->epll * 2 / eplldiv.bf.lsaxi_divsel;
	else
		ca_soc->lsaxi_clk = ca_soc->epll; /* impossible */

	ca_soc->hsaxi_clk = ca_soc->lsaxi_clk << 1;

	if (eplldiv.bf.core_divsel != 0)
		ca_soc->core_clk = ca_soc->epll * 2 / eplldiv.bf.core_divsel;
	else
		ca_soc->core_clk = ca_soc->epll;  /* impossible */


	/* FPLL related */
	pediv.wrd = readl(ca_soc->clk_base + OFF_PEDIV);
	if (pediv.bf.offload_sel) {
		if (pediv.bf.c2f_divsel != 0)
			ca_soc->pe_clk = ca_soc->cpll * 2 / pediv.bf.c2f_divsel;
		else
			ca_soc->pe_clk = ca_soc->cpll;
	} else {
		if (pediv.bf.f_divsel != 0)
			ca_soc->pe_clk = ca_soc->fpll * 2 / pediv.bf.f_divsel;
		else
			ca_soc->pe_clk = ca_soc->fpll;
	}

	if (pediv.bf.peaxi_fullspeedsel)
		ca_soc->peaxi_clk = ca_soc->pe_clk;
	else
		ca_soc->peaxi_clk = ca_soc->pe_clk / 2;
}

static void __ca_clock_get(void)
{
	if(IS_TAURUS){
		__ca_clock_get_c();
	}else if(IS_ELNATH){
		__ca_clock_get_9607f();
	}else{
		__ca_clock_get_b();
	}
}

#endif /* IS_FPGA_BOARD */

#if (IS_FPGA_BOARD == 0)
extern int plat_soc_get_chipmode(u8 *buf, int buflen);
#endif

static void __ca_jtag_id_dump(void)
{
#if (IS_FPGA_BOARD == 0)
	unsigned short chip_mode = 0;
#endif
	dev_info(&ca_soc->pdev->dev, "Vendor ID: 0x%03x\n", ca_soc->vendor_id);
	dev_info(&ca_soc->pdev->dev, "Chip ID: 0x%04x\n", ca_soc->chip_id);
	dev_info(&ca_soc->pdev->dev, "Chip Rev: %u\n", ca_soc->chip_revision);
#if (IS_FPGA_BOARD == 0)
	plat_soc_get_chipmode((u8 *)&chip_mode, sizeof(chip_mode));
	dev_info(&ca_soc->pdev->dev, "Chip Mode: 0x%02x\n", chip_mode);
#endif
}

static void __ca_jtag_id_get(void)
{
	uint32_t val;

	val = readl(ca_soc->jtag_id_base);

	ca_soc->chip_id = CA_SOC_CHIP_ID(val);

	if((ca_soc->chip_id == CHIP_CA8277B) || (ca_soc->chip_id == CHIP_CA8289)){
		ca_soc->vendor_id = CA_SOC_VENDOR_ID(val);
		ca_soc->chip_id = CA_SOC_CHIP_ID(val);
		ca_soc->chip_revision = 0; //next step, assigned by get_chip_revision
	}else{
		/* TODO SMC CALL */
		ca_soc->vendor_id = RTK_SOC_VENDOR_ID_GET(val);
		ca_soc->chip_id = RTK_SOC_CHIP_ID_GET(val);
		ca_soc->chip_revision = 0;
	}
}

#if (IS_FPGA_BOARD == 0)
unsigned char get_chip_revision(void)
{
       unsigned int revision = 0;
       /* use Secure Monitor Call to retrieve chip revision.
        */
       revision = CA_SMC_CALL_ROM_VERSION();
       if ((revision < 'A') || (revision == 0xffffffff)) {
               printk("\033[0;31mWarning: Can't get CHIP Revision, so use default value(A).\033[0m\n");
               printk("\033[0;31mPlease update bootloader to the recent version!\033[0m\n");
               revision = 'A';
       }
       return (unsigned char)revision;
}
#endif

#define REASON_MASK 0xf
static void *glbl_software;
void rtk_soc_reboot_reason_set(int reason) {
	u32 val;
	if(glbl_software) {
		val = readl(glbl_software);
		val = (val & ~REASON_MASK) | (reason & REASON_MASK);
		writel(val, glbl_software);
	}
}
EXPORT_SYMBOL(rtk_soc_reboot_reason_set);

int rtk_soc_reboot_reason_get(void) {
	if (unlikely(!glbl_software)) {
		glbl_software = ioremap(GLOBAL_SOFTWARE, 4);
	}

	if (glbl_software)
		return (readl(glbl_software) & REASON_MASK);
	else
		return -1;
}

static int ca_soc_probe(struct platform_device *pdev)
{
	struct resource *res;

	ca_soc = devm_kzalloc(&pdev->dev, sizeof(*ca_soc), GFP_KERNEL);
	if (!ca_soc)
		return -ENOMEM;

	ca_soc->pdev = pdev;



	/*** JTAG ID, mem resource #0 ***/

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "JTAG_ID");
	if (!res) {
		dev_err(&pdev->dev, "cannot get resource JTAG_ID\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource JTAG_ID\n");
		return -EBUSY;
	}

	ca_soc->jtag_id_base = ioremap(res->start, resource_size(res));
	if (!ca_soc->jtag_id_base) {
		dev_err(&pdev->dev, "cannot ioremap resource JTAG_ID\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}

	release_resource(res);
	kfree(res);

#if (IS_FPGA_BOARD == 0)
	/*** Clocks, mem resource #1 ***/

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "CLOCK");
	if (!res) {
		dev_err(&pdev->dev, "cannot get resource CLOCK\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource CLOCK\n");
		return -EBUSY;
	}

	ca_soc->clk_base = ioremap(res->start, resource_size(res));
	if (!ca_soc->clk_base) {
		dev_err(&pdev->dev, "cannot ioremap resource CLOCK\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}

	release_resource(res);
	kfree(res);

	/*** GLOBAL_STRAP, mem resource #2 ***/
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "STRAP");
	if (!res) {
		dev_err(&pdev->dev, "cannot get resource STRAP\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource STRAP\n");
		return -EBUSY;
	}

	ca_soc->strap_base = ioremap(res->start, resource_size(res));
	if (!ca_soc->strap_base) {
		dev_err(&pdev->dev, "cannot ioremap resource STRAP\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}

	release_resource(res);
	kfree(res);

#endif /* IS_FPGA_BOARD */

	__ca_jtag_id_get();



#if (IS_FPGA_BOARD == 0)
	ca_soc->chip_revision = get_chip_revision();
#endif

	__ca_clock_get();

	ca_proc_dir = proc_mkdir("driver/cortina", NULL);

	__ca_jtag_id_dump();
	__ca_clock_dump();

	return 0;
}

unsigned int rtk_soc_chipid_get(void){
	if (ca_soc == NULL) return -EPERM;

	return ca_soc->chip_id;
}
EXPORT_SYMBOL(rtk_soc_chipid_get);


int ca_soc_data_get(struct ca_soc_data *data)
{
	if (data == NULL) return -EINVAL;
	if (ca_soc == NULL) return -EPERM;

	memcpy(data, ca_soc, sizeof(*ca_soc));
	return 0;
}
EXPORT_SYMBOL(ca_soc_data_get);

static int ca_soc_remove(struct platform_device *pdev)
{
	iounmap(ca_soc->jtag_id_base);

	return 0;
}

static const struct of_device_id ca_soc_dt_match[] = {
	{ .compatible = "cortina,venus", },
	{ .compatible = "realtek,taurus", },
	{ .compatible = "realtek,9607f", },
	{ },
};

MODULE_DEVICE_TABLE(of, ca_soc_dt_match);

static struct platform_driver ca_soc_driver = {
	.driver = {
		.name		= "ca_soc",
		.owner		= THIS_MODULE,
		.of_match_table	= ca_soc_dt_match,
	},
	.probe = ca_soc_probe,
	.remove	= ca_soc_remove,
};

module_platform_driver(ca_soc_driver);

MODULE_AUTHOR("RealTEK");
MODULE_DESCRIPTION("REALTEK SoC driver");
MODULE_LICENSE("GPL v2");

