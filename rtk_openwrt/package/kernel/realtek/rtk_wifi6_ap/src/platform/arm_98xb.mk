ifeq ($(CONFIG_PLATFORM_RTL8198XB), y)

$(info Building Realtek G6 wifi driver for Realtek RTL8198XB...)

CONFIG_PHL_TEST_SUITE = y

$(info Enabled loading PHY parameters from file)
CONFIG_LOAD_PHY_PARA_FROM_FILE = y

EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN -DCONFIG_PLATFORM_RTL8198XB
EXTRA_CFLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT

EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_PRE_CONFIG
EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_POST_CONFIG
EXTRA_CFLAGS += -I$(src)/platform/arm_98xb

ifeq ($(CONFIG_RTW_ENABLE_CUSTOM_PARA_PATH), y)
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=CONFIG_RTW_CUSTOM_PARA_PATH
else
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=\"/etc/conf/\"
endif

EXTRA_CFLAGS += -DCONFIG_FIRMWARE_PATH=\"/var/config/\"

# define necessary variables for building wifi6 driver locally.
ifeq ($(CONFIG_ARM64),)
include $(shell pwd)/../../../../../../.config
include $(shell pwd)/../../../../../../$(CONFIG_LINUXDIR)/.config
endif

DIR_LINUX ?= $(shell pwd)/../../../../../../$(CONFIG_LINUXDIR)
KDIR ?= $(shell pwd)/../../../../../../$(CONFIG_LINUXDIR)
export DIR_LINUX KDIR
ARCH ?= arm64
CROSS_COMPILE ?=
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/
INSTALL_PREFIX :=
STAGINGMODDIR := /lib/modules/$(KVER)/kernel/drivers/staging

# Suppress warnings for simple zero initialization "= {0}".
EXTRA_CFLAGS += -Wno-missing-braces

_PLATFORM_FILES :=

ifeq ($(CONFIG_PCI_HCI), y)
EXTRA_CFLAGS += -DCONFIG_PLATFORM_OPS
_PLATFORM_FILES += platform/platform_arm_98xb_pci.o
endif

# For storage of calibration and configurations
ifeq ($(call check_config,RTW_DRV_HAS_NVM), y)
include $(src)/platform/rtk_ap/nvm.mk
endif

OBJS += $(_PLATFORM_FILES)

ifneq ($(ROMFSDIR),)
$(shell mkdir -p $(ROMFSDIR)/etc/conf)
# rtl8852ae paramters
ifeq ($(CONFIG_RTL8852AE), y)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852ae $(ROMFSDIR)/etc/conf)
endif
# rtl8192xbe paramters
ifeq ($(CONFIG_RTL8192XB), y)
$(shell cp -rf $(src)/platform/mips_98d/rtl8192xbe $(ROMFSDIR)/etc/conf)
endif
# rtl8832bre paramters
ifeq ($(CONFIG_RTL8832BR), y)
$(shell cp -rf $(src)/platform/mips_98d/rtl8832bre $(ROMFSDIR)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8832brvte $(ROMFSDIR)/etc/conf)
endif
# rtl8852ce paramters
ifeq ($(CONFIG_RTL8852CE), y)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852ce $(ROMFSDIR)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8832crvue $(ROMFSDIR)/etc/conf)
endif
# rtl8852de paramters
ifeq ($(CONFIG_RTL8852DE), y)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852de $(ROMFSDIR)/etc/conf)
endif
endif
endif
