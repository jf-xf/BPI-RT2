#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/cpuhotplug.h>
#include <soc/cortina/cortina-soc.h>

#define TAURUS_GLOBAL_ARM_CPU0_STATE 	0xf432011c
#define TAURUS_GLOBAL_ARM_CONFIG_B		0xf4320140
#define TAURUS_GLOBAL_ARM_RESET			0xf43200a8

#define VENUS_GLOBAL_ARM_CPU0_STATE 	0xf432009c
#define VENUS_GLOBAL_ARM_CONFIG_B		0xf43200c0
#define VENUS_GLOBAL_ARM_RESET			0xf4320034

#define CPU_STATE_ON		0x0100
#define CPU_STATE_OFF		0x0000
#define CPU_STATE_MASK		GENMASK(17,0)


#define COREPACCEPT_BIT		BIT(20)

#ifdef DEBUG
#ifdef writel
#undef writel
#define writel(v,c)		({ __iowmb(); writel_relaxed((v),(c)); printk("writel(0x%08x, %px)\n", v, c);})
#endif
#endif

static void __iomem *global_arm_cpu0_state_vaddr 	= NULL;
static void __iomem *global_arm_config_b_vaddr 		= NULL;
static void __iomem *global_arm_reset_vaddr			= NULL;
static int __init percpu_cpu_reg_mapping(void){
	u32 global_arm_cpu0_state_reg, global_arm_config_b_reg, global_arm_reset_reg;

	if(IS_TAURUS){
		global_arm_cpu0_state_reg 	= TAURUS_GLOBAL_ARM_CPU0_STATE;
		global_arm_config_b_reg		= TAURUS_GLOBAL_ARM_CONFIG_B;
		global_arm_reset_reg		= TAURUS_GLOBAL_ARM_RESET;
	}else{
		global_arm_cpu0_state_reg 	= VENUS_GLOBAL_ARM_CPU0_STATE;
		global_arm_config_b_reg		= VENUS_GLOBAL_ARM_CONFIG_B;
		global_arm_reset_reg		= VENUS_GLOBAL_ARM_RESET;
	}
    global_arm_cpu0_state_vaddr = ioremap(global_arm_cpu0_state_reg, 4*4);
	if(global_arm_cpu0_state_vaddr == NULL){
		goto error;
	}
    global_arm_config_b_vaddr = ioremap(global_arm_config_b_reg, 2*4);
	if(global_arm_config_b_vaddr == NULL){
		goto error;
	}
    global_arm_reset_vaddr = ioremap(global_arm_reset_reg, 4);
	if(global_arm_reset_vaddr == NULL){
		goto error;
	}

	return 0;
error:
	if(global_arm_cpu0_state_vaddr){
		kfree(global_arm_cpu0_state_vaddr);
		global_arm_cpu0_state_vaddr = NULL;
	}
	if(global_arm_config_b_vaddr){
		kfree(global_arm_config_b_vaddr);
		global_arm_config_b_vaddr = NULL;
	}

	return -1;
}

static DEFINE_SPINLOCK(rtk_pwr_ctrl_lock);
static int percpu_cpu_offline(unsigned int cpu)
{
	u32 timeout  = 1000;
	u32 reg_v;
	spin_lock(&rtk_pwr_ctrl_lock);
	do{
		reg_v = readl(global_arm_cpu0_state_vaddr + cpu*4);
		timeout--;
	}while(((reg_v & CPU_STATE_MASK) != CPU_STATE_OFF) && timeout >0);

	/* PSTATE write 0x0 */
	reg_v = readl(global_arm_config_b_vaddr + 4);
	reg_v = reg_v & ~(0xff << cpu*8);
	writel(reg_v, global_arm_config_b_vaddr + 4);

	/*assert PREQ */
	reg_v |= (0x80 << cpu*8);
	writel(reg_v, global_arm_config_b_vaddr + 4);

	/* paccept go high */
	timeout = 1000;
	do{
		reg_v = readl(global_arm_cpu0_state_vaddr + cpu*4);
		timeout--;
	}while(((reg_v & COREPACCEPT_BIT) == 0) && timeout >0);

	/* reset cpu state to on and PREQ->0 , global_arm_config_c*/
	reg_v = readl(global_arm_config_b_vaddr + 4);
	reg_v = reg_v & ~(0xff << cpu*8);
	reg_v	|= 0x08 << (cpu*8);

	writel(reg_v, global_arm_config_b_vaddr + 4);

	/* power controller comunication with cpu , Doen
	 *cpu should flush and disable  local cacahe and remove coherency domain
	 */
	wmb();
	 /* Now power off cpu */

	/* 1. clamp */
	reg_v = readl(global_arm_config_b_vaddr);
	reg_v |= 1<<(cpu+ 16);
	writel(reg_v, global_arm_config_b_vaddr);

	/* 2. cpu on reset */
	reg_v = readl(global_arm_reset_vaddr);
	reg_v |= 0x1<<(cpu+1) | 0x1<<(cpu+5);
	writel(reg_v, global_arm_reset_vaddr);

	/* 3. ca55_core_pwrctrl_in */
	reg_v = readl(global_arm_config_b_vaddr);
	reg_v |= 1<<(cpu+ 20);
	writel(reg_v, global_arm_config_b_vaddr);

	/* 4. ca55_core_pwrctrl_2nd_in */
	reg_v |= 1<<(cpu+ 24);
	writel(reg_v, global_arm_config_b_vaddr);
	wmb();
	spin_unlock(&rtk_pwr_ctrl_lock);
	return 0;
}

static int __init rtk_taurus_pwr_ctrl_init(void){
	int ret = 0;

	ret =  cpuhp_setup_state_nocalls(CPUHP_BP_PREPARE_DYN + 1, "soc/rtk_pwr_ctrl:offline", NULL, percpu_cpu_offline);

	if(ret){
			printk("rtk_taurus_pwr_ctrl_init: cpuhp_setup_state_nocalls failed! ret=%d\n", ret);
			return -EBUSY;
	}

	if(percpu_cpu_reg_mapping()){
			printk("rtk_taurus_pwr_ctrl_init: Mem mapping fail!\n");
			return -ENOMEM;
	}
	return 0;
}

static void __exit rtk_taurus_pwr_ctrl_exit(void){
	/* todo */
}

module_init(rtk_taurus_pwr_ctrl_init);
module_exit(rtk_taurus_pwr_ctrl_exit);