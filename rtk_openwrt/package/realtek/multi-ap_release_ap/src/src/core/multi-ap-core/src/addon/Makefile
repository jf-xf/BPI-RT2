# Addon Libraries in Multi-AP
# Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

ADDON_DIR        := addon

SRC_INDEPENDENT  := $(wildcard src_independent/*.c)
SRC_PLATFORM     := $(wildcard src_$(PLATFORM)/*.c)

OBJ_INDEPENDENT  := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)/%.o,$(SRC_INDEPENDENT))
OBJ_PLATFORM     := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)/%.o,$(SRC_PLATFORM))

INTERNAL_INC     := interfaces
EXTERNAL_INC     :=

HDR              := $(shell find $(INTERNAL_INC) $(EXTERNAL_INC) -name *.h)

################################################################################
# Targets
################################################################################

.PHONY: all
all: $(OBJ_INDEPENDENT) $(OBJ_PLATFORM)
	$(AR) -rcs $(ADDON_LIB) $^

$(OBJ_INDEPENDENT) : $(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)/%.o : %.c $(HDR)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_INDEPENDENT)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $< -o $@

$(OBJ_PLATFORM) : $(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)/%.o : %.c $(HDR)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_PLATFORM)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $< -o $@

.PHONY: clean
clean:
	rm -rf $(ADDON_LIB)
	rm -rf $(OUTPUT_FOLDER)/tmp/$(ADDON_DIR)
