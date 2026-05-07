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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>	/* misc_register */
//#include <fs/proc/internal.h>
#include <common/rt_error.h>
#include <rtk_ext_gphy.h>

#if !defined(CONFIG_LAN_SDS_FEATURE) && !defined(CONFIG_LAN_SDS1_FEATURE)
#define RTK_EXT_GPHY_SET_NI_SDS_MODE	(1)
#endif

#define MDIO_ST_CODE_C22		(1)
#define MDIO_ST_CODE_C45		(0)
#define DEFAULT_MDIO_DEVICE_SET_ID	(0)
#define MAX_EXTERNAL_GPHYS_NUMBER	(6)

/*Organizationally Unique Identifier (OUI) */
#define PHY_OUI_BCM5461            0x000818
#define PHY_OUI_BCM54612           0x00d897
#define PHY_OUI_RTK                0x000732
#define PHY_OUI_VTS                0x0001c1
#define PHY_OUI_AR                 0x001374
#define PHY_OUI_MVL                0x000ac2
#define PHY_OUI_AQR_G2             0x00e86d
#define PHY_OUI_AQR_G4             0x0070c7
#define PHY_OUI_INTL               0x00f277

/*Manufacturer's Model Number  */
#define PHY_MODEL_BCM5461          0x000c
#define PHY_MODEL_BCM54612         0x0026
#define PHY_MODEL_RTL8211E         0x0010
#define PHY_MODEL_RTL8211          0x0011
#define PHY_MODEL_RTL8211_ASIC     0x0018
#define PHY_MODEL_RTL8201F         0x0001
#define PHY_MODEL_RTL8214          0x0026
#define PHY_MODEL_RTL8214C         0x0014
#define PHY_MODEL_RTL8226B         0x0004
#define PHY_MODEL_RTL8261I         0x002f
#define PHY_MODEL_VTS8641          0x0003
#define PHY_MODEL_AR8305           0x0007
#define PHY_MODEL_RTL8198F         0x0008
#define PHY_MODEL_MVL88X3310       0x001a
#define PHY_MODEL_AQR107           0x000e
#define PHY_MODEL_AQR113           0x0001
#define PHY_MODEL_INTL212B         0x0020
#define PHY_MODEL_INTL241          0x0000

/**************************************************************/
/*       Etherent PHY MII Standard Registers definition       */
/**************************************************************/

#define PHY_REG_CTRL                             (0)
#define _PHY_REG_STATUS                           (1)
#define PHY_REG_ID_1                             (2)
#define PHY_REG_ID_2                             (3)
#define PHY_REG_AUTONEG_ADV                      (4)
#define PHY_REG_LP_ABILITY_BASE                  (5)
#define PHY_REG_AN_EXT                           (6)
#define PHY_REG_AUTONEG_NEXT_PAGE                (7)
#define PHY_REG_LP_NEXT_PAGE                     (8)
#define PHY_REG_1000BASET_CTRL                   (9)
#define PHY_REG_1000BASET_STATUS                 (10)
#define PHY_REG_RSVD1                            (11)
#define PHY_REG_RSVD2                            (12)
#define PHY_REG_RSVD3                            (13)
#define PHY_REG_RSVD4                            (14)
#define PHY_REG_1000BSET_STATUS_EXT              (15)
#define PHY_REG_SPECIFIC_STATUS                  (17)

/* Control register bits */
#define PHY_REG_CTRL_BIT_SPEED_SEL_MSB           (6)
#define PHY_REG_CTRL_BIT_COLLISTION_TEST         (7)
#define PHY_REG_CTRL_BIT_DUPLEX                  (8)
#define PHY_REG_CTRL_BIT_AN_RESTART              (9)
#define PHY_REG_CTRL_BIT_ISOLATE                 (10)
#define PHY_REG_CTRL_BIT_PWR_DOWN                (11)
#define PHY_REG_CTRL_BIT_AN_CTRL                 (12)
#define PHY_REG_CTRL_BIT_SPEED_SEL_LSB           (13)
#define PHY_REG_CTRL_BIT_LPBK                    (14)
#define PHY_REG_CTRL_BIT_RESET                   (15)


/* Status register bits */
#define PHY_REG_STATUS_BIT_EXT_CAP               (0)
#define PHY_REG_STATUS_BIT_JBRD                  (1)
#define PHY_REG_STATUS_BIT_LNK                   (2)
#define PHY_REG_STATUS_BIT_AN_CAPABLE            (3)
#define PHY_REG_STATUS_BIT_RMT_FAULT             (4)
#define PHY_REG_STATUS_BIT_AN_DONE               (5)
#define PHY_REG_STATUS_BIT_EXT_STATUS            (8)
#define PHY_REG_STATUS_BIT_100BT2_HD_SUPP        (9)
#define PHY_REG_STATUS_BIT_100BT2_FD_SUPP        (10)
#define PHY_REG_STATUS_BIT_10_HD_SUPP            (11)
#define PHY_REG_STATUS_BIT_10_FD_SUPP            (12)
#define PHY_REG_STATUS_BIT_100BX_HD_SUPP         (13)
#define PHY_REG_STATUS_BIT_100BX_FD_SUPP         (14)
#define PHY_REG_STATUS_BIT_100BT4_SUPP           (15)

/* Advertisement control register bits */
#define PHY_REG_AN_ADVR_BIT_CSMA                 (0)
#define PHY_REG_AN_ADVR_BIT_10BT_HD              (5)
#define PHY_REG_AN_ADVR_BIT_10BT_FD              (6)
#define PHY_REG_AN_ADVR_BIT_100BX_HD             (7)
#define PHY_REG_AN_ADVR_BIT_100BX_FD             (8)
#define PHY_REG_AN_ADVR_BIT_100BT4_CAP           (9)
#define PHY_REG_AN_ADVR_BIT_PAUSE_CAP            (10)
#define PHY_REG_AN_ADVR_BIT_PAUSE_ASYM           (11)
#define PHY_REG_AN_ADVR_BIT_RMT_FAULT            (13)
#define PHY_REG_AN_ADVR_BIT_NPAGE                (15)

/* Link partner ability register bits */
#define PHY_REG_LP_ABLITY_BIT_10BT_HD            (5)
#define PHY_REG_LP_ABLITY_BIT_10BT_FD            (6)
#define PHY_REG_LP_ABLITY_BIT_100BX_HD           (7)
#define PHY_REG_LP_ABLITY_BIT_100BX_FD           (8)
#define PHY_REG_LP_ABLITY_BIT_100BT4_CAP         (9)
#define PHY_REG_LP_ABLITY_BIT_PAUSE_CAP          (10)
#define PHY_REG_LP_ABLITY_BIT_PAUSE_ASYM         (11)
#define PHY_REG_LP_ABLITY_BIT_RESV               (12)
#define PHY_REG_LP_ABLITY_BIT_RMT_FAULT          (13)
#define PHY_REG_LP_ABLITY_BIT_LP_ACK             (14)
#define PHY_REG_LP_ABLITY_BIT_NP                 (15)

/* Auto-negotiation Expansion register bits */
 #define PHY_REG_AN_EXT_BIT_LP_AN_CAP            (0)
 #define PHY_REG_AN_EXT_BIT_LP_NP_RX             (1)
 #define PHY_REG_AN_EXT_BIT_NP_CAP               (2)
 #define PHY_REG_AN_EXT_BIT_LP_NP_CAP            (3)
 #define PHY_REG_AN_EXT_BIT_P_FAULT              (4)

/* 1000BASE-T Control register bits */
#define PHY_REG_1000BT_CTRL_BIT_ADVR_1000_HD     (8)
#define PHY_REG_1000BT_CTRL_BIT_ADVR_1000_FD     (9)
#define PHY_REG_1000BT_CTRL_BIT_MASTER_CFG       (10)
#define PHY_REG_1000BT_CTRL_BIT_MMODE_CFG_ENBL   (11)

/* 1000BASE-T Status register bits */
#define PHY_REG_1000BT_STATUS_BIT_LP_1000_HD     (10)
#define PHY_REG_1000BT_STATUS_BIT_LP_1000_FD     (11)
#define PHY_REG_1000BT_STATUS_BIT_RMTRX_OK       (12)
#define PHY_REG_1000BT_STATUS_BIT_LCLRX_OK       (13)

/* Interrupts Definitions */
#define PHY_INTR_LINK_CHANGE           0x0001 /* Link Change Interrupt   */
#define PHY_INTR_SPEED_CHANGE          0x0002 /* Speed Change Interrupt  */
#define PHY_INTR_DUPLEX_CHANGE         0x0004 /* Duplex Change Interrupt */
#define PHY_INTR_AN_DONE               0x0008 /* AN Done Interrupt       */
#define PHY_INTR_AN_FAIL               0x0010 /* AN Fail Interrupt       */

//Register 13================================================================
//  Bits   |  R/W  | Default |   DESC
//---------+-------+---------+----------------------------------------------
//[15:14]  |   WO  |00       | Function Code, detail is as following:
//         |       |         | 00: Address
//         |       |         | 01: Data with no post increment
//         |       |         | 10: Data with post increment on reads and writes
//         |       |         | 11: Data with post increment on writes only
//[13:05]  | RO    |000000000|  Reserved.
//[04:00]  | WO    |0        |  Device Address.
//============================================================================
#define PHY_REG_MACR          13 // for MMD access
#define PHY_REG_MAADR         14 // for MMD content W/R

//Register 14=================================================================
//  Bits   |  R/W  | Default   |   DESC
//---------+-------+-----------+----------------------------------------------
//[15:0]   | RW    | 0         | -If MMD Function == 0, means register address
//         |       |           | -If MMD Function != 0, means register data
//============================================================================

#define MMD_FUNC_ADDR          0
#define MMD_FUNC_DATA_NO_INC   1
#define MMD_FUNC_DATA_INC_RW   2
#define MMD_FUNC_DATA_INC_WO   3
#define MAKE_MMD(fcode, mmddev) ((((fcode)&0x3)<<14)|((mmddev) &0x1f))

#define ETH_PHY_NUM_MAX		(32)
#ifdef CONFIG_FC_RTL8277C_SERIES
#endif

#ifdef CONFIG_LUNA_G3_SERIES

#include <ca_event.h>
#include <aal_phy.h>
#include <aal_sds.h>

#define phy_reg_GBCR_t		aal_phy_reg_GBCR_t
#define phy_reg_GBSR_t		aal_phy_reg_GBSR_t
#define phy_reg_ANAR_t		aal_phy_reg_ANAR_t
#define phy_reg_ANLPAR_t	aal_phy_reg_ANLPAR_t

#define event_port_link_t	ca_event_port_link_t

/*
 * Default Serdes Mode (write bit [5:0] = 0/1/2/3  SDS mode)
 * 0: 2500Base-X/SGMII
 * 1: HiSGMII/SGMII
 * 2: 2500Base-X
 * 3: HiSGMII
 */
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

#define RTL9607C_SET0_MDC_PIN       6
#define RTL9607C_SET0_MDIO_PIN      7
#define RTL9607C_SET1_MDC_PIN       12
#define RTL9607C_SET1_MDIO_PIN      10

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
#define DEVICE0_SERDES_NUMBER		0
#define DEVICE1_SERDES_NUMBER		1

#endif

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

unsigned int ext_gphy_debug=0;

int RTK_MmdPhyRead (
	uint16 device_id,
	uint8 st_code,
	uint8 phy_addr,
	uint16 dev,
	uint16 addr,
	uint16 *data)
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

	ret = aal_mdio_read(device_id, st_code, phy_addr, dev, addr, data);
	if (ret != CA_E_OK) {
		RTKEXTGPHY_ERROR("MDIO READ(dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (FAILED)", dev, addr, *data, ret);
		return 1;
	}
#else
	int32 ret;

	//rtk_gpio_state_set(hDevice.gpio_mdc, DISABLED);
	//rtk_gpio_state_set(hDevice.gpio_mdio, DISABLED);
	//spin_lock_bh(&lock_mdio);
	if (st_code == MDIO_ST_CODE_C22) {
		rtk_mdio_cfg_set(device_id, DEFAULT_MDIO_PORT_NUM, phy_addr, RTK_MDIO_FMT_C22);
		ret = rtk_mdio_c22_read((uint8)addr, data);
	}
	else if (st_code == MDIO_ST_CODE_C45) {
		rtk_mdio_cfg_set(device_id, DEFAULT_MDIO_PORT_NUM, phy_addr, RTK_MDIO_FMT_C45);
		ret = rtk_mdio_c45_read((uint8)dev, addr, data);
	}
	else {
		RTKEXTGPHY_ERROR("Wrong st_code = %d", st_code);
		return 1;
	}

	if (ret == RT_ERR_FAILED) {
		RTKEXTGPHY_ERROR("MDIO READ(dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (FAILED)", dev, addr, *data, ret);
		return 1;
	}
	//spin_unlock_bh(&lock_mdio);
#endif
	RTKEXTGPHY_DEBUG("MDIO READ(dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (OK)", dev, addr, *data, ret);

	return 0;
}

int RTK_MmdPhyWrite (
    uint16 device_id,
    uint8 st_code,
    uint8 phy_addr,
    uint16 dev,
    uint16 addr,
    uint16 data)
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

	ret = aal_mdio_write(device_id, st_code, phy_addr, dev, addr, data);
	if (ret != CA_E_OK) {
		RTKEXTGPHY_ERROR("MDIO WRITE(dev = %d, addr = 0x%04x, data = 0x%04x): - ret = %d (FAILED)", dev, addr, data, ret);
		return 1;
	}
#else
	int32   ret;

	//rtk_gpio_state_set(hDevice.gpio_mdc, DISABLED);
	//rtk_gpio_state_set(hDevice.gpio_mdio, DISABLED);
	//spin_lock_bh(&lock_mdio);
	if (st_code == MDIO_ST_CODE_C22) {
		rtk_mdio_cfg_set(device_id, DEFAULT_MDIO_PORT_NUM, phy_addr, RTK_MDIO_FMT_C22);
		ret = rtk_mdio_c22_write((uint8)addr, data);
	}
	else if (st_code == MDIO_ST_CODE_C45) {
		rtk_mdio_cfg_set(device_id, DEFAULT_MDIO_PORT_NUM, phy_addr, RTK_MDIO_FMT_C45);
		ret = rtk_mdio_c45_write((uint8)dev, addr, data);
	}
	else {
		RTKEXTGPHY_ERROR("Wrong st_code = %d", st_code);
		return 1;
	}

	if (ret == RT_ERR_FAILED) {
		RTKEXTGPHY_ERROR("MDIO WRITE(dev = %d, addr = 0x%04x, data = 0x%04x): - ret = %d (FAILED)", dev, addr, data, ret);
		return 1;
	}
	//spin_unlock_bh(&lock_mdio);
#endif
	RTKEXTGPHY_DEBUG("MDIO WRITE(dev = %d, addr = 0x%04x): data = 0x%04x - ret = %d (OK)", dev, addr, data, ret);

	return 0;
}

/**************************************************************/
/*       Internal Etherent PHY register access functions      */
/**************************************************************/
#define PHY_REG_MAX_LEN            16

int rtk_eth_phy_regfield_read (
	uint8   phy_dev,
	uint8   reg,
	uint8   offset,
	uint8   length,
	uint16 *p_val16 )
{
	int ret = 0;
	uint16 data = 0;

	if (length == 0
		|| offset >= PHY_REG_MAX_LEN
		|| length > (PHY_REG_MAX_LEN - offset)
		|| !p_val16)
		return 1;

	ret = RTK_MmdPhyRead(0, 1, phy_dev, reg, 0, &data);
	if (ret != 0)	goto end;

	if ((0 == offset) && (16 == length))	*p_val16 = data & 0xffff;
	else					*p_val16 = (data >> offset) & ((0x1 << length) -1);

end:
	return ret;
}

int rtk_eth_phy_regfield_write (
	uint8   phy_dev,
	uint8   reg,
	uint8   offset,
	uint8   length,
	uint16  val16 )
{
	int ret = 0;
	uint16 data = 0;
	uint16 data16 = 0;

	if (length == 0
		|| offset >= PHY_REG_MAX_LEN
		|| length > (PHY_REG_MAX_LEN - offset))
		return -1;//CA_E_PARAM;

	ret = RTK_MmdPhyRead(0, 1, phy_dev, reg, 0, &data);
	if (ret != 0x0 )	goto end;

	data16 = (data & ~(((0x1 << length) -1) << offset)) | (val16 << offset);

	if ((data & 0xffff) == data16)	goto end;

	ret = RTK_MmdPhyWrite(0, 1, phy_dev, reg, 0, data16);
	if (ret != 0x0 )	goto end;

end:
	return ret;

}

#if 0
ca_status_t __eth_phy_regfield_mask_write
(
    ca_uint8_t   phy_dev,
    ca_uint8_t   reg,
    ca_uint16_t  mask,
    ca_uint16_t  val16
)
{
    ca_status_t ret = CA_E_OK;
    ca_uint16_t data = 0;
    ca_uint16_t data16 = val16;

    if (mask != 0xffff) {
        ret = aal_mdio_read(0, 1, phy_dev, reg, 0, &data);
        if (ret != CA_E_OK ) {
            goto end;
        }

        data &= ~mask;  /*mask the bits want to be overwrited */
        data16 = (data & 0xffff) | (val16 & mask); /*overwrite the data */
    }

    ret = aal_mdio_write(0, 1, phy_dev, reg, 0, data16);
    if (ret != CA_E_OK ) {
        goto end;
    }

end:

    return ret;
}

static ca_status_t __eth_phy_mmd_read
(
    ca_uint8_t   phy_dev,
    ca_uint8_t   mmd_dev,
    ca_uint8_t   reg,
    ca_uint16_t *p_val16
)
{
    ca_status_t ret    = CA_E_OK;

    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MACR, 0, MAKE_MMD(MMD_FUNC_ADDR, mmd_dev));
    if (ret != CA_E_OK) {
        goto end;
    }
    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MAADR, 0, reg);
    if (ret != CA_E_OK) {
        goto end;
    }
    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MACR, 0, MAKE_MMD(MMD_FUNC_DATA_NO_INC, mmd_dev));
    if (ret != CA_E_OK) {
        goto end;
    }
    ret = aal_mdio_read(0, 1, phy_dev, PHY_REG_MAADR, 0, p_val16);


end:
    return ret;

}

static ca_status_t __eth_phy_mmd_write
(
    ca_uint8_t   phy_dev,
    ca_uint8_t   mmd_dev,
    ca_uint8_t   reg,
    ca_uint16_t  val16
)
{


    ca_status_t ret = CA_E_OK;

    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MACR, 0, MAKE_MMD(MMD_FUNC_ADDR, mmd_dev));
    if (ret != CA_E_OK) {
        goto end;
    }
    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MAADR, 0, reg);
    if (ret != CA_E_OK) {
        goto end;
    }
    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MACR, 0, MAKE_MMD(MMD_FUNC_DATA_NO_INC, mmd_dev));
    if (ret != CA_E_OK) {
        goto end;
    }
    ret = aal_mdio_write(0, 1, phy_dev, PHY_REG_MAADR, 0, val16);

end:
    return ret;

}

/**************************************************************/
/*       Internal Etherent PHY clause 45 register access functions      */
/**************************************************************/
static ca_status_t __eth_phy_clause45_regfield_read
(
    ca_uint8_t   phy_dev,
    ca_uint8_t   reg_dev_addr,
    ca_uint16_t   reg,
    ca_uint8_t   offset,
    ca_uint8_t   length,
    ca_uint16_t *p_val16
)
{
    ca_status_t ret = CA_E_OK;
    ca_uint16_t data = 0;

    if (length == 0
        || offset >= PHY_REG_MAX_LEN
        || length > (PHY_REG_MAX_LEN - offset)
        || !p_val16) {
        return CA_E_PARAM;
    }

    ret = aal_mdio_read(0, 0, phy_dev, reg_dev_addr, reg, &data);
    if (ret != CA_E_OK) {
        goto end;
    }

    if ((0 == offset) && (16 == length)) {
        *p_val16 = data & 0xffff;
    }
    else {
        *p_val16 = (data >> offset) & ((0x1 << length) -1);
    }

end:
    return ret;

}
#endif

/* RTL8211 Functions */
/* Specific Register */
#define __RTL8211F_REG_SPEC_PHYCR1                   (0x18)
#define __RTL8211F_REG_SPEC_PHYCR2                   (0x19)
#define __RTL8211F_REG_SPEC_PHYSR                    (0x1A)

/*Int mask Register */
#define __RTL8211_REG_INTR_EN                        (0x12)
#define __RTL8211_REG_INTR_EN_BIT_AN_ERR              (0)
#define __RTL8211_REG_INTR_EN_BIT_PAGE_RCVD           (2)
#define __RTL8211_REG_INTR_EN_BIT_AN_DONE             (3)
#define __RTL8211_REG_INTR_EN_BIT_LINK                (4)
#define __RTL8211_REG_INTR_EN_BIT_PME                 (7)
#define __RTL8211_REG_INTR_EN_BIT_ALDPS               (9)
#define __RTL8211_REG_INTR_EN_BIT_JABBER              (10)

/*MDI mode Register*/
#define __RTL8211_REG_MDI_MODE                       (0x18)
#define __RTL8211_REG_MDI_MODE_BIT_MDI_AUTO          (9)
#define __RTL8211_REG_MDI_MODE_BIT_MDI_FORCE         (8)

/*Int Status Register*/
#define __RTL8211_REG_INTR_STATUS                    (0x1d)

#define __RTL8211_REG_INTR_STATUS_BIT_AN_ERR         (0)
#define __RTL8211_REG_INTR_STATUS_BIT_PAGE_RCVD      (2)
#define __RTL8211_REG_INTR_STATUS_BIT_AN_DONE        (3)
#define __RTL8211_REG_INTR_STATUS_BIT_LINK           (4)
#define __RTL8211_REG_INTR_STATUS_BIT_PME            (7)
#define __RTL8211_REG_INTR_STATUS_BIT_ALDPS          (9)
#define __RTL8211_REG_INTR_STATUS_BIT_JABBER         (10)

 /* Page select register */
#define __RTL8211_REG_PAGE_SEL                       (0x1f)
#define __RTL8211_REG_PAGE_INTR_EN                   (0x0)
#define __RTL8211_REG_PAGE_LCR                       (0xd04)
#define __RTL8211_REG_PAGE_INTBCR                    (0xd40)

#define  __RTL8211_POWER_DOWN_CYCLE_TIME              (10)
#define  __RTL8211_POWER_UP_CYCLE_TIME                (500)

/**************************************************************/
/*             Common Etherent PHY APIs                       */
/**************************************************************/
int  __eth_phy_reset(uint8 phy_dev)
{
	return rtk_eth_phy_regfield_write(phy_dev, PHY_REG_CTRL, PHY_REG_CTRL_BIT_RESET, 1, 1);
}

int  __eth_phy_auto_neg_restart(uint8 phy_dev)
{
	return rtk_eth_phy_regfield_write(phy_dev, PHY_REG_CTRL, PHY_REG_CTRL_BIT_AN_RESTART, 1, 1);
}

int  __eth_phy_power_set(uint8 phy_dev, bool power_up)
{
	return rtk_eth_phy_regfield_write(phy_dev, PHY_REG_CTRL, PHY_REG_CTRL_BIT_PWR_DOWN, 1, power_up ? FALSE : TRUE);
}

int  __eth_phy_power_get(uint8 phy_dev, bool *power_up)
{
	uint16 data16 = 0;
	int ret = 0;

	ret = rtk_eth_phy_regfield_read(phy_dev, PHY_REG_CTRL, PHY_REG_CTRL_BIT_PWR_DOWN, 1, &data16);
	*power_up = data16 ? FALSE : TRUE;

	return ret;
}

int  __eth_phy_auto_neg_set(uint8 phy_dev, bool enable)
{
	return rtk_eth_phy_regfield_write(phy_dev, PHY_REG_CTRL, PHY_REG_CTRL_BIT_AN_CTRL, 1, enable ? 1 : 0);
}

int  __eth_phy_auto_neg_get(uint8 phy_dev, bool *enable)
{
	uint16 data16 = 0;
	int ret = 0;

	ret = rtk_eth_phy_regfield_read(phy_dev, PHY_REG_CTRL, PHY_REG_CTRL_BIT_AN_CTRL, 1, &data16);
	*enable = data16 ? TRUE : FALSE;

	return ret;
}


//extern u32 __eth_phy_rtl8211_add_phy(u32 port, u8 phy_addr);
u32 __attribute__((weak)) __eth_phy_rtl8211_add_phy(u32 port, u8 phy_addr) {	RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
//extern u32 __eth_phy_rtl8211_init(u8 phy_dev);
u32 __attribute__((weak)) __eth_phy_rtl8211_init(u8 phy_dev) {			RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8211_auto_neg_set(u8 phy_addr)		{RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
//extern u32 __eth_phy_rtl8211_link_status_get(u8 phy_dev);
#ifdef CONFIG_LUNA_G3_SERIES
u32 __attribute__((weak)) __eth_phy_rtl8211_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status) {
	uint16 data = 0;
	phy_reg_GBCR_t      reg_GBCR_val;
	phy_reg_GBSR_t      reg_GBSR_val;
	phy_reg_ANAR_t      reg_ANAR_val;
	phy_reg_ANLPAR_t    reg_ANLPAR_val;

	RTK_MmdPhyRead(0, 1, phy_addr, __RTL8211F_REG_SPEC_PHYSR, 0, &data);
	/*
	 * __RTL8211F_REG_SPEC_PHYSR_BIT_LNK             2
	 * __RTL8211F_REG_SPEC_PHYSR_BIT_DUPLEX          3
	 * __RTL8211F_REG_SPEC_PHYSR_BIT_SPEED           4,5
	 */
	link_status->link_up = (data >> 2) & 0x1;

	if (!link_status->link_up) {
		link_status->speed    = EXT_PHY_SPEED_10;
		link_status->duplex   = FALSE;
		return 0;
	}

	RTK_MmdPhyRead(0, 1, phy_addr, PHY_REG_1000BASET_CTRL, 0, (u16 *) &reg_GBCR_val.wrd);
	RTK_MmdPhyRead(0, 1, phy_addr, PHY_REG_1000BASET_STATUS, 0, (u16 *) &reg_GBSR_val.wrd);
	RTKEXTGPHY_DEBUG("%s: reg_GBCR_val.wrd=0x%x, reg_GBSR_val.wrd=0x%x\n", __func__, reg_GBCR_val.wrd, reg_GBSR_val.wrd);
	/* check 1000Mbps connection */
	if (reg_GBCR_val.bf.adv_1000base_t_full && reg_GBSR_val.bf.lp_1000base_t_full) {
		RTKEXTGPHY_DEBUG("%s: Link at 1000Mbps Full Duplex\n", __func__);
		link_status->speed = EXT_PHY_SPEED_1000;
		link_status->duplex = EXT_PHY_DUPLEX_FULL;
		return 0;
	}

	RTK_MmdPhyRead(0, 1, phy_addr, PHY_REG_CTRL, 0, &data);
	if (!((data >> PHY_REG_CTRL_BIT_AN_CTRL) & 0x1)) {/* no autoneg */
		link_status->duplex = (data >> PHY_REG_CTRL_BIT_DUPLEX) & 0x1;
		if (0x2000 == (data & 0x2040))
			link_status->speed = EXT_PHY_SPEED_100;
		else if (0x40 == (data & 0x2040))
			link_status->speed = EXT_PHY_SPEED_1000;
		else
			link_status->speed = EXT_PHY_SPEED_10;
		return 0;
	}

	/* autoneg */
	RTK_MmdPhyRead(0, 1, phy_addr, PHY_REG_AUTONEG_ADV, 0, (u16 *) &reg_ANAR_val.wrd);
	RTK_MmdPhyRead(0, 1, phy_addr, PHY_REG_LP_ABILITY_BASE, 0, (u16 *) &reg_ANLPAR_val.wrd);
	RTKEXTGPHY_DEBUG("%s: reg_ANAR_val.wrd=0x%x, reg_ANLPAR_val.wrd=0x%x\n", __func__, reg_ANAR_val.wrd, reg_ANLPAR_val.wrd);

	if (reg_ANAR_val.bf.adv_100base_tx_full && reg_ANLPAR_val.bf.adv_100base_tx_full) {
		RTKEXTGPHY_DEBUG("%s: Link at 100Mbps Full Duplex\n", __func__);
		link_status->speed = EXT_PHY_SPEED_100;
		link_status->duplex = EXT_PHY_DUPLEX_FULL;
		return 0;
	}

	if (reg_ANAR_val.bf.adv_100base_tx_half && reg_ANLPAR_val.bf.adv_100base_tx_half) {
		RTKEXTGPHY_DEBUG("%s: Link at 100Mbps Half Duplex\n", __func__);
		link_status->speed = EXT_PHY_SPEED_100;
		link_status->duplex = EXT_PHY_DUPLEX_HALF;
		return 0;
	}

	if (reg_ANAR_val.bf.adv_10base_t_full && reg_ANLPAR_val.bf.adv_10base_t_full) {
		RTKEXTGPHY_DEBUG("%s: Link at 10Mbps Full Duplex\n", __func__);
		link_status->speed = EXT_PHY_SPEED_10;
		link_status->duplex = EXT_PHY_DUPLEX_FULL;
		return 0;
	}

	if (reg_ANAR_val.bf.adv_10base_t_half && reg_ANLPAR_val.bf.adv_10base_t_half) {
		RTKEXTGPHY_DEBUG("%s: Link at 10Mbps Half Duplex\n", __func__);
		link_status->speed = EXT_PHY_SPEED_10;
		link_status->duplex = EXT_PHY_DUPLEX_HALF;
		return 0;
	}

	return 0;
}
#else
u32 __attribute__((weak)) __eth_phy_rtl8211_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status) {RTKEXTGPHY_ERROR("Empty Function:");   return 0;}
#endif

/* RTL8226 Functions */
#ifdef CONFIG_RTL_8221B_SUPPORT
extern u32 __eth_phy_rtl8226b_add_phy(u32 port, u8 phy_addr);
extern u32 __eth_phy_rtl8226b_init(u8 phy_dev);
extern u32 __eth_phy_rtl8226b_link_status_get(u8 phy_dev, ext_phy_link_status_t *link_status);
extern u32 __eth_phy_rtl8226b_flow_ctrl_get(u8 phy_dev, u8 *tx_fc, u8 *rx_fc);
#else
u32 __attribute__((weak)) __eth_phy_rtl8226b_add_phy(u32 port, u8 phy_addr) {					RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8226b_init(u8 phy_dev) {							RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8226b_auto_neg_set(u8 phy_addr) {					RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8226b_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status) {	RTKEXTGPHY_ERROR("Empty Function:"); link_status->link_up = 0; return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8226b_flow_ctrl_get(u8 phy_dev, u8 *tx_fc, u8 *rx_fc) {	RTKEXTGPHY_ERROR("Empty Function:"); *tx_fc=0; *rx_fc=0; return 0;}
#endif

/* RTL8261 Functions */
#ifdef CONFIG_RTL8261_SUPPORT
extern u32 __eth_phy_rtl8261i_reg_get(u8 phy_addr, rtk_extgphy_reg_t *pReg);
extern u32 __eth_phy_rtl8261i_reg_set(u8 phy_addr, rtk_extgphy_reg_t *pReg);
extern u32 __eth_phy_rtl8261i_add_phy(u32 port, u8 phy_addr);
extern u32 __eth_phy_rtl8261i_init(u8 phy_dev);
extern u32 __eth_phy_rtl8261i_auto_neg_set(u8 phy_addr, rtk_extgphy_ability_t *pAbility);
extern u32 __eth_phy_rtl8261i_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status);
#else
u32 __attribute__((weak)) __eth_phy_rtl8261i_reg_get(u8 phy_addr, rtk_extgphy_reg_t *pReg)			{RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8261i_reg_set(u8 phy_addr, rtk_extgphy_reg_t *pReg)			{RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8261i_add_phy(u32 port, u8 phy_addr)					{RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8261i_init(u8 phy_dev)							{RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8261i_auto_neg_set(u8 phy_addr, rtk_extgphy_ability_t *pAbility)		{RTKEXTGPHY_ERROR("Empty Function:");	return 0;}
u32 __attribute__((weak)) __eth_phy_rtl8261i_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status)	{RTKEXTGPHY_ERROR("Empty Function:"); link_status->link_up = 0; return 0;}
#endif

/* Ethernet PHY driver object */
typedef struct {
	u32 (*reg_get)(u8, rtk_extgphy_reg_t*);
	u32 (*reg_set)(u8, rtk_extgphy_reg_t*);
	u32 (*add_phy)(u32, u8);
	u32 (*init)(u8);
	//u32 (*reset)(ca_uint8_t );
	//u32 (*auto_neg_restart)(ca_uint8_t );
	//u32 (*power_set)(ca_uint8_t, ca_boolean_t );
	//u32 (*power_get)(ca_uint8_t, ca_boolean_t*);
	u32 (*auto_neg_set)(u8, rtk_extgphy_ability_t*);
	//u32 (*auto_neg_get)(ca_uint8_t, ca_boolean_t*);
	//u32 (*ability_adv_set)(ca_uint8_t, aal_phy_autoneg_adv_t );
	//u32 (*ability_adv_get)(ca_uint8_t, aal_phy_autoneg_adv_t*);
	//u32 (*partner_ability_adv_get)(ca_uint8_t, aal_phy_autoneg_adv_t*);
	u32 (*link_status_get)(u8, ext_phy_link_status_t*);
	//u32 (*speed_set)(ca_uint8_t, aal_phy_speed_mode_t );
	//u32 (*speed_get)(ca_uint8_t, aal_phy_speed_mode_t*);
	//u32 (*duplex_set)(ca_uint8_t, ca_boolean_t );
	//u32 (*duplex_get)(ca_uint8_t, ca_boolean_t*);
	//u32 (*flow_ctrl_set)(ca_uint8_t, ca_boolean_t );
	u32 (*flow_ctrl_get)(u8, u8*, u8*);
	//u32 (*mdix_set)(ca_uint8_t, aal_phy_mdi_mode_t );
	//u32 (*mdix_get)(ca_uint8_t, aal_phy_mdi_mode_t*);
	//u32 (*loopback_set)(ca_uint8_t, aal_phy_lpbk_mode_t);
	//u32 (*loopback_get)(ca_uint8_t, aal_phy_lpbk_mode_t*);
	//u32 (*int_mask_all)(ca_uint8_t);
	//u32 (*int_en_set)(ca_uint8_t, ca_uint16_t);
	//u32 (*int_en_get)(ca_uint8_t, ca_uint16_t*);
	//u32 (*int_status_get)(ca_uint8_t, ca_uint16_t*);
	//u32 (*int_status_clr)(ca_uint8_t);
	//u32 (*debug_info_get)(ca_uint8_t);
	//u32 (*fw_upgrade_set)(ca_uint8_t, aal_phy_boot_mode_t);
	//u32 (*fw_upgrade_get)(ca_uint8_t, aal_phy_boot_mode_t*);
} phy_drv_t;

typedef struct {
	char        name[32];
	u32 oui;
	u32 model;
	phy_drv_t   *drv;
} phy_dev_t;

phy_drv_t rtl8211_phy_drv = {
	NULL, //__eth_phy_rtl8211_reg_get,;
	NULL, //__eth_phy_rtl8211_reg_set,;
	NULL, //__eth_phy_rtl8211_add_phy,
	NULL, //__eth_phy_rtl8211_init,
	//__eth_phy_reset,
	//__eth_phy_auto_neg_restart,
	//__eth_phy_power_set,
	//__eth_phy_power_get,
	NULL, //__eth_phy_auto_neg_set,
	//__eth_phy_auto_neg_get,
	//__eth_phy_ability_adv_set,
	//__eth_phy_ability_adv_get,
	//__eth_phy_partner_ability_adv_get,
	__eth_phy_rtl8211_link_status_get,
	//__eth_phy_speed_set,
	//__eth_phy_speed_get,
	//__eth_phy_duplex_set,
	//__eth_phy_duplex_get,
	//__eth_phy_flow_ctrl_set,
	NULL, //__eth_phy_flow_ctrl_get,
	//__eth_phy_rtl8211_mdix_set,
	//__eth_phy_rtl8211_mdix_get,
	//__eth_phy_loopback_set,
	//__eth_phy_loopback_get,
	//__eth_phy_rtl8211_int_mask_all,
	//__eth_phy_rtl8211_int_en_set,
	//__eth_phy_rtl8211_int_en_get,
	//__eth_phy_rtl8211_int_status_get,
	//__eth_phy_rtl8211_int_status_clr,
	//__eth_phy_debug_info_get
};

phy_dev_t __phy_rtl_8211_dev = {
	.name  = "RTL8211",
	.oui   = PHY_OUI_RTK,
	.model = PHY_MODEL_RTL8211,
	.drv   = &rtl8211_phy_drv,
};

phy_drv_t phy_rtl_8226b_drv = {
	NULL, //__eth_phy_rtl8226b_reg_get,;
	NULL, //__eth_phy_rtl8226b_reg_set,;
	__eth_phy_rtl8226b_add_phy,
	__eth_phy_rtl8226b_init,
	//__eth_phy_reset,
	//__eth_phy_auto_neg_restart,
	//__eth_phy_power_set,
	//__eth_phy_power_get,
	NULL, //__eth_phy_auto_neg_set,
	//__eth_phy_auto_neg_get,
	//__eth_phy_ability_adv_set,
	//__eth_phy_ability_adv_get,
	//__eth_phy_partner_ability_adv_get,
	__eth_phy_rtl8226b_link_status_get,
	//NULL,
	//NULL,
	//NULL,
	//__eth_phy_rtl8226b_duplex_get,
	//NULL,
	__eth_phy_rtl8226b_flow_ctrl_get,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
	//NULL,
};

phy_dev_t __phy_rtl_8226b_dev ={
        .name  = "RTL8226b",
        .oui   = PHY_OUI_RTK,
        .model = PHY_MODEL_RTL8226B,
        .drv   = &phy_rtl_8226b_drv,
};

phy_drv_t phy_rtl_8261i_drv = {
	__eth_phy_rtl8261i_reg_get,
	__eth_phy_rtl8261i_reg_set,
        __eth_phy_rtl8261i_add_phy,
        __eth_phy_rtl8261i_init,
        //__eth_phy_reset,
        //__eth_phy_auto_neg_restart,
        //__eth_phy_power_set,
        //__eth_phy_power_get,
        __eth_phy_rtl8261i_auto_neg_set,
        //__eth_phy_auto_neg_get,
        //__eth_phy_ability_adv_set,
        //__eth_phy_ability_adv_get,
        //__eth_phy_partner_ability_adv_get,
        __eth_phy_rtl8261i_link_status_get,
        //__eth_phy_speed_set,
        //__eth_phy_speed_get,
        //__eth_phy_duplex_set,
        //__eth_phy_rtl8261i_duplex_get,
        //__eth_phy_flow_ctrl_set,
        NULL, //__eth_phy_flow_ctrl_get,
        //__eth_phy_rtl8211_mdix_set,
        //__eth_phy_rtl8211_mdix_get,
        //__eth_phy_loopback_set,
        //__eth_phy_loopback_get,
        //__eth_phy_rtl8211_int_mask_all,
        //__eth_phy_rtl8211_int_en_set,
        //__eth_phy_rtl8211_int_en_get,
        //__eth_phy_rtl8211_int_status_get,
        //__eth_phy_rtl8211_int_status_clr,
        //__eth_phy_debug_info_get,
        //NULL,
};

phy_dev_t __phy_rtl_8261i_dev ={
        .name  = "RTL8261I",
        .oui   = PHY_OUI_RTK,
        .model = PHY_MODEL_RTL8261I,
        .drv   = &phy_rtl_8261i_drv,
};

static struct task_struct *rtk_ext_gphy_task;
#define RTK_EX_GPHY_KTHREAD_CPU	0

struct ext_gphy_data {
	u32			port;
	u8			phy_addr;
	u8			st_code;
	ext_phy_link_status_t	link_status;
	phy_dev_t		*dev;
	struct	list_head	list;
};

LIST_HEAD(gphy_list);

static void rtk_ext_gphy_show_list(void)
{
	int i = 0;
	struct list_head	*iterator;

	RTKEXTGPHY_MSG("External GPHY List:");
	list_for_each(iterator, &gphy_list) {
		RTKEXTGPHY_MSG("#%X node (%px): [Port %d] [phyAddr %d] [ST %s] [%s]", i++,
			list_entry(iterator, struct ext_gphy_data, list),
			list_entry(iterator, struct ext_gphy_data, list)->port,
			list_entry(iterator, struct ext_gphy_data, list)->phy_addr,
			list_entry(iterator, struct ext_gphy_data, list)->st_code ? "C22" : "C45",
			(list_entry(iterator, struct ext_gphy_data, list)->dev == NULL) ? "NO Driver" : list_entry(iterator, struct ext_gphy_data, list)->dev->name);
	}
	RTKEXTGPHY_MSG("Total in the list: %d", i);
}

static int rtk_ext_gphy_clear_list(void)
{
	int i = 0;
	struct list_head *iterator, *tmp;

	list_for_each_safe(iterator, tmp, &gphy_list) {
		list_del(iterator);
		RTKEXTGPHY_MSG("Free #%X node (%px): [Port %d] [phyAddr %d] [%s]", i++,
			list_entry(iterator, struct ext_gphy_data, list),
			list_entry(iterator, struct ext_gphy_data, list)->port,
			list_entry(iterator, struct ext_gphy_data, list)->phy_addr,
			(list_entry(iterator, struct ext_gphy_data, list)->dev == NULL) ? "NO Driver" : list_entry(iterator, struct ext_gphy_data, list)->dev->name);

		kfree((struct ext_gphy_data *)(list_entry(iterator, struct ext_gphy_data, list)));
	}

	if (list_empty(&gphy_list))	RTKEXTGPHY_MSG("List is already empty");

	return 0;
}

static int rtk_ext_gphy_add_to_list(unsigned int port, unsigned int phy_addr)
{
	int i = 0;
	struct list_head	*iterator;
	struct ext_gphy_data	*ext_gphy;

	list_for_each(iterator, &gphy_list)	i++;
	if(i >= MAX_EXTERNAL_GPHYS_NUMBER) {
		RTKEXTGPHY_ERROR("Reach MAX External GPHYs list number(%d)\n", MAX_EXTERNAL_GPHYS_NUMBER);
		return 1;
	}
	RTKEXTGPHY_DEBUG("Total External GPHYs in the list: %d", i);

	ext_gphy = (struct ext_gphy_data *)kmalloc(sizeof(struct ext_gphy_data), GFP_KERNEL);
	RTKEXTGPHY_DEBUG("kmalloc ext_gphy: %px", ext_gphy);

	ext_gphy->port = port;
	ext_gphy->phy_addr = phy_addr;
	ext_gphy->st_code = 0;
	ext_gphy->link_status.link_up = 0;
	ext_gphy->link_status.duplex = EXT_PHY_DUPLEX_INVALID;
	ext_gphy->link_status.speed = EXT_PHY_SPEED_MAX;
	ext_gphy->dev = NULL;

	if (list_empty(&gphy_list)) {
		RTKEXTGPHY_DEBUG("List is empty");
		list_add(&ext_gphy->list, &gphy_list);
	}
	else {
		i = 0;
		list_for_each (iterator, &gphy_list){
			if (port < list_entry(iterator, struct ext_gphy_data, list)->port) {
				list_add_tail(&ext_gphy->list, iterator);
				i = 1;
				break;
			}
			else if (port == list_entry(iterator, struct ext_gphy_data, list)->port) {
				RTKEXTGPHY_ERROR("The Port #%d info is existed", port);
				return 1;
			}
			else
				continue;
		}
		if(!i)	list_add_tail(&ext_gphy->list, &gphy_list);	/* File with MAXMUM Port# */
	}

	RTKEXTGPHY_DEBUG("Add new node (%px): [Port %d] [phyAddr %d]", ext_gphy, ext_gphy->port, ext_gphy->phy_addr);

	return 0;
}

static int rtk_ext_gphy_auto_detect(void)
{
	struct list_head	*iterator;
	u32 port;
	u8 phy_addr;
	u16 phy_id1 = 0, phy_id2 = 0, phy_oui = 0, phy_model = 0;
	int ret;

	list_for_each(iterator, &gphy_list) {
		port = list_entry(iterator, struct ext_gphy_data, list)->port;
		phy_addr = list_entry(iterator, struct ext_gphy_data, list)->phy_addr;

		ret = RTK_MmdPhyRead(DEFAULT_MDIO_DEVICE_SET_ID, MDIO_ST_CODE_C45,
				phy_addr, 1, PHY_REG_ID_1, &phy_id1);
		if (ret)	continue;
		ret = RTK_MmdPhyRead(DEFAULT_MDIO_DEVICE_SET_ID, MDIO_ST_CODE_C45,
				phy_addr, 1, PHY_REG_ID_2, &phy_id2);
		if (ret)	continue;

		if ((phy_id1 == 0xFFFF) && (phy_id2 == 0xFFFF)) {
			ret = RTK_MmdPhyRead(DEFAULT_MDIO_DEVICE_SET_ID, MDIO_ST_CODE_C22,
					phy_addr, PHY_REG_ID_1, 0, &phy_id1);
			if (ret)	continue;
			ret = RTK_MmdPhyRead(DEFAULT_MDIO_DEVICE_SET_ID, MDIO_ST_CODE_C22,
					phy_addr, PHY_REG_ID_2, 0, &phy_id2);
			if (ret)	continue;

			if ((phy_id1 == 0xFFFF) && (phy_id2 == 0xFFFF)) {
				RTKEXTGPHY_ERROR("Can NOT read PHY ID from phyAddr: %d", phy_addr);
				continue;
			}
			else
				list_entry(iterator, struct ext_gphy_data, list)->st_code = MDIO_ST_CODE_C22;
		}
		else
			list_entry(iterator, struct ext_gphy_data, list)->st_code = MDIO_ST_CODE_C45;

		phy_oui   = (((phy_id1 & 0x3ff) << 6)|((phy_id2 & 0xfc00) >> 10));
		phy_model = ((phy_id2 & 0x03f0) >> 4);

		RTKEXTGPHY_MSG("read Ethernet PHY %u IDs(%#x %#x): phy_oui %#x, phy_model %#x",
			phy_addr, phy_id1, phy_id2, phy_oui, phy_model);

		if(phy_oui == PHY_OUI_RTK) { /* RealTek Ethernet PHY */
			switch (phy_model) {
			case PHY_MODEL_RTL8201F:
				//list_entry(iterator, struct ext_gphy_data, list)->dev = &__phy_rtl_8201f_dev;
				RTKEXTGPHY_MSG("detect Eth PHY RTL8201F at PHY addr %u", phy_addr);
				break;
			case PHY_MODEL_RTL8211:
			case PHY_MODEL_RTL8211E:
			case PHY_MODEL_RTL8211_ASIC:
				list_entry(iterator, struct ext_gphy_data, list)->dev = &__phy_rtl_8211_dev;
				RTKEXTGPHY_MSG("detect Eth PHY RTL8211 at PHY addr %u", phy_addr);
#ifdef RTK_EXT_GPHY_SET_NI_SDS_MODE
#ifdef CONFIG_LUNA_G3_SERIES
				RTKEXTGPHY_MSG("Set port %d as RGMII (Default 1G)", port);
				aal_sds_mode_set(0, port, AAL_SDS_MODE_RGMII, AAL_SDS_SPEED_DEFAULT);
#endif
#endif
				break;
			case PHY_MODEL_RTL8198F:
				list_entry(iterator, struct ext_gphy_data, list)->dev = &__phy_rtl_8211_dev;
				RTKEXTGPHY_MSG("detect Embedded FE PHY RTL8198F at PHY addr %u", phy_addr);
				break;
			case PHY_MODEL_RTL8214:
			case PHY_MODEL_RTL8214C:
				//list_entry(iterator, struct ext_gphy_data, list)->dev = &__phy_rtl_8214_dev;
				RTKEXTGPHY_MSG("detect Eth PHY RTL8214(C) at PHY addr %u", phy_addr);
				break;
			case PHY_MODEL_RTL8226B:
				list_entry(iterator, struct ext_gphy_data, list)->dev = &__phy_rtl_8226b_dev;
				RTKEXTGPHY_MSG("detect Eth PHY RTL8226B at PHY addr %u",phy_addr);
#ifdef RTK_EXT_GPHY_SET_NI_SDS_MODE
#ifdef CONFIG_LUNA_G3_SERIES
				RTKEXTGPHY_MSG("Set port %d as HiSGMII/SGMII (Default 1G)", port);
				aal_sds_mode_set(0, port, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_DEFAULT);
#endif
#endif
				break;
			case PHY_MODEL_RTL8261I:
				list_entry(iterator, struct ext_gphy_data, list)->dev = &__phy_rtl_8261i_dev;
				RTKEXTGPHY_MSG("detect Eth PHY RTL8261I at PHY addr %u", phy_addr);
#ifdef RTK_EXT_GPHY_SET_NI_SDS_MODE
#ifdef CONFIG_LUNA_G3_SERIES
				RTKEXTGPHY_MSG("Set port %d as USXGMII (Default 10G)", port);
				aal_sds_mode_set(0, port, AAL_SDS_MODE_USXGMII, AAL_SDS_SPEED_DEFAULT);
#endif
#endif
				break;
			default:
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
				return 1;
			}
			if (list_entry(iterator, struct ext_gphy_data, list)->dev == NULL)
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
		} else if ( phy_oui == PHY_OUI_MVL ) { /* Marvel 10G Ethernet PHY */
			switch (phy_model) {
			case PHY_MODEL_MVL88X3310:
				//if (cap_phy_drv_get(device_id, CA_PHY_TYPE_0, (void**)&p_eth_phy_dev[phy_addr])) {
				//	p_eth_phy_dev[phy_addr] = NULL;
				//	RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
				//	return CA_E_NOT_SUPPORT;
				//}
				RTKEXTGPHY_MSG("detect Eth PHY MVL88x3310 at PHY addr %u", phy_addr);
				break;
			default:
				//p_eth_phy_dev[phy_addr] = NULL;
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
				return 1;
			}
			if (list_entry(iterator, struct ext_gphy_data, list)->dev == NULL)
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
		} else if ( phy_oui == PHY_OUI_AQR_G2 ) { /* Aquantia 10G Ethernet PHY */
			switch (phy_model) {
			case PHY_MODEL_AQR107:
				//if (cap_phy_drv_get(device_id, CA_PHY_TYPE_1, (void**)&p_eth_phy_dev[phy_addr])) {
				//	p_eth_phy_dev[phy_addr] = NULL;
				//	RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
				//	return CA_E_NOT_SUPPORT;
				//}
				RTKEXTGPHY_MSG("detect Eth PHY AQR107 at PHY addr %u", phy_addr);
				break;
			default:
				//p_eth_phy_dev[phy_addr] = NULL;
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
				return 1;
			}
			if (list_entry(iterator, struct ext_gphy_data, list)->dev == NULL)
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
		} else if (phy_oui == PHY_OUI_AQR_G4) { /* Aquantia Gen4-14nm 10G Ethernet PHY */
			switch (phy_model) {
			case PHY_MODEL_AQR113:
				//if (cap_phy_drv_get(device_id, CA_PHY_TYPE_2, (void **)&p_eth_phy_dev[phy_addr])) {
				//	p_eth_phy_dev[phy_addr] = NULL;
				//	RTKEXTGPHY_MSG("!!!cap_phy_drv_get() failed for PHY at addr %u", phy_addr);
				//	break;
				//}
				RTKEXTGPHY_MSG("detect Eth PHY AQR113 at PHY addr %u", phy_addr);
				break;
			default:
				//p_eth_phy_dev[phy_addr] = NULL;
				RTKEXTGPHY_MSG("!!!No driver support for AQR PHY at addr %u", phy_addr);
				return 1;
			}
			if (list_entry(iterator, struct ext_gphy_data, list)->dev == NULL)
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
		} else if (phy_oui == PHY_OUI_INTL) { /* Intel 2.5G Ethernet PHY */
			switch (phy_model) {
			case PHY_MODEL_INTL212B:
				//p_eth_phy_dev[phy_addr] = &__phy_intl_212b_dev;
				RTKEXTGPHY_MSG("detect Eth PHY INTL212B at PHY addr %u", phy_addr);
				break;
			case PHY_MODEL_INTL241:
				//p_eth_phy_dev[phy_addr] = &__phy_intl_gpy241_dev;
				RTKEXTGPHY_MSG("detect Eth PHY INTL241 at PHY addr %u", phy_addr);
				break;
			default:
				//p_eth_phy_dev[phy_addr] = NULL;
				RTKEXTGPHY_MSG("!!!No driver support for AQR PHY at addr %u", phy_addr);
				return 1;
			}
			if (list_entry(iterator, struct ext_gphy_data, list)->dev == NULL)
				RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
		} else {
			RTKEXTGPHY_MSG("!!!No driver support for PHY at addr %u", phy_addr);
			return 1;
		}
		if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->add_phy != NULL)
			list_entry(iterator, struct ext_gphy_data, list)->dev->drv->add_phy(port, phy_addr);
	}

	return 0;
}

static void rtk_ext_gphy_all_init(void)
{
	//int i = 0;
	struct list_head	*iterator;

	list_for_each(iterator, &gphy_list) {
		if (list_entry(iterator, struct ext_gphy_data, list)->dev) {
			if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->init) {
				list_entry(iterator, struct ext_gphy_data, list)->dev->drv->init(list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
			}
			else
				RTKEXTGPHY_MSG("!!!No init function for PHY at addr %u", list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
		}
		else
			RTKEXTGPHY_ERROR("!!!No driver support for PHY at addr %u", list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
	}

	return;
}

static int rtk_ext_gphy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;
	int i, length_args, num_gphys, num_cells, offset, port, phyAddr;

	dev_dbg(&pdev->dev, "%s started\n", __FUNCTION__);
	RTKEXTGPHY_MSG("%s with .dts", __FUNCTION__);

	if (node) {
		/* Get Number of Cells (num-cells) */
		ret = of_property_read_u32_index(node, "num-cells", 0, &num_cells);
		if (ret) {
			RTKEXTGPHY_ERROR("of_property_read_u32_index Error !! (%d)", ret);
			goto err;
		}
		RTKEXTGPHY_DEBUG("Number of Cells (num-cells) = %d (.dts)", num_cells);

		/* Get port_mapping_phyAddr data length */
		of_get_property(node, "port_mapping_phyAddr", &length_args);
		RTKEXTGPHY_DEBUG("Length of port_mapping_phyAddr = %d (.dts)", length_args);

		/* Count how many port-gphy mapping are defined */
		num_gphys = length_args / (sizeof(u32) * num_cells);
		RTKEXTGPHY_MSG("Number of Port/GPHY Mapping (.dts) = %d", num_gphys);

		for (i = 0; i < num_gphys; i++) {
			offset = i * num_cells;

			ret = of_property_read_u32_index(node, "port_mapping_phyAddr", offset, &port);
			if (ret) {
				RTKEXTGPHY_ERROR("of_property_read_u32_index Error !! (%d)", ret);
				goto err;
			}
			ret = of_property_read_u32_index(node, "port_mapping_phyAddr", offset + 1, &phyAddr);
			if (ret) {
				RTKEXTGPHY_ERROR("of_property_read_u32_index Error !! (%d)", ret);
				goto err;
			}

			RTKEXTGPHY_DEBUG("Find a phy on Port %d with phyAddress %d (.dts)", port, phyAddr);
			rtk_ext_gphy_add_to_list(port, phyAddr);
		}
	}
	else {
		RTKEXTGPHY_ERROR("of_find_node_by_name Error !!");
	}

	return 0;
err:
	return ret;
}

static const struct of_device_id rtk_ext_gphy_match_table[] = {
	{ .compatible = "realtek,ext-gphy" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtk_ext_gphy_match_table);

static struct platform_driver rtk_ext_gphy_driver = {
	.driver = {
		.name = "rtk-ext-gphy",
		.of_match_table = rtk_ext_gphy_match_table,
	},
	.probe = rtk_ext_gphy_probe,
};
//module_platform_driver(rtk_ext_gphy_driver);

static char *rtk_eth_phy_speed_str_get(ext_phy_speed_mode_t phy_speed)
{
	switch(phy_speed) {
	case EXT_PHY_SPEED_10:
		return "10M";
	case EXT_PHY_SPEED_100:
		return "100M";
	case EXT_PHY_SPEED_500:
		return "500M";
	case EXT_PHY_SPEED_1000:
		return "1G";
	case EXT_PHY_SPEED_10000:
		return "10G";
	case EXT_PHY_SPEED_2000:
		return "2G";
	case EXT_PHY_SPEED_2500:
		return "2.5G";
	case EXT_PHY_SPEED_5000:
		return "5G";
	case EXT_PHY_SPEED_2500LITE:
		return "2.5G Lite";
	case EXT_PHY_SPEED_5000LITE:
		return "5G Lite";
	case EXT_PHY_SPEED_10000LITE:
		return "10G Lite";
	case EXT_PHY_SPEED_MAX:
	default:
		return "Unkown";
	}
}

#ifndef CONFIG_LUNA_G3_SERIES
#define CONFIG_SDK_KERNEL_LINUX 1
#include <osal/lib.h>
#include <rtk/switch.h>
#include <rtk/port.h>
int rtk_ext_gphy_sds_mode_set(unsigned int mode)
{
    int ret;
    uint32 chipId, rev, subType;

    if(mode >= LAN_SDS_MODE_END)
    {
        return RT_ERR_INPUT;
    }

    if((ret = rtk_switch_version_get(&chipId, &rev, &subType)) != RT_ERR_OK)
    {
        return ret;
    }

    switch(chipId)
    {
#if defined(CONFIG_SDK_RTL9601B)
    case RTL9601B_CHIP_ID:
        _rtl9601b_lan_sds_mode_set(mode);
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
    case RTL9602C_CHIP_ID:
        if(rev >= CHIP_REV_ID_B)
        {
            if((ret = _rtl9602c_lan_sds_mode_set(mode)) != RT_ERR_OK)
            {
                return ret;
            }
        }
        else
        {
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
#endif
//#if defined(CONFIG_SDK_RTL9607C)
#if defined(CONFIG_RTL9607C_SERIES)
    case RTL9607C_CHIP_ID:
    {
        rtk_port_macAbility_t macAbility;
	RTKEXTGPHY_MSG("Set Port 6 sds mode as %d (9607C)", mode);

        if((ret = rtk_port_serdesMode_set(0,mode)) != RT_ERR_OK)
        {
            return ret;
        }

        return RT_ERR_OK;

        switch(mode)
        {
        case LAN_SDS_MODE_GE_FE_PHY:
            return RT_ERR_CHIP_NOT_SUPPORTED;
        case LAN_SDS_MODE_FIBER_1G:
        case LAN_SDS_MODE_SGMII_PHY:
        case LAN_SDS_MODE_SGMII_MAC:
            /* Port MAC force */
            osal_memset(&macAbility, 0x00, sizeof(rtk_port_macAbility_t));
            macAbility.speed           = PORT_SPEED_1000M;
            macAbility.duplex          = PORT_FULL_DUPLEX;
            macAbility.linkFib1g       = DISABLED;
            macAbility.linkStatus      = PORT_LINKUP;
            macAbility.txFc            = DISABLED;
            macAbility.rxFc            = DISABLED;
            macAbility.nwayAbility     = DISABLED;
            macAbility.masterMod       = DISABLED;
            macAbility.nwayFault       = DISABLED;
	    macAbility.lpi_100m        = DISABLED;
	    macAbility.lpi_giga        = DISABLED;

            if((ret = rtk_port_macForceAbility_set(6, macAbility)) != RT_ERR_OK)
            {
                return ret;
            }
            if((ret = rtk_port_macForceAbilityState_set(6, ENABLED)) != RT_ERR_OK)
            {
                return ret;
            }
            break;
        case LAN_SDS_MODE_HSGMII_PHY:
        case LAN_SDS_MODE_HSGMII_MAC:
        case LAN_SDS_MODE_2500BASEX:
            /* Port MAC force */
            osal_memset(&macAbility, 0x00, sizeof(rtk_port_macAbility_t));
            macAbility.speed           = PORT_SPEED_2G5;
            macAbility.duplex          = PORT_FULL_DUPLEX;
            macAbility.linkFib1g       = DISABLED;
            macAbility.linkStatus      = PORT_LINKUP;
            macAbility.txFc            = DISABLED;
            macAbility.rxFc            = DISABLED;
            macAbility.nwayAbility     = DISABLED;
            macAbility.masterMod       = DISABLED;
            macAbility.nwayFault       = DISABLED;
            macAbility.lpi_100m        = DISABLED;
            macAbility.lpi_giga        = DISABLED;
            if((ret = rtk_port_macForceAbility_set(6, macAbility)) != RT_ERR_OK)
            {
                return ret;
            }
            if((ret = rtk_port_macForceAbilityState_set(6, ENABLED)) != RT_ERR_OK)
            {
                return ret;
            }
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    }
#endif
#if defined(CONFIG_SDK_CA8277B)
    case CA8277B_CHIP_ID:
    {
        if((ret = rtk_port_serdesMode_set(0, mode)) != RT_ERR_OK)
        {
            return ret;
        }
        break;
    }
#endif

    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    //lan_sds_mode = mode;

    return RT_ERR_OK;
}

int rtk_ext_gphy_sds1_mode_set(unsigned int mode)
{
    int ret;
    uint32 chipId, rev, subType;

    if(mode >= LAN_SDS_MODE_END)
    {
        return RT_ERR_INPUT;
    }

    if((ret = rtk_switch_version_get(&chipId, &rev, &subType)) != RT_ERR_OK)
    {
        return ret;
    }

    switch(chipId)
    {
#if defined(CONFIG_SDK_RTL9607C)
    case RTL9607C_CHIP_ID:
    {
        rtk_port_macAbility_t macAbility;
	RTKEXTGPHY_MSG("Set Port 7 sds mode as %d (9607C)", mode);

        if((ret = rtk_port_serdesMode_set(1,mode)) != RT_ERR_OK)
        {
            return ret;
        }

        return RT_ERR_OK;

        switch(mode)
        {
        case LAN_SDS_MODE_GE_FE_PHY:
            return RT_ERR_CHIP_NOT_SUPPORTED;
        case LAN_SDS_MODE_FIBER_1G:
        case LAN_SDS_MODE_SGMII_PHY:
        case LAN_SDS_MODE_SGMII_MAC:
            /* Port MAC force */
            osal_memset(&macAbility, 0x00, sizeof(rtk_port_macAbility_t));
            macAbility.speed           = PORT_SPEED_1000M;
            macAbility.duplex          = PORT_FULL_DUPLEX;
            macAbility.linkFib1g       = DISABLED;
            macAbility.linkStatus      = PORT_LINKUP;
            macAbility.txFc            = DISABLED;
            macAbility.rxFc            = DISABLED;
            macAbility.nwayAbility     = DISABLED;
            macAbility.masterMod       = DISABLED;
            macAbility.nwayFault       = DISABLED;
            macAbility.lpi_100m        = DISABLED;
            macAbility.lpi_giga        = DISABLED;
            if((ret = rtk_port_macForceAbility_set(7, macAbility)) != RT_ERR_OK)
            {
                return ret;
            }
            if((ret = rtk_port_macForceAbilityState_set(7, ENABLED)) != RT_ERR_OK)
            {
                return ret;
            }
            break;
        case LAN_SDS_MODE_HSGMII_PHY:
        case LAN_SDS_MODE_HSGMII_MAC:
        case LAN_SDS_MODE_2500BASEX:
            /* Port MAC force */
            osal_memset(&macAbility, 0x00, sizeof(rtk_port_macAbility_t));
            macAbility.speed           = PORT_SPEED_2G5;
            macAbility.duplex          = PORT_FULL_DUPLEX;
            macAbility.linkFib1g       = DISABLED;
            macAbility.linkStatus      = PORT_LINKUP;
            macAbility.txFc            = DISABLED;
            macAbility.rxFc            = DISABLED;
            macAbility.nwayAbility     = DISABLED;
            macAbility.masterMod       = DISABLED;
            macAbility.nwayFault       = DISABLED;
            macAbility.lpi_100m        = DISABLED;
            macAbility.lpi_giga        = DISABLED;
            if((ret = rtk_port_macForceAbility_set(7, macAbility)) != RT_ERR_OK)
            {
                return ret;
            }
            if((ret = rtk_port_macForceAbilityState_set(7, ENABLED)) != RT_ERR_OK)
            {
                return ret;
            }
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    }
#endif
    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    //lan_sds1_mode = mode;

    return ret;
}

#endif

#ifdef CONFIG_RTL9607C_SERIES
#define SYS_SETTLE              (40*5)	//40 sec, thread sleep 0.2 sec
#define CHECK_PERIOD            (5*5)	//5 sec, thread sleep 0.2 sec
#define RESET_THRESHOLD         (3)
#define REG_DEBUG_SEL                   (0xBB000264)
#define REG_DEBUG_SEL_TOP               (0xBB000268)
#define REG_CHIP_DEBUG_OUT              (0xBB0000A4)
#define REG_SOFTWARE_RST                (0xBB000108)
#define REG_PORT6_RX_UC_PKT_CNT         (0xBB032920)
#define REG_PORT6_RX_MC_PKT_CNT         (0xBB032928)
#define REG_PORT6_RX_BC_PKT_CNT         (0xBB03292C)
#define BIT_SW_RST                      10

unsigned int thread_cntr=0;
unsigned int system_settle=0;
unsigned int mac_reset_count=0;
unsigned int mac_failed_count=0;
unsigned int prev_port6_rx_dma_dbgo;
unsigned int prev_port6_rx_pkt_cnt;

void rtk_mac6_check(void)
{
	unsigned int rx_pkt_cnt, rx_dma_dbgo;

	if (system_settle) {
		thread_cntr++;
		if (thread_cntr > CHECK_PERIOD) {
			REG32(REG_DEBUG_SEL_TOP) = 0x900;
			REG32(REG_DEBUG_SEL) = 0x61006;
			rx_pkt_cnt = REG32(REG_PORT6_RX_UC_PKT_CNT) + REG32(REG_PORT6_RX_MC_PKT_CNT) + REG32(REG_PORT6_RX_BC_PKT_CNT);
			rx_dma_dbgo = REG32(REG_CHIP_DEBUG_OUT);
			REG32(REG_DEBUG_SEL) = 0;
			REG32(REG_DEBUG_SEL_TOP) = 0;

			if ((rx_pkt_cnt != prev_port6_rx_pkt_cnt) && (rx_dma_dbgo == prev_port6_rx_dma_dbgo))
				mac_failed_count++;

			else if ((rx_pkt_cnt != prev_port6_rx_pkt_cnt) && (rx_dma_dbgo != prev_port6_rx_dma_dbgo))
				mac_failed_count = 0;

			prev_port6_rx_dma_dbgo = rx_dma_dbgo;
			prev_port6_rx_pkt_cnt = rx_pkt_cnt;

			if (mac_failed_count >= RESET_THRESHOLD) {
				REG32(REG_SOFTWARE_RST) |= (1 << BIT_SW_RST);
				mac_failed_count = 0;
				mac_reset_count++;
			}
			thread_cntr = 0;
		}
	}
	else {
		thread_cntr++;
		if (thread_cntr > SYS_SETTLE) {
			system_settle = 1;
			thread_cntr = 0;

			prev_port6_rx_dma_dbgo = REG32(REG_CHIP_DEBUG_OUT);
			prev_port6_rx_pkt_cnt = REG32(REG_PORT6_RX_UC_PKT_CNT) + REG32(REG_PORT6_RX_MC_PKT_CNT) + REG32(REG_PORT6_RX_BC_PKT_CNT);
		}
	}
}
#endif

static int rtk_ext_gphy_thread (void *data)
{
	struct list_head	*iterator;
	u32			port;
	ext_phy_link_status_t	link_status;
#ifdef CONFIG_LUNA_G3_SERIES
	event_port_link_t	link_event;
	aal_sds_mode_t		mode;
	aal_sds_speed_t		speed;
#else
	int32 ret = RT_ERR_FAILED;
	rtk_port_macAbility_t portAbility;
	u8 tx_fc=DISABLED, rx_fc=DISABLED;
#endif
	while(!kthread_should_stop()) {
		/* No need to wake up earlier */
		//set_current_state(TASK_UNINTERRUPTIBLE);

		list_for_each(iterator, &gphy_list) {
			if (list_entry(iterator, struct ext_gphy_data, list)->dev) {
				if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->link_status_get) {
					list_entry(iterator, struct ext_gphy_data, list)->dev->drv->link_status_get(list_entry(iterator, struct ext_gphy_data, list)->phy_addr, &link_status);

					if (list_entry(iterator, struct ext_gphy_data, list)->link_status.link_up != link_status.link_up) {
						port = list_entry(iterator, struct ext_gphy_data, list)->port;
						RTKEXTGPHY_PRINT("INFO: Device %d Ethernet Port %u Link %s%s%s%s",
							DEFAULT_MDIO_DEVICE_SET_ID, port,
							link_status.link_up ? "UP " : "DOWN",
							link_status.link_up ? rtk_eth_phy_speed_str_get(link_status.speed) : "",
							link_status.link_up ? "bps/" : "",
							link_status.link_up ? (link_status.duplex ? "FULL" : "HALF") : "");
						list_entry(iterator, struct ext_gphy_data, list)->link_status.link_up = link_status.link_up;
						list_entry(iterator, struct ext_gphy_data, list)->link_status.speed = link_status.speed;
						list_entry(iterator, struct ext_gphy_data, list)->link_status.duplex = link_status.duplex;
#ifdef CONFIG_LUNA_G3_SERIES
#ifdef CONFIG_FC_RTL8277C_SERIES
						if (link_status.link_up && ((port == CA_PORT_ID_NI4) || (port == CA_PORT_ID_NI5) || (port == CA_PORT_ID_NI6))) {
							aal_sds_mode_get(0, port, &mode, &speed);	/* Get current sds mode and do NOT change it */
							switch (link_status.speed) {
							case EXT_PHY_SPEED_10000:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 10G", port);
								aal_sds_mode_set(0, port, mode, AAL_SDS_SPEED_10G);
								break;
							case EXT_PHY_SPEED_5000:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 5G", port);
								aal_sds_mode_set(0, port, mode, AAL_SDS_SPEED_5G);
								break;
							case EXT_PHY_SPEED_2500:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 2.5G", port);
								aal_sds_mode_set(0, port, mode, AAL_SDS_SPEED_2G5);
								break;
							case EXT_PHY_SPEED_1000:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 1G", port);
								aal_sds_mode_set(0, port, mode, AAL_SDS_SPEED_1G);
								break;
							case EXT_PHY_SPEED_100:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 100M", port);
								aal_sds_mode_set(0, port, mode, AAL_SDS_SPEED_100M);
								break;
							case EXT_PHY_SPEED_10:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 10M", port);
								aal_sds_mode_set(0, port, mode, AAL_SDS_SPEED_10M);
								break;
							default:
								RTKEXTGPHY_MSG("UNKNOWN Speed (%d) on Port %d !!!", link_status.speed, port);
							}
						}
#endif
						/* Send link event, ref. __port_link_evt_send */
						memset(&link_event, 0, sizeof(event_port_link_t));
						link_event.port_id = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, port);
						link_event.status = (link_status.link_up) ? 1 : 0 ;
						ca_event_send(DEFAULT_MDIO_DEVICE_SET_ID, CA_EVENT_ETH_PORT_LINK, (void *)&link_event, sizeof(event_port_link_t));
#else
/* CONFIG_RTL9607C_SERIES */
						rtk_port_macForceAbilityState_set(port, ENABLED); // do it once, can move to outside while(), but no "port" info
						
						if (link_status.link_up && ((port == 6) || (port == 7))) {
							
							if((ret = rtk_port_macForceAbility_get(port, &portAbility)) != RT_ERR_OK) {
								RTKEXTGPHY_ERROR("rtk_port_macForceAbility_get Fail (port %d)", port);
								return ret;
							}
							RTKEXTGPHY_INFO("rtk_port_macForceAbility_get OK (port %d)", port);

							if(link_status.duplex)	portAbility.duplex = PORT_FULL_DUPLEX;
							else			portAbility.duplex = PORT_HALF_DUPLEX;

							if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->flow_ctrl_get) {
								list_entry(iterator, struct ext_gphy_data, list)->dev->drv->flow_ctrl_get(list_entry(iterator, struct ext_gphy_data, list)->phy_addr, &tx_fc, &rx_fc);
							}
							portAbility.txFc = tx_fc;
							portAbility.rxFc = rx_fc;

							switch (link_status.speed) {
							case EXT_PHY_SPEED_10000:
								RTKEXTGPHY_MSG("Not Support 10G speed on port %d", port);
								break;
							case EXT_PHY_SPEED_5000:
								RTKEXTGPHY_MSG("Not Support 5G speed on port %d", port);
								break;
							case EXT_PHY_SPEED_2500:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 2.5G", port);
								if (port == 6) {
									rtk_ext_gphy_sds_mode_set(LAN_SDS_MODE_HSGMII_MAC);
								}
								else if (port == 7) {
									rtk_ext_gphy_sds1_mode_set(LAN_SDS_MODE_HSGMII_MAC);
								}
								portAbility.speed = PORT_SPEED_2G5;
								break;
							case EXT_PHY_SPEED_1000:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 1G", port);
								if (port == 6) {
									rtk_ext_gphy_sds_mode_set(LAN_SDS_MODE_SGMII_MAC);
								}
								else if (port == 7) {
									rtk_ext_gphy_sds1_mode_set(LAN_SDS_MODE_SGMII_MAC);
								}
								portAbility.speed = PORT_SPEED_1000M;
								break;
							case EXT_PHY_SPEED_100:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 100M", port);
								if (port == 6) {
									rtk_ext_gphy_sds_mode_set(LAN_SDS_MODE_SGMII_MAC);
								}
								else if (port == 7) {
									rtk_ext_gphy_sds1_mode_set(LAN_SDS_MODE_SGMII_MAC);
								}
								portAbility.speed = PORT_SPEED_100M;
								break;
							case EXT_PHY_SPEED_10:
								RTKEXTGPHY_MSG("Set Port %d sds speed as 10M", port);
								if (port == 6) {
									rtk_ext_gphy_sds_mode_set(LAN_SDS_MODE_SGMII_MAC);
								}
								else if (port == 7) {
									rtk_ext_gphy_sds1_mode_set(LAN_SDS_MODE_SGMII_MAC);
								}
								portAbility.speed = PORT_SPEED_10M;
								break;
							default:
								RTKEXTGPHY_MSG("UNKNOWN Speed (%d) on Port %d !!!", link_status.speed, port);
								break;
							}

							portAbility.linkStatus = PORT_LINKUP;
							if((ret = rtk_port_macForceAbility_set(port, portAbility)) != RT_ERR_OK) {
								RTKEXTGPHY_ERROR("rtk_port_macForceAbility_set Fail (port %d)", port);
								return ret;
							}
							RTKEXTGPHY_INFO("rtk_port_macForceAbility_set OK (port %d)", port);
						}
						else if (!link_status.link_up && ((port == 6) || (port == 7))) {
							if((ret = rtk_port_macForceAbility_get(port, &portAbility)) != RT_ERR_OK) {
								RTKEXTGPHY_ERROR("rtk_port_macForceAbility_get Fail (port %d)", port);
								return ret;
							}

							portAbility.linkStatus = PORT_LINKDOWN;
							rtk_port_macForceAbility_set(port, portAbility);
						}						
#endif
					}
				}
			}
		}
		//schedule_timeout(3 * HZ);
		msleep(200);
		
#ifdef CONFIG_RTL9607C_SERIES
		// due to the root cause has been found, disable the workaround patch for mac 6 failure situation
		//rtk_mac6_check();
#endif
	}
	return 0;
}

static void rtk_ext_gphy_status(void)
{
	struct list_head	*iterator;

	RTKEXTGPHY_MSG("External GPHYs");
	list_for_each(iterator, &gphy_list) {
		RTKEXTGPHY_PRINT("P%d/addr%d [%s]:\tLink %s%s%s%s",
			list_entry(iterator, struct ext_gphy_data, list)->port,
			list_entry(iterator, struct ext_gphy_data, list)->phy_addr,
			(list_entry(iterator, struct ext_gphy_data, list)->dev == NULL) ? "NO Driver" : list_entry(iterator, struct ext_gphy_data, list)->dev->name,
			list_entry(iterator, struct ext_gphy_data, list)->link_status.link_up ? "UP " : "DOWN",
			list_entry(iterator, struct ext_gphy_data, list)->link_status.link_up ? rtk_eth_phy_speed_str_get(list_entry(iterator, struct ext_gphy_data, list)->link_status.speed) : "",
			list_entry(iterator, struct ext_gphy_data, list)->link_status.link_up ? "bps/" : "",
			list_entry(iterator, struct ext_gphy_data, list)->link_status.link_up ? (list_entry(iterator, struct ext_gphy_data, list)->link_status.duplex ? "FULL" : "HALF") : "");
	}
}

static int rtk_ext_gphyReg_get(unsigned int port, rtk_extgphy_reg_t *reg)
{
	struct list_head	*iterator;

	RTKEXTGPHY_PRINT("%s(%d)", __FUNCTION__, __LINE__);
	list_for_each(iterator, &gphy_list) {
		if (port == list_entry(iterator, struct ext_gphy_data, list)->port) {
			RTKEXTGPHY_PRINT("Found port: %d - addr: %d", port, list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
			if (list_entry(iterator, struct ext_gphy_data, list)->dev) {
				if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->reg_get) {
					return list_entry(iterator, struct ext_gphy_data, list)->dev->drv->reg_get(list_entry(iterator, struct ext_gphy_data, list)->phy_addr, reg);
				}
				else {
					RTKEXTGPHY_MSG("!!!No reg_get function for PHY at port %u / addr %u",
						list_entry(iterator, struct ext_gphy_data, list)->port,
						list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
				}
			}
			else
				RTKEXTGPHY_ERROR("!!!No driver support for PHY at addr %u", list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
		}
		//else
			//RTKEXTGPHY_PRINT("NOT Found port: %d\n", port);
	}
	return 0;
}


static int rtk_ext_gphyReg_set(unsigned int port, rtk_extgphy_reg_t *reg)
{
	struct list_head	*iterator;

	RTKEXTGPHY_PRINT("%s(%d)", __FUNCTION__, __LINE__);
	list_for_each(iterator, &gphy_list) {
		if (port == list_entry(iterator, struct ext_gphy_data, list)->port) {
			RTKEXTGPHY_PRINT("Found port: %d - addr: %d", port, list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
			if (list_entry(iterator, struct ext_gphy_data, list)->dev) {
				if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->reg_set) {
					return list_entry(iterator, struct ext_gphy_data, list)->dev->drv->reg_set(list_entry(iterator, struct ext_gphy_data, list)->phy_addr, reg);
				}
				else {
					RTKEXTGPHY_MSG("!!!No reg_set function for PHY at port %u / addr %u",
						list_entry(iterator, struct ext_gphy_data, list)->port,
						list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
				}
			}
			else
				RTKEXTGPHY_ERROR("!!!No driver support for PHY at addr %u", list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
		}
		//else
			//RTKEXTGPHY_PRINT("NOT Found port: %d\n", port);
	}
	return 0;
}

static int rtk_ext_gphyAutoNegoAbility_set(unsigned int port, rtk_extgphy_ability_t *ability)
{
	struct list_head	*iterator;

	RTKEXTGPHY_PRINT("%s(%d)", __FUNCTION__, __LINE__);
	list_for_each(iterator, &gphy_list) {
		if (port == list_entry(iterator, struct ext_gphy_data, list)->port) {
			RTKEXTGPHY_PRINT("Found port: %d - addr: %d", port, list_entry(iterator, struct ext_gphy_data, list)->phy_addr);

			if (list_entry(iterator, struct ext_gphy_data, list)->dev) {
				if (list_entry(iterator, struct ext_gphy_data, list)->dev->drv->auto_neg_set) {
					return list_entry(iterator, struct ext_gphy_data, list)->dev->drv->auto_neg_set(list_entry(iterator, struct ext_gphy_data, list)->phy_addr, ability);
				}
				else
					RTKEXTGPHY_MSG("!!!No auto_neg_set function for PHY at port %u / addr %u",
						list_entry(iterator, struct ext_gphy_data, list)->port,
						list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
			}
			else
				RTKEXTGPHY_ERROR("!!!No driver support for PHY at addr %u", list_entry(iterator, struct ext_gphy_data, list)->phy_addr);
		}
		//else
			//RTKEXTGPHY_PRINT("NOT Found port: %d\n", port);
	}
	return 0;
}

void rtk_ext_gphy_usage(const char *filename)
{
	RTKEXTGPHY_MSG("[Usage]: N/A");
	return;
}


/*
 * ioctl function
 */
#define KERNEL_IOCTL_CMD_MAX_NUM	32
#define KERNEL_IOCTL_CMD_NAME_MAX_LEN	128

typedef int kernel_ioctl_function_t(void *arg);
typedef struct {
	unsigned int cmd;
	char name[KERNEL_IOCTL_CMD_NAME_MAX_LEN];
	kernel_ioctl_function_t * handler;
} kernel_ioctl_cmd_def_t;

#define KERNEL_IOCTL_FUNCTION(func, cmd) \
	int ioctl_##func(void *arg); \
	kernel_ioctl_cmd_def_t func##_cmd = { \
		cmd, \
		#func, \
		ioctl_##func \
	}; \
	int ioctl_##func(void *arg)

#define KERNEL_IOCTL_CMD_INSTALL(func) \
	rtk_extgphy_cmd_reg(&func##_cmd)

/* Define ioctl command and handler */
#define rtk_extgphy_cmd_install() do { \
		KERNEL_IOCTL_CMD_INSTALL(rtk_ext_gphyReg_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtk_ext_gphyReg_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtk_ext_gphyAutoNegoAbility_set); \
	} while (0)

KERNEL_IOCTL_FUNCTION(rtk_ext_gphyReg_get, RTK_EXTGPHY_REG_GET_CMD)
{
	rtk_extgphy_cmd_reg_get_t cmd_reg;

	copy_from_user(&cmd_reg, (struct rtk_extgphy_cmd_reg_get_t *)arg, sizeof(cmd_reg));

	cmd_reg.ret = rtk_ext_gphyReg_get(cmd_reg.port, &cmd_reg.reg);

	copy_to_user((struct rtk_extgphy_cmd_reg_get_t *)arg, &cmd_reg, sizeof(cmd_reg));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtk_ext_gphyReg_set, RTK_EXTGPHY_REG_SET_CMD)
{
	rtk_extgphy_cmd_reg_set_t cmd_reg;

	copy_from_user(&cmd_reg, (struct rtk_extgphy_cmd_reg_set_t *)arg, sizeof(cmd_reg));

	cmd_reg.ret = rtk_ext_gphyReg_set(cmd_reg.port, &cmd_reg.reg);

	copy_to_user((struct rtk_extgphy_cmd_reg_set_t *)arg, &cmd_reg, sizeof(cmd_reg));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtk_ext_gphyAutoNegoAbility_set, RTK_EXTGPHY_AUTO_NEGO_ABILITY_CMD)
{
	rtk_extgphy_cmd_phy_ability_t	portAbility;

	copy_from_user(&portAbility, (struct rtk_extgphy_cmd_phy_ability_t *)arg, sizeof(portAbility));

	portAbility.ret = rtk_ext_gphyAutoNegoAbility_set(portAbility.port, &portAbility.ability);

	copy_to_user((struct rtk_extgphy_cmd_phy_ability_t *)arg, &portAbility, sizeof(portAbility));

	return 0;
}

/* ioctl and cmd function */
static kernel_ioctl_cmd_def_t	g_rtk_extgphy_ioctl_cmd_mapping[KERNEL_IOCTL_CMD_MAX_NUM];
static unsigned int		g_rtk_extgphy_ioctl_cmd_num = 0;

void rtk_extgphy_cmd_reg(kernel_ioctl_cmd_def_t* f_cmd)
{
	if (g_rtk_extgphy_ioctl_cmd_num >= KERNEL_IOCTL_CMD_MAX_NUM) {
		RTKEXTGPHY_ERROR("Reach maxmum commands number\n");
		return;
	}

	g_rtk_extgphy_ioctl_cmd_mapping[g_rtk_extgphy_ioctl_cmd_num].cmd = f_cmd->cmd;
	g_rtk_extgphy_ioctl_cmd_mapping[g_rtk_extgphy_ioctl_cmd_num].handler = f_cmd->handler;
	strncpy(g_rtk_extgphy_ioctl_cmd_mapping[g_rtk_extgphy_ioctl_cmd_num].name, f_cmd->name, KERNEL_IOCTL_CMD_NAME_MAX_LEN - 1);
	g_rtk_extgphy_ioctl_cmd_num ++;
}

void rtk_extgphy_cmd_init(void)
{
	unsigned int i = 0;

	for (i = 0 ; i < KERNEL_IOCTL_CMD_MAX_NUM ; i++) {
		g_rtk_extgphy_ioctl_cmd_mapping[i].cmd = RTK_EXTGPHY_NONE_CMD;
		memset(g_rtk_extgphy_ioctl_cmd_mapping[i].name, 0x00, sizeof(g_rtk_extgphy_ioctl_cmd_mapping[i].name));
		g_rtk_extgphy_ioctl_cmd_mapping[i].handler = NULL;
	}

	g_rtk_extgphy_ioctl_cmd_num = 0;
}

#ifdef RTKEXTGPHY_PHY_DEBUG
void  rtk_extgphy_cmd_scan(void)
{
	unsigned int  i = 0;

	for (i = 0 ; i < g_rtk_extgphy_ioctl_cmd_num ; i++) {
		RTL8367_MSG("cmd: %0x, name: %s, handler: %p",
		g_rtk_extgphy_ioctl_cmd_mapping[i].cmd,
		g_rtk_extgphy_ioctl_cmd_mapping[i].name,
		g_rtk_extgphy_ioctl_cmd_mapping[i].handler);
	}
}
#endif

static long rtk_extgphy_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0, i = 0;

	RTKEXTGPHY_DEBUG("cmd: 0x%x (arg = 0x%x)", cmd, arg);

	for(i = 0 ; i < KERNEL_IOCTL_CMD_MAX_NUM; i++) {
		if(cmd == g_rtk_extgphy_ioctl_cmd_mapping[i].cmd) {
			if(g_rtk_extgphy_ioctl_cmd_mapping[i].handler != NULL) {
				g_rtk_extgphy_ioctl_cmd_mapping[i].handler((void *)arg);
			}
		}
	}

	return ret;
}

static const struct file_operations rtk_extgphy_fops = {
	.owner			= THIS_MODULE,
	//.write		= rtk_extgphy_write,
	//.open			= rtk_extgphy_open,
	//.release		= rtk_extgphy_close,
	.llseek			= no_llseek,
	.unlocked_ioctl		= rtk_extgphy_ioctl,
};

struct miscdevice rtk_extgphy_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = RTK_EXTGPHY_IOCTL_NAME,
	.fops = &rtk_extgphy_fops,
};

void rtk_ext_gphy_ioctl_init(void)
{
	int ret;
	ret = misc_register(&rtk_extgphy_device);
	if (ret)
		RTKEXTGPHY_ERROR("Register a miscellaneous device ERROR.");
	else
		RTKEXTGPHY_MSG("Register a miscellaneous device [//dev//%s]:", RTK_EXTGPHY_IOCTL_NAME);

	rtk_extgphy_cmd_init();
	rtk_extgphy_cmd_install();
#ifdef RTKEXTGPHY_PHY_DEBUG
	rtk_extgphy_cmd_scan();       /* Checking all registered function */
#endif
}


/*
 * proc function
 */
#define PROC_FILE_RTK_EXT_GPHY		"extGphy"
extern struct proc_dir_entry		*realtek_proc;

static int rtk_ext_gphy_read_proc(struct seq_file *seq, void *v)
{
	rtk_ext_gphy_status();
	return 0;
}

static int rtk_ext_gphy_open(struct inode *inode, struct file *file)
{
        return single_open(file, rtk_ext_gphy_read_proc, inode->i_private);
}

static ssize_t rtk_ext_gphy_write(struct file *filp, const char *buf, size_t count, loff_t *offp )
{
	unsigned char tmpBuf[64] = {0};
	char		*strptr, *cmd_addr, *tokptr;
	int len = (count > 63) ? 63 : count;

	if (buf && !copy_from_user(tmpBuf, buf, len))
	{
		tmpBuf[count] = '\0';
		strptr=tmpBuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
			goto errout;

		if (!memcmp(cmd_addr, "debug", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			ext_gphy_debug=simple_strtoul(tokptr, NULL, 16);
			printk(KERN_CONT "set ext_gphy_debug= %d\n", ext_gphy_debug);
		}
#ifdef CONFIG_RTL9607C_SERIES
		else if (!memcmp(cmd_addr, "dump", 4))
		{
			printk(KERN_CONT "mac_reset_count= %d\n", mac_reset_count);
		}
#endif

errout:
		return count;
	}
	return -EFAULT;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
static const struct file_operations rtk_ext_gphy_fops = {
        .owner          = THIS_MODULE,
        .open           = rtk_ext_gphy_open,
        .read           = seq_read,
        .write          = rtk_ext_gphy_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#else
static struct proc_ops rtk_ext_gphy_fops = {
        .proc_open           = rtk_ext_gphy_open,
        .proc_read           = seq_read,
        .proc_write          = rtk_ext_gphy_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#endif

BOOLEAN rtk_ext_gphy_proc_init(void)
{
	struct proc_dir_entry *pe = NULL;

	pe = proc_create_data(PROC_FILE_RTK_EXT_GPHY,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
		realtek_proc, &rtk_ext_gphy_fops, NULL);

	if (!pe)
		RTKEXTGPHY_ERROR("can't create proc entry for rtk_ext_gphy");

	RTKEXTGPHY_MSG("Creat proc entry.");

	return SUCCESS;
}

void rtk_ext_gphy_proc_exit(void)
{
	remove_proc_entry(PROC_FILE_RTK_EXT_GPHY, realtek_proc);
}


/*
 * module function
 */
static int __init rtk_ext_gphy_moudle_init(void)
{
	//ca_status_t ret = CA_E_OK;
	//struct device_node *node;

#ifdef RTK_EXT_GPHY_SET_NI_SDS_MODE
#ifdef CONFIG_LUNA_G3_SERIES
	aal_sds_mode_t		mode;
	aal_sds_speed_t		speed;
#endif
#endif
	RTKEXTGPHY_MSG("RTK External GPHY Module Init.");

	platform_driver_register(&rtk_ext_gphy_driver);

	rtk_ext_gphy_auto_detect();
	rtk_ext_gphy_show_list();
	rtk_ext_gphy_all_init();

#if 0
/* If defined glb_phy in dts
&glb_phy {
	p4_ext_gphy_addr = <6>;
	p5_ext_gphy_addr = <7>;
	p6_ext_gphy_addr = <8>;
	p7_ext_gphy_addr = <9>;
};
*/
	node = of_find_node_by_name(NULL, "phy");
	if (node) {
		ret = of_property_read_u32_index(node, "p4_ext_gphy_addr", 0, &p4PhyAddr);
		if(ret)
			RTKEXTGPHY_ERROR("of_property_read_u32_index Error !! (%d)", ret);
		else
			RTKEXTGPHY_INFO("PHY: P4 phy addr = %d", p4PhyAddr);
	}
	else {
		RTKEXTGPHY_ERROR("of_find_node_by_name Error !!");
	}
#endif

#ifdef RTK_EXT_GPHY_SET_NI_SDS_MODE
#ifdef CONFIG_LUNA_G3_SERIES
	aal_sds_mode_get(0, CA_PORT_ID_NI6, &mode, &speed);
	if (mode == AAL_SDS_MODE_NONE) {
		RTKEXTGPHY_MSG("Set port 6 as XFI (10G)");
		aal_sds_mode_set(0, CA_PORT_ID_NI6, AAL_SDS_MODE_XFI, AAL_SDS_SPEED_DEFAULT);
	}
#endif
#endif
	/* Create rtk ex gphy link status polling kthread */
	rtk_ext_gphy_task = kthread_create(rtk_ext_gphy_thread, NULL, "rtk_ext_gphy/%d", RTK_EX_GPHY_KTHREAD_CPU);
	if (WARN_ON(!rtk_ext_gphy_task)) {
		RTKEXTGPHY_ERROR("Create rtk_ext_gphy/%d failed!", RTK_EX_GPHY_KTHREAD_CPU);
	}
	else {
		kthread_bind(rtk_ext_gphy_task, RTK_EX_GPHY_KTHREAD_CPU);
		wake_up_process(rtk_ext_gphy_task);
	}

	/* Init ioctl device */
	rtk_ext_gphy_ioctl_init();

	/* Init proc device */
	rtk_ext_gphy_proc_init();

	return 0;
}

static void __exit rtk_ext_gphy_module_exit(void)
{
	rtk_ext_gphy_proc_exit();
	rtk_ext_gphy_clear_list();
	platform_driver_unregister(&rtk_ext_gphy_driver);

	RTKEXTGPHY_MSG("%s", __FUNCTION__);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTK EXT GPHY Module");
MODULE_AUTHOR("Realtek.com");

module_init(rtk_ext_gphy_moudle_init);
module_exit(rtk_ext_gphy_module_exit);

