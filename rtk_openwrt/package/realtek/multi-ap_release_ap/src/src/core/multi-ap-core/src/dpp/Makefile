# Multi-AP Device Provisioning Protocol
# Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

SRC           := $(wildcard src_$(PLATFORM)/*.c)
OBJ           := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/dpp/%.o,$(SRC))

INTERNAL_INC  := interfaces
EXTERNAL_INC  := $(ADDON_INC) $(COMMON_INC) $(CRYPTO_INC)

HDR           := $(shell find $(INTERNAL_INC) $(EXTERNAL_INC) -name *.h)

################################################################################
# Targets
################################################################################

.PHONY: all
all: $(OBJ)
	$(CC) -shared $^ -o $(DPP_LIB_SHARED)
	$(AR) -rcs $(DPP_LIB) $^

$(OBJ) : $(OUTPUT_FOLDER)/tmp/dpp/%.o : %.c $(HDR)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/dpp/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC) $(EXTERNAL_INC)) $< -o $@

.PHONY: clean
clean:
	rm -rf $(DPP_LIB_SHARED)
	rm -rf $(DPP_LIB)
	rm -rf $(OUTPUT_FOLDER)/tmp/dpp
