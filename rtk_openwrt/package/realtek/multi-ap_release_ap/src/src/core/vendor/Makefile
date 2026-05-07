CCFLAGS       := -g -fno-omit-frame-pointer
CCFLAGS       += -Wall -Wextra -Werror
CCFLAGS       += -I$(MULTI_AP_DIR)/include/
CCFLAGS       += -I$(shell pwd)/

ifeq ($(FLAVOUR), luna)
CCFLAGS       += -I$(ROOTDIR)/config/
else ifeq ($(FLAVOUR), jungle)
CCFLAGS       += -I$(DIR_ROOT)/config/
endif

LDFLAGS       += -L$(ROOTDIR)/lib/libssl/ -L$(OPENSSL_DIR)/

LDFLAGS       += -lmultiap -lcrypto -lrt -lpthread -fPIC

export CCFLAGS LDFLAGS

################################################################################
# Targets
################################################################################

# Select one method from below

# Dummy library for linking only
METHOD_DIR := empty
endif
endif

.PHONY: all
all: clean
	$(MAKE) -C $(METHOD_DIR)

.PHONY: clean
clean:
	$(MAKE) -C $(METHOD_DIR) clean

.PHONY: romfs
romfs:
	$(MAKE) -C $(METHOD_DIR) romfs
