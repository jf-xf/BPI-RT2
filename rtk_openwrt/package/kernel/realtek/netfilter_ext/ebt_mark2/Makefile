#
# Copyright (C) 2021 Realtek.com
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ebt_mark2
PKG_VERSION:=1.0
PKG_RELEASE:=0

include $(INCLUDE_DIR)/package.mk

MARK2_AUTOLOAD:= \
	ebt_mark2 \
	ebt_mark2_m

define KernelPackage/$(PKG_NAME)
  SECTION:=Kernel modules
  SUBMENU:=Other modules
  TITLE:= SKB MARK2 in ebtables
  FILES:=$(PKG_BUILD_DIR)/module_mark2/src/ebt_mark2.ko $(PKG_BUILD_DIR)/module_mark2_m/src/ebt_mark2_m.ko
  AUTOLOAD:=$(call AutoProbe, $(MARK2_AUTOLOAD))
  KCONFIG:=CONFIG_RTK_SKB_MARK2=y
  DEPENDS:= +ebtables
endef

DIR_LINUX = $(LINUX_DIR)
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
KERNEL_VERSION = $(LINUX_VERSION)
ROOTDIR = $(TOPDIR)
INCDIR = $(STAGING_DIR)
ARCH = $(LINUX_KARCH)

export ARCH DIR_LINUX KERNEL_VERSION ROOTDIR LINUXDIR KDIR INCDIR

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

#Disable Dependencies Check
define CheckDependencies
endef

define KernelPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/ebtables/dyn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libebt_mark2/libebt_mark2.so $(1)/usr/lib/ebtables/dyn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libebt_mark2_m/libebt_mark2_m.so $(1)/usr/lib/ebtables/dyn
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
