#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TYPE:=col
LUCI_BASENAME:=luci-rtk-generic

LUCI_TITLE:=Realtek Generic LuCI interface with Uhttpd as Webserver
LUCI_DESCRIPTION:=OpenWrt set including Realtek generic feature.
LUCI_DEPENDS:= +uhttpd +luci-mod-rtk-generic +luci-theme-rtk-generic

PKG_LICENSE:=Apache-2.0

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
