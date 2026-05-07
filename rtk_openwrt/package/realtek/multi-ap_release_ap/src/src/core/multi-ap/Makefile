# Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

OUTPUT_FOLDER              := output

RELEASE_FOLDER             := $(shell pwd)/../../../

SHARED_FOLDER              := shared

ifeq ($(FLAVOUR),jungle)
SDK_PREFIX                 := jungle
else
SDK_PREFIX                 := luna
endif

CONTROLLER_EXE             := $(OUTPUT_FOLDER)/map_controller
AGENT_EXE                  := $(OUTPUT_FOLDER)/map_agent

CONTROLLER_SRC_MAIN        := controller_main.c
CONTROLLER_OBJ_MAIN        := $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(CONTROLLER_SRC_MAIN))

AGENT_SRC_MAIN             := agent_main.c
AGENT_OBJ_MAIN             := $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(AGENT_SRC_MAIN))

INI_PARSER_SRC             := $(SHARED_FOLDER)/ini.c
CONFIG_HANDLER_SRC         := $(SHARED_FOLDER)/config_file_handler.c

CONFIG_OBJ                 := $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(INI_PARSER_SRC))
CONFIG_OBJ                 += $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(CONFIG_HANDLER_SRC))

SHARED_SRC                 := $(wildcard $(SHARED_FOLDER)/*.c)
SHARED_SRC                 += $(wildcard $(SHARED_FOLDER)/$(SDK_PREFIX)/*.c)

SHARED_OBJ                 := $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(SHARED_SRC))

ifeq ($(MAP_DEBUG),1)
	CCFLAGS       := -g -fno-omit-frame-pointer
	ifeq ($(DYNAMIC_CHECK),1)
	CCFLAGS       += -pg -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
	endif
else
	CCFLAGS       := -O2
endif

CCFLAGS       += -Wall -Wextra -Werror
CCFLAGS       += -DRTK_MULTI_AP -DWIFI_SIMPLE_CONFIG -DEASY_MESH_VERSION="\"V2.0\""

ifeq ($(FLAVOUR),openwrt)
	ifeq ($(USE_COMMOM_STRUCTURE), y)
		CCFLAGS       += -D_DRIVER_COMMON_STRUCTURE_
		CCFLAGS       += -D_BACKPORTS_DRIVER_
	endif

	ifeq ($(CONFIG_RTK_DEV_AP),y)
		LDFLAGS       +=  -lssl -lcrypt
		CCFLAGS       += -DRTK_RETRY_GET_FLOCK
	else # PON/XDSL(SD5/SD9) platform
		CCFLAGS       += -DCONFIG_RTK_PON_XDSL
		ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/rtl8192cd)","")
			EXTERNAL_INC  := $(MIB_INC) $(shell pwd)/include $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/rtl8192cd/ $(ROOTDIR)/include
		else ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/rtl8192cd)","")
			EXTERNAL_INC  := $(MIB_INC) $(shell pwd)/include $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/rtl8192cd/ $(ROOTDIR)/include
		endif
	endif

else ifeq ($(FLAVOUR),luna)
	ifeq ($(USE_COMMOM_STRUCTURE), y)
		CCFLAGS       += -D_DRIVER_COMMON_STRUCTURE_
		CCFLAGS       += -D_BACKPORTS_DRIVER_
	endif

	ifeq ($(CONFIG_RTK_DEV_AP),y)
		MIB_INC       := $(shell pwd)/../../boa/src/LINUX/
		MIB_LDFLAGS   := -L../../boa/src-project/LINUX $(LIBMIB) $(LIBVOIP)
		LDFLAGS       +=  -lssl -lcrypt
		CCFLAGS       += -DRTK_RETRY_GET_FLOCK
	else # PON/XDSL(SD5/SD9) platform
		CCFLAGS       += -DCONFIG_RTK_PON_XDSL
		ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/rtl8192cd)","")
			EXTERNAL_INC  := $(MIB_INC) $(shell pwd)/include $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/rtl8192cd/ $(ROOTDIR)/include
		else ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/rtl8192cd)","")
			EXTERNAL_INC  := $(MIB_INC) $(shell pwd)/include $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/rtl8192cd/ $(ROOTDIR)/include
		endif
	endif

	ifeq ($(USE_ELINKSDK_SUPPORT),y)
		LDFLAGS     += -L$(shell pwd)/../../lib_elinksdk -lelinksdk
	endif

	LDFLAGS       += -L$(ROOTDIR)/lib/libssl/

else ifeq ($(FLAVOUR),jungle)
	LDFLAGS       += -L$(OPENSSL_DIR)/
	MIB_INC       := $(shell pwd)/../../boa/apmib/
	ifeq ($(USE_BACKPORTS),y)
		CCFLAGS += -D_BACKPORTS_DRIVER_
	endif
endif

CCFLAGS       += $(COMMON_CFLAGS) $(EXTRA_CFLAGS)
ifdef MIB_INC
LDFLAGS       += -L$(MIB_INC)
endif
LDFLAGS       += -L$(shell pwd)/lib/
LDFLAGS       += -lrt -lpthread
LDFLAGS       += -lcrypto

ifeq ($(MAP_DEBUG),1)
	ifeq ($(DYNAMIC_CHECK),1)
	LDFLAGS       += -pg -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
	endif
endif

EXTRA_LDFLAGS := -lmultiap -lmultiap_dpp -lmultiap_crypto -lmultiap_addon

EXTRA_LDFLAGS += -lmapvendor

INTERNAL_INC  := $(shell pwd)/include $(SHARED_FOLDER) $(SHARED_FOLDER)/$(SDK_PREFIX)

EXTERNAL_INC  := $(MIB_INC)

LIB_MAPVENDORDATA_LDFLAGS := -lmapVendorData
EXTRA_LDFLAGS += -lm
CCFLAGS += -fPIC

CCFLAGS += $(ARCH_CFLAGS)
EXTRA_LDFLAGS += $(ARCH_LDFLAGS)
################################################################################
# Targets
################################################################################

.PHONY: all
all:
	$(MAKE) clean
	$(MAKE) customized
	$(MAKE) binary

.PHONY: customized
customized:
	$(MAKE) -C customized

.PHONY: binary
binary: $(CONTROLLER_EXE) $(AGENT_EXE)

$(CONTROLLER_EXE): $(CONTROLLER_OBJ_MAIN) $(SHARED_OBJ)
	$(CC) $(CONTROLLER_OBJ_MAIN) $(SHARED_OBJ) $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) -L$(OUTPUT_FOLDER) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LIB_MAPVENDORDATA_LDFLAGS) -o $@

$(CONTROLLER_OBJ_MAIN)    : $(OUTPUT_FOLDER)/%.o : %.c
	mkdir -p $(OUTPUT_FOLDER)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $< -o $@

$(AGENT_EXE): $(AGENT_OBJ_MAIN) $(SHARED_OBJ)
	$(CC) $(AGENT_OBJ_MAIN) $(SHARED_OBJ) $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) -L$(OUTPUT_FOLDER) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LIB_MAPVENDORDATA_LDFLAGS) -o $@

$(AGENT_OBJ_MAIN)    : $(OUTPUT_FOLDER)/%.o : %.c
	mkdir -p $(OUTPUT_FOLDER)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $< -o $@

########### Shared Objects ###########

$(SHARED_OBJ)    : $(OUTPUT_FOLDER)/%.o : %.c
	mkdir -p $(OUTPUT_FOLDER)/shared/$(SDK_PREFIX)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $< -o $@

.PHONY: clean
clean:
	rm -rf $(CONTROLLER_EXE) $(AGENT_EXE) $(MAP_REINIT_EXE) $(MAP_RESET_EXE) $(LIB_MAPVENDORDATA_SO) $(MAP_VLAN_EXE)
	rm -rf $(OUTPUT_FOLDER)/*.o
	rm -rf $(OUTPUT_FOLDER)/**/*.o
	rm -rf $(OUTPUT_FOLDER)/**/**/*.o
	$(MAKE) -C customized clean

.PHONY: release
release:
	cp $(CONTROLLER_EXE) $(AGENT_EXE) $(RELEASE_FOLDER)/target
	cp $(shell pwd)/../multi-ap-core/output/hle_entity $(RELEASE_FOLDER)/target
ifeq ($(BUILD_AL_SHARED),1)
	cp lib/libmultiap.so $(RELEASE_FOLDER)/target/lib
endif
ifeq ($(BUILD_SHARED_MODULES),1)
	cp lib/libmultiap_crypto.so $(RELEASE_FOLDER)/target/lib
	cp lib/libmultiap_dpp.so $(RELEASE_FOLDER)/target/lib
endif
	cp include/* $(RELEASE_FOLDER)/src/include
	cp $(SHARED_FOLDER)/config_file_handler.* $(SHARED_FOLDER)/ini.* $(SHARED_FOLDER)/easymesh_datamodel.h $(MAP_REINIT_SRC_MAIN) $(MAP_RESET_SRC_MAIN) $(MAP_VLAN_SRC_MAIN) $(MAP_CHECKER_SRC_MAIN) $(RELEASE_FOLDER)/src/tools/src

.PHONY: romfs
romfs:
	$(ROMFSINST) $(CONTROLLER_EXE) /bin/map_controller
	$(ROMFSINST) $(AGENT_EXE) /bin/map_agent
	$(ROMFSINST) $(shell pwd)/../multi-ap-core/output/hle_entity /bin/hle_entity
ifeq ($(BUILD_AL_SHARED),1)
	$(ROMFSINST) lib/libmultiap.so /lib/libmultiap.so
endif
ifeq ($(BUILD_SHARED_MODULES),1)
	$(ROMFSINST) lib/libmultiap_crypto.so /lib/libmultiap_crypto.so
	$(ROMFSINST) lib/libmultiap_dpp.so /lib/libmultiap_dpp.so
endif
