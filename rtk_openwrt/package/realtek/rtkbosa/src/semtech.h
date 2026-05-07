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
#ifndef _SEMTECH_H
#define _SEMTECH_H

#include <rtkbosa_utils.h>

int is_semtech_gn2xl9x();

int Semtech_check_EEPROM_present(int bosa);
void Semtech_GN25L95_GN28L95_Initialization_From_MCU(const char *filename, int bosa);
void Semtech_GN25L95_GN28L95_ModuleBackup_Procedure_Rev6(const char *filename, int bosa);

void gn28l95_ddmi_k_temperature(double temp);
void gn28l95_ddmi_k_tx_power(double power);
void gn28l95_ddmi_k_rx_power(void);
void gn28l95_rx_vapd_calibration(void);

void Semtech_GN28L96_GN28L97_Initialization_From_MCU(const char *filename, int bosa);
void Semtech_GN28L96_GN28L97_Digital_Module_Backup_Procedure_Rev1(const char *filename, int bosa);

int gn25l95_is_calibrated();
#define gn25l95_init(filename)		Semtech_GN25L95_GN28L95_Initialization_From_MCU(filename, RTKBOSA_SEMTECH_GN25L95)
#define gn28l95_backup(filename)	Semtech_GN25L95_GN28L95_ModuleBackup_Procedure_Rev6(filename, RTKBOSA_SEMTECH_GN28L95)

int gn28l95_is_calibrated();
#define gn28l95_init(filename)		Semtech_GN25L95_GN28L95_Initialization_From_MCU(filename, RTKBOSA_SEMTECH_GN28L95)
#define gn25l95_backup(filename)	Semtech_GN25L95_GN28L95_ModuleBackup_Procedure_Rev6(filename, RTKBOSA_SEMTECH_GN25L95)

int gn28l96_is_calibrated();
#define gn28l96_init(filename)		Semtech_GN28L96_GN28L97_Initialization_From_MCU(filename, RTKBOSA_SEMTECH_GN28L96)
#define gn28l96_backup(filename)	Semtech_GN28L96_GN28L97_Digital_Module_Backup_Procedure_Rev1(filename, RTKBOSA_SEMTECH_GN28L96)

int gn28l97_is_calibrated();
#define gn28l97_init(filename)		Semtech_GN28L96_GN28L97_Initialization_From_MCU(filename, RTKBOSA_SEMTECH_GN28L97)
#define gn28l97_backup(filename)	Semtech_GN28L96_GN28L97_Digital_Module_Backup_Procedure_Rev1(filename, RTKBOSA_SEMTECH_GN28L97)

#endif	/* _SEMTECH_H */

