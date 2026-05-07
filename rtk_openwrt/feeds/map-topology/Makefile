#
# Copyright (C) 2020-22 IOPSYS Software Solutions AB
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=map-topology
PKG_VERSION:=2.5.0.15

LOCAL_DEV:=0
ifneq ($(LOCAL_DEV),1)
PKG_SOURCE_VERSION:=ad2df376c558d16286f21308759342cb470f3efd
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://dev.iopsys.eu/iopsys/map-topology.git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.xz
PKG_MIRROR_HASH:=skip
endif
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=ieee1905

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/map-topology
	CATEGORY:=Utilities
	DEPENDS:=+libubox +ubus +libpthread +libuci +libeasy \
		 +libavahi-nodbus-support +libnetfilter-conntrack +libnfnetlink +libmnl +ieee1905 +map-plugin
	TITLE:=Utility to build topology of a multi-AP network
endef

define Package/map-topology/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/libnl3 \
	-I$(STAGING_DIR)/usr/include/libnetfilter_conntrack \
	-D_GNU_SOURCE

define Package/map-topology/description
	Constructs network topology and show it as json structure over UBUS
endef

MAKE_PATH:=src

ifeq ($(CONFIG_TOPOLOGYD_EASYMESH_VENDOR_EXT),y)
TARGET_CFLAGS += -DEASYMESH_VENDOR_EXT_OUI=\\\"$(CONFIG_TOPOLOGYD_EASYMESH_VENDOR_EXT_OUI)\\\"
endif

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib/ieee1905/ -lieee1905 -lmaputil

ifeq ($(LOCAL_DEV),1)
define Build/Prepare
        $(CP) -rf ~/git/map-topology/* $(PKG_BUILD_DIR)/
endef
endif

define Package/map-topology/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/topologyd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,map-topology))
