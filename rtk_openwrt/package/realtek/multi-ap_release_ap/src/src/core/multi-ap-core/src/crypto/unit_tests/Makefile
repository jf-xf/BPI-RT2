# Unit Tests for Platform AL Components
#
# Copyright (c) 2021, Realtek Semiconductor Corp.
#
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
#   MKDIR ---------> Tool to create a directory
#

UNITS := crypto_basics
UNITS += crypto_aes
UNITS += crypto_ecc

OUTPUT_TMP_UNIT_TESTS_DIR := $(OUTPUT_FOLDER)/tmp/crypto/unit_tests

SRC_PLATFORM_CRYPTO       := $(wildcard ../src_$(PLATFORM)/*.c)
OBJ_PLATFORM_CRYPTO       := $(patsubst ../%.c, $(OUTPUT_TMP_UNIT_TESTS_DIR)/%.o, $(SRC_PLATFORM_CRYPTO))
INC_PLATFORM_CRYPTO       := $(COMMON_INC) ../interfaces

EXE                       := $(addprefix $(OUTPUT_FOLDER)/UNITTEST_, $(UNITS))
SRC                       := $(addsuffix .c, $(UNITS))
OBJ                       := $(addprefix $(OUTPUT_TMP_UNIT_TESTS_DIR)/, $(addsuffix .o, $(UNITS)))

TESTS                     := $(addprefix UNITTEST_,$(basename $(UNITS)))

################################################################################
# Targets
################################################################################

.PHONY: all
all: $(TESTS)

.PHONY: $(TESTS)
$(TESTS) : UNITTEST_% : $(OUTPUT_FOLDER)/UNITTEST_%
	$<

$(EXE) : $(OUTPUT_FOLDER)/UNITTEST_% : $(OUTPUT_TMP_UNIT_TESTS_DIR)/%.o $(OBJ_PLATFORM_CRYPTO) $(COMMON_LIB)
	$(MKDIR) $(dir $@)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ) : $(OUTPUT_TMP_UNIT_TESTS_DIR)/%.o : %.c
	$(MKDIR) $(dir $@)
	$(CC)  $(CCFLAGS) -c $(addprefix -I, $(INC_PLATFORM_CRYPTO)) $< -o $@

$(OBJ_PLATFORM_CRYPTO) : $(OUTPUT_TMP_UNIT_TESTS_DIR)/%.o : ../%.c
	$(MKDIR) $(dir $@)
	$(CC)  $(CCFLAGS) -c $(addprefix -I, $(INC_PLATFORM_CRYPTO)) $< -o $@

.PHONY: clean
clean:
	rm -f $(OUTPUT_TMP_UNIT_TESTS_DIR)
	rm -f $(EXE)
