/*
 * Cortina-Access Peripheral WDT driver
 *
 * Copyright (C) 2017 Cortina Access, Inc.
 *		http://www.cortina-access.com
 *
 * Based on cadence_wdt.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/watchdog.h>
#include <soc/cortina/cortina-soc.h>

#define CORTINA_WDT_DEFAULT_TIMEOUT		60
#define CORTINA_WDT_DEFAULT_HIGH_RESOLUTION	0

/* Supports 1 - 2^32 or 2^32/1000 sec */
#define CORTINA_WDT_MIN_TIMEOUT	1

#define SUPPORT_NOTIFIER_CALL 0 /* RTK doesn't use it */
static int wdt_timeout = CORTINA_WDT_DEFAULT_TIMEOUT;
static int nowayout = WATCHDOG_NOWAYOUT;
static int high_resolution = CORTINA_WDT_DEFAULT_HIGH_RESOLUTION;

module_param(wdt_timeout, int, 0);
MODULE_PARM_DESC(wdt_timeout,
		 "Watchdog time in seconds. (default="
		 __MODULE_STRING(CORTINA_WDT_DEFAULT_TIMEOUT) ")");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
		 "Watchdog cannot be stopped once started (default="
		 __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

module_param(high_resolution, int, 0);

/************* RTK LUNA_WDT_SUPPORT************************************/
#ifdef CONFIG_CORITNA_PER_WATCHDOG_KTHREAD_KICK
	#define LUNA_WDT_SUPPORT 	1
#else
	#define LUNA_WDT_SUPPORT 	0
#endif
#if LUNA_WDT_SUPPORT
struct watchdog_device *rtk_wdt_device = NULL;
static void wdt_thread_maintain(int flag);

static void RTK_WDG_EN(void);
static void RTK_WDG_DIS(void);
static void RTK_WDG_KICK(void);
static void luna_watchdog_thread_init(struct watchdog_device *wdd);
unsigned int watchdog_flag = 0;
unsigned int kick_wdg_time = 5;

#endif
/**** func***********************************************/
static int cortina_wdt_reload(struct watchdog_device *wdd);
static int cortina_wdt_start(struct watchdog_device *wdd);
static int cortina_wdt_stop(struct watchdog_device *wdd);

/**
 * struct cortina_wdt - Watchdog device structure
 * @regs: baseaddress of device
 * @rst: reset flag
 * @clk: struct clk * of a clock source
 * @prescaler: for saving prescaler value
 * @ctrl_clksel: counter clock prescaler selection
 * @io_lock: spinlock for IO register access
 * @cortina_wdt_device: watchdog device structure
 * @cortina_wdt_notifier: notifier structure
 *
 * Structure containing parameters specific to cadence watchdog.
 */
struct cortina_wdt {
	void __iomem		*regs;
	void __iomem		*rst_reg;
	struct clk		*clk;
	u32			prescaler;
	u16			ps_div;
	u32			delay_reset;
	bool			rst;
	bool			ctrl_clksel;
	spinlock_t		io_lock;	/* the lock for io operations */
	struct watchdog_device	cortina_wdt_device;
	struct notifier_block	cortina_wdt_notifier;
};

/* Read access to Registers */
static inline u32 cortina_wdt_readreg(struct cortina_wdt *wdt, u32 offset)
{
	return readl(wdt->regs + offset);
}

/* Write access to Registers */
static inline void cortina_wdt_writereg(struct cortina_wdt *wdt, u32 offset,
					u32 val)
{
	writel_relaxed(val, wdt->regs + offset);
}

/*************************Register Map**************************************/

/* PER_WDT Registers */
#define CORTINA_WDT_CTRL	0x00		/* control */
#define   WDT_CTRL_WDTEN	  BIT(0)
#define   WDT_CTRL_RSTEN	  BIT(1)
#define   WDT_CTRL_CLKSEL	  BIT(2)
#define   WDT_CTRL_DELAY_SHFT	  12
#define   WDT_CTRL_MAX_DELAY	  0x000FFFFF
#define CORTINA_WDT_PS		0x04		/* pre-scaler load value */
#define CORTINA_WDT_DIV		0x08		/* divider for the pre-scaler */
#define CORTINA_WDT_LD		0x0C		/* load value */
#define CORTINA_WDT_LOADE	0x10		/* load enable */
#define   WDT_LOADE_WDT		  BIT(0)
#define   WDT_LOADE_PRE		  BIT(1)
#define CORTINA_WDT_CNT		0x14		/* instantaneous value */
#define CORTINA_WDT_IE_0	0x18		/* interrupt enable */
#define   WDT_IE_WDTE		  BIT(0)
#define CORTINA_WDT_INT_0	0x1C		/* interrupt */
#define   WDT_INT_WDTI		  BIT(0)
#define CORTINA_WDT_STAT_0	0x20		/* interrupt status */
#define   WDT_STAT_WDTS		  BIT(0)

/* Global Config Register */
#define WD_RESET_SUBSYS_ENABLE	BIT(4)
#define WD_RESET_PCIE			BIT(5)
#define WD_RESET_ALL_BLOCKS	BIT(6)
#define WD_RESET_REMAP		BIT(7)
#define WD_RESET_EXT_RESET	BIT(8)
#define EXT_RESET		BIT(9)

/*****************************************************/
/**
 * cortina_wdt_stop - Stop the watchdog.
 *
 * @wdd: watchdog device
 *
 * Read the contents of the ZMR register, clear the WDEN bit
 * in the register and set the access key for successful write.
 *
 * Return: always 0
 */
static int cortina_wdt_stop(struct watchdog_device *wdd)
{
	struct cortina_wdt *wdt = watchdog_get_drvdata(wdd);
	u32 val;

	spin_lock(&wdt->io_lock);

	val = cortina_wdt_readreg(wdt, CORTINA_WDT_CTRL);
	val &= ~WDT_CTRL_WDTEN;
	cortina_wdt_writereg(wdt, CORTINA_WDT_CTRL, val);

	spin_unlock(&wdt->io_lock);

	return 0;
}

/**
 * cortina_wdt_reload - Reload the watchdog timer (i.e. pat the watchdog).
 *
 * @wdd: watchdog device
 *
 * Write the restart key value (0x00001999) to the restart register.
 *
 * Return: always 0
 */
static int cortina_wdt_reload(struct watchdog_device *wdd)
{
	struct cortina_wdt *wdt = watchdog_get_drvdata(wdd);
	u32 val;

	spin_lock(&wdt->io_lock);

	val = WDT_LOADE_WDT | WDT_LOADE_PRE;
	cortina_wdt_writereg(wdt, CORTINA_WDT_LOADE, val);

	spin_unlock(&wdt->io_lock);

	return 0;
}

/**
 * cortina_wdt_start - Enable and start the watchdog.
 *
 * @wdd: watchdog device
 *
 * The counter value is calculated according to the formula:
 *		calculated count = (timeout * clock) / prescaler + 1.
 * The calculated count is divided by 0x1000 to obtain the field value
 * to write to counter control register.
 * Clears the contents of prescaler and counter reset value. Sets the
 * prescaler to 4096 and the calculated count and access key
 * to write to CCR Register.
 * Sets the WDT (WDEN bit) and either the Reset signal(RSTEN bit)
 * or Interrupt signal(IRQEN) with a specified cycles and the access
 * key to write to ZMR Register.
 *
 * Return: always 0
 */
static int cortina_wdt_start(struct watchdog_device *wdd)
{
	struct cortina_wdt *wdt = watchdog_get_drvdata(wdd);
	unsigned int data = 0;
	unsigned short count;
	unsigned int clock_rate, delay_cycle;
	extern void rtk_soc_reboot_reason_set(int);

	count = high_resolution ? wdd->timeout * 1000 : wdd->timeout;

	spin_lock(&wdt->io_lock);

	cortina_wdt_writereg(wdt, CORTINA_WDT_PS, wdt->prescaler);
	cortina_wdt_writereg(wdt, CORTINA_WDT_DIV, wdt->ps_div);
	cortina_wdt_writereg(wdt, CORTINA_WDT_LD, count);

	cortina_wdt_writereg(wdt, CORTINA_WDT_IE_0, WDT_IE_WDTE);

	/* Reset on timeout if specified in device tree. */
	if (wdt->rst)
		data |= WDT_CTRL_RSTEN;
	if (wdt->ctrl_clksel)
		data |= WDT_CTRL_CLKSEL;
	data |= WDT_CTRL_WDTEN;
	if (wdt->delay_reset) {
		clock_rate = clk_get_rate(wdt->clk);
		delay_cycle = wdt->delay_reset * (clock_rate / 16000);
		if (delay_cycle >= WDT_CTRL_MAX_DELAY) {
			/* Set to maximum delay if overflow */
			delay_cycle = WDT_CTRL_MAX_DELAY;
		}
		data |= delay_cycle << WDT_CTRL_DELAY_SHFT;
	}
	cortina_wdt_writereg(wdt, CORTINA_WDT_CTRL, data);

	spin_unlock(&wdt->io_lock);

	rtk_soc_reboot_reason_set(3);

	return 0;
}

/**
 * cortina_wdt_settimeout - Set a new timeout value for the watchdog device.
 *
 * @wdd: watchdog device
 * @new_time: new timeout value that needs to be set
 * Return: 0 on success
 *
 * Update the watchdog_device timeout with new value which is used when
 * cortina_wdt_start is called.
 */
static int __maybe_unused  cortina_wdt_settimeout(struct watchdog_device *wdd,
				  unsigned int new_time)
{
	wdd->timeout = new_time;

	return cortina_wdt_start(wdd);
}

/**
 * cortina_wdt_settimeout - Set a new timeout value for the watchdog device.
 *
 * @wdd: watchdog device
 * @new_time: new timeout value that needs to be set
 * Return: 0 on success
 *
 * Update the watchdog_device timeout with new value which is used when
 * cortina_wdt_start is called.
 */
static unsigned int __maybe_unused cortina_wdt_gettimeleft(struct watchdog_device *wdd)
{
	struct cortina_wdt *wdt = watchdog_get_drvdata(wdd);
	u32 val;

	spin_lock(&wdt->io_lock);

	val = cortina_wdt_readreg(wdt, CORTINA_WDT_CNT);

	spin_unlock(&wdt->io_lock);

	return high_resolution ? val / 1000 : val;
}

/**
 * cortina_wdt_irq_handler - Notifies of watchdog timeout.
 *
 * @irq: interrupt number
 * @dev_id: pointer to a platform device structure
 * Return: IRQ_HANDLED
 *
 * The handler is invoked when the watchdog times out and a
 * reset on timeout has not been enabled.
 */
static irqreturn_t cortina_wdt_irq_handler(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct cortina_wdt *wdt = platform_get_drvdata(pdev);

	spin_lock(&wdt->io_lock);

	cortina_wdt_writereg(wdt, CORTINA_WDT_INT_0, WDT_INT_WDTI);
	cortina_wdt_writereg(wdt, CORTINA_WDT_IE_0, 0);

	spin_unlock(&wdt->io_lock);

	dev_info(&pdev->dev,
		 "Watchdog timed out. Internal reset not enabled\n");

	return IRQ_HANDLED;
}

/*
 * Info structure used to indicate the features supported by the device
 * to the upper layers. This is defined in watchdog.h header file.
 */
static struct watchdog_info cortina_wdt_info = {
	.identity	= "cdns_wdt watchdog",
	.options	= WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
			  WDIOF_MAGICCLOSE,
};

#ifdef CONFIG_CORITNA_PER_WATCHDOG_KTHREAD_KICK
/* Watchdog Core Ops */
static int cortina_wdt_dumy(struct watchdog_device *wdd){
	printk("\ncortina,wdt: Not support userspace daemon. Uses Kthread instead.\n");
	return 0;
}
static int cortina_wdt_dumy_settime(struct watchdog_device *wdd, unsigned int newtime){
	printk("\ncortina,wdt: Not support userspace daemon. Uses Kthread instead.\n");
	return 0;
}
static unsigned int cortina_wdt_dumy_gettime(struct watchdog_device *wdd){
	printk("\ncortina,wdt: Not support userspace daemon. Uses Kthread instead.\n");
	return 0;
}
static struct watchdog_ops cortina_wdt_ops = {
	.owner = THIS_MODULE,
	.start = cortina_wdt_dumy,
	.stop = cortina_wdt_dumy,
	.ping = cortina_wdt_dumy,
	.set_timeout = cortina_wdt_dumy_settime,
	.get_timeleft =	cortina_wdt_dumy_gettime,
};
#else
/* Watchdog Core Ops */
static struct watchdog_ops cortina_wdt_ops = {
	.owner = THIS_MODULE,
	.start = cortina_wdt_start,
	.stop = cortina_wdt_stop,
	.ping = cortina_wdt_reload,
	.set_timeout = cortina_wdt_settimeout,
	.get_timeleft =	cortina_wdt_gettimeleft,
};
#endif
#if SUPPORT_NOTIFIER_CALL
/**
 * cortina_wdt_notify_sys - Notifier for reboot or shutdown.
 *
 * @this: handle to notifier block
 * @code: turn off indicator
 * @unused: unused
 * Return: NOTIFY_DONE
 *
 * This notifier is invoked whenever the system reboot or shutdown occur
 * because we need to disable the WDT before system goes down as WDT might
 * reset on the next boot.
 */
static int cortina_wdt_notify_sys(struct notifier_block *this,
				  unsigned long code, void *unused)
{
	struct cortina_wdt *wdt = container_of(this, struct cortina_wdt,
					    cortina_wdt_notifier);
	if (code == SYS_DOWN || code == SYS_HALT)
		cortina_wdt_stop(&wdt->cortina_wdt_device);

	return NOTIFY_DONE;
}
#endif

#if defined(CONFIG_ARCH_REALTEK_9607F)
#define DDR_SET_MODE_SUPPORT 0 //Workaround, todo  0301
#else
#define DDR_SET_MODE_SUPPORT 0
#endif

#if DDR_SET_MODE_SUPPORT
#define CONFIG_DDR_MODE_COLD_RESET	0x0
#define CONFIG_DDR_MODE_WARM_RESET	0x1

#define GLOBAL_DDR_RESET_MODE           0xf43203c8 /* 9607F */

/******************************************************
 * TAURUS(8277C)/Venus don't  support CONFIG_DDR_MODE_WARM_RESET
 *****************************************************/
static void config_ddr_reset_mode_set(void){
	void __iomem *addr;
	addr = ioremap(GLOBAL_DDR_RESET_MODE, 4);
	if (addr) {
		writel_relaxed(CONFIG_DDR_MODE_WARM_RESET, addr);
		iounmap(addr);
	}else{
		printk("WDT: config_ddr_mode_warm_reset failed!");
	}
}
#else
#define config_ddr_reset_mode_set()
#endif
/************************Platform Operations*****************************/
/**
 * cortina_wdt_probe - Probe call for the device.
 *
 * @pdev: handle to the platform device structure.
 * Return: 0 on success, negative error otherwise.
 *
 * It does all the memory allocation and registration for the device.
 */
static int cortina_wdt_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret, irq;
	unsigned long clock_f;
	struct cortina_wdt *wdt;
	struct watchdog_device *cortina_wdt_device;

	wdt = devm_kzalloc(&pdev->dev, sizeof(*wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	cortina_wdt_device = &wdt->cortina_wdt_device;
	cortina_wdt_device->info = &cortina_wdt_info;
	cortina_wdt_device->ops = &cortina_wdt_ops;
	cortina_wdt_device->timeout = CORTINA_WDT_DEFAULT_TIMEOUT;
	cortina_wdt_device->min_timeout = CORTINA_WDT_MIN_TIMEOUT;
	if (high_resolution)
		cortina_wdt_device->max_timeout = U32_MAX / 1000;
	else
		cortina_wdt_device->max_timeout = U32_MAX;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	wdt->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (IS_ERR(wdt->regs))
		return PTR_ERR(wdt->regs);

	/* Delay between the WDT interrupt firing and the actual reset */
	ret = of_property_read_u32(pdev->dev.of_node, "delay-reset",
				   &wdt->delay_reset);
	if (ret != 0)
		wdt->delay_reset = 0;

	/* Register the interrupt */
	wdt->rst = of_property_read_bool(pdev->dev.of_node, "reset-on-timeout");
	irq = platform_get_irq(pdev, 0);
	if (irq >= 0) {
		ret = devm_request_irq(&pdev->dev, irq, cortina_wdt_irq_handler,
				       0, pdev->name, pdev);
		if (ret) {
			dev_err(&pdev->dev,
				"cannot register interrupt handler err=%d\n",
				ret);
			return ret;
		}
	}
	if (wdt->rst) {
		u32 val;

		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		wdt->rst_reg = devm_ioremap(&pdev->dev, res->start,
				resource_size(res));
		if (IS_ERR(wdt->rst_reg))
			return PTR_ERR(wdt->rst_reg);

		val = readl(wdt->rst_reg);
		val |= WD_RESET_SUBSYS_ENABLE | WD_RESET_ALL_BLOCKS |
		       WD_RESET_REMAP | WD_RESET_EXT_RESET;
#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS) ||  defined(CONFIG_ARCH_REALTEK_9607F)
		if (RTK_SOC_TAURUS_GEN){
			val |= WD_RESET_PCIE;
		}
		if (IS_ELNATH){
			config_ddr_reset_mode_set();
		}
#endif
		writel_relaxed(val, wdt->rst_reg);
	}

	/* Initialize the members of cortina_wdt structure */
	cortina_wdt_device->parent = &pdev->dev;

	ret = watchdog_init_timeout(cortina_wdt_device, wdt_timeout,
				    &pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "unable to set timeout value\n");
		return ret;
	}

	watchdog_set_nowayout(cortina_wdt_device, nowayout);
	watchdog_set_drvdata(cortina_wdt_device, wdt);

	wdt->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(wdt->clk)) {
		dev_err(&pdev->dev, "input clock not found\n");
		ret = PTR_ERR(wdt->clk);
		return ret;
	}

	ret = clk_prepare_enable(wdt->clk);
	if (ret) {
		dev_err(&pdev->dev, "unable to enable clock\n");
		return ret;
	}

	clock_f = clk_get_rate(wdt->clk);
	wdt->prescaler = clock_f / 1000 - 1; /* 1ms */
	if (high_resolution) {
		wdt->ps_div = 0;
		wdt->ctrl_clksel = 1;
	} else {
		wdt->ps_div = 1000; /* 1s */
		wdt->ctrl_clksel = 0;
	}

	spin_lock_init(&wdt->io_lock);
#if SUPPORT_NOTIFIER_CALL //Realtek, avoid reboot cmd hang but watchdong is also disabled
	wdt->cortina_wdt_notifier.notifier_call = &cortina_wdt_notify_sys;
	ret = register_reboot_notifier(&wdt->cortina_wdt_notifier);
	if (ret != 0) {
		dev_err(&pdev->dev, "cannot register reboot notifier err=%d)\n",
			ret);
		goto err_clk_disable;
	}
#endif
	ret = watchdog_register_device(cortina_wdt_device);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register wdt device\n");
		goto err_clk_disable;
	}
	platform_set_drvdata(pdev, wdt);

	dev_dbg(&pdev->dev, "Cortina Watchdog Timer at %p with timeout %ds%s\n",
		wdt->regs, cortina_wdt_device->timeout,
		nowayout ? ", nowayout" : "");
#if LUNA_WDT_SUPPORT
	{
		luna_watchdog_thread_init(cortina_wdt_device);
	}
#endif
	return 0;

err_clk_disable:
	clk_disable_unprepare(wdt->clk);

	return ret;
}

/**
 * cortina_wdt_remove - Probe call for the device.
 *
 * @pdev: handle to the platform device structure.
 * Return: 0 on success, otherwise negative error.
 *
 * Unregister the device after releasing the resources.
 */
static int cortina_wdt_remove(struct platform_device *pdev)
{
	struct cortina_wdt *wdt = platform_get_drvdata(pdev);

	cortina_wdt_stop(&wdt->cortina_wdt_device);
	watchdog_unregister_device(&wdt->cortina_wdt_device);
	unregister_reboot_notifier(&wdt->cortina_wdt_notifier);
	clk_disable_unprepare(wdt->clk);

	return 0;
}

#if SUPPORT_NOTIFIER_CALL
/**
 * cortina_wdt_shutdown - Stop the device.
 *
 * @pdev: handle to the platform structure.
 *
 */
static void cortina_wdt_shutdown(struct platform_device *pdev)
{
	struct cortina_wdt *wdt = platform_get_drvdata(pdev);

	cortina_wdt_stop(&wdt->cortina_wdt_device);
	clk_disable_unprepare(wdt->clk);
}
#endif

/**
 * cortina_wdt_suspend - Stop the device.
 *
 * @dev: handle to the device structure.
 * Return: 0 always.
 */
static int __maybe_unused cortina_wdt_suspend(struct device *dev)
{
	struct platform_device *pdev = container_of(dev,
			struct platform_device, dev);
	struct cortina_wdt *wdt = platform_get_drvdata(pdev);

	cortina_wdt_stop(&wdt->cortina_wdt_device);
	clk_disable_unprepare(wdt->clk);

	return 0;
}

/**
 * cortina_wdt_resume - Resume the device.
 *
 * @dev: handle to the device structure.
 * Return: 0 on success, errno otherwise.
 */
static int __maybe_unused cortina_wdt_resume(struct device *dev)
{
	int ret;
	struct platform_device *pdev = container_of(dev,
			struct platform_device, dev);
	struct cortina_wdt *wdt = platform_get_drvdata(pdev);

	ret = clk_prepare_enable(wdt->clk);
	if (ret) {
		dev_err(dev, "unable to enable clock\n");
		return ret;
	}
	cortina_wdt_start(&wdt->cortina_wdt_device);

	return 0;
}

static SIMPLE_DEV_PM_OPS(cortina_wdt_pm_ops, cortina_wdt_suspend,
			 cortina_wdt_resume);

static const struct of_device_id cortina_wdt_of_match[] = {
	{ .compatible = "cortina,wdt", },
	{ /* end of table */ }
};
MODULE_DEVICE_TABLE(of, cortina_wdt_of_match);

/* Driver Structure */
static struct platform_driver cortina_wdt_driver = {
	.probe		= cortina_wdt_probe,
	.remove		= cortina_wdt_remove,
#if 0 //Realtek, avoid reboot cmd hang but watchdong is also disabled
	.shutdown	= cortina_wdt_shutdown,
#endif
	.driver		= {
		.name	= "cortina-wdt",
		.of_match_table = cortina_wdt_of_match,
		.pm	= &cortina_wdt_pm_ops,
	},
};

module_platform_driver(cortina_wdt_driver);

MODULE_DESCRIPTION("Watchdog driver for Cortina-Access Peripheral WDT");
MODULE_LICENSE("GPL");





/************* RTK ************************************/

#if LUNA_WDT_SUPPORT
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>


#define _DEBUG_RTK_WDT_ALWAYS_   0xffffffff

#define DEBUG_RTK_WDT
#ifdef DEBUG_RTK_WDT
time64_t  start;
time64_t  end;
static unsigned int _debug_rtk_wdt_ = (_DEBUG_RTK_WDT_ALWAYS_);//|DEBUG_READ|DEBUG_WRITE|DEBUG_ERASE);
static int debug_mask = 0;
#define DEBUG_RTK_WDT_PRINT(mask, string) \
            if ((_debug_rtk_wdt_ & mask) || (mask == _DEBUG_RTK_WDT_ALWAYS_)) \
            printk string
#else
#define DEBUG_RTK_WDT_PRINT(mask, string)
#endif




struct task_struct *pWatchdogTask[NR_CPUS];

static void RTK_WDG_EN(void){
	if(rtk_wdt_device){
		cortina_wdt_start(rtk_wdt_device);
	}
	return;
}
static void RTK_WDG_DIS(void){
	if(rtk_wdt_device){
		cortina_wdt_stop(rtk_wdt_device);
	}
	return;
}

static void RTK_WDG_KICK(void) {
	if(rtk_wdt_device){
		cortina_wdt_reload(rtk_wdt_device);
	}
	return;
}


#if defined(CONFIG_SMP)
#include <linux/mutex.h>
static DEFINE_MUTEX(luna_wtd_mutex);
static DECLARE_BITMAP(wtd_count, NR_CPUS);

static void inline touch_wtd_count(void)
{
	int cpu = get_cpu();

	set_bit(cpu, wtd_count);
	put_cpu();
}

static int luna_watchdog_thread (void *data)
{
	unsigned int target = (unsigned long)data;

	while(!kthread_should_stop()) {

		//gettimeofday(&start,NULL);
		/* No need to wake up earlier */
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(kick_wdg_time * HZ);

		if(target != smp_processor_id()){
//			printk("Target=%d, Current=%d\n", target, get_cpu());

		}else{
				if(watchdog_flag){
					touch_wtd_count();
				}
		}
	}

	return 0;
}

static void luan_watchdog_thread_init(void)
{
	unsigned long cpu;

	for_each_online_cpu(cpu) {
		if (cpu == 0)
			continue;

		if (cpu >= NR_CPUS)
			break;

		pWatchdogTask[cpu] = kthread_create(luna_watchdog_thread, (void *)cpu, "luna_watchdog/%lu", cpu);
		if (WARN_ON(!pWatchdogTask[cpu])) {
			printk("Create luan_watchdog/%lu failed!", cpu);
			goto out_free;
		}
		set_user_nice(pWatchdogTask[cpu], MIN_NICE);
		kthread_bind(pWatchdogTask[cpu], cpu);
		wake_up_process(pWatchdogTask[cpu]);
	}

	return;

out_free:
	for_each_online_cpu(cpu) {
		if (cpu >= NR_CPUS)
			break;

		kthread_stop(pWatchdogTask[cpu]);
		pWatchdogTask[cpu] = NULL;
	}

	return;
}

static void CHECK_CPUS_AND_KICK(void)
{
	int cpu;
	int debug = 0;

	if (_debug_rtk_wdt_ & debug_mask)	debug = 1;

	for_each_online_cpu(cpu) {
		if ( cpu == 0 )
			continue;

		if(!test_bit(cpu, wtd_count)) {
			if(debug)	printk("luna_watchdog: luna_watchdog/%d not respond!\n", cpu);
			return;
		}
	}

	for_each_online_cpu(cpu)
		clear_bit(cpu, wtd_count);

	RTK_WDG_KICK();

	if(debug)	printk("CPU%d kick watchdog!\n", smp_processor_id());
}

#endif



void kick_rtk_watchdog(void)
{
	if (_debug_rtk_wdt_ & debug_mask){
		end = ktime_get_seconds();

		printk("tv_sec:%lld\t",end);
	}


	CHECK_CPUS_AND_KICK();

}

/*
 *   CPU0 : Kick watchdog  register
 */
int watchdog_timeout_thread(void *data)
{
	while(!kthread_should_stop()) {
		//gettimeofday(&start,NULL);
		/* No need to wake up earlier */
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(kick_wdg_time * HZ);

		if(watchdog_flag)	kick_rtk_watchdog();
	}

	return 0;
}

static void wdt_thread_maintain(int flag){
	unsigned long cpu = 0;
	if(flag) {
		mutex_lock(&luna_wtd_mutex);
		if( pWatchdogTask[cpu] == NULL){
			pWatchdogTask[cpu] = kthread_create(watchdog_timeout_thread, (void *)cpu, "luna_watchdog/%lu", cpu);

			if(IS_ERR(pWatchdogTask[cpu])) {
				printk("[Kthread : luna_watchdog] init failed %lu!\n", PTR_ERR(pWatchdogTask[cpu]));
			}
			else
			{
				kthread_bind( pWatchdogTask[cpu], cpu);		/* CPU0 */
				set_user_nice(pWatchdogTask[cpu], MIN_NICE);
				wake_up_process(pWatchdogTask[cpu]);
#ifdef CONFIG_SMP
				luan_watchdog_thread_init();			/* CPU1~CPUxx */
#endif
				printk("[Kthread : luan_watchdog ] init complete!\n");
			}
		}
		mutex_unlock(&luna_wtd_mutex);
	}else {
		mutex_lock(&luna_wtd_mutex);
		for_each_online_cpu(cpu) {
			if (cpu >= NR_CPUS)
							break;
			if( pWatchdogTask[cpu] != NULL) {
				kthread_stop(pWatchdogTask[cpu]);
				pWatchdogTask[cpu] = NULL;
			}
		}
		mutex_unlock(&luna_wtd_mutex);
	}
}

#ifdef CONFIG_HOTPLUG_CPU
#include <linux/cpu.h>
static int percpu_luna_wdt_hotcpu_callback_offline(unsigned int cpu)
{
		if(watchdog_flag ==0){
			        return 0;
		}

		{//offline
			mutex_lock(&luna_wtd_mutex);
			if( pWatchdogTask[cpu] != NULL) {
				kthread_stop(pWatchdogTask[cpu]);
				pWatchdogTask[cpu] = NULL;
			}
			mutex_unlock(&luna_wtd_mutex);
		}
        return 0;
}
static int percpu_luna_wdt_hotcpu_callback_online(unsigned int cpu)
{
		unsigned long target_cpu = cpu;
		if(watchdog_flag ==0){
			        return 0;
		}
		mutex_lock(&luna_wtd_mutex);
		if(pWatchdogTask[cpu] == NULL){
				pWatchdogTask[cpu] = kthread_create(luna_watchdog_thread, (void *)target_cpu, "luna_watchdog/%u", cpu);
				if (WARN_ON(!pWatchdogTask[cpu])) {
					printk("Create luan_watchdog/%u failed!", cpu);
				}else{
					set_user_nice(pWatchdogTask[cpu], MIN_NICE);
					kthread_bind(pWatchdogTask[cpu], cpu);
					wake_up_process(pWatchdogTask[cpu]);
				}
		}
		mutex_unlock(&luna_wtd_mutex);
        return 0;
}
static void luna_wdt_hotplug_support(void){
	int ret;
#if 0
	ret = cpuhp_setup_state_nocalls(CPUHP_OFFLINE, "watchdog/luna_watchdog:kthread_stop",
					NULL, percpu_luna_wdt_hotcpu_callback_offline);
//	WARN_ON(ret < 0);

	ret = cpuhp_setup_state_nocalls(CPUHP_ONLINE, "watchdog/luna_watchdog:startup",
					NULL, percpu_luna_wdt_hotcpu_callback_online);
//	WARN_ON(ret < 0);
#endif
	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					"CA_WDT/watchdog:online",
					percpu_luna_wdt_hotcpu_callback_online,
					percpu_luna_wdt_hotcpu_callback_offline);
	if (ret < 0) {
		pr_warn("%s: could not be initialized\n", __func__);
	}
}
#else
#define luna_wdt_hotplug_support()
#endif

static int watchdog_info_show(struct seq_file *seq, void *v)
{
	struct cortina_wdt *wdt = watchdog_get_drvdata(rtk_wdt_device);
	seq_printf(seq, "[watchdog_flag = 0x%08x]\n", watchdog_flag);
	seq_printf(seq, "0x1: enable hw watchdog timeout\n");
	seq_printf(seq, "0x3: enable hw watchdog timeout and debug message\n");
	seq_printf(seq, "PER_WDT_CTRL = 0x%08x\n", cortina_wdt_readreg(wdt, CORTINA_WDT_CTRL));
	return 0;
}

static ssize_t watchdog_write(struct file *file,  const char __user *buf, size_t size, loff_t *_pos)
{
	unsigned char tmpBuf[16] = {0};
	unsigned int val;
	int len = (size > 15) ? 15 : size;

	if (buf && !copy_from_user(tmpBuf, buf, len)) {
		val = simple_strtoul(tmpBuf, NULL, 16);
		printk("write watchdog_flag to 0x%08x\n", val);

		if((watchdog_flag==0) && (val>0)){
			wdt_thread_maintain(1);
			RTK_WDG_EN();
			RTK_WDG_KICK();
			watchdog_flag=val;
		}else if((watchdog_flag>0) &&(val==0)){
			RTK_WDG_DIS();
			wdt_thread_maintain(0);		/* stop thread */
			watchdog_flag = 0;
		}

		if(watchdog_flag & 0x02)	debug_mask = 1;
		else				debug_mask = 0;

		return size;
	}

	return -EFAULT;
}


static int watchdog_open(struct inode *inode, struct file *file)
{
	return single_open(file, watchdog_info_show, inode->i_private);
}

static const struct proc_ops bsp_luna_wdt_flag_fops = {
    .proc_open       = watchdog_open,
    .proc_read       = seq_read,
    .proc_lseek     = seq_lseek,
    .proc_release    = single_release,
    .proc_write      = watchdog_write,
};



/****  Timeout proc ****************/

/* Watch Dog Timer Control Register */
static int wdg_timeout_show(struct seq_file *seq, void *v)
{
	unsigned int reg_val;

	struct cortina_wdt *wdt = watchdog_get_drvdata(rtk_wdt_device);
	reg_val = cortina_wdt_readreg(wdt, CORTINA_WDT_CTRL);
	seq_printf(seq, "WDT_E=%d, (1-enable, 0-disable)\n", (  reg_val & 0x1));

	//CORTINA_WDT_LD
	seq_printf(seq, "WDT Timeout Value = %u(s)\n", cortina_wdt_readreg(wdt, CORTINA_WDT_LD));

	return 0;
}

static ssize_t wdg_timeout_write(struct file *file,  const char __user *buf, size_t size, loff_t *_pos)
{
	unsigned char tmpBuf[16] = {0};
	unsigned short new_time,count;
	struct cortina_wdt *wdt;
	int len = (size > 15) ? 15 : size;

	if (buf && !copy_from_user(tmpBuf, buf, len)) {
		wdt = watchdog_get_drvdata(rtk_wdt_device);
		new_time = simple_strtoul(tmpBuf, NULL, 10);
		printk("write wdg_timeout to value(%d)\n", new_time);
		rtk_wdt_device->timeout = new_time;
		count = high_resolution ? rtk_wdt_device->timeout * 1000 : rtk_wdt_device->timeout;
		cortina_wdt_writereg(wdt, CORTINA_WDT_LD, count);
		return size;
	}

	return -EFAULT;
}

static int timeout_open(struct inode *inode, struct file *file)
{
	return single_open(file, wdg_timeout_show, inode->i_private);
}

static const struct proc_ops bsp_luna_wdt_timeout_fops = {
    .proc_open       = timeout_open,
    .proc_read       = seq_read,
    .proc_lseek     = seq_lseek,
    .proc_release    = single_release,
    .proc_write      = wdg_timeout_write,
};


/*****************************************************************/

static void luna_watchdog_thread_init(struct watchdog_device *wdd){
	struct proc_dir_entry *watchdog_proc_dir = NULL;

	rtk_wdt_device = wdd;
	wdt_thread_maintain(1);

	RTK_WDG_EN();
	watchdog_flag = 1;
	luna_wdt_hotplug_support();

	watchdog_proc_dir = proc_mkdir("luna_watchdog", NULL);
	if (watchdog_proc_dir == NULL) {
		printk("create /proc/luna_watchdog failed!\n");
		return;
	}

	if(! proc_create("watchdog_flag", 0444, watchdog_proc_dir, &bsp_luna_wdt_flag_fops)) {
		printk("create proc /proc/luna_watchdog/watchdog_flag failed!\n");
		return;
	}


	if(! proc_create("wdg_timeout", 0444, watchdog_proc_dir, &bsp_luna_wdt_timeout_fops)) {
		printk("create proc  /proc/luna_watchdog/watchdog_flag failed!\n");
		return;
	}


}


#endif
