
##
# Copyright 2019 Shun Li <riverscn@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Firewall and Portforwarding application
LUCI_DEPENDS:=+luci-compat +firewall +kmod-ipt_dns +kmod-ipt_weburl +kmod-ipt_connmark2 +kmod-ipt_mark2

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
