################################################################################
#                 Makefile Arch. Gen-2
################################################################################

.PHONY : helpme package_help

#
# Attribute definition
#
SPECIFIC_PROJECTS			:= k u dpa rise uextc phyModule
SPECIFIC_PROJECTS_withCfg	:= $(addsuffix _withCfg,$(SPECIFIC_PROJECTS))
KernelModeBasic				:= rt_kernel rtcore-k rtcore-i rtk-k rtk-i rtdrv-k rtdrv-i rtnic-k rtnic-i rtusr-so rtusr-i 
BUILD_GROUP_rest			:= diag-e diag-i rt_sdk_app sdk_romfs image

# projects:
BUILD_GROUP_rise            := $(KernelModeBasic) rise-k rise-i $(BUILD_GROUP_rest)
BUILD_CONFIG_rise           := 93xxs config=1
BUILD_GROUP_dpa             := $(KernelModeBasic) rtsdn-so rtsdn-i $(BUILD_GROUP_rest)
BUILD_CONFIG_dpa            := 9310 config=1
BUILD_GROUP_k               := $(KernelModeBasic) $(BUILD_GROUP_rest)
BUILD_GROUP_u               := rt_kernel rtcoreu-k rtcoreu-i rtcore-so rtcore-i rtk-so rtk-i 
BUILD_GROUP_NL              := rtcore-a rtk-a
BUILD_GROUP_uextc           := rtcoreu_lkm-k rtsdk-a usrApp-e 
BUILD_CONFIG_uextc          := 9310uextc
BUILD_GROUP_phyModule       := phy-a phy-so example-e
BUILD_CONFIG_pphyModule     := phyOnly config=1


# define default BUILD_GROUP
ifeq ($(CONFIG_SDK_KERNEL_LINUX),y)
    ifeq ($(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE),y)
      BUILD_GROUP           := $(BUILD_GROUP_k)
      DEBUG_MSG             := KERNEL_MODE
    else ifeq ($(CONFIG_SDK_KERNEL_LINUX_USER_MODE),y)
      BUILD_GROUP           := $(BUILD_GROUP_u)
      DEBUG_MSG             := USER_MODE
    else
      $(warning: Should define one of CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE or CONFIG_SDK_KERNEL_LINUX_USER_MODE)
    endif
else
    BUILD_GROUP             := $(BUILD_GROUP_NL)
    DEBUG_MSG               := OTHER_OS
endif
BUILD_GROUP                 := $(BUILD_GROUP_k)


#
# make
#
group g:
	$(foreach target,$(BUILD_GROUP), @rm -f newMake_*;touch "newMake_make_$(target)"; make $(target) || exit "$$?";)
$(SPECIFIC_PROJECTS):
	$(foreach target,$(BUILD_GROUP_$@), make $(target) || exit "$$?";)
$(SPECIFIC_PROJECTS_withCfg):
	make $(BUILD_CONFIG_$(subst _withCfg,,$@))
	make $(subst _withCfg,,$@)


#
# for module
#
#ALL_MODULE          := rtk rtcore rise rtdrv rtnic rtusr rtsdn diag
ALL_MODULE          := $(shell ls module)
ALL_TYPES           := lkm a so exe
APP_LKM             := $(addsuffix -k,$(ALL_MODULE))
APP_LIB             := $(addsuffix -a,$(ALL_MODULE))
APP_SO              := $(addsuffix -so,$(ALL_MODULE))
APP_EXE             := $(addsuffix -e,$(ALL_MODULE))
APP_ALL             := $(addsuffix -all,$(ALL_MODULE))
APP_CLEAN           := $(addsuffix -c,$(ALL_MODULE))
APP_INSTALL         := $(addsuffix -i,$(ALL_MODULE))

$(APP_LKM):
	make -C module/$(subst -k,,$@)/lkm
$(APP_LIB):
	make -C module/$(subst -a,,$@)/a
$(APP_SO):
	make -C module/$(subst -so,,$@)/so
$(APP_EXE):
	make -C module/$(subst -e,,$@)/exe
$(APP_ALL):
	make -C module/$(subst -all,,$@)/lkm
	make -C module/$(subst -all,,$@)/a
	make -C module/$(subst -all,,$@)/so
	make -C module/$(subst -all,,$@)/exe
$(APP_CLEAN):
	@$(foreach type,$(ALL_TYPES), \
	  if [ -d module/$(subst -c,,$@)/$(type) ]; then make -C module/$(subst -c,,$@)/$(type) clean; fi;)
$(APP_INSTALL):
	@$(foreach type,$(ALL_TYPES), \
	  if [ -d module/$(subst -i,,$@)/$(type) ]; then make -C module/$(subst -i,,$@)/$(type) install; fi;)


#
# misc
#
new_all:
	@$(foreach module_app,$(ALL_MODULE), \
	    $(foreach type,$(ALL_TYPES), \
	        if [ -d module/$(module_app)/$(type) ]; then make -C module/$(module_app)/$(type); fi; \
	    ) \
    )
new_all-c:
	@$(foreach module_app,$(ALL_MODULE), \
	    $(foreach type,$(ALL_TYPES), \
	        if [ -d module/$(module_app)/$(type) ]; then make $(module_app)-c; fi; \
	    ) \
    )

#
# to connect to original makefile system
#
rt_kernel:
	@if [ ! -d $(KERNEL_DIR)/$(OS_TYPE)/romfs ]; then \
		mkdir -p $(KERNEL_DIR)/$(OS_TYPE)/romfs; \
	fi
	make -C $(KERNEL_DIR)/$(OS_TYPE) rtKernel

rt_sdk_app:
	make -C $(KERNEL_DIR)/$(OS_TYPE) rtSdkApp

sdk_romfs:
	make -C $(KERNEL_DIR)/$(OS_TYPE) romfs.subdirs2

image:
	make -C $(KERNEL_DIR)/$(OS_TYPE) post_setup
	make -C $(KERNEL_DIR)/$(OS_TYPE) post_mkimage

rt_loader:
	make -C $(LOADER_DIR)

#
# help message
#
helpme:
	@printf "\nApp Make:\n"
	@printf "   make MODULE-k           (Linux Kernel Module)\n"
	@printf "   make MODULE-a           (Static lib)\n"
	@printf "   make MODULE-so          (Shared lib)\n"
	@printf "   make MODULE-e           (Executable)\n"
	@printf "   make MODULE-all         (Build all types)\n"
	@printf "   \n"
	@printf "   make MODULE-c           (Clean all types)\n"
	@printf "   make MODULE-i           (Install to romfs)\n"
	@printf "   make image              (output image)\n"
	@printf "   make rise               (build whole rise project)\n"
	@printf "   make rise_withCfg     	(build whole rise project with default cfg)\n"
	@printf "   make dpa                (build whole dpa project)\n"
	@printf "   make dpa_withCfg     	(build whole dpa project with default cfg)\n"

package_help:
	@printf "\nPackage commands:\n"
	@printf "   make package            (extract package)\n"
	@printf "   make package_clean      (clean package)\n"
