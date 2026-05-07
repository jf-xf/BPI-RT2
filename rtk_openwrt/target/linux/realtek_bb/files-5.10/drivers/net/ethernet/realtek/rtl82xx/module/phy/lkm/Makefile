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
    MODULE_NAME         := phy
    MODULE_DIR          := module/$(MODULE_NAME)/lkm
    MODULE_INC_PATH     := $(ROOT_PATH)/module

  #******************************************************
  # FLAG
  #******************************************************
    EXTRA_CFLAGS        += -I$(MODULE_INC_PATH) -I$(ROOT_PATH)/module/$(MODULE_NAME)/inc -I$(SDK_PATH)/include -I$(SDK_SYSTEM_PATH)/include
    EXTRA_CFLAGS        += -DPHY_ONLY
    EXTRA_LDFLAGS       +=

  #******************************************************
  # object files
  #    	MODULE_OBJ: all obj with full path name
  #******************************************************
    SDK_BUILD_MODE      := linux-kernel
    SDK_CONFIG_FILE     := $(ROOT_PATH)/sdk/config/.config
    include             $(SDK_CONFIG_FILE)
    include             $(ROOT_PATH)/sdk/build/Makefile.phy
    include             $(ROOT_PATH)/module/$(MODULE_NAME)/src/Makefile

    MODULE_OBJ          := $(addprefix $(ROOT_PATH)/sdk/src/,$(PHY_OBJ))
    MODULE_OBJ          += $(PHY_EXTRA)
    MODULE_OBJ          += $(ROOT_PATH)/$(MODULE_DIR)/$(MODULE_NAME)_lkm.o

#-----------------------------------------------------------------------------
# Build Linux Kernel Module
#-----------------------------------------------------------------------------
    ifeq ($(ROOT_PATH),)
        ROOT_PATH       := $(patsubst %/$(MODULE_DIR),%,$(shell pwd))
    endif
    include             $(ROOT_PATH)/make/Makefile.lkm


