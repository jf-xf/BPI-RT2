#
#copyright (C) 2017 Realtek Semiconductor Corporation
#
include $(TOPDIR)/rules.mk

MAJOR_VERSION = 2
MINOR_VERSION = 12

PKG_NAME:=rtkbosa
PKG_VERSION:=$(MAJOR_VERSION).$(MINOR_VERSION)
PKG_RELEASE:=1

#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
ifeq ($(CONFIG_RTK_LIB_MIB),y)
PKG_BUILD_DEPENDS = RTK_LIB_MIB:rtk_mib
endif

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/rtkbosa
  SECTION:=utils
  CATEGORY:=Realtek
  TITLE:=Realtek Bosa Tool
  MAINTAINER:=JeT Kuo <jet.kuo@realtek.com>
ifeq ($(CONFIG_RTK_LIB_MIB),y)
  DEPENDS:=+libpthread +libgsl +rtk_mib
else
  DEPENDS:=+libpthread +libgsl
endif
endef

define Package/rtkbosa/description
  Bosa Utility
endef

APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900

MAKE_FLAGS += \
	CFLAGS+="-DRTKBOSA_MAJOR_VERSION=$(MAJOR_VERSION)" \
	CFLAGS+="-DRTKBOSA_MINOR_VERSION=$(MINOR_VERSION)" \
	CFLAGS+="-include $(KDIR)/include/generated/autoconf.h" \
	CFLAGS+="-include $(TOPDIR)/autoconf.h" \
	CFLAGS+="-I$(APOLLODIR)/sdk/include" \
	CFLAGS+="-I$(APOLLODIR)/sdk/system/include" \
	CFLAGS+="-I$(PKG_BUILD_DIR)" \
	CFLAGS+="-I$(STAGING_DIR)/include/rtk" \
	LDFLAGS+="-L$(APOLLODIR)/object/src/app/lib -lm -lpthread -lrtk -lgsl -lgslcblas"

ifeq ($(CONFIG_RTK_LIB_MIB),y)
MAKE_FLAGS += \
	LDFLAGS+="-L$(STAGING_DIR)/lib -lmib"
endif

define Build/Prepare
	$(Build/Prepare/Default)
endef

define Package/rtkbosa/install
	@echo "ipkg: $(1)"
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(APOLLODIR)/object/src/app/lib/librtk.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtkbosa $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./src/rtkbosa.sh $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rtkbosa.init $(1)/etc/init.d/rtkbosa
endef

$(eval $(call BuildPackage,rtkbosa))
