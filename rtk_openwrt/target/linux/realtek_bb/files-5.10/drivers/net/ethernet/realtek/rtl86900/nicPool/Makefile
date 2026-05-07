#
# Makefile for the Realtek NIC skb pool mechanism.
#

include $(APOLLODIR)/Compiler_Flag

EXTRA_CFLAGS += -O2

ifeq ($(CONFIG_RTK_L34_G3_PLATFORM), y)
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include
ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-77c/include
else
ifeq ($(CONFIG_ARCH_CORTINA_VENUS), y)
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include
else
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal/include
endif
endif
endif

obj-$(CONFIG_RTL_ETH_RECYCLED_SKB) += re_recycleskb.o
