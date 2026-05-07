#
# Copyright (C) 2021 Realtek.com
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ebt_ftos
PKG_VERSION:=1.0
PKG_RELEASE:=0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Kernel modules
  SUBMENU:=Other modules
  TITLE:= ftos in ebtables
  FILES:=$(PKG_BUILD_DIR)/module/src/ebt_ftos.ko
  AUTOLOAD:=$(call AutoProbe, ebt_ftos)
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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libebt_ftos/libebt_ftos.so $(1)/usr/lib/ebtables/dyn
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
