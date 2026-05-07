include $(APOLLODIR)/Compiler_Flag

ccflags-y := -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include \
		-I$(ROOTDIR)/ca_packages/ca-plat/1.0-r0/ca-plat-1.0/inc \
		-I$(ROOTDIR)/ca_packages/ca-scfg/1.0-r0/ca-scfg-1.0/scfg/include \
		-I$(SDKDIR)/include \
		-I$(SDKDIR)/system/include \
		$(PON_CFLAGS) -DCONFIG_SDK_KERNEL_LINUX -DCONFIG_RTK_NIC_RX_HOOK


MODULE_NAME = nicHook
PWD = $(shell pwd)


obj-y := $(MODULE_NAME).o
$(MODULE_NAME)-objs:= nichook.o
 

all:
	@echo " BUILD module $(MODULE_NAME), $(FLAGS)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)

 

install:
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules

 

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko


