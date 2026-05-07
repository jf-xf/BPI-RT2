#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Realtek LuCI Service
LUCI_DEPENDS:=+luci-base \
	+luci-app-rtk-samba +luci-app-rtk-upnp \
	+luci-app-rtk-firewall +luci-app-rtk-dhcpv4

PKG_LICENSE:=Apache-2.0

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

