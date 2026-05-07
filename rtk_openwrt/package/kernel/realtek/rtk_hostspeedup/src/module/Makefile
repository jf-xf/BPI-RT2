include $(KDIR)/.config

PWD := $(shell pwd)
LINUXDIR = $(shell basename $(KDIR))
ROOTDIR = $(shell dirname $(KDIR))
include $(APOLLODIR)/Compiler_Flag

srctree = $(KDIR)

EXTRA_CFLAGS += -Wall 
EXTRA_CFLAGS += $(PON_CFLAGS)
EXTRA_CFLAGS += -I$(M)/../include
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
EXTRA_CFLAGS += -I$(APOLLODIR)/romeDriver
else
EXTRA_CFLAGS += -I$(APOLLODIR)/FleetConntrackDriver
endif

EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX
ifeq ($(CONFIG_RTL8686NIC),y)
EXTRA_CFLAGS += -I$(APOLLODIR)/nicDriver
endif
ifeq ($(CONFIG_RTL9607C),y)
EXTRA_CFLAGS += -I$(KDIR)/arch/mips/rtl9607c
endif
ifeq ($(CONFIG_RTL8686),y)
EXTRA_CFLAGS += -I$(KDIR)/arch/rlx/bsp_rtl8686
endif

EXTRA_CFLAGS += -I$(APOLLODIR)/../
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include/common
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include/rtk/rt
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/diag_rtk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/API

ifeq ($(CONFIG_ARCH_CORTINA), y)
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/inc 
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/osal/osal/linux_kernel 
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/peri 
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/osal/include 
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/cortina-api/include 
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/include/generated 
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ca-ne-rtk/

ifeq ($(CONFIG_CA8279_SERIES), y)
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ni-drv -I$(APOLLODIR)/ca-ne/aal/include 
endif
ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ni-drv-77c -I$(APOLLODIR)/ca-ne/aal-77c/include 
endif
ifeq ($(CONFIG_ARCH_CORTINA_VENUS), y)
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ni-drv-gen2 -I$(APOLLODIR)/ca-ne/aal-gen2/include 
endif
ifeq ($(CONFIG_ARCH_REALTEK_9607F), y)
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ni-drv-07f -I$(APOLLODIR)/ca-ne/aal-07f/include 
endif

endif

MODULE_NAME = hostSpeedUp
obj-m := $(MODULE_NAME).o

