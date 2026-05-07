#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pstore_ram.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <asm-generic/cacheflush.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/kmsg_dump.h>


#define PSTORE_XRAM_ADDR /*0xef00000*/0xf4500008 //XRAM 0xf4500000 + 8
#define PSTORE_XRAM_SIZE 0x2000-8 //8K-8 byte

static dma_addr_t buffer;
static unsigned long vbuf;
static void *xram_vaddr;

static u64 rtk_pstore_dmamask = DMA_BIT_MASK(32);

static struct ramoops_platform_data rtk_ramoops_data = {
        .mem_size       = PSTORE_XRAM_SIZE,
        .record_size    = PSTORE_XRAM_SIZE,
        //.console_size   = 128,
        //.ftrace_size    = 128,
        //.pmsg_size		= 128,
        .mem_type		= 1,
};

static struct platform_device rtk_ramoops = {
        .name = "ramoops",
        .dev = {
                .platform_data = &rtk_ramoops_data,
        },
};

static void rtk_pstore_dump(struct kmsg_dumper *dumper,
                        enum kmsg_dump_reason reason)
{
	//transfer dram => xram
	memcpy(xram_vaddr,(void*)vbuf,PSTORE_XRAM_SIZE);
	flush_cache_all();
}

static struct kmsg_dumper rtk_pstore_dumper = {
        .dump = rtk_pstore_dump,
};

static void rtk_pstore_register_kmsg(void)
{
        kmsg_dump_register(&rtk_pstore_dumper);
}

static struct platform_device rtk_pstore_dev = {
	.name           = "rtk_pstore",
	.id             = 0,
	.dev = {
		.dma_mask               = &rtk_pstore_dmamask,
		.coherent_dma_mask      = DMA_BIT_MASK(32),
	},
	.num_resources  = 0,
};



static __init int rtk_pstore_init(void)
{
	int ret;

	ret=platform_device_register(&rtk_pstore_dev);
	if (ret) {
		printk(KERN_ERR "unable to register platform device\n");
		return ret;
	}
	of_dma_configure(&rtk_pstore_dev.dev, NULL,0);

	vbuf=(unsigned long)dma_alloc_coherent(&rtk_pstore_dev.dev, PSTORE_XRAM_SIZE,&buffer, GFP_KERNEL);

	xram_vaddr = ioremap(PSTORE_XRAM_ADDR, PSTORE_XRAM_SIZE);
	if(xram_vaddr){
		//transfer xram=>dram
		memcpy((void*)vbuf,xram_vaddr,PSTORE_XRAM_SIZE);
		flush_cache_all();
		memset(xram_vaddr,0xff,PSTORE_XRAM_SIZE); //After swap data to buffer, clean XRAM data
	}

	rtk_ramoops_data.mem_address = buffer;
	rtk_ramoops_data.max_reason = KMSG_DUMP_OOPS;
	printk("\n### rtk_ramoops_data.mem_address=%llx ###\n",rtk_ramoops_data.mem_address);

	ret = platform_device_register(&rtk_ramoops);
	if (ret) {
		printk(KERN_ERR "unable to register platform device\n");
		return ret;
	}

	rtk_pstore_register_kmsg();

	return 0;
}

static void rtk_pstore_exit(void)
{
		platform_device_unregister(&rtk_ramoops);
}

arch_initcall(rtk_pstore_init);
module_exit(rtk_pstore_exit);


