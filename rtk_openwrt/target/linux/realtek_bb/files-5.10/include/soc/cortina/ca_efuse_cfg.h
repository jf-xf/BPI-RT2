#ifndef _CA_EFUSE_CFG_H_
#define _CA_EFUSE_CFG_H_

#define ca_phy_param_rtl9607f_t ca_phy_param_rtl8277c_t
#define ca_phy_patch_rtl9607f_t ca_phy_patch_rtl8277c_t
#define ca_phy_k_rtl9607f_t ca_phy_k_rtl8277c_t

#if defined(CONFIG_ARCH_CORTINA_SATURN2) || defined(CONFIG_ARCH_CORTINA_VENUS)
typedef struct ca_phy_param_s {
	uint32_t reserved3_0	: 4;
	uint32_t gphy_cal_cmplt : 1;
	uint32_t reserved3_1	: 3;

	uint32_t sw_patch	: 1;
	uint32_t phy_cmd	: 2;
	uint32_t reserved3_2	: 2;

	uint32_t dgac_lb_dn_up	: 1;
	uint32_t spd_chg	: 1;
	uint32_t reserved3_3	: 1;

	uint32_t en_ado_cal	: 1;
	uint32_t en_rc_cal	: 1;
	uint32_t en_r_cal	: 1;

	uint32_t en_amp_cal	: 1;
	uint32_t disable_500m	: 1;
	uint32_t reserved3_4	: 11;
} ca_phy_param_t;
#else
typedef struct ca_phy_param_s {
        uint32_t sw_patch       : 1;
        uint32_t phy_cmd        : 2;
        uint32_t reserved3_0    : 1;

        uint32_t gphy_cal_cmplt : 1;
        uint32_t dgac_lb_dn_up  : 1;
        uint32_t spd_chg        : 1;
        uint32_t reserved3_1    : 9;

        uint32_t en_ado_cal     : 1;
        uint32_t en_rc_cal      : 1;
        uint32_t en_r_cal       : 1;

        uint32_t en_amp_cal     : 1;
        uint32_t disable_500m   : 1;
        uint32_t reserved3_2    : 11;
} ca_phy_param_t;
#endif

/* param field extended for RTL8277C
 * It should be the same size as ca_phy_patch_t
 */
typedef struct ca_phy_param_rtl8277c_s {
        uint32_t sw_patch       : 1;
        uint32_t phy_cmd        : 2;
        uint32_t reserved3_0    : 1;

        uint32_t gphy_cal_cmplt : 1;
        uint32_t dgac_lb_dn_up  : 1;
        uint32_t spd_chg        : 1;
        uint32_t reserved3_1    : 9;

        uint32_t en_rc_cal_l    : 1;
        uint32_t en_rc_cal_s    : 1;
        uint32_t en_r_cal       : 1;

        uint32_t en_amp_cal     : 1;
        uint32_t disable_500m   : 1;
        uint32_t reserved3_2    : 11;
} ca_phy_param_rtl8277c_t;


typedef struct ca_phy_patch_s {
    uint32_t patch0;
    uint32_t patch1;
    uint32_t patch2;
    uint32_t test_prog_ver;
} ca_phy_patch_t;

/* patch field extended for RTL8277C
 * It should be the same size as ca_phy_patch_t
 */
typedef struct ca_phy_patch_rtl8277c_s {
    uint32_t patch0;
    uint32_t patch1;
    uint32_t patch2;
    uint32_t test_prog_ver      :16;
    uint32_t r_cal_msbs         :16;
} ca_phy_patch_rtl8277c_t;

typedef struct ca_phy_k_u {
	uint32_t rc_cal_len	: 16;
	uint32_t amp_cal	: 16;
	uint32_t adc0_cal	: 16;
	uint32_t r_cal		: 4;
	uint32_t reserved	: 12;
} ca_phy_k_t;

/* k field extended for RTL8277C
 * It should be the same size as ca_phy_k_t
 */
typedef struct ca_phy_k_rtl8277c_s {
	uint32_t rc_cal_len_s	: 16;
	uint32_t amp_cal	    : 16;
	uint32_t rc_cal_len_l	: 16;
	uint32_t r_cal		    : 16;
} ca_phy_k_rtl8277c_t;

typedef struct ca_efuse_cfg_s {
	uint32_t uuid;
	uint32_t hvsid		: 8;
	uint32_t reserved1_0	: 24;
	uint32_t reserved2;

	ca_phy_param_t param;

	uint32_t phy_patch[4];

	ca_phy_k_t phy_k_data[4];
} ca_efuse_cfg_t;

#define EFUSE_PHY_PARAM_OFFSET	0x0C
#define EFUSE_PHY_PATH_OFFSET	0x10
#define EFUSE_PHY_CALIB_OFFSET	0x20

#endif /* _CA_EFUSE_CFG_H_ */
