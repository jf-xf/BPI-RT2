#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <soc/cortina/cortina-soc.h>

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    u32 quad_off             :  1 ; /* bits 0:0 */
    u32 rsrvd1               :  3 ;
    u32 wd_reset_subsys_enable :  1 ; /* bits 4:4 */
    u32 rsrvd2               :  1 ;
    u32 wd_reset_all_blocks  :  1 ; /* bits 6:6 */
    u32 wd_reset_remap       :  1 ; /* bits 7:7 */
    u32 wd_reset_ext_reset   :  1 ; /* bits 8:8 */
    u32 ext_reset            :  1 ; /* bits 9:9 */
    u32 cfg_pcie_0_clken     :  1 ; /* bits 10:10 */
    u32 rsrvd3               :  1 ;
    u32 cfg_pcie_1_clken     :  1 ; /* bits 12:12 */
    u32 rsrvd4               :  1 ;
    u32 cfg_pcie_2_clken     :  1 ; /* bits 14:14 */
    u32 gpon_pd              :  1 ; /* bits 15:15 */
    u32 epon_pd              :  1 ; /* bits 16:16 */
    u32 ext_eth_refclk       :  1 ; /* bits 17:17 */
    u32 refclk_sel           :  2 ; /* bits 19:18 */
    u32 rsrvd5               :  2 ;
    u32 ps_pcie0_en          :  1 ; /* bits 22:22 */
    u32 ps_pcie1_en          :  1 ; /* bits 23:23 */
    u32 ps_pcie2_en          :  1 ; /* bits 24:24 */
    u32 ps_usb3_en           :  1 ; /* bits 25:25 */
    u32 ps_pe_en             :  1 ; /* bits 26:26 */
    u32 l3fe_pd              :  1 ; /* bits 27:27 */
    u32 offload0_pd          :  1 ; /* bits 28:28 */
    u32 offload1_pd          :  1 ; /* bits 29:29 */
    u32 crypto_pd            :  1 ; /* bits 30:30 */
    u32 core_pd              :  1 ; /* bits 31:31 */
  } bf ;
  u32     wrd ;
} GLOBAL_GLOBAL_CONFIG_t;
#endif /* !__ASSEMBLER__ */


#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    u32 cfg_xfi0_10g         :  1 ; /* bits 0:0 */
    u32 cfg_xfi1_10g         :  1 ; /* bits 1:1 */
    u32 s0_PCIE2_ISOLATE_0   :  1 ; /* bits 2:2 */
    u32 s0_POW_PCIE2_0       :  1 ; /* bits 3:3 */
    u32 s0_PCIE2_ISOLATE_1   :  1 ; /* bits 4:4 */
    u32 s0_POW_PCIE2_1       :  1 ; /* bits 5:5 */
    u32 s1_PCIE2_ISOLATE_0   :  1 ; /* bits 6:6 */
    u32 s1_POW_PCIE2_0       :  1 ; /* bits 7:7 */
    u32 s1_PCIE2_ISOLATE_1   :  1 ; /* bits 8:8 */
    u32 s1_POW_PCIE2_1       :  1 ; /* bits 9:9 */
    u32 s2_CPHY_ISOLATE      :  1 ; /* bits 10:10 */
    u32 s2_ISO_ANA_B         :  1 ; /* bits 11:11 */
    u32 s2_POW_PCIE2         :  1 ; /* bits 12:12 */
    u32 s2_POW_USB3          :  1 ; /* bits 13:13 */
    u32 rsrvd1               :  1 ;
    u32 s3_ISO_ANA_B         :  1 ; /* bits 15:15 */
    u32 rsrvd2               :  1 ;
    u32 s3_POW_USB3          :  1 ; /* bits 17:17 */
    u32 s3_POW_SGMII         :  1 ; /* bits 18:18 */
    u32 USB2_ISOLATE         :  1 ; /* bits 19:19 */
    u32 rsrvd3               :  3 ;
    u32 s2_combo_sel         :  1 ; /* bits 23:23 */
    u32 s3_combo_sel         :  1 ; /* bits 24:24 */
    u32 s3_sata_sgmii_sel    :  2 ; /* bits 26:25 */
    u32 s0_rxaui_mode        :  1 ; /* bits 27:27 */
    u32 s0_p_mdio_enable_reg :  1 ; /* bits 28:28 */
    u32 s1_p_mdio_enable_reg :  1 ; /* bits 29:29 */
    u32 s2_p_mdio_enable_reg :  1 ; /* bits 30:30 */
    u32 s3_p_mdio_enable_reg :  1 ; /* bits 31:31 */
  } bf ;
  u32     wrd ;
} GLOBAL_PHY_CONTROL_t;
#endif /* !__ASSEMBLER__ */



#define CORTINA_SERIAL_PH_REG_BITS  32

#define VENUS_Temperature_Cooling 1

/********VENUS PHY Control *******************/
char global_phy_control_str[CORTINA_SERIAL_PH_REG_BITS][32] = {
"cfg_xfi0_10g",
"cfg_xfi1_10g",
"s0_PCIE2_ISOLATE_0",
"s0_POW_PCIE2_0",
"s0_PCIE2_ISOLATE_1",
"s0_POW_PCIE2_1",
"s1_PCIE2_ISOLATE_0",
"s1_POW_PCIE2_0",
"s1_PCIE2_ISOLATE_1",
"s1_POW_PCIE2_1",
"s2_CPHY_ISOLATE",
"s2_ISO_ANA_B",
"s2_POW_PCIE2",
"s2_POW_USB3",
"reserved0",
"s3_ISO_ANA_B",
"reserved1",
"s3_POW_USB3",
"s3_POW_SGMII",
"USB2_ISOLATE",
"reserved2",
"reserved3",
"reserved4",
"s2_combo_sel",
"s3_combo_sel",
"s3_sata_sgmii_sel[0]",
"s3_sata_sgmii_sel[1]",
"s0_rxaui_mode",
"s0_p_mdio_enable_reg",
"s1_p_mdio_enable_reg",
"s2_p_mdio_enable_reg",
"s3_p_mdio_enable_reg",
};


extern struct proc_dir_entry *realtek_proc;

static void  __maybe_unused rtk_peri_reg_write(uint32_t paddr, uint32_t reg_value)
{
    void __iomem *addr;
    addr = ioremap(paddr, 4);
    if (addr) {
        writel_relaxed(reg_value, addr);
        iounmap(addr);
    } else {
        printk("ioremap(0x%08x) fail!\n", paddr);
    }
}


static int rtk_peri_reg_read(uint32_t paddr, uint32_t * reg_v)
{
    void __iomem *addr;

    addr = ioremap(paddr, 4);
    if (addr) {
        *reg_v = readl_relaxed(addr);
        iounmap(addr);
        return 0;
    } else {
        return -1;
    }
}


#ifdef VENUS_Temperature_Cooling
#define   CORE_PD                BIT(31)
#define   CRYPTO_PD              BIT(30)
#define   OFFLOAD1_PD            BIT(29)
#define   OFFLOAD0_PD            BIT(28)
#define   L3FE_PD                BIT(27)
#define   PS_PE_EN               BIT(26)
#define   PS_USB3_EN             BIT(25)
#define   PS_PCIE2_EN            BIT(24)
#define   PS_PCIE1_EN            BIT(23)
#define   PS_PCIE0_EN            BIT(22)
#define   REFCLK_SEL_1           BIT(19)
#define   REFCLK_SEL_0           BIT(18)
#define   EXT_ETH_REFCLK         BIT(17)
#define   EPON_PD                BIT(16)
#define   GPON_PD                BIT(15)
#define   CFG_PCIE_2_CLKEN       BIT(14)
#define   CFG_PCIE_1_CLKEN       BIT(12)
#define   CFG_PCIE_0_CLKEN       BIT(10)
u32 GLOBAL_CONFIG_BOOT_VALUE=0;
enum{
	GLOBAL_GLOBAL_CONFIG_N = 0,
	GLOBAL_PHY_CONTROL_N = 1,
	GLOBAL_PIN_MUX_N = 2,
	GLOBAL_GPIO_MUX_0_N = 3,
	GLOBAL_GIGE_PHY_N = 4,
};
#define GLOBAL_GLOBAL_CONFIG_ADDR REG_ADD_LIST[GLOBAL_GLOBAL_CONFIG_N]
#define GLOBAL_PHY_CONTROL_ADDR REG_ADD_LIST[GLOBAL_PHY_CONTROL_N]
#define GLOBAL_PIN_MUX_ADDR REG_ADD_LIST[GLOBAL_PIN_MUX_N]
#define GLOBAL_GPIO_MUX_0_ADDR REG_ADD_LIST[GLOBAL_GPIO_MUX_0_N]
#define GLOBAL_GIGE_PHY_ADDR REG_ADD_LIST[GLOBAL_GIGE_PHY_N]
#define GLOBAL_JTAG_ID_ADDR 0xf4320000
static unsigned int REG_ADD_LIST[] = { 0 , 0, 0, 0, 0};

#ifdef CONFIG_RTK_DP_DIS_SOC
__weak void disableSLIC(void) {
	printk("%s(%d): not set\n",__func__,__LINE__);
}
__weak void pcie_shutdown_all(void) {
	printk("%s(%d): not set\n",__func__,__LINE__);
}
__weak void vbus_deassert(void){
	printk("%s(%d): not set\n",__func__,__LINE__);
}
void dyinggasp_dis_function(void)
{
	pcie_shutdown_all(); //close wlan
	vbus_deassert(); //power down USB
	disableSLIC();
}
static 	void __iomem *JTAG2GIGE_vaddr = NULL;
#define GLOBAL_JTAG2GIGE_SIZE (GLOBAL_GIGE_PHY_ADDR  - GLOBAL_JTAG_ID_ADDR)
static void JTAG2GIGE_vaddr_init(void){

	if (JTAG2GIGE_vaddr == NULL){
		JTAG2GIGE_vaddr = ioremap(GLOBAL_JTAG_ID_ADDR, GLOBAL_JTAG2GIGE_SIZE);
	}
	printk("###################################\n");
	printk("JTAG2GIGE_vaddr=%p!!\n",JTAG2GIGE_vaddr);
	printk("###################################\n");
}
void rtk_dyinggasp_dis_soc(void){
	GLOBAL_GLOBAL_CONFIG_t reg_v;

	dyinggasp_dis_function();
	if(JTAG2GIGE_vaddr){
		/* GLOBAL_PHY_CONTROL
		 * reset to default value, dis phys
		 */
		reg_v.wrd = 0x00008803 ; //default, disable phys
		writel_relaxed(reg_v.wrd, JTAG2GIGE_vaddr +(GLOBAL_PHY_CONTROL_ADDR - GLOBAL_JTAG_ID_ADDR));

		/* GLOBAL_GLOBAL_CONFIG
		 * dis most clocks, except core
		 */
		reg_v.wrd = 0x740001D0 ; //disable most IP clocks, except core_pd, l3fe_pd, ps_pe_en. Keep networing work.
		writel_relaxed(reg_v.wrd, JTAG2GIGE_vaddr +(GLOBAL_GLOBAL_CONFIG_ADDR - GLOBAL_JTAG_ID_ADDR));

		/* GLOBAL_PIN_MUX
		 * reset to default
		 */
		reg_v.wrd = 0x80000000;
		writel_relaxed(reg_v.wrd, JTAG2GIGE_vaddr +(GLOBAL_PIN_MUX_ADDR - GLOBAL_JTAG_ID_ADDR));

		/* GLOBAL_GPIO_MUX_0
		 * reset to default, dis gpio0's 0~15 for leds
		 */
		reg_v.wrd = 0x0;
		writel_relaxed(reg_v.wrd, JTAG2GIGE_vaddr +(GLOBAL_GPIO_MUX_0_ADDR - GLOBAL_JTAG_ID_ADDR));
	}
}
EXPORT_SYMBOL(rtk_dyinggasp_dis_soc);
#endif

/* Disable some IP to cool.
 * 1. crypto
 * 2. offload0

*/


static void rtk_GLOBAL_CONFIG_dis_unused(void __iomem *addr){
         GLOBAL_GLOBAL_CONFIG_t reg_v;
         if(addr == NULL){
             return;
         }

         reg_v.wrd = readl_relaxed(addr);

         if(GLOBAL_CONFIG_BOOT_VALUE==0){
                GLOBAL_CONFIG_BOOT_VALUE=reg_v.wrd;
         }
		#ifndef CONFIG_CORTINA_CRYPTO
		if(!IS_TAURUS){
                 /*xxx_PD:  When set power down core clock.,  1-> Power down */
                 reg_v.bf.crypto_pd = 1;
				 printk("\033[1;31mRTK GLOBAL_GLOBAL_CONFIG: Power down crypto_pd.\033[0m\n");
		}
		#endif
		#ifndef CONFIG_OFFLOAD1_PE1_SUPPORT
                 reg_v.bf.offload1_pd = 1;
				 printk("\033[1;31mRTK GLOBAL_GLOBAL_CONFIG: Power down offload1 cpu clock(PE1).\033[0m\n");
		#endif
		#ifndef CONFIG_PCIE2_SUPPORT
                 /* When set to 1 xxxx totally powered up */
                 reg_v.bf.ps_pcie2_en =0;
                 reg_v.bf.cfg_pcie_2_clken=0;
                 smp_mb();
                 writel_relaxed(reg_v.wrd, addr);
				 printk("\033[1;31mRTK GLOBAL_GLOBAL_CONFIG: Power down PCIE2 clock.\033[0m\n");
		#endif
		return;
}

static void rtk_GLOBAL_GLOBAL_CONFIG(int action)
{
    GLOBAL_GLOBAL_CONFIG_t reg_v;
    void __iomem *addr;
    addr = ioremap(GLOBAL_GLOBAL_CONFIG_ADDR, 4);

    if (addr) {
        switch (action){
            case 1:
                 rtk_GLOBAL_CONFIG_dis_unused(addr);
                 break;
            case 0:
                 if(GLOBAL_CONFIG_BOOT_VALUE){
                     reg_v.wrd = GLOBAL_CONFIG_BOOT_VALUE;
                     writel_relaxed(reg_v.wrd, addr);
                 }
                 break;
        }
        iounmap(addr);
    } else {
        printk("GLOBAL_GLOBAL_CONFIG: ioremap failed!\n");
    }
}



static int rtk_g3_cooling_proc_read(struct seq_file *sf, void *v)
{
    uint32_t tmp = 0;
    int i = 0;
    int array_size;
    array_size = sizeof(REG_ADD_LIST) / sizeof(int);
    printk("8277x:\n");
    for (i = 0; i < array_size; i++) {
        rtk_peri_reg_read(REG_ADD_LIST[i], &tmp);
        printk("Addr(0x%08x) = 0x%08x\n", REG_ADD_LIST[i], tmp);
    }
    return 0;
}


static int rtk_g3_cooling_single_open(struct inode *inode,
                                      struct file *file)
{
    return (single_open(file, rtk_g3_cooling_proc_read, NULL));
}


static ssize_t rtk_g3_cooling_proc_write(struct file *file,
                                         const char __user * buffer,
                                         size_t count, loff_t * off)
{
	char tmpbuf[64];
    int tmp;
	
	if(!(buffer && count < sizeof(tmpbuf) && !copy_from_user(tmpbuf, buffer, count)))
		return -EFAULT;
	
	tmpbuf[count]='\0';
    if (sscanf(tmpbuf, "%d", &tmp) != 1) {
        return -1;
    }

    if (tmp) {
		switch (tmp) {
		case 1:
			rtk_GLOBAL_GLOBAL_CONFIG(1);
			break;
		case 9:
#ifdef CONFIG_RTK_DP_DIS_SOC
			rtk_dyinggasp_dis_soc();
#endif
			break;
		default:
			rtk_GLOBAL_GLOBAL_CONFIG(1);
		}
    } else {
        printk("Revert to Boot Setting.");
        rtk_GLOBAL_GLOBAL_CONFIG(0);
    }
    return count;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
static const struct file_operations fops_g3_cooling = {
        .owner          = THIS_MODULE,
        .open           = rtk_g3_cooling_single_open,
        .read           = seq_read,
        .write          = rtk_g3_cooling_proc_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#else
static struct proc_ops fops_g3_cooling = {
        .proc_open           = rtk_g3_cooling_single_open,
        .proc_read           = seq_read,
        .proc_write          = rtk_g3_cooling_proc_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#endif
#endif  //VENUS_Temperature_Cooling


static int rtk_phy_control_proc_read(struct seq_file *sf, void *v)
{
    GLOBAL_PHY_CONTROL_t reg_v;
    void __iomem *addr;
    int i;

    addr = ioremap(GLOBAL_PHY_CONTROL_ADDR, 4);
    if (addr) {
        reg_v.wrd = readl_relaxed(addr);
        seq_printf(sf, "GLOBAL_PHY_CONTROL: 0x%x\n", reg_v.wrd);

        for (i = 0; i < CORTINA_SERIAL_PH_REG_BITS; i++) {
            seq_printf(sf, "   %s, bits(%d): %d\n",
                       &global_phy_control_str[i][0], i,
                       (reg_v.wrd & (1 << i)) >> i);
        }

        iounmap(addr);
    } else {
        printk("GLOBAL_PHY_CONTROL: ioremap failed!\n");
    }


    return 0;
}

static int rtk_phy_control_single_open(struct inode *inode,
                                       struct file *file)
{
    return (single_open(file, rtk_phy_control_proc_read, NULL));
}

static void rtk_phy_control_bit_write(int bit)
{

    GLOBAL_PHY_CONTROL_t reg_v;
    void __iomem *addr;
    addr = ioremap(GLOBAL_PHY_CONTROL_ADDR, 4);

    if (addr) {
        reg_v.wrd = readl_relaxed(addr);
        //printk("GLOBAL_PHY_CONTROL: addr(0x%x) = 0x%p\n", addr, reg_v.wrd);
        reg_v.wrd &= ~(1 << bit);
        writel_relaxed(reg_v.wrd, addr);
        smp_mb();
        reg_v.wrd = readl_relaxed(addr);
        printk("\033[1;33mGLOBAL_PHY_CONTROL: 0x%x. %s put into reset.\033[0m\n", reg_v.wrd, &global_phy_control_str[bit][0]);
        iounmap(addr);
    } else {
        printk("GLOBAL_PHY_CONTROL: ioremap failed!\n");
    }

}

static ssize_t rtk_phy_control_proc_write(struct file *file,
                                          const char __user * buffer,
                                          size_t count, loff_t * off)
{

    char buf[128];
    int i;
    if (copy_from_user(buf, buffer, count)) {
        return -EFAULT;
    }

    if (count > 0)
    buf[count - 1] = '\0';

    for (i = 0; i < CORTINA_SERIAL_PH_REG_BITS; i++) {
        if (strncmp(buf, &global_phy_control_str[i][0], 32) == 0) {
            rtk_phy_control_bit_write(i);
            break;
        }
    }

    return count;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
static const struct file_operations fops_phy_control = {
        .owner          = THIS_MODULE,
        .open           = rtk_phy_control_single_open,
        .read           = seq_read,
        .write          = rtk_phy_control_proc_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#else
static struct proc_ops fops_phy_control = {
        .proc_open           = rtk_phy_control_single_open,
        .proc_read           = seq_read,
        .proc_write          = rtk_phy_control_proc_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#endif

static int __init rtk_g3phy_clr_init(void)
{
    struct proc_dir_entry *pe;

    pe = proc_create_data("g3_phy_ctrl_bit_clr",
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
                          realtek_proc, &fops_phy_control, NULL);
    if (!pe) {
        return -EINVAL;
    }
#if defined(VENUS_Temperature_Cooling)
    pe = proc_create_data("8277x_POWER_DOWN_UNUSED_UNIT",
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
                          realtek_proc, &fops_g3_cooling, NULL);
    if (!pe) {
        return -EINVAL;
    }
#endif

#ifdef CONFIG_RTK_DP_DIS_SOC
    JTAG2GIGE_vaddr_init();
#endif

    return 0;
}

static void __exit rtk_g3phy_clr_exit(void)
{
    return;
}

module_init(rtk_g3phy_clr_init);
module_exit(rtk_g3phy_clr_exit);
#define GLOBAL_PHY_CONTROL_B                       0xf4320050
#define GLOBAL_PHY_CONTROL_C                       0xf43200c4
static int __init soc_hw_config(void){
	if(IS_TAURUS){
		REG_ADD_LIST[GLOBAL_GLOBAL_CONFIG_N]	= 0xf43200c0;
		REG_ADD_LIST[GLOBAL_PHY_CONTROL_N]		= GLOBAL_PHY_CONTROL_C;
		REG_ADD_LIST[GLOBAL_PIN_MUX_N]		= 0xf43200f0;
		REG_ADD_LIST[GLOBAL_GPIO_MUX_0_N]	= 0xf4320100;
		REG_ADD_LIST[GLOBAL_GIGE_PHY_N]		= 0xf4320114;
		rtk_GLOBAL_GLOBAL_CONFIG(1);
	}else{
		/*
			VENUS is call "echo 1 > /proc/realtek/8277x_POWER_DOWN_UNUSED_UNIT"
			by /etc/scripts/board_init.sh with rc35 of init process.
		*/
		REG_ADD_LIST[GLOBAL_GLOBAL_CONFIG_N]	= 0xf432004c;
		REG_ADD_LIST[GLOBAL_PHY_CONTROL_N]		= GLOBAL_PHY_CONTROL_B;
		REG_ADD_LIST[GLOBAL_PIN_MUX_N]		= 0xf4320078;
		REG_ADD_LIST[GLOBAL_GPIO_MUX_0_N]	= 0xf4320080;
		REG_ADD_LIST[GLOBAL_GIGE_PHY_N]		= 0xf4320094;
	}
	return 0;
}
/* after arch_initcall for RTK_CHIP_ID to judge whether is taurus or not */
subsys_initcall(soc_hw_config);
