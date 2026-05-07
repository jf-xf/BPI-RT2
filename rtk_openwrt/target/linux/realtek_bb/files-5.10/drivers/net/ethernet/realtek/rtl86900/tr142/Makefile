include $(APOLLODIR)/Compiler_Flag

srctree=$(KDIR)

EXTRA_CFLAGS += -Wall
EXTRA_CFLAGS += $(PON_CFLAGS)
OMCI_PF_SYMVER =
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
EXTRA_CFLAGS += -I$(APOLLODIR)/romeDriver
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
OMCI_PF_SYMVER = $(APOLLODIR)/object/src/app/omci_v1/DRV/platform/ca8279/Module.symvers
else
OMCI_PF_SYMVER = $(APOLLODIR)/object/src/app/omci_v1/DRV/platform/rtl9607_rg/Module.symvers
endif
else
EXTRA_CFLAGS += -I$(APOLLODIR)/FleetConntrackDriver
OMCI_PF_SYMVER = $(APOLLODIR)/object/src/app/omci_v1/DRV/platform/rtl9607_fc/Module.symvers
endif

ifeq ($(CONFIG_LUNA_G3_SERIES),y)
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ca-ne-rtk
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/ni-drv
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/aal/include
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-ne/cortina-api/include
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/inc
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/osal/include
EXTRA_CFLAGS += -I$(APOLLODIR)/ca-plat/osal/osal/linux_kernel
LINUXINCLUDE += -include $(APOLLODIR)/ca-ne/include/generated/ca_ne_autoconf.h

EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/linux
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/OMCI/src
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/net
EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX
ifeq ($(CONFIG_DEFAULTS_KERNEL_3_18),y)
    EXTRA_CFLAGS += -I$(srctree)/drivers/net/ethernet/realtek
else
ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
    EXTRA_CFLAGS += -I$(srctree)/drivers/net/ethernet/realtek
else
    EXTRA_CFLAGS += -I$(srctree)/drivers/net
endif
endif

EXTRA_CFLAGS += -I$(APOLLODIR)/tr142/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/diag_rtk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/API

MODULE_NAME=rtk_tr142
obj-m := $(MODULE_NAME).o

all:
	$(MAKE) -C $(KDIR) M=$(APOLLODIR)/tr142 CROSS_COMPILE=$(CROSS_COMPILE) \
	KBUILD_EXTRA_SYMBOLS=$(OMCI_PF_SYMVER) \
	modules

install:
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules
	cp -af $(MODULE_NAME).sh $(ROMFSDIR)/etc

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko

