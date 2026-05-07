include $(KDIR)/.config

LINUXDIR = $(shell basename $(KDIR))
ROOTDIR = $(shell dirname $(KDIR))
include $(APOLLODIR)/Compiler_Flag

srctree = $(KDIR)

EXTRA_CFLAGS += -Wall 
EXTRA_CFLAGS += $(PON_CFLAGS)
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
EXTRA_CFLAGS += -I$(APOLLODIR)/romeDriver
else
EXTRA_CFLAGS += -I$(APOLLODIR)/FleetConntrackDriver
endif

EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX
ifeq ($(CONFIG_RTL8686NIC),y)
    EXTRA_CFLAGS += -I$(APOLLODIR)/nicDriver
endif
EXTRA_CFLAGS += -I$(APOLLODIR)/../

EXTRA_CFLAGS += -I$(src)/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include/common
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include/rtk/rt
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/diag_rtk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/API

MODULE_NAME = smuxdev
obj-m := $(MODULE_NAME).o

