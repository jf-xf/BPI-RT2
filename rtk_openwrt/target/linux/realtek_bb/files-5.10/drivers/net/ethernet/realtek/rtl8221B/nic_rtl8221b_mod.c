#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>	/* mdelay() */
#include <fs/proc/internal.h>

#include <common/rt_error.h>
#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
#include <rtk_ext_gphy.h>
#endif
#include "nic_rtl8226b.h"
#include "nic_rtl8226b_init.h"

#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
#define RTK_EXTGPHY_RTL8221B_MAX	2
static HANDLE hDevice[RTK_EXTGPHY_RTL8221B_MAX];
static int rtl8221b_count = 0;
static int rtl8221b_init = 0;
#endif

#define CONFIG_RTL_8221B_DEFAULT_MDIO_SET	(0)
#ifndef CONFIG_RTL_8221B_DEVICE_0_MDIO_SET
#define CONFIG_RTL_8221B_DEVICE_0_MDIO_SET	CONFIG_RTL_8221B_DEFAULT_MDIO_SET
#endif
#ifndef CONFIG_RTL_8221B_DEVICE_1_MDIO_SET
#define CONFIG_RTL_8221B_DEVICE_1_MDIO_SET	CONFIG_RTL_8221B_DEFAULT_MDIO_SET
#endif

#ifdef CONFIG_LUNA_G3_SERIES

#include <aal_phy.h>

#define CA_AAL_MDIO_ST_CODE_C22		1
#define CA_AAL_MDIO_ST_CODE_C45		0

#define RTL8221B_RESET_GPIO_NO		14			/* GPIO4_[1]: 4 * 32 + 1 */
#define RTL8221B_RESET_GPIO_LABEL	"rtl8221b_reset"

/*
 * Default Serdes Mode (write bit [5:0] = 0/1/2/3  SDS mode)
 * 0: 2500Base-X/SGMII
 * 1: HiSGMII/SGMII
 * 2: 2500Base-X
 * 3: HiSGMII
 */
#define DEFAULT_SERDES_MODE		0x1

extern ca_status_t aal_mdio_write(/*CODEGEN_IGNORE_TAG*/
    CA_IN       ca_device_id_t             device_id,
    CA_IN      ca_uint8_t              st_code,
    CA_IN      ca_uint8_t              phy_port_addr,
    CA_IN      ca_uint8_t              reg_dev_addr,
    CA_IN      ca_uint16_t              addr,
    CA_IN      ca_uint16_t              data);

extern ca_status_t aal_mdio_read(/*CODEGEN_IGNORE_TAG*/
    CA_IN       ca_device_id_t             device_id,
    CA_IN      ca_uint8_t              st_code,
    CA_IN      ca_uint8_t              phy_port_addr,
    CA_IN      ca_uint8_t              reg_dev_addr,
    CA_IN      ca_uint16_t              addr,
    CA_OUT     ca_uint16_t             *data);

#else

#include <rtk/gpio.h>
//#include <rtk/port.h>		/* for rtk_port_serdesMode_get */
#include <rtk/mdio.h>

//#define IO_GPIO_EN_REG		0xBB000038
#define IO_MODE_EN_REG			0xBB023014
#define MDIO_MASTER_EN			(1 << 10)
#define EXT_MDX_M_EN			(1 << 11)

#define RTL8221B_MDC_SW_GPIO_NO_6	6
#define RTL8221B_MDC_SW_GPIO_NO_10	10

#define RTL9607C_SET0_MDC_PIN       6
#define RTL9607C_SET0_MDIO_PIN      7
#define RTL9607C_SET1_MDC_PIN       12
#define RTL9607C_SET1_MDIO_PIN      10

#define RTL8221B_DEV0_RESET_PIN      60
#define DEFAULT_MDIO_PORT_NUM		0
#define RTK_MDIO_FMT_C22		0
#define RTK_MDIO_FMT_C45		1

/*
 * Default Serdes Mode (write bit [5:0] = 0/1/2/3  SDS mode)
 * 0: 2500Base-X/SGMII
 * 1: HiSGMII/SGMII
 * 2: 2500Base-X
 * 3: HiSGMII
 */
#define DEFAULT_SERDES_MODE		0x1

#define DEVICE0_SERDES_NUMBER		0
#define DEVICE1_SERDES_NUMBER		1

#endif

struct proc_dir_entry *rtl8221b_proc_dir = NULL;

#define PROC_DIR_RTL8221B		"rtl8221b"
#define PROC_FILE_HELP			"help"
#define PROC_FILE_PHY			"phy"
#define PROC_FILE_LINK_STATUS		"link_status"

/* Register access macro */
#ifndef REG32
#define REG32(reg)      (*((volatile unsigned int *)(reg)))
#endif
#ifndef REG16
#define REG16(reg)      (*((volatile unsigned short *)(reg)))
#endif
#ifndef REG8
#define REG8(reg)       (*((volatile unsigned char *)(reg)))
#endif

#ifdef CONFIG_RTL_8221B_DEVICE_0
//extern int rtk_8221b_reset_gpio_get(unsigned int dev);
int __attribute__((weak)) rtk_8221b_reset_gpio_get(unsigned int dev)		{return (-1);}
#endif

#if 0
//spinlock_t lock_mdio;
BOOLEAN
init_mdio_lock(void)
{
    printk("Init mdio lock\n");
    spin_lock_init(&lock_mdio);
    return SUCCESS;
}
#endif

BOOLEAN
MmdPhyRead(
    IN  HANDLE hDevice,
    IN  UINT16 dev,
    IN  UINT16 addr,
    OUT UINT16 *data)
{
#ifdef CONFIG_LUNA_G3_SERIES
    /**aal_mdio_read:
    *@breif: Read MDIO slaver device data with 16bits width
    *@param [in] st_code : Start of Frame (01 for Clause 22;00 for Clause 45)
    *@param [in] phy_port_addr : PHY Address, 0~31
    *@param [in] reg_dev_addr  : Register Address, 0~31
    *@param [in] addr  : extended address for Clause45, normally NOT used
    *@param [out] data : the Data returned from MDIO slaver
    *@return: CA_E_OK if successfully, otherwise return CA_E_PARAM
    */
    ca_status_t ret = CA_E_OK;

    ret = aal_mdio_read(hDevice.unit, CA_AAL_MDIO_ST_CODE_C45, hDevice.port, dev, addr, data);
    if (ret != CA_E_OK)
    {
        RTL8221B_ERROR("%s (dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (FAILED)\n",
                       __FUNCTION__, dev, addr, *data, ret);
        return FAILURE;
    }
#else
    int32 ret;

    rtk_gpio_state_set(hDevice.gpio_mdc, DISABLED);
    rtk_gpio_state_set(hDevice.gpio_mdio, DISABLED);
    //spin_lock_bh(&lock_mdio);
    rtk_mdio_cfg_set(hDevice.unit, DEFAULT_MDIO_PORT_NUM, hDevice.port, RTK_MDIO_FMT_C45);
    ret = rtk_mdio_c45_read((uint8)dev, addr, data);
    if (ret == RT_ERR_FAILED)
    {
        RTL8221B_ERROR("%s (dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (FAILED)\n",
                       __FUNCTION__, dev, addr, *data, ret);
        return FAILURE;
    }
    //spin_unlock_bh(&lock_mdio);
    //RTL8221B_DEBUG("%s (dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (OK)\n",
    //	__FUNCTION__, dev, addr, *data, ret);
#endif
    return SUCCESS;
}

BOOLEAN
MmdPhyWrite(
    IN HANDLE hDevice,
    IN UINT16 dev,
    IN UINT16 addr,
    IN UINT16 data)
{
#ifdef CONFIG_LUNA_G3_SERIES
    /**aal_mdio_write:
    *@breif: Write data to MDIO slaver device with 16bits width
    *@param [in] st_code : Start of Frame (01 for Clause 22;00 for Clause 45)
    *@param [in] phy_port_addr : PHY Address, 0~31
    *@param [in] reg_dev_addr  : Register Address, 0~31
    *@param [in] addr  : extended address for Clause45, normally NOT used
    *@param [in] data  : the Data writen to MDIO slaver
    *@return: CA_E_OK if successfully, otherwise return CA_E_PARAM
    */
    ca_status_t ret = CA_E_OK;

    ret = aal_mdio_write(hDevice.unit, CA_AAL_MDIO_ST_CODE_C45, hDevice.port, dev, addr, data);
    if (ret != CA_E_OK)
    {
        RTL8221B_ERROR("%s (dev = %d, addr = 0x%04x, data = 0x%04x): - ret = %d (FAILED)\n",
                       __FUNCTION__, dev, addr, data, ret);
        return FAILURE;
    }
#else
    int32   ret;

    rtk_gpio_state_set(hDevice.gpio_mdc, DISABLED);
    rtk_gpio_state_set(hDevice.gpio_mdio, DISABLED);
    //spin_lock_bh(&lock_mdio);
    rtk_mdio_cfg_set(hDevice.unit, DEFAULT_MDIO_PORT_NUM, hDevice.port, RTK_MDIO_FMT_C45);
    ret = rtk_mdio_c45_write((uint8)dev, addr, data);
    if (ret == RT_ERR_FAILED)
    {
        RTL8221B_ERROR("%s (dev = %d, addr = 0x%04x, data = 0x%04x): - ret = %d (FAILED)\n",
                       __FUNCTION__, dev, addr, data, ret);
        return FAILURE;
    }
    //spin_unlock_bh(&lock_mdio);
    //RTL8221B_DEBUG("%s (dev = %d, addr = 0x%04x, data = 0x%04x): - ret = %d (OK)\n",
    //		__FUNCTION__, dev, addr, data, ret);
#endif
    return SUCCESS;
}

void rtl8221b_usage(const char *filename)
{
    RTL8221B_MSG("[Usage]");

    if((strcasecmp(filename, PROC_FILE_PHY) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0))
    {
        RTL8221B_MSG("echo [$action] [$phyad]> /proc/%s/%s", PROC_DIR_RTL8221B, PROC_FILE_PHY);
        RTL8221B_MSG("\t$action: {init,reset}");
        RTL8221B_MSG("\t$phyad: rtl8221b phy address");
        if(strcasecmp(filename, PROC_FILE_PHY) == 0)		return;
    }

    if((strcasecmp(filename, PROC_FILE_LINK_STATUS) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0))
    {
        RTL8221B_MSG("cat /proc/%s/%s", PROC_DIR_RTL8221B, PROC_FILE_LINK_STATUS);
        if(strcasecmp(filename, PROC_FILE_LINK_STATUS) == 0)		return;
    }

    return;
}

/*
 * rtl8221b proc function
 */
#define INVALID_VALUE	99
#define MAX_COMMAND_LEN	32

static HANDLE hDevice0;
static HANDLE hDevice1;

static int help_fops(struct seq_file *s, void *v)
{
    rtl8221b_usage(PROC_FILE_HELP);
    return SUCCESS;
}

static int Rtl8221b_phy_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    char tmpbuf[MAX_COMMAND_LEN] = {0};
    int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
    static PHY_LINK_ABILITY phylinkability;
    int phyad;
    int xmdio,xvend;
    HANDLE xDevice;
    UINT16 xreg,phydata = 0;

    BOOL status = FAILURE;

    RTL8221B_DEBUG("%s - %s: %s (%d) - %s", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
    if (buffer)
    {
        char *strptr, *split_str;

        /* copy data to the buffer */
        strncpy(tmpbuf, buffer, len);
        tmpbuf[len] = '\0';

        strptr = tmpbuf;
        RTL8221B_DEBUG("strptr: [%s]", strptr);

        /*parse command*/
        split_str = strsep(&strptr," ");
        RTL8221B_DEBUG("split_str [%s]", split_str);

        if(strcasecmp(split_str, "init") == 0)
        {
            BOOL singlephy = 1;
            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            phyad = simple_strtol(split_str, NULL, 0);

            if (phyad == hDevice0.port)
                status = Rtl8226b_phy_init(hDevice0, &phylinkability,singlephy);
            else if (phyad == hDevice1.port)
                status = Rtl8226b_phy_init(hDevice1, &phylinkability,singlephy);
            else
            {
                RTL8221B_ERROR("UNKNOWN phyad %d", phyad);
                goto phy_error;
            }

            if (status == SUCCESS)
            {
                RTL8221B_MSG("Rtl8226b_phy_init Done (phyad %d)", phyad);
#ifdef CONFIG_SWITCH_INIT_LINKDOWN
                if (phyad == hDevice0.port)
                    Rtl8226b_enable_set(hDevice0, ENABLED);
                else if (phyad == hDevice1.port)
                    Rtl8226b_enable_set(hDevice1, ENABLED);
#endif
            }
            else
                RTL8221B_ERROR("Rtl8226b_phy_init Fail (phyad %d)", phyad);
        }
        else if(strcasecmp(split_str, "reset") == 0)
        {
            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            phyad = simple_strtol(split_str, NULL, 0);

            if (phyad == hDevice0.port)
                status = Rtl8226b_phy_reset(hDevice0);
            else if (phyad == hDevice1.port)
                status = Rtl8226b_phy_reset(hDevice1);
            else
            {
                RTL8221B_ERROR("UNKNOWN phyad %d", phyad);
                goto phy_error;
            }

            if (status == SUCCESS)
                RTL8221B_MSG("Rtl8226b_phy_reset Done (phyad %d)", phyad);
            else
                RTL8221B_ERROR("Rtl8226b_phy_reset Fail (phyad %d)", phyad);
        }
        /*
        	usage:
        	echo mmdr [mdio_set] [phy_id] [vender_id] [reg] > /proc/rtl8221b/phy
        	echo mmdw [mdio_set] [phy_id] [vender_id] [reg] [data] > /proc/rtl8221b/phy
         */
        else if(strcasecmp(split_str, "mmdr") == 0)
        {
            split_str = strsep(&strptr," ");
            xmdio = simple_strtol(split_str, NULL, 0);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            phyad = simple_strtol(split_str, NULL, 0);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            xvend = simple_strtol(split_str, NULL, 0);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            xreg = simple_strtol(split_str, NULL, 16);

            xDevice.unit = xmdio;
            xDevice.port = phyad;
#ifndef CONFIG_LUNA_G3_SERIES
            if (xmdio == 0) {
                xDevice.gpio_mdc = RTL9607C_SET0_MDC_PIN;
                xDevice.gpio_mdio = RTL9607C_SET0_MDIO_PIN;
            }
            else {
                xDevice.gpio_mdc = RTL9607C_SET1_MDC_PIN;
                xDevice.gpio_mdio = RTL9607C_SET1_MDIO_PIN;
            }
#endif
            status = MmdPhyRead(xDevice, xvend, xreg, &phydata);
            if (status == SUCCESS)
                printk(" => MmdPhyRead: reg 0x%x, data 0x%x\n",xreg,phydata);
            else
                printk(" => MmdPhyRead error\n");
        }
        else if(strcasecmp(split_str, "mmdw") == 0)
        {
            split_str = strsep(&strptr," ");
            xmdio = simple_strtol(split_str, NULL, 0);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            phyad = simple_strtol(split_str, NULL, 0);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            xvend = simple_strtol(split_str, NULL, 0);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            xreg = simple_strtol(split_str, NULL, 16);

            split_str = strsep(&strptr," ");
            if (split_str == NULL)
                goto phy_error;
            phydata = simple_strtol(split_str, NULL, 16);

            xDevice.unit = xmdio;
            xDevice.port = phyad;
#ifndef CONFIG_LUNA_G3_SERIES
            if (xmdio == 0) {
                xDevice.gpio_mdc = RTL9607C_SET0_MDC_PIN;
                xDevice.gpio_mdio = RTL9607C_SET0_MDIO_PIN;
            }
            else {
                xDevice.gpio_mdc = RTL9607C_SET1_MDC_PIN;
                xDevice.gpio_mdio = RTL9607C_SET1_MDIO_PIN;
            }
#endif
            status = MmdPhyWrite(xDevice, xvend, xreg, phydata);
            if (status == SUCCESS)
                printk(" => MmdPhyWrite: reg 0x%x, data 0x%x\n",xreg,phydata);
            else
                printk(" => MmdPhyWrite error\n");
        }
        else if(strcasecmp(split_str, "power_up") == 0)
        {
#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
	int i;
        for (i = 0 ; i  < rtl8221b_count ; i++)
            Rtl8226b_enable_set(hDevice[i], ENABLED);
#else
#ifdef CONFIG_RTL_8221B_DEVICE_0
            Rtl8226b_enable_set(hDevice0, ENABLED);
#endif
#ifdef CONFIG_RTL_8221B_DEVICE_1
            Rtl8226b_enable_set(hDevice1, ENABLED);
#endif
#endif
        }
        else
        {
            goto phy_error;
        }
    }
    return count;

phy_error:
    RTL8221B_ERROR("%s() FAIL ......", __FUNCTION__);
    rtl8221b_usage(file->f_path.dentry->d_iname);
    return -EFAULT;
}

static int Rtl8221_link_status_fops(struct seq_file *s, void *v)
{
#if (defined(CONFIG_RTL_8221B_DEVICE_0) || defined(CONFIG_RTL_8221B_DEVICE_1))
    BOOL status = FAILURE;
    BOOL plinkok = INVALID_VALUE;
    UINT16 pSpeed;

    BOOL perdesLink;
    PHY_SERDES_MODE SerdesMode;

    char sDesc[16];
#endif

#ifdef CONFIG_RTL_8221B_DEVICE_0
    status = Rtl8226b_is_link(hDevice0, &plinkok);
    if (status == SUCCESS)
    {
        RTL8221B_MSG("Rtl8226b_is_link(phyAddr %d): %d [%s]", hDevice0.port, plinkok, plinkok ? "link" : "not link");

        status = Rtl8226b_speed_get(hDevice0, &pSpeed);
        if (status == SUCCESS)
        {
            switch (pSpeed)
            {
            case LINK_SPEED_10M:
                strncpy(sDesc, "10M\0", 4);
                break;
            case LINK_SPEED_100M:
                strncpy(sDesc, "100M\0", 5);
                break;
            case LINK_SPEED_500M:
                strncpy(sDesc, "500M\0", 5);
                break;
            case LINK_SPEED_1G:
                strncpy(sDesc, "1G\0", 3);
                break;
            case LINK_SPEED_2P5G:
                strncpy(sDesc, "2.5G\0", 5);
                break;
            case NO_LINK:
                strncpy(sDesc, "nolink", 7);
                break;
            default:
                strncpy(sDesc, "unknown\0", 8);
                break;
            }
            RTL8221B_MSG("Rtl8226b_speed_get(phyAddr %d): %d [%s]", hDevice0.port, pSpeed, sDesc);

            status = Rtl8226b_serdes_link_get(hDevice0, &perdesLink, &SerdesMode);
            if (status == SUCCESS)
            {
                switch (SerdesMode)
                {
                case PHY_SERDES_MODE_SGMII:
                    strncpy(sDesc, "SGMII\0", 6);
                    break;
                case PHY_SERDES_MODE_USXGMII:
                    strncpy(sDesc, "USXGMII\0", 8);
                    break;
                case PHY_SERDES_MODE_HiSGMII:
                    strncpy(sDesc, "HiSGMII\0", 8);
                    break;
                case PHY_SERDES_MODE_2500BASEX:
                    strncpy(sDesc, "2500BASEX\0", 10);
                    break;
                case PHY_SERDES_MODE_NO_SDS:
                case PHY_SERDES_MODE_OTHER:
                default:
                    strncpy(sDesc, "unknown\0", 8);
                    break;
                }
                RTL8221B_MSG("Rtl8226b_serdes_link_get(phyAddr %d): %d / %d [%s]", hDevice0.port, perdesLink, pSpeed, sDesc);
            }
            else
            {
                RTL8221B_ERROR("Rtl8226b_serdes_link_get(phyAddr %d) Fail, [status = %d]", hDevice0.port, status);
                goto link_status_error;
            }
        }
        else
        {
            RTL8221B_ERROR("Rtl8226b_speed_get(phyAddr %d) Fail, [status = %d]", hDevice0.port, status);
            goto link_status_error;
        }
    }
    else
    {
        RTL8221B_ERROR("Rtl8226b_is_link(phyAddr %d) Fail, [status = %d]", hDevice0.port, status);
        goto link_status_error;
    }
#endif

#ifdef CONFIG_RTL_8221B_DEVICE_1
    status = Rtl8226b_is_link(hDevice1, &plinkok);
    if (status == SUCCESS)
    {
        RTL8221B_MSG("Rtl8226b_is_link(phyAddr %d): %d [%s]", hDevice1.port, plinkok, plinkok ? "link" : "not link");

        status = Rtl8226b_speed_get(hDevice1, &pSpeed);
        if (status == SUCCESS)
        {
            switch (pSpeed)
            {
            case LINK_SPEED_10M:
                strncpy(sDesc, "10M\0", 4);
                break;
            case LINK_SPEED_100M:
                strncpy(sDesc, "100M\0", 5);
                break;
            case LINK_SPEED_500M:
                strncpy(sDesc, "500M\0", 5);
                break;
            case LINK_SPEED_1G:
                strncpy(sDesc, "1G\0", 3);
                break;
            case LINK_SPEED_2P5G:
                strncpy(sDesc, "2.5G\0", 5);
                break;
            case NO_LINK:
                strncpy(sDesc, "nolink", 7);
                break;
            default:
                strncpy(sDesc, "unknown\0", 8);
                break;
            }
            RTL8221B_MSG("Rtl8226b_speed_get(phyAddr %d): %d [%s]", hDevice1.port, pSpeed, sDesc);

            status = Rtl8226b_serdes_link_get(hDevice1, &perdesLink, &SerdesMode);
            if (status == SUCCESS)
            {
                switch (SerdesMode)
                {
                case PHY_SERDES_MODE_SGMII:
                    strncpy(sDesc, "SGMII\0", 6);
                    break;
                case PHY_SERDES_MODE_USXGMII:
                    strncpy(sDesc, "USXGMII\0", 8);
                    break;
                case PHY_SERDES_MODE_HiSGMII:
                    strncpy(sDesc, "HiSGMII\0", 8);
                    break;
                case PHY_SERDES_MODE_2500BASEX:
                    strncpy(sDesc, "2500BASEX\0", 10);
                    break;
                case PHY_SERDES_MODE_NO_SDS:
                case PHY_SERDES_MODE_OTHER:
                default:
                    strncpy(sDesc, "unknown\0", 8);
                    break;
                }
                RTL8221B_MSG("Rtl8226b_serdes_link_get(phyAddr %d): %d / %d [%s]", hDevice1.port, perdesLink, pSpeed, sDesc);
            }
            else
            {
                RTL8221B_ERROR("Rtl8226b_serdes_link_get(phyAddr %d) Fail, [status = %d]", hDevice0.port, status);
                goto link_status_error;
            }

        }
        else
        {
            RTL8221B_ERROR("Rtl8226b_speed_get_(phyAddr %d) Fail, [status = %d]", hDevice1.port, status);
            goto link_status_error;
        }
    }
    else
    {
        RTL8221B_ERROR("Rtl8226b_is_link(phyAddr %d) Fail, [status = %d]", hDevice1.port, status);
        goto link_status_error;
    }

#endif
    return SUCCESS;

link_status_error:
    RTL8221B_ERROR("%s() FAIL ......", __FUNCTION__);
    rtl8221b_usage(PROC_FILE_LINK_STATUS);
    return -EFAULT;
}

/*
 * rtl8221b proc definition
 */
typedef enum rtk_rtl8221b_procDir_e
{
    PROC_DIR_RTL8221B_ROOT,
    PROC_DIR_RTL8221B_LEEF //this field must put at last.
} rtk_rtl8221b_procDir_t;

typedef struct rtk_rtl8221b_proc_s
{
    char *name;
    int (*get) (struct seq_file *s, void *v);
    int (*set) (struct file *file, const char __user *buffer, size_t count, loff_t *ppos);
    unsigned int inode_id;
    unsigned char unlockBefortWrite;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
    struct proc_ops proc_fops;
#else
    struct file_operations proc_fops;
#endif
    rtk_rtl8221b_procDir_t dir;
} rtk_rtl8221b_proc_t;

rtk_rtl8221b_proc_t rtl8221bProc[]=
{
    {
        .name= PROC_FILE_HELP,
        .get = help_fops,
        .set = NULL,
        .dir = PROC_DIR_RTL8221B_ROOT,
    },
    {
        .name= PROC_FILE_PHY,
        .get = NULL,
        .set = Rtl8221b_phy_fops,
        .dir = PROC_DIR_RTL8221B_ROOT,
    },
    {
        .name= PROC_FILE_LINK_STATUS,
        .get = Rtl8221_link_status_fops,
        .set = NULL,
        .dir = PROC_DIR_RTL8221B_ROOT,
    },
};

/*
 * rtl8221b common proc function
 */
static void *rtk_rtl8221b_single_start(struct seq_file *p, loff_t *pos)
{
    return NULL + (*pos == 0);
}

static void *rtk_rtl8221b_single_next(struct seq_file *p, void *v, loff_t *pos)
{
    ++*pos;
    return NULL;
}

static void rtk_rtl8221b_single_stop(struct seq_file *p, void *v)
{
}

int rtk_rtl8221b_seq_open(struct file *file, const struct seq_operations *op)
{
#if 1
	seq_open(file, op);
#else
    struct seq_file *p = file->private_data;

    if (!p)
    {
        p = kmalloc(sizeof(*p), GFP_ATOMIC);
        if (!p)
        {
            return -ENOMEM;
        }
        file->private_data = p;
    }
    memset(p, 0, sizeof(*p));
    mutex_init(&p->lock);
    p->op = op;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 79)
    p->file = file;
#else
#ifdef CONFIG_USER_NS
    p->user_ns = file->f_cred->user_ns;
#endif
#endif
    /*
     * Wrappers around seq_open(e.g. swaps_open) need to be
     * aware of this. If they set f_version themselves, they
     * should call seq_open first and then set f_version.
     */
    file->f_version = 0;

    /*
     * seq_files support lseek() and pread().  They do not implement
     * write() at all, but we clear FMODE_PWRITE here for historical
     * reasons.
     *
     * If a client of seq_files a) implements file.write() and b) wishes to
     * support pwrite() then that client will need to implement its own
     * file.open() which calls seq_open() and then sets FMODE_PWRITE.
     */
    file->f_mode &= ~FMODE_PWRITE;
#endif
    return 0;
}


int rtk_rtl8221b_single_open(struct file *file, int (*show) (struct seq_file *m, void *v), void *data)
{
    struct seq_operations *op = kmalloc(sizeof(*op), GFP_ATOMIC);
    int res = -ENOMEM;

    if (op)
    {
        op->start = rtk_rtl8221b_single_start;
        op->next = rtk_rtl8221b_single_next;
        op->stop = rtk_rtl8221b_single_stop;
        op->show = show;
        res = rtk_rtl8221b_seq_open(file, op);
        if (!res)
            ((struct seq_file *)file->private_data)->private = data;
        else
            kfree(op);
    }
    return res;
}

static int rtk_rtl8221b_nullDebugGet(struct seq_file *s, void *v)
{
    return 0;
}

static int rtk_rtl8221b_nullDebugSingleOpen(struct inode *inode, struct file *file)
{
    return(single_open(file, rtk_rtl8221b_nullDebugGet, NULL));
}

static int rtk_rtl8221b_commonDebugSingleOpen(struct inode *inode, struct file *file)
{
    int i, ret = -1;

    //rtl8221b_spin_lock_bh(rtl8221bSysdb.lock_rtl8221b);
    //========================= Critical Section Start =========================//
    for( i = 0; i < (sizeof(rtl8221bProc) / sizeof(rtk_rtl8221b_proc_t)) ; i++)
    {
        //RTL8221B_MSG("common_single_open inode_id=%u i_ino=%u", rtl8221bProc[i].inode_id,(unsigned int)inode->i_ino);

        if(rtl8221bProc[i].inode_id == (unsigned int)inode->i_ino)
        {
            ret = rtk_rtl8221b_single_open(file, rtl8221bProc[i].get, NULL);
            break;
        }
    }
    //========================= Critical Section End =========================//
    //rtl8221b_spin_unlock_bh(rtl8221bSysdb.lock_rtl8221b);

    return ret;
}

static ssize_t rtk_rtl8221b_commonDebugSingleWrite(struct file * file, const char __user * userbuf,
        size_t count, loff_t * off)
{
    int i, ret = -1;
    char procBuffer[count];
    char *pBuffer = NULL;

    /* write data to the buffer */
    memset(procBuffer, 0, sizeof(procBuffer));
    if (copy_from_user(procBuffer, userbuf, count))
    {
        return -EFAULT;
    }
    procBuffer[count-1] = '\0';
    pBuffer = procBuffer;

    //rtl8221b_spin_lock_bh(rtl8221bSysdb.lock_rtl8221b);
    //========================= Critical Section Start =========================//
    for( i = 0; i < (sizeof(rtl8221bProc) / sizeof(rtk_rtl8221b_proc_t)) ; i++)
    {
        //RTL8221B_MSG("common_single_write inode_id=%u i_ino=%u",rtl8221bProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);

        if(rtl8221bProc[i].inode_id == (unsigned int)file->f_inode->i_ino)
        {
            //if(rtl8221bProc[i].unlockBefortWrite)
            //	rtl8221b_spin_unlock_bh(rtl8221bSysdb.lock_rtl8221b);
            ret = rtl8221bProc[i].set(file, pBuffer, count, off);
            break;
        }
    }
    //========================= Critical Section End =========================//
    //if((i!=(sizeof(rtl8221bProc)/sizeof(rtk_rtl8221b_proc_t))) && !rtl8221bProc[i].unlockBefortWrite)
    //	rtl8221b_spin_unlock_bh(rtl8221bSysdb.lock_rtl8221b);

    return ret;
}

BOOLEAN rtk_rtl8221b_proc_init(void)
{
    int i = 0;
    struct proc_dir_entry *p = NULL;

    rtl8221b_proc_dir = proc_mkdir(PROC_DIR_RTL8221B, NULL);
    if (rtl8221b_proc_dir == NULL)
    {
        RTL8221B_ERROR("create /proc/%s failed!", PROC_DIR_RTL8221B);
        return FAILURE;
    }

    for(i = 0; i < (sizeof(rtl8221bProc)/sizeof(rtk_rtl8221b_proc_t)) ; i++)
    {
        struct proc_dir_entry *parentDir = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
        if(rtl8221bProc[i].get == NULL)
            rtl8221bProc[i].proc_fops.proc_open = rtk_rtl8221b_nullDebugSingleOpen;
        else
            rtl8221bProc[i].proc_fops.proc_open = rtk_rtl8221b_commonDebugSingleOpen;

        if(rtl8221bProc[i].set == NULL)
            rtl8221bProc[i].proc_fops.proc_write = NULL;
        else
            rtl8221bProc[i].proc_fops.proc_write = rtk_rtl8221b_commonDebugSingleWrite;

        rtl8221bProc[i].proc_fops.proc_read = seq_read;
        rtl8221bProc[i].proc_fops.proc_lseek = seq_lseek;
        rtl8221bProc[i].proc_fops.proc_release = single_release;
#else
        if(rtl8221bProc[i].get == NULL)
            rtl8221bProc[i].proc_fops.open = rtk_rtl8221b_nullDebugSingleOpen;
        else
            rtl8221bProc[i].proc_fops.open = rtk_rtl8221b_commonDebugSingleOpen;

        if(rtl8221bProc[i].set == NULL)
            rtl8221bProc[i].proc_fops.write = NULL;
        else
            rtl8221bProc[i].proc_fops.write = rtk_rtl8221b_commonDebugSingleWrite;

        rtl8221bProc[i].proc_fops.read = seq_read;
        rtl8221bProc[i].proc_fops.llseek = seq_lseek;
        rtl8221bProc[i].proc_fops.release = single_release;
#endif

        switch(rtl8221bProc[i].dir)
        {
        case PROC_DIR_RTL8221B_ROOT:
            parentDir = rtl8221b_proc_dir;
            break;
        default:
            break;
        }

        p = proc_create_data(rtl8221bProc[i].name, S_IRUGO, parentDir, &(rtl8221bProc[i].proc_fops),NULL);
        if(!p)
        {
            RTL8221B_ERROR("create proc %s failed!", rtl8221bProc[i].name);
        }
        rtl8221bProc[i].inode_id = p->low_ino;
    }
    RTL8221B_MSG("Creat %d proc entry.", i);

    return SUCCESS;
}


void rtk_rtl8221b_proc_exit(void)
{
    int i = 0;

    for(i = 0 ; i < (sizeof(rtl8221bProc) / sizeof(rtk_rtl8221b_proc_t)) ; i++)
    {
        struct proc_dir_entry *parentDir = NULL;

        switch(rtl8221bProc[i].dir)
        {
        case PROC_DIR_RTL8221B_ROOT:
            parentDir = rtl8221b_proc_dir;
            break;
        default:
            break;
        }

        remove_proc_entry(rtl8221bProc[i].name, parentDir);
    }

    proc_remove(rtl8221b_proc_dir);
}

static struct task_struct *rtl8221b_device_init_task;
#define RTL8221B_DEVICE_INIT_KTHREAD_CPU	0

static int rtk_rtl8221b_device_init_thread (void *data)
{
    BOOL status = FAILURE;
    BOOL singlephy = 1;
    static PHY_LINK_ABILITY phylinkability;

#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
	int i;

	for (i = 0 ; i  < rtl8221b_count ; i++) {
		RTL8221B_DEBUG("Init Device %d - unit %d ; port %d", i, hDevice[i].unit, hDevice[i].port);
		status = Rtl8226b_phy_init(hDevice[i], &phylinkability, singlephy);
		if (status == SUCCESS) {
			RTL8221B_INFO("Init PHY addr%d Done.", hDevice[i].port);

			if ((status = Rtl8226b_serdes_option_set(hDevice[i], DEFAULT_SERDES_MODE /* HiSGMII/SGMII */)) != SUCCESS)
				RTL8221B_ERROR("Device %d unit %u port %u RTL8221b Rtl8226b_serdes_option_set status = 0x%x", i, hDevice[i].unit, hDevice[i].port, status);
			else
				RTL8221B_DEBUG("Device %d unit %u port %u RTL8221b Rtl8226b_serdes_option_set status = 0x%x", i, hDevice[i].unit, hDevice[i].port, status);
		}
		else
			RTL8221B_ERROR("Init PHY addr%d Fail.", hDevice[i].port);
	}
#else

#if (defined(CONFIG_RTL_8221B_DEVICE_0) || defined(CONFIG_RTL_8221B_DEVICE_1))
    //uint8 cfg;
    //char sSdsMode[16];
#endif
#ifdef CONFIG_RTL_8221B_DEVICE_0
    RTL8221B_DEBUG("Init Device 0 (mdio set: %d, phyad: %d)", hDevice0.unit, hDevice0.port);
    status = Rtl8226b_phy_init(hDevice0, &phylinkability,singlephy);
    if (status == SUCCESS)
    {
        RTL8221B_INFO("Init Device 0 (addr%d) Done.", hDevice0.port);
#if 0
        rtk_port_serdesMode_get(DEVICE0_SERDES_NUMBER, &cfg);
        switch(cfg)
        {
        case LAN_SDS_MODE_GE_FE_PHY:
            strncpy(sSdsMode, "GE_FE_PHY\0", 10);
            break;
        case LAN_SDS_MODE_FIBER_1G:
            strncpy(sSdsMode, "FIBER_1G\0", 9);
            break;
        case LAN_SDS_MODE_SGMII_PHY:
            strncpy(sSdsMode, "SGMII_PHY\0", 10);
            break;
        case LAN_SDS_MODE_SGMII_MAC:
            strncpy(sSdsMode, "SGMII_MAC\0", 10);
            break;
        case LAN_SDS_MODE_HSGMII_PHY:
            strncpy(sSdsMode, "HSGMII_PHY\0", 11);
            break;
        case LAN_SDS_MODE_HSGMII_MAC:
            strncpy(sSdsMode, "HSGMII_MAC\0", 11);
            break;
        case LAN_SDS_MODE_2500BASEX_PHY:
            strncpy(sSdsMode, "2500BASEX_PHY\0", 14);
            break;
        case LAN_SDS_MODE_2500BASEX_MAC:
            strncpy(sSdsMode, "2500BASEX_MAC\0", 14);
            break;
        case LAN_SDS_MODE_END:
        default:
            strncpy(sSdsMode, "UNKNOWN Mode\0", 13);
            break;
        }
        RTL8221B_INFO("Device 0 Serdes Mode: %s(%d)", sSdsMode, cfg);
#endif
        if ((status = Rtl8226b_serdes_option_set(hDevice0, DEFAULT_SERDES_MODE /* HiSGMII/SGMII */)) != SUCCESS)
            RTL8221B_ERROR("Device0 unit %u port %u RTL8221b Rtl8226b_serdes_option_set status = 0x%x", hDevice0.unit, hDevice0.port, status);
        else
            RTL8221B_DEBUG("Device0 unit %u port %u RTL8221b Rtl8226b_serdes_option_set(%d) status = 0x%x", hDevice0.unit, hDevice0.port, DEFAULT_SERDES_MODE, status);

    }
    else
        RTL8221B_ERROR("Init Device 0 (addr%d) Fail.", hDevice0.port);
#endif
#ifdef CONFIG_RTL_8221B_DEVICE_1
    RTL8221B_DEBUG("Init Device 1 (mdio set: %d, phyad: %d)", hDevice1.unit, hDevice1.port);
    status = Rtl8226b_phy_init(hDevice1, &phylinkability,singlephy);
    if (status == SUCCESS)
    {
        RTL8221B_INFO("Init Device 1 (addr%d) Done.", hDevice1.port);
#if 0
        rtk_port_serdesMode_get(DEVICE1_SERDES_NUMBER, &cfg);
        switch(cfg)
        {
        case LAN_SDS_MODE_GE_FE_PHY:
            strncpy(sSdsMode, "GE_FE_PHY\0", 10);
            break;
        case LAN_SDS_MODE_FIBER_1G:
            strncpy(sSdsMode, "FIBER_1G\0", 9);
            break;
        case LAN_SDS_MODE_SGMII_PHY:
            strncpy(sSdsMode, "SGMII_PHY\0", 10);
            break;
        case LAN_SDS_MODE_SGMII_MAC:
            strncpy(sSdsMode, "SGMII_MAC\0", 10);
            break;
        case LAN_SDS_MODE_HSGMII_PHY:
            strncpy(sSdsMode, "HSGMII_PHY\0", 11);
            break;
        case LAN_SDS_MODE_HSGMII_MAC:
            strncpy(sSdsMode, "HSGMII_MAC\0", 11);
            break;
        case LAN_SDS_MODE_2500BASEX_PHY:
            strncpy(sSdsMode, "2500BASEX_PHY\0", 14);
            break;
        case LAN_SDS_MODE_2500BASEX_MAC:
            strncpy(sSdsMode, "2500BASEX_MAC\0", 14);
            break;
        case LAN_SDS_MODE_END:
        default:
            strncpy(sSdsMode, "UNKNOWN Mode\0", 13);
            break;
        }
        RTL8221B_INFO("Device 1 Serdes Mode: %s(%d)", sSdsMode, cfg);
#endif
        if ((status = Rtl8226b_serdes_option_set(hDevice1, DEFAULT_SERDES_MODE /* HiSGMII/SGMII */)) != SUCCESS)
            RTL8221B_ERROR("Device1 unit %u port %u RTL8221b Rtl8226b_serdes_option_set status = 0x%x", hDevice1.unit, hDevice1.port, status);
        else
            RTL8221B_DEBUG("Device1 unit %u port %u RTL8221b Rtl8226b_serdes_option_set status = 0x%x", hDevice1.unit, hDevice1.port, status);
    }
    else
        RTL8221B_ERROR("Init Device 1 (addr%d) Fail.", hDevice1.port);
#endif

#endif	/* CONFIG_RTK_EXT_GPHY */
    return 0;
}

#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)

u32 __eth_phy_rtl8226b_add_phy(u32 port, u8 phy_addr) {
	RTL8221B_DEBUG("%s(%d): port = %d, phy_dev = %d", __FUNCTION__, __LINE__, port, phy_addr);
	if (rtl8221b_count < RTK_EXTGPHY_RTL8221B_MAX) {
		hDevice[rtl8221b_count].unit = CONFIG_RTL_8221B_DEVICE_0_MDIO_SET;
		hDevice[rtl8221b_count].port = phy_addr;
#ifndef CONFIG_LUNA_G3_SERIES
		if(CONFIG_RTL_8221B_DEVICE_0_MDIO_SET == 0) {
			hDevice[rtl8221b_count].gpio_mdc = RTL9607C_SET0_MDC_PIN;
			hDevice[rtl8221b_count].gpio_mdio = RTL9607C_SET0_MDIO_PIN;
		}
		else if(CONFIG_RTL_8221B_DEVICE_0_MDIO_SET == 1) {
			hDevice[rtl8221b_count].gpio_mdc = RTL9607C_SET1_MDC_PIN;
			hDevice[rtl8221b_count].gpio_mdio = RTL9607C_SET1_MDIO_PIN;
		}
#endif
		rtl8221b_count++;
	}
	else
		RTL8221B_ERROR("Reach Support rtl8221B number for port  %u ; phyAddr %u", port, phy_addr);

	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8226b_add_phy);

u32 __eth_phy_rtl8226b_init(u8 phy_dev) {
	int i;
	RTL8221B_DEBUG("%s(%d)", __FUNCTION__, __LINE__);

	if (rtl8221b_init) {
		RTL8221B_MSG("All RTL8221b are init.");
		return 1;
	}

	for (i = 0 ; i  < rtl8221b_count ; i++)
		RTL8221B_DEBUG("#%d: unit %d ; port %d", i, hDevice[i].unit, hDevice[i].port);
#if 0
    RTL8221B_MSG("Init Device 0 (mdio set: %d, phyad: %d)", hDevice0.unit, hDevice0.port);
    status = Rtl8221b_phy_init(hDevice0, &phylinkability,singlephy);
    if (status == SUCCESS)
        RTL8221B_INFO("Init Device 0 done.");
    else
        RTL8221B_ERROR("Init Device 0 fail.");

    RTL8221B_MSG("Init Device 1 (mdio set: %d, phyad: %d)", hDevice1.unit, hDevice1.port);
    status = Rtl8221b_phy_init(hDevice1, &phylinkability,singlephy);
    if (status == SUCCESS)
        RTL8221B_INFO("Init Device 1 done.");
    else
        RTL8221B_ERROR("Init Device 1 fail.");
#else
	/* Create rtl8221b phy init kthread */
	rtl8221b_device_init_task = kthread_create(rtk_rtl8221b_device_init_thread, NULL, "rtl8221b_device_init/%d", RTL8221B_DEVICE_INIT_KTHREAD_CPU);
	if (WARN_ON(!rtl8221b_device_init_task))
	{
	    RTL8221B_ERROR("Create rtl8221b_device1_init/%d failed!", RTL8221B_DEVICE_INIT_KTHREAD_CPU);
	}
	kthread_bind(rtl8221b_device_init_task, RTL8221B_DEVICE_INIT_KTHREAD_CPU);
	wake_up_process(rtl8221b_device_init_task);
#endif
	rtl8221b_init = 1;
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8226b_init);

u32 get_hDevice_index(u8 phy_addr) {
	int i;
	for (i = 0 ; i  < rtl8221b_count ; i++)
		if (phy_addr == hDevice[i].port)	return i;

	RTL8221B_ERROR("phy_addr %d is not found in hDevice", phy_addr);
	return 99;	/* invalid value */
}

u32 __eth_phy_rtl8226b_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status) {
	link_status->link_up = 0;
	UINT16 speed;
	BOOL enable;

	if (Rtl8226b_is_link(hDevice[get_hDevice_index(phy_addr)], &link_status->link_up) != SUCCESS)
	        RTL8221B_ERROR("Rtl8226b_is_link(phyAddr %d) Fail", phy_addr);
	else {
		RTL8221B_DEBUG("phy_addr %d link : %s",phy_addr ,link_status->link_up ? "UP" : "DOWN");
		if (Rtl8226b_speed_get(hDevice[get_hDevice_index(phy_addr)], &speed) == SUCCESS) {
			switch (speed) {
			case LINK_SPEED_10M:
				link_status->speed = EXT_PHY_SPEED_10;
				break;
			case LINK_SPEED_100M:
				link_status->speed = EXT_PHY_SPEED_100;
				break;
			case LINK_SPEED_500M:
				link_status->speed = EXT_PHY_SPEED_500;
				break;
			case LINK_SPEED_1G:
				link_status->speed = EXT_PHY_SPEED_1000;
				break;
			case LINK_SPEED_2P5G:
				link_status->speed = EXT_PHY_SPEED_2500;
				break;
			case NO_LINK:
			default:
				break;
			}
			//RTL8221B_DEBUG("Rtl8226b_speed_get(phyAddr %d): %d [%s]", hDevice0.port, speed, sDesc);
		}

		if (Rtl8226b_duplex_get(hDevice[get_hDevice_index(phy_addr)], &enable) == SUCCESS)
			link_status->duplex = enable;
	}
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8226b_link_status_get);

u32 __eth_phy_rtl8226b_flow_ctrl_get(u8 phy_addr, u8 *tx_fc, u8 *rx_fc)
{
	Rtl8226b_fc_sts_get(hDevice[get_hDevice_index(phy_addr)], tx_fc, rx_fc);
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8226b_flow_ctrl_get);

#endif


unsigned int io_mode_en_value_backup;

static int __init rtk_rtl8221b_moudle_init(void)
{
#ifdef CONFIG_LUNA_G3_SERIES
    int ret;
#else
    int gpio;
#endif

    RTL8221B_MSG("%s", __FUNCTION__);
    hDevice0.unit = INVALID_VALUE;
    hDevice0.port = INVALID_VALUE;
    hDevice0.gpio_mdc = INVALID_VALUE;
    hDevice0.gpio_mdio = INVALID_VALUE;
    hDevice1.unit = INVALID_VALUE;
    hDevice1.port = INVALID_VALUE;
    hDevice1.gpio_mdc = INVALID_VALUE;
    hDevice1.gpio_mdio = INVALID_VALUE;

    //BOOL status = FAILURE;
    //BOOL singlephy = 1;
    //static PHY_LINK_ABILITY phylinkability;

#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
    RTL8221B_MSG("Phys detecttion will be done by extGphy");
#else

#if (!defined(CONFIG_RTL_8221B_DEVICE_0) && !defined(CONFIG_RTL_8221B_DEVICE_1))
    RTL8221B_ERROR("No RTL8221B Device Found !!!");
    return -ENODEV;
#endif

#ifdef CONFIG_RTL_8221B_DEVICE_0
    hDevice0.unit = CONFIG_RTL_8221B_DEVICE_0_MDIO_SET;
    hDevice0.port = CONFIG_RTL_8221B_DEVICE_0_PHY_ADDR;
    RTL8221B_MSG("RTL8221B Device 0 (mdio set: %d, phyad: %d)", hDevice0.unit, hDevice0.port);
#endif

#ifdef CONFIG_RTL_8221B_DEVICE_1
    hDevice1.unit = CONFIG_RTL_8221B_DEVICE_1_MDIO_SET;
    hDevice1.port = CONFIG_RTL_8221B_DEVICE_1_PHY_ADDR;
    RTL8221B_MSG("RTL8221B Device 1 (mdio set: %d, phyad: %d)", hDevice1.unit, hDevice1.port);
#endif

#if (defined(CONFIG_RTL_8221B_DEVICE_0) && defined(CONFIG_RTL_8221B_DEVICE_1))
    if ((hDevice0.unit == hDevice1.unit) && (hDevice0.port == hDevice1.port))
    {
        RTL8221B_ERROR("RTL8221B Device 0 and Device 1 are using same MDIO Set (%d) with Same PHY Add (%d)", hDevice0.unit, hDevice0.port);
        return -EINVAL;
    }
#endif

#endif

#ifdef CONFIG_LUNA_G3_SERIES
    /* Reset RTL8221B */
    ret = gpio_is_valid(RTL8221B_RESET_GPIO_NO);
    if (ret != 1)
    {
        RTL8221B_ERROR("GPIO #%d is NOT Valid !!! (ret = %d)", RTL8221B_RESET_GPIO_NO, ret);
        return -EINVAL;
    }
    ret = gpio_request(RTL8221B_RESET_GPIO_NO, RTL8221B_RESET_GPIO_LABEL);
    if (ret < 0)
    {
        RTL8221B_ERROR("Request GPIO #%d Error !!! (ret = %d)", RTL8221B_RESET_GPIO_NO, ret);
        return -EINVAL;
    }
    ret = gpio_direction_output(RTL8221B_RESET_GPIO_NO, 1);
    if (ret != 0)
    {
        RTL8221B_ERROR("Set GPIO #%d Output as 1 Error !!! (ret = %d)", RTL8221B_RESET_GPIO_NO, ret);
        return -EINVAL;
    }
    //gpio_set_value(RTL8221B_RESET_GPIO_NO, 0);
    //mdelay(200);
    //gpio_set_value(RTL8221B_RESET_GPIO_NO, 1);
    gpio_free(RTL8221B_RESET_GPIO_NO);
    mdelay(300);		/* Waitting rtl8221b reset ??? */

#else
    /* MDIO init. */
    io_mode_en_value_backup = REG32(IO_MODE_EN_REG);
#ifdef CONFIG_RTL_8221B_DEVICE_0

    gpio = rtk_8221b_reset_gpio_get(0);
    if (gpio < 0)
        gpio = RTL8221B_DEV0_RESET_PIN;

    rtk_gpio_state_set(gpio, ENABLED);
    rtk_gpio_mode_set(gpio, GPIO_OUTPUT);
    rtk_gpio_databit_set(gpio, 0);
    mdelay(50);
    rtk_gpio_databit_set(gpio, 1);
    mdelay(100);

    if(CONFIG_RTL_8221B_DEVICE_0_MDIO_SET == 0)
    {
        RTL8221B_INFO("Enable MDIO master PAD_LED0/PAD_LED1 for 8221B device 0......");
        hDevice0.gpio_mdc = RTL9607C_SET0_MDC_PIN;
        hDevice0.gpio_mdio = RTL9607C_SET0_MDIO_PIN;
        REG32(IO_MODE_EN_REG) |= MDIO_MASTER_EN;
    }
    else if(CONFIG_RTL_8221B_DEVICE_0_MDIO_SET == 1)
    {
        RTL8221B_INFO("Enable MDIO master PAD_LED4/PAD_LED5 for 8221B device 0......");
        hDevice0.gpio_mdc = RTL9607C_SET1_MDC_PIN;
        hDevice0.gpio_mdio = RTL9607C_SET1_MDIO_PIN;
        REG32(IO_MODE_EN_REG) |= EXT_MDX_M_EN;
    }
    rtk_gpio_state_set(hDevice0.gpio_mdc, DISABLED);
    rtk_gpio_state_set(hDevice0.gpio_mdio, DISABLED);
    //REG32(IO_GPIO_EN_REG + ((RTL8221B_MDC_SW_GPIO_NO >> 5) << 2)) &= ~(1 << (RTL8221B_MDC_SW_GPIO_NO % 32));
#endif

#ifdef CONFIG_RTL_8221B_DEVICE_1
    if(CONFIG_RTL_8221B_DEVICE_1_MDIO_SET == 0)
    {
        RTL8221B_INFO("Enable MDIO master PAD_LED0/PAD_LED1 for 8221B device 1......");
        hDevice1.gpio_mdc = RTL9607C_SET0_MDC_PIN;
        hDevice1.gpio_mdio = RTL9607C_SET0_MDIO_PIN;
        REG32(IO_MODE_EN_REG) |= MDIO_MASTER_EN;
    }
    else if(CONFIG_RTL_8221B_DEVICE_1_MDIO_SET == 1)
    {
        RTL8221B_INFO("Enable MDIO master PAD_LED4/PAD_LED5 for 8221B device 1......");
        hDevice1.gpio_mdc = RTL9607C_SET1_MDC_PIN;
        hDevice1.gpio_mdio = RTL9607C_SET1_MDIO_PIN;
        REG32(IO_MODE_EN_REG) |= EXT_MDX_M_EN;
    }
    rtk_gpio_state_set(hDevice1.gpio_mdc, DISABLED);
    rtk_gpio_state_set(hDevice1.gpio_mdio, DISABLED);
#endif
    rtk_mdio_init();
    //init_mdio_lock();
#endif

#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
        RTL8221B_MSG("Phys init. will be done by extGphy");
#else

#if 0
    RTL8221B_MSG("Init Device 0 (mdio set: %d, phyad: %d)", hDevice0.unit, hDevice0.port);
    status = Rtl8221b_phy_init(hDevice0, &phylinkability,singlephy);
    if (status == SUCCESS)
        RTL8221B_INFO("Init Device 0 done.");
    else
        RTL8221B_ERROR("Init Device 0 fail.");

    RTL8221B_MSG("Init Device 1 (mdio set: %d, phyad: %d)", hDevice1.unit, hDevice1.port);
    status = Rtl8221b_phy_init(hDevice1, &phylinkability,singlephy);
    if (status == SUCCESS)
        RTL8221B_INFO("Init Device 1 done.");
    else
        RTL8221B_ERROR("Init Device 1 fail.");
#else
    /* Create rtl8221b phy init kthread */
    rtl8221b_device_init_task = kthread_create(rtk_rtl8221b_device_init_thread, NULL, "rtl8221b_device_init/%d", RTL8221B_DEVICE_INIT_KTHREAD_CPU);
    if (WARN_ON(!rtl8221b_device_init_task))
    {
        RTL8221B_ERROR("Create rtl8221b_device1_init/%d failed!", RTL8221B_DEVICE_INIT_KTHREAD_CPU);
    }
    kthread_bind(rtl8221b_device_init_task, RTL8221B_DEVICE_INIT_KTHREAD_CPU);
    wake_up_process(rtl8221b_device_init_task);
#endif

#endif

    rtk_rtl8221b_proc_init();

    return 0;
}

static void __exit rtk_rtl8221b_module_exit(void)
{
#ifndef CONFIG_LUNA_G3_SERIES
    REG32(IO_MODE_EN_REG)		= io_mode_en_value_backup;
#endif
    rtk_rtl8221b_proc_exit();

    RTL8221B_MSG("%s\n", __FUNCTION__);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTL8221B Module");
MODULE_AUTHOR("Realtek.com");

module_init(rtk_rtl8221b_moudle_init);
module_exit(rtk_rtl8221b_module_exit);

