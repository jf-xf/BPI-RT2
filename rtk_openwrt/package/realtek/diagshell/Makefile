#
#copyright (C) 2017 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=diagshell
PKG_VERSION:=4.1
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=ca_packages

include $(INCLUDE_DIR)/package.mk

define Package/diagshell
  SECTION:=utils
  CATEGORY:=Realtek
  TITLE:=Realtek switch/pon debug Tool
  MAINTAINER:=Realtek
  DEPENDS:=+libpthread +librt
endef
  #DEPENDS:=+libpthread +librt +libubacktrace

define Package/diagshell/description
  Switch/Pon runtime debug/configuration tool
endef

#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
CONFIG_DEFAULTS_KERNEL_4_14=y
CONFIG_SDK_DRIVER_UNITTEST=y
export CONFIG_DEFAULTS_KERNEL_4_14 CONFIG_SDK_DRIVER_UNITTEST

ifeq ($(CONFIG_LUNA_G3_SERIES),y)
ARCH = arm64
endif
ROOTDIR = $(LINUX_DIR)/../
TOP_DIR = $(TOPDIR)
KDIR = $(LINUX_DIR)
KERNEL_SRC = $(LINUX_DIR)
LINUXDIR = linux-$(LINUX_VERSION)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
SDKDIR = $(APOLLODIR)/sdk
export ARCH ROOTDIR KDIR KERNEL_SRC LINUXDIR APOLLODIR SDKDIR TOP_DIR

ROMFSDIR:=$(PKG_INSTALL_DIR)
export ROMFSDIR

STRIP = /bin/echo 1>/dev/null
export STRIP

CHECK_SET:=$(shell expr `$(TARGET_CC) -dumpversion | cut -f1 -d.` \>= 9)
ifeq ("$(CHECK_SET)", "1")
  TARGET_CFLAGS += -Wno-error=address-of-packed-member -Wno-error=incompatible-pointer-types
  PON_CFLAGS += -Wno-error=address-of-packed-member -Wno-error=incompatible-pointer-types
endif

CFLAGS += $(TARGET_CFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)
export CFLAGS LDFLAGS PON_CFLAGS

define Build/Prepare
	@echo "==========================================================="
	@echo "DIR_LINUX: $(DIR_LINUX)"
	@echo "LINUXDIR: $(LINUXDIR)"
	@echo "APOLLODIR: $(APOLLODIR)"
	@echo "SDKDIR: $(SDKDIR)"
	@echo "KERNEL_SRC: $(KERNEL_SRC)"
	@echo "==========================================================="
	$(Build/Prepare/Default)
	#$(RM) -rf $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app
	#$(CP) $(TOPDIR)/target/linux/realtek/files-4.14/drivers/net/ethernet/realtek/rtl86900/sdk/ $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/
	#$(CP) $(TOPDIR)/target/linux/realtek/files-4.14/drivers/net/ethernet/realtek/rtl86900/Compiler_Flag $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/Compiler_Flag
#ifeq (,$(wildcard ./pretendChange))
	#touch pretendChange
#else
	#rm pretendChange
#endif
endef

#define Build/Compile
#	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
#		CC="$(TARGET_CC)" \
#		AR="$(TARGET_CROSS)ar" \
#		all
#endef

## for openwrt 19.X up, need override force-clean-build, because diagshell need do make clean
force-clean-build:
	
define Build/Clean
	$(MAKE) -C $(PKG_BUILD_DIR) clean
	rm -rf $(PKG_BUILD_DIR)
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/object/src/app/lib/librtk.so $(1)/lib/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/object/src/app/lib/libut.so $(1)/lib/
ifneq ("$(wildcard $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/lib/*.so)", "")
	$(CP) -rf $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/lib/*.so $(1)/lib/
endif
	$(CP) -rf $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/object/src/app/lib/*.so $(1)/lib/
endef

define Package/diagshell/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/modules
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/scripts
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/etc/
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/sbin

	@echo "BUILD_DIR: "$(BUILD_DIR)
	@echo "KERNEL_BUILD_DIR: "$(KERNEL_BUILD_DIR)
	@echo "==========================================================="
	@env
	@echo "==========================================================="
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/diag_rtk/bin/diag $(1)/bin/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/pon_detect/bin/pondetect $(1)/bin/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/object/src/app/lib/librtk.so $(1)/lib/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/object/src/app/lib/libut.so $(1)/lib/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/ds_pkt_bcaster/dpbcaster.ko $(1)/lib/modules/
ifeq ($(CONFIG_RTK_OMCI_REDIRECT),y)
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/omci_redirect/omcir.ko $(1)/lib/modules/
endif
ifeq ($(CONFIG_RTK_OMCI_V1),y)
	$(MAKE) -C $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/omci_v1 SDKDIR=$(APOLLODIR)/sdk BINDIR="../bin" install KDIR=$(LINUX_DIR)
endif
ifeq ($(CONFIG_EPON_FEATURE),y)
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
	$(MAKE) $(MAKE_FLAGS) -C $(APOLLODIR)/sdk/src/module/ca_rtk_epon_drv
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/ca_rtk_epon_drv/ca-rtk-epon-drv.ko $(1)/lib/modules/$(LINUX_VERSION)
ifeq ($(CONFIG_CA_RTK_EPON_TEST_FEATURE),y)
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/ca_rtk_epon_drv/ca-rtk-epon-test.ko $(1)/lib/modules/$(LINUX_VERSION)
endif
else
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/epon_drv/epon_drv.ko $(1)/lib/modules/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/epon_mpcp/epon_mpcp.ko $(1)/lib/modules/
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/epon_polling/epon_polling.ko $(1)/lib/modules/
endif
endif
ifeq ($(CONFIG_RTK_IGMP),y)
	$(MAKE) -C $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/rtk_igmp SDKDIR=$(APOLLODIR)/sdk BINDIR="../bin" install KDIR=$(LINUX_DIR)
endif
	$(CP) -rf $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/bin $(1)/
ifneq ("$(wildcard $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/lib/*.so)", "")
	$(CP) -rf $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/lib/*.so $(1)/lib/
endif
	$(CP) -rf $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/object/src/app/lib/*.so $(1)/lib/
ifneq (, $(filter y, $(CONFIG_RTK_OMCI_V1) $(CONFIG_RTK_OAM_V1)))
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
	$(INSTALL_BIN)  $(SDKDIR)/ca_rtk.etc/insdrv.sh $(1)/etc/
	$(INSTALL_BIN)  $(SDKDIR)/ca_rtk.etc/runoam.sh $(1)/etc/
	$(INSTALL_BIN)  $(SDKDIR)/ca_rtk.etc/runsdk.sh $(1)/etc/
	$(INSTALL_BIN)  $(SDKDIR)/ca_rtk.etc/run_customized_sdk.sh $(1)/etc/
else
	$(INSTALL_BIN)  $(SDKDIR)/rtk.etc/insdrv.sh $(1)/etc/
	$(INSTALL_BIN)  $(SDKDIR)/rtk.etc/runoam.sh $(1)/etc/
	$(INSTALL_BIN)  $(SDKDIR)/rtk.etc/runsdk.sh $(1)/etc/
	$(INSTALL_BIN)  $(SDKDIR)/rtk.etc/run_customized_sdk.sh $(1)/etc/
endif
endif
	#copy rtk system needed scripts
	$(INSTALL_BIN) ./files/*.sh $(1)/etc/scripts/
	$(CP) -rf $(PKG_INSTALL_DIR)/lib $(1)/
	$(CP) -rf $(PKG_INSTALL_DIR)/bin $(1)/
	$(LN) /bin/diag $(1)/sbin/diag
	$(CP) -rf $(PKG_INSTALL_DIR)/sbin $(1)/
ifeq ($(CONFIG_RTK_OMCI_V1),y)
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/omci_custom_opt.conf $(1)/etc
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/omci_ignore_mib_tbl.conf $(1)/etc
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/omci_mib.cfg $(1)/etc
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/runomci.sh $(1)/etc
endif
	$(INSTALL_BIN) ./files/diagshell.init $(1)/etc/init.d/diagshell
ifeq ($(CONFIG_RTK_IGMP),y)
	$(INSTALL_DATA) $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/app/rtk_igmp/config/igmp_gpon_9607.conf $(1)/etc/igmp_gpon.conf
endif

endef

$(eval $(call BuildPackage,diagshell))
