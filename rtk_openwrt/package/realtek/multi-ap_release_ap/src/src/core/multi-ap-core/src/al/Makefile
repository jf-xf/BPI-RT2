# Broadband Forum IEEE 1905.1/1a stack
#
# Copyright (c) 2017, Broadband Forum
# Copyright (c) 2018, Realtek Semiconductor Corp.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# Subject to the terms and conditions of this license, each copyright
# holder and contributor hereby grants to those receiving rights under
# this license a perpetual, worldwide, non-exclusive, no-charge,
# royalty-free, irrevocable (except for failure to satisfy the
# conditions of this license) patent license to make, have made, use,
# offer to sell, sell, import, and otherwise transfer this software,
# where such license applies only to those patent claims, already
# acquired or hereafter acquired, licensable by such copyright holder or
# contributor that are necessarily infringed by:
#
# (a) their Contribution(s) (the licensed copyrights of copyright holders
#     and non-copyrightable additions of contributors, in source or binary
#     form) alone; or
#
# (b) combination of their Contribution(s) with the work of authorship to
#     which such Contribution(s) was added by such copyright holder or
#     contributor, if, at the time the Contribution is added, such addition
#     causes such combination to be necessarily infringed. The patent
#     license shall not apply to any other combinations which include the
#     Contribution.
#
# Except as expressly stated above, no rights or licenses from any
# copyright holder or contributor is granted under this license, whether
# expressly, by implication, estoppel or otherwise.
#
# DISCLAIMER
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.


# When calling this Makefile, the following environment variables must be set:
#
#   CC ------------> Path to the compiler
#   CCFLAGS -------> Extra flags to use while compiling
#   LDGLAGS -------> Extra flags to use while linking
#
#   PLATFORM      -> Possible values: "linux"
#   OUTPUT_FOLDER -> Absolute path to the folder where binaries will be built
#
#   COMMON_LIB ----> Absolute path to the "common library" *.a file
#   COMMON_INC ----> Absolute path to the folder containing common (to all
#                    sub-projects) header files
#
#   FACTORY_LIB   -> Absolute path to the "factory library" *.a file
#   FACTORY_INC   -> Absolute path to the folder containing the "factory
#                    library" header files
#
#   MKDIR ---------> Tool to create a directory
#

ifeq ($(FLAVOUR),jungle)
FLAVOUR_DIR                := jungle
else
FLAVOUR_DIR                := luna
endif

LIB                        := $(AL_LIB)
SHARED_LIB                 := $(AL_LIB_SHARED)

CORE_OBJECT                := $(OUTPUT_FOLDER)/tmp/multiap_core.o

AL_DIRECTORY               := al

SRC_INDEPENDENT            := $(wildcard src_independent/*.c)
SRC_INDEPENDENT            += $(wildcard src_independent/extensions/*/*.c)
SRC_PLATFORM               := $(wildcard src_$(PLATFORM)/*.c)

ifeq ($(USE_HAPD),y)
SRC_PLATFORM               := $(filter-out src_$(PLATFORM)/platform_interfaces_rtk_general.c, $(SRC_PLATFORM))
else
SRC_PLATFORM               := $(filter-out src_$(PLATFORM)/platform_interfaces_rtk_hapd.c, $(SRC_PLATFORM))
endif
SRC_PLATFORM_FLAVOUR       := $(wildcard src_$(PLATFORM)/$(FLAVOUR_DIR)/*.c)

OBJ_INDEPENDENT            := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o,$(SRC_INDEPENDENT))
OBJ_PLATFORM               := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o,$(SRC_PLATFORM) $(SRC_PLATFORM_FLAVOUR))

OBJ_LIB_COMMON             := $(wildcard $(OUTPUT_FOLDER)/tmp/common/*/*.o)
OBJ_LIB_FACTORY            := $(wildcard $(OUTPUT_FOLDER)/tmp/factory/*/*.o)

INTERNAL_INC_INDEPENDENT   := src_independent internal_interfaces
INTERNAL_INC_INDEPENDENT   += $(sort $(dir $(wildcard interfaces/extensions/*/))) $(sort $(dir $(wildcard  src_independent/extensions/*/)))

INTERNAL_INC_PLATFORM      := src_$(PLATFORM) src_independent internal_interfaces src_$(PLATFORM)/$(FLAVOUR_DIR)

EXTERNAL_INC               := $(COMMON_INC) $(CRYPTO_INC) $(DPP_INC) $(FACTORY_INC) $(ADDON_INC) $(MIB_INC) $(MULTI_AP_DIR)/customized

HDR_INDEPENDENT            := $(shell find $(INTERNAL_INC_INDEPENDENT) $(EXTERNAL_INC) -name *.h)
HDR_PLATFORM               := $(shell find $(INTERNAL_INC_PLATFORM)    $(EXTERNAL_INC) -name *.h)

LDFLAGS += $(ISP_LDFLAGS)

################################################################################
# Targets
################################################################################

.PHONY: all
all: lib

lib: $(SHARED_LIB) $(LIB)

$(LIB): $(CORE_OBJECT)
	$(AR) -rcs $@ $^

$(CORE_OBJECT): $(OBJ_INDEPENDENT) $(OBJ_PLATFORM) $(OBJ_LIB_FACTORY) $(OBJ_LIB_COMMON)
	$(LD) -Ur $^ -o $@

$(SHARED_LIB): $(OBJ_INDEPENDENT) $(OBJ_PLATFORM) $(OBJ_LIB_FACTORY) $(OBJ_LIB_COMMON)
	$(CC) -shared $^ -o $@

$(OBJ_INDEPENDENT) : $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o : %.c
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_INDEPENDENT)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC_INDEPENDENT) $(EXTERNAL_INC)) $< -o $@

$(OBJ_PLATFORM)    : $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o : %.c
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_PLATFORM)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/$(directory);)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_PLATFORM_FLAVOUR)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC_PLATFORM) $(EXTERNAL_INC)) $< -o $@

# .PHONY: unit_tests
# unit_tests: $(LIB)
# 	$(MAKE) -C api/unit_tests all

.PHONY: clean
clean:
	rm -rf $(EXE) $(LIB) $(SHARED_LIB)
	rm -rf $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)
	# $(MAKE) -C api     clean
	# $(MAKE) -C api/unit_tests     clean
