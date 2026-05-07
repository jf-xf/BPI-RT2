PWD := $(shell pwd)

# Path for kernel module
MODULE_PATH = $(PWD)/module

#.EXPORT_ALL_VARIABLES:
DESTDIR:= $(strip $(ROMFSDIR))
ifeq ("$(DESTDIR)", "")
  DESTDIR=`pwd`/.install
endif

all: install-header
	$(MAKE) -C $(KDIR) M=$(MODULE_PATH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(MODULE_PATH) clean

install-header:
	mkdir -p $(ROOTDIR)/include/rtk
	cp -af include/*.h $(ROOTDIR)/include/rtk

romfs install:
	mkdir -p $(DESTDIR)/lib/modules/users
	cp -af $(MODULE_PATH)/hostSpeedUp.ko $(DESTDIR)/lib/modules/users
