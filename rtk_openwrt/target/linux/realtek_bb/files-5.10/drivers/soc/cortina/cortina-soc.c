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
 * cortina-soc.c: Cortina SoC driver
 *
 */

#include <soc/cortina/cortina-soc.h>
#include <soc/cortina/cortina-smc.h>
#include <soc/cortina/registers.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/arm-smccc.h>

#define MHZ	(1000000)

struct proc_dir_entry *ca_proc_dir = NULL;
EXPORT_SYMBOL(ca_proc_dir);

struct ca_soc_data *ca_soc = NULL;

#ifdef CONFIG_ARCH_CORTINA_G3LITE
void __iomem *rtl8198f_peri_reg_base = NULL;
void __iomem *rtl8198f_glb_reg_base = NULL;

/* porting form dtsi */
#define CA_SOC_JTAG_ID_BASE_OFFSET      0
#define CA_SOC_CLK_BASE_OFFSET          0xfc
#define CA_SOC_STRAPE_BASE_OFFSET       0x18
#define CA_SOC_FW_CTRL_BASE_OFFSET      0x528

void *rtl8198f_peri_reg_iobase = NULL;
void rtl8198f_peri_reg_base_init(void *iobase)
{
	rtl8198f_peri_reg_iobase = iobase;
}

void *rtl8198f_glb_reg_iobase = NULL;
void rtl8198f_glb_reg_base_init(void *iobase)
{
	rtl8198f_glb_reg_iobase = iobase;
}

static const struct of_device_id ca_soc_dt_match[];
#endif

/* Structure for Chip version check */
#define VER_STR_LEN 20
struct rom_version {
    unsigned char tapeout_version;
    uintptr_t version_str_addr; /* Address of version string */
    unsigned char version_str[VER_STR_LEN]; /* Version string */
};
#ifdef CONFIG_ARCH_CORTINA_G3HGU
struct rom_version saturn_revision_string[]= {
    {'C', 0xf3954080, "v1.3(release):957a6b"},  /* Saturn REV-C */
    {'B', 0xf39539c0, "v1.3(release):be66b2"}   /* Saturn REV-B */
};
#else
#if defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU)
struct rom_version saturn_revision_string[]= {
	{'C', 0x42054080, "v1.3(release):957a6b"},      /* Saturn REV-C */
	{'B', 0x420539c0, "v1.3(release):be66b2"}       /* Saturn REV-B */
};
#endif	/* defined(CONFIG_ARCH_CORTINA_SATURN_SFU) ... */
#endif	/* CONFIG_ARCH_CORTINA_G3HGU */

static void __ca_clock_dump(void)
{
	if (ca_soc->cpll != 0)
		dev_info(&ca_soc->pdev->dev, "CPLL: %u.%02u MHZ\n", ca_soc->cpll / MHZ, (ca_soc->cpll / 10000) % 100);
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU)|| defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
	if (ca_soc->epll != 0)
		dev_info(&ca_soc->pdev->dev, "EPLL: %u.%02u MHZ\n", ca_soc->epll / MHZ, (ca_soc->epll / 10000) % 100);
	if (ca_soc->fpll != 0)
		dev_info(&ca_soc->pdev->dev, "FPLL: %u.%02u MHZ\n", ca_soc->fpll / MHZ, (ca_soc->fpll / 10000) % 100);
#endif

	if (ca_soc->cpu_clk != 0)
		dev_info(&ca_soc->pdev->dev, "CPU Clock: %u.%02u MHZ\n", ca_soc->cpu_clk / MHZ, (ca_soc->cpu_clk / 10000) % 100);
	if (ca_soc->core_clk != 0)
		dev_info(&ca_soc->pdev->dev, "Core Clock: %u.%02u MHZ\n", ca_soc->core_clk / MHZ, (ca_soc->core_clk / 10000) % 100);
	if (ca_soc->eaxi_clk != 0)
		dev_info(&ca_soc->pdev->dev, "EAXI Clock: %u.%02u MHZ\n", ca_soc->eaxi_clk / MHZ, (ca_soc->eaxi_clk / 10000) % 100);
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
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
#endif

#if defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU)
	if (ca_soc->io_clk != 0)
		dev_info(&ca_soc->pdev->dev, "I/O Clock: %u.%02u MHZ\n", ca_soc->io_clk / MHZ, (ca_soc->io_clk / 10000) % 100);
	if (ca_soc->eth_ref2_clk != 0)
		dev_info(&ca_soc->pdev->dev, "Eth Ref 2 Clock: %u.%02u MHZ\n", ca_soc->eth_ref2_clk / MHZ, (ca_soc->eth_ref2_clk / 10000) % 100);
#endif
}

#if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_CORTINA_BOARD_SATURN_SFU_TITAN) || defined(CONFIG_REALTEK_BOARD_FPGA)
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
#else /* CONFIG_CORTINA_BOARD_FPGA || CONFIG_CORTINA_BOARD_SATURN_SFU_TITAN */
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/clk.h>
//extern struct clk *cpu_daemon_clk;
__weak struct clk *cpu_daemon_clk;

static void __ca_clock_get(void)
{
	uint32_t ref_clk = 25 * MHZ;
	GLOBAL_CPLL0_t cpll0;
	GLOBAL_EPLL0_t epll0;
	GLOBAL_FPLL0_t fpll0;
	GLOBAL_CPLLDIV_t cplldiv;
	GLOBAL_EPLLDIV_t eplldiv;
	GLOBAL_EPLLDIV2_t eplldiv2;
	GLOBAL_PEDIV_t pediv;
	GLOBAL_STRAP_t strap;

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

#if defined(CONFIG_CORTINA_BOARD_VENUS_ENG) || defined(CONFIG_REALTEK_BOARD_TAURUS_ENG)
	ca_soc->cci_clk = ca_soc->epll/((eplldiv2.wrd >>8)/2);
#else
	if (cplldiv.bf.cci_onetoone)
		ca_soc->cci_clk = ca_soc->cpu_clk;
	else
		ca_soc->cci_clk = ca_soc->cpu_clk / 2;
#endif

	if (eplldiv.bf.lsaxi_divsel != 0)
		ca_soc->lsaxi_clk = ca_soc->epll * 2 / eplldiv.bf.lsaxi_divsel;
	else
		ca_soc->lsaxi_clk = ca_soc->epll; /* impossible */
#if defined(CONFIG_CORTINA_BOARD_VENUS_ENG) || defined(CONFIG_REALTEK_BOARD_TAURUS_ENG)
	ca_soc->hsaxi_clk = ca_soc->lsaxi_clk << 1;
#else
	if (eplldiv.bf.hsaxi_divsel != 0)
		ca_soc->hsaxi_clk = ca_soc->epll * 2 / eplldiv.bf.hsaxi_divsel;
	else
		ca_soc->hsaxi_clk = ca_soc->epll; /* impossible */
#endif
	if (eplldiv.bf.core_divsel != 0)
		ca_soc->core_clk = ca_soc->epll * 2 / eplldiv.bf.core_divsel;
	else
		ca_soc->core_clk = ca_soc->epll;  /* impossible */

	if (eplldiv.bf.crypto_divsel != 0)
		ca_soc->crypto_clk = ca_soc->epll * 2 / eplldiv.bf.crypto_divsel;
	else
		ca_soc->crypto_clk = ca_soc->epll; /* impossible */
#if !defined(CONFIG_CORTINA_BOARD_VENUS_ENG ) && !defined(CONFIG_REALTEK_BOARD_TAURUS_ENG)
	if (eplldiv2.bf.eaxi_divsel != 0)
		ca_soc->eaxi_clk = ca_soc->epll * 2 / eplldiv2.bf.eaxi_divsel;
	else
		ca_soc->eaxi_clk = ca_soc->epll;

	if (eplldiv2.bf.atb_divsel != 0)
		ca_soc->atb_clk = ca_soc->epll * 2 / eplldiv2.bf.atb_divsel;
	else
		ca_soc->atb_clk = ca_soc->epll;
#endif
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

#elif defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU)
static void __ca_clock_get(void)
{
	uint32_t ref_clk = 25 * MHZ;
	CLKGEN_CPLL0_t cpll0;
	CLKGEN_CPLLDIV_t cplldiv;
	CLKGEN_CPLLDIV2_t cplldiv2;
	CLKGEN_TAROKO_AP_FREQ_CONF_t cpu_freq_conf;
	GLOBAL_STRAP_t strap;

	/* GLOBAL_STRAP */
	strap.wrd = readl(ca_soc->strap_base);

	/* CPLL */
	cpll0.wrd = readl(ca_soc->clk_base + OFF_CPLL0);
	if (cpll0.bf.DIV_PREDIV_BPS)
		ca_soc->cpll = ref_clk * (cpll0.bf.SCPU_DIV_DIVN + 3);
	else
		ca_soc->cpll = ref_clk / (cpll0.bf.DIV_PREDIV_SEL + 2) * (cpll0.bf.SCPU_DIV_DIVN + 3);

	/* CPLL related */
	cplldiv.wrd = readl(ca_soc->clk_base + OFF_CPLLDIV);
	cplldiv2.wrd = readl(ca_soc->clk_base + OFF_CPLLDIV2);
	cpu_freq_conf.wrd = readl(ca_soc->clk_base + OFF_AP_FREQ_CONF);

	if (cpu_freq_conf.bf.cpu_ap_speed_change_enable) {
		/* use AP_FREQ_CONF to get CPU clock */
		ca_soc->cpu_clk = ca_soc->cpll * 2 / cplldiv.bf.proc_clk_divsel;
		if(strap.bf.speed==0)
			ca_soc->cpu_clk = (2* 2000 *( MHZ /cpu_freq_conf.bf.cpu_ap_new_divsel));
		else
			ca_soc->cpu_clk = (2* 1000 *( MHZ /cpu_freq_conf.bf.cpu_ap_new_divsel));
	} else {
		/* use STRAP pin to get CPU clock */
		switch(strap.bf.speed) {
		case 0:
			ca_soc->cpu_clk = 2000 * MHZ / 3;
			break;
		case 1:
			ca_soc->cpu_clk = 500 * MHZ;
			break;
		case 2:
		case 3:
			ca_soc->cpu_clk = 200 * MHZ;
			break;
		}
	}

	if (cplldiv.bf.core_clk_div_override) {
		/* use CPLL to get core clock */
		if(strap.bf.speed==0)
			ca_soc->core_clk = 2*(2000 * MHZ /cplldiv.bf.core_clk_divsel);
		else
			ca_soc->core_clk = 2*(1000 * MHZ /cplldiv.bf.core_clk_divsel);
	} else {
		/* use STRAP pin to get core clock */
		ca_soc->core_clk = 1000 * MHZ / 3;
	}

	if (cplldiv2.bf.io_clk_div_override) {
		/* use CPLL to get I/O clock */
		ca_soc->io_clk = ca_soc->cpll * 2 / cplldiv2.bf.io_clk_divsel;
	} else {
		/* use STRAP pin to get I/O clock */
		switch(strap.bf.speed) {
		case 0:
		case 1:
			ca_soc->io_clk = 100 * MHZ;
			break;
		case 2:
			ca_soc->io_clk = 50 * MHZ;
			break;
		case 3:
			ca_soc->io_clk = 0 * MHZ;
			break;
		}
	}

	ca_soc->eaxi_clk = ca_soc->cpll * 2 / cplldiv2.bf.eaxi_divsel;

	if (cplldiv2.bf.eth_ref_2_clk_div_override) {
		/* use CPLL to get eth_ref2 clock */
		ca_soc->eth_ref2_clk = ca_soc->cpll * 2 / cplldiv2.bf.eth_ref_2_clk_divsel;
	} else {
		/* use STRAP pin to get eth_ref2 clock */
		switch(strap.bf.speed) {
		case 0:
		case 1:
		case 2:
			ca_soc->eth_ref2_clk = 250 * MHZ;
			break;
		case 3:
			ca_soc->eth_ref2_clk = 0 * MHZ;
			break;
		}
	}
}
#elif defined(CONFIG_ARCH_CORTINA_G3LITE)
static void __ca_clock_get(void)
{
	GLOBAL_FW_CTRL_t fw_ctrl;
	uint32_t ref_clk = 20 * MHZ;

	fw_ctrl.wrd = readl(ca_soc->clk_base);

	ca_soc->cpll = ref_clk;
	ca_soc->epll = ref_clk;
	ca_soc->fpll = ref_clk;
	ca_soc->cpu_clk = (700 + 50 * fw_ctrl.bf.fw_ck_cpu_freq_sel) * MHZ;
	ca_soc->cci_clk = 400 * MHZ;
	ca_soc->lsaxi_clk = 100 * MHZ;
	ca_soc->hsaxi_clk = 200 * MHZ;
	ca_soc->core_clk = 200 * MHZ;
	ca_soc->crypto_clk = 0;
	ca_soc->eaxi_clk = ref_clk;
	ca_soc->atb_clk = ref_clk;
	ca_soc->pe_clk = 600 * MHZ;
	ca_soc->peaxi_clk = 300 * MHZ;
}
#else /* CONFIG_ARCH_CORTINA_xxx */
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
	ca_soc->core_clk = ref_clk;
	ca_soc->crypto_clk = ref_clk;
	ca_soc->eaxi_clk = ref_clk;
	ca_soc->atb_clk = ref_clk;
	ca_soc->pe_clk = ref_clk;
	ca_soc->peaxi_clk = ref_clk;
}
#endif /* CONFIG_ARCH_CORTINA_xxx */
#endif /* CONFIG_CORTINA_BOARD_FPGA || CONFIG_CORTINA_BOARD_SATURN_SFU_TITAN */

#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_REALTEK_BOARD_TAURUS_ENG)
extern int plat_soc_get_chipmode(u8 *buf, int buflen);
#endif

static void __ca_jtag_id_dump(void)
{
#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_REALTEK_BOARD_TAURUS_ENG)
	unsigned short chip_mode = 0;
#endif
	dev_info(&ca_soc->pdev->dev, "Vendor ID: 0x%03x\n", ca_soc->vendor_id);
	dev_info(&ca_soc->pdev->dev, "Chip ID: 0x%04x\n", ca_soc->chip_id);
	dev_info(&ca_soc->pdev->dev, "Chip Rev: %u\n", ca_soc->chip_revision);
#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_REALTEK_BOARD_TAURUS_ENG)
	plat_soc_get_chipmode((u8 *)&chip_mode, sizeof(chip_mode));
	dev_info(&ca_soc->pdev->dev, "Chip Mode: 0x%02x\n", chip_mode);
#endif
#if defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU)
	dev_info(&ca_soc->pdev->dev, "PON Rev: %c\n", ca_soc->pon_revision);
#endif
}

static void __ca_jtag_id_get(void)
{
	uint32_t val;

	val = readl(ca_soc->jtag_id_base);

	ca_soc->vendor_id = CA_SOC_VENDOR_ID(val);
	ca_soc->chip_id = CA_SOC_CHIP_ID(val);
	ca_soc->chip_revision = CA_SOC_CHIP_REV(val);
}

#ifdef CONFIG_ARCH_CORTINA_VENUS
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

#if defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU) || defined(CONFIG_ARCH_CORTINA_G3HGU)
unsigned char get_saturn_revision(void)
{
	int i;
	struct rom_version *asic_rom_version;
	unsigned char *vaddr_rom_version, revision;

	/* Chip revision locates at end of ROM after REV-D */
#if defined(CONFIG_ARCH_CORTINA_G3HGU)
	vaddr_rom_version = ioremap(0xf395fffc, 4);
#else
	vaddr_rom_version = ioremap(0x4205fffc, 4);
#endif
	if(vaddr_rom_version==NULL){
		printk("Unable to Map SSB area!\n");
		return 'C';
	}
	if(*vaddr_rom_version != 0){
		revision = 'D' + *vaddr_rom_version - 0xd;
		iounmap(vaddr_rom_version);
		printk("Saturn revision=%x\n", revision);
		return revision;
	}

	/* Compare string */
	for(i=0; i< sizeof(saturn_revision_string)/sizeof(struct rom_version); i++){
		asic_rom_version = &saturn_revision_string[i];
		vaddr_rom_version = ioremap(asic_rom_version->version_str_addr, VER_STR_LEN);
		if(vaddr_rom_version==NULL){
			printk("Unable to Map SSB area!\n");
			return 'C';
        }
        if(memcmp(vaddr_rom_version, asic_rom_version->version_str, VER_STR_LEN)==0){
			iounmap(vaddr_rom_version);
			return asic_rom_version->tapeout_version;
		}
		iounmap(vaddr_rom_version);
    }
	printk("Default C\n");
	return 'C';
}
#endif	/* defined(CONFIG_ARCH_CORTINA_SATURN_SFU) ... */

static int ca_soc_probe(struct platform_device *pdev)
{
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU)
	void __iomem *map_io_base;
	unsigned int usb2_id;
#endif
#ifdef CONFIG_ARCH_CORTINA_G3LITE
	GLOBAL_MASTER_OVERRIDE_SECURITY_t secure;
	struct resource mem_resource;
	int ret = 0;
	const struct of_device_id *match;
	struct device_node *np = pdev->dev.of_node;
#endif
	struct resource *res;

	ca_soc = devm_kzalloc(&pdev->dev, sizeof(*ca_soc), GFP_KERNEL);
	if (!ca_soc)
		return -ENOMEM;

	ca_soc->pdev = pdev;

#if defined(CONFIG_ARCH_CORTINA_G3LITE)
	match = of_match_device(ca_soc_dt_match, &pdev->dev);
	if (!match)
	{
		printk("ca_soc fail \n");
		return -EINVAL;
	}

	ret = of_address_to_resource(np, 0, &mem_resource);
	if (ret) {
		printk("%s: of_address_to_resource BASE return %d\n", __func__, ret);
		return EBUSY;
	}

	rtl8198f_glb_reg_base = devm_ioremap(&pdev->dev, mem_resource.start, resource_size(&mem_resource));
	if (!rtl8198f_glb_reg_base) {
		printk("%s: ioremap(RTL8198F_GLB_BASE) failed!!!\n", __func__);
		return ENODEV;
	}
	rtl8198f_glb_reg_base_init(rtl8198f_glb_reg_base);

	ret = of_address_to_resource(np, 1, &mem_resource);
	if (ret) {
		printk("%s: of_address_to_resource BASE return %d\n", __func__, ret);
		return EBUSY;
	}

	rtl8198f_peri_reg_base = devm_ioremap(&pdev->dev, mem_resource.start, resource_size(&mem_resource));
	if (!rtl8198f_peri_reg_base) {
		printk("%s: ioremap(RTL8198F_PERI_BASE) failed!!!\n", __func__);
		return ENODEV;
	}
	rtl8198f_peri_reg_base_init(rtl8198f_peri_reg_base);

	ca_soc->jtag_id_base = rtl8198f_glb_reg_base + CA_SOC_JTAG_ID_BASE_OFFSET;
//	ca_soc->clk_base = rtl8198f_glb_reg_base + CA_SOC_CLK_BASE_OFFSET;
	ca_soc->strap_base = rtl8198f_glb_reg_base + CA_SOC_STRAPE_BASE_OFFSET;
//	ca_soc->fw_crtl_base = rtl8198f_glb_reg_base + CA_SOC_FW_CTRL_BASE_OFFSET;
	ca_soc->clk_base = rtl8198f_glb_reg_base + CA_SOC_FW_CTRL_BASE_OFFSET;

#else /* CONFIG_ARCH_CORTINA_G3LITE */

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

#if !defined(CONFIG_CORTINA_BOARD_FPGA) && !defined(CONFIG_CORTINA_BOARD_SATURN_SFU_TITAN) && !defined(CONFIG_REALTEK_BOARD_FPGA)
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

#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU)
	/*** Map USB2 for revision check ***/
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "USB2_ID");
	if (!res) {
		dev_err(&pdev->dev, "cannot get resource USB2_ID\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource USB2_ID\n");
		return -EBUSY;
	}

	map_io_base = ioremap(res->start, resource_size(res));
	if (map_io_base == NULL) {
		dev_err(&pdev->dev, "cannot ioremap resource USB2_ID\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}

	usb2_id = readl(map_io_base);

	release_resource(res);
	kfree(res);
#endif /* CONFIG_ARCH_CORTINA_G3 || CONFIG_ARCH_CORTINA_G3HGU */
#endif /* ! CONFIG_CORTINA_BOARD_FPGA && ! CONFIG_CORTINA_BOARD_SATURN_SFU_TITAN */

#endif /* CONFIG_ARCH_CORTINA_G3LITE */

	__ca_jtag_id_get();
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU)
	if(usb2_id == 0x01100020)	/* Synopsis vendor ID */
		ca_soc->chip_revision = 0x4;	/* Revision-D */
#endif

#if defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU) || defined(CONFIG_ARCH_CORTINA_G3HGU)
	if((ca_soc->vendor_id == VID_CORTINA) && \
		((ca_soc->chip_id==CHIP_CA8271N) || (ca_soc->chip_id==CHIP_CA8271) || \
		(ca_soc->chip_id==CHIP_CA8271S) || (ca_soc->chip_id==CHIP_CA8279))) {
		ca_soc->pon_revision = get_saturn_revision();
		if((ca_soc->chip_id==CHIP_CA8271N) || \
			(ca_soc->chip_id==CHIP_CA8271) || (ca_soc->chip_id==CHIP_CA8271S))
			ca_soc->chip_revision = ca_soc->pon_revision;
	}
#endif
#ifdef CONFIG_ARCH_CORTINA_VENUS
       if ((ca_soc->vendor_id == VID_CORTINA) && \
               ((ca_soc->chip_id == CHIP_CA8289) || (ca_soc->chip_id == CHIP_CA8277B))) {
                       ca_soc->chip_revision = get_chip_revision();
       }
#endif

	__ca_clock_get();

	ca_proc_dir = proc_mkdir("driver/cortina", NULL);

	__ca_jtag_id_dump();
	__ca_clock_dump();

#if defined(CONFIG_ARCH_CORTINA_G3LITE) && defined(CONFIG_CORTINA_BOARD_FPGA)
	/*** Security Mode ***/

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "SECURITY_OVERRIDE");
	if (!res) {
		dev_err(&pdev->dev, "cannot get resource SECURITY_OVERRIDE\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource SECURITY_OVERRIDE\n");
		return -EBUSY;
	}

	ca_soc->secure_base = ioremap(res->start, resource_size(res));
	if (!ca_soc->secure_base) {
		dev_err(&pdev->dev, "cannot ioremap resource SECURITY_OVERRIDE\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}

	release_resource(res);
	kfree(res);

	secure.wrd = readl(ca_soc->secure_base + OFF_MASTER_OVERRIDE);
	secure.bf.master_secure_override_fbm = 1;
	secure.bf.master_secure_override_ldma = 1;
	secure.bf.master_secure_override_dma = 1;
	secure.bf.master_secure_override_ne = 1;
	writel(secure.wrd, ca_soc->secure_base + OFF_MASTER_OVERRIDE);
#endif /* CONFIG_ARCH_CORTINA_G3LITE && CONFIG_CORTINA_BOARD_FPGA */

	return 0;
}

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
	{ .compatible = "cortina,g3", },
	{ .compatible = "cortina,g3lite", },
	{ .compatible = "cortina,saturn-sfu", },
	{ .compatible = "cortina,venus", },
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

MODULE_AUTHOR("Raymond Tseng <raymond.tseng@coritna-access.com>");
MODULE_DESCRIPTION("Cortina SoC driver");
MODULE_LICENSE("GPL v2");

