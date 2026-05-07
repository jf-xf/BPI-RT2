# 
# Makefile for the Linux network (ethercard) device drivers.
#

obj-$(CONFIG_MODULE_BUILD_IN)+=sdk/

ifeq ($(CONFIG_MODULE_BUILD_IN),y)
obj-$(CONFIG_RTL8686NIC)+=nicDriver/
obj-$(CONFIG_RTK_L34_ENABLE)+=romeDriver/
obj-$(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)+=FleetConntrackDriver/
obj-$(CONFIG_RTK_NIC_RX_HOOK)+=nicHook/
obj-$(CONFIG_RTK_NIC_RX_POOL)+=nicPool/
endif

obj-$(CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE)+=igmpHookModule/

ifeq ($(CONFIG_COMMON_RT_API_DEMO),y)
obj-m+=RT_API_demo/
endif

obj-$(CONFIG_YUEME_DPI) += dpi_ctc/

obj-$(CONFIG_CU_DPI) += dpi_cu/

all:
	$(MAKE) -C sdk all;

ifeq ($(CONFIG_RTL8686NIC),y)
	$(MAKE) -C nicDriver all;
endif
	
ifeq ($(CONFIG_RTK_L34_ENABLE),y)
	$(MAKE) -C romeDriver all;
endif

ifeq ($(CONFIG_RTK_NIC_RX_HOOK),y)
	$(MAKE) -C nicHook all;
endif

ifeq ($(CONFIG_YUEME_DPI),y)
	$(MAKE) -C dpi_ctc all;
endif

install:
	$(MAKE) -C sdk install;

ifeq ($(CONFIG_RTL8686NIC),y)
	$(MAKE) -C nicDriver install;
endif

ifeq ($(CONFIG_RTK_L34_ENABLE),y)
	$(MAKE) -C romeDriver install;
endif

ifeq ($(CONFIG_RTK_NIC_RX_HOOK),y)
	$(MAKE) -C nicHook install;
endif

ifeq ($(CONFIG_YUEME_DPI),y)
	$(MAKE) -C dpi_ctc install;
endif

ifeq ($(CONFIG_CU_DPI),y)
	$(MAKE) -C dpi_cu install;
endif
