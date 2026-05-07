define KernelPackage/ca_rtk
  SUBMENU:=$(OTHER_MENU)
  TITLE:=CA RTK module
  DEPENDS:= +kmod-ca_packages
  KCONFIG:=CONFIG_SDK_MODULES=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/dal/ca8277b/ca-rtk.ko
ifeq ($(CONFIG_RTL8277C_SERIES),y)
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/dal/rtl8277c/ca-rtk.ko
endif
ifeq ($(CONFIG_RTL9607F_SERIES),y)
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/sdk/src/dal/rtl9607f/ca-rtk.ko
endif
  AUTOLOAD:=$(call AutoLoad,11,ca-rtk,1)
endef

define KernelPackage/ca_rtk/description
 Kernel module for RTK Apollo Modules
endef

$(eval $(call KernelPackage,ca_rtk))

define KernelPackage/rtk_fc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RTK FleetConntrack module
  DEPENDS:= kmod-ca_packages +kmod-nf-conntrack
  KCONFIG:=CONFIG_RTK_L34_FC_KERNEL_MODULE=m
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_8277b.ko \
	$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_mgr.ko
ifeq ($(CONFIG_RTL8277C_SERIES),y)
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_8277c.ko \
	$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_mgr.ko
endif
ifeq ($(CONFIG_RTL9607F_SERIES),y)
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_9607f.ko \
	$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_mgr.ko
endif
#ifeq ($(CONFIG_RTL9607C_SERIES),y)
#  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_drv.ko \
#	$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_mgr.ko
#endif
  AUTOLOAD:=$(call AutoLoad,12,fc_mgr,1)
endef

define KernelPackage/rtk_fc/description
 Kernel module for RTK FleetConntrack
endef

$(eval $(call KernelPackage,rtk_fc))

define KernelPackage/rtk_igmp_hook
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RTK IGMP/MLD Snooping module
  DEPENDS:=
  KCONFIG:=CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE=m
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900/igmpHookModule/rtk_igmp_hook.ko 
  AUTOLOAD:=$(call AutoLoad,13,rtk_igmp_hook,1)
endef

define KernelPackage/rtk_igmp_hook/description
 Kernel module for RTK IGMP/MLD Snooping
endef

$(eval $(call KernelPackage,rtk_igmp_hook))

define KernelPackage/rtl8221b
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Realtek RTL8221B 2.5G Ethernet Transceiver
  DEPENDS:=+kmod-ca_packages
  KCONFIG:=CONFIG_RTL_8221B_SUPPORT=y CONFIG_RTL_8221B_MODULE=m CONFIG_RTL_8221B_DEVICE_0=n CONFIG_RTL_8221B_DEVICE_1=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/rtl8221B/rtl8221b.ko
  AUTOLOAD:=$(call AutoLoad,91,rtl8221b,1)
endef

define KernelPackage/rtl8221b/description
 Kernel module for Realtek RTL8221B 2.5G Ethernet Transceiver
endef

$(eval $(call KernelPackage,rtl8221b))

define KernelPackage/extGphy
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Realtek Extarnal GPHY driver
  DEPENDS:=+kmod-ca_packages +kmod-rtl8221b
  KCONFIG:=CONFIG_RTK_EXT_GPHY=m
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/extGphy.ko
  AUTOLOAD:=$(call AutoLoad,92,extGphy,1)
endef

define KernelPackage/extGphy/description
 Kernel module for Realtek Extarnal GPHY driver
endef

$(eval $(call KernelPackage,extGphy))

