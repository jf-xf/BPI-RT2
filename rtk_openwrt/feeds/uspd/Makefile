#
# Copyright (C) 2019 Iopsys
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uspd
PKG_VERSION:=2.2.14

LOCAL_DEV:=0
ifneq ($(LOCAL_DEV),1)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://dev.iopsys.eu/iopsys/uspd.git
PKG_SOURCE_VERSION:=1c3b886bb5e654049ae81cda726dffae950369c7
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=skip
endif

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/uspd/default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=TRx69
  TITLE:=Datamodel ubus backend
  DEPENDS:=+USE_GLIBC:librt +libubox +ubus +@BBFDM_ENABLE_JSON_PLUGIN +@BBFDM_ENABLE_DOTSO_PLUGIN \
           +@BBF_TR181 +@BBF_TR104 +@BBF_TR143
endef

define Package/uspd-mbedtls
  $(Package/uspd/default)
  TITLE += (mbedtls)
  DEPENDS += +PACKAGE_uspd-mbedtls:libbbfdm-mbedtls
  VARIANT:=mbedtls
  DEFAULT_VARIANT:=1
endef

define Package/uspd-openssl
  $(Package/uspd/default)
  TITLE += (openssl)
  DEPENDS += +PACKAGE_uspd-openssl:libbbfdm-openssl
  CONFLICTS := uspd-mbedtls
  VARIANT:=openssl
endef

define Package/uspd-wolfssl
  $(Package/uspd/default)
  TITLE += (wolfssl)
  DEPENDS += +PACKAGE_uspd-wolfssl:libbbfdm-wolfssl
  CONFLICTS := uspd-mbedtls uspd-openssl
  VARIANT:=wolfssl
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-Wall -Werror \
	-DUSPD_MAX_MSG_LEN="10485760"

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)
ifeq ($(BUILD_VARIANT),openssl)
  MAKE_FLAGS += USE_OPENSSL="yes"
endif

ifeq ($(BUILD_VARIANT),wolfssl)
  MAKE_FLAGS += USE_WOLFSSL="yes"
endif

ifeq ($(BUILD_VARIANT),mbedtls)
  MAKE_FLAGS += USE_MBEDTLS="yes"
endif

ifeq ($(LOCAL_DEV),1)
define Build/Prepare
	$(CP) -rf ~/git/uspd/* $(PKG_BUILD_DIR)/
endef
endif

define Package/uspd/default/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/etc/init.d/uspd $(1)/etc/init.d/uspd
	$(INSTALL_CONF) ./files/etc/config/uspd $(1)/etc/config/uspd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uspd $(1)/usr/sbin/
endef

Package/uspd-openssl/install = $(Package/uspd/default/install)
Package/uspd-wolfssl/install = $(Package/uspd/default/install)
Package/uspd-mbedtls/install = $(Package/uspd/default/install)

$(eval $(call BuildPackage,uspd-openssl))
$(eval $(call BuildPackage,uspd-wolfssl))
$(eval $(call BuildPackage,uspd-mbedtls))
