PRODUCT_VERSION     := RTK_SDK
PRODUCT_NAME        := $(PRODUCT_VERSION)

export OS_TYPE      := uClinux
export ROOT_PATH    := $(shell pwd)/drivers/net/ethernet/realtek/rtl82xx
export KERNEL_DIR   := kernel
export MODULE_DIR   := module
export SDK_DIR      := sdk
export SDK_DAL_DIR  := sdk/src/dal
export SDK_SYS_DIR  := sdk/system
LOADER_DIR          := loader/u-boot-2011.12
TURNKEY_DIR         := turnkey
BIN_DIR             := bin
DIFF_LOG            := $(OS_TYPE)_diff.log

KERNELRELEASE       := linux-4.4.x

RTLPHY_NAME         := rtl8261
#RTLPHY_NAME         := example
#RTLPHY_NAME         := phy
include $(ROOT_PATH)/$(MODULE_DIR)/$(RTLPHY_NAME)/lkm/Makefile

export PRODUCT_NAME
.PHONY : all loader both menuconfig clean loaderclean distclean image

sinclude make/Makefile.package

all:
	make -C $(KERNEL_DIR)/$(OS_TYPE)

ni:
	make -C $(KERNEL_DIR)/$(OS_TYPE) ni

loader:
	make -C $(LOADER_DIR)

both: loader all

final:
	make -C $(KERNEL_DIR)/$(OS_TYPE) final

menuconfig:
	make -C $(KERNEL_DIR)/$(OS_TYPE) menuconfig

sdkconfig:
	make -C $(SDK_DIR)/config menuconfig

sysconfig:
	make -C $(TURNKEY_DIR)/config menuconfig

devmod:
	make -C $(KERNEL_DIR)/$(OS_TYPE) devmod

clean:module_clean
	make -C $(KERNEL_DIR)/$(OS_TYPE) clean

devclean:
	make -C $(KERNEL_DIR)/$(OS_TYPE) devclean

loaderclean:
	make -C $(LOADER_DIR) clean

cparser:
	make -f sdk/build/Makefile.diagtree_gen "RTK_SDKDIR=${PWD}/sdk" diag-tree

cparser-clean:
	make -f sdk/build/Makefile.diagtree_gen "RTK_SDKDIR=${PWD}/sdk" diag-tree-clean

cparser-phyOnly:
	make -f sdk/build/Makefile.diagtree_gen "RTK_SDKDIR=${PWD}/sdk" "PHY_ONLY:=y" diag-tree

distclean:module_clean
	make -C $(KERNEL_DIR)/$(OS_TYPE) distclean

module_clean:
	@if [ -d $(MODULE_DIR) ]; then \
		$(MAKE) new_all-c; \
	fi

svnup:
	cd $(TURNKEY_DIR) && svn up
	cd $(SDK_DIR) && svn up
	cd $(LOADER_DIR) && svn up
	cd $(KERNEL_DIR)/$(OS_TYPE) && svn up
	@echo "SVN update $(TURNKEY_DIR) $(SDK_DIR), $(LOADER_DIR) $(KERNEL_DIR)/$(OS_TYPE) done!"

svndiff:
	cd $(TURNKEY_DIR) && svn diff > ../$(DIFF_LOG)
	cd $(SDK_DIR) && svn diff > ../$(DIFF_LOG)
	cd $(LOADER_DIR) && svn diff >> ../$(DIFF_LOG)
	cd $(KERNEL_DIR)/$(OS_TYPE) && svn diff >> ../../$(DIFF_LOG)
	@echo "svn diff start["
	@cat $(DIFF_LOG)
	@echo "]svn diff end"
	@echo "SVN diff $(TURNKEY_DIR) $(SDK_DIR), $(LOADER_DIR) $(KERNEL_DIR)/$(OS_TYPE) done!"
release:
	find . -name .svn | xargs rm -rf
	$(BIN_DIR)/sdk_release.sh $(OS_TYPE)
	rm -rf $(BIN_DIR)
	@echo "Software release package is done!"

package:
	tar xvf sdkPackage_*.tar.bz2
package_clean:
	rm -rf kernel/uClinux/linux-2.6.x
	rm -rf kernel/uClinux/linux-3.18.24.x
	rm -rf kernel/uClinux/lib
	rm -rf kernel/uClinux/uClibc-0.9.33
	rm -rf kernel/uClinux/uClibc-0.9.33-485-318
	rm -rf kernel/uClinux/user/busybox
	rm -rf kernel/uClinux/user/busybox-1.18.3

sinclude make/Makefile
sinclude kernel/uClinux/vendors/Realtek/SDK/defConf/defConf.mk
