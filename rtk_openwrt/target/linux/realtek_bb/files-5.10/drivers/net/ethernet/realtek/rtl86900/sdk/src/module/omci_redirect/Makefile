#########################################################
#	Build Realtek OMCI Redirect modules									#
#########################################################

include $(APOLLODIR)/Compiler_Flag

MODULE_NAME=omcir
PWD = $(shell pwd)

SDKDIR=$(APOLLODIR)/sdk
SDK_INCLUDE   = $(SDKDIR)/include
SYS_INCLUDE   = $(SDKDIR)/system/include
DRV_INCLUDE   = $(SDKDIR)/system/linux
MOD_INCLUDE   = $(SDKDIR)/src/module

EXTRA_CFLAGS+=$(PON_CFLAGS)

FLAGS+=-I$(SDK_INCLUDE) -I$(SYS_INCLUDE) -I$(DRV_INCLUDE) -I$(NETDIR)


EXTRA_CFLAGS+=$(FLAGS)

obj-m := $(MODULE_NAME).o

$(MODULE_NAME)-objs:= omci_redirect.o

all:
	@echo "   BUILD module $(MODULE_NAME), $(FLAGS)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=rlx CROSS_COMPILE=$(CROSS_COMPILE)
 

install:
	@if [ ! -d $(ROMFSDIR)/lib/modules ]; \
        then \
                mkdir $(ROMFSDIR)/lib/modules ; \
    fi
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules/

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko 
