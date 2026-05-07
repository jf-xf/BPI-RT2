#
# Copyright (C) 2022 Realtek.com
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk_ther_ctrl
PKG_RELEASE:=1
PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Kernel modules
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek Thermal Control module
  FILES:=$(PKG_BUILD_DIR)/rtk_ther_ctrl.ko
  AUTOLOAD:=$(call AutoLoad,60,$(PKG_NAME))
endef

DIR_LINUX = $(LINUX_DIR)
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
KERNEL_VERSION = $(LINUX_VERSION)
ROOTDIR = $(TOPDIR)
INCDIR = $(STAGING_DIR)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
MODULE_SRC_PATH = `realpath --relative-to=$(LINUX_DIR) $(PKG_BUILD_DIR)`

export DIR_LINUX KERNEL_VERSION ROOTDIR APOLLODIR LINUXDIR KDIR INCDIR

define Build/Prepare
	$(call Build/Prepare/Default)
endef

CFLAGS += $(TARGET_CFLAGS) 
LDFLAGS += $(TARGET_LDFLAGS)

export CFLAGS LDFLAGS

BUILD_MODULES =

ifeq ($(CONFIG_PACKAGE_kmod-rtk_ther_ctrl),y)
  define Build/Compile/$(PKG_NAME)
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(KERNEL_MAKE_FLAGS) \
		M="$(MODULE_SRC_PATH)" \
		modules
  endef
endif

define Build/Compile
	$(call Build/Compile/$(PKG_NAME))
endef

define KernelPackage/rtk_ther_ctrl/install
	@echo -e "\n\n[Installing rtk_ther_ctrl]"
endef

$(eval $(call KernelPackage,$(PKG_NAME)))

