#ifndef __CORTINA_SOC_H__
#define __CORTINA_SOC_H__

#include <linux/module.h>
#include <linux/platform_device.h>
#ifdef CONFIG_ARCH_CORTINA_SMC
#include "cortina-smc.h"
#endif
/* Vendor ID */
#define VID_CORTINA			0x8F3

/* Chip ID */
#define CHIP_CA76XX			0x0800	/* G3Lite */
#define CHIP_CA7742			0x010C	/* G3 */
#define CHIP_CA8271N			0xA19C	/* Saturn2 SFU */
#define CHIP_CA8271			0xA17C	/* Saturn SFU */
#define CHIP_CA8271S		0xA17D	/* Saturn SFP+ */
#define CHIP_CA8279			0x010D	/* Saturn HGU */
#define CHIP_CA8289			0xA18C	/* Venus */
#define CHIP_CA8277B		0xA18D  /* Venus B*/
#define CHIP_CA8277C			0x6706  /* TAURUS */
#define CHIP_RTL9607F			0x6900  /* ELNATH */

#define CA_SOC_VENDOR_ID(jtag_id)	((jtag_id) & 0xFFF)
#define CA_SOC_CHIP_ID(jtag_id)		(((jtag_id) >> 12) & 0xFFFF)
#define CA_SOC_CHIP_REV(jtag_id)	(((jtag_id) >> 28) & 0xF)

/* Realtek TAURUS Serial */
#define RTK_SOC_VENDOR_ID_GET(jtag_id)       ((jtag_id) & 0xFFF)
#define RTK_SOC_CHIP_ID_GET(jtag_id)         (((jtag_id) >> 16) & 0xFFFF)
#define RTK_SOC_CHIP_REV_GET(jtag_id)        (((jtag_id) >> 14) & 0x3)

/* offset of clock registers */
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS) || defined(CONFIG_ARCH_REALTEK_9607F)
#define OFF_CPLLDIV		0x00
#define OFF_CPLLMUX		0x04
#define OFF_EPLLDIV		0x08
#define OFF_EPLLDIV2		0x0c
#define OFF_PEDIV		0x10
#define OFF_PEMUX		0x14
	#if  defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS) || defined(CONFIG_ARCH_REALTEK_9607F)
		#define OFF_CPLL0		0x30
		#define OFF_EPLL0		0x38
		#define OFF_FPLL0		0x40
	#else
		#define OFF_CPLL0		0x2c
		#define OFF_EPLL0		0x34
		#define OFF_FPLL0		0x3c
	#endif
#elif defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN_SFP_PLUS)
#define OFF_CLKGEN_CONFIG	0x00
#define OFF_CPLL0		0x08
#define OFF_CPLLDIV		0x10
#define OFF_CPLLDIV2		0x14
#define OFF_CPLL_STATE		0x18
#define OFF_AP_FREQ_CONF	0x28
#define OFF_AP_FREQ_STATUS	0x30
#define OFF_DSP_FREQ_CONF	0x34
#define OFF_DSP_FREQ_STATUS	0x3c
#endif

/* offset of secure mode registers */
#define OFF_OVERRIDE		0x00
#define OFF_MASTER_OVERRIDE	0x04

struct ca_soc_data {
	struct platform_device *pdev;

	void __iomem *jtag_id_base;
	void __iomem *clk_base;
	void __iomem *strap_base;
	void __iomem *secure_base;

	uint32_t vendor_id;
	uint32_t chip_id;
	uint32_t chip_revision;
#if defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU)
	uint32_t pon_revision;
#endif
	uint32_t cpll;
	uint32_t epll;
	uint32_t fpll;
	uint32_t cpu_clk;
	uint32_t cci_clk;
	uint32_t lsaxi_clk;
	uint32_t hsaxi_clk;
	uint32_t core_clk;
	uint32_t crypto_clk;
	uint32_t eaxi_clk;
	uint32_t atb_clk;
	uint32_t pe_clk;
	uint32_t peaxi_clk;

	uint32_t io_clk;
	uint32_t eth_ref2_clk;
};
#if (defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS) || defined(CONFIG_ARCH_REALTEK_9607F)) && !defined(CONFIG_CORTINA_BOARD_FPGA)
#define IS_TAURUS	(RTK_SOC_CHIP_ID == CHIP_CA8277C)
#define IS_ELNATH	(RTK_SOC_CHIP_ID == CHIP_RTL9607F)
#else
#define IS_TAURUS	0
#define IS_ELNATH	(0)
#endif

#define RTK_SOC_TAURUS_GEN	(IS_TAURUS || IS_ELNATH)

extern uint32_t RTK_SOC_CHIP_ID;
extern struct proc_dir_entry *ca_proc_dir;

int ca_soc_data_get(struct ca_soc_data *data);

#endif /* __CORTINA_SOC_H__ */
