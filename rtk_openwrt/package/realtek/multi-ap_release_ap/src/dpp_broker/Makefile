# Copyright (C) 2022 Realtek Semiconductor Corp. - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

OUTPUT_FOLDER              := output

DPP_BROKER_SRC                 := $(wildcard *.c)
DPP_BROKER_OBJ                 := $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(DPP_BROKER_SRC))

CFLAGS := -g -Wall -Werror

################################################################################
# Targets
################################################################################

.PHONY: all

all:
	$(MAKE) clean
	$(MAKE) binary

.PHONY: binary
binary: $(DPP_BROKER_OBJ)
	$(CC) $(DPP_BROKER_OBJ) $(CFLAGS) -o dpp_broker

$(DPP_BROKER_OBJ): $(OUTPUT_FOLDER)/%.o: %.c
	mkdir -p $(OUTPUT_FOLDER)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf dpp_broker
	rm -rf $(OUTPUT_FOLDER)/*.o

.PHONY: romfs
romfs:
	$(ROMFSINST) dpp_broker /bin/dpp_broker
