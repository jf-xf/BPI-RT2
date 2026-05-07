PWD := $(shell pwd)
MODULE_NAME = smuxdev

SMUXDEV_PATH = $(PWD)/src

.PHONY: all install clean

all modules: 
	@echo -e "\n\n[smuxdev $@]"
	@echo "KDIR: $(KDIR)"
	@echo "SMUXDEV_PATH: $(SMUXDEV_PATH)"
	@echo "APOLLODIR: $(APOLLODIR)"
	@echo "EXTRA_CFLAGS: $(EXTRA_CFLAGS)"
	@echo "CFLAGS: $(CFLAGS)"
	$(MAKE) -C $(KDIR) M=$(SMUXDEV_PATH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(SMUXDEV_PATH) clean

install:
	mkdir -p $(DESTDIR)/lib/modules
	cp -af $(SMUXDEV_PATH)/$(MODULE_NAME).ko $(DESTDIR)/lib/modules
