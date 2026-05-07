
# Copyright (C) 2009 Realtek Semiconductor Corp.
#
# Makefile for SDK Linux Stacking Module
#

MODULE_SRC_PATH     := $(ROOT_PATH)/module/phy/src
ifeq ($(SDK_BUILD_MODE),linux-user)
        SDK_OSAL_PATH := $(ROOT_PATH)/sdk/system/osal/linux/user
else ifeq ($(SDK_BUILD_MODE),linux-kernel)
        SDK_OSAL_PATH := $(ROOT_PATH)/sdk/system/osal/linux
else
	SDK_OSAL_PATH :=
endif

PHY_EXTRA_UTIL := \
    $(ROOT_PATH)/sdk/src/common/util/rt_bitop.o \
    $(ROOT_PATH)/sdk/system/hwp/hwp.o \
    $(ROOT_PATH)/sdk/system/hwp/hwp_init.o \
    $(ROOT_PATH)/sdk/src/hal/mac/miim_common_drv.o \
    $(ROOT_PATH)/sdk/src/dal/dal_waMon.o \
    $(SDK_OSAL_PATH)/memory.o \
    $(SDK_OSAL_PATH)/sem.o \
    $(SDK_OSAL_PATH)/time.o \
    $(SDK_OSAL_PATH)/thread.o \
    $(MODULE_SRC_PATH)/util/phy_driver_hwp.o \
    $(MODULE_SRC_PATH)/util/phy_wrapped_miim.o \
    $(MODULE_SRC_PATH)/util/phy_misc.o


PHY_EXTRA_PORTING := \
    $(MODULE_SRC_PATH)/phy_hal.o

PHY_EXTRA_INIT := \
    $(MODULE_SRC_PATH)/phy_init.o

PHY_EXTRA := $(PHY_EXTRA_UTIL) $(PHY_EXTRA_PORTING) $(PHY_EXTRA_INIT)
