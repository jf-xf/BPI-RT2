CC        := $(CROSS_COMPILE)gcc
CXX       := $(CROSS_COMPILE)g++
LD        := $(CROSS_COMPILE)ld
AR        := $(CROSS_COMPILE)ar
STRIP     := $(CROSS_COMPILE)strip

export

DIRECTORY := $(sort $(dir $(wildcard */)))

.PHONY: all
all:
	$(info $(DIRECTORY))
	$(foreach dir, $(DIRECTORY), $(MAKE) -C $(dir) || exit;)

.PHONY: romfs
romfs:
	$(info $(DIRECTORY))
	$(foreach dir, $(DIRECTORY), $(MAKE) -C $(dir) $@;)

.PHONY: clean
clean:
	$(info $(DIRECTORY))
	$(foreach dir, $(DIRECTORY), $(MAKE) -C $(dir) $@;)