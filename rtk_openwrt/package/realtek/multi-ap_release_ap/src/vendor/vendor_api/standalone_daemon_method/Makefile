################################################################################
# Targets
################################################################################

.PHONY: all
all: clean
	$(CC) vendor_code.c $(CCFLAGS) $(LDFLAGS) -o map_vendor
	$(CC) map_vendor_api.c $(CCFLAGS) $(LDFLAGS) -c -o map_vendor_api.o
	$(CC) -shared map_vendor_api.o -o libmapvendor.so
	$(CC) rtic_daemon.c rtic_util.c $(CCFLAGS) $(LDFLAGS) -o map_rtic
	$(CC) rnr_daemon.c $(CCFLAGS) $(LDFLAGS) -o map_rnr

.PHONY: clean
clean:
	rm -rf map_vendor
	rm -rf map_rtic
	rm -rf *.o
	rm -rf libmapvendor.so
	rm -rf map_rnr

.PHONY: romfs
romfs:
	$(ROMFSINST) map_vendor /bin/map_vendor
	$(ROMFSINST) map_rtic /bin/map_rtic
	$(ROMFSINST) libmapvendor.so /lib/libmapvendor.so
	$(ROMFSINST) map_rnr /bin/map_rnr
