#
#copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_igmpproxy
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DEPENDS = RTK_LIB_MIB:rtk_mib

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk
ROOTDIR = $(TOPDIR)
APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
LINUXDIR = $(LINUX_DIR)
KDIR = $(LINUX_DIR)
INCDIR = $(STAGING_DIR)
export ROOTDIR CFLAGS LDFLAGS PKG_BUILD_DIR TOPDIR APOLLODIR LINUXDIR KDIR INCDIR

define Package/rtk_igmpproxy/config
	source "$(SOURCE)/Config.in"
endef

#Disable Dependencies Check
define CheckDependencies
endef

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/rtk_igmpproxy
	SECTION:=service
	CATEGORY:=Realtek
	TITLE:=realtek igmpproxy
	DEPENDS:= +libpthread +libuci
endef

# Package description; a more verbose description on what our package does
define Package/description
	IGMPproxy is a simple dynamic Multicast Routing Daemon using
	only IGMP signalling (Internet Group Management Protocol).
endef

# Package preparation instructions; create the build directory and copy the source code. 
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
ifeq ($(CONFIG_RTK_LIB_MIB),y)
TARGET_CFLAGS += -I$(STAGING_DIR)/include -I$(STAGING_DIR)/include/rtk -include $(LINUX_DIR)/include/generated/autoconf.h -include $(TOPDIR)/autoconf.h
TARGET_LDFLAGS += -L$(INCDIR)/lib -Wl,-rpath-link=$(STAGING_DIR)/lib:$(STAGING_DIR)/usr/lib:$(STAGING_DIR)/usr/local/lib -lmib 
export CONFIG_RTK_LIB_MIB
endif

CFLAGS += $(TARGET_CFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)
export CFLAGS LDFLAGS

define Build/Prepare
	$(Build/Prepare/Default)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" 
endef

# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/rtk_igmpproxy/install
	$(INSTALL_DIR) $(1)/bin $(1)/etc/init.d $(1)/etc/rc.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/igmpproxy $(1)/bin
ifneq ($(CONFIG_RTK_LIB_MIB), y)
	$(INSTALL_BIN) ./files/rtk_igmpproxy.init $(1)/etc/init.d/igmpproxy
endif
endef


# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,rtk_igmpproxy))
