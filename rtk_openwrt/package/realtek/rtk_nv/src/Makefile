
include $(LINUXDIR)/.config
include $(ROOTDIR)/.config

GIT_COMMIT  := $(shell git rev-parse --verify HEAD)
GIT_DATE    := $(firstword $(shell git --no-pager show --date=iso-strict --format="%ad" --name-only))
CFLAGS += -fPIC -I. 
CFLAGS += -DGIT_COMMIT=\"$(GIT_COMMIT)\" -DGIT_DATE=\"$(GIT_DATE)\"
CFLAGS += -include $(LINUXDIR)/include/generated/autoconf.h
CFLAGS += -include $(ROOTDIR)/autoconf.h
LDFLAGS += -L./

DESTDIR:= $(ROMFSDIR) 

## EXEC ##
exec_y = nv
nv_objs = main.o

## LIB ##
lib_y = libnv.so
libnv_objs = libnv.o
ifeq ($(CONFIG_ARM64),y)
  libnv_objs += uboot_2015_crc32.o
else
  libnv_objs += rtk_crc32.o
  CFLAGS += -DHAVE_REDUND_ENV -DOTTO_ENV_SIZE
endif

#ifdef CONFIG_LUNA_MULTI_UBOOT_ENV
UBOOT-ENV-OPT= -DHAVE_REDUND_ENV
#endif

ifeq ($(CONFIG_ARM64),y)
ifeq ($(CONFIG_MTD_SPI_NAND), y)
  CFLAGS += -D__LITTLE_ENDIAN -DG3_ENV_SIZE=0x40000 $(UBOOT-ENV-OPT)
else # 98F nor flash
  CFLAGS += -D__LITTLE_ENDIAN -DG3_ENV_SIZE=0x10000 $(UBOOT-ENV-OPT)
endif
else
endif


.PHONY: all install-dev install romfs clean rcX

all: $(lib_y) $(exec_y)

.c.o:
	$(CC) -M $(CFLAGS) $< > $@.d
	$(CC) $(CFLAGS) -c $< -o $@

nv: $(nv_objs) 
	$(CC) $(LDFLAGS) -o $@ $^ -lnv
	
libnv.so: $(libnv_objs)
	$(CC) $(LDFLAGS) -shared -o $@ $^ 
	
install-dev: all
	mkdir -p $(DESTDIR)/include/rtk
	cp -f libnv.h $(DESTDIR)/include/rtk
	mkdir -p $(DESTDIR)/usr/lib
	cp -af $(lib_y)  $(DESTDIR)/usr/lib

install: all
	mkdir -p $(DESTDIR)/bin
	cp -af $(exec_y) $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/usr/lib
	cp -af $(lib_y) $(DESTDIR)/usr/lib

romfs: install

clean:
	-rm -f $(lib_y) $(exec_y) *.elf *.o *.d

rcX:

