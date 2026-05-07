ifeq ($(CONFIG_PLATFORM_RTL8198D_TAROKO), y)

$(info Building Realtek G6 wifi driver for Realtek RTL8198D TAROKO...)
$(shell ./call_gen_git_info.sh)

CONFIG_PHL_TEST_SUITE = y
CONFIG_RTW_SUPPORT_MBSSID_VAP = y

$(info Enabled loading PHY parameters from file)
CONFIG_LOAD_PHY_PARA_FROM_FILE = y

EXTRA_CFLAGS += -DCONFIG_BIG_ENDIAN -DCONFIG_PLATFORM_RTL8198D_TAROKO
EXTRA_CFLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT

EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_PRE_CONFIG
EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_POST_CONFIG
EXTRA_CFLAGS += -I$(src)/platform/mips_98d

ifeq ($(CONFIG_RTW_ENABLE_CUSTOM_PARA_PATH), y)
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=CONFIG_RTW_CUSTOM_PARA_PATH
else
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=\"/etc/conf/\"
endif

EXTRA_CFLAGS += -DCONFIG_FIRMWARE_PATH=\"/var/\"

ARCH ?= mips
CROSS_COMPILE ?= /toolchain/msdk-4.8.5-mips-EB-4.4-u0.9.33-m32ut-180206/bin/msdk-linux- 

# Suppress warnings for simple zero initialization "= {0}".
EXTRA_CFLAGS += -Wno-missing-braces

_PLATFORM_FILES :=

ifeq ($(CONFIG_PCI_HCI), y)
EXTRA_CFLAGS += -DCONFIG_PLATFORM_OPS
_PLATFORM_FILES += platform/platform_taroko_98d_pci.o
_PLATFORM_FILES += os_dep/ecos/pci_98d_wfo.o
endif

_PLATFORM_FILES += os_dep/ecos/startup.o
#_PLATFORM_FILES += os_dep/ecos/if_g6_wfo.o
_PLATFORM_FILES += os_dep/ecos/g6_wfo.o

# For storage of calibration and configurations
ifeq ($(call check_config,RTW_DRV_HAS_NVM), y)
include $(src)/platform/rtk_ap/nvm.mk
endif

OBJS += $(_PLATFORM_FILES)

endif
