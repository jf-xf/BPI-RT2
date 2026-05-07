#
# copyright (C) 2021 Realtek Semiconductor Corporation
#

include $(TOPDIR)/rules.mk
# See /LICENSE for more information.

PKG_NAME:=libmd5
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

export ABI_VERSION

define Package/libmd5
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=RTK MD5 message-digest algorithm library
  ABI_VERSION:=0
endef

define Package/libmnl/description
This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

Equivalent code is available from RSA Data Security, Inc.
This code has been tested against that, and is equivalent,
except that you don't need to include two pages of legalese
with every copy.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS+= \
	--enable-shared \
	--enable-static

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libmd5.{a,so*} $(1)/usr/lib/
	-$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmd5.so.* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libmd5.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/libmd5wrapper.h $(1)/usr/include
	#$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmd5.{a,so*} $(1)/usr/lib/
endef

define Package/libmd5/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libmd5.so* $(1)/usr/lib/
	-$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmd5.so.* $(1)/usr/lib/
	#$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmd5.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmd5))
