#
# Copyright (c) 2023, Realtek Semiconductor Corp.
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_PO_VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)

LUCI_TITLE:=LuCI rtk easymesh
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+luci-base

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
