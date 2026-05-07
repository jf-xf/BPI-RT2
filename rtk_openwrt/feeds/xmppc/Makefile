#
# Copyright (C) 2020 iopsys Software Solutions AB
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xmppc
PKG_VERSION:=2.0.2

LOCAL_DEV:=0
ifneq ($(LOCAL_DEV),1)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://dev.iopsys.eu/iopsys/xmppc.git
PKG_SOURCE_VERSION:=63ba23c1dc5de3d136a3c1568ae9de08762fd0d0
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=skip
endif

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=bbf

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=TRx69
  TITLE:=BBF xmpp Client
  DEPENDS:=+libuci +libubox +libstrophe +libubus
endef

define Package/$(PKG_NAME)/description
	BBF XMPP Client
endef

ifeq ($(LOCAL_DEV),1)
define Build/Prepare
	$(CP) -rf ~/git/xmppc/* $(PKG_BUILD_DIR)/
endef
endif

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-Wall -Werror

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xmppc $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib/bbfdm
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/bbfdm
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
