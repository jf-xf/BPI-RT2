#
# Copyright (C) 2021 Realtek.com
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=smuxdev
PKG_VERSION:=1.0
PKG_RELEASE:=0

PKG_APP_NAME:=smuxctl
PKG_BUILD_DEPENDS:=diagshell

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Kernel modules
  SUBMENU:=Other modules
  TITLE:=Realtek SMUX device
  FILES:=$(PKG_BUILD_DIR)/module/src/smuxdev.ko
  AUTOLOAD:=$(call AutoLoad,91, $(PKG_NAME))
  KCONFIG:=CONFIG_RTL_MULTI_ETH_WAN=y CONFIG_RTL_SMUX_DEV=y
endef

define Package/$(PKG_APP_NAME)
  SECTION:=utils
  CATEGORY:=Realtek
  TYPE:=boolean
  TITLE:=Administrate for Realtek SMUX device ($(PKG_NAME))
  DEPENDS:=@PACKAGE_kmod-$(PKG_NAME)
  DEFAULT:=y if PACKAGE_kmod-$(PKG_NAME)
endef

define Package/$(PKG_APP_NAME)/description
  This package is used to administrate Realtek SMUX device ($(PKG_NAME)) driver from user.
endef

DIR_LINUX = $(LINUX_DIR)
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
KERNEL_VERSION = $(LINUX_VERSION)
ROOTDIR = $(TOPDIR)
INCDIR = $(STAGING_DIR)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
ARCH = $(LINUX_KARCH)

export ARCH DIR_LINUX KERNEL_VERSION ROOTDIR APOLLODIR LINUXDIR KDIR INCDIR

define Build/Prepare
	$(call Build/Prepare/Default)
endef

CFLAGS += $(TARGET_CFLAGS) 
LDFLAGS += $(TARGET_LDFLAGS)

export CFLAGS LDFLAGS PON_CFLAGS EXTRA_CFLAGS

define Build/Compile
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		all
endef

define KernelPackage/$(PKG_NAME)/install
	@echo -e "\n\n[Installing smuxdev]"
	#$(MAKE) -C $(PKG_BUILD_DIR)/module \
	#	modules_install
endef

define Package/$(PKG_APP_NAME)/install
	@echo -e "\n\n[Installing smuxctl]"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/$(PKG_APP_NAME) $(1)/bin/
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_APP_NAME)))
