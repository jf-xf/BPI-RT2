LIB_MAPVENDORDATA_SO = libmapVendorData.so
LIB_MAPVENDORDATA_OBJ = map_vendor_specific_data.o
LIB_MAPVENDORDATA_OBJ += $(MULTI_AP_DIR)/lib/libmultiap_addon.a

ifneq ($(FLAVOUR),openwrt)
MIB_INC     := $(ROOTDIR)/user/boa/src/LINUX/
endif

EXTERNAL_INC  := $(MIB_INC) $(MULTI_AP_DIR)/include $(ROOTDIR)/include

CFLAGS       += -Wall -Wextra -Werror -O2 $(addprefix -I,$(EXTERNAL_INC))

ifneq ($(FLAVOUR),openwrt)
CFLAGS       += -DRTK_ETHERNET_LANNETINFO
endif
CFLAGS       += -DRTK_RETRY_GET_FLOCK

CFLAGS       += -fPIC
CFLAGS       += -lm

ifeq ($(FLAVOUR),luna)
ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
	CFLAGS       += -DCONFIG_RTK_PON_XDSL
endif
endif

.PHONY: all
all: $(LIB_MAPVENDORDATA_SO)
	cp $(LIB_MAPVENDORDATA_SO) ../lib/

$(LIB_MAPVENDORDATA_SO): $(LIB_MAPVENDORDATA_OBJ)
	$(CC) $(CFLAGS) -s -shared -o $@ $^

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf $(LIB_MAPVENDORDATA_SO)

.PHONY: romfs
romfs:
	$(ROMFSINST) $(LIB_MAPVENDORDATA_SO) /lib
