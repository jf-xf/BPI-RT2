include $(APOLLODIR)/Compiler_Flag

## SUB MODULE NAME ##
SUB_MODULE = $(CONFIG_RTK_L34_MODULE_STRING)

## MODULE NAME ##
CORE_MODULE_NAME = fc_$(patsubst "%",%,$(SUB_MODULE))
MGR_MODULE_NAME = fc_mgr
ASSISTANT_MODULE_NAME = fc_assistant

## MODULE PATH ##
FC_DRVFOLDER = FleetConntrackDriver
FC_COREFOLDER = core
FC_COREFOLDER_OBJ = release_$(patsubst "%",%,$(SUB_MODULE))_obj
FC_SRCFOLDER = src
FC_ASICDRVFOLDER = asicDrv
FC_ONEIMAGE_77C = oneimage_8277c
FC_DRVDIR = $(APOLLODIR)/$(FC_DRVFOLDER)
FC_COREDIR = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_COREFOLDER)
FC_COREDIR_OBJ = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_COREFOLDER_OBJ)
FC_SRCDIR = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_SRCFOLDER)
FC_VER_FILE = rtk_fc_ver.h

FC_BUILD_SRC = $(shell test -e $(FC_COREDIR) && echo -n y)
srctree=$(KDIR)

export FC_DRVFOLDER FC_COREFOLDER FC_SRCFOLDER FC_ONEIMAGE_77C FC_DRVDIR FC_COREDIR FC_SRCDIR
#$(info fc cc flags = $(EXTRA_CFLAGS))

## COMPILE FLAGS ##
FC_CFLAGS += -Wall \
		-Werror \
		-O2 \
		$(PON_CFLAGS) \
		-I$(FC_COREDIR) \
		-I$(FC_DRVDIR) \
		-I$(FC_DRVDIR)/include \
		-I$(FC_COREDIR)/$(FC_ASICDRVFOLDER) \
		-I$(APOLLODIR)/nicDriver \
		-I$(APOLLODIR)/sdk/include \
		-I$(APOLLODIR)/sdk/system/include \
		-I$(APOLLODIR)/sdk/src/app/diag_rtk/include \
		-I$(KDIR)/net/bridge \
		-I$(KDIR)/fs/proc \
		-I$(KDIR)/include/soc/cortina \

ifeq ($(CONFIG_OPENWRT_SDK),y)
	FC_CFLAGS += -include ${TOPDIR}/autoconf.h
endif
				
ifeq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
	FC_CFLAGS += -I$(srctree)/drivers/net
else
	FC_CFLAGS += -I$(srctree)/drivers/net/ethernet/realtek
endif

FC_CFLAGS += -I$(FC_DRVDIR)/rt/include

FC_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX

ifeq ($(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID), y)
FC_CFLAGS += -DCONFIG_RG_RTL9607C_SERIES
endif
ifeq ($(CONFIG_FC_RTL9607C_SERIES), y)
FC_CFLAGS += -DCONFIG_RG_RTL9607C_SERIES
endif
ifeq ($(CONFIG_FC_RTL9603CVD_SERIES), y)
FC_CFLAGS += -DCONFIG_RG_RTL9603CVD_SERIES
endif


ifeq ($(CONFIG_RTK_L34_G3_PLATFORM), y)

ifneq ($(CONFIG_FC_CA8277B_RTL8277C_HYBRID), y)

FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include
#LINUXINCLUDE += -include $(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include/generated/ca_ne_autoconf.h

ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-77c/include
endif
ifeq ($(CONFIG_ARCH_REALTEK_9607F), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-07f/include
endif
ifeq ($(CONFIG_ARCH_CORTINA_VENUS), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include
endif
endif #CONFIG_FC_CA8277B_RTL8277C_HYBRID

ifeq ($(CONFIG_ARCH_CORTINA_G3HGU), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal/include
endif

endif

## VERSION CONTROL ##
ifeq ($(FC_BUILD_SRC),y)

GIT_VER_CTRL = $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) remote get-url origin | grep -q fleetConntrack && echo -n y || echo -n n)

ifeq ($(GIT_VER_CTRL),y)
FC_RELEASE_VER := $(shell bash $(FC_COREDIR)/getGitTagNumber.sh $(FC_DRVDIR))
FC_PATCH_VER := $(shell bash $(FC_COREDIR)/getGitPatchNumber.sh $(FC_DRVDIR))
FC_CUR_SHA1 := $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) rev-parse HEAD | cut -c1-8)
FC_CUR_SHA1_DATE_RAW := $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) log -n1 --format="%at")
FC_CUR_SHA1_DATE := $(shell date -d @$(FC_CUR_SHA1_DATE_RAW) +%Y-%m-%d-%H%M)
FC_CUR_SHA1_AUTHOR := $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) log -n1 --format="%an")
FC_VERSION := $(FC_RELEASE_VER).$(FC_PATCH_VER)
FC_COMPILE_BY := $(shell whoami | sed 's/\\/\\\\/')
FC_COMPILE_DATE := $(shell date "+%Y-%m-%d-%H%M")

# update version file
$(shell echo \#define FC_GIT_VERSION "\""$(FC_VERSION)"\"" > $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_GIT_SHA1 "\""$(FC_CUR_SHA1)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_GIT_SHA1_DATE "\""$(FC_CUR_SHA1_DATE)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_GIT_AUTHOR "\""$(FC_CUR_SHA1_AUTHOR)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_COMPILE_BY "\""$(FC_COMPILE_BY)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_COMPILE_DATE "\""$(FC_COMPILE_DATE)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))

endif #GIT_VER_CTRL

#$(info fc debug Makefile = $(GIT_VER_CTRL) $(FC_CUR_SHA1))
FC_VER_CONTROL = $(shell test -e $(FC_DRVDIR)/$(FC_VER_FILE) && echo -n y)
ifeq ($(FC_VER_CONTROL),y)
FC_CFLAGS += -DFC_VER_CONTROL=$(FC_VER_CONTROL)
endif

FC_CORE_BUILD_FOLDER = $(FC_COREFOLDER)
FC_ASICDRV_BUILD_FOLDER = $(FC_ASICDRVFOLDER)

else #!FC_BUILD_SRC

FC_CORE_BUILD_FOLDER = $(FC_COREFOLDER_OBJ)
FC_ASICDRV_BUILD_FOLDER =

endif #FC_BUILD_SRC

export FC_CFLAGS FC_VERSION
EXTRA_CFLAGS += $(FC_CFLAGS)

#$(info fc cc flags = $(FC_CFLAGS))

## ADDITIONAL CONTROL ##
ifeq ($(CONFIG_FC_CA8277B_RTL8277C_HYBRID), y)
CORE_MODULE_NAME := fc_8277b
MGR_MODULE_NAME := fc_mgr_8277b

FC_COREFOLDER_2ND := $(FC_ONEIMAGE_77C)\/core
FC_SRCFOLDER_2ND := $(FC_ONEIMAGE_77C)\/src
FC_COREDIR_2ND = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_COREFOLDER_2ND)
FC_SRCDIR_2ND = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_SRCFOLDER_2ND)

ifeq ($(FC_BUILD_SRC),y)
core-file = $(shell find ${FC_COREDIR}/ -type f \( -iname "*.c" \))
$(shell mkdir -p $(FC_COREDIR_2ND))
$(shell mkdir -p $(FC_COREDIR_2ND)/$(FC_ASICDRVFOLDER))
$(shell for i in $(core-file); do ln -sf $$i $$(echo $$i | sed 's/$(FC_COREFOLDER)/$(FC_COREFOLDER_2ND)/') ;done )
endif #FC_BUILD_SRC

src-file = $(shell find ${FC_SRCDIR}/ -type f \( -iname "*.c" \))
$(shell mkdir -p $(FC_SRCDIR_2ND))
$(shell for i in $(src-file); do ln -sf $$i $$(echo $$i | sed 's/$(FC_SRCFOLDER)/$(FC_SRCFOLDER_2ND)/') ;done )

#LINUXINCLUDE += -include $(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-venus/include/generated/ca_ne_autoconf.h
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-venus/include
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-venus/aal-gen2/include
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-venus/ni-drv-gen2/
EXTRA_CFLAGS += -DCONFIG_ARCH_CORTINA_VENUS -DCONFIG_CA8277B_SERIES -DCONFIG_FC_CA8277B_SERIES -DONE_IMAGE_VENUS
#EXTRA_CFLAGS:=$(filter-out -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-77c/include,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/ni-drv-gen2,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/ni-drv-77c,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/aal-gen2/include,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/aal-77c/include,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_ARCH_CORTINA_TAURUS,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_RTL8277C_SERIES,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_FC_RTL8277C_SERIES,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DONE_IMAGE_TAURUS,$(EXTRA_CFLAGS))

endif #CONFIG_FC_CA8277B_RTL8277C_HYBRID


## CORE MODULE FILES ##
RTK_FC_OBJ = $(FC_CORE_BUILD_FOLDER)/rtk_fc_driver.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_debug.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_internal.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_external.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_multicast.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_proc.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_mappingAPI.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_callback.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_dualHeader.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_acl.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_mapper_api.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_vxlan.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_ipsec.o \

export RTK_FC_OBJ
		
ifeq ($(CONFIG_RTL9607C_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_apolloPro_asicDriver.o \
                $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_apolloPro_internal.o
else ifeq ($(CONFIG_RTL9603CVD_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_rtl9603cvd_asicDriver.o \
                $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_apolloPro_internal.o
else ifeq ($(CONFIG_FC_RTL8277C_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_8277c_asicDriver.o
else ifeq ($(CONFIG_FC_RTL9607F_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_9607f_asicDriver.o
else ifeq ($(CONFIG_RTK_L34_G3_PLATFORM), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_g3_internal.o \
		$(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_ca8277_asicDriver.o				
endif

## MGR MODULE FILES ##
RTK_FC_MGR_OBJ = src/rtk_fc_mgr.o \
		src/rtk_fc_mgrTRx.o \
		src/rtk_fc_helper.o \
		src/rtk_fc_helper_wlan.o \
		src/rtk_fc_helper_ps.o \
		src/rtk_fc_helper_rt_api.o \
		src/rtk_fc_helper_common.o \
		src/rtk_fc_helper_multicast.o

export RTK_FC_MGR_OBJ


#####################################
## FC ASSISTANT MODULE            ###
#####################################
ASSISTANT_MODULE_MODE=y

RTK_FC_ASSISTANT_OBJ = src/rtk_fc_assistant.o
obj-$(ASSISTANT_MODULE_MODE) += $(ASSISTANT_MODULE_NAME).o
$(ASSISTANT_MODULE_NAME)-objs := $(RTK_FC_ASSISTANT_OBJ)


ifneq ($(FC_BUILD_SRC),y)
#####################################
### FC CORE MODULE - build binary ###
#####################################
FC_LATEST_OBJ = $(shell find $(FC_DRVDIR) -name "$(CORE_MODULE_NAME).r.*" | sort -n | tail -n 1)

ifeq ($(CONFIG_RTK_L34_FC_KERNEL_MODULE),y)
# built-in

CORE-OBJ-RFILES := $(shell find ${FC_COREDIR_OBJ}/ -type f \( -iname "*.r" \))
$(shell for i in $(CORE-OBJ-RFILES); do ln -sf $$i $$(echo $$i | sed 's/\.o.r/\.o/') ;done )

obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(CORE_MODULE_NAME).o
$(CORE_MODULE_NAME)-objs := $(RTK_FC_OBJ) $(RTK_FC_ASIC_OBJ)
else
# module
ifeq ($(shell test $(VERSION) -gt 4; echo $$?),0)
$(shell ln -sf $(FC_LATEST_OBJ) $(FC_DRVDIR)/$(CORE_MODULE_NAME).o )
obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(CORE_MODULE_NAME).o
else
obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(CORE_MODULE_NAME).o
$(CORE_MODULE_NAME)-objs :=  $(shell basename $(FC_LATEST_OBJ))
endif
endif


else #!FC_BUILD_SRC
#####################################
### FC CORE MODULE - build source ###
#####################################

$(shell touch $(FC_COREDIR)/rtk_fc_driver.c)

obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(CORE_MODULE_NAME).o
$(CORE_MODULE_NAME)-objs := $(RTK_FC_OBJ) $(RTK_FC_ASIC_OBJ)


ifeq ($(CONFIG_RTK_L34_FC_KERNEL_MODULE),y)
$(shell mkdir -p $(FC_COREDIR_OBJ))
CORE-OBJ := $(shell find ${FC_COREDIR}/ -type f \( -iname "*.o" \))
$(shell for i in $(CORE-OBJ); do cp -f $$i $(FC_COREDIR_OBJ) ;done )
CORE-OBJ-COPY := $(shell find ${FC_COREDIR_OBJ}/ -type f \( -iname "*.o" \))
$(shell for i in $(CORE-OBJ-COPY); do mv $$i $$(echo $$i | sed 's/\.o/\.o.r/') ;done )

else
$(shell rm -rf $(FC_DRVDIR)/$(CORE_MODULE_NAME).r.*)
$(shell test -e $(FC_DRVDIR)/$(CORE_MODULE_NAME).o && cp -rf $(FC_DRVDIR)/$(CORE_MODULE_NAME).o $(FC_DRVDIR)/$(CORE_MODULE_NAME).r.$(FC_VERSION))
endif


endif #FC_BUILD_SRC
#####################################
### FC MANAGER MODULE             ###
#####################################
obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(MGR_MODULE_NAME).o
$(MGR_MODULE_NAME)-objs := $(RTK_FC_MGR_OBJ)
                                
####################################
### FC DUAL MODULE SUPPORT       ###
####################################
ifeq ($(CONFIG_FC_CA8277B_RTL8277C_HYBRID), y)
obj-$(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)+= $(FC_ONEIMAGE_77C)/
endif
