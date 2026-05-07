#
#copyright (C) 2017 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_nv
#PKG_VERSION:=r14895
PKG_RELEASE:=r14895

#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/rtk_nv
  SECTION:=utils
  CATEGORY:=Realtek
  TITLE:=Realtek nv Tool
  MAINTAINER:=JeT Kuo <jet.kuo@realtek.com>
endef

define Package/rtk_nv/description
  U-Boot evn update utility
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

LINUXDIR = $(LINUX_DIR)
ROOTDIR = $(TOPDIR)
CONFIG_PRODUCT = luna
export CONFIG_PRODUCT ROOTDIR LINUXDIR

CFLAGS = $(TARGET_CFLAGS)
LDFLAGS = $(TARGET_LDFLAGS)

define Build/Compile
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		all
endef

define Build/InstallDev
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		DESTDIR=$(1) \
		install-dev
endef

define Package/rtk_nv/install
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		DESTDIR=$(1) \
		install
endef

$(eval $(call BuildPackage,rtk_nv))
