#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/reset.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#include <linux/of.h>
#endif
#include <ca_pe_ctrl_api.h>

struct s_pe_ctrl {
	struct reset_control	*pe0_reset;
	struct reset_control	*pe1_reset;
	struct device		*dev;
};

struct s_pe_ctrl *g_pe_ctrl;

static long pe_ctrl_proc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int reset_status;

	switch(cmd){
    	case PE_CTRL_IOCTL_RESET_PE0 :
			//reset_status = reset_control_status(g_pe_ctrl->pe0_reset);

			//if(reset_status == 0)
				reset_control_assert(g_pe_ctrl->pe0_reset);

			break;
		case PE_CTRL_IOCTL_ENABLE_PE0 :
			//reset_status = reset_control_status(g_pe_ctrl->pe0_reset);

			//if(reset_status != 0)
				reset_control_deassert(g_pe_ctrl->pe0_reset);

			break;
		case PE_CTRL_IOCTL_RESET_PE1 :
			//reset_status = reset_control_status(g_pe_ctrl->pe1_reset);

			//if(reset_status == 0)
				reset_control_assert(g_pe_ctrl->pe1_reset);

			break;
		case PE_CTRL_IOCTL_ENABLE_PE1 :
			//reset_status = reset_control_status(g_pe_ctrl->pe1_reset);

			//if(reset_status != 0)
				reset_control_deassert(g_pe_ctrl->pe1_reset);

			break;

	}
	return 0;
}

static int pe_ctrl_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
static struct proc_ops proc_pe_ctrl_operations = {
    .proc_open          = pe_ctrl_proc_open,
    .proc_ioctl         = pe_ctrl_proc_ioctl,
};
#else
const struct file_operations proc_pe_ctrl_operations = {
	.open       		= pe_ctrl_proc_open,
    .unlocked_ioctl  	= (void*)pe_ctrl_proc_ioctl,
};
#endif

static int __init init_procfs_msg(void)
{
	proc_create("pe_ctrl", S_IRUSR | S_IWUSR, NULL, &proc_pe_ctrl_operations);

	return 0;
}

static int pe_ctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret =0;

	printk("%s \n", __func__);

	g_pe_ctrl = devm_kzalloc(dev, sizeof(*g_pe_ctrl), GFP_KERNEL);
	if (!g_pe_ctrl)
		return -ENOMEM;
	g_pe_ctrl->dev = dev;

#ifdef CONFIG_RESET_TAURUS_PE
	/* get PE reset controller for cpu0 */
	g_pe_ctrl->pe0_reset = of_reset_control_get(np, "pe_reset_cpu0");
#else	// not CONFIG_RESET_TAURUS_PE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	g_pe_ctrl->pe0_reset = of_reset_control_get_shared(np, "pe0_reset");
#else
	g_pe_ctrl->pe0_reset = of_reset_control_get(np, "pe0_reset");
#endif
#endif	//end CONFIG_RESET_TAURUS_PE
	if (IS_ERR(g_pe_ctrl->pe0_reset)) {
		ret = PTR_ERR(g_pe_ctrl->pe0_reset);
		goto exit;
	}

#ifdef CONFIG_RESET_TAURUS_PE
	/* get PE reset controller for cpu1 */
	g_pe_ctrl->pe1_reset = of_reset_control_get(np, "pe_reset_cpu1");
#else	// not CONFIG_RESET_TAURUS_PE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	g_pe_ctrl->pe1_reset = of_reset_control_get_shared(np, "pe1_reset");
#else
	g_pe_ctrl->pe1_reset = of_reset_control_get(np, "pe1_reset");
#endif
#endif	//end CONFIG_RESET_TAURUS_PE
	if (IS_ERR(g_pe_ctrl->pe1_reset)) {
		ret = PTR_ERR(g_pe_ctrl->pe1_reset);
		goto exit;
	}

	init_procfs_msg();

exit:
	
	return ret;
}




#define MAJOR_NUM          99
#define MODULE_NAME                "PE_CTRL"




static void __exit pe_ctrl_exit(void)
{
	return;
}

#ifdef CONFIG_OF
static const struct of_device_id pe_ctrl_of_match[] = {
#ifdef CONFIG_RESET_TAURUS_PE
	{.compatible = "rtk,pe_taurus_pe_reset"},
#else
	{.compatible = "cortina,pe-ctrl"},
#endif	
	{},
};
MODULE_DEVICE_TABLE(of, pe_ctrl_of_match);
#endif

static struct platform_driver pe_ctrl_driver = {
	.probe = pe_ctrl_probe,
	.remove		= __exit_p(pe_ctrl_exit),
	.driver = {
		.name	= "cortina-pectrl",
		.of_match_table = pe_ctrl_of_match,
	},
};

static int pe_ctrl_init(void)
{

	return platform_driver_register(&pe_ctrl_driver);

	return 0;
}

module_init(pe_ctrl_init);
module_exit(pe_ctrl_exit);