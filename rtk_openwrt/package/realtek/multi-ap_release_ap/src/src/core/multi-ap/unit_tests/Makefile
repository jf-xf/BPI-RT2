# Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

OUTPUT_DIR                 := $(OUTPUT_FOLDER)/api/unit_tests

EXE_PARSING  := $(OUTPUT_DIR)/multi_ap_api_queue_message_parsing
EXE_FORGING  := $(OUTPUT_DIR)/multi_ap_api_queue_message_forging

SHARED_SRC := multi_ap_api_queue_message_test_vectors.c
SHARED_OBJ := $(addprefix $(OUTPUT_FOLDER)/,$(patsubst %.c,%.o,$(SHARED_SRC)))
SHARED_HDR := multi_ap_api_queue_message_test_vectors.h

SRC_PARSING                   := multi_ap_api_queue_message_parsing.c
SRC_FORGING                   := multi_ap_api_queue_message_forging.c
OBJ_PARSING                   := $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(SRC_PARSING))
OBJ_FORGING                   := $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(SRC_FORGING))
################################################################################
# Targets
################################################################################

.PHONY: all
all: $(EXE_PARSING) $(EXE_FORGING)

$(EXE_PARSING): $(OBJ_PARSING) $(SHARED_OBJ)
	$(CC) $(OBJ_PARSING) $(SHARED_OBJ) -L$(OUTPUT_FOLDER) -lmultiap $(LDFLAGS) -o $@

$(OBJ_PARSING)    : $(OUTPUT_DIR)/%.o : %.c $(SHARED_HDR)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

$(EXE_FORGING): $(OBJ_FORGING) $(SHARED_OBJ)
	$(CC) $(OBJ_FORGING) $(SHARED_OBJ) -L$(OUTPUT_FOLDER) -lmultiap $(LDFLAGS) -o $@

$(OBJ_FORGING)    : $(OUTPUT_DIR)/%.o : %.c $(SHARED_HDR)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

$(SHARED_OBJ) : $(OUTPUT_FOLDER)/%.o : %.c $(SHARED_HDR)
	$(foreach directory, $(sort $(dir $(wildcard $(SHARED_SRC)))), $(MKDIR) $(OUTPUT_FOLDER)/$(directory);)
	$(CC)  $(CCFLAGS) -c $< -o $@;

.PHONY: clean
clean:
	rm -f $(EXE)
	rm -rf $(OUTPUT_FOLDER)/UNITTEST_*
	rm -rf $(OUTPUT_FOLDER)/tmp/api
	rm -rf $(OUTPUT_FOLDER)/tmp/$(UNIT_TESTS_DIRECTORY)
