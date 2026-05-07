# Path for kernel module

#.EXPORT_ALL_VARIABLES:

DESTDIR:= $(ROMFSDIR) 
ifeq ("$(DESTDIR)", "")
  DESTDIR=`pwd`/.install
endif

# Path for user space app
APP_PATH = libebt_mark2
APP1_PATH = libebt_mark2_m
MODULE_PATH = module_mark2
MODULE1_PATH = module_mark2_m

all: 
	$(MAKE) -C $(MODULE_PATH) all
	$(MAKE) -C $(MODULE1_PATH) all
	$(MAKE) -C $(APP_PATH) all
	$(MAKE) -C $(APP1_PATH) all

clean:
	$(MAKE) -C $(MODULE_PATH) clean
	$(MAKE) -C $(MODULE1_PATH) clean
	$(MAKE) -C $(APP_PATH) clean
	$(MAKE) -C $(APP1_PATH) clean

romfs install:
	$(MAKE) -C $(MODULE_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(MODULE1_PATH) install DESTDIR=$(DESTDIR)
	$(MAKE) -C $(APP_PATH) install DESTDIR=$(DESTDIR) 
	$(MAKE) -C $(APP1_PATH) install DESTDIR=$(DESTDIR) 
