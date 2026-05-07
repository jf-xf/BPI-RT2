#
#copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=control_agent
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

CA_DIR := /etc/ca

define Package/$(PKG_NAME)
	SECTION:=service
	CATEGORY:=Realtek
	TITLE:=RTK_control_agent
	DEPENDS:= +@RTW_MULTI_AP +@RTK_MULTI_AP_WFA +@PACKAGE_luasocket
endef

define Package/description
	WFA Control Agent for RTK wireless chip
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

define Build/Compile
	$(CP) ./src/logger.lua $(PKG_BUILD_DIR)/logger.lua
	$(CP) ./src/map_ca.lua $(PKG_BUILD_DIR)/map_ca.lua
	$(CP) ./src/socket.lua $(PKG_BUILD_DIR)/socket.lua
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)$(CA_DIR)
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/logger.lua $(1)$(CA_DIR)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/map_ca.lua $(1)$(CA_DIR)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/socket.lua $(1)$(CA_DIR)/

	$(INSTALL_BIN) ./files/rtk_ca $(1)/etc/init.d/
endef



$(eval $(call BuildPackage,$(PKG_NAME)))
