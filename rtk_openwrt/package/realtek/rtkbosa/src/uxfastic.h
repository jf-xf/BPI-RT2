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
#ifndef _UXFASTIC_H
#define _UXFASTIC_H

#include <rtkbosa_utils.h>

int is_uxfastic_ux3320s();
int is_uxfastic_ux3360();
int is_uxfastic_ux3361();
int ux3320_is_working();
int ux3360_is_working();
int ux3361_is_working();

int ux3320_is_calibrated();
void ux3320_init(const char *filename);
void ux3320_backup(const char *filename);
void ux3320s_init(const char *filename);

void ux3320_lut_generate_bias_current(void);
void ux3320_lut_generate_mod_dac(void);
void ux3320_ddmi_k_temperature(double temp);
void ux3320_ddmi_k_tx_power(double power);
void ux3320_ddmi_k_rx_power(double power);
void ux3320_tx_power_auto_k(double power);
void ux3320_rx_los_auto_calibration(unsigned int min, unsigned int max);
void ux3320_rx_error_counter(unsigned int counter);
void ux3320_checksum_caculate(void);

int ux3320s_is_calibrated();

int ux3360_is_calibrated();
void ux3360_init(const char *filename);
void ux3360_backup(const char *filename);

int ux3361_is_calibrated();
void ux3361_init(const char *filename);
void ux3361_backup(const char *filename);
int ux3320_internal_multiplier_abnormal();
void ux3320_external_calibration();
#endif	/* _UXFASTIC_H */

