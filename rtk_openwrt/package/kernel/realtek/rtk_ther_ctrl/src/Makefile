PWD := $(shell pwd)
MODULE_NAME = rtk_ther_ctrl
SRC_PATH = $(PWD)

obj-m := rtk_ther_ctrl.o
rtk_ther_ctrl-objs := ther_ctrl.o
$(MODULE_NAME)-y = $(OBJS)

.PHONY: all install clean

all: modules

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(SRC_PATH) modules

strip:
	$(CROSS_COMPILE)strip rtk_ther_ctrl.ko --strip-unneeded

clean:
	$(MAKE) -C $(KDIR) M=$(SRC_PATH) clean

install:
	mkdir -p $(DESTDIR)/lib/modules
	cp -af $(SMUXDEV_PATH)/$(MODULE_NAME).ko $(DESTDIR)/lib/modules
	#install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	#/sbin/depmod -a ${KVER}
