#
# Copyright (C) 2021 Realtek.com
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk_hostspeedup
PKG_VERSION:=1.0
PKG_RELEASE:=0

PKG_BUILD_DEPENDS:=diagshell

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Kernel modules
  SUBMENU:=Other modules
  TITLE:=Realtek HostSpeedUp module
  FILES:=$(PKG_BUILD_DIR)/module/hostSpeedUp.ko
  KCONFIG:=CONFIG_RTK_HOST_SPEEDUP=y
endef

define KernelPackage/$(PKG_NAME)/description
  The speedtest module for TR143/iperf is a software tool that measures the performance
  and quality of a network connection. It uses the TR-143 protocol and the iperf application
  to conduct network throughput tests.
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

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)/module" \
		modules
endef

define Build/InstallDev
	mkdir -p $(1)/include/rtk
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/include/rtk
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
