include $(APOLLODIR)/Compiler_Flag

FC_DRVDIR = $(APOLLODIR)/$(FC_DRVFOLDER)
FC_SRCDIR = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_SRCFOLDER)

EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include/rtk/rt/
srctree=$(KDIR)

## COMPILE FLAGS ##
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
				
EXTRA_CFLAGS += -I$(srctree)/drivers/net/ethernet/realtek
EXTRA_CFLAGS += -I$(FC_DRVDIR)/rt/include
EXTRA_CFLAGS += -I$(APOLLODIR)/igmpohokModule/rt_igmpHook/include

#####################################
### RT API ACL
#####################################
#RTK_API_ACL = rtk-acl.o
obj-m += rtk-acl.o
#rtk-acl-objs := rtk-acl.o

