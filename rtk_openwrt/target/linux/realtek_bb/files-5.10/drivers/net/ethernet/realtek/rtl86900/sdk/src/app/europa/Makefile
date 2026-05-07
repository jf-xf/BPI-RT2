#
# Copyright (C) 2011 Realtek Semiconductor Corp.
#
# Makefile for SDK EUROPA parameter Source
#

APOLLO_SDK_PATH = $(SDKDIR)

# -------------------------------------------------------------------
PWD=$(shell pwd)
DRV_DIR=drv

EUROPA_DRV=europa_drv

# EUROPA_DRIVER_VERSION 0x00031C /* 0.3.28 */
MODULE_VERSION = 0x00031C 

#VERSION=$(shell printf "%d" $(MODULE_VERSION))
VERSION_MAJOR = $(shell echo "" | awk -F: '{ print and(rshift($(MODULE_VERSION),16),0xff); }')
VERSION_MINOR = $(shell echo "" | awk -F: '{ print and(rshift($(MODULE_VERSION),8),0xff); }')
VERSION_PATCH = $(shell echo "" | awk -F: '{ print and(rshift($(MODULE_VERSION),0),0xff); }')

RELEASE_VERSION=_$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
# -------------------------------------------------------------------


include $(APOLLODIR)/Compiler_Flag

LIB += ../lib/libpr.a
LIB += -L$(APOLLODIR)/object/src/app/lib -lrtk -lpthread -lrt -lm

#ASIC mode
CFLAGS := -DRTK_X86_CLE -DRTK_X86_ASICDRV -DCONFIG_APOLLO_CMD -DCONFIG_LINUX_USER_SHELL -D'EUROPA_VERSION="$(RELEASE_VERSION)"'


SDK_ROOT_DIR = $(APOLLO_SDK_PATH)
SDK_INCLUDE   = $(SDK_ROOT_DIR)/include
SYS_DIR = $(SDK_ROOT_DIR)/system/linux
SYS_INCLUDE = $(SDK_ROOT_DIR)/system/include
RTUSR_INC = $(SYS_DIR)/rtk/rtusr/include
COMMON_DIR = $(SDK_ROOT_DIR)/src/common

EUROPA_DIR := .
EUROPA_INC_DIR  = $(EUROPA_DIR)/inc
EUROPA_SRC_DIR := $(EUROPA_DIR)/src
EUROPA_USER_INC_DIR := $(EUROPA_INC_DIR)/user
EUROPA_USER_SRC_DIR := $(EUROPA_SRC_DIR)/user

ifeq ($(CONFIG_KERNEL_2_6_30),y)
  ifeq ($(CONFIG_KALLSYMS),y)
    EUROPA_DRV_DIR= drv_2_6_30
  else
    EUROPA_DRV_DIR= drv_2_6_30_no_kallsyms
  endif
endif

ifeq ($(CONFIG_KERNEL_3_18_x),y)
	ifeq ($(CONFIG_SMP),y)
		EUROPA_DRV_DIR= $(shell echo $(CONFIG_RSDK_DIR) | awk -F/ '{print $$NF}' | awk -F- '{print "drv_3_18-"$$3"-"$$6}')
	else
		EUROPA_DRV_DIR= $(shell echo $(CONFIG_RSDK_DIR) | awk -F/ '{print $$NF}' | awk -F- '{print "drv_3_18-"$$3"-"$$6"-no-smp"}')
	endif
endif

ifeq ($(CONFIG_KERNEL_4_4_x),y)
	ifeq ($(CONFIG_COMMON_RT_API),y)
		EUROPA_DRV_DIR= $(shell echo $(CONFIG_RSDK_DIR) | awk -F/ '{print $$NF}' | awk -F- '{print "drv_4_4-"$$3"-"$$6"-rt"}')
	else
		EUROPA_DRV_DIR= $(shell echo $(CONFIG_RSDK_DIR) | awk -F/ '{print $$NF}' | awk -F- '{print "drv_4_4-"$$3"-"$$6}')
	endif
endif

ifeq ($(CONFIG_KERNEL_5_10_x),y)
    ifeq ($(CONFIG_COMMON_RT_API),y)
        EUROPA_DRV_DIR= $(shell echo $(CONFIG_RSDK_DIR) | awk -F/ '{print $$NF}' | awk -F- '{print "drv_5_10-"$$3"-"$$6"-rt"}')
    else
        EUROPA_DRV_DIR= $(shell echo $(CONFIG_RSDK_DIR) | awk -F/ '{print $$NF}' | awk -F- '{print "drv_5_10-"$$3"-"$$6}')
    endif
endif

INCLUDE += -I$(EUROPA_INC_DIR) \
           -I$(EUROPA_USER_INC_DIR) \
           -I$(SDK_INCLUDE) \
           -I$(SYS_INCLUDE) \
           -I$(SDK_INCLUDE)/hal/chipdef/apollo \
           -I$(SDKDIR)/src/app/europa/inc

ifeq ($(CONFIG_OE_MODULE_I2C_PORT_0),y)
        EXTRA_CFLAGS += -DEUROPA_I2C_PORT=0
else
        EXTRA_CFLAGS += -DEUROPA_I2C_PORT=1
endif

#EXEC = europad
EXECLI = europacli
BIN = bin

EUROPACORE_INC := $(shell ls $(EUROPA_INC_DIR)/*.h)

EUROPACLI := \
       $(EUROPA_SRC_DIR)/europa_cli.o
       
$(EXECLI): $(EUROPACLI)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) $(EUROPACLI) $(LIB) -o $(BIN)/$@
	$(STRIP) $(BIN)/$@

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

# Force to rebuild all every time
all: clean $(EXECLI) Driver
	##$(MAKE) -C $(DRV_DIR) all MODULE_VERSION=$(MODULE_VERSION)
	##cp -af drv/$(EUROPA_DRV).ko drv/$(EUROPA_DRV).ko$(RELEASE_VERSION)

Driver:	
	if [ ! -d $(EUROPA_DRV_DIR) ];  then \
		mkdir $(EUROPA_DRV_DIR) ;\
	fi

	if [ -f $(PWD)/$(DRV_DIR)/Makefile ];  then \
		@echo "Building europa driver..."; \
		$(MAKE) -C $(DRV_DIR) all MODULE_VERSION=$(MODULE_VERSION); \
		cp -af $(DRV_DIR)/$(EUROPA_DRV).ko $(DRV_DIR)/$(EUROPA_DRV).ko$(RELEASE_VERSION); \
		cp -af $(DRV_DIR)/$(EUROPA_DRV).ko$(RELEASE_VERSION) $(EUROPA_DRV_DIR)/$(EUROPA_DRV).ko$(RELEASE_VERSION); \
		cp -af $(DRV_DIR)/insert_europa.sh $(EUROPA_DRV_DIR)/ ; \
	fi


DriverClean:
	if [ -f $(PWD)/$(DRV_DIR)/Makefile ];  then \
	   @echo "Cleaning europa driver..."; \
	   $(MAKE) -C $(DRV_DIR) clean RELEASE_VERSION=$(RELEASE_VERSION); \
	fi 
	
install: $(EXECLI)
	cp $(BIN)/$(EXECLI) $(BINDIR)
	cp -af $(DRV_DIR)/$(EUROPA_DRV).ko$(RELEASE_VERSION) $(ROMFSDIR)/lib/modules/$(EUROPA_DRV).ko
	cp -af $(DRV_DIR)/insert_europa.sh $(ROMFSDIR)/etc/scripts/
	chmod 755 $(ROMFSDIR)/etc/scripts/insert_europa.sh
	# board parameters
	@echo "Building europa script..."; 

ifeq ("$(CONFIG_TX_POWER_TURN_ON_FEATURE)","")
	CONFIG_TX_POWER_GPO_PIN=255
endif

ifeq ($(CONFIG_OE_MODULE_I2C_PORT_0),y)
	$(ROMFSINST) -a "I2C_PORT=0 INTR_PIN=$(CONFIG_EUROPA_INTERRUPT_GPIO_PIN) TXDIS_PIN=$(CONFIG_TX_DISABLE_GPIO_PIN) TXPWR_PIN=$(CONFIG_TX_POWER_GPO_PIN)" /etc/scripts/insert_europa.sh
else
	$(ROMFSINST) -a "I2C_PORT=1 INTR_PIN=$(CONFIG_EUROPA_INTERRUPT_GPIO_PIN) TXDIS_PIN=$(CONFIG_TX_DISABLE_GPIO_PIN) TXPWR_PIN=$(CONFIG_TX_POWER_GPO_PIN)" /etc/scripts/insert_europa.sh
endif


clean: DriverClean
	rm -f $(EXECLI) *.gdb *.elf *.elf2flt
