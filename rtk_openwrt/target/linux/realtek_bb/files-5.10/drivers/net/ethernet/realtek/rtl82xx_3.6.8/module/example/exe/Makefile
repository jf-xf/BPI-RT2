# Copyright (C) 2016 Realtek Semiconductor Corp.
#
# Makefile for Application
#

#-----------------------------------------------------------------------------
# Customization per module
#-----------------------------------------------------------------------------
  include             $(ROOT_PATH)/make/Makefile.settings

  #******************************************************
  # module
  #******************************************************
    BUILD_OUTPUT_DIR    := build
    MODULE_NAME         := example
    MODULE_DIR          := module/$(MODULE_NAME)/exe
    MODULE_SRC_PATH     := $(ROOT_PATH)/module/$(MODULE_NAME)/src
    MODULE_SHARED_LIB   := -L$(ROOT_PATH)/module/phy/a -lphy -lpthread

  #******************************************************
  # FLAG
  #******************************************************
    EXTRA_CFLAGS        += -I$(ROOT_PATH)/module -I$(SDK_PATH)/include -I$(SDK_SYSTEM_PATH)/include
    EXTRA_LDFLAGS       +=

  #******************************************************
  # object files
  #******************************************************
    include             $(MODULE_SRC_PATH)/Makefile
    MODULE_OBJ          := $(ROOT_PATH)/$(MODULE_DIR)/$(MODULE_NAME)_main.o
    MODULE_OBJ          += $(EXAMPLE_OBJ)
$(warning XXXXXXXXX=$(MODULE_OBJ))
    MODULE_SRC          += $(subst .o,.c,$(MODULE_OBJ))
    #ALL_SRC_PATH        := $(sort $(dir $(MODULE_SRC)))
    MODULE_OBJ          := $(addprefix $(BUILD_OUTPUT_DIR)/,$(subst $(ROOT_PATH)/,,$(MODULE_OBJ)))

    #vpath %.c $(ALL_SRC_PATH)

#----------------------------------------------------------------------------
# Build Application
#-----------------------------------------------------------------------------
    executable          := yes
    ifeq ($(ROOT_PATH),)
        ROOT_PATH       := $(patsubst %/$(MODULE_DIR),%,$(shell pwd))
    endif
    include             $(ROOT_PATH)/make/Makefile.lib
