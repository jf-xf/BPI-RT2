#APOLLO_SDK_PATH = ../../vendors/Realtek/apollo/sdk
#APOLLO_SDK_PATH = $(SDKDIR)
#ccwei add
# Mason Yu. patch_apollo
#APOLLODIR = $(LINUX_DIR)/drivers/net/ethernet/realtek/rtl86900
SDKDIR = $(APOLLODIR)/sdk
#KDIR = $(LINUX_DIR)
KERNEL_DIR = $(KDIR)
#export APOLLODIR
export SDKDIR
#export KDIR
export KERNEL_DIR
APOLLO_SDK_PATH = $(APOLLODIR)/sdk
CFLAGS += -include $(KDIR)/include/generated/autoconf.h
#include $(KDIR)/.config
include $(APOLLODIR)/Compiler_Flag
#CONFIG_FPGA_MODE=n
#CONFIG_VIRTUAL_DRV_MODE=n
CLI_PATH:= $(APOLLO_SDK_PATH)/src/app/diag_rtk/config
CPARSER_FILE:=$(APOLLO_SDK_PATH)/src/app/diag_rtk/src/parser/cparser_tree.c
diag_cli=$(wildcard $(CLI_PATH)/*.cli)
#export CONFIG_FPGA_MODE CONFIG_VIRTUAL_DRV_MODE


all: $(CPARSER_FILE)
	$(MAKE) -C $(APOLLO_SDK_PATH)/src all
#	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest all
ifeq ($(CONFIG_SDK_DRIVER_UNITTEST),y)
	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest
endif
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app

diag: check_tree
	$(MAKE) -C $(APOLLO_SDK_PATH)/src all
ifeq ($(CONFIG_SDK_DRIVER_UNITTEST),y)
	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest
endif
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app


$(CPARSER_FILE): $(diag_cli)
	$(MAKE) shell_tree;

clean:

	$(MAKE) -C $(APOLLO_SDK_PATH)/src clean
ifeq ($(CONFIG_SDK_DRIVER_UNITTEST),y)
	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest clean
endif
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app clean

#Mason Yu. patch_apollo
shell_tree:
	$(MAKE) -C $(APOLLO_SDK_PATH) shell_tree
	
romfs:
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app install
	cp -af $(APOLLO_SDK_PATH)/src/app/bin/* $(ROMFSDIR)/bin/
