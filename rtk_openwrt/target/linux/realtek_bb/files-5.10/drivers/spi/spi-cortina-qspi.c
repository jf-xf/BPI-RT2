// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Cortina QSPI Controller
 *
 * Copyright (C) 2020 Cortina Access Inc. All Rights Reserved.
 *
 * Author: PengPeng Chen <pengpeng.chen@cortina-access.com>
 * Signed-off-by: Jason Li <jason.li@cortina-access.com>
 *
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include <linux/sizes.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>
#include <linux/types.h>
#include <soc/cortina/cortina-soc.h>


/* CA QSPI register offsets */
#define QSPI_ID_REG			0x0000
#define QSPI_TC_REG			0x0004
#define QSPI_STATUS_REG		0x0008
#define QSPI_TYPE_REG		0x000c
#define QSPI_BUSY_REG		0x0010
#define QSPI_INT_STATUS		0x0014
#define QSPI_INT_MASK		0x0018
#define QSPI_FIFO_CTL		0x001c
#define QSPI_FIFO_STATUS	0x0020
#define QSPI_FIFO_ADDR		0x0024
#define QSPI_FIFP_MATCH		0x0028
#define QSPI_FIFO_DATA		0x002C
#define QSPI_ACCESS_REG		0x0030
#define QSPI_EXT_ACCESS		0x0034
#define QSPI_ADDR_REG		0x0038
#define QSPI_DATA_REG		0x003c
#define QSPI_TIMING			0x0040

#define QSPI_SIZE_MASK               GENMASK(10, 9)
#define QSPI_SIZE(sz)                (((sz) << 9) & QSPI_SIZE_MASK)
#define QSPI_WIDTH                   BIT(11)
#define QSPI_TYPE_MASK               GENMASK(14, 12)
#define QSPI_TYPE(tp)                (((tp) << 12) & QSPI_TYPE_MASK)
#define QSPI_PIN                     BIT(15)

#define QSPI_START_EN                BIT(1)
#define QSPI_FIFO_START_EN           BIT(3)
#define QSPI_WR_ACCESS_EN            BIT(9)

#define QSPI_REG_IRQ                 BIT(0)
#define QSPI_FIFO_IRQ                BIT(1)

#define QSPI_OPCODE_MASK             GENMASK(7, 0)
#define QSPI_OPCODE(op)              (((op) << 0) & QSPI_OPCODE_MASK)
#define QSPI_ACCODE_MASK             GENMASK(11, 8)
#define QSPI_ACCODE(ac)              (((ac) << 8) & QSPI_ACCODE_MASK)
#define QSPI_FORCE_TERM              BIT(12)
#define QSPI_FORCE_BURST             BIT(13)
#define QSPI_AUTO_MODE_EN            BIT(15)
#define QSPI_CHIP_EN_ALT             BIT(16)
#define QSPI_HI_SPEED_RD             BIT(17)
#define QSPI_MIO_INF_DC              BIT(24)
#define QSPI_MIO_INF_AC              BIT(25)
#define QSPI_MIO_INF_CC              BIT(26)
#define QSPI_DDR_MASK                GENMASK(29, 28)
#define QSPI_DDR(ddr)                (((ddr) << 28) & QSPI_DDR_MASK)
#define QSPI_MIO_INF_MASK            GENMASK(31, 30)
#define QSPI_MIO_INF(io)             (((io) << 30) & QSPI_MIO_INF_MASK)

#define QSPI_EXT_OPCODE_MASK         GENMASK(7, 0)
#define QSPI_EXT_OPCODE(op)          (((op) << 0) & QSPI_EXT_OPCODE_MASK)
#define QSPI_EXT_DATA_CNT_MASK       GENMASK(20, 8)
#define QSPI_EXT_DATA_CNT(cnt)       (((cnt) << 8) & QSPI_EXT_DATA_CNT_MASK)
#define QSPI_EXT_DATA_CNT_MAX        (2048)
#define QSPI_EXT_ADDR_CNT_MASK       GENMASK(23, 21)
#define QSPI_EXT_ADDR_CNT(cnt)       (((cnt) << 21) & QSPI_EXT_ADDR_CNT_MASK)
#define QSPI_EXT_ADDR_CNT_MAX        (7)
#define QSPI_EXT_DUMY_CNT_MASK       GENMASK(29, 24)
#define QSPI_EXT_DUMY_CNT(cnt)       (((cnt) << 24) & QSPI_EXT_DUMY_CNT_MASK)
#define QSPI_EXT_DUMY_CNT_MAX        (63)
#define QSPI_EXT_DRD_CMD_EN          BIT(31)

#define QSPI_ADDR_MASK               GENMASK(31, 0)
#define QSPI_ADDR(addr)              (((addr) << 0) & QSPI_ADDR_MASK)

#define QSPI_DATA_MASK               GENMASK(31, 0)
#define QSPI_DATA(addr)              (((addr) << 0) & QSPI_DATA_MASK)

#define QSPI_TMR_IDLE_MASK           GENMASK(7, 0)
#define QSPI_TMR_IDLE(idle)          (((idle) << 0) & QSPI_TMR_IDLE_MASK)
#define QSPI_TMR_HOLD_MASK           GENMASK(15, 8)
#define QSPI_TMR_HOLD(hold)          (((hold) << 8) & QSPI_TMR_HOLD_MASK)
#define QSPI_TMR_SETUP_MASK          GENMASK(23, 16)
#define QSPI_TMR_SETUP(setup)        (((setup) << 16) & QSPI_TMR_SETUP_MASK)
#define QSPI_TMR_CLK_MASK            GENMASK(26, 24)
#define QSPI_TMR_CLK(clk)            (((clk) << 24) & QSPI_TMR_CLK_MASK)

#define QSPI_AC_OP                   0x00
#define QSPI_AC_OP_1_DATA            0x01
#define QSPI_AC_OP_2_DATA            0x02
#define QSPI_AC_OP_3_DATA            0x03
#define QSPI_AC_OP_4_DATA            0x04
#define QSPI_AC_OP_3_ADDR            0x05
#define QSPI_AC_OP_4_ADDR            (QSPI_AC_OP_3_ADDR)
#define QSPI_AC_OP_3_ADDR_1_DATA     0x06
#define QSPI_AC_OP_3_ADDR_2_DATA     0x07
#define QSPI_AC_OP_3_ADDR_3_DATA     0x08
#define QSPI_AC_OP_3_ADDR_4_DATA     0x09
#define QSPI_AC_OP_3_ADDR_X_1_DATA   0x0A
#define QSPI_AC_OP_3_ADDR_X_2_DATA   0x0B
#define QSPI_AC_OP_3_ADDR_X_3_DATA   0x0C
#define QSPI_AC_OP_3_ADDR_X_4_DATA   0x0D
#define QSPI_AC_OP_3_ADDR_4X_1_DATA  0x0E
#define QSPI_AC_OP_EXTEND            0x0F

#define QSPI_ACCESS_MIO_SINGLE       0
#define QSPI_ACCESS_MIO_DUAL         1
#define QSPI_ACCESS_MIO_QUAD         2
/* undelay 1 -> ndelay 20
#define QSPI_BUSY_TIMEOUT_US         30000
 */
#define QSPI_BUSY_TIMEOUT_US         30000*500


struct ca_qspi {
	struct device *dev;
	struct spi_master *master;
	void __iomem *base;
	u8 *internal_buf;
	u32 ready_mask;
	bool fifo_mode;
	u8 tx_bus_width;
	u8 rx_bus_width;
};

static inline u32 ca_reg_read(struct ca_qspi *qspi, u8 offset)
{
    return readl(qspi->base + offset);
}

static inline void ca_reg_write(struct ca_qspi *qspi,
	u32 val, u8 offset)
{
    writel(val, qspi->base + offset);
}
#if 0
static int ca_qspi_wait_cmd(struct ca_qspi *qspi)
{
	u32 stat = 0;
	u32 val = 0;
	int timeout = QSPI_BUSY_TIMEOUT_US;
    u32 ready_mask = (RTK_SOC_TAURUS_GEN)?(BIT(16)|QSPI_START_EN):QSPI_START_EN;

	val = ca_reg_read(qspi, QSPI_BUSY_REG);
	val |= QSPI_START_EN;
	ca_reg_write(qspi, val, QSPI_BUSY_REG);

	do {
		stat = ca_reg_read(qspi, QSPI_BUSY_REG);
		if ((stat & ready_mask) == 0 ){
			return 0;
		}
	} while (timeout--);

	dev_err(qspi->dev, "!error busy timeout (stat:%#x)\n", stat);
	return -1;
}
#endif

static inline int ca_qspi_wait_wr_cmd(struct ca_qspi *qspi)
{
	u32 stat = 0;
	int timeout = QSPI_BUSY_TIMEOUT_US;

	ca_reg_write(qspi, (QSPI_WR_ACCESS_EN | QSPI_START_EN), QSPI_BUSY_REG);

	do {
		stat = ca_reg_read(qspi, QSPI_BUSY_REG);
		if ((stat & qspi->ready_mask) == 0 ){
			return 0;
		}
	} while (timeout--);

	dev_err(qspi->dev, "!error busy timeout (stat:%#x)\n", stat);
	return -1;
}

static inline int ca_qspi_wait_rd_cmd(struct ca_qspi *qspi)
{
	u32 stat = 0;

	int timeout = QSPI_BUSY_TIMEOUT_US;

	ca_reg_write(qspi, QSPI_START_EN, QSPI_BUSY_REG);

	do {
		stat = ca_reg_read(qspi, QSPI_BUSY_REG);
		if ((stat & qspi->ready_mask) == 0 ){
			return 0;
		}
	} while (timeout--);

	dev_err(qspi->dev, "!error busy timeout (stat:%#x)\n", stat);
	return -1;
}

static inline int ca_qspi_wait_cmd_select(struct ca_qspi *qspi, struct spi_mem_op *op){
	if (op->data.dir == SPI_MEM_DATA_IN) {
		return ca_qspi_wait_rd_cmd(qspi);
    } else if ((op->data.dir == SPI_MEM_NO_DATA) && (RTK_SOC_TAURUS_GEN)){
        return ca_qspi_wait_rd_cmd(qspi);
	} else {
		return ca_qspi_wait_wr_cmd(qspi);
	}
}

static int ca_qspi_rx(struct ca_qspi *qspi,
	u8 *buf, unsigned int data_len)
{
	u32 data = 0;
	int len = data_len;

	while (len >= 4) {
		if (ca_qspi_wait_rd_cmd(qspi))
			return -1;
		data = ca_reg_read(qspi, QSPI_DATA_REG);
		*buf++ = data & 0xFF;
		*buf++ = (data >> 8) & 0xFF;
		*buf++ = (data >> 16) & 0xFF;
		*buf++ = (data >> 24) & 0xFF;
		len -= 4;
	}
	if (len > 0) {
		if (ca_qspi_wait_rd_cmd(qspi))
			return -1;
		data = ca_reg_read(qspi, QSPI_DATA_REG);
		*buf++ = data & 0xFF;
		switch (len) {
		case 3:
			*buf++ = (data >> 8) & 0xFF;
			*buf++ = (data >> 16) & 0xFF;
			break;
		case 2:
			*buf++ = (data >> 8) & 0xFF;
			break;
		case 1:
			break;
		default:
			dev_err(qspi->dev, "!err datalen=%d.\n", len);
			return -1;
		}
	}
	return 0;
}

static int ca_qspi_tx(struct ca_qspi *qspi,
	u8 *buf, unsigned int data_len)
{
	u32 data = 0;
	int len = data_len;

	while (len >= 4) {
		data = *buf | (*(buf + 1) << 8)
			| (*(buf + 2) << 16) | (*(buf + 3) << 24);
		ca_reg_write(qspi, data, QSPI_DATA_REG);
		if (ca_qspi_wait_wr_cmd(qspi))
			return -1;
		len -= 4;
		buf += 4;
	}
	if (len > 0) {
		data = *buf;
		switch (len) {
		case 3:
			data |= (*(buf + 1) << 8) | ((*(buf + 2)) << 16);
			break;
		case 2:
			data |= (*(buf + 1) << 8);
			break;
		case 1:
			break;
		default:
			dev_err(qspi->dev, "!err datalen=%d\n", len);
			return -1;
		}
		ca_reg_write(qspi, data, QSPI_DATA_REG);
		if (ca_qspi_wait_wr_cmd(qspi))
			return -1;
	}
	return 0;
}

static int ca_qspi_xfer_data(struct ca_qspi *qspi,
	struct spi_mem_op *op)
{

	u8 *data_buf = NULL;

	unsigned int len = 0;
	u8 addr_cnt = op->addr.nbytes;
	int data_cnt = op->data.nbytes;
	u32 addr_offset = (u32)op->addr.val;
#if 0
	unsigned int buf_len = data_cnt > QSPI_EXT_DATA_CNT_MAX ?
			QSPI_EXT_DATA_CNT_MAX : data_cnt;

	val = ca_reg_read(qspi, QSPI_BUSY_REG);
	if (op->data.dir == SPI_MEM_DATA_IN) {
		val &= ~QSPI_WR_ACCESS_EN;
		data_buf = (u8 *)op->data.buf.in;
	} else {
		val |= QSPI_WR_ACCESS_EN;
		data_buf = (u8 *)op->data.buf.out;
	}
	ca_reg_write(qspi, val, QSPI_BUSY_REG);
#else
	if (op->data.dir == SPI_MEM_DATA_IN) {
		data_buf = (u8 *)op->data.buf.in;
	} else {
		data_buf = (u8 *)op->data.buf.out;
	}
#endif

	while (data_cnt > 0) {
		len = data_cnt > QSPI_EXT_DATA_CNT_MAX ?
			QSPI_EXT_DATA_CNT_MAX : data_cnt;
		if (addr_cnt > 0)
			ca_reg_write(qspi, (u32)addr_offset, QSPI_ADDR_REG);
		memset(qspi->internal_buf, 0, len);
		if (op->data.dir == SPI_MEM_DATA_IN) {
			if (ca_qspi_rx(qspi, qspi->internal_buf, len))
				break;
			memcpy(data_buf, qspi->internal_buf, len);
		} else {
			memcpy(qspi->internal_buf, data_buf, len);
			if (ca_qspi_tx(qspi, qspi->internal_buf, len))
				break;
		}
		data_cnt -= len;
		data_buf += len;
		addr_offset += data_cnt > 0 ? len : 0;
	}

	return data_cnt > 0 ? -1 : 0;
}

static int ca_qspi_buswidth_set(struct ca_qspi *qspi,
	struct spi_mem_op *op)
{
	u32 val = 0;
//	u8 buswidth = op->data.buswidth;

	val = ca_reg_read(qspi, QSPI_ACCESS_REG);
	val &= ~(QSPI_MIO_INF_MASK
			| QSPI_MIO_INF_DC
			| QSPI_MIO_INF_AC
			| QSPI_MIO_INF_CC);
	if (op->data.buswidth == 4) {
		val |= QSPI_MIO_INF_DC;
		val |= QSPI_MIO_INF(QSPI_ACCESS_MIO_QUAD);
	} else if (op->data.buswidth == 2) {
		val |= QSPI_MIO_INF_DC;
		val |= QSPI_MIO_INF(QSPI_ACCESS_MIO_DUAL);
	} else if (op->data.buswidth == 1) {
		val &= ~QSPI_MIO_INF_DC;
		val |= QSPI_MIO_INF(QSPI_ACCESS_MIO_SINGLE);
	} else {
		dev_err(qspi->dev, "!err buswidth %d.\n", op->data.buswidth);
		return -1;
	}

	if (op->addr.buswidth == 4 || op->addr.buswidth == 2)
		val |= QSPI_MIO_INF_AC;
	if (op->cmd.buswidth == 4 || op->cmd.buswidth == 2)
		val |= QSPI_MIO_INF_CC;
	ca_reg_write(qspi, val, QSPI_ACCESS_REG);
	return 0;
}



static int ca_qspi_issue_cmd(struct ca_qspi *qspi,
	struct spi_mem_op *op, u8 opcode)
{
	u8 addr_cnt = op->addr.nbytes;

	u32 val = 0;
	unsigned int data_cnt = op->data.nbytes;
	u32 addr_offset = (u32)op->addr.val;
	u8 dummy_cnt = op->dummy.buswidth != 0 ? ((op->dummy.nbytes * 8)
		/ op->dummy.buswidth) : (op->dummy.nbytes * 8);
    u32 ext_data_cnt;

	val |= QSPI_ACCODE(opcode);
	ca_reg_write(qspi, val, QSPI_ACCESS_REG);
	if (opcode == QSPI_AC_OP_EXTEND) {
		if (data_cnt > 6) {
			if (ca_qspi_buswidth_set(qspi, op))
				return -1;
		}
		/* Setup burst mode */
		if (!qspi->fifo_mode && (data_cnt > 4)) {
			val = ca_reg_read(qspi, QSPI_ACCESS_REG);
			val &= ~QSPI_FORCE_TERM;
			val |= QSPI_FORCE_BURST;
			ca_reg_write(qspi, val, QSPI_ACCESS_REG);
			data_cnt = 4;
		}
		ext_data_cnt = (RTK_SOC_TAURUS_GEN)?(op->data.nbytes):data_cnt;
		ca_reg_write(qspi, 0x0, QSPI_EXT_ACCESS);
		val = ca_reg_read(qspi, QSPI_EXT_ACCESS);
		val |= (QSPI_EXT_OPCODE(op->cmd.opcode)
			| QSPI_EXT_DUMY_CNT(dummy_cnt - 1)
			| QSPI_EXT_ADDR_CNT(addr_cnt - 1)
			| QSPI_EXT_DATA_CNT(ext_data_cnt - 1)
			| QSPI_EXT_DRD_CMD_EN);
		ca_reg_write(qspi, val, QSPI_EXT_ACCESS);
		if (ca_qspi_xfer_data(qspi, op))
			return -1;
		/* Exit burst mode */
		if (!qspi->fifo_mode && (op->data.nbytes > 4)) {
			val = ca_reg_read(qspi, QSPI_ACCESS_REG);
			val &= ~QSPI_FORCE_BURST;
			val |= QSPI_FORCE_TERM;
			ca_reg_write(qspi, val, QSPI_ACCESS_REG);
		}
	} else {
		val = ca_reg_read(qspi, QSPI_ACCESS_REG);
		val |= QSPI_OPCODE(op->cmd.opcode);
		ca_reg_write(qspi, val, QSPI_ACCESS_REG);

		if (opcode == QSPI_AC_OP_4_ADDR) {
			/* Configure address length */
			val = ca_reg_read(qspi, QSPI_TYPE_REG);
			val &= ~QSPI_SIZE_MASK;
			if (addr_cnt > 3)
				val |= QSPI_SIZE(2);
			ca_reg_write(qspi, val, QSPI_TYPE_REG);
			if (addr_cnt > 0)
				ca_reg_write(qspi, (u32)addr_offset, QSPI_ADDR_REG);
		}
			if (ca_qspi_wait_cmd_select(qspi, op))
				return -1;
		    udelay(10);
			return 0;
		}
	return 0;
}
/*
static bool ca_qspi_mem_supports_op(struct spi_mem *mem,
				const struct spi_mem_op *op)
{
	return true;
}
*/
static int ca_qspi_exec_op(struct spi_mem *mem,
			     const struct spi_mem_op *op)
{
	struct ca_qspi *qspi = spi_master_get_devdata(mem->spi->master);
	u8 opcode;

	if (op->data.nbytes == 0 && op->addr.nbytes == 0) {
		opcode = QSPI_AC_OP;
	} else if (op->data.nbytes == 0 && op->addr.nbytes > 0) {
		opcode = QSPI_AC_OP_4_ADDR;
	} else if (op->data.nbytes > 0) {
		opcode = QSPI_AC_OP_EXTEND;
	} else {
		dev_err(qspi->dev,
			"!invalid opcode:(0x%02x)\n", op->cmd.opcode);
		return -1;
	}
	return ca_qspi_issue_cmd(qspi, (struct spi_mem_op *)op, opcode);
}

static void ca_qspi_hw_init(struct ca_qspi *qspi)
{
	u32 val = 0;

	val = QSPI_SIZE(2);
	ca_reg_write(qspi, val, QSPI_TYPE_REG);
	if (!RTK_SOC_TAURUS_GEN) {
    	val = (QSPI_TMR_CLK(0x07) | QSPI_TMR_SETUP(0x01)
    		| QSPI_TMR_HOLD(0x01) | QSPI_TMR_IDLE(0x01));
        ca_reg_write(qspi, val, QSPI_TIMING);
    }
}

static const struct spi_controller_mem_ops ca_qspi_mem_ops = {
	/* .supports_op = ca_qspi_mem_supports_op, */
	.exec_op = ca_qspi_exec_op,
};

static const struct of_device_id ca_qspi_of_match[] = {
	{.compatible = "cortina,ca-qspi"},
	{},
};

MODULE_DEVICE_TABLE(of, ca_qspi_of_match);

static int ca_qspi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct ca_qspi *qspi;
	struct spi_master *master;
	struct resource *res;
	u32 tx_bus_width = 0, rx_bus_width = 0;
	int ret = 0;
	u8 *i_buf;
	i_buf = kmalloc(QSPI_EXT_DATA_CNT_MAX, GFP_KERNEL);
	if(i_buf == NULL){
		return -ENOMEM;
	}

	/* Only support device-tree instantiation */
	if (!np) {
		kfree(i_buf);
		return -ENODEV;
	}
	if (!of_match_node(ca_qspi_of_match, np)) {
		kfree(i_buf);
		return -ENODEV;
	}
	master = spi_alloc_master(dev, sizeof(struct ca_qspi));
	if (!master) {
		dev_err(dev, "!error allocating spi_master\n");
		kfree(i_buf);
		return -ENOMEM;
	}
	master->bus_num = -1;
	if (!of_property_read_u32(np, "spi-tx-bus-width", &tx_bus_width)) {
		switch (tx_bus_width) {
		case 1:
			break;
		case 2:
			master->mode_bits |= SPI_TX_DUAL;
			break;
		case 4:
			master->mode_bits |= SPI_TX_QUAD;
			break;
		default:
			dev_warn(dev,
				"spi-tx-bus-width %d not supported\n",
				tx_bus_width);
			break;
		}
	}
	if (!of_property_read_u32(np, "spi-rx-bus-width", &rx_bus_width)) {
		switch (rx_bus_width) {
		case 1:
			break;
		case 2:
			master->mode_bits |= SPI_RX_DUAL;
			break;
		case 4:
			master->mode_bits |= SPI_RX_QUAD;
			break;
		default:
			dev_warn(dev,
				"spi-rx-bus-width %d not supported\n",
				rx_bus_width);
			break;
		}
	}
	master->mem_ops = &ca_qspi_mem_ops;
	master->dev.of_node = dev->of_node;
	qspi = spi_master_get_devdata(master);
	qspi->master = master;
	qspi->dev = &pdev->dev;
	qspi->tx_bus_width = (u8)tx_bus_width;
	qspi->rx_bus_width = (u8)rx_bus_width;
	qspi->fifo_mode = of_property_read_bool(np, "fifo-mode");
	qspi->ready_mask = (RTK_SOC_TAURUS_GEN)?(BIT(16)|QSPI_START_EN):QSPI_START_EN;
	qspi->internal_buf = i_buf;
	platform_set_drvdata(pdev, qspi);

	res= platform_get_resource_byname(pdev, IORESOURCE_MEM, "qspi-base");
	if (res) {
		qspi->base = devm_ioremap_resource(dev, res);
		if (IS_ERR(qspi->base)) {
			ret = PTR_ERR(qspi->base);
			goto err;
		}
		dev_info(dev, "ca-qspi resource - %pr mapped at 0x%pK\n",
			res, qspi->base);
	} else {
		dev_err(dev,
			"!can't get regs base addresses(ret = %d)!\n", ret);
		goto err;

	}
	ca_qspi_hw_init(qspi);
	ret = devm_spi_register_master(dev, master);
	if (ret < 0) {
		dev_err(dev, "!can't register master\n");
		goto err;
	}
	dev_info(dev,
		"spi-flash controller probed, while mode_bits=%#06x\n",
		master->mode_bits);
	return 0;
err:
	if(i_buf){
		kfree(i_buf);
	}
	spi_master_put(master);
	return ret;
}

static int ca_qspi_remove(struct platform_device *pdev)
{
	struct ca_qspi *qspi = platform_get_drvdata(pdev);
	if(qspi->internal_buf)
		kfree(qspi->internal_buf);

	spi_unregister_master(qspi->master);

	return 0;
}

static int __maybe_unused ca_qspi_suspend(struct device *dev)
{
    //struct ca_qspi *qspi = dev_get_drvdata(dev);
    return 0;
};

static int __maybe_unused ca_qspi_resume(struct device *dev)
{
    //struct ca_qspi *qspi = dev_get_drvdata(dev);
    return 0;
}

static SIMPLE_DEV_PM_OPS(ca_qspi_pm_ops, ca_qspi_suspend, ca_qspi_resume);

static struct platform_driver ca_qspi_driver = {
	.probe	= ca_qspi_probe,
	.remove	= ca_qspi_remove,
	.driver	= {
		.name	= "ca-qspi",
		.pm =   &ca_qspi_pm_ops,
		.of_match_table	= ca_qspi_of_match,
	}
};

module_platform_driver(ca_qspi_driver);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("CORTINA ACCESS QSPI Controller Driver");
MODULE_ALIAS("platform:ca-qspi");
