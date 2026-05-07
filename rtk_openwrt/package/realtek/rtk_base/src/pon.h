#ifndef INCLUDE_PON_H
#define INCLUDE_PON_H

typedef enum Transceiver_Type_e {
	TRANSCEIVER_TYPE_VENDOR_NAME = 0,
	TRANSCEIVER_TYPE_SN,
	TRANSCEIVER_TYPE_VENDOR_PART_NUM,
	TRANSCEIVER_TYPE_TEMPERATURE,
	TRANSCEIVER_TYPE_VOLTAGE,
	TRANSCEIVER_TYPE_BIAS_CURRENT,
	TRANSCEIVER_TYPE_TX_POWER,
	TRANSCEIVER_TYPE_RX_POWER,
	TRANSCEIVER_TYPE_MAX
} Transceiver_Type_t;

typedef struct TransceiverInfo{
	char vender_name[128];
	char part_number[128];
	char temperature[128];
	char voltage[128];
	char tx_power[128];
	char rx_power[128];
	char bias_current[128];
} TransceiverInfo_T, *TransceiverInfo_Tp;

typedef struct GponStateInfo{
	char onu_state[128];
	char onu_id[128];
	char loid_status[128];
} GponStateInfo_T, *GponStateInfo_Tp;


void formatPloamPasswordToHex(char *src, char* dest, int length);
int rtk_pon_reinit(void);
int rtk_get_transceiver_status(TransceiverInfo_Tp pInfo, int type);
int rtk_get_gpon_state(GponStateInfo_Tp pInfo);

#endif

