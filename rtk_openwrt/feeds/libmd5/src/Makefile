#defined on openwrt Makefile: TARGET_CFLAGS += $(FPIC)
#CFLAGS += -fPIC

OPENWRT_LIBMD5_SO := $(if ${ABI_VERSION}, libmd5.so.${ABI_VERSION}, libmd5.so)

all: $(OPENWRT_LIBMD5_SO)

libmd5.o: libmd5.h libmd5.c
	$(CC) -Os -c -o $@ $(CFLAGS) libmd5.c

libmd5.a: libmd5.o
	$(AR) $(ARFLAGS) $@ $?

#Be careful to use soname and ABI_VERSION, openwrt will check it when other package link this LIB.
#$(LD) -shared --warn-common --warn-once -z combreloc -soname=$@.${ABI_VERSION}
$(OPENWRT_LIBMD5_SO): libmd5.a
	$(LD) -shared --warn-common --warn-once -z combreloc -soname=$@ \
		-o $@ --whole-archive $?
	-ln -s $(OPENWRT_LIBMD5_SO) libmd5.so
ifdef CONFIG_USE_RSDK_WRAPPER
	install -m 644 libmd5.so $(ROOTDIR)/lib/librtk/${OPENWRT_LIBMD5_SO}
	install -m 644 libmd5.a $(ROOTDIR)/lib/librtk/libmd5.a
endif #CONFIG_USE_RSDK_WRAPPER

clean:
	rm -rf libmd5.o libmd5.a libmd5.so*
ifdef CONFIG_USE_RSDK_WRAPPER
	rm -rf $(ROOTDIR)/lib/librtk/${OPENWRT_LIBMD5_SO}
	rm -rf $(ROOTDIR)/lib/librtk/libmd5.a
endif #CONFIG_USE_RSDK_WRAPPER


