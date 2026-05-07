include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_wifi6_ap
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

#PKG_SOURCE_URL:=https://github.com/greearb/rtl8812AU_8821AU_linux.git
#PKG_MIRROR_HASH:=42b32c56cf31bdf3c9cd4e304c9aac761b623c8737d8c57518117acdc5a84cfe
#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_DATE:=2018-11-16
#PKG_SOURCE_VERSION:=661268fd174d4a74834c82d7d3987b0a560e6c57

#PKG_MAINTAINER:=Ben Greear <greearb@candelatech.com>
PKG_BUILD_PARALLEL:=1
#PKG_EXTMOD_SUBDIRS:=rtl8812au-ct
PKG_BUILD_DEPENDS:=diagshell

ifneq ($(CONFIG_PACKAGE_kmod-cfg80211),)
STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h
endif

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Wireless Drivers
  TITLE:=Driver for Realtek PCIE WiFi6 Chip
  DEPENDS:= @PCI_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11AX_SUPPORT \
	+@LINUX_4_14:kmod-mac80211 +@LINUX_4_14:kmod-cfg80211 \
	@!PACKAGE_kmod-rtk_wifi6 \
	+@LINUX_5_10:kmod-cfg80211_rtk \
	@!LINUX_4_9 +@PACKAGE_wlan_daemon +@PACKAGE_wifi_script +@WLAN_MANAGER:PACKAGE_wlan_manager
  FILES:=\
	$(PKG_BUILD_DIR)/rtk_wifi6.ko
  #AUTOLOAD:=$(call AutoProbe,rtk_wifi6)
  #MODPARAMS.rtk_wifi6:= \
  #	rtw_vht_enable=2 \
  #	rtw_he_enable=2 \
  #	rtw_drv_log_level=0
  PROVIDES:=kmod-rtk_wifi6_ap
endef

ifneq ($(CONFIG_PACKAGE_kmod-cfg80211),)
NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/backport.h
else
NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include
endif

NOSTDINC_FLAGS += \
	-include package_config.h

#NOSTDINC_FLAGS = \
        -I$(PKG_BUILD_DIR) \
        -I$(PKG_BUILD_DIR)/include

#NOSTDINC_FLAGS+=-DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DBUILD_OPENWRT
NOSTDINC_FLAGS+=-DCPTCFG_VERSION

ifeq ($(CONFIG_PACKAGE_kmod-rtk_ther_ctrl), y)
NOSTDINC_FLAGS += -DWIFI6_THER_CTRL -DRTW_STA_BWC -DSTA_BWC_SHARE_TP
NOSTDINC_FLAGS += -I$(TOPDIR)/rtk_openwrt/package/kernel/realtek/rtk_ther_ctrl/src/
endif

#add to support check efem type when install wifi6 driver.
ifneq ($(CONFIG_2G_ON_WLAN0)$(CPTCFG_2G_ON_WLAN0),)
WLAN0_EFEM_TYPE := wlan1_efem_type
WLAN1_EFEM_TYPE := wlan0_efem_type
else
WLAN0_EFEM_TYPE := wlan0_efem_type
WLAN1_EFEM_TYPE := wlan1_efem_type
endif
ifneq ($(CONFIG_RTW_EFEM_IDENTIFY_WITH_FLASH)$(CPTCFG_RTW_EFEM_IDENTIFY_WITH_FLASH),)
FEM_TYPE_ENABLED := 1
else
FEM_TYPE_ENABLED := 0
endif
ifneq ($(CONFIG_PACKAGE_rtk_boa),)
BR_NAME := br0
else
BR_NAME := br-lan
endif
#end of add
KSRC = $(LINUX_DIR)
export KSRC

#Disable Dependencies Check
define CheckDependencies
endef

define Build/Prepare/$(PKG_NAME)
	$(info PKG build DIR is $(PKG_BUILD_DIR))
	@echo -n "Building package_rtk_wifi6.mk and package_config.h ..."
	@grep -f local-symbols $(TOPDIR)/.config > $(PKG_BUILD_DIR)/package_rtk_wifi6.mk
	./package_rtk_wifi6_autoconf.sh local-symbols $(TOPDIR)/.config $(PKG_BUILD_DIR)/package_config.h
endef

define Build/Prepare
	@echo -n "Generating phl git info in advanced ..."
	./src/call_gen_git_info.sh
	$(Build/Prepare/Default)
	$(call Build/Prepare/$(PKG_NAME))
endef

MODULE_SRC_PATH = `realpath --relative-to=$(LINUX_DIR) $(PKG_BUILD_DIR)`

define Build/Compile/$(PKG_NAME)
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(MODULE_SRC_PATH)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		BR_NAME="$(BR_NAME)" \
		modules
endef

define Build/Compile
	$(call Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/include/rtk
	$(CP) $(PKG_BUILD_DIR)/include/wifi_common.h $(STAGING_DIR)/include/rtk/wifi_common.h
endef

define KernelPackage/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define KernelPackage/$(PKG_NAME)/install
	@echo -e "\n[Installing rtk_wifi6 parameter files]"
	$(INSTALL_DIR) $(1)/etc/conf
	#add to support check efem type when install wifi6 driver.
	$(INSTALL_DIR) $(1)/etc/scripts/
	$(INSTALL_DIR) $(1)/etc/init.d/
	sed -e 's/WLAN0_EFEM_TYPE/$(WLAN0_EFEM_TYPE)/g;s/WLAN1_EFEM_TYPE/$(WLAN1_EFEM_TYPE)/g;s/FEM_TYPE_ENABLED/$(FEM_TYPE_ENABLED)/g' \
	./rtk_files/wifi_modutils.sh.template > $(1)/etc/scripts/wifi_modutils.sh;
	if [ -f ${TOPDIR}/preconfig/customize_wifi_modutils.sh ]; then \
		${TOPDIR}/preconfig/customize_wifi_modutils.sh $(1)/etc/scripts/wifi_modutils.sh; \
	fi
	chmod +x $(1)/etc/scripts/wifi_modutils.sh
	$(INSTALL_BIN) ./rtk_files/rtk_wifi6.init $(1)/etc/init.d/load_rtk_wifi6
	#end of add
	make -f conf_install.mk LINUX_DIR=$(LINUX_DIR) src=$(PKG_BUILD_DIR) ROMFSDIR=$(1)
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
