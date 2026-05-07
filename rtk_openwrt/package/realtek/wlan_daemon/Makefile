#
#copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wlan_daemon
PKG_VERSION:=1.0
PKG_RELEASE:=1
#PKG_BUILD_DEPENDS:=TARGET_realtek
PKG_BUILD_DEPENDS = RTK_LIB_MIB:rtk_mib

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

DIR_LINUX = $(LINUX_DIR)
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
ROOTDIR = $(TOPDIR)
INCDIR = $(STAGING_DIR)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
export DIR_LINUX LINUXDIR KDIR ROOTDIR INCDIR APOLLODIR

define Package/wlan_daemon
	SECTION:=service
	CATEGORY:=Realtek
	TITLE:=RTK Wireless MP daemon
	DEPENDS:= +libc +libpthread 
endef

define Package/description
	MP daemon for RTK wireless chip
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

ifdef CONFIG_RTK_LIB_MIB
CFLAGS += -I$(INCDIR)/include -include $(LINUXDIR)/include/generated/autoconf.h -include $(ROOTDIR)/autoconf.h
export CONFIG_RTK_LIB_MIB
endif

#CFLAGS += $(TARGET_CFLAGS) -DCONFIG_RTL8852AE=m
CFLAGS += $(TARGET_CFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)

export CFLAGS LDFLAGS

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		all
endef

define Package/wlan_daemon/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/scripts
	$(INSTALL_DIR) $(1)/etc
	if [ -f $(PKG_BUILD_DIR)/AX_UDPserver ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/AX_UDPserver $(1)/bin/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/11N_UDPserver ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/11N_UDPserver $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/orf $(1)/etc/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/irf $(1)/etc/; \
	fi
	$(INSTALL_BIN) ./files/mp_start.sh $(1)/etc/scripts/
	$(INSTALL_BIN) ./files/mp_stop.sh $(1)/etc/scripts/
endef



$(eval $(call BuildPackage,wlan_daemon))
