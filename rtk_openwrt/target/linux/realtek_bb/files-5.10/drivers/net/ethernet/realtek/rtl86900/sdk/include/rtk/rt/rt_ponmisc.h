/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: $
 * $Date: $
 *
 * Purpose : Definition of PON MISC API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) PON Mode Configuration
 *           (2) Transceiver DDMI/Infomation Access
 *           (3) Burst/RX_LOS Polarity Control
 *           (4) PRBS Control
 */
#ifndef __RT_PONMISC_H__
#define __RT_PONMISC_H__

/*
 * Include Files
 */

/*
 * Symbol Definition
 */
#define TRANSCEIVER_LEN 24
#ifdef CONFIG_OE_MODULE_I2C_PORT_1
#define TRANSCEIVER_PORT 1
#else
#define TRANSCEIVER_PORT 0
#endif
#define TRANSCEIVER_A0 0x50
#define TRANSCEIVER_A2 0x51

#define PON_MODE_DISABLE 0xFF

/*
 * Data Declaration
 */
typedef struct rt_ponmisc_sidInfo_s {/*<NOGEN_TAG>*/
    rtk_enable_t enFlag;
    rtk_enable_t dqFlag;
    uint8 dstPort;
    uint8 pri;
    uint16 flowId;
} rt_ponmisc_sidInfo_t;

typedef struct rt_transceiver_data_s{
    uint8 buf[TRANSCEIVER_LEN];	/* Data buffer for the transceiver information */
    uint32 tx_pwr_scale;		/* TX power scale, unit is 0.1 uW */
}rt_transceiver_data_t;

typedef struct rt_ponmisc_prbsRxStatus_s {
    uint32 prbsRxState;		/* Indicate the PRBS RX state is Ready, LOS or NotAvailable */
    uint32 prbsRxErrCnt;		/* PRBS RX Error count */
    uint32 prbsRxErrRate;		/* PRBS RX Error rate */
} rt_ponmisc_prbsRxStatus_t;

/*
 * Macro Declaration
 */
typedef enum{
    RT_GPON_MODE,		/* For GPON/XGPON/XGSPON modes */
    RT_EPON_MODE,		/* For EPON/10G EPON modes */
    RT_NGPON2_MODE,	/* For NG-PON2 mode */
    RT_XFI_MODE,		/* For XFI/10G */
    RT_FIBER_MODE,		/* For 100BASE-X, 1000BASE-X, 2500BASE-X */
    RT_SGMII_MODE,		/* For SGMII/1G, HiSGMII/2.5G */
    RT_USXGMII_MODE,	/* For USXGMII/10G */
    RT_PONMODE_END,
}rt_ponmisc_ponMode_t;

typedef enum{
    RT_1G25G_SPEED,          /* For GPON or EPON */
    RT_DN10G_SPEED,          /* For XG-PON or 10G/1G EPON */
    RT_BOTH10G_SPEED,        /* For XGS-PON or 10G/10G EPON */
    RT_NGP2_DN10G_SPEED,     /* For NG-PON2 with 2.5G Upstream */
    RT_NGP2_BOTH10G_SPEED,   /* For NG-PON2 with 10G Upstream */
    RT_PONSPEED_END,
}rt_ponmisc_ponSpeed_t;

typedef enum{
    RT_100M_SPEED,			/* Speed: 100Mbps */
    RT_1G_SPEED,			/* Speed: 1Gbps */
    RT_2DOT5G_SPEED,		/* Speed: 2.5Gbps */
    RT_5G_SPEED,			/* Speed: 5Gbps */
    RT_10G_SPEED,			/* Speed: 10Gbps */
    RT_10M_SPEED,			/* Speed: 10Mbps */
    RT_ETHSPEED_END,
}rt_ponmisc_ethSpeed_t;

typedef enum{
    RT_PON_QUEUE_FIXED_MODE,	/* Queue mapping mode: Fixed */
    RT_PON_QUEUE_FLOW_MODE,	/* Queue mapping mode: by Flow */
    RT_PON_QUEUE_MODE_END,
}rt_ponmisc_ponQueueMode_t;

typedef enum rt_transceiver_patameter_type_e{
    RT_TRANSCEIVER_PARA_TYPE_VENDOR_NAME = 0,	/* Transceiver Vendor Name */
    RT_TRANSCEIVER_PARA_TYPE_SN,					/* Transceiver Serial Number */
    RT_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM,	/* Transceiver Part Number */
    RT_TRANSCEIVER_PARA_TYPE_TEMPERATURE,			/* Transceiver Temperature */
    RT_TRANSCEIVER_PARA_TYPE_VOLTAGE,				/* Transceiver Voltage */
    RT_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT,		/* Transceiver Bias Current */
    RT_TRANSCEIVER_PARA_TYPE_TX_POWER,			/* Transceiver TX Power */
    RT_TRANSCEIVER_PARA_TYPE_RX_POWER,			/* Transceiver RX Power */
    RT_TRANSCEIVER_PARA_TYPE_MAX
}rt_transceiver_parameter_type_t;

typedef enum rt_ponmisc_laser_status_e{
    RT_PONMISC_LASER_STATUS_NORMAL = 0,		/* Normal, follow PONMAC BEN */
    RT_PONMISC_LASER_STATUS_FORCE_ON = 1,		/* Force Laser ON */
    RT_PONMISC_LASER_STATUS_FORCE_OFF = 2,	/* Force Laser OFF */
    RT_PONMISC_LASER_STATUS_END
}rt_ponmisc_laser_status_t;

typedef enum rt_ponmisc_prbs_e
{
    RT_PONMISC_PRBS_OFF,		/* PRBS OFF */
    RT_PONMISC_PRBS_31,		/* PRBS 31 */
    RT_PONMISC_PRBS_23,		/* PRBS 23 */
    RT_PONMISC_PRBS_15,		/* PRBS 15 */
    RT_PONMISC_PRBS_11,		/* PRBS 11 */
    RT_PONMISC_PRBS_9,		/* PRBS 9 */
    RT_PONMISC_PRBS_7,		/* PRBS 7 */
    RT_PONMISC_PRBS_3,		/* PRBS 3 */
    RT_PONMISC_PRBS_END
}rt_ponmisc_prbs_t;

typedef enum rt_ponmisc_polarity_e{
    RT_PONMISC_POLARITY_REVERSE_OFF=0,	/* Polarity Reverse: OFF */
    RT_PONMISC_POLARITY_REVERSE_ON=1,		/* Polarity Reverse: ON */
    RT_PONMISC_POLARITY_END
}rt_ponmisc_polarity_t;

typedef enum rt_ponmisc_prbs_rx_state_e{
    RT_PONMISC_PRBS_RX_READY=0,	/* PRBS RX is ready*/
    RT_PONMISC_PRBS_RX_LOS=1,           /* Loss of Signal */
    RT_PONMISC_PRBS_RX_NOTAVAILABLE=2,  /* PRBS RX pattern is not matched */
    RT_PONMISC_PRBS_RX_END
}rt_ponmisc_prbs_rx_state_t;

typedef struct rt_ponmisc_rogue_cnt_s{
    uint32   rogue_mismatch; /* self rogue counter for RX_SD mismatch case */
    uint32   rogue_toolong;  /* self rogue counter for TX_SD too long case */
}rt_ponmisc_rogue_cnt_t;

 /*
 * Function Declaration
 */
/* Function Name:
 *      rt_ponmisc_init
 * Description:
 *      Initialize pon misc module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      Must initialize before calling any other APIs in this module.
 */
extern int32
rt_ponmisc_init(void);

/* Function Name:
 *      rt_ponmisc_modeSpeed_get
 * Description:
 *      Get pon mode and pon speed.
 * Input:
 *      None
 * Output:
 *      pPonMode    - current running PON mode
 *      pPonSpeed   - current running PON speed
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *
 */
extern int32
rt_ponmisc_modeSpeed_get(rt_ponmisc_ponMode_t *pPonMode,rt_ponmisc_ponSpeed_t *pPonSpeed);

/* Function Name:
 *      rt_ponmisc_modeSpeed_set
 * Description:
 *      Set pon mode and pon speed.
 * Input:
 *      ponMode    - PON mode
 *      ponSpeed   - PON speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *
 */
extern int32
rt_ponmisc_modeSpeed_set(rt_ponmisc_ponMode_t ponMode,rt_ponmisc_ponSpeed_t ponSpeed);

/* Function Name:
 *      rt_ponmisc_sid_get <NOGEN_TAG>
 * Description:
 *      Get sid configuration.
 * Input:
 *      sid         - stream id
 * Output:
 *      pSidInfo    - corresponding sid infotmation
 * Return:
 *      RT_ERR_OK             - OK
 *      RT_ERR_FAILED         - Failed
 *      RT_ERR_INPUT          - Invalid input parameters.
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      
 */
extern int32
rt_ponmisc_sid_get(uint16 sid,rt_ponmisc_sidInfo_t *pSidInfo);

/* Function Name:
 *      rt_ponmisc_sid_set <NOGEN_TAG>
 * Description:
 *      Set sid configuration.
 * Input:
 *      sid         - stream id
 *      sidInfo     - corresponding sid infotmation
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *
 */
extern int32
rt_ponmisc_sid_set(uint16 sid,rt_ponmisc_sidInfo_t sidInfo);

/* Function Name:
 *      rt_ponmisc_transceiver_get
 * Description:
 *      Get transceiver value by the specific type.
 * Input:
 *      type            - the transceiver parameter type
 * Output:
 *      pData           - the pointer of data for the specific transceiver parameter
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
extern int32
rt_ponmisc_transceiver_get(rt_transceiver_parameter_type_t type, rt_transceiver_data_t *pData);

/* Function Name:
 *              rt_ponmisc_transceiver_tx_power_scale_set
 * Description:
 *              Set transceiver tx power scale.
 * Input:
 *              scale   - scale for TX output power with LSB (in unit 0.1 uW)
 * Output:
 *              N/A
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
extern int32
rt_ponmisc_transceiver_tx_power_scale_set(uint32 scale);

/* Function Name:
 *      rt_ponmisc_burstPolarityReverse_get
 * Description:
 *      Get the status of PON burst polarity reverse.
 * Input:
 *      none
 * Output:
 *      pPolarity       - pointer of burst polarity reverse status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
extern int32
rt_ponmisc_burstPolarityReverse_get(rt_ponmisc_polarity_t *pPolarity);

/* Function Name:
 *      rt_ponmisc_burstPolarityReverse_set
 * Description:
 *      Set the status of PON burst polarity reverse.
 * Input:
 *      polarity        - the burst polarity reverse status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *
 */
extern int32
rt_ponmisc_burstPolarityReverse_set(rt_ponmisc_polarity_t polarity);

/* Function Name:
 *      rt_ponmisc_rxlosPolarityReverse_get
 * Description:
 *      Get the status of PON rx_los polarity reverse.
 * Input:
 *      none
 * Output:
 *      pPolarity       - pointer of rx_los polarity reverse status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
extern int32
rt_ponmisc_rxlosPolarityReverse_get(rt_ponmisc_polarity_t *pPolarity);

/* Function Name:
 *      rt_ponmisc_rxlosPolarityReverse_set
 * Description:
 *      Set the status of PON rx_los polarity reverse.
 * Input:
 *      polarity        - the rx_los polarity reverse status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *
 */
extern int32
rt_ponmisc_rxlosPolarityReverse_set(rt_ponmisc_polarity_t polarity);

/* Function Name:
 *      rt_ponmisc_forceLaserState_get
 * Description:
 *      Get Force Laser status.
 * Input:
 *      none
 * Output:
 *      pStatus       - pointer of Force Laser status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
extern int32
rt_ponmisc_forceLaserState_get(rt_ponmisc_laser_status_t *pStatus);

/* Function Name:
 *      rt_ponmisc_forceLaserState_set
 * Description:
 *      Set Force Laser status.
 * Input:
 *      status       - Force Laser status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *
 */
extern int32
rt_ponmisc_forceLaserState_set(rt_ponmisc_laser_status_t status);

/* Function Name:
 *      rt_ponmisc_forcePRBS_get
 * Description:
 *      Get the PRBS TX config.
 * Input:
 *      none
 * Output:
 *      pPrbsCfg       - pointer of PRBS TX config
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      This API is to get PRBS TX config only.
 */
extern int32
rt_ponmisc_forcePRBS_get(rt_ponmisc_prbs_t *pPrbsCfg);

/* Function Name:
 *      rt_ponmisc_forcePRBS_set
 * Description:
 *      Set the PRBS TX config.
 * Input:
 *      prbsCfg       - PRBS TX config
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is to set PRBS TX config only.
 */
extern int32
rt_ponmisc_forcePRBS_set(rt_ponmisc_prbs_t prbsCfg);

/* Function Name:
 *      rt_ponmisc_forcePRBS_rx_get
 * Description:
 *      Get the PRBS RX config.
 * Input:
 *      none
 * Output:
 *      pPrbsCfg       - pointer of PRBS RX config
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      This API is to get PRBS RX config only.
 */
extern int32
rt_ponmisc_forcePRBS_rx_get(rt_ponmisc_prbs_t *pPrbsCfg);

/* Function Name:
 *      rt_ponmisc_forcePRBS_rx_set
 * Description:
 *      Set the PRBS RX config.
 * Input:
 *      prbsCfg       - PRBS RX config
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is to set PRBS RX config only.
 */
extern int32
rt_ponmisc_forcePRBS_rx_set(rt_ponmisc_prbs_t prbsCfg);

/* Function Name:
 *      rt_ponmisc_prbsRxStatus_get
 * Description:
 *      Get the PRBS RX Status.
 * Input:
 *      none
 * Output:
 *      pPrbsRxStatus       - pointer of PRBS RX Status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      State "RT_PONMISC_PRBS_RX_NOTAVAILABLE" means PRBS RX pattern is not matched.
 */
extern int32
rt_ponmisc_prbsRxStatus_get(rt_ponmisc_prbsRxStatus_t *pPrbsRxStatus);

/* Function Name:
*      rt_ponmisc_selfRogue_cfg_get
* Description:
*      Get the self rogue detect config
* Input:
*      none
* Output:
*      pCfg       - pointer of self rogue detect config
* Return:
*      RT_ERR_OK            - OK
*      RT_ERR_FAILED        - Failed
*      RT_ERR_NULL_POINTER  - input parameter may be null pointer
*      RT_ERR_OUT_OF_RANGE  - value out of range
* Note:
*
*/
extern int32
rt_ponmisc_selfRogue_cfg_get(rtk_enable_t *pCfg);

/* Function Name:
*      rt_ponmisc_selfRogue_cfg_set
* Description:
*      Set the self rogue detect config
* Input:
*      cfg       - self rogue detect config
* Output:
*      none
* Return:
*      RT_ERR_OK           - OK
*      RT_ERR_FAILED       - Failed
*      RT_ERR_INPUT        - Invalid input parameters.
* Note:
*
*/
extern int32
rt_ponmisc_selfRogue_cfg_set(rtk_enable_t cfg);

/* Function Name:
*      rt_ponmisc_selfRogue_count_get
* Description:
*      Get the self rogue detect counter
* Input:
*      none
* Output:
*      pCnt       - pointer of self rogue detect counter
* Return:
*      RT_ERR_OK            - OK
*      RT_ERR_FAILED        - Failed
*      RT_ERR_NULL_POINTER  - input parameter may be null pointer
* Note:
*
*/
extern int32
rt_ponmisc_selfRogue_count_get(rt_ponmisc_rogue_cnt_t *pCnt);

/* Function Name:
*      rt_ponmisc_selfRogue_count_clear
* Description:
*      Clear the self rogue detect counter
* Input:
*      none
* Output:
*      none
* Return:
*      RT_ERR_OK           - OK
*      RT_ERR_FAILED       - Failed
*      RT_ERR_INPUT        - Invalid input parameters.
* Note:
*
*/
extern int32
rt_ponmisc_selfRogue_count_clear(void);
#endif
