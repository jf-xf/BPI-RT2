#
#copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_lualib
PKG_VERSION:=2.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/rtk_lualib
	SECTION:=service
	CATEGORY:=Realtek
	TITLE:=RTK Lua Library
endef

define Package/description
	RTK Lua Library
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

CFLAGS += $(TARGET_CFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)

export CFLAGS LDFLAGS

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r files $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define Package/rtk_lualib/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/rtk
ifneq ($(CONFIG_2G_ON_WLAN0),y)
	sed -i 's/__2g_on_wlan0 = 1/__2g_on_wlan0 = 0/g' $(PKG_BUILD_DIR)/files/mapconf.lua
endif
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/*.lua $(1)/usr/lib/lua/rtk
endef



$(eval $(call BuildPackage,rtk_lualib))
