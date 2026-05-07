CCFLAGS       := -I$(MULTI_AP_DIR)/include

################################################################################
# Targets
################################################################################

.PHONY: all
all: clean
	$(CC) map_vendor_api.c $(CCFLAGS) $(LDFLAGS) -c -o map_vendor_api.o
	$(CC) -shared map_vendor_api.o -o libmapvendor.so
	$(AR) -rcs libmapvendor.a map_vendor_api.o

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf libmapvendor.so

.PHONY: romfs
romfs:
	$(ROMFSINST) libmapvendor.so /lib/libmapvendor.so
