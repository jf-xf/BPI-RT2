#
# Copyright (C) 2022 iopsys Software Solutions AB
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bulkdata
PKG_VERSION:=2.0.3.1

LOCAL_DEV:=0
ifneq ($(LOCAL_DEV),1)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://dev.iopsys.eu/iopsys/bulkdata.git
PKG_SOURCE_VERSION:=33a6648de9ee0af33c44518656b56b0a30b6c1ab
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
  TITLE:=BBF BulkData Collection
  DEPENDS:=+libubus +libuci +libubox +libjson-c +libcurl +libblobmsg-json +zlib
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-Wall -Werror

ifeq ($(LOCAL_DEV),1)
define Build/Prepare
	$(CP) -rf ~/git/bulkdata/* $(PKG_BUILD_DIR)/
endef
endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bulkdatad $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib/bbfdm
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bbf_plugin/*.so $(1)/usr/lib/bbfdm
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
