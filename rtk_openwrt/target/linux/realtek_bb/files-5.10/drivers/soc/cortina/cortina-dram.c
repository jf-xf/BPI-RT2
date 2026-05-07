/*
 * Copyright (c) Cortina-Access Limited 2017.  All rights reserved.
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
 * cortina-dram.: Cortina DRAM calibration driver
 *
 */

#include <soc/cortina/registers.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <soc/cortina/ca_dram_ctrl.h>

spinlock_t zq_cal_lock;
struct timer_list ca_soc_dram_tl;
struct ca_dram_priv_data ca_dram_priv;

#define read(A)	readl(ca_dram_priv.ddr_ctrl_base + (A - CRT_CTL))
#define write(A,V)	writel(V ,ca_dram_priv.ddr_ctrl_base + (A - CRT_CTL))

int ddr_phy_zq_cal(void)
{
	unsigned int reg_data;
	unsigned char redo_set0_counter, redo_set1_counter;
	unsigned char zq_cal_error;
	int rc = 0;

	write(PAD_BUS_1, 0);

	reg_data = read(PAD_CTRL_PROG);
	reg_data &= ~0x70000000;	//  select set0 to store ZQ result
	reg_data &= ~(1 << 27);		//  not auto update when calibration is done
	reg_data &= ~(1 << 18);		//  not update during refresh and so no any update at all
	write(PAD_CTRL_PROG, reg_data);

//  PAD_BUS_0: nothing needs to change
//  DPI_CTRL_0: nothing needs to change

	reg_data = 0x0028763B;		//  zprog fro set0
	write(PAD_CTRL_ZPROG, reg_data);

	zq_cal_error = 1;
	redo_set0_counter = 0;
	while ((zq_cal_error == 1) && (redo_set0_counter < 3)) {
//  start ZQ calibration for set0
		reg_data = read(PAD_CTRL_PROG);
		reg_data |= (1 << 24);
		write(PAD_CTRL_PROG, reg_data);

		reg_data = 0;
		while ((reg_data & 0x80000000) == 0) {
			reg_data = read(PAD_ZCTRL_STATUS);
		}

		if (reg_data & 0x1fE00000)	/* bit 21~28 */
			zq_cal_error = 1;
		else
			zq_cal_error = 0;

//  stop ZQ calibration
		reg_data = read(PAD_CTRL_PROG);
		reg_data &= ~(1 << 24);
		write(PAD_CTRL_PROG, reg_data);

		redo_set0_counter++;
	}

	reg_data = read(PAD_CTRL_PROG);
	reg_data &= ~0x70000000;
	reg_data |= (1 << 28);		//  select set1 to store ZQ result
	write(PAD_CTRL_PROG, reg_data);

	reg_data = 0x0020763B;		//  zprog fro set1
	write(PAD_CTRL_ZPROG, reg_data);

	zq_cal_error = 1;
	redo_set1_counter = 0;
	while ((zq_cal_error == 1) && (redo_set1_counter < 3)) {
//  start ZQ calibration for set1
		reg_data = read(PAD_CTRL_PROG);
		reg_data |= (1 << 24);
		write(PAD_CTRL_PROG, reg_data);

		reg_data = 0;
		while ((reg_data & 0x80000000) == 0) {
			reg_data = read(PAD_ZCTRL_STATUS);
		}

		if (reg_data & 0x1fE00000)	/* bit 21~28 */
			zq_cal_error = 1;
		else
			zq_cal_error = 0;
//  stop ZQ calibration
		reg_data = read(PAD_CTRL_PROG);
		reg_data &= ~(1 << 24);
		write(PAD_CTRL_PROG, reg_data);

		redo_set1_counter++;
	}

	if ((redo_set0_counter < 3) && (redo_set1_counter < 3)) {
		reg_data = read(PAD_CTRL_PROG);
		reg_data |= (1 << 18);	//  select update during refresh only after ZQ calibration is successful
		write(PAD_CTRL_PROG, reg_data);
	}

	reg_data = 0;
#if defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN2_SFU)
	reg_data |= (1 << 6);		//  zq pad PD
#else
	reg_data |= (1 << 4);		//  zq pad PD
#endif
	write(PAD_BUS_1, reg_data);

	return rc;
}								//  ddr_phy_zq_cal

void read_DQS_IN_DLY_reg(unsigned char which_bit, unsigned char rising_edge,
						 unsigned int *reg_data)
{
//-----------------------------------------------------------------------------
	unsigned int reg_addr;
	if (rising_edge) {
		if ((which_bit >= 0) && (which_bit <= 3))
			reg_addr = DQS_IN_DLY_0_0;
		else if ((which_bit >= 4) && (which_bit <= 7))
			reg_addr = DQS_IN_DLY_1_0;
		else if ((which_bit >= 8) && (which_bit <= 11))
			reg_addr = DQS_IN_DLY_0_1;
		else if ((which_bit >= 12) && (which_bit <= 15))
			reg_addr = DQS_IN_DLY_1_1;
		else if ((which_bit >= 16) && (which_bit <= 19))
			reg_addr = DQS_IN_DLY_0_2;
		else if ((which_bit >= 20) && (which_bit <= 23))
			reg_addr = DQS_IN_DLY_1_2;
		else if ((which_bit >= 24) && (which_bit <= 27))
			reg_addr = DQS_IN_DLY_0_3;
		else
			reg_addr = DQS_IN_DLY_1_3;
	} else {
		if ((which_bit >= 0) && (which_bit <= 3))
			reg_addr = DQS_IN_DLY_2_0;
		else if ((which_bit >= 4) && (which_bit <= 7))
			reg_addr = DQS_IN_DLY_3_0;
		else if ((which_bit >= 8) && (which_bit <= 11))
			reg_addr = DQS_IN_DLY_2_1;
		else if ((which_bit >= 12) && (which_bit <= 15))
			reg_addr = DQS_IN_DLY_3_1;
		else if ((which_bit >= 16) && (which_bit <= 19))
			reg_addr = DQS_IN_DLY_2_2;
		else if ((which_bit >= 20) && (which_bit <= 23))
			reg_addr = DQS_IN_DLY_3_2;
		else if ((which_bit >= 24) && (which_bit <= 27))
			reg_addr = DQS_IN_DLY_2_3;
		else
			reg_addr = DQS_IN_DLY_3_3;
	}
	*reg_data = read(reg_addr);
}

void fill_DQS_IN_DLY_reg(int which_bit, unsigned char rising_edge,
						 unsigned int reg_data)
{
	unsigned int reg_addr;
	if (rising_edge) {
		if ((which_bit >= 0) && (which_bit <= 3))
			reg_addr = DQS_IN_DLY_0_0;
		else if ((which_bit >= 4) && (which_bit <= 7))
			reg_addr = DQS_IN_DLY_1_0;
		else if ((which_bit >= 8) && (which_bit <= 11))
			reg_addr = DQS_IN_DLY_0_1;
		else if ((which_bit >= 12) && (which_bit <= 15))
			reg_addr = DQS_IN_DLY_1_1;
		else if ((which_bit >= 16) && (which_bit <= 19))
			reg_addr = DQS_IN_DLY_0_2;
		else if ((which_bit >= 20) && (which_bit <= 23))
			reg_addr = DQS_IN_DLY_1_2;
		else if ((which_bit >= 24) && (which_bit <= 27))
			reg_addr = DQS_IN_DLY_0_3;
		else
			reg_addr = DQS_IN_DLY_1_3;
	} else {
		if ((which_bit >= 0) && (which_bit <= 3))
			reg_addr = DQS_IN_DLY_2_0;
		else if ((which_bit >= 4) && (which_bit <= 7))
			reg_addr = DQS_IN_DLY_3_0;
		else if ((which_bit >= 8) && (which_bit <= 11))
			reg_addr = DQS_IN_DLY_2_1;
		else if ((which_bit >= 12) && (which_bit <= 15))
			reg_addr = DQS_IN_DLY_3_1;
		else if ((which_bit >= 16) && (which_bit <= 19))
			reg_addr = DQS_IN_DLY_2_2;
		else if ((which_bit >= 20) && (which_bit <= 23))
			reg_addr = DQS_IN_DLY_3_2;
		else if ((which_bit >= 24) && (which_bit <= 27))
			reg_addr = DQS_IN_DLY_2_3;
		else
			reg_addr = DQS_IN_DLY_3_3;
	}
	write(reg_addr, reg_data);
}								//  fill_DQS_IN_DLY_reg

void trigger_phy_fw_set_rd_dly(void)
{
	write(DPI_CTRL_1, 0x0C);
}

void dqs_in_dealy_in_os(void)
{
//  assume dqs_in_dly_start_value is the variable that software saved during DDR training
//-----------------------------------------------------------------------------
	unsigned int reg_data, tmp_reg;
	unsigned char new_cyc_value;
	int i, j, inc_dec_delay, tmp, decrease = 0;

//  detect cycle delay tap number
	reg_data = 0;
	reg_data |= (1 << 7);		//  det_enable
	write(HALF_CYC_DET, reg_data);
	while ((reg_data & 0x40) == 0) {
		reg_data = read(HALF_CYC_DET);
	}
#if defined(CONFIG_ARCH_CORTINA_SATURN_SFU) || defined(CONFIG_ARCH_CORTINA_SATURN_SFP_PLUS)
	reg_data |= (1 << 6);		//  zq pad PD
	new_cyc_value = ((reg_data & 0x3F) + 1) / 2;
    if (new_cyc_value > 20)
        new_cyc_value = 20;     //  treat it the same as dqs_in_dly_start_value
#else
	new_cyc_value = ((reg_data & 0x3F) + 2) / 4;
#endif
	reg_data &= ~(1 << 7);		//  turn off detection
	write(HALF_CYC_DET, reg_data);

	if (new_cyc_value < ca_dram_priv.dqs_in_dly_start_value)
		decrease = 1;

	if (decrease == 1)
		inc_dec_delay = ca_dram_priv.dqs_in_dly_start_value - new_cyc_value;
	else
		inc_dec_delay = new_cyc_value - ca_dram_priv.dqs_in_dly_start_value;

	if (inc_dec_delay != 0) {
		//printk("\n%s delay:%x\n", decrease ? "Dec" : "Inc", inc_dec_delay);
		for (i = 0; i < ca_dram_priv.byte_lane * 2; i++) {
			//printk("Byte Lane %d\n",i);
			for (j = 0; j < 2; j++) {
				read_DQS_IN_DLY_reg(i * 4, j, &tmp_reg);

				//printk("DLY:0x%x => ",tmp_reg);
				reg_data = tmp_reg & ~(0x1F1F1F1F);
				if (decrease == 1)
					tmp = ((tmp_reg >> 24) & 0x1F) - inc_dec_delay;
				else
					tmp = ((tmp_reg >> 24) & 0x1F) + inc_dec_delay;
				reg_data |= (tmp << 24);

				if (decrease == 1)
					tmp = ((tmp_reg >> 16) & 0x1F) - inc_dec_delay;
				else
					tmp = ((tmp_reg >> 16) & 0x1F) + inc_dec_delay;
				reg_data |= (tmp << 16);

				if (decrease == 1)
					tmp = ((tmp_reg >> 8) & 0x1F) - inc_dec_delay;
				else
					tmp = ((tmp_reg >> 8) & 0x1F) + inc_dec_delay;
				reg_data |= (tmp << 8);

				if (decrease == 1)
					tmp = ((tmp_reg >> 0) & 0x1F) - inc_dec_delay;
				else
					tmp = ((tmp_reg >> 0) & 0x1F) + inc_dec_delay;
				reg_data |= (tmp << 0);
				//printk("0x%x\n",reg_data);
				fill_DQS_IN_DLY_reg(i * 4, j, reg_data);
			}
		}
		ca_dram_priv.dqs_in_dly_start_value = new_cyc_value;
		trigger_phy_fw_set_rd_dly();	//  there is a tiny chance this may not success
		trigger_phy_fw_set_rd_dly();
	}
}

int do_za_calib(void *arg)
{
	//spin_lock_irqsave(&zq_cal_lock, flags);
	/* Provided by SK for G3 DDR PHY */
	ddr_phy_zq_cal();

	//  do this here for dqs_in_dealy_in_os() and it only needs one programming in Linux
	//reg_data = read(DPI_CTRL_0);
	//reg_data &= ~0x3;
	//reg_data |= 0x1;			//  update during refresh
	//write(DPI_CTRL_0, reg_data);

	//dqs_in_dealy_in_os();
	//spin_unlock_irqrestore(&zq_cal_lock, flags);

	/* TBD, double DDR refresh rate if (Temperature >= 80) degree Cels..
	 * We need thermal sensor to for this adaptation.
	 */
#if 0
	if ((temperature > ca_dram_priv.high_temp_threshold) &&
		(ca_dram_priv.last_temp < ca_dram_priv.high_temp_threshold)) {

		/* Double  */
		reg_data = read(PCTL_DRR);
		tmp = (reg_data >> 8) & 0xFFFF;
		tmp /= 2;				/* Double refresh rate */
		reg_data &= ~0x00FFFF00;
		reg_data |= (tmp << 8);
		write(PCTL_DRR, reg_data);
	} else if ((temperature < ca_dram_priv.low_temp_threshold) &&
			   (ca_dram_priv.last_temp > ca_dram_priv.low_temp_threshold)) {

		/* Reduce refresh rate */
		reg_data = read(PCTL_DRR);
		tmp = (reg_v >> 8) & 0xFFFF;
		tmp *= 2;				/* reduce refresh rate */
		reg_data &= ~0x00FFFF00;
		reg_data |= (tmp << 8);
		write(PCTL_DRR, reg_data);
	}
	ca_dram_priv.last_temp = temperature;
#endif

	ca_soc_dram_tl.expires = jiffies + HZ * ca_dram_priv.calibration_time_gap;
	add_timer(&ca_soc_dram_tl);
	return 0;
}

static int ca_soc_dram_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device_node *np = pdev->dev.of_node;
	unsigned int duration, tmp;

	/* DDR controller */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "cannot get DDR_CTRL resource\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource #0\n");
		return -EBUSY;
	}

	ca_dram_priv.ddr_ctrl_base = ioremap(res->start, resource_size(res));
	if (!ca_dram_priv.ddr_ctrl_base) {
		dev_err(&pdev->dev, "cannot ioremap DDR_CTRL resource\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}
	printk("ddr_ctrl_base@0x%lx\n", (unsigned long)ca_dram_priv.ddr_ctrl_base);

	/* DDR parameter buffer */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_err(&pdev->dev, "cannot get  parameter buffer\n");
		return -EBUSY;
	}

	res = request_mem_region(res->start, resource_size(res), res->name);
	if (!res) {
		dev_err(&pdev->dev, "mmio already reserved for resource #1\n");
		return -EBUSY;
	}

	ca_dram_priv.ddr_parameter_buff = ioremap(res->start, resource_size(res));
	if (!ca_dram_priv.ddr_parameter_buff) {
		dev_err(&pdev->dev, "cannot ioremap parameter buffer\n");
		release_resource(res);
		kfree(res);
		return -ENODEV;
	}

	printk("ddr_parameter buff@0x%lx\n", (unsigned long)ca_dram_priv.ddr_parameter_buff);
	ca_dram_priv.dqs_in_dly_start_value =
		readl(ca_dram_priv.ddr_parameter_buff);
	ca_dram_priv.byte_lane = readl(ca_dram_priv.ddr_parameter_buff + 0x4);

	printk("dqs_in_dly_start_value=0x%x\n",
		   ca_dram_priv.dqs_in_dly_start_value);

	//release_resource(res);
//	kfree(res);

	if (!of_property_read_u32(np, "calibrate-druation", &duration))
		ca_dram_priv.calibration_time_gap = duration;
	printk("ZQ recalibration every %d seconds\n", duration);

	if (!of_property_read_u32(np, "low_temp_threshold", &tmp))
		ca_dram_priv.low_temp_threshold = tmp;
	printk("Low temperature threshold %d degree\n", tmp);

	if (!of_property_read_u32(np, "high_temp_threshold", &tmp))
		ca_dram_priv.high_temp_threshold = tmp;
	printk("High temperature threshold %d degree\n", tmp);

	ca_dram_priv.last_temp = 45;	/* Assume 50 degree on boot stage */

	/* set calibration timer */
	ca_soc_dram_tl.function = (void *)do_za_calib;
	ca_soc_dram_tl.data = 0;
	init_timer(&ca_soc_dram_tl);
	ca_soc_dram_tl.expires = jiffies + HZ * ca_dram_priv.calibration_time_gap;
	add_timer(&ca_soc_dram_tl);

	return 0;
}

static int ca_soc_dram_remove(struct platform_device *pdev)
{
	iounmap(ca_dram_priv.ddr_ctrl_base);
	return 0;
}

static const struct of_device_id ca_soc_dram_dt_match[] = {
	{.compatible = "cortina,g3-dram",},
	{},
};

MODULE_DEVICE_TABLE(of, ca_soc_dt_match);

static struct platform_driver ca_soc_dram_driver = {
	.driver = {
			   .name = "ca_g3_dram",.owner = THIS_MODULE,.of_match_table =
			   ca_soc_dram_dt_match,},.probe = ca_soc_dram_probe,.remove =
		ca_soc_dram_remove,
};

module_platform_driver(ca_soc_dram_driver);

MODULE_AUTHOR("Jason Li <jaso.li@coritna-access.com>");
MODULE_DESCRIPTION("Cortina SoC driver");
MODULE_LICENSE("GPL v2");
