#
# Copyright (C) 2011 Realtek Semiconductor Corp.
#
# Makefile for SDK EPON OAM protocol Source
#

APOLLO_SDK_PATH = $(SDKDIR)

USE_OAM_BINARY = n

ifeq ($(CONFIG_RTL9601B_SERIES), y)
DIR = 9601B_series
USE_OAM_BINARY = y
else
ifeq ($(CONFIG_RTL9600_SERIES), y)
DIR = 9600_series
USE_OAM_BINARY = y
else
ifeq ($(CONFIG_RTL9602C_SERIES), y)
DIR = 9602C_series
USE_OAM_BINARY = y
endif
endif
endif

ifeq ($(CONFIG_RTL9607C_SERIES), y)
DIR = 9607C_series
USE_OAM_BINARY = y
endif

ifeq ($(CONFIG_LUNA_G3_SERIES), y)
G3DIR = g3_series
ifeq ($(CONFIG_ARM64), y)
DIR = $(G3DIR)/arm64
else
DIR = $(G3DIR)/arm32
endif
USE_OAM_BINARY = y
endif

ifeq ($(USE_OAM_BINARY), y)
ifeq ($(CONFIG_HWNAT_FLEETCONNTRACK),y)
VERSION=1.2.3
else
VERSION=1.1.2
endif
else
VERSION=1.2.1
endif

include $(APOLLODIR)/Compiler_Flag


LIB += -L$(APOLLODIR)/object/src/app/lib -lrtk -lpthread -lrt -lpr

#ASIC mode
CFLAGS := -DRTK_X86_CLE -DRTK_X86_ASICDRV -DCONFIG_APOLLO_CMD -DCONFIG_SMIDA1011 -DDISABLE_VLAN_SHADOW -DCONFIG_LINUX_USER_SHELL -D'EPON_OAM_VERSION="$(VERSION)"'


SDK_ROOT_DIR = $(APOLLO_SDK_PATH)
SDK_INCLUDE   = $(SDK_ROOT_DIR)/include
SYS_DIR = $(SDK_ROOT_DIR)/system/linux
SYS_INCLUDE = $(SDK_ROOT_DIR)/system/include
RTUSR_INC = $(SYS_DIR)/rtk/rtusr/include
COMMON_DIR = $(SDK_ROOT_DIR)/src/common

EPON_OAM_DIR := .
EPON_OAM_INC_DIR  = $(EPON_OAM_DIR)/inc
EPON_OAM_SRC_DIR := $(EPON_OAM_DIR)/src
USER_OAM_INC_DIR := $(EPON_OAM_INC_DIR)/user
USER_OAM_SRC_DIR := $(EPON_OAM_SRC_DIR)/user

INCLUDE += -I$(EPON_OAM_INC_DIR) \
           -I$(USER_OAM_INC_DIR) \
           -I$(SDK_INCLUDE) \
           -I$(SYS_INCLUDE) \
           -I$(SDK_INCLUDE)/hal/chipdef/apollo \
           -I$(SDKDIR)/src/app/pkt_redirect/inc

CFLAGS += -DCONFIG_RTL865X \
          -DCONFIG_RTL865XC \
          -DRTL865X_MODEL_USER \

ifeq ($(CONFIG_RTK_L34_ENABLE), y)
EPON_DEVICE_TYPE = hgu
else
ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
EPON_DEVICE_TYPE = fc/hgu
else
EPON_DEVICE_TYPE = sfu
endif
endif

EXEC = eponoamd
EXECLI = oamcli
BIN = bin

OAMCORE := \
       $(EPON_OAM_SRC_DIR)/epon_oam_main.o \
       $(EPON_OAM_SRC_DIR)/epon_oam_db.o \
       $(EPON_OAM_SRC_DIR)/epon_oam_rx.o \
       $(EPON_OAM_SRC_DIR)/epon_oam_tx.o \
       $(EPON_OAM_SRC_DIR)/epon_oam_state.o \
       $(EPON_OAM_SRC_DIR)/epon_oam_dbg.o

OAMCORE_INC := $(shell ls $(EPON_OAM_INC_DIR)/*.h)

OAMCLI := \
       $(EPON_OAM_SRC_DIR)/epon_oam_cli.o \

USEROAM := \
       $(USER_OAM_SRC_DIR)/ctc_oam.o \
       $(USER_OAM_SRC_DIR)/ctc_oam_var.o

USEROAM_INC := $(shell ls $(USER_OAM_INC_DIR)/*.h)

$(EXEC): $(OAMCORE) $(OAMCORE_INC) $(USEROAM) $(USEROAM_INC)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) $(OAMCORE) $(USEROAM) $(LIB) -o $(BIN)/$@
	$(STRIP) $(BIN)/$@

$(EXECLI): $(OAMCLI) $(OAMCORE_INC)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) $(OAMCLI) $(LIB) -o $(BIN)/$@
	$(STRIP) $(BIN)/$@

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

# Force to rebuild all every time
ifeq ($(USE_OAM_BINARY), y)
all:
	cp $(EPON_DEVICE_TYPE)/$(DIR)/$(EXEC).$(VERSION) $(BIN)/$(EXEC)
	cp $(EPON_DEVICE_TYPE)/$(DIR)/$(EXECLI).$(VERSION) $(BIN)/$(EXECLI)

install: all
	chmod 775 $(BIN)/$(EXEC)
	chmod 775 $(BIN)/$(EXECLI)
	cp -af $(BIN)/$(EXEC) $(BINDIR)
	cp -af $(BIN)/$(EXECLI) $(BINDIR)
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
#Moving to ca_rtk_epon_drv
else
ifeq ($(CONFIG_EPON_FEATURE),y)
	$(MAKE) -C $(SDK_ROOT_DIR)/src/module/epon_drv install
	$(MAKE) -C $(SDK_ROOT_DIR)/src/module/epon_mpcp install
	$(MAKE) -C $(SDK_ROOT_DIR)/src/module/epon_polling install
endif
endif
else
all: clean $(EXEC) $(EXECLI)

install: $(EXEC) $(EXECLI)
	cp $(BIN)/$(EXEC) $(BINDIR)
	cp $(BIN)/$(EXECLI) $(BINDIR)
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
#Moving to ca_rtk_epon_drv
else
ifeq ($(CONFIG_EPON_FEATURE),y)
	$(MAKE) -C $(SDK_ROOT_DIR)/src/module/epon_drv install
	$(MAKE) -C $(SDK_ROOT_DIR)/src/module/epon_mpcp install
	$(MAKE) -C $(SDK_ROOT_DIR)/src/module/epon_polling install
endif
endif
endif

clean:
	rm -f $(OAMCORE) $(USEROAM) $(BIN)/$(EXEC) $(BIN)/$(EXECLI) *.gdb *.elf *.elf2flt
