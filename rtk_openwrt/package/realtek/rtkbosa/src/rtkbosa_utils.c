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
/*
 * Include Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <common/rt_error.h>
#include <rtk/i2c.h>
#include <rtkbosa_utils.h>
#if defined(CONFIG_LUNA_G3_SERIES)
#include <gsl/gsl_multifit.h>
#endif
#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
#include <mib.h>
#include <sysconfig.h>
#endif

#define RTKBOSA_RT_ERR_PRINT(err_num) \
	do { \
            RTKBOSA_MSG("Error (0x%x): %s", err_num, rt_error_numToStr(err_num)); \
   	   } while (0)

#define LD_ADDR_MAX		(255)
#define I2C_WIDTH_8bit		(0)

#if (defined CONFIG_OE_MODULE_I2C_PORT_0)
	#define I2CPORT 0
#elif (defined CONFIG_OE_MODULE_I2C_PORT_1)
	#define I2CPORT 1
#else
	#error "I2C Port No. for Laser Driver is NOT defined !!!"
#endif

int32 rtkbosa_i2c_init()
{
	int ret = RT_ERR_OK;

	if((ret = rtk_i2c_init(I2CPORT)) != RT_ERR_OK)
		RTKBOSA_RT_ERR_PRINT(ret);
	return ret;
}

int32 rtkbosa_byte_read(uint32 devID, uint32 addr, uint8 *data)
{
	int ret = RT_ERR_OK;
	uint32 value;

	if(addr > LD_ADDR_MAX)
		return -1;

	if(NULL == data)
		return -1;

#ifndef CONFIG_LUNA_G3_SERIES
	/* Configure I2C 8 bits data */
	if((ret = rtk_i2c_dataWidth_set(I2CPORT, I2C_WIDTH_8bit)) != RT_ERR_OK) {
		RTKBOSA_RT_ERR_PRINT(ret);
		return ret;
	}
#endif

	/* Configure I2C dev ID
	 * The access to different page is through different device ID
	 */
	//rtk_i2c_read(I2CPORT, (addr >= 256) ? 0x51 : 0x50, addr % 256, &value);
	if((ret = rtk_i2c_read(I2CPORT, devID, addr, &value)) != RT_ERR_OK) {
		RTKBOSA_RT_ERR_PRINT(ret);
		return ret;
	}
	*data = (uint8) value;

	return ret;
}

int32 rtkbosa_byte_write(uint32 devID, uint32 addr, uint8 data)
{
	int ret = RT_ERR_OK;
	uint32 value;

	if(addr > LD_ADDR_MAX)
		return -1;

#ifndef CONFIG_LUNA_G3_SERIES
	/* Configure I2C 8 bits data */
	if((ret = rtk_i2c_dataWidth_set(I2CPORT, I2C_WIDTH_8bit)) != RT_ERR_OK) {
		RTKBOSA_RT_ERR_PRINT(ret);
		return ret;
	}
#endif

	/* Configure I2C dev ID
	 * The access to different page is through different device ID
	 */
	value = (uint32) data;
	//rtk_i2c_write(I2CPORT, (addr >= 256) ? 0x51 : 0x50, addr % 256, value);
	if((ret = rtk_i2c_write(I2CPORT, devID, addr, value)) != RT_ERR_OK) {
		RTKBOSA_RT_ERR_PRINT(ret);
		return ret;
	}

	return ret;
}

#define A0_VENDOR_NAME_PN_SN_FIELDS_SIZE	(16)
uint32 rtkbosa_vendor_name_pn_sn(int bosa)
{
	unsigned char vendor_name[A0_VENDOR_NAME_PN_SN_FIELDS_SIZE + 1] = {0};
	unsigned char vendor_pn[A0_VENDOR_NAME_PN_SN_FIELDS_SIZE + 1] = {0};
	unsigned char vendor_sn[A0_VENDOR_NAME_PN_SN_FIELDS_SIZE + 1] = {0};
#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
	unsigned char vendor_data[A0_VENDOR_NAME_PN_SN_FIELDS_SIZE + 1] = {0};
#endif
	unsigned int i = 0, isFF = 1;
	uint8 regData;
	int is_semtech = 0;

	unsigned char* str_realtek = "Realtek";
	unsigned char* str_rtl8290b = "RTL8290B";

	unsigned char* str_semtech = "Semtech";
	unsigned char* str_gn25l95 = "GN25L95";
	unsigned char* str_gn28l95 = "GN28L95";
	unsigned char* str_gn28l96 = "GN28L96";
	unsigned char* str_gn28l97 = "GN28L97(B)";

	unsigned char* str_uxfastic = "Uxfastic";
	unsigned char* str_ux3320 = "UX3320";
	unsigned char* str_ux3320s = "UX3320_S";
	unsigned char* str_ux3360 = "UX3360";
	unsigned char* str_ux3361 = "UX3361";

	unsigned char* str_unknown = "Unknown";

	switch(bosa) {
	case RTKBOSA_REALTEK_RTL8290B:
		strncpy(vendor_name, str_realtek, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_rtl8290b, sizeof(vendor_pn)-1);
		break;
	case RTKBOSA_SEMTECH_GN25L95:
		strncpy(vendor_name, str_semtech, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_gn25l95, sizeof(vendor_pn)-1);
		is_semtech = 1;
		break;
	case RTKBOSA_SEMTECH_GN28L95:
		strncpy(vendor_name, str_semtech, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_gn28l95, sizeof(vendor_pn)-1);
		is_semtech = 1;
		break;
	case RTKBOSA_SEMTECH_GN28L96:
		strncpy(vendor_name, str_semtech, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_gn28l96, sizeof(vendor_pn)-1);
		is_semtech = 1;
		break;
	case RTKBOSA_SEMTECH_GN28L97:
		strncpy(vendor_name, str_semtech, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_gn28l97, sizeof(vendor_pn)-1);
		is_semtech = 1;
		break;
	case RTKBOSA_UXFASTIC_UX3320:
		strncpy(vendor_name, str_uxfastic, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_ux3320, sizeof(vendor_pn)-1);
		break;
	case RTKBOSA_UXFASTIC_UX3320S:
		strncpy(vendor_name, str_uxfastic, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_ux3320s, sizeof(vendor_pn)-1);
		break;
	case RTKBOSA_UXFASTIC_UX3360:
		strncpy(vendor_name, str_uxfastic, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_ux3360, sizeof(vendor_pn)-1);
		break;
	case RTKBOSA_UXFASTIC_UX3361:
		strncpy(vendor_name, str_uxfastic, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_ux3361, sizeof(vendor_pn)-1);
		break;
	default:
		strncpy(vendor_name, str_unknown, sizeof(vendor_name)-1);
		strncpy(vendor_pn, str_unknown, sizeof(vendor_pn)-1);
		break;
	}

	/* In MCU mode, Semtech GN2xL9x only responds to I2C device address A2h. */
	if (is_semtech) {
		RTKBOSA_MSG("[LD] Do NOT save A0 data for Semtech BOSA");
	}
	else {
		/* Check the Vendor name or PN is set or not */
		for(i = 0 ; i < A0_VENDOR_NAME_PN_SN_FIELDS_SIZE ; i++){
			rtkbosa_byte_read(TRANSCEIVER_A0, 0x14 + i, &regData);
			isFF &= (regData == 0xFF);
			rtkbosa_byte_read(TRANSCEIVER_A0, 0x28 + i, &regData);
			isFF &= (regData == 0xFF);
			rtkbosa_byte_read(TRANSCEIVER_A0, 0x44 + i, &regData);
			isFF &= (regData == 0xFF);
		}
		if (!isFF) {
			RTKBOSA_INFO("The Vendor Name or PN or SN is not empty, do NOT set it.");
		}
		else {
			/* Clear Data */
			for(i = 0 ; i < A0_VENDOR_NAME_PN_SN_FIELDS_SIZE ; i++){
				rtkbosa_byte_write(TRANSCEIVER_A0, 0x14 + i, 0);
				msleep(1);	/* for UX3361 issue, delay 1ms after i2c write */
				rtkbosa_byte_write(TRANSCEIVER_A0, 0x28 + i, 0);
				msleep(1);	/* for UX3361 issue, delay 1ms after i2c write */
				rtkbosa_byte_write(TRANSCEIVER_A0, 0x44 + i, 0);
				msleep(1);	/* for UX3361 issue, delay 1ms after i2c write */
			}

			RTKBOSA_MSG("[LD] Set Vender Name : %s (len = %d)", vendor_name, strlen(vendor_name));
			RTKBOSA_MSG("[LD] Set Vender PN : %s (len = %d)", vendor_pn, strlen(vendor_pn));
			for(i = 0 ; i < (strlen(vendor_name)) && (i <= A0_VENDOR_NAME_PN_SN_FIELDS_SIZE) ; i++) {
				rtkbosa_byte_write(TRANSCEIVER_A0, 0x14 + i, vendor_name[i]);
				msleep(1);	/* for UX3361 issue, delay 1ms after i2c write */
			}
			for(i = 0 ; i < (strlen(vendor_pn)) && (i <= A0_VENDOR_NAME_PN_SN_FIELDS_SIZE) ; i++) {
				rtkbosa_byte_write(TRANSCEIVER_A0, 0x28 + i, vendor_pn[i]);
				msleep(1);	/* for UX3361 issue, delay 1ms after i2c write */
			}
#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
			mib_get_s(MIB_BOSA_SERIAL_NUMBER, &vendor_data, sizeof(vendor_data));
			RTKBOSA_MSG("[LD] Set Vender SN : %s (len = %d)", vendor_data, strlen(vendor_data));
			for(i = 0 ; i < (strlen(vendor_data)) && (i <= A0_VENDOR_NAME_PN_SN_FIELDS_SIZE) ; i++) {
				rtkbosa_byte_write(TRANSCEIVER_A0, 0x44 + i, vendor_data[i]);
				msleep(1);	/* for UX3361 issue, delay 1ms after i2c write */
			}
#endif
		}
	}
#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
	mib_get_s(MIB_BOSA_VENDOR_NAME, &vendor_data, sizeof(vendor_data));
	if (strncmp(vendor_name, vendor_data, sizeof(vendor_name)) != 0) {
		RTKBOSA_MSG("[MIB] Old BOSA_VENDOR_NAME: \"%s\", Set it as \"%s\"", vendor_data, vendor_name);
		if ( !mib_set(MIB_BOSA_VENDOR_NAME, (void *)&vendor_name) ) {
			RTKBOSA_ERROR("[MIB] set BOSA_VENDOR_NAME Error");
		}
	}
	mib_get_s(MIB_BOSA_PART_NUMBER, &vendor_data, sizeof(vendor_data));
	if (strncmp(vendor_pn, vendor_data, sizeof(vendor_pn)) != 0) {
		RTKBOSA_MSG("[MIB] Old BOSA_PART_NUMBER: \"%s\", Set it as \"%s\"", vendor_data, vendor_pn);
		if ( !mib_set(MIB_BOSA_PART_NUMBER, (void *)&vendor_pn) ) {
			RTKBOSA_ERROR("[MIB] set MIB_BOSA_PART_NUMBER Error");
		}
	}
#endif
	return 0;
}

uint32 rtkbosa_checksum(uint8 *addr, int32 len)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	//register int32 sum = 0;
	uint32 sum = 0;
	uint8 *source = (uint8 *) addr;

	while (len > 0) {
		/*  This is the inner loop */
		sum += *source++;
		len -= 1;
	}

	/*  Fold 32-bit sum to 16 bits */
	//while (sum >> 16)
	//	sum = (sum & 0xffff) + (sum >> 16);

	//sum = ~sum;
	//sum &= 0xFF;

	return sum;
}

#if defined(CONFIG_LUNA_G3_SERIES)
uint32 polynomialfit(int obs, int degree, double *dx, double *dy, double *store) /* n, p */
{
	gsl_multifit_linear_workspace *ws;
	gsl_matrix *cov, *X;
	gsl_vector *y, *c;
	double chisq;

	int i, j;

	X = gsl_matrix_alloc(obs, degree);
	y = gsl_vector_alloc(obs);
	c = gsl_vector_alloc(degree);
	cov = gsl_matrix_alloc(degree, degree);

	for(i=0; i < obs; i++) {
		for(j=0; j < degree; j++) {
			gsl_matrix_set(X, i, j, pow(dx[i], j));
		}
		gsl_vector_set(y, i, dy[i]);
	}

	ws = gsl_multifit_linear_alloc(obs, degree);
	gsl_multifit_linear(X, y, c, cov, &chisq, ws);

	/* store result ... */
	for(i=0; i < degree; i++)
	{
		store[i] = gsl_vector_get(c, i);
	}

	gsl_multifit_linear_free(ws);
	gsl_matrix_free(X);
	gsl_matrix_free(cov);
	gsl_vector_free(y);
	gsl_vector_free(c);

	/* we do not "analyse" the result (cov matrix mainly) to know if the fit is "good" */
	return 0;
}
#endif

#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
#define MIB_RTK_PCB_ID_FIELDS_SIZE	(16)
void rtkbosa_print_rtk_pcb_id(void)
{
	unsigned char rtk_pcb_id[MIB_RTK_PCB_ID_FIELDS_SIZE + 1] = {0};

	mib_get_s(MIB_RTK_PCB_ID, &rtk_pcb_id, sizeof(rtk_pcb_id));
	RTKBOSA_MSG("[MIB] RTK PCB ID : %s", rtk_pcb_id);

	return;
}
#endif

