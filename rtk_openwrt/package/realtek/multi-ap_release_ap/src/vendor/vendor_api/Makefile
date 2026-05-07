CCFLAGS       := -g -fno-omit-frame-pointer
CCFLAGS       += -Wall -Wextra -Werror
CCFLAGS       += -I$(MULTI_AP_DIR)/include/
CCFLAGS       += -I$(shell pwd)/
CCFLAGS       += -I$(ROOTDIR)/config/

LDFLAGS       += -L$(BINARY_DIR)/lib/ -L$(ROOTDIR)/lib/libssl/
LDFLAGS       += -L$(shell pwd)/../customized -lmapVendorData
LDFLAGS       += -lmultiap -lcrypto -lrt -lpthread -fPIC

export CCFLAGS LDFLAGS

################################################################################
# Targets
################################################################################

# Select one method from below

# METHOD_DIR := append_tlv_method
ifdef USE_RTIC_CONTROL
METHOD_DIR := standalone_daemon_method
else
ifdef USE_RTK_RNR
METHOD_DIR := standalone_daemon_method
else
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
