/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#include "halrf_precomp.h"

void halrf_watchdog(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	halrf_inn_watchdog(rf);

	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);	
}

#if 0
enum rtw_hal_status halrf_chl_rfk_trigger(void *rf_void,
			   enum phl_phy_idx phy_idx,
			   enum rfk_tri_type rfk_tri_typ)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	halrf_inn_chl_rfk_trigger(rf, phy_idx, rfk_tri_typ);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
	return RTW_HAL_STATUS_SUCCESS;
}
#endif

enum rtw_hal_status halrf_iqk_trigger(void *rf_void,
		       enum phl_phy_idx phy_idx,
		       bool force)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	halrf_inn_iqk_trigger(rf, phy_idx, force);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status halrf_dpk_trigger(void *rf_void,
		       enum phl_phy_idx phy_idx,
		       bool force)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	halrf_inn_dpk_trigger(rf, phy_idx, force);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status halrf_tssi_trigger(void *rf_void, enum phl_phy_idx phy_idx, bool hwtx_en)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	halrf_inn_tssi_trigger(rf, phy_idx, hwtx_en);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
	return RTW_HAL_STATUS_SUCCESS;
}