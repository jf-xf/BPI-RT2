#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "rtk/ponmac.h"
#include "rtk/gpon.h"
#include "rtk/epon.h"
#include "rtk/stat.h"
#include "rtk/switch.h"
#ifdef CONFIG_COMMON_RT_API
#include <rtk/rt/rt_rate.h>
#include <rtk/rt/rt_l2.h>
#include <rtk/rt/rt_qos.h>
#include <rtk/rt/rt_epon.h>
#include <rtk/rt/rt_gpon.h>
#include <rtk/rt/rt_ponmisc.h>
#include <rtk/rt/rt_switch.h>
#include "rt_rate_ext.h"
#include "rt_igmp_ext.h"
#endif

#include <common/util/rt_util.h>

#if defined(CONFIG_RTL9607C_SERIES)
#include <dal/rtl9607c/dal_rtl9607c_switch.h>
#endif
#include "fc_api.h"
#include "pon.h"
#include "sockmark_define.h"
#include "common.h"

#ifdef CONFIG_RTK_OMCI_V1
#include <omci_api.h>
#include <gos_type.h>
#endif

void formatPloamPasswordToHex(char *src, char* dest, int length)
{
	char tmp[10] = {0};
	int i = 0;

	if(strlen(src) > length){
		return;
	}

	for(i=0; i < strlen(src) ; i++){
		sprintf(tmp, "%x", src[i]);
		strcat(dest, tmp);
	}

	if(strlen(src) < length){
		for(i=0; i < (length-strlen(src)); i++){
			strcat(dest, "00");
		}
	}
}

int rtk_pon_reinit(void)
{
	unsigned int gpon_speed=0;
	int ploam_pw_length=GPON_PLOAM_PASSWORD_LENGTH;
	unsigned char password_hex[128]={0};
	unsigned char password[37]={0};
	unsigned char loid[32]={0};
	unsigned char loid_passwd[37]={0};
	char omcicli_cmd[256]={0};
	char buf[256]={0};

	if(mib_get_s2(MIB_PON_SPEED, buf, sizeof(buf)))
		gpon_speed = (unsigned int)atoi(buf);
	
	if(gpon_speed==0){
		ploam_pw_length=GPON_PLOAM_PASSWORD_LENGTH;
	}
	else{
		ploam_pw_length=NGPON_PLOAM_PASSWORD_LENGTH;
	}

	mib_get_s2(MIB_LOID, loid, sizeof(loid));
	mib_get_s2(MIB_LOID_PASSWD, loid_passwd, sizeof(loid_passwd));

	if(loid[0] && loid_passwd[0])
	{
		va_cmd("/bin/omcicli", 4, 1, "set", "loid", loid,loid_passwd);
	}else if ( loid[0] && !loid_passwd[0] )
	{
		va_cmd("/bin/omcicli", 3, 1, "set", "loid", loid);
	}

	printf("GPON Deactivate.\n");
#ifdef CONFIG_COMMON_RT_API
	if(rt_gpon_deactivate() != RT_ERR_OK)
		printf("rt_gpon_deactivate failed: %s %d\n", __func__, __LINE__);

	if(mib_get_s2(MIB_GPON_PLOAM_PASSWD, password, sizeof(password)))
	{
		if(strlen(password) > 0){
			//printf("GPON_PLOAM_PASSWD is %s\n", password);
			formatPloamPasswordToHex(password, password_hex, ploam_pw_length);
			//printf("GPON_PLOAM_PASSWD hex is %s\n", password_hex);
			sprintf(omcicli_cmd , "/sbin/diag rt_gpon set registrationId-hex %s", password_hex);
			//printf("oamcli_cmd is %s\n", oamcli_cmd);
			system(omcicli_cmd);
		}
	}
#endif

	system("omcicli mib reset");

	printf("GPON Activate again.\n");
#ifdef CONFIG_COMMON_RT_API
	rt_gpon_activate(RT_GPON_ONU_INIT_STATE_O1);
#endif

	return 0;
}

int rtk_get_transceiver_status(TransceiverInfo_Tp pInfo, int type)
{
	rt_transceiver_data_t transceiver, readableCfg;
	int ret = 1;

	if (type == TRANSCEIVER_TYPE_VENDOR_NAME || type == TRANSCEIVER_TYPE_MAX)
	{
		//vendor name
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		if ( readableCfg.buf[0] > 0x7F )
			memset(readableCfg.buf, 0x0, TRANSCEIVER_LEN);

		snprintf(pInfo->vender_name, sizeof(pInfo->vender_name) ,"%s", readableCfg.buf);
	}

	if (type == TRANSCEIVER_TYPE_VENDOR_PART_NUM || type == TRANSCEIVER_TYPE_MAX)
	{
		//part number
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		if ( readableCfg.buf[0] > 0x7F )
			memset(readableCfg.buf, 0x0, TRANSCEIVER_LEN);

		snprintf(pInfo->part_number, sizeof(pInfo->part_number) ,"%s", readableCfg.buf);
	}

	if (type == TRANSCEIVER_TYPE_TEMPERATURE || type == TRANSCEIVER_TYPE_MAX)
	{
		//temperature
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_TEMPERATURE, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		snprintf(pInfo->temperature, sizeof(pInfo->temperature) ,"%s", readableCfg.buf);
	}

	if (type == TRANSCEIVER_TYPE_VOLTAGE || type == TRANSCEIVER_TYPE_MAX)
	{
		//voltage
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_VOLTAGE, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_VOLTAGE, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		snprintf(pInfo->voltage, sizeof(pInfo->voltage) ,"%s", readableCfg.buf);
	}

	if (type == TRANSCEIVER_TYPE_TX_POWER || type == TRANSCEIVER_TYPE_MAX)
	{
		//tx-power
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_TX_POWER, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_TX_POWER, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		if(strstr(readableCfg.buf, "-inf"))
			snprintf(pInfo->tx_power, sizeof(pInfo->tx_power) ,"%s", "-inf");
		else
			snprintf(pInfo->tx_power, sizeof(pInfo->tx_power) ,"%s", readableCfg.buf);
	}

	if (type == TRANSCEIVER_TYPE_RX_POWER || type == TRANSCEIVER_TYPE_MAX)
	{
		//rx-power
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_RX_POWER, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		if(strstr(readableCfg.buf, "-inf"))
			snprintf(pInfo->rx_power, sizeof(pInfo->rx_power) ,"%s", "-inf");
		else
			snprintf(pInfo->rx_power, sizeof(pInfo->rx_power) ,"%s", readableCfg.buf);
	}

	if (type == TRANSCEIVER_TYPE_BIAS_CURRENT || type == TRANSCEIVER_TYPE_MAX)
	{
		//bias-current
		memset(&transceiver, 0, sizeof(transceiver));
		memset(&readableCfg, 0, sizeof(readableCfg));
		if(rt_ponmisc_transceiver_get(RT_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver) != RT_ERR_OK) {
			printf("rt_ponmisc_transceiver_get failed: %s %d\n", __func__, __LINE__);
			ret = 0;
		}

		_get_data_by_type(RT_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, (rtk_transceiver_data_t *)&transceiver, (rtk_transceiver_data_t *)&readableCfg);
		snprintf(pInfo->bias_current, sizeof(pInfo->bias_current) ,"%s", readableCfg.buf);
	}

	return ret;
}

static int rtgponstatus2int(int rt_onu_state)
{
	switch (rt_onu_state)
	{
		case RT_GPON_ONU_STATES_O1:
		return 1;
		case RT_GPON_ONU_STATES_O2:
		return 2;
		case RT_GPON_ONU_STATES_O3:
		return 3;
		case RT_GPON_ONU_STATES_O4:
		return 4;
		case RT_GPON_ONU_STATES_O5:
		return 5;
		case RT_GPON_ONU_STATES_O6:
		return 6;
		case RT_GPON_ONU_STATES_O7:
		return 7;
		default: //something wrong
		return 8;
	}
}

const char *loid_get_authstatus(int index)
{
	switch (index)
	{
		case 0:
		return "Initial Status";
		case 1:
		return "Successful Authentication";
		case 2:
		return "LOID Error";
		case 3:
		return "Password Error";
		case 4:
		return "Duplicate LOID";
		default:
		return "WRONG";
	}
}

int rtk_get_gpon_state(GponStateInfo_Tp pInfo)
{
	rt_gpon_onuState_t onu;
	int loid_st = 0;
	rt_gpon_omcc_t rtOmcc;
#ifdef CONFIG_RTK_OMCI_V1
	PON_OMCI_CMD_T msg;
#endif

	//onu state
	rt_gpon_onuState_get(&onu);
	onu = rtgponstatus2int(onu);
	snprintf(pInfo->onu_state, sizeof(pInfo->onu_state) ,"O%d", onu);

	//onu id
	if(rt_gpon_omcc_get(&rtOmcc) != RT_ERR_OK)
		printf("rt_gpon_omcc_get failed: %s %d\n", __func__, __LINE__);

	if(onu != 5)
		snprintf(pInfo->onu_id, sizeof(pInfo->onu_id) ,"-1");
	else
		snprintf(pInfo->onu_id, sizeof(pInfo->onu_id) ,"%d", rtOmcc.allocId);

	//loid status
#ifdef CONFIG_RTK_OMCI_V1
	memset(&msg, 0, sizeof(msg));
	msg.cmd = PON_OMCI_CMD_LOIDAUTH_GET_RSP;

	if(omci_SendCmdAndGet(&msg) == GOS_OK)
		loid_st = msg.state;
	else
		loid_st = -1; // wrong status

	snprintf(pInfo->loid_status, sizeof(pInfo->loid_status) ,"%s", loid_get_authstatus(loid_st));
#endif	

	return 0;
}
