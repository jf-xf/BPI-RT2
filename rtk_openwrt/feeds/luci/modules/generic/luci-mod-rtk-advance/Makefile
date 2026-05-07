#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Realtek LuCI Advance
LUCI_DEPENDS:=+luci-base \
	+luci-app-rtk-radvd +luci-app-rtk-dhcpv6 +luci-mod-rtk-route +luci-mod-rtk-bridging

PKG_LICENSE:=Apache-2.0

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

