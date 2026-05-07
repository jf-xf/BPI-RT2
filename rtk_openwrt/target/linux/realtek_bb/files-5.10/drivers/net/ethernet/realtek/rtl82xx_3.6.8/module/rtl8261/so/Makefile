#
# Copyright (C) 2017 Realtek Semiconductor Corp.
#
# Makefile for shared lib
#

#-----------------------------------------------------------------------------
# Customization per module
#-----------------------------------------------------------------------------
  include             $(ROOT_PATH)/make/Makefile.settings

  #******************************************************
  # module
  #******************************************************
    BUILD_OUTPUT_DIR    := build
    MODULE_NAME         := phy
    MODULE_DIR          := module/$(MODULE_NAME)/so
    MODULE_INC_PATH     := $(ROOT_PATH)/module

  #******************************************************
  # FLAG
  #******************************************************
    EXTRA_CFLAGS        += -I$(MODULE_INC_PATH) -I$(ROOT_PATH)/module/$(MODULE_NAME)/inc -I$(SDK_PATH)/include -I$(SDK_SYSTEM_PATH)/include
    EXTRA_CFLAGS        += -DPHY_ONLY
    EXTRA_LDFLAGS       +=

  #******************************************************
  # object files
  #******************************************************
    SDK_BUILD_MODE      := linux-user
    SDK_CONFIG_FILE     := $(ROOT_PATH)/sdk/config/.config
    sinclude            $(SDK_CONFIG_FILE)
    include             $(ROOT_PATH)/sdk/build/Makefile.phy
    include             $(ROOT_PATH)/module/$(MODULE_NAME)/src/Makefile

    MODULE_SRC          := $(addprefix $(ROOT_PATH)/sdk/src/,$(subst .o,.c,$(PHY_OBJ)))
    MODULE_SRC          += $(subst .o,.c,$(PHY_EXTRA))
    ALL_SRC_PATH        := $(sort $(dir $(MODULE_SRC)))
    MODULE_OBJ          := $(addprefix $(BUILD_OUTPUT_DIR)/,$(subst .c,.o,$(subst $(ROOT_PATH)/,,$(MODULE_SRC))))
    vpath %.c $(ALL_SRC_PATH)

#-----------------------------------------------------------------------------
# Build lib
#-----------------------------------------------------------------------------
    sharedlib           := yes
    ifeq ($(ROOT_PATH),)
        ROOT_PATH       := $(patsubst %/$(MODULE_DIR),%,$(shell pwd))
    endif
    include             $(ROOT_PATH)/make/Makefile.lib
