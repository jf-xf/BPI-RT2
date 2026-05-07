include $(TOPDIR)/rules.mk

PKG_NAME:=netifd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/netifd.git
PKG_SOURCE_DATE:=2022-08-25
PKG_SOURCE_VERSION:=76d2d41b7355e02f95fbfa79affbd232fb090595
PKG_MIRROR_HASH:=cd754decce7d9f9c69e0ad8c6801f306fa37dd6c8a7039aea610c1c71d06b8f9
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/netifd
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libuci +libnl-tiny +libubus +ubus +ubusd +jshn +libubox +@PACKAGE_rtk_mib:BUSYBOX_CONFIG_UDHCPC
  TITLE:=OpenWrt Network Interface Configuration Daemon
endef

define Package/netifd/conffiles
/etc/udhcpc.user
/etc/udhcpc.user.d/
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include \
	-include $(TOPDIR)/autoconf.h \
	-flto

TARGET_LDFLAGS += -flto -fuse-linker-plugin

CMAKE_OPTIONS += \
	-DLIBNL_LIBS=-lnl-tiny \
	-DDEBUG=1

define Package/netifd/config
	menu "Netifd setting"
	depends on PACKAGE_netifd

	config RTK_NETIFD
	bool "Realtek netifd feature flag"
	default y if RTK_LIB_MIB

	endmenu
endef

define Package/netifd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netifd $(1)/sbin/
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/etc/udhcpc.user.d/
	$(CP) $(PKG_BUILD_DIR)/scripts/* $(1)/lib/netifd/
ifneq ($(CONFIG_RTK_LIB_MIB),)
	- test -d ./rtk_files/rtkwrt/ && rsync -arv --chmod=Du=rwx,Dgo=rx,Fu=rwx,Fgo=rx ./rtk_files/rtkwrt/* $(1)/
else
ifneq ($(CONFIG_PACKAGE_luci-rtk-generic),)
	- test -d ./rtk_files/generic/ && rsync -arv --chmod=Du=rwx,Dgo=rx,Fu=rwx,Fgo=rx ./rtk_files/generic/* $(1)/
else
	#- test -e ./rtk_files/generic/etc/init.d/network && rsync -arv --chmod=Du=rwx,Dgo=rx,Fu=rwx,Fgo=rx ./rtk_files/generic/etc/init.d/network $(1)/etc/init.d
	chmod +x ./rtk_files/generic/etc/init.d/network
	- test -e ./rtk_files/generic/etc/init.d/network && cp -r ./rtk_files/generic/etc/init.d/network $(1)/etc/init.d
	chmod +x $(1)/etc/init.d
endif
endif
endef

$(eval $(call BuildPackage,netifd))
