ifeq ($(FLAVOUR),openwrt)
	SDK_PREFIX             := openwrt
else ifeq ($(FLAVOUR),luna)
	SDK_PREFIX             := luna
else
	SDK_PREFIX             := jungle
endif

CC        := $(CROSS_COMPILE)gcc
CXX       := $(CROSS_COMPILE)g++
LD        := $(CROSS_COMPILE)ld
AR        := $(CROSS_COMPILE)ar
STRIP     := $(CROSS_COMPILE)strip

BUILD_FOLDER               := $(shell pwd)/build
NATIVE_BUILD_FOLDER        := $(shell pwd)/native_build

ifeq ($(FLAVOUR),luna)
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
	FILE_POSTFIX           := _sysconf
endif
# currently only use sysconf version for openwrt
else ifeq ($(FLAVOUR),openwrt)
	FILE_POSTFIX           := _sysconf
endif

MAP_REINIT_EXE             := map_reinit
MAP_REINIT_SRC_MAIN        := src/$(SDK_PREFIX)_map_reinit$(FILE_POSTFIX).c src/ini.c src/config_file_handler.c src/base64.c
MAP_REINIT_OBJ_MAIN        := $(patsubst src/%.c,$(BUILD_FOLDER)/%.o,$(MAP_REINIT_SRC_MAIN))

MAP_RESET_EXE              := map_reset
MAP_RESET_SRC_MAIN         := src/$(SDK_PREFIX)_map_reset$(FILE_POSTFIX).c
MAP_RESET_OBJ_MAIN         := $(patsubst src/%.c,$(BUILD_FOLDER)/%.o,$(MAP_RESET_SRC_MAIN))

MAP_VLAN_EXE               := map_vlan_config
ifneq ($(FLAVOUR),jungle)
MAP_VLAN_SRC_MAIN          := src/$(SDK_PREFIX)_map_vlan_config.c src/ini.c src/config_file_handler.c src/base64.c
else
MAP_VLAN_SRC_MAIN          := src/$(SDK_PREFIX)_map_vlan_config.c
endif
MAP_VLAN_OBJ_MAIN          := $(patsubst src/%.c,$(BUILD_FOLDER)/%.o,$(MAP_VLAN_SRC_MAIN))

MAP_CHECKER_EXE            := map_checker
MAP_CHECKER_SRC_MAIN       := src/$(SDK_PREFIX)_map_checker.c src/ini.c src/config_file_handler.c src/base64.c
MAP_CHECKER_OBJ_MAIN       := $(patsubst src/%.c,$(BUILD_FOLDER)/%.o,$(MAP_CHECKER_SRC_MAIN))

DPP_BOOTSTRAP_GEN                := dpp_bootstrap_gen
DPP_BOOTSTRAP_GEN_SRC_MAIN       := src/dpp_bootstrap_gen.c src/base64.c
DPP_BOOTSTRAP_GEN_OBJ_MAIN       := $(patsubst src/%.c,$(BUILD_FOLDER)/%.o,$(DPP_BOOTSTRAP_GEN_SRC_MAIN))

ifeq ($(FLAVOUR),luna)
MIB_INC       := $(ROOTDIR)/user/boa/src/LINUX/
else ifeq ($(FLAVOUR),jungle)
MIB_INC       := $(DIR_ROOT)/users/boa/apmib
CRYPTO_INC    := $(OPENSSL_DIR)/include
endif

ifeq ($(FLAVOUR),openwrt)
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
EXTERNAL_INC  := $(shell pwd)/../include $(KERNEL_BUILD_DIR)/rtk_wifi6/include $(ROOTDIR)/include
else
EXTERNAL_INC  := $(shell pwd)/../include $(KERNEL_BUILD_DIR)/rtk_wifi6_ap/include $(ROOTDIR)/include
endif
else ifeq ($(FLAVOUR),luna)
EXTERNAL_INC  := $(MIB_INC) $(shell pwd)/../include $(ROOTDIR)/$(LINUX_BASE)/drivers/net/wireless/realtek/rtl8192cd/ $(ROOTDIR)/include $(ROOTDIR)/lib/libssl/include
else
EXTERNAL_INC  := $(MIB_INC) $(shell pwd)/../include $(DIR_ROOT)/include $(CRYPTO_INC)
endif

CCFLAGS       := -Wall -Wextra -Werror -O2 $(addprefix -I,$(EXTERNAL_INC))

ifeq ($(FLAVOUR),luna)
LDFLAGS       += -L$(ROOTDIR)/lib
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
#LDFLAGS       += -lmib
CCFLAGS       += $(INCSSL)
else
LDFLAGS       += -L$(MIB_INC) $(LIBMIB) $(LIBVOIP)
LDFLAGS       += -lmib -lrtk
endif
else ifeq ($(FLAVOUR),jungle)
CCFLAGS       += $(COMMON_CFLAGS)
LDFLAGS       += -L$(MIB_INC) -lapmib
endif

ifeq ($(USE_ELINKSDK_SUPPORT),y)
	LDFLAGS     += -L$(shell pwd)/../../lib_elinksdk -L$(ROOTDIR)/user/ctcap/lib_elinksdk -lelinksdk
endif

ifeq ($(USE_RTK_MULTI_AP_WFA),y)
	CCFLAGS       += -DRTK_MULTI_AP_WFA
endif

ifeq ($(FLAVOUR),openwrt)
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
	CCFLAGS       += -DCONFIG_RTK_PON_XDSL
ifeq ($(CONFIG_RTL_11W_SUPPORT), y)
	CCFLAGS       += -DWLAN_11W
else ifeq ($(CPTCFG_RTL_11W_SUPPORT), y)
	CCFLAGS       += -DWLAN_11W
else ifeq ($(CPTCFG_RTW_80211W_SUPPORT),y)
	CCFLAGS       += -DWLAN_11W
endif
endif
endif

ifeq ($(FLAVOUR),luna)
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
	CCFLAGS       += -DCONFIG_RTK_PON_XDSL
ifeq ($(CONFIG_RTL_WPA3_SUPPORT), y)
	CCFLAGS       += -DWLAN_WPA3
else ifeq ($(CPTCFG_WIFI6_WPA3),y)
	CCFLAGS       += -DWLAN_WPA3
endif
ifeq ($(CONFIG_RTL_11W_SUPPORT), y)
	CCFLAGS       += -DWLAN_11W
else ifeq ($(CPTCFG_RTL_11W_SUPPORT), y)
	CCFLAGS       += -DWLAN_11W
else ifeq ($(CPTCFG_RTW_80211W_SUPPORT),y)
	CCFLAGS       += -DWLAN_11W
endif
ifeq ($(CONFIG_WIFI_SIMPLE_CONFIG), y)
	CCFLAGS       += -DCONFIG_WIFI_SIMPLE_CONFIG
endif
ifeq ($(CONFIG_RTL_SMUX_DEV), y)
	CCFLAGS       += -DWLAN_SMUX_DEV
else ifeq ($(CPTCFG_RTL_SMUX_DEV), y)
	CCFLAGS       += -DWLAN_SMUX_DEV
endif
else
	CCFLAGS       += -DWLAN_SMUX_DEV
endif # PON/XDSL(SD5/SD9) platform
ifeq ($(CONFIG_RTL8852AE_BACKPORTS),y)
ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/g6_wifi_driver)","")
CCFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/g6_wifi_driver
endif
else
ifneq ($(CONFIG_RTL8852AE)$(CONFIG_RTLWIFI6),)
ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/g6_wifi_driver)","")
CCFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/g6_wifi_driver
endif
else
ifneq ($(CONFIG_RTL8852CE)$(CONFIG_RTL8852DE),)
ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/g6_wifi_driver)","")
CCFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/g6_wifi_driver
endif
endif
endif
endif # CONFIG_RTL8852AE_BACKPORTS
endif # FLAVOUR luna

CCFLAGS      += -fPIC

CCFLAGS      += -DEASYMESH_ETH_MONITOR

ifeq ($(FLAVOUR),openwrt)
CRYPTO_LDFLAGS := -lcrypto
else ifeq ($(FLAVOUR),luna)
CRYPTO_LDFLAGS := -L$(ROOTDIR)/lib/libssl/ -lcrypto
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
CRYPTO_LDFLAGS += $(LIBSSL)
endif
else
CRYPTO_LDFLAGS := -L$(DIR_ROOT)/lib/libssl/ -L$(OPENSSL_DIR)/ -lcrypto
endif

MULTI_AP_DIR := $(shell pwd)/

export MULTI_AP_DIR

################################################################################
# Targets
################################################################################
.PHONY: all
all:
	$(MAKE) clean
	$(MAKE) binary

.PHONY: binary
ifeq ($(FLAVOUR),openwrt)
binary: $(MAP_VLAN_EXE) $(MAP_CHECKER_EXE) $(DPP_BOOTSTRAP_GEN)
else
binary: $(MAP_REINIT_EXE) $(MAP_RESET_EXE) $(MAP_VLAN_EXE) $(MAP_CHECKER_EXE) $(DPP_BOOTSTRAP_GEN)
endif

$(MAP_REINIT_EXE): $(MAP_REINIT_OBJ_MAIN)
	$(CC) $(MAP_REINIT_OBJ_MAIN) $(LDFLAGS) -o $@

$(MAP_RESET_EXE): $(MAP_RESET_OBJ_MAIN)
	$(CC) $(CCFLAGS) $(LDFLAGS) $< -o $@

$(MAP_VLAN_EXE): $(MAP_VLAN_OBJ_MAIN)
ifeq ($(FLAVOUR),luna)
	$(CC) $(MAP_VLAN_OBJ_MAIN) $(LDFLAGS) -o $@
else
	$(CC) $(MAP_VLAN_OBJ_MAIN) -o $@
endif

$(MAP_CHECKER_EXE): $(MAP_CHECKER_OBJ_MAIN)
ifeq ($(FLAVOUR),luna)
	$(CC) $(CCFLAGS) $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) -L$(OUTPUT_FOLDER) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LIB_MAPVENDORDATA_LDFLAGS) $^ -o $@
else
	$(CC) $(CCFLAGS) $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LIB_MAPVENDORDATA_LDFLAGS) $^ -o $@
endif

$(DPP_BOOTSTRAP_GEN): $(DPP_BOOTSTRAP_GEN_OBJ_MAIN)
ifeq ($(FLAVOUR),luna)
	$(CC) $^ $(CRYPTO_LDFLAGS) -o $@
else
	$(CC) $(CCFLAGS) $(CRYPTO_LDFLAGS) $^ -o $@
endif

$(BUILD_FOLDER)/%.o : src/%.c clean
	mkdir -p $(BUILD_FOLDER)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(EXTERNAL_INC)) $< -o $@

.PHONY: clean
clean:
	rm -f $(BUILD_FOLDER)/*.o
	rm -f $(MAP_REINIT_EXE)
	rm -f $(MAP_RESET_EXE)
	rm -f $(MAP_VLAN_EXE)
	rm -f $(MAP_CHECKER_EXE)
	rm -f $(DPP_BOOTSTRAP_GEN)

.PHONY: romfs
romfs:
	$(ROMFSINST) $(MAP_REINIT_EXE) /bin/map_reinit
	$(ROMFSINST) $(MAP_RESET_EXE) /bin/map_reset
	$(ROMFSINST) $(MAP_VLAN_EXE) /bin/map_vlan_config
	$(ROMFSINST) $(MAP_CHECKER_EXE) /bin/map_checker
	$(ROMFSINST) $(DPP_BOOTSTRAP_GEN) /bin/dpp_bootstrap_gen
