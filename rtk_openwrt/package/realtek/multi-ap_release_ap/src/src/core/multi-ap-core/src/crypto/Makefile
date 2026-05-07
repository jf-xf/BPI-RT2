SRC           := $(wildcard src_$(PLATFORM)/*.c)
OBJ           := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/crypto/%.o,$(SRC))

INTERNAL_INC  := interfaces

HDR           := $(shell find $(INTERNAL_INC) -name *.h)

################################################################################
# Targets
################################################################################

.PHONY: all
all: $(OBJ)
	$(CC) -shared $^ -o $(CRYPTO_LIB_SHARED)
	$(AR) -rcs $(CRYPTO_LIB) $^

$(OBJ) : $(OUTPUT_FOLDER)/tmp/crypto/%.o : %.c $(HDR)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/crypto/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC)) $< -o $@

.PHONY: unit_tests
unit_tests:
	$(MAKE) -C unit_tests all

.PHONY: clean
clean:
	rm -rf $(CRYPTO_LIB_SHARED)
	rm -rf $(CRYPTO_LIB)
	rm -rf $(OUTPUT_FOLDER)/tmp/crypto
	$(MAKE) -C unit_tests clean

