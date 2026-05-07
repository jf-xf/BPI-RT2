#
# Copyright (C) 2020-2022 IOPSYS Software Solutions AB
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=icwmp
PKG_VERSION:=8.5.9

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://dev.iopsys.eu/iopsys/icwmp.git
PKG_SOURCE_VERSION:=5553a890ae29692d8e3c7e1b1734adb0722b020a
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=skip

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/icwmp/default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=TRx69
  TITLE:=TR069 CWMP client
  DEPENDS:=+libuci +libubox +libblobmsg-json +libubus +libjson-c +libcurl +mxml +@LIBCURL_CRYPTO_AUTH
endef

define Package/icwmp-openssl
  $(Package/icwmp/default)
  TITLE += (openssl)
  DEPENDS += +PACKAGE_icwmp-openssl:libopenssl
  VARIANT:=openssl
endef

define Package/icwmp-wolfssl
  $(Package/icwmp/default)
  TITLE += (wolfssl)
  DEPENDS += +PACKAGE_icwmp-wolfssl:libwolfssl
  CONFLICTS := icwmp-openssl
  VARIANT:=wolfssl
endef

define Package/icwmp-mbedtls
  $(Package/icwmp/default)
  TITLE += (mbedtls)
  DEPENDS += +PACKAGE_icwmp-mbedtls:libmbedtls
  CONFLICTS := icwmp-openssl icwmp-wolfssl
  VARIANT:=mbedtls
  DEFAULT_VARIANT:=1
endef

ifeq ($(BUILD_VARIANT),openssl)
CMAKE_OPTIONS += -DWITH_OPENSSL=ON
endif

ifeq ($(BUILD_VARIANT),wolfssl)
CMAKE_OPTIONS += -DWITH_WOLFSSL=ON
endif

ifeq ($(BUILD_VARIANT),mbedtls)
CMAKE_OPTIONS += -DWITH_MBEDTLS=ON
endif

USE_LOCAL=$(shell ls ./src/ 2>/dev/null >/dev/null && echo 1)
ifneq ($(USE_LOCAL),)
define Build/Prepare
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
endif

define Package/icwmp/default/install
	$(INSTALL_DIR) $(1)/etc/icwmpd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DIR) $(1)/etc/bbfdm/json/
	$(INSTALL_DIR) $(1)/etc/udhcpc.user.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/icwmpd $(1)/usr/sbin/icwmpd
	$(INSTALL_DATA) ./files/etc/config/cwmp $(1)/etc/config/cwmp
	$(INSTALL_BIN) ./files/etc/firewall.cwmp $(1)/etc/firewall.cwmp
	$(INSTALL_BIN) ./files/etc/init.d/icwmpd $(1)/etc/init.d/icwmpd
	#$(INSTALL_BIN) ./files/etc/uci-defaults/85-cwmp-set-userid $(1)/etc/uci-defaults/
	$(INSTALL_BIN) ./files/etc/uci-defaults/90-cwmpfirewall $(1)/etc/uci-defaults/
	$(INSTALL_DATA) ./files/lib/upgrade/keep.d/icwmp $(1)/lib/upgrade/keep.d/icwmp
	$(INSTALL_BIN) ./files/etc/icwmpd/update.sh $(1)/etc/icwmpd/update.sh
	$(INSTALL_DATA) ./files/etc/bbfdm/json/CWMPManagementServer.json $(1)/etc/bbfdm/json/
	$(INSTALL_BIN) ./files/etc/udhcpc.user.d/udhcpc_icwmp.user $(1)/etc/udhcpc.user.d/udhcpc_icwmp.user
endef

Package/icwmp-openssl/install = $(Package/icwmp/default/install)
Package/icwmp-wolfssl/install = $(Package/icwmp/default/install)
Package/icwmp-mbedtls/install = $(Package/icwmp/default/install)

$(eval $(call BuildPackage,icwmp-openssl))
$(eval $(call BuildPackage,icwmp-wolfssl))
$(eval $(call BuildPackage,icwmp-mbedtls))
