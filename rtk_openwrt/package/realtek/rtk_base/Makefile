#
#copyright (C) 2015 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_base
PKG_VERSION:=1.0
PKG_RELEASE:=0

PKG_BUILD_DEPENDS:=diagshell USER_RTK_VOIP:rtk_voip

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

DIR_LINUX = $(LINUX_DIR)
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
KERNEL_VERSION = $(LINUX_VERSION)
ROOTDIR = $(TOPDIR)
INCDIR = $(STAGING_DIR)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900

export DIR_LINUX KERNEL_VERSION ROOTDIR APOLLODIR LINUXDIR KDIR INCDIR TOPDIR

# for firmware version control
CONFIG_SW_DEFAULT_VERSION= "V2.0.0"
CONFIG_SW_SUBVERSION= "%y%m%d"
CONFIG_SW_VERSION= "$(CONFIG_SW_DEFAULT_VERSION)-$(shell date +'$(CONFIG_SW_SUBVERSION)')"
export CONFIG_SW_VERSION

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Realtek
  TITLE:=Realtek Base config
  DEPENDS:=@!PACKAGE_rtk_mib +uci +@PACKAGE_busybox:BUSYBOX_DEFAULT_MOUNTPOINT +libxml2
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(Build/Prepare/Default)

	if [ -f ${TOPDIR}/preconfig/rtk_smp_affinity.init ]; then \
		cp ${TOPDIR}/preconfig/rtk_smp_affinity.init ./files/; \
	fi

ifneq ($(CONFIG_2G_ON_WLAN0),)
	if [ -f ./files/rtk_smp_affinity.init ]; then \
		sed -i 's/IFNAME="wlan 1"/IFNAME="wlan 0"/g' ./files/rtk_smp_affinity.init; \
		sed -i 's/IFNAME1="wlan 0"/IFNAME1="wlan 1"/g' ./files/rtk_smp_affinity.init; \
	fi
else
	if [ -f ./files/rtk_smp_affinity.init ]; then \
		sed -i 's/IFNAME="wlan 0"/IFNAME="wlan 1"/g' ./files/rtk_smp_affinity.init; \
		sed -i 's/IFNAME1="wlan 1"/IFNAME1="wlan 0"/g' ./files/rtk_smp_affinity.init; \
	fi
endif
endef

# for debug memory statck ##
# TARGET_CFLAGS += -g -O0 -fstack-protector-all -Wno-error=cpp
# TARGET_LDFLAGS += -lssp
#

CFLAGS += $(TARGET_CFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)

export CFLAGS LDFLAGS

define Build/Compile
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		all
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Build/InstallDev
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		DESTDIR=$(1) \
		install-dev
endef

define Package/$(PKG_NAME)/install
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		DESTDIR=$(1) \
		install
		
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/scripts
	$(INSTALL_BIN) ./files/rtk_config.init $(1)/etc/init.d/rtk_config
	$(INSTALL_BIN) ./files/rtk_base.init $(1)/etc/init.d/rtk_base
	$(INSTALL_BIN) ./files/rtk_rpc.init $(1)/etc/init.d/rtk_rpc
	$(INSTALL_BIN) ./files/rtk_crypto.init $(1)/etc/init.d/rtk_crypto
	$(INSTALL_BIN) ./files/crypto_init.sh $(1)/etc/scripts/crypto_init.sh
	if [ -f ./files/rtk_smp_affinity.init ]; then \
		$(INSTALL_BIN) ./files/rtk_smp_affinity.init $(1)/etc/init.d/rtk_smp_affinity; \
	fi
	$(INSTALL_DIR) $(1)/etc/config
	
	if [ -f ${TOPDIR}/preconfig/rtkmib ]; then \
		$(CP) ${TOPDIR}/preconfig/rtkmib $(1)/etc/config/rtkmib; \
		$(CP) ${TOPDIR}/preconfig/rtkmib $(1)/etc/rtkmib_default; \
	else \
		$(CP) ./files/rtkmib $(1)/etc/config/rtkmib; \
		$(CP) ./files/rtkmib $(1)/etc/rtkmib_default; \
	fi
	chmod 644 $(1)/etc/config/rtkmib
	chmod 444 $(1)/etc/rtkmib_default
	
	if [ -f ${TOPDIR}/preconfig/config_default_hs.xml ]; then \
		$(CP) ${TOPDIR}/preconfig/config_default_hs.xml $(1)/etc/config_default_hs.xml; \
	else \
		$(CP) ./files/config_default_hs.xml $(1)/etc/config_default_hs.xml; \
	fi
	chmod 444 $(1)/etc/config_default_hs.xml
	
	if [ -f ${TOPDIR}/preconfig/config_default.xml ]; then \
		$(CP) ${TOPDIR}/preconfig/config_default.xml $(1)/etc/config_default.xml; \
	else \
		$(CP) ./files/config_default.xml $(1)/etc/config_default.xml; \
	fi
	chmod 444 $(1)/etc/config_default.xml
	

	$(INSTALL_BIN) ./files/rtk_igmpsnooping.init $(1)/etc/init.d/rtk_igmpsnooping
	$(CP) ./files/igmpsnooping $(1)/etc/config;
	chmod 644 $(1)/etc/config/igmpsnooping

	# $(INSTALL_DIR) $(1)/bin
	# $(INSTALL_BIN) ./files/mib.sh $(1)/bin/mib
	$(INSTALL_BIN) ./files/fw_loaded.sh $(1)/etc/scripts
	$(INSTALL_BIN) ./files/fwu_starter.sh $(1)/etc/scripts
	$(INSTALL_BIN) ./files/clean_memory.sh $(1)/etc/scripts
	$(INSTALL_DIR) $(1)/etc/profile.d
	$(INSTALL_BIN) ./files/rtkenv.sh $(1)/etc/profile.d
	$(INSTALL_BIN) ./files/fc_env.sh $(1)/etc/profile.d
	./files/gen_rtk_script_env.sh $(1)/etc/profile.d/rtk_script_env.sh
	$(INSTALL_DIR) $(1)/lib/functions
	./files/gen_interface_config.sh $(1)/lib/functions/interface_config.sh 2> /dev/null

	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/led_inet.sh $(1)/etc/hotplug.d/iface/05-led_inet
	$(INSTALL_DIR) $(1)/etc/rc.button
	$(INSTALL_BIN) ./files/wlan-hotplug.sh $(1)/etc/rc.button/wlan

	$(INSTALL_BIN) ./files/rtk_sysctl.init $(1)/etc/init.d/rtk_sysctl
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/rtk_sysctl.config $(1)/etc/uci-defaults/99-rtk_sysctl

	$(LN) /etc/openwrt_version $(1)/etc/version
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
