#
#copyright (C) 2020 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk

PKG_NAME:=librtk_timerlist
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

DIR_LINUX = $(LINUX_DIR)
ROOTDIR = $(TOPDIR)

define Package/librtk_timerlist
  SECTION:=utils
  CATEGORY:=Realtek
  TITLE:=Realtek Timer API
  DEPENDS:= 
endef

define Package/librtk_timerlist/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

define Build/InstallDev
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		DESTDIR=$(1) \
		install-dev
endef

define Package/librtk_timerlist/install
	$(MAKE) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		DESTDIR=$(1) \
		install
endef

$(eval $(call BuildPackage,librtk_timerlist))
