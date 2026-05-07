#
#copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=multi-ap_release_ap
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

LINUXDIR = $(LINUX_DIR)
ROOTDIR = $(TOPDIR)
RSDK_TOOLCHAIN_DIR = $(TOOLCHAIN_DIR)
DRIVER_INC_PATH := $(KERNEL_BUILD_DIR)/rtk_wifi6_ap/include
CONFIG_CPU_BIG_ENDIAN=$(CONFIG_BIG_ENDIAN)
export LINUXDIR KERNEL_BUILD_DIR DRIVER_INC_PATH ROOTDIR RSDK_TOOLCHAIN_DIR CONFIG_CPU_BIG_ENDIAN

define Package/multi-ap_release_ap
	SECTION:=service
	CATEGORY:=Realtek
	TITLE:=RTK Multi-AP_Release_AP
	DEPENDS:= +kmod-rtk_wifi6_ap +libopenssl +rtk_lualib +ebtables-legacy
endef

define Package/description
	Multi-AP_Release_AP for RTK wireless chip
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifneq ($(CONFIG_RTW_MULTI_AP_R4),)
	sed -i "s/option profile '[0-9]'/option profile '4'/g" ./files/rtkmap
else ifneq ($(CONFIG_RTW_MULTI_AP_R3),)
	sed -i "s/option profile '[0-9]'/option profile '3'/g" ./files/rtkmap
else ifneq ($(CONFIG_RTW_MULTI_AP_R2),)
	sed -i "s/option profile '[0-9]'/option profile '2'/g" ./files/rtkmap
else ifneq ($(CONFIG_RTW_MULTI_AP_R1),)
	sed -i "s/option profile '[0-9]'/option profile '1'/g" ./files/rtkmap
endif
ifneq ($(CONFIG_RTW_MULTI_AP_LOGO),)
	sed -i "s/option logotest '[0-9]'/option logotest '1'/g" ./files/rtkmap
else
	sed -i "s/option logotest '[0-9]'/option logotest '0'/g" ./files/rtkmap
endif
ifneq ($(CONFIG_MULTI_AP_STP_SWITCH),)
	sed -i "s/stp_monitor_enable = [0-9]/stp_monitor_enable = 1/g" ./src/src/multiap.conf
else
	sed -i "s/stp_monitor_enable = [0-9]/stp_monitor_enable = 0/g" ./src/src/multiap.conf
endif
	$(Build/Prepare/Default)
endef

CFLAGS += $(TARGET_CFLAGS) -g
LDFLAGS += $(TARGET_LDFLAGS)

export CFLAGS LDFLAGS

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		LD="$(TARGET_CROSS)ld" \
		AR="$(TARGET_CROSS)ar" \
		CONFIG_OPENWRT=y \
		CONFIG_LIB_LIBSSL_1_1_1Q=y \
		all
endef

define Package/multi-ap_release_ap/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/core/multi-ap/output/map_controller $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/core/multi-ap/output/map_agent $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/core/multi-ap-core/output/hle_entity $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/core/multi-ap/lib/libmultiap.so $(1)/lib/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/map_vlan_config $(1)/bin/
	$(INSTALL_BIN) ./files/map_iface_checker $(1)/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/map_checker $(1)/bin/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/multiap.conf $(1)/etc/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vendor/customized/libmapVendorData.so $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vendor/vendor_api/empty/libmapvendor.so $(1)/lib/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dpp_broker/dpp_broker $(1)/bin/

	$(INSTALL_BIN) ./files/rtk_multiap $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/rtk_multiap_check $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/dpp_broker $(1)/etc/init.d/

	$(INSTALL_BIN) ./files/rtkmap $(1)/etc/config
	$(INSTALL_BIN) ./files/map_init $(1)/bin/
	$(INSTALL_BIN) ./files/map_reinit $(1)/bin/
	$(INSTALL_BIN) ./files/map_reset $(1)/bin/
	$(INSTALL_BIN) ./files/map_hapd_wpas $(1)/bin/
endef



$(eval $(call BuildPackage,multi-ap_release_ap))
