#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sock_opt_ext
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/sk_rcvmark
  SECTION:=Kernel modules
  SUBMENU:=Other modules
  TITLE:=Socket support get SKB MARK by recvmsg
  KCONFIG:= CONFIG_SOCK_RCVMARK=y
endef

define KernelPackage/sk_rcvmark/description
Support get SKB MARK on socket by recvmsg
endef


define KernelPackage/sk_rcvmark/install
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define KernelPackage/depends
endef

$(eval $(call KernelPackage,sk_rcvmark))

