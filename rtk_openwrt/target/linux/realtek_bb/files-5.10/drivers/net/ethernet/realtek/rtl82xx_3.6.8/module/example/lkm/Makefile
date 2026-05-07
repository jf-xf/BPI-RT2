#
# Copyright (C) 2017 Realtek Semiconductor Corp.
#
# Makefile for Linux Kernel Module
#

#-----------------------------------------------------------------------------
# Customization per module
#-----------------------------------------------------------------------------
    include             $(ROOT_PATH)/make/Makefile.settings

  #******************************************************
  # module
  #******************************************************
    MODULE_NAME         := example
    MODULE_DIR          := module/$(MODULE_NAME)/lkm

  #******************************************************
  # FLAG
  #******************************************************
    EXTRA_CFLAGS        += -I$(ROOT_PATH)/module -I$(SDK_PATH)/include -I$(SDK_SYSTEM_PATH)/include
    EXTRA_LDFLAGS       +=
    KBUILD_EXTRA_SYMBOLS += $(ROOT_PATH)/module/phy/lkm/Module.symvers
    export KBUILD_EXTRA_SYMBOLS

  #******************************************************
  # object files
  #    	MODULE_OBJ: all obj with full path name
  #******************************************************
    include             $(ROOT_PATH)/module/example/src/Makefile
	MODULE_OBJ          := $(EXAMPLE_OBJ)
	MODULE_OBJ          += $(ROOT_PATH)/$(MODULE_DIR)/$(MODULE_NAME)_lkm.o

#-----------------------------------------------------------------------------
# Build Linux Kernel Module
#-----------------------------------------------------------------------------
    ifeq ($(ROOT_PATH),)
        ROOT_PATH       := $(patsubst %/$(MODULE_DIR),%,$(shell pwd))
    endif
    include             $(ROOT_PATH)/make/Makefile.lkm


