#include <linux/version.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/arm-smccc.h>
#include <soc/cortina/cortina-smc.h>
#include <linux/slab.h>
#include <linux/dma-direction.h>
#include <asm/cacheflush.h>
#include <soc/cortina/cortina-soc.h>
#include <soc/cortina/registers.h>
#ifdef CONFIG_ARM_DSU_PMU
#include <linux/arm-smccc.h>
#include <soc/cortina/cortina-smc.h>

#define CA_SMC_CALL_SREG_ACTLR_READ(elx)			rtk_smc_call(CA_SVC_SREG_ACTLR_READ, elx, 0, 0, 0)
#define CA_SMC_CALL_SREG_ACTLR_WRITE(elx, val) 		rtk_smc_call(CA_SVC_SREG_ACTLR_SET, BIT(elx), val, 0, 0)

#define CLUSTERPMUEN_BIT BIT(12)
#define EL2 0x2
#define EL3 0x3
#define ACTLR_CLUSTERPMUEN_PASS 1
#define ACTLR_CLUSTERPMUEN_FAIL 0


u64 rtk_smc_call(unsigned int smc_id, unsigned long x1,unsigned long x2, \
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


static void rtk_actlr_set(void *info){
	unsigned int *result = (unsigned int *)info;
	unsigned long val;

	val = CA_SMC_CALL_SREG_ACTLR_READ(EL3) | CLUSTERPMUEN_BIT;
	CA_SMC_CALL_SREG_ACTLR_WRITE(EL3, val);
	val = CA_SMC_CALL_SREG_ACTLR_READ(EL2) | CLUSTERPMUEN_BIT;
	CA_SMC_CALL_SREG_ACTLR_WRITE(EL2, val);


	if((CA_SMC_CALL_SREG_ACTLR_READ(EL3) & CLUSTERPMUEN_BIT )
		&& (CA_SMC_CALL_SREG_ACTLR_READ(EL2) & CLUSTERPMUEN_BIT )){
		*result =1;
		return;
	}
	*result = 0;
}

static void rtk_actlr_confirm_smp(void){
	int cpu;
	unsigned int result;
	for_each_online_cpu(cpu) {
		smp_call_function_single(cpu, rtk_actlr_set, (void *)&result, 1);
		if(result ==0){
			printk("DSU_PMU: ACTLR_EL2/3 - Can't set CLUSTERPMUEN bit.\n");
			return;
		}
	}
}
#endif

/*****************************************************************************
 *  SMC CALL with Preserved Buff
 *****************************************************************************/
#include <dt-bindings/soc/rtk_trustzone.h>
#ifdef CONFIG_OPTEE
#define   TZ_SMC_BUF_ADDR	TZ_SMC_BUF_PADDR_WITH_TEE
#else
#define   TZ_SMC_BUF_ADDR	TZ_SMC_BUF_PADDR_WITHOUT_TEE
#endif

struct rtk_soc_data{
	void __iomem *map_smc_buf;
	unsigned long smc_buf_phy;
	size_t smc_buf_len;
};

static struct rtk_soc_data rtk_soc = { NULL, TZ_SMC_BUF_ADDR, TZ_SMC_BUF_SIZE };

static DEFINE_SPINLOCK(smc_buff_lock);
static int rtk_smc_buff_call(int smc_id, char *buf, size_t len){
	struct arm_smccc_res res;
	int ret=0;
	if(IS_ELNATH){
		rtk_soc.map_smc_buf = kmalloc(TZ_SMC_BUF_SIZE, GFP_ATOMIC);
		if(rtk_soc.map_smc_buf){
			 rtk_soc.smc_buf_phy = virt_to_phys(rtk_soc.map_smc_buf);
		}else{
			return -ENOMEM;
		}
	}

	if( rtk_soc.map_smc_buf == NULL ){
		ret = -ENOMEM;
		goto SMC_BUFF_EXIT;
	}

	if(len >  TZ_SMC_BUF_SIZE){
		printk("rtk_smc_buff_call failed! Output Buff over ReservedMem_Size\n");
		ret = -EINVAL;
		goto SMC_BUFF_EXIT;
	}
	spin_lock(&smc_buff_lock);
#ifdef CONFIG_ARM64
	__dma_unmap_area(rtk_soc.map_smc_buf, rtk_soc.smc_buf_len ,DMA_FROM_DEVICE);
#endif
#ifdef CONFIG_ARM
	__cpuc_flush_dcache_area(rtk_soc.map_smc_buf, rtk_soc.smc_buf_len);
#endif
	arm_smccc_smc(smc_id, 0, 0, rtk_soc.smc_buf_phy, 0, 0, 0, 0, &res);

	memcpy(buf, rtk_soc.map_smc_buf, len);

	spin_unlock(&smc_buff_lock);

SMC_BUFF_EXIT:
	if(IS_ELNATH){
		if(rtk_soc.map_smc_buf){
			kfree(rtk_soc.map_smc_buf);
			rtk_soc.map_smc_buf = NULL;
			rtk_soc.smc_buf_phy = 0;
		}
	}
	return ret;
}
int rtk_smc_gphy_patch_read(char *buff, size_t len){
	if(len<1024){
		printk("rtk_smc_gphy_patch failed! Expected Buff_SIZE is over 1024, but input 0x%zx\n", len);
		return  -EINVAL;
	}
	return rtk_smc_buff_call(CA_SVC_OTP_GPHY_PATCH_READ, buff, len);
}
EXPORT_SYMBOL(rtk_smc_gphy_patch_read);

//#define GHPY_SMC_PATCH_TEST
#ifdef GHPY_SMC_PATCH_TEST
#define test_buff_size 1024

static void test_smc_gphy(void){
	int i;
	char *buff = kzalloc(test_buff_size, GFP_KERNEL);
	if(buff){
		rtk_smc_gphy_patch_read(buff, test_buff_size); 
		{
			for(i=0;i<(test_buff_size);i++){
				printk("0x%x ", buff[i]);
			}
		}
		kfree(buff);
	}
	printk("\n");
	return;
}
#endif

static __init int soc_smc_buff_init(void){
	struct resource *res_smc;
	if(IS_ELNATH){
		rtk_soc.map_smc_buf = NULL;
	}else{
		res_smc = request_mem_region(TZ_SMC_BUF_ADDR, TZ_SMC_BUF_SIZE, "smc_buff");
		if (!res_smc) {
			printk("[%s]mmio already reserved for resource smc_buf\n", __func__);
			return -EBUSY;
		}
		rtk_soc.map_smc_buf = ioremap_cache(TZ_SMC_BUF_ADDR, TZ_SMC_BUF_SIZE);

		if (!rtk_soc.map_smc_buf) {
			printk("[%s]cannot ioremap resource smc_buf\n", __func__);
			release_resource(res_smc);
			kfree(res_smc);
			return -ENODEV;
		}
		printk("rtk_soc.smc_buf_phy=0x%lx, rtk_soc.smc_buf_len=0x%zx, mapped at 0x%px\n", rtk_soc.smc_buf_phy, rtk_soc.smc_buf_len, rtk_soc.map_smc_buf );
	}
	return 0;
}

/*********************************************************************************************/
uint32_t RTK_SOC_CHIP_ID = 0;
EXPORT_SYMBOL(RTK_SOC_CHIP_ID);
static void __rtk_jtag_id_get(void){
	void __iomem *jtag_id_base;
	u32 val = 0;
	jtag_id_base = ioremap(GLOBAL_JTAG_ID, 4);
	if(jtag_id_base){
		val = readl(jtag_id_base);
		if((CA_SOC_CHIP_ID(val) == CHIP_CA8277B) || (CA_SOC_CHIP_ID(val) == CHIP_CA8289)){
			RTK_SOC_CHIP_ID = CA_SOC_CHIP_ID(val);
		}else{
		/* TODO SMC CALL */
			RTK_SOC_CHIP_ID = RTK_SOC_CHIP_ID_GET(val);
		}
		printk("SOC: RTK_SOC_CHIP_ID = 0x%x\n", RTK_SOC_CHIP_ID);
		iounmap(jtag_id_base);
	}else{
		printk("SOC: Get RTK_SOC_CHIP_ID failed.\n");
	}
}

/*********************************************************************************************/
static u8 initialized = 0;
static u8 plat_soc_chipid[2] = { 0x0, 0x0};

int plat_soc_get_chipmode(u8 *buf, int buflen) {
	int len;
	if (unlikely(!initialized))
		return -EPERM;
	len = (buflen < sizeof(plat_soc_chipid)) ? buflen : sizeof(plat_soc_chipid);
	memcpy(buf, plat_soc_chipid, len);
	return 0;
}

EXPORT_SYMBOL(plat_soc_get_chipmode);


static __init void plat_soc_init_chipmode(void){
	rtk_smc_buff_call(CA_SVC_OTP_CHIP_MODE_READ, (char *)plat_soc_chipid, 2);
	initialized = 1;
}

static __init int soc_arch_init(void) {
	__rtk_jtag_id_get();
	soc_smc_buff_init();
#ifdef GHPY_SMC_PATCH_TEST
	test_smc_gphy();
#endif
	plat_soc_init_chipmode();
#ifdef CONFIG_ARM_DSU_PMU
	rtk_actlr_confirm_smp();
#endif
	return 0;
}

arch_initcall(soc_arch_init);

#ifdef CONFIG_RTK_SOC_BC_COMPATIBLE
/****************************************/
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/io.h>
#include <asm/uaccess.h>

extern struct proc_dir_entry *realtek_proc;

static int rtk_chip_id_proc_read(struct seq_file *sf, void *v)
{
	if(RTK_SOC_CHIP_ID == CHIP_CA8277C){
		seq_printf(sf, "%s\n", "8277C");
	}else
	if(RTK_SOC_CHIP_ID == CHIP_CA8277B){
		seq_printf(sf, "%s\n", "8277B");
	}else
	if(RTK_SOC_CHIP_ID == CHIP_CA8289){
		seq_printf(sf, "%s\n", "8289");
	}else{
		seq_printf(sf, "%s\n", "unknown");
	}
    return 0;
}

static int rtk_chip_id_single_open(struct inode *inode,
                                       struct file *file)
{
    return (single_open(file, rtk_chip_id_proc_read, NULL));
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
static const struct file_operations fops_chip_id = {
    .open = rtk_chip_id_single_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};
#else
static struct proc_ops fops_chip_id = {
        .proc_open           = rtk_chip_id_single_open,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#endif

static int __init rtk_chip_id_proc_init(void)
{
    struct proc_dir_entry *pe;

    pe = proc_create_data("rtk_chip_id_show",
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
                          realtek_proc, &fops_chip_id, NULL);
    if (!pe) {
        return -EINVAL;
    }


    return 0;
}
module_init(rtk_chip_id_proc_init);
#endif
