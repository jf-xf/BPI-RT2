/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <realtek.h>
//#include <rtk/ldd.h>
#include <../src/app/europa/inc/europa_cli.h>

#define RTL8290B_KERNEL_MODULE_FILE	"/lib/modules/europa_drv.ko"
#define RTL8290B_INSERT_MODULE_SCRIPT	"/etc/scripts/insert_europa.sh"

int is_realtek_rtl8290b() {
	int rv = 1;
	uint8 regData;

	RTKBOSA_MSG("Detecting RTL8290B ...");

	if ((rtkbosa_byte_read(0x55, 0x90, &regData)) == 0) {
		RTKBOSA_MSG("Read 0x55.0x90: 0x%02x", regData);
		rv &= (regData == 0x82);
	}
	else
		return 0;

	if ((rtkbosa_byte_read(0x55, 0x91, &regData)) == 0) {
		RTKBOSA_MSG("Read 0x55.0x91: 0x%02x", regData);
		rv &= (regData == 0x90);
	}
	else
		return 0;

	if ((rtkbosa_byte_read(0x55, 0x94, &regData)) == 0) {
		RTKBOSA_MSG("Read 0x55.0x94: 0x%02x", regData);
		rv &= (regData == 0x01);
	}
	else
		return 0;

	return rv;
}

int insert_rtl8290b() {
	int rv = 0;
	char command[128];

	if (access(RTL8290B_INSERT_MODULE_SCRIPT, 0) != 0) {
		RTKBOSA_ERROR("Insert script for rtl8290b does NOT exist, init fail !!!");
		rv = 0;
	}
	else {
		/* Execute insert script */
		snprintf(command, sizeof(command), "/bin/sh %s", RTL8290B_INSERT_MODULE_SCRIPT);
		printf("$ %s", command);
		system (command);
		rv = 1;
	}

	return rv;
}

void _europa_cal_short_data_get(uint8 *ptr_data, uint32 length, uint32 *value)
{
	uint8  i, temp8;
	uint32 temp32;

	if ((length==0)||(length>4))
	{
	     printf("Data Length Error!!!!!!!\n");
	     return;
	}
	temp32 = 0;
	for(i=0;i<length;i++)
	{
	    temp8 = *ptr_data;
	    temp32 = temp32 | ((uint32)temp8<<(8*((length-1)-i)));
	    ptr_data++;
	}

	//printf("%s = 0x%x!!!!!!!\n", __FUNCTION__, temp32);

	*value = temp32;
	return;
}

void _europa_cal_flash_data_get(uint32 address, uint32 length, uint32 *value)
{
	FILE *fp;
	uint8 *init_data, *ptr_data;

	fp = fopen(RTL8290B_FILE_LOCATION,"rb");
	if (NULL ==fp)
	{
	    printf("Open file in /var/config/ error!!!!!!!\n");
	    return;
	}

	init_data = (uint8 *)malloc(RTL8290B_PARAMETER_SIZE*sizeof(uint8));

	fseek(fp, 0, SEEK_SET);
	memset(init_data, 0x0, sizeof(uint8)*RTL8290B_PARAMETER_SIZE);
	fread(init_data, 1, RTL8290B_PARAMETER_SIZE, fp);
	ptr_data = init_data;

	ptr_data = ptr_data + address;
	_europa_cal_short_data_get(ptr_data, length, value);

	free(init_data);
	fclose(fp);

	return;
}

/* Need CONFIG_EUROPA_FEATURE */
#if 0
static int _europa_rx_parameter_get(europa_rxparam_t *ldd_rxparam)
{
	int ret;
	rtk_ldd_cfg_t ldd_cfg;

	memset(&ldd_cfg, 0 , sizeof(rtk_ldd_cfg_t));
	ldd_cfg.rssi_v0 = 0x8290B;
	ret = rtk_ldd_rssiVoltage_get(&ldd_cfg);
	if(ret)
	{
		printf("Get parameter Fail(%d)\n", ret);
		return -1;
	}

	ldd_rxparam->rssi_v0 = ldd_cfg.rssi_v0;
	ldd_rxparam->vdd_v0 = ldd_cfg.vdd_code;
	ldd_rxparam->gnd_v0 = ldd_cfg.gnd_code;
	ldd_rxparam->half_v0 = ldd_cfg.half_vdd_code;
	ldd_rxparam->rssi_r1 = ldd_cfg.tx_bias;
	ldd_rxparam->rssi_r2 = ldd_cfg.tx_mod;
	ldd_rxparam->rssi_k = (int)ldd_cfg.rssi_code;
	ldd_rxparam->rx_a   = (int)ldd_cfg.ldo_code;
	ldd_rxparam->rx_b   = (int)ldd_cfg.rssi_voltage;
	ldd_rxparam->rx_c   = (int)ldd_cfg.rssi_voltage2;

	printf("Current RX power RSSI V0 = %d\n", ldd_cfg.rssi_v0);
	printf("Current RX power VDD V0 = %d\n", ldd_cfg.vdd_code);
	printf("Current RX power GND V0 = %d\n", ldd_cfg.gnd_code);
	printf("Current RX power Half VDD V0 = %d\n", ldd_cfg.half_vdd_code);
	printf("Current RX power R1 = %d\n", ldd_cfg.tx_bias);
	printf("Current RX power R2 = %d\n", ldd_cfg.tx_mod);
	printf("Current RX power RSSI-K = %d\n", (int)ldd_cfg.rssi_code);
	printf("Current RX power rx_a = %d\n", ldd_rxparam->rx_a);
	printf("Current RX power rx_b = %d\n", ldd_rxparam->rx_b);
	printf("Current RX power rx_c = %d\n", ldd_rxparam->rx_c);

	return 0;
}
#endif

int rtl8290b_is_calibrated() {
	uint32 rv = 0, addr, para1;

	/* europa_cli_txparam_get */
	addr = RTL8290B_TX_A_ADDR;
	_europa_cal_flash_data_get(addr, 4, &para1);
	printf("TX parameter a: flash = %d\n",  para1);
	rv |= para1;
	addr = RTL8290B_TX_B_ADDR;
	_europa_cal_flash_data_get(addr, 4, &para1);
	printf("TX parameter b: flash = %d\n", para1);
	addr = RTL8290B_TX_C_ADDR;
	_europa_cal_flash_data_get(addr, 4, &para1);
	printf("TX parameter c: flash = %d\n", para1);

	/* europa_cli_rxparam_get */
	//europa_rxparam_t ldd_rxparam;

	addr = RTL8290B_RX_A_ADDR;
	_europa_cal_flash_data_get(addr, 4, &para1);
	printf("RX parameter a: flash = %d\n",  para1);
	addr = RTL8290B_RX_B_ADDR;
	_europa_cal_flash_data_get(addr, 4, &para1);
	printf("RX parameter b: flash = %d\n", para1);
	rv |= para1;
	addr = RTL8290B_RX_C_ADDR;
	_europa_cal_flash_data_get(addr, 4, &para1);
	printf("RX parameter c: flash = %d\n", para1);

	/* Need CONFIG_EUROPA_FEATURE */
	//memset(&ldd_rxparam, 0, sizeof(europa_rxparam_t));
	//_europa_rx_parameter_get(&ldd_rxparam);

	/* The rtl8290b is not calibrated if both rx_b and tx_b are 0 */
	if (rv)
		RTKBOSA_INFO("Bosa is Calibrated");
	else
		RTKBOSA_ERROR("Bosa is NOT Calibrated");

	return rv;
}

