#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Realtek LuCI Page - full-featured for full control
LUCI_DEPENDS:=+luci-base \
	+luci-mod-rtk-status +luci-mod-rtk-lan +luci-mod-rtk-wlan +luci-mod-rtk-wan +luci-mod-rtk-service \
	+luci-mod-rtk-voip +luci-mod-rtk-advance +luci-mod-rtk-diag +luci-mod-rtk-admin +luci-mod-rtk-statistic

PKG_LICENSE:=Apache-2.0

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

