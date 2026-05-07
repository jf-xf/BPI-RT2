#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mac80211_rtk
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

WMENU:=Wireless Drivers

define KernelPackage/cfg80211_rtk
  SUBMENU:=$(WMENU)
  TITLE:=cfg80211 - wireless configuration API
  DEPENDS+= +iw +iwinfo +wireless-regdb @!PACKAGE_kmod-cfg80211
  KCONFIG:= \
	CONFIG_CFG80211=m \
	CONFIG_NL80211_TESTMODE=n \
	CONFIG_CFG80211_REQUIRE_SIGNED_REGDB=n \
	CONFIG_CFG80211_DEVELOPER_WARNINGS=n \
	CONFIG_CFG80211_CERTIFICATION_ONUS=y \
	CONFIG_CFG80211_DEFAULT_PS=n \
	CONFIG_CFG80211_DEBUGFS=n \
	CONFIG_CFG80211_USE_KERNEL_REGDB_KEYS=y \
	CONFIG_CFG80211_CRDA_SUPPORT=y \
	CONFIG_CFG80211_REG_CELLULAR_HINTS=n \
	CONFIG_CFG80211_REG_RELAX_NO_IR=n \
	CONFIG_CFG80211_WEXT=y \
	CONFIG_VIRT_WIFI=n \
	CONFIG_PKCS8_PRIVATE_KEY_PARSER=n \
	CONFIG_PKCS7_TEST_KEY=n \
	CONFIG_SIGNED_PE_FILE_VERIFICATION=n \
	CONFIG_SYSTEM_EXTRA_CERTIFICATE=n \
	CONFIG_SECONDARY_TRUSTED_KEYRING=n
  FILES:= \
	$(LINUX_DIR)/net/wireless/cfg80211.ko
  AUTOLOAD:=$(call AutoProbe,cfg80211)
endef

define KernelPackage/cfg80211_rtk/description
cfg80211 is the Linux wireless LAN (802.11) configuration API.
endef


define KernelPackage/mac80211_rtk
  SUBMENU:=$(WMENU)
  TITLE:=Linux 802.11 Wireless Networking Stack
  # +kmod-crypto-cmac is a runtime only dependency of net/mac80211/aes_cmac.c
  DEPENDS+= +kmod-cfg80211_rtk +kmod-crypto-cmac +kmod-crypto-ccm +kmod-crypto-gcm +hostapd-common @!PACKAGE_kmod-mac80211
  KCONFIG:=\
	CONFIG_MAC80211=m \
	CONFIG_MAC80211_HAS_RC=y \
	CONFIG_MAC80211_RC_MINSTREL=y \
	CONFIG_MAC80211_RC_DEFAULT_MINSTREL=y \
	CONFIG_MAC80211_MESH=y \
	CONFIG_MAC80211_LEDS=y \
	CONFIG_MAC80211_DEBUGFS=y \
	CONFIG_MAC80211_DEBUG_MENU=n \
	CONFIG_MAC80211_HWSIM=n
  FILES:= \
	$(LINUX_DIR)/net/mac80211/mac80211.ko
  AUTOLOAD:=$(call AutoProbe,mac80211)
endef

define KernelPackage/mac80211_rtk/description
Generic IEEE 802.11 Networking Stack (mac80211)
endef

define KernelPackage/lib80211_rtk
  SUBMENU:=$(WMENU)
  TITLE:=802.11 Networking stack
  DEPENDS:=+kmod-cfg80211_rtk +kmod-crypto-hash +kmod-crypto-ccm @!PACKAGE_kmod-lib80211
  KCONFIG:= \
    CONFIG_LIB80211=m \
    CONFIG_LIB80211_CRYPT_WEP=m \
    CONFIG_LIB80211_CRYPT_CCMP=m \
    CONFIG_LIB80211_CRYPT_TKIP=m 
  FILES:= \
	$(LINUX_DIR)/net/wireless/lib80211.ko \
	$(LINUX_DIR)/net/wireless/lib80211_crypt_wep.ko \
	$(LINUX_DIR)/net/wireless/lib80211_crypt_ccmp.ko \
	$(LINUX_DIR)/net/wireless/lib80211_crypt_tkip.ko
  AUTOLOAD:=$(call AutoProbe, \
	lib80211 \
	lib80211_crypt_wep \
	lib80211_crypt_ccmp \
	lib80211_crypt_tkip \
  )
endef

define KernelPackage/lib80211_rtk/description
 Kernel modules for 802.11 Networking stack
 Includes:
 - lib80211
 - lib80211_crypt_wep
 - lib80211_crypt_tkip
 - lib80211_crytp_ccmp
endef


define KernelPackage/cfg80211_rtk/install
	$(INSTALL_DIR) $(1)/lib/wifi $(1)/lib/netifd/wireless
	$(INSTALL_DATA) ./files/lib/wifi/mac80211.sh $(1)/lib/wifi
	$(INSTALL_DATA) ./files/lib/wifi/rtk_wifi.sh $(1)/lib/wifi
	# $(INSTALL_DATA) ./files/lib/wifi/run_cal.sh $(1)/lib/wifi
	$(INSTALL_BIN) ./files/lib/netifd/wireless/mac80211.sh $(1)/lib/netifd/wireless
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ieee80211
	$(INSTALL_DATA) ./files/mac80211.hotplug $(1)/etc/hotplug.d/ieee80211/10-wifi-detect
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifneq ($(CONFIG_2G_ON_WLAN0),)
	sed -i 's/ifname="wlan0"/ifname="wlan1"/g' ./files/lib/wifi/mac80211.sh
	sed -i 's/ifname1="wlan1"/ifname1="wlan0"/g' ./files/lib/wifi/mac80211.sh
else
	sed -i 's/ifname="wlan1"/ifname="wlan0"/g' ./files/lib/wifi/mac80211.sh
	sed -i 's/ifname1="wlan0"/ifname1="wlan1"/g' ./files/lib/wifi/mac80211.sh
endif
endef

define Build/Compile
endef

define KernelPackage/depends
endef

$(eval $(call KernelPackage,cfg80211_rtk))
$(eval $(call KernelPackage,mac80211_rtk))
$(eval $(call KernelPackage,lib80211_rtk))
