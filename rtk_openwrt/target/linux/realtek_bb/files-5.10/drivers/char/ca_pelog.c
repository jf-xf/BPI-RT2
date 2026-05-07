#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/version.h>

#include <linux/module.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include <ca_types.h>

#define MAGIC_NUM		 0x7533967
#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
#define PELOG_CPU_NUM 2
#elif defined(CONFIG_ARCH_REALTEK_9607F)
#define PELOG_CPU_NUM 3
#else
#errro "no support"
#endif

typedef struct {
	uint32_t magicNum;
	uint32_t logOverwrite;
	uint32_t write_offset;
	char data[];
}__attribute__ ((__packed__)) pelog_header_t;


typedef struct pelog_module {
	void __iomem* pelog_shm_vaddr;
	resource_size_t pelog_shm_paddr;
	resource_size_t shm_size;
	uint32_t u32ReadOffset;
	char*		rebuild_msgBuf;
}pelog_module_t;


pelog_module_t pelog_module0,pelog_module1;

MODULE_LICENSE("GPL");
#ifdef CONFIG_OF
/* Match table for of_platform binding */
static const struct of_device_id cortina_pelog_of_match[] = {
        { .compatible = "cortina,pelog", },
        {},
};
MODULE_DEVICE_TABLE(of, cortina_pelog_of_match);
#endif

int isPelogInitSucess0 = 0,isPelogInitSucess1 = 0;

static int ca_pelog_probe(struct platform_device *pdev)
{
	pelog_header_t *pPelog;
	struct device_node *np;
	struct resource mem_resource;
	const struct of_device_id *match;
	int ret=-ENOMEM;

	printk("%s \n", __func__);
	///* assign DT node pointer */
	//np = pdev->dev.of_node;

		/* search DT for a match */
	match = of_match_device(cortina_pelog_of_match, &pdev->dev);
	if (!match){
		return -EINVAL;
	}

	np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!np) {
  		printk("No %s specified\n", "memory-region");
  		return ret;
	}

	/* get "pelog0 shm" from DT and convert to platform mem address resource */
	ret = of_address_to_resource(np, 0, &mem_resource);
	if (ret) {
		printk("%s:pelog of_address_to_resource return %d\n", __func__, ret);
		return ret;
	}

	pelog_module0.shm_size = resource_size(&mem_resource)/PELOG_CPU_NUM;
	pelog_module0.pelog_shm_vaddr =
		devm_ioremap(&pdev->dev, mem_resource.start, pelog_module0.shm_size);

	if (!pelog_module0.pelog_shm_vaddr) {
		printk("pelog0: devm_ioremap fail\n");
		return -ENOMEM;
	}

	pelog_module0.pelog_shm_paddr = mem_resource.start;
	pelog_module0.u32ReadOffset = 0;

	printk("pelog0: shm physical address : 0x%lx\n", (unsigned long)pelog_module0.pelog_shm_paddr);
	printk("pelog0: shm virtual address : 0x%lx\n", (unsigned long)pelog_module0.pelog_shm_vaddr);
	printk("pelog0: shm size: 0x%lx\n", (unsigned long) pelog_module0.shm_size);
	printk("pelog0: reset magicNum\n");

	pPelog = (pelog_header_t*) pelog_module0.pelog_shm_vaddr;
	pPelog->magicNum = 0;

	pelog_module0.rebuild_msgBuf = kmalloc(pelog_module0.shm_size, GFP_KERNEL);

	if (!pelog_module0.rebuild_msgBuf) {
		printk("pelog0: kmalloc fail\n");
		return -ENOMEM;
	}

	isPelogInitSucess0 = 1;

		/* get "pelog1 shm" from DT and convert to platform mem address resource */
	//ret = of_address_to_resource(np, 1, &mem_resource);
	//if (ret) {
	//	printk("%s:pelog1 of_address_to_resource return %d\n", __func__, ret);
	//	return ret;
	//}

	pelog_module1.shm_size = resource_size(&mem_resource)/PELOG_CPU_NUM;
	pelog_module1.pelog_shm_vaddr =
		devm_ioremap(&pdev->dev, (mem_resource.start+pelog_module0.shm_size), pelog_module1.shm_size);

	if (!pelog_module1.pelog_shm_vaddr) {
		printk("pelog1: devm_ioremap fail\n");
		return -ENOMEM;
	}

	pelog_module1.pelog_shm_paddr = mem_resource.start+pelog_module0.shm_size;
	pelog_module1.u32ReadOffset = 0;

	printk("pelog1: shm physical address : 0x%lx\n", (unsigned long)pelog_module1.pelog_shm_paddr);
	printk("pelog1: shm virtual address : 0x%lx\n", (unsigned long)pelog_module1.pelog_shm_vaddr);
	printk("pelog1: shm size: 0x%lx\n", (unsigned long) pelog_module1.shm_size);
	printk("pelog1: reset magicNum\n");

	pPelog = (pelog_header_t*) pelog_module1.pelog_shm_vaddr;
	pPelog->magicNum = 0;

	pelog_module1.rebuild_msgBuf = kmalloc(pelog_module1.shm_size, GFP_KERNEL);

	if (!pelog_module1.rebuild_msgBuf) {
		printk("pelog1: kmalloc fail\n");
		return -ENOMEM;
	}

	isPelogInitSucess1 = 1;

	return 0;
}

static int ca_pelog_remove(struct platform_device *op)
{
	return 0;
}


static struct platform_driver ca_pelog_driver = {
	.probe = ca_pelog_probe,
	.remove = ca_pelog_remove,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "ca_pelog",
		   .of_match_table = of_match_ptr(cortina_pelog_of_match),
		   },
};


static int __init ca_pelog_init(void)
{
	//return init_procfs_pelog();
	return platform_driver_register(&ca_pelog_driver);
}

static void __exit ca_pelog_exit(void)
{
	platform_driver_unregister(&ca_pelog_driver);
}

module_init(ca_pelog_init);
module_exit(ca_pelog_exit);


static int pelog0_proc_show(struct seq_file *m, void *v)
{
	pelog_header_t* pelog =(pelog_header_t*) pelog_module0.pelog_shm_vaddr;
	uint64_t header_size = (uint64_t)(pelog->data) - (uint64_t)pelog;
	uint64_t msgBuf_size = (uint64_t)pelog_module0.shm_size - (uint64_t)header_size;

	uint32_t logOffset = pelog->write_offset;
	uint32_t logOverwrite = pelog->logOverwrite;

	//logOffset = logOffset - logOffset%4; //4 bytes alignment

	if(isPelogInitSucess0 == 0)
	{
		printk("Pelog0 init fail\n");
		return 0;
	}

	if(pelog->magicNum != MAGIC_NUM)
	{
		printk("PEDSP0's Pelog not enable\n");
		return 0;		
	}

	if(logOffset == pelog_module0.u32ReadOffset && logOverwrite==0)
	{
		printk("PEDSP0's Pelog no log\n");
		return 0;
	}

	memset(pelog_module0.rebuild_msgBuf,0,pelog_module0.shm_size);

	if(logOverwrite == 0)
	{
		//printk("%s %d\n",__FUNCTION__,__LINE__);
		if(pelog_module0.u32ReadOffset > logOffset) //DSP reboot case
		{
			printk("PEDSP0 Pelog log reset\n");
			memcpy_fromio(pelog_module0.rebuild_msgBuf,(pelog->data),logOffset);
			pelog_module0.u32ReadOffset = logOffset;
		}
		else
		{
			memcpy_fromio(pelog_module0.rebuild_msgBuf,(pelog->data) + pelog_module0.u32ReadOffset,logOffset - pelog_module0.u32ReadOffset);
			pelog_module0.u32ReadOffset = logOffset;
		}
	}else if(logOverwrite == 1)
	{
		if(pelog_module0.u32ReadOffset > logOffset)
		{
			memcpy_fromio(pelog_module0.rebuild_msgBuf, (pelog->data) + pelog_module0.u32ReadOffset, (msgBuf_size - pelog_module0.u32ReadOffset));
			memcpy_fromio((pelog_module0.rebuild_msgBuf)+(msgBuf_size-pelog_module0.u32ReadOffset),(pelog->data),logOffset);
			pelog->logOverwrite = 0;
			pelog_module0.u32ReadOffset = logOffset;
		}else
		{
			printk("PEDSP0 Pelog log overwrite\n");
			memcpy_fromio(pelog_module0.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
			memcpy_fromio((pelog_module0.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
			pelog->logOverwrite = 0;
			pelog_module0.u32ReadOffset = logOffset;
		}

		//memcpy(pelog_module.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
		//memcpy((pelog_module.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
		//printk("%s %d\n",__FUNCTION__,__LINE__);
	}else
	{
			printk("PEDSP0 Pelog log overwrite more then once\n");
			memcpy_fromio(pelog_module0.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
			memcpy_fromio((pelog_module0.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
			pelog->logOverwrite = 0;
			pelog_module0.u32ReadOffset = logOffset;
	}

	seq_printf(m,(char*)pelog_module0.rebuild_msgBuf);

	return 0;
}

static int pelog0_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pelog0_proc_show, NULL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
static const struct proc_ops pelog0_proc_fops = {
	.proc_open		= pelog0_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};
#else
static const struct file_operations pelog0_proc_fops = {
	.open		= pelog0_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int pelog1_proc_show(struct seq_file *m, void *v)
{
	pelog_header_t* pelog =(pelog_header_t*) pelog_module1.pelog_shm_vaddr;
	uint64_t header_size = (uint64_t)(pelog->data) - (uint64_t)pelog;
	uint64_t msgBuf_size = (uint64_t)pelog_module1.shm_size - (uint64_t)header_size;

	uint32_t logOffset = pelog->write_offset;
	uint32_t logOverwrite = pelog->logOverwrite;

	if(isPelogInitSucess1 == 0)
	{
		printk("Pelog1 init fail\n");
		return 0;
	}

	if(pelog->magicNum != MAGIC_NUM)
	{
		printk("PEDSP1's Pelog not enable\n");
		return 0;		
	}

	if(logOffset == pelog_module1.u32ReadOffset && logOverwrite==0)
	{
		printk("PEDSP1's Pelog no log\n");
		return 0;
	}

	memset(pelog_module1.rebuild_msgBuf,0,pelog_module1.shm_size);

	if(logOverwrite == 0)
	{
		//printk("%s %d\n",__FUNCTION__,__LINE__);
		if(pelog_module1.u32ReadOffset > logOffset) //DSP reboot case
		{
			printk("PEDSP1 Pelog log reset\n");
			memcpy_fromio(pelog_module1.rebuild_msgBuf,(pelog->data),logOffset);
			pelog_module1.u32ReadOffset = logOffset;
		}
		else
		{
			memcpy_fromio(pelog_module1.rebuild_msgBuf,(pelog->data) + pelog_module1.u32ReadOffset,logOffset - pelog_module1.u32ReadOffset);
			pelog_module1.u32ReadOffset = logOffset;
		}
	}else if(logOverwrite == 1)
	{
		if(pelog_module1.u32ReadOffset > logOffset)
		{
			memcpy_fromio(pelog_module1.rebuild_msgBuf, (pelog->data) + pelog_module1.u32ReadOffset, (msgBuf_size - pelog_module1.u32ReadOffset));
			memcpy_fromio((pelog_module1.rebuild_msgBuf)+(msgBuf_size-pelog_module1.u32ReadOffset),(pelog->data),logOffset);
			pelog->logOverwrite = 0;
			pelog_module1.u32ReadOffset = logOffset;
		}else
		{
			printk("PEDSP1 Pelog log overwrite\n");
			memcpy_fromio(pelog_module1.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
			memcpy_fromio((pelog_module1.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
			pelog->logOverwrite = 0;
			pelog_module1.u32ReadOffset = logOffset;
		}

		//memcpy(pelog_module.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
		//memcpy((pelog_module.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
		//printk("%s %d\n",__FUNCTION__,__LINE__);
	}else
	{
			printk("PEDSP1 Pelog log overwrite more then once\n");
			memcpy_fromio(pelog_module1.rebuild_msgBuf,(pelog->data)+logOffset,(msgBuf_size-logOffset));
			memcpy_fromio((pelog_module1.rebuild_msgBuf)+(msgBuf_size-logOffset),(pelog->data),logOffset);
			pelog->logOverwrite = 0;
			pelog_module1.u32ReadOffset = logOffset;
	}

	seq_printf(m,(char*)pelog_module1.rebuild_msgBuf);

	return 0;
}

static int pelog1_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pelog1_proc_show, NULL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
static const struct proc_ops pelog1_proc_fops = {
	.proc_open		= pelog1_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};
#else
static const struct file_operations pelog1_proc_fops = {
	.open		= pelog1_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int __init proc_pelog_init(void)
{
	proc_create("pelog0", 0, NULL, &pelog0_proc_fops);
	proc_create("pelog1", 0, NULL, &pelog1_proc_fops);
	return 0;
}
fs_initcall(proc_pelog_init);
