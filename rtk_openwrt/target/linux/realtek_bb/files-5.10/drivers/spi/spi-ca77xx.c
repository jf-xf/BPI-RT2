/*
 * SPI host controller driver for Cortina-Access CA77XX SoCs
 *
 * Copyright (C) 2016 Cortina Access, Inc.
 *		http://www.cortina-access.com
 *
 * Based on spi-bcm2835.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/spi/spi.h>
#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
#include <linux/mutex.h>
#endif
#include <soc/cortina/registers.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <linux/sched/clock.h>
#endif

/* SPI register offsets */
#define CA77XX_SPI_CLK			0x00
#define CA77XX_SPI_CFG			0x04
#define CA77XX_SPI_MODE			0x08
#define CA77XX_SPI_CTRL			0x0C
#define CA77XX_SPI_CA0			0x10
#define CA77XX_SPI_CA1			0x14
#define CA77XX_SPI_CA2			0x18
#define CA77XX_SPI_WDAT1		0x1C
#define CA77XX_SPI_WDAT0		0x20
#define CA77XX_SPI_RDAT1		0x24
#define CA77XX_SPI_RDAT0		0x28
#define CA77XX_SPI_IE0			0x2C
#define CA77XX_SPI_INT0			0x30
#define CA77XX_SPI_IE1			0x34
#define CA77XX_SPI_INT1			0x38
#define CA77XX_SPI_STAT			0x3C

/* Bitfields in CA77XX_SPI_MODE */
#define CA77XX_SPI_MODE_CPOL		0x00000001
#define CA77XX_SPI_MODE_CPHA		0x00000002
#define CA77XX_SPI_MODE_CMDS		0x00000004
#define CA77XX_SPI_MODE_ISAM		0x00000008

/* Bitfields in CA77XX_SPI_CTRL */
#define CA77XX_SPI_CTRL_DONE		0x00000001
#define CA77XX_SPI_CTRL_START		0x00000080

/* Bitfields in CA77XX_SPI_INT0 */
#define CA77XX_SPI_INT0_INT		0x00000001

#ifdef CONFIG_ARCH_REALTEK_9607F
#define EXTEND_SPI_CS
#define CA77XX_SPI_CS_NUM		8
#define EXTEND_SPI_CS_NUM		3
#else
#define CA77XX_SPI_CS_NUM		5
#endif
#define CA77XX_SPI_MAX_CDIV		131072	/* (max(counter_load)+1)*2 */

#define CA77XX_SPI_POLLING_LIMIT_US	50
#define CA77XX_SPI_POLLING_FACTOR	200

#define DRV_NAME	"spi-ca77xx"

struct ca77xx_spi {
	void __iomem *regs;
	struct clk *clk;
	struct notifier_block clk_rate_nb;
	u32 clk_rate;

	int irq;
	const u8 *tx_buf;
	u8 *rx_buf;
	int tx_len;
	int rx_len;

	u32 dev_mode;
	u32 wsize;

	struct spi_master *master;
#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
	spinlock_t lock;	/* direct access lock */
#endif
	bool clk_available;
	char cs_name[CA77XX_SPI_CS_NUM][8];
};

#define to_ca77xx_spi(x) container_of(x, struct ca77xx_spi, clk_rate_nb)

#define time64_after_eq(a, b)	\
	(typecheck(unsigned long long, a) && \
	 typecheck(unsigned long long, b) && \
	 (((long long)(a) - (long long)(b)) >= 0))
#define time64_before_eq(a, b)	time64_after_eq(b, a)

#define busy_wait_nsec(cond, timeout_nsec)			\
	({							\
	u64 end_nsec = local_clock() + timeout_nsec;		\
	bool succeeded = false;					\
	do {							\
		if (cond) {					\
			succeeded = true;			\
			break;					\
		}						\
		cpu_relax();					\
	} while (time64_before_eq(local_clock(), end_nsec));	\
	succeeded;						\
	})

#define REG_ACCESS_RD		0x00000001
#define REG_ACCESS_WR		0x00000002
#define FUNC_ENTRY		0x00000004
#define POLL_TIME		0x00000008

#define OPMODE_NORMAL		0
#define OPMODE_IRQ		1
#define OPMODE_POLL		2

static u32 dbg_flag;
#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
static u32 opmode = OPMODE_POLL;
#else
static u32 opmode = OPMODE_NORMAL;
#endif

static inline u32 ca77xx_rd_reg(struct ca77xx_spi *cs, u32 reg)
{
	if (dbg_flag & REG_ACCESS_RD) {
		u32 val = readl(cs->regs + reg);

		dev_dbg(&cs->master->dev, "R, reg=0x%02x, val=0x%08x\n", reg,
			val);

		return val;
	}

	return readl(cs->regs + reg);
}

static inline void ca77xx_wr_reg(struct ca77xx_spi *cs, u32 reg, u32 val)
{
	if (dbg_flag & REG_ACCESS_WR)
		dev_dbg(&cs->master->dev, "W, reg=0x%02x, val=0x%08x\n", reg,
			val);

	writel(val, cs->regs + reg);
}

static void dump_regs(struct ca77xx_spi *cs)
{
	u32 reg;

	reg = CA77XX_SPI_CLK;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_CLK = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_CFG;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_CFG = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_MODE;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_MODE = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_CTRL;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_CTRL = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_CA0;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_CA0 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_CA1;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_CA1 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_CA2;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_CA2 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_WDAT1;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_WDAT1 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_WDAT0;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_WDAT0 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_RDAT1;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_RDAT1 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_RDAT0;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_RDAT0 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_IE0;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_IE0 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_INT0;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_INT0 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_IE1;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_IE1 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_INT1;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_INT1 = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
	reg = CA77XX_SPI_STAT;
	dev_info(&cs->master->dev, "\t0x%02X: SPI_STAT = 0x%x\n",
		 reg, ca77xx_rd_reg(cs, reg));
}

static void ca77xx_access_data_req(struct ca77xx_spi *cs)
{
	PER_SPI_CFG_t reg_cfg;
	PER_SPI_CTRL_t reg_ctrl;
	u32 val = 0;
	int i;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	if (cs->tx_len) {
		for (i = 0; i < cs->wsize; i++) {
			if ((i % 4) == 0)
				val = 0;

			val |= ((u32)(*cs->tx_buf++)) << (24 - 8 * (i % 4));

			if ((i & 3) == 3)
				ca77xx_wr_reg(cs, CA77XX_SPI_CA0 +
					      4 * (i / 4), val);
		}
		if (i % 4)
			ca77xx_wr_reg(cs, CA77XX_SPI_CA0 + 4 * (i / 4), val);
	} else {
		ca77xx_wr_reg(cs, CA77XX_SPI_CA0, 0);
		if (cs->wsize > 4)
			ca77xx_wr_reg(cs, CA77XX_SPI_CA1, 0);
		if (cs->wsize > 8)
			ca77xx_wr_reg(cs, CA77XX_SPI_CA2, 0);
	}

	reg_cfg.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CFG);
	reg_cfg.bf.command_cyc = 1;
	reg_cfg.bf.read_write = 1;
	reg_cfg.bf.ssp_cmd_cnt = cs->wsize * 8 - 1;
	/* latch data in command phase */
	reg_cfg.bf.pre_ssp_dat_cnt = 0;
	ca77xx_wr_reg(cs, CA77XX_SPI_CFG, reg_cfg.wrd);

	/* Run and Transfer! */
	reg_ctrl.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CTRL);
	reg_ctrl.bf.sspstart = 1;
	ca77xx_wr_reg(cs, CA77XX_SPI_CTRL, reg_ctrl.wrd);
}

static void ca77xx_access_data_req_ex(struct ca77xx_spi *cs, struct spi_device *spi)
{
	PER_SPI_CFG_t reg_cfg;
	PER_SPI_CTRL_t reg_ctrl;
	u32 val = 0;
	int i;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	if (cs->tx_len) {
		for (i = 0; i < cs->wsize; i++) {
			if ((i % 4) == 0)
				val = 0;

			val |= ((u32)(*cs->tx_buf++)) << (24 - 8 * (i % 4));

			if ((i & 3) == 3)
				ca77xx_wr_reg(cs, CA77XX_SPI_CA0 +
					      4 * (i / 4), val);
		}
		if (i % 4)
			ca77xx_wr_reg(cs, CA77XX_SPI_CA0 + 4 * (i / 4), val);
	} else {
		ca77xx_wr_reg(cs, CA77XX_SPI_CA0, 0);
		if (cs->wsize > 4)
			ca77xx_wr_reg(cs, CA77XX_SPI_CA1, 0);
		if (cs->wsize > 8)
			ca77xx_wr_reg(cs, CA77XX_SPI_CA2, 0);
	}

	reg_cfg.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CFG);
	reg_cfg.bf.command_cyc = 1;
	reg_cfg.bf.read_write = 1;
	reg_cfg.bf.ssp_cmd_cnt = cs->wsize * 8 - 1;
	/* latch data in command phase */
	reg_cfg.bf.pre_ssp_dat_cnt = 0;
	ca77xx_wr_reg(cs, CA77XX_SPI_CFG, reg_cfg.wrd);

	/* Run and Transfer! */
	reg_ctrl.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CTRL);
	reg_ctrl.bf.sspstart = 1;

	if (gpio_is_valid(spi->cs_gpio))
		gpio_set_value(spi->cs_gpio, 0);

	ca77xx_wr_reg(cs, CA77XX_SPI_CTRL, reg_ctrl.wrd);
}

static int ca77xx_access_data_done(struct ca77xx_spi *cs)
{
	PER_SPI_CTRL_t reg_ctrl;
	int done = 0;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	reg_ctrl.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CTRL);
	if (reg_ctrl.bf.sspdone) {
		done = 1;
		ca77xx_wr_reg(cs, CA77XX_SPI_CTRL, reg_ctrl.wrd);
	}

	return done;
}

static int ca77xx_access_data_done_ex(struct ca77xx_spi *cs, struct spi_device *spi)
{
	PER_SPI_CTRL_t reg_ctrl;
	int done = 0;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	reg_ctrl.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CTRL);
	if (reg_ctrl.bf.sspdone) {
		done = 1;
		ca77xx_wr_reg(cs, CA77XX_SPI_CTRL, reg_ctrl.wrd);
		if (gpio_is_valid(spi->cs_gpio))
			gpio_set_value(spi->cs_gpio, 1);
	}

	return done;
}

static void ca77xx_access_data(struct ca77xx_spi *cs)
{
	u32 wsize, val, loop, offset;
	u8 byte;
	int i;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	if (cs->tx_len)
		cs->tx_len -= cs->wsize;

	if (cs->rx_len) {
		wsize = cs->wsize;
REMAINDER:
		val = ca77xx_rd_reg(cs, wsize > 4 ?
			CA77XX_SPI_RDAT1 : CA77XX_SPI_RDAT0);

		loop = wsize > 4 ? wsize - 4 : wsize;
		for (i = 0; i < loop; i++) {
			offset = ((loop - 1) % 4) - (i % 4);
			offset *= 8;

			byte = (val >> offset) & 0xFF;
			*cs->rx_buf++ = byte;
		}

		if (wsize > 4) {
			wsize = 4;
			goto REMAINDER;
		}

		cs->rx_len -= cs->wsize;
	}
}

static irqreturn_t ca77xx_spi_interrupt(int irq, void *dev_id)
{
	struct spi_master *master = dev_id;
	struct ca77xx_spi *cs = spi_master_get_devdata(master);
	u32 reg;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	ca77xx_wr_reg(cs, CA77XX_SPI_INT0, CA77XX_SPI_INT0_INT);

	if (ca77xx_access_data_done(cs))
		ca77xx_access_data(cs);

	if (cs->tx_len || cs->rx_len) {
		ca77xx_access_data_req(cs);
	} else {
		reg = ca77xx_rd_reg(cs, CA77XX_SPI_IE0);
		reg &= (~CA77XX_SPI_INT0_INT);
		ca77xx_wr_reg(cs, CA77XX_SPI_IE0, reg);

		/* wake up the framework */
		complete(&master->xfer_completion);
	}

	return IRQ_HANDLED;
}

static int ca77xx_spi_transfer_one_irq(struct spi_master *master,
				       struct spi_device *spi,
				       struct spi_transfer *tfr)
{
	struct ca77xx_spi *cs = spi_master_get_devdata(master);

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	ca77xx_wr_reg(cs, CA77XX_SPI_IE0, CA77XX_SPI_INT0_INT);

	ca77xx_access_data_req(cs);

	/* signal that we need to wait for completion */
	return 1;
}

static int ca77xx_spi_transfer_one_poll(struct spi_master *master,
					struct spi_device *spi,
					struct spi_transfer *tfr,
					unsigned long long xfer_time_us)
{
	struct ca77xx_spi *cs = spi_master_get_devdata(master);
	u64 dbg_start_nsec = 0, dbg_end_nsec = 0;
	static s64 max_optime = 0, optime;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	while (cs->tx_len || cs->rx_len) {
		ca77xx_access_data_req_ex(cs, spi);

		if (dbg_flag & POLL_TIME)
			dbg_start_nsec = local_clock();

		if (!busy_wait_nsec(ca77xx_access_data_done_ex(cs, spi),
				    xfer_time_us * 1000)) {
			dev_err(&spi->dev, "transfer %d bytes timeout(%lld us)\n",
				tfr->bits_per_word / 8, xfer_time_us);
			dump_regs(cs);
			if (gpio_is_valid(spi->cs_gpio))
				gpio_set_value(spi->cs_gpio, 1);
			return -EIO;
		}

		if (dbg_flag & POLL_TIME) {
			dbg_end_nsec = local_clock();

			optime = (s64)dbg_end_nsec - (s64)dbg_start_nsec;
			if (optime > max_optime) {
				max_optime = optime;
				dev_dbg(&spi->dev, "max optime:%lld\n",
					max_optime);
			}
		}

		ca77xx_access_data(cs);

		if (tfr->delay_usecs)
			udelay(tfr->delay_usecs);
	}

	return 0;
}

#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
int ca77xx_spi_transfer_one(struct spi_master *master,
			    struct spi_device *spi,
			    struct spi_transfer *tfr)
#else
static int ca77xx_spi_transfer_one(struct spi_master *master,
				   struct spi_device *spi,
				   struct spi_transfer *tfr)
#endif
{
	struct ca77xx_spi *cs = spi_master_get_devdata(master);
	unsigned long spi_hz, clk_hz, cdiv;
	unsigned long spi_used_hz;
	unsigned long long xfer_time_us;
	PER_SPI_CLK_t reg_clk;
#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
	PER_SPI_CFG_t reg_cfg;
	unsigned long flags;
	u32 mode = 0;
#ifdef EXTEND_SPI_CS
	unsigned char ex_cs_num = CA77XX_SPI_CS_NUM - EXTEND_SPI_CS_NUM;
	unsigned char ex_cs_bit = 0;
#endif
#endif
	int rc;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	if (!cs->clk_available) {
		dev_warn(&cs->master->dev, "clock is not available!\n");
		return -EIO;
	}

#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
	spin_lock_irqsave(&cs->lock, flags);
#ifdef EXTEND_SPI_CS
	reg_cfg.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CFG);
	if(spi->chip_select >= ex_cs_num){
		reg_cfg.bf.micro_wire_cs_sel = 0;
		reg_cfg.bf.sel_ssp_cs = 0;
	}
	else{
		if (spi->mode & SPI_CS_HIGH)
			reg_cfg.bf.micro_wire_cs_sel = BIT(spi->chip_select);
		else
			reg_cfg.bf.micro_wire_cs_sel = 0;
		reg_cfg.bf.sel_ssp_cs = BIT(spi->chip_select);
	}
	ca77xx_wr_reg(cs, CA77XX_SPI_CFG, reg_cfg.wrd);
#else
	/* prepare */
	reg_cfg.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CFG);
	if (spi->mode & SPI_CS_HIGH)
		reg_cfg.bf.micro_wire_cs_sel = 1;
	else
		reg_cfg.bf.micro_wire_cs_sel = 0;
	reg_cfg.bf.sel_ssp_cs = BIT(spi->chip_select);
	ca77xx_wr_reg(cs, CA77XX_SPI_CFG, reg_cfg.wrd);
#endif
	cs->dev_mode = spi->mode;

	if (spi->mode & SPI_CPOL)
		mode |= CA77XX_SPI_MODE_CPOL;
	if (spi->mode & SPI_CPHA)
		mode |= CA77XX_SPI_MODE_CPHA;
	mode |= CA77XX_SPI_MODE_CMDS;
	if (spi->mode & SPI_XSI)
		mode |= CA77XX_SPI_MODE_ISAM;
	ca77xx_wr_reg(cs, CA77XX_SPI_MODE, mode);
#endif

	/* set clock */
	spi_hz = tfr->speed_hz ? tfr->speed_hz : spi->max_speed_hz;
	clk_hz = cs->clk_rate;

	if (spi_hz >= clk_hz / 2) {
		cdiv = 2; /* clk_hz/2 is the fastest we can go */
	} else if (spi_hz) {
		/* CDIV must be a multiple of two */
		cdiv = DIV_ROUND_UP(clk_hz, spi_hz);
		cdiv += (cdiv % 2);

		if (cdiv >= CA77XX_SPI_MAX_CDIV)
			cdiv = CA77XX_SPI_MAX_CDIV;
	} else {
		cdiv = CA77XX_SPI_MAX_CDIV;
	}
	spi_used_hz = clk_hz / cdiv;

	reg_clk.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CLK);
	reg_clk.bf.ssp_igap = 0;
	reg_clk.bf.counter_load = cdiv / 2 - 1;
#ifdef EXTEND_SPI_CS
	if(spi->chip_select >= ex_cs_num){
		ex_cs_bit = (spi->chip_select) - ex_cs_num;

		if (spi->mode & SPI_CS_HIGH)
			reg_clk.bf.micro_wire_cs_sel_extend = BIT(ex_cs_bit);
		else
			reg_clk.bf.micro_wire_cs_sel_extend = 0;

		reg_clk.bf.sel_ssp_cs_extend = BIT(ex_cs_bit);
	}else{
		reg_clk.bf.micro_wire_cs_sel_extend = 0;
		reg_clk.bf.sel_ssp_cs_extend = 0;
	}
#endif
	ca77xx_wr_reg(cs, CA77XX_SPI_CLK, reg_clk.wrd);

	if (tfr->bits_per_word)
		cs->wsize = tfr->bits_per_word / 8;
	else
		cs->wsize = spi->bits_per_word / 8;

	/* set transmit buffers and length */
	cs->tx_buf = tfr->tx_buf;
	cs->rx_buf = tfr->rx_buf;
	cs->tx_len = tfr->tx_buf ? tfr->len : 0;
	cs->rx_len = tfr->rx_buf ? tfr->len : 0;

	if (opmode == OPMODE_IRQ)
		goto FORCE_IRQ;

	/* calculate the estimated time in us the transfer runs */
	xfer_time_us = (unsigned long long)cs->wsize
		* 12 /* clocks/byte - SPI-HW waits 4 clock after each byte */
		* 1000000;
	do_div(xfer_time_us, spi_used_hz);
	xfer_time_us += 1;

	/* for short requests run polling */
	if (opmode == OPMODE_POLL ||
	    xfer_time_us <= CA77XX_SPI_POLLING_LIMIT_US) {
		rc = ca77xx_spi_transfer_one_poll(master, spi, tfr,
						  xfer_time_us *
						  CA77XX_SPI_POLLING_FACTOR);
#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
		spin_unlock_irqrestore(&cs->lock, flags);
#endif
		return rc;
	}
FORCE_IRQ:
	/* run in interrupt-mode */
	rc = ca77xx_spi_transfer_one_irq(master, spi, tfr);
#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
	spin_unlock_irqrestore(&cs->lock, flags);
#endif
	return rc;
}

static int ca77xx_spi_prepare_message(struct spi_master *master,
				      struct spi_message *msg)
{
#ifndef CONFIG_BYPASS_SPI_FRAMEWORK
	struct spi_device *spi = msg->spi;
	struct ca77xx_spi *cs = spi_master_get_devdata(master);
	PER_SPI_CFG_t reg_cfg;
	u32 mode = 0;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&cs->master->dev, "%s()\n", __func__);

	reg_cfg.wrd = ca77xx_rd_reg(cs, CA77XX_SPI_CFG);
	if (spi->mode & SPI_CS_HIGH)
		reg_cfg.bf.micro_wire_cs_sel = 1;
	else
		reg_cfg.bf.micro_wire_cs_sel = 0;
	reg_cfg.bf.sel_ssp_cs = BIT(spi->chip_select);
	ca77xx_wr_reg(cs, CA77XX_SPI_CFG, reg_cfg.wrd);

	cs->dev_mode = spi->mode;

	if (spi->mode & SPI_CPOL)
		mode |= CA77XX_SPI_MODE_CPOL;
	if (spi->mode & SPI_CPHA)
		mode |= CA77XX_SPI_MODE_CPHA;
	mode |= CA77XX_SPI_MODE_CMDS;
	if (spi->mode & SPI_XSI)
		mode |= CA77XX_SPI_MODE_ISAM;
	ca77xx_wr_reg(cs, CA77XX_SPI_MODE, mode);
#endif
	return 0;
}

static int ca77xx_spi_setup(struct spi_device *spi)
{
	struct spi_master *master = spi->master;

	if (dbg_flag & FUNC_ENTRY)
		dev_dbg(&master->dev, "%s()\n", __func__);

	/* only check max_speed_hz and other properties will be checked in
	 * framework
	 */
	if (spi->max_speed_hz > master->max_speed_hz) {
		dev_warn(&master->dev,
			 "setup: max speed is only supported to %d(Hz). downgrade to the speed\n",
			 master->max_speed_hz);
		spi->max_speed_hz = master->max_speed_hz;
	}

	if (spi->max_speed_hz < master->min_speed_hz) {
		dev_err(&spi->dev,
			"setup: min speed is only supported to %d(Hz).\n",
			master->min_speed_hz);
		return -EPERM;
	}

	if (gpio_is_valid(spi->cs_gpio)) {
		if (!spi->controller_state) {
			struct ca77xx_spi *cs = spi_master_get_devdata(master);
			int i = spi->chip_select;
			int ret;

			sprintf(cs->cs_name[i], "spi-cs%d", i);
			ret = devm_gpio_request(&master->dev, spi->cs_gpio,
						cs->cs_name[i]);
			if (ret) {
				dev_err(&master->dev,
					"can't get SPI CS%d GPIO\n", i);
				return ret;
			}

			spi->controller_state = spi;
		}

		gpio_direction_output(spi->cs_gpio, !(spi->mode & SPI_CS_HIGH));
	}

	return 0;
}

static int ca77xx_spi_clk_notifier_cb(struct notifier_block *nb, unsigned long
				      event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct ca77xx_spi *cs = to_ca77xx_spi(nb);

	switch (event) {
	case PRE_RATE_CHANGE:
		cs->clk_available = false;
		return NOTIFY_OK;
	case POST_RATE_CHANGE:
		cs->clk_rate = ndata->new_rate;
		cs->clk_available = true;
		return NOTIFY_OK;
	case ABORT_RATE_CHANGE:
		cs->clk_available = true;
		return NOTIFY_OK;
	default:
		return NOTIFY_DONE;
	}
}

static int ca77xx_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct ca77xx_spi *cs;
	struct resource *res;
	int err;

	master = spi_alloc_master(&pdev->dev, sizeof(*cs));
	if (!master) {
		dev_err(&pdev->dev, "spi_alloc_master() failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, master);

	master->dev.of_node = pdev->dev.of_node;
	master->bus_num = pdev->id;
	master->num_chipselect = CA77XX_SPI_CS_NUM;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_XSI; /* SPI_LSB_FIRST ? */
	master->bits_per_word_mask = SPI_BPW_MASK(8) | SPI_BPW_MASK(16) |
				     SPI_BPW_MASK(32);
	/* master->flags = SPI_MASTER_HALF_DUPLEX ? */

	master->setup = ca77xx_spi_setup;
	/* transfer_one() - framework will handle statistics and trace */
	master->transfer_one = ca77xx_spi_transfer_one;
	master->prepare_message = ca77xx_spi_prepare_message;

	cs = spi_master_get_devdata(master);
	cs->master = master;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	cs->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(cs->regs)) {
		err = PTR_ERR(cs->regs);
		goto out_master_put;
	}
	dev_info(&pdev->dev, "resource - %pr mapped at 0x%pK\n", res,
		 cs->regs);

	cs->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(cs->clk)) {
		err = PTR_ERR(cs->clk);
		dev_err(&pdev->dev, "could not get clk: %d\n", err);
		goto out_master_put;
	}
	master->max_speed_hz = clk_get_rate(cs->clk) / 2;
	master->min_speed_hz = clk_get_rate(cs->clk) / CA77XX_SPI_MAX_CDIV;
	cs->clk_rate = clk_get_rate(cs->clk);
	cs->clk_rate_nb.notifier_call = ca77xx_spi_clk_notifier_cb;
	err = clk_notifier_register(cs->clk, &cs->clk_rate_nb);
	if (err != 0) {
		dev_err(&pdev->dev, "Unable to register clock notifier\n");
		goto out_master_put;
	}

	cs->irq = platform_get_irq(pdev, 0);
	if (cs->irq <= 0) {
		dev_err(&pdev->dev, "could not get IRQ: %d\n", cs->irq);
		err = cs->irq ? cs->irq : -ENODEV;
		goto out_master_put;
	}
	dev_info(&pdev->dev, "irq %d\n", cs->irq);

	clk_prepare_enable(cs->clk);
	cs->clk_available = true;

	err = devm_request_irq(&pdev->dev, cs->irq, ca77xx_spi_interrupt, 0,
			       dev_name(&pdev->dev), master);
	if (err) {
		dev_err(&pdev->dev, "could not request IRQ: %d\n", err);
		goto out_clk_disable;
	}

#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
	spin_lock_init(&cs->lock);
#endif
	err = devm_spi_register_master(&pdev->dev, master);
	if (err) {
		dev_err(&pdev->dev, "could not register SPI master: %d\n", err);
		goto out_clk_disable;
	}

	return 0;

out_clk_disable:
	clk_disable_unprepare(cs->clk);
out_master_put:
	spi_master_put(master);
	return err;
}

static int ca77xx_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct ca77xx_spi *cs = spi_master_get_devdata(master);

	clk_notifier_unregister(cs->clk, &cs->clk_rate_nb);
	clk_disable_unprepare(cs->clk);

	return 0;
}

static const struct of_device_id ca77xx_spi_match[] = {
	{ .compatible = "cortina,ca77xx-spi", },
	{ },
};
MODULE_DEVICE_TABLE(of, ca77xx_spi_match);

static struct platform_driver ca77xx_spi_driver = {
	.probe = ca77xx_spi_probe,
	.remove = ca77xx_spi_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(ca77xx_spi_match),
	},
};
module_platform_driver(ca77xx_spi_driver);

module_param_named(debug, dbg_flag, uint, 0644);
module_param_named(opmode, opmode, uint, 0644);
MODULE_PARM_DESC(opmode, "operation mode(0:normal, 1:irq only, 2:poll only)\n");

#ifdef CONFIG_BYPASS_SPI_FRAMEWORK
EXPORT_SYMBOL(ca77xx_spi_transfer_one);
#endif

MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DESCRIPTION("Cortina Access CA77xx SPI controller driver");
MODULE_LICENSE("GPL v2");