include $(TOPDIR)/rules.mk

# 注意：这里直接引用 SDK 根目录下的 luci.mk
include $(TOPDIR)/feeds/luci/luci.mk

# 这里的名称必须和文件夹目录名完全一致
PKG_NAME:=luci-theme-minimal
PKG_VERSION:=1.0
PKG_RELEASE:=1

# 定义在菜单中的显示位置
LUCI_TITLE:=Minimal RTK Theme
LUCI_DEPENDS:=
LUCI_DESCRIPTION:=Customized modern theme for RTK Router.

# 这里的 BuildPackage 调用必须在最后
$(eval $(call BuildPackage,$(PKG_NAME)))