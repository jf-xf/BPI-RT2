#
#copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifi_script
PKG_VERSION:=1.0
PKG_RELEASE:=1
#PKG_BUILD_DEPENDS:=TARGET_realtek
#PKG_BUILD_DEPENDS = RTK_LIB_MIB:rtk_mib

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

DIR_LINUX = $(LINUX_DIR)
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
ROOTDIR = $(TOPDIR)
INCDIR = $(STAGING_DIR)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
export DIR_LINUX LINUXDIR KDIR ROOTDIR INCDIR APOLLODIR

define Package/wifi_script
	SECTION:=utils
	CATEGORY:=Realtek
	TITLE:=RTK Wireless Debug Scripts
endef

define Package/description
	Debug scripts for RTK wireless chip
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

#ifdef CONFIG_RTK_LIB_MIB
#CFLAGS += -I$(INCDIR)/include -include $(LINUXDIR)/include/generated/autoconf.h -include $(ROOTDIR)/autoconf.h
#export CONFIG_RTK_LIB_MIB
#endif

#CFLAGS += $(TARGET_CFLAGS)
#LDFLAGS += $(TARGET_LDFLAGS)

#export CFLAGS LDFLAGS

define Build/Compile
endef

define Package/wifi_script/install
	$(INSTALL_DIR) $(1)/etc/scripts
	$(CP) ./files/* $(1)/etc/scripts
endef


$(eval $(call BuildPackage,wifi_script))
