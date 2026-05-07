#
# Copyright (C) 2020-2022 IOPSYS Software Solutions AB
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stunc
PKG_RELEASE:=1
PKG_VERSION:=1.1.12

LOCAL_DEV:=0
ifneq ($(LOCAL_DEV),1)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://dev.iopsys.eu/iopsys/stunc.git
PKG_SOURCE_VERSION:=5b65364709a921db0956bf74a4f126217cfb1775
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=skip
endif

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=bbf

include $(INCLUDE_DIR)/package.mk

define Package/stunc/default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=TRx69
  TITLE:=BBF STUN Client
  DEPENDS:=+libubus +libuci +libubox +libjson-c +libblobmsg-json
endef

define Package/stunc-mbedtls
  $(Package/stunc/default)
  TITLE += (mbedtls)
  DEPENDS += +PACKAGE_stunc-mbedtls:libmbedtls
  VARIANT:=mbedtls
  DEFAULT_VARIANT:=1
endef

define Package/stunc-openssl
  $(Package/stunc/default)
  TITLE += (openssl)
  DEPENDS += +PACKAGE_stunc-openssl:libopenssl
  CONFLICTS := stunc-mbedtls
  VARIANT:=openssl
endef

define Package/stunc-wolfssl
  $(Package/stunc/default)
  TITLE += (wolfssl)
  DEPENDS += +PACKAGE_stunc-wolfssl:libwolfssl
  CONFLICTS := stunc-mbedtls stunc-openssl
  VARIANT:=wolfssl
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-Wall -Werror

ifeq ($(BUILD_VARIANT),wolfssl)
  MAKE_FLAGS += \
	USE_WOLFSSL="yes"
endif

ifeq ($(BUILD_VARIANT),mbedtls)
  MAKE_FLAGS += \
	USE_MBEDTLS="yes"
endif

ifeq ($(BUILD_VARIANT),openssl)
  MAKE_FLAGS += \
	USE_OPENSSL="yes"
endif

ifeq ($(LOCAL_DEV),1)
define Build/Prepare
	$(CP) -rf ~/git/stunc/* $(PKG_BUILD_DIR)/
endef
endif

define Package/stunc/default/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib/bbfdm
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libstunc.so $(1)/usr/lib/bbfdm/libstunc.so
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/stunc $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/etc/init.d/stunc $(1)/etc/init.d/stunc
	$(INSTALL_DATA) ./files/etc/config/stunc $(1)/etc/config/stunc
endef

Package/stunc-openssl/install = $(Package/stunc/default/install)
Package/stunc-wolfssl/install = $(Package/stunc/default/install)
Package/stunc-mbedtls/install = $(Package/stunc/default/install)

$(eval $(call BuildPackage,stunc-openssl))
$(eval $(call BuildPackage,stunc-wolfssl))
$(eval $(call BuildPackage,stunc-mbedtls))
