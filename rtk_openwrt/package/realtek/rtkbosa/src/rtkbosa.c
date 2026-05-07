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
#include <errno.h>
#include <getopt.h>

#include <realtek.h>
#include <semtech.h>
#include <uxfastic.h>

static void usage(char *cmd)
{
	printf("Usage: %s [-c|--config] <filename>:		Config Laser Driver with <filename>\n", cmd);
	printf("       %s [-b|--backup] <filename>:		Backup Laser Driver config in <filename>\n", cmd);
	printf("       %s [-B|--lut_generate_bias_current]:	Generate bias current lut, A2 Table 4, 0x80~ 0xe8\n", cmd);
	printf("       %s [-M|--lut_generate_mod_dac]:		Generate mod dac lut, A2 Table 5, 0x80~ 0xe8\n", cmd);
	printf("       %s [-P|--ddmi_k_temp] <temperature (C)>:	Calibrate BOB temperature DDMI <temperature (C)>\n", cmd);
	printf("       %s [-T|--ddmi_k_tx_power] <power (dbm)>:	Calibrate BOB Tx-Power DDMI <power (dbm)>\n", cmd);
	printf("       %s [-R|--ddmi_k_rx_power] <power (dbm)>:	Calibrate BOB Rx-Power DDMI <power (dbm)>\n", cmd);
	printf("       %s [-t|--tx_power_auto_k] <power (dbm)>:	TX PWR Auto K <power (dbm)>\n", cmd);
	printf("       %s [-V|--rx_vapd]:			Calibrate RX VAPD\n", cmd);
	printf("       %s [-L|--rx_los_auto_k] <min max>:	RX_LOS auto calibration <Range>\n", cmd);
	printf("       %s [-E|--rx_error_counter] <counter>:	RX Error counter <Counetr>\n", cmd);
	printf("       %s [-S|--checksum_caculate]:		Caculate checksum\n", cmd);
	printf("       %s [-i|--info]:				Get bosa information\n", cmd);
	//exit(0);
}

static int bosa = 0, withEEPROM = 0;

int rtkbosa_detection()
{
	int rv = 0;
/******************************************************************************
 * Realtek RTL8290B
 ******************************************************************************/
	rv = is_realtek_rtl8290b();
	if (rv == 1) {
		RTKBOSA_INFO("RTL8290B is Found.");
		bosa = RTKBOSA_REALTEK_RTL8290B;
		return rv;
	}
/******************************************************************************
 * Uxfastic UX3320_S
 ******************************************************************************/
	rv = is_uxfastic_ux3320s();
	if (rv < 0)	/* I2C access failed */
		return rv;
	else if (rv == 1) {
		RTKBOSA_INFO("UXFASTIC UX3320_S is Found.");
		bosa = RTKBOSA_UXFASTIC_UX3320S;

		rv = ux3320_is_working();
		if (rv < 0)	/* I2C access failed */
			return rv;
		else if (rv == 0) {
			RTKBOSA_INFO("none EEPROM exists.");
			withEEPROM = 0;
		}
		else if (rv == 1) {
			RTKBOSA_MSG("EEPROM exists, bosa should be working.");
			withEEPROM = 1;
		}
		return 1;
	}
/******************************************************************************
 * Uxfastic UX3360
 ******************************************************************************/
	rv = is_uxfastic_ux3360();
	if (rv < 0)	/* I2C access failed */
		return rv;
	else if (rv == 1) {
		RTKBOSA_INFO("UXFASTIC UX3360 is Found.");
		bosa = RTKBOSA_UXFASTIC_UX3360;

		rv = ux3360_is_working();
		if (rv < 0)	/* I2C access failed */
			return rv;
		else if (rv == 0) {
			RTKBOSA_INFO("none EEPROM exists.");
			withEEPROM = 0;
		}
		else if (rv == 1) {
			RTKBOSA_MSG("EEPROM exists, bosa should be working.");
			withEEPROM = 1;
		}
		return 1;
	}
/******************************************************************************
 * Uxfastic UX3361
 ******************************************************************************/
	rv = is_uxfastic_ux3361();
	if (rv < 0)	/* I2C access failed */
		return rv;
	else if (rv == 1) {
		RTKBOSA_INFO("UXFASTIC UX3361 is Found.");
		bosa = RTKBOSA_UXFASTIC_UX3361;

		rv = ux3361_is_working();
		if (rv < 0)	/* I2C access failed */
			return rv;
		else if (rv == 0) {
			RTKBOSA_INFO("none EEPROM exists.");
			withEEPROM = 0;
		}
		else if (rv == 1) {
			RTKBOSA_MSG("EEPROM exists, bosa should be working.");
			withEEPROM = 1;
		}
		return 1;
	}
/******************************************************************************
 * Semtech GN2xL9x
 ******************************************************************************/
	rv = is_semtech_gn2xl9x();
	if (rv < 0)	/* I2C access failed */
		return rv;
	else if (rv >= 1) {
		bosa = rv;
		if (bosa == RTKBOSA_SEMTECH_GN25L95)
			RTKBOSA_INFO("SEMTECH GN25L95 is Found");
		else if (bosa == RTKBOSA_SEMTECH_GN28L95)
			RTKBOSA_INFO("SEMTECH GN28L95 is Found");
		else if (bosa == RTKBOSA_SEMTECH_GN28L96)
			RTKBOSA_INFO("SEMTECH GN28L96 is Found");
		else if (bosa == RTKBOSA_SEMTECH_GN28L97)
			RTKBOSA_INFO("SEMTECH GN28L97(B) is Found");
		else {
			RTKBOSA_ERROR("Unsupported Semtech bosa: %d", bosa);
			return 0;
		}

		/* Check EEPROM */
		rv = Semtech_check_EEPROM_present(bosa);
		if (rv < 0)	/* I2C access failed */
			return rv;
		else if (rv == 0) {
			RTKBOSA_INFO("none EEPROM exists.");
			withEEPROM = 0;
		}
		else if (rv == 1) {
			RTKBOSA_MSG("EEPROM exists, bosa should be working.");
			withEEPROM = 1;
		}
		return 1;
	}
/******************************************************************************
 * Uxfastic UX3320 (Default)
 ******************************************************************************/
	RTKBOSA_INFO("NOT RTL8290B/GN2xL9x/UX3360, do for UXFASTIC UX3320");
	bosa = RTKBOSA_UXFASTIC_UX3320;

	rv = ux3320_is_working();
	if (rv < 0)	/* I2C access failed */
		return rv;
	else if (rv == 0) {
		RTKBOSA_INFO("none EEPROM exists.");
		withEEPROM = 0;
	}
	else if (rv == 1) {
		RTKBOSA_MSG("EEPROM exists, bosa should be working.");
		withEEPROM = 1;
	}
	return 1;
}

void rtkbosa_info(int bosa)
{
	if (bosa == RTKBOSA_REALTEK_RTL8290B)
		rtl8290b_is_calibrated();
	else if (bosa == RTKBOSA_UXFASTIC_UX3320)
		ux3320_is_calibrated();
	else if (bosa == RTKBOSA_UXFASTIC_UX3320S)
		ux3320s_is_calibrated();
	else if (bosa == RTKBOSA_UXFASTIC_UX3360)
		ux3360_is_calibrated();
	else if (bosa == RTKBOSA_UXFASTIC_UX3361)
		ux3361_is_calibrated();
	else if (bosa == RTKBOSA_SEMTECH_GN25L95)
		gn25l95_is_calibrated();
	else if (bosa == RTKBOSA_SEMTECH_GN28L95)
		gn28l95_is_calibrated();
	else if (bosa == RTKBOSA_SEMTECH_GN28L96)
		gn28l96_is_calibrated();
	else if (bosa == RTKBOSA_SEMTECH_GN28L97)
		gn28l97_is_calibrated();
	else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);

#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
	rtkbosa_print_rtk_pcb_id();
#endif
	return;
}

int main(int argc, char *argv[])
{
	int rv = 0, opt;

	static const char *shortopts = "c:b:BMP:T:RtLEVSi";
	struct option longopts[] = {
		{"config", required_argument, NULL, 'c'},
		{"backup", required_argument, NULL, 'b'},
		{"lut_generate_bias_current", no_argument, NULL, 'B'},
		{"lut_generate_mod_dac", no_argument, NULL, 'M'},
		{"ddmi_k_temp", required_argument, NULL, 'P'},
		{"ddmi_k_tx_power", required_argument, NULL, 'T'},
		{"ddmi_k_rx_power", optional_argument, NULL, 'R'},
		{"tx_power_auto_k", required_argument, NULL, 't'},
		{"rx_vapd", no_argument, NULL, 'V'},
		{"rx_los_auto_k", required_argument, NULL, 'L'},
		{"rx_error_counter", required_argument, NULL, 'E'},
		{"checksum_caculate", no_argument, NULL, 'S'},
		{"info", no_argument, NULL, 'i'},
		{0, 0, 0, 0},
	};

	RTKBOSA_INFO("Version %d.%d (%s - %s)", RTKBOSA_MAJOR_VERSION, RTKBOSA_MINOR_VERSION, __DATE__, __TIME__);
	rtkbosa_i2c_init();

	if (argc < 2){
		RTKBOSA_ERROR("Command error!");
		usage(argv[0]);
		return rv;
	}

	rtkbosa_detection();
	if ((opt = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(opt) {
			case 'c':
				if (bosa == RTKBOSA_REALTEK_RTL8290B) {
					if (insert_rtl8290b() == 0)
						RTKBOSA_ERROR("Insert RTL8290B Fail !!!");
					return rv;
				}

				if (withEEPROM){
					if ((bosa == RTKBOSA_UXFASTIC_UX3320) && (ux3320_internal_multiplier_abnormal()))
						ux3320_external_calibration();
					return rv;
				}

				/* none EEPROM */
				if (bosa == RTKBOSA_UXFASTIC_UX3320) {
					ux3320_init(argv[2]);
					/* UX3320 report exception software handling solution */
					if (ux3320_internal_multiplier_abnormal())
						ux3320_external_calibration();
					//ux3320_external_calibration();	/* for DEBUG */
				}
				else if (bosa == RTKBOSA_UXFASTIC_UX3320S)
					ux3320s_init(argv[2]);
				else if (bosa == RTKBOSA_UXFASTIC_UX3360)
					ux3360_init(argv[2]);
				else if (bosa == RTKBOSA_UXFASTIC_UX3361)
					ux3361_init(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN25L95)
					gn25l95_init(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN28L95)
					gn28l95_init(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN28L96)
					gn28l96_init(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN28L97)
					gn28l97_init(argv[2]);
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);

				rtkbosa_vendor_name_pn_sn(bosa);
				break;
			case 'b':
				if (bosa == RTKBOSA_UXFASTIC_UX3320)
					ux3320_backup(argv[2]);
				else if (bosa == RTKBOSA_UXFASTIC_UX3320S)
					ux3320_backup(argv[2]);
				else if (bosa == RTKBOSA_UXFASTIC_UX3360)
					ux3360_backup(argv[2]);
				else if (bosa == RTKBOSA_UXFASTIC_UX3361)
					ux3361_backup(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN25L95)
					gn25l95_backup(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN28L95)
					gn28l95_backup(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN28L96)
					gn28l96_backup(argv[2]);
				else if (bosa == RTKBOSA_SEMTECH_GN28L97)
					gn28l97_backup(argv[2]);
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'B':
				if ((bosa == RTKBOSA_UXFASTIC_UX3320) || (bosa == RTKBOSA_UXFASTIC_UX3320S))
					ux3320_lut_generate_bias_current();
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'M':
				if ((bosa == RTKBOSA_UXFASTIC_UX3320) || (bosa == RTKBOSA_UXFASTIC_UX3320S))
					ux3320_lut_generate_mod_dac();
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'P':
				if ((bosa == RTKBOSA_UXFASTIC_UX3320) || (bosa == RTKBOSA_UXFASTIC_UX3320S))
					ux3320_ddmi_k_temperature(atof(argv[2]));
				else if (bosa == RTKBOSA_SEMTECH_GN28L95)
					gn28l95_ddmi_k_temperature(atof(argv[2]));
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'T':
				if ((bosa == RTKBOSA_UXFASTIC_UX3320) || (bosa == RTKBOSA_UXFASTIC_UX3320S))
					ux3320_ddmi_k_tx_power(atof(argv[2]));
				else if (bosa == RTKBOSA_SEMTECH_GN28L95)
					gn28l95_ddmi_k_tx_power(atof(argv[2]));
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 't':
				if (bosa == RTKBOSA_UXFASTIC_UX3320)
					ux3320_tx_power_auto_k(atof(argv[2]));
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'R':
				if ((bosa == RTKBOSA_UXFASTIC_UX3320) || (bosa == RTKBOSA_UXFASTIC_UX3320S)) {
					if (argv[2] == NULL)	RTKBOSA_ERROR("Please input rx power (dbm) !!!");
					else			ux3320_ddmi_k_rx_power(atof(argv[2]));
				}
#if defined(CONFIG_LUNA_G3_SERIES)
				else if (bosa == RTKBOSA_SEMTECH_GN28L95)
					gn28l95_ddmi_k_rx_power();
#endif
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'V':
				if (bosa == RTKBOSA_SEMTECH_GN28L95)
					gn28l95_rx_vapd_calibration();
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'L':
				if (bosa == RTKBOSA_UXFASTIC_UX3320) {
					if (argc == 4)
						ux3320_rx_los_auto_calibration(strtol(argv[2], NULL, 16), strtol(argv[3], NULL, 16));
					else {
						RTKBOSA_ERROR("Command error!");
						usage(argv[0]);
					}
				} else
					RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'E':
				if (bosa == RTKBOSA_UXFASTIC_UX3320) {
					if (argc == 3)
						ux3320_rx_error_counter(strtol(argv[2], NULL, 10));
					else {
						RTKBOSA_ERROR("Command error!");
						usage(argv[0]);
					}
				} else
					RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;

			case 'S':
				if ((bosa == RTKBOSA_UXFASTIC_UX3320) || (bosa == RTKBOSA_UXFASTIC_UX3320S))
					ux3320_checksum_caculate();
				else	RTKBOSA_ERROR("Unsupported bosa (%d) !!!", bosa);
				break;
			case 'i':
				rtkbosa_info(bosa);
				break;

			default:
				RTKBOSA_ERROR("Command error!");
				usage(argv[0]);
				return 1;
		}
	}

	printf("\n");
	RTKBOSA_INFO("done");
	return rv;
}

