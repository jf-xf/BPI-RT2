EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX
EXTRA_CFLAGS += -I$(KDIR)
EXTRA_CFLAGS += -I$(NETDIR)
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/include

ifeq ($(CONFIG_LUNA_G3_SERIES), y)
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-plat/1.0-r0/ca-plat-1.0/inc \
		-I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include
endif
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include

MODULE_NAME  = rtl8221b
obj-$(CONFIG_RTL_8221B_MODULE) += $(MODULE_NAME).o
$(MODULE_NAME)-objs := nic_rtl8226b.o nic_rtl8226b_init.o nic_rtl8221b_mod.o

