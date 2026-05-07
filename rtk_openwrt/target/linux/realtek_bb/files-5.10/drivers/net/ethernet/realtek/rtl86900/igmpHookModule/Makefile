include $(APOLLODIR)/Compiler_Flag

PWD         := $(shell pwd) 
KVERSION    := $(shell uname -r)
KERNEL_DIR   = /usr/src/linux-headers-$(KVERSION)/

FC_DRVFOLDER = FleetConntrackDriver
FC_SRCFOLDER = core
FC_DRVDIR = $(APOLLODIR)/$(FC_DRVFOLDER)
FC_SRCDIR = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_SRCFOLDER)

EXTRA_CFLAGS += -Wall \
		-Werror \
		$(PON_CFLAGS) \
		-I$(FC_SRCDIR) \
		-I$(FC_DRVDIR) \
		-I$(FC_DRVDIR)/include \
		-I$(FC_SRCDIR)/asicDriver \
		-I$(APOLLODIR)/nicDriver \
		-I$(APOLLODIR)/sdk/include \
		-I$(APOLLODIR)/sdk/system/include \
		-I$(APOLLODIR)/sdk/src/app/diag_rtk/include \
		-I$(KDIR)/net/bridge \
		-I$(APOLLODIR)/igmpHookModule \
		-I$(APOLLODIR)/igmpHookModule/rt_igmpHook/include \

EXTRA_CFLAGS += -I$(srctree)
		
ifeq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
	EXTRA_CFLAGS += -I$(srctree)/drivers/net
else
	EXTRA_CFLAGS += -I$(srctree)/drivers/net/ethernet/realtek
endif

EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX
EXTRA_CFLAGS += -I$(APOLLODIR)/rt_igmpHook/include

MODULE_NAME  = rtk_igmp_hook
obj-$(CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE)		:= $(MODULE_NAME).o 

$(MODULE_NAME)-objs :=	rtk_igmp_hook_main.o  \
						rtk_igmp_debug.o  \
						rtk_igmp_proc.o	\
						rtk_igmp_snooping.o \
						rtk_igmp_tinyPS_mcfwd.o \
						rtk_igmp_hook_module_api.o \


ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
$(MODULE_NAME)-objs += rtk_igmp_hwCb.o
endif
