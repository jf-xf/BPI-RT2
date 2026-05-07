# Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

BUILD_AL_SHARED := 1

BUILD_SHARED_MODULES := 0

MAP_DEBUG := 0

# Automatically generate the first 5 bytes of git commit id
MAP_VER_ID := $(shell git rev-parse --short=10 HEAD)
MAP_VER_DATE := $(shell git log -1 --format=%ci | cut -c1-10)

CC        := $(CROSS_COMPILE)gcc
CXX       := $(CROSS_COMPILE)g++
LD        := $(CROSS_COMPILE)ld
AR        := $(CROSS_COMPILE)ar
STRIP     := $(CROSS_COMPILE)strip

GCC_FEATURES := $(shell $(CC) -dM -E - <<< "\#include <features.h>")

USING_GLIBC := 0

ifneq (,$(findstring __GLIBC__,$(GCC_FEATURES)))
	USING_GLIBC := 1
endif

ifeq ($(MAP_DEBUG),1)
	ifeq ($(USING_GLIBC),1)
		DYNAMIC_CHECK := 1
	endif
endif

ifeq ($(origin FLAVOUR), undefined)
$(error Undefined build flavour)
endif

export

MULTI_AP_FOLDER := multi-ap

MULTI_AP_DIR := $(shell pwd)/$(MULTI_AP_FOLDER)

export MULTI_AP_DIR

.PHONY: all
all: lib
ifeq ($(BUILD_AL_SHARED),1)
	rm -rf $(MULTI_AP_FOLDER)/lib/libmapvendor.a
	if [ ! -e $(MULTI_AP_FOLDER)/lib/libmapvendor.so ]; then \
		$(MAKE) -C vendor/empty; \
		cp vendor/empty/libmapvendor.so $(MULTI_AP_FOLDER)/lib/libmapvendor.so; \
	fi
else
	rm -rf $(MULTI_AP_FOLDER)/lib/libmapvendor.so
	if [ ! -e $(MULTI_AP_FOLDER)/lib/libmapvendor.a ]; then \
		$(MAKE) -C vendor/empty; \
		cp vendor/empty/libmapvendor.a $(MULTI_AP_FOLDER)/lib/libmapvendor.a; \
	fi
endif
	$(MAKE) -C $(MULTI_AP_FOLDER)

.PHONY: lib
lib:
	$(MAKE) -C multi-ap-core
	if ! [ -d "$(MULTI_AP_FOLDER)/lib" ]; then \
		rm $(MULTI_AP_FOLDER)/lib; \
		mkdir -p $(MULTI_AP_FOLDER)/lib; \
	fi
	rm -f $(MULTI_AP_FOLDER)/lib/libmultiap.*
	rm -f $(MULTI_AP_FOLDER)/lib/libmultiap_addon.*
	rm -f $(MULTI_AP_FOLDER)/lib/libmultiap_crypto.*
	rm -f $(MULTI_AP_FOLDER)/lib/libmultiap_dpp.*
ifeq ($(BUILD_AL_SHARED),1)
	cp multi-ap-core/output/libmultiap.so $(MULTI_AP_FOLDER)/lib
else
	cp multi-ap-core/output/libmultiap.a $(MULTI_AP_FOLDER)/lib
endif
ifeq ($(BUILD_SHARED_MODULES),1)
	cp multi-ap-core/output/libmultiap_crypto.so $(MULTI_AP_FOLDER)/lib
	cp multi-ap-core/output/libmultiap_dpp.so $(MULTI_AP_FOLDER)/lib
else
	cp multi-ap-core/output/libmultiap_crypto.a $(MULTI_AP_FOLDER)/lib
	cp multi-ap-core/output/libmultiap_dpp.a $(MULTI_AP_FOLDER)/lib
endif
	cp multi-ap-core/output/libmultiap_addon.a $(MULTI_AP_FOLDER)/lib
	cp multi-ap-core/src/al/src_independent/map_commands.h $(MULTI_AP_FOLDER)/include
	cp multi-ap-core/src/al/internal_interfaces/map_utils.h $(MULTI_AP_FOLDER)/include
	cp multi-ap-core/src/al/internal_interfaces/map_initialization.h $(MULTI_AP_FOLDER)/include


.PHONY: clean
clean:
	$(MAKE) -C multi-ap-core  clean
	$(MAKE) -C $(MULTI_AP_FOLDER) clean

.PHONY: romfs
romfs:
	$(MAKE) -C $(MULTI_AP_FOLDER) romfs

.PHONY: static-analysis
static-analysis: clean
	# scan-build $(MAKE) lib
	rm -rf cppcheck-report
	cppcheck --enable=warning,performance,portability --inconclusive --xml-version=2 --error-exitcode=2 --suppress=memleakOnRealloc:* multi-ap-core multi-ap 2> cppcheck-result.xml || ( cppcheck-htmlreport --source-encoding="iso8859-1" --title="multi-ap" --source-dir=. --report-dir=cppcheck-report --file=cppcheck-result.xml && rm cppcheck-result.xml )

.PHONY: release
release:
	$(MAKE) -C multi-ap release
