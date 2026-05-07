PWD := $(shell pwd)
MODULE_NAME = ebt_mark2

MOD_PATH = $(PWD)/src

.PHONY: all install clean w

all: 
	make -C $(KDIR) M=$(MOD_PATH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(MOD_PATH) clean

install:
	mkdir -p $(DESTDIR)/lib/modules
	cp -af $(MOD_PATH)/$(MODULE_NAME).ko $(DESTDIR)/lib/modules
