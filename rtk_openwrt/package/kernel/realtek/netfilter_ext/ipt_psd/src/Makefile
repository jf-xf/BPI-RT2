# Path for kernel module

#.EXPORT_ALL_VARIABLES:

DESTDIR:= $(ROMFSDIR) 
ifeq ("$(DESTDIR)", "")
  DESTDIR=`pwd`/.install
endif

# Path for user space app
APP_PATH = libxt_psd
MODULE_PATH = module

all: 
	$(MAKE) -C $(MODULE_PATH) all
	$(MAKE) -C $(APP_PATH) all

clean:
	$(MAKE) -C $(MODULE_PATH) clean
	$(MAKE) -C $(APP_PATH) clean

romfs install:
	$(MAKE) -C $(MODULE_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(APP_PATH) install DESTDIR=$(DESTDIR) 
