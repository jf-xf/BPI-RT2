# Broadband Forum IEEE 1905.1/1a stack
# 
# Copyright (c) 2017, Broadband Forum
# Copyright (c) 2018, Realtek Semiconductor Corp.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# Subject to the terms and conditions of this license, each copyright
# holder and contributor hereby grants to those receiving rights under
# this license a perpetual, worldwide, non-exclusive, no-charge,
# royalty-free, irrevocable (except for failure to satisfy the
# conditions of this license) patent license to make, have made, use,
# offer to sell, sell, import, and otherwise transfer this software,
# where such license applies only to those patent claims, already
# acquired or hereafter acquired, licensable by such copyright holder or
# contributor that are necessarily infringed by:
# 
# (a) their Contribution(s) (the licensed copyrights of copyright holders
#     and non-copyrightable additions of contributors, in source or binary
#     form) alone; or
# 
# (b) combination of their Contribution(s) with the work of authorship to
#     which such Contribution(s) was added by such copyright holder or
#     contributor, if, at the time the Contribution is added, such addition
#     causes such combination to be necessarily infringed. The patent
#     license shall not apply to any other combinations which include the
#     Contribution.
# 
# Except as expressly stated above, no rights or licenses from any
# copyright holder or contributor is granted under this license, whether
# expressly, by implication, estoppel or otherwise.
# 
# DISCLAIMER
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.

################################################################################
# Configuration section. Tune the parameters below according to your needs
################################################################################

# Platform dependent CONFIGs
#
# You can either "export PLATFORM=..." (and "FLAVOUR", if needed) before
# calling this Makefile or let "PLATFORM" undefined to use the default value
# (which is "linux")

# Shortcuts for later
#
SRC_FOLDER    := $(shell pwd)/src
OUTPUT_FOLDER := $(shell pwd)/output

CORE_VERSION_R1 := \"EasyMesh V1.3F\"
CORE_VERSION_R2 := \"EasyMesh V2.0\"
CORE_VERSION_R3 := \"EasyMesh V3.0\"
CORE_VERSION_R4 := \"EasyMesh V4.0\"

ifeq ($(PLATFORM),linux)
	ifeq ($(FLAVOUR), x86_generic)
		CC        := gcc
		#CC        := clang

		CCFLAGS   := -D_FLAVOUR_X86_GENERIC_
		CCFLAGS   += -D_HOST_IS_LITTLE_ENDIAN_=1

		CCFLAGS   += -fPIC

		CCFLAGS   += -DRTK_MULTI_AP_R2    # For R2 Unit Tests
		CCFLAGS   += -DRTK_MULTI_AP_R3	  # For R3 Unit Tests
		CCFLAGS   += -DRTK_MULTI_AP_R4	  # For R4 Unit Tests

		LDFLAGS       += -lrt -lpthread   # For threads
		LDFLAGS       += -lpcap           # For packet capture
		LDFLAGS       += -lcrypto         # For WPS crypto

		AL_SUPPORTED  := no
		HLE_SUPPORTED := yes
	else ifeq ($(FLAVOUR), x86_windows_mingw)
		CC        := $(MINGW_CROSS)gcc
		LD        := $(MINGW_CROSS)ld
		AR        := $(MINGW_CROSS)ar
		CCFLAGS   += -I $(MINGW_SYSROOT)/include/
		LDFLAGS   := -L $(MINGW_SYSROOT)/lib/

		CCFLAGS   += -D_FLAVOUR_X86_WINDOWS_MINGW_

		LDFLAGS       += -lws2_32 # Network stuff

		AL_SUPPORTED  := no
		HLE_SUPPORTED := yes
	else ifeq ($(FLAVOUR), jungle)
		CC        := $(CROSS_COMPILE)gcc
		LD        := $(CROSS_COMPILE)ld
		AR        := $(CROSS_COMPILE)ar
		STRIP     := $(CROSS_COMPILE)strip

		EXTERNAL_INC := $(OPENSSL_DIR)/include

		CCFLAGS   += -D_RTK_LINUX_
		ifeq ($(HOST_IS_BIG_ENDIAN), y)
			CCFLAGS += -D_HOST_IS_BIG_ENDIAN_=1
		else
			CCFLAGS += -D_HOST_IS_LITTLE_ENDIAN_=1
		endif

		ifeq ($(USE_BACKPORTS),y)
			CCFLAGS += -D_BACKPORTS_DRIVER_
			CCFLAGS += -D_USE_G6_DRIVER_
		endif

		ifeq ($(USE_COMMOM_STRUCTURE), y)
			CCFLAGS += -D_DRIVER_COMMON_STRUCTURE_
			EXTERNAL_INC += $(DIR_ROOT)/$(DRIVER_INC_PATH)
		endif

		CCFLAGS   += -DRTK_MULTI_AP

		CCFLAGS   += $(COMMON_CFLAGS) $(EXTRA_CFLAGS)

		CCFLAGS   += -fPIC

		LDFLAGS       += -lrt -lpthread
		LDFLAGS       += -L$(OPENSSL_DIR)/
		LDFLAGS       += -lcrypto         # For WPS crypto

		AL_SUPPORTED  := yes
		HLE_SUPPORTED := yes
	else ifeq ($(FLAVOUR), luna)
		CC        := $(CROSS_COMPILE)gcc
		LD        := $(CROSS_COMPILE)ld
		AR        := $(CROSS_COMPILE)ar
		STRIP     := $(CROSS_COMPILE)strip

		CCFLAGS   += -D_RTK_LINUX_

		ifeq ($(HOST_IS_BIG_ENDIAN), y)
			CCFLAGS += -D_HOST_IS_BIG_ENDIAN_=1
		else
			CCFLAGS += -D_HOST_IS_LITTLE_ENDIAN_=1
		endif

		CCFLAGS   += -DRTK_MULTI_AP

		ifeq ($(USE_COMMOM_STRUCTURE), y)
			CCFLAGS += -D_DRIVER_COMMON_STRUCTURE_
			CCFLAGS += -D_BACKPORTS_DRIVER_
			CCFLAGS += -D_USE_G6_DRIVER_
			EXTERNAL_INC += $(ROOTDIR)/$(DRIVER_INC_PATH)
		endif

		ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
			CCFLAGS += -DCONFIG_RTK_PON_XDSL
		endif

		ifeq ($(CONFIG_RTK_DEV_AP),y)
			CCFLAGS   += -D_MIB_LIB_
		endif

		ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
			CCFLAGS   += $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS)
		endif

		CCFLAGS   += -I$(ROOTDIR)/lib/libssl/include

		CCFLAGS   += -fPIC

		LDFLAGS       += -lrt -lpthread
		LDFLAGS       += -L$(ROOTDIR)/lib/libssl/
		LDFLAGS       += -lcrypto         # For WPS crypto

		ifeq ($(CONFIG_RTK_DEV_AP),y)
		CCFLAGS += -DRTK_RETRY_GET_FLOCK
		endif

		AL_SUPPORTED  := yes
		HLE_SUPPORTED := yes

		ifeq ($(CONFIG_RTK_DEV_AP),y)
		MIB_INC     := $(SRC_FOLDER)/../../../boa/src/LINUX/
		endif

	else ifeq ($(FLAVOUR), openwrt)
		CC        := $(CROSS_COMPILE)gcc
		LD        := $(CROSS_COMPILE)ld
		AR        := $(CROSS_COMPILE)ar
		STRIP     := $(CROSS_COMPILE)strip

		CCFLAGS   += -D_RTK_LINUX_

		ifeq ($(HOST_IS_BIG_ENDIAN), y)
			CCFLAGS += -D_HOST_IS_BIG_ENDIAN_=1
		else
			CCFLAGS += -D_HOST_IS_LITTLE_ENDIAN_=1
		endif

		CCFLAGS   += -DRTK_MULTI_AP

		ifeq ($(USE_COMMOM_STRUCTURE), y)
			CCFLAGS += -D_DRIVER_COMMON_STRUCTURE_
			CCFLAGS += -D_BACKPORTS_DRIVER_
			EXTERNAL_INC += $(DRIVER_INC_PATH)
		endif

		ifneq ($(CONFIG_RTK_DEV_AP),y) # PON/XDSL(SD5/SD9) platform
			CCFLAGS += -DCONFIG_RTK_PON_XDSL
			CCFLAGS   += $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS)
		endif

		CCFLAGS   += -fPIC

		LDFLAGS       += -lrt -lpthread
		LDFLAGS       += -lcrypto         # For WPS crypto

		ifeq ($(CONFIG_RTK_DEV_AP),y)
		CCFLAGS += -DRTK_RETRY_GET_FLOCK
		endif

		AL_SUPPORTED  := yes
		HLE_SUPPORTED := yes

	else
		$(error "Linux FLAVOUR unknown")
	endif

	CCFLAGS       += -Wall -Wextra -Werror -Wno-array-bounds

	ifeq ($(MAP_DEBUG),1)
		CCFLAGS       += -g -rdynamic -fno-omit-frame-pointer
		ifeq ($(DYNAMIC_CHECK),1)
			CCFLAGS       += -pg -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
			LDFLAGS += -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
		endif
	else
		CCFLAGS       += -Os
	endif
	CCFLAGS       += -DMAX_NETWORK_SEGMENT_SIZE=1500
	CCFLAGS       += -DINT8U="unsigned char"
	CCFLAGS       += -DINT16U="unsigned short int"
	CCFLAGS       += -DINT32U="unsigned int"
	CCFLAGS       += -DINT64U="unsigned long long"
	CCFLAGS       += -DINT8S="signed char"
	CCFLAGS       += -DINT16S="signed short int"
	CCFLAGS       += -DINT32S="signed int"
	CCFLAGS       += -DINT64S="signed long long"
	CCFLAGS       += -D_GNU_SOURCE
	ifeq ($(USE_WPA3), y)
		CCFLAGS       += -DWPA3_SUPPORT
	endif
else
$(error "'PLATFORM' variable was not set to a valid value")
endif

# Platform independent CONFIGs

ifdef MAP_VER_ID
	CCFLAGS   += -DMAP_VER_ID=\"$(MAP_VER_ID)\"
endif

ifdef MAP_VER_DATE
	CCFLAGS   += -DMAP_VER_DATE=\"$(MAP_VER_DATE)\"
endif

ifeq ($(USE_MULTI_AP_R4),y)
	CCFLAGS   += -DRTK_MULTI_AP_R4 -DRTK_MULTI_AP_R3 -DRTK_MULTI_AP_R2
	CCFLAGS   += -DMAP_VERSION="$(CORE_VERSION_R4)"
else ifeq ($(USE_MULTI_AP_R3),y)
	CCFLAGS   += -DRTK_MULTI_AP_R3 -DRTK_MULTI_AP_R2
	CCFLAGS   += -DMAP_VERSION="$(CORE_VERSION_R3)"
else ifeq ($(USE_MULTI_AP_R2),y)
	CCFLAGS   += -DRTK_MULTI_AP_R2
	CCFLAGS   += -DMAP_VERSION="$(CORE_VERSION_R2)"
else
	CCFLAGS   += -DMAP_VERSION="$(CORE_VERSION_R1)"
endif

#CCFLAGS += -DDO_NOT_ACCEPT_UNAUTHENTICATED_COMMANDS
CCFLAGS += -DSEND_EMPTY_TLVS
CCFLAGS += -DFIX_BROKEN_TLVS
CCFLAGS += -DSPEED_UP_DISCOVERY
  #
  # These are special flags that change the way the implementation behaves.
  # The README file contains more information regarding them.

# CCFLAGS += -D_BUILD_NUMBER_=\"$(shell cat version.txt)\"
  #
  # Version flag to identify the binaries
# CCFLAGS += -DREGISTER_EXTENSION_BBF
  #
  # These are special flags to enable Protocol extensions
# enable flags for extention Multi AP specification
# CCFLAGS += -DREGISTER_EXTENSION_MULTI_AP

CCFLAGS += $(addprefix -I,$(EXTERNAL_INC))

################################################################################
# End of configuration section. Do not touch anything from this point on
################################################################################

# "Addon" library. Includes open source libraries (such as cJSON)
# that add convenient features for both multi-ap-core and multi-ap.
#
ADDON_LIB        := $(OUTPUT_FOLDER)/libmultiap_addon.a
ADDON_INC        := $(SRC_FOLDER)/addon/interfaces

# "Common" library. Includes platform specific code that is used by both the
# AL and HLE entities (example: "PLATFORM_MALLOC()", "PLATFORM_PRINTF()", ...)
#
COMMON_LIB    := $(OUTPUT_FOLDER)/libcommon.a
COMMON_INC    := $(SRC_FOLDER)/common/interfaces

# "Crypto" library. Includes cryptographic functions (such as PLATFORM_SHA256)
# that calls platform specific SSL libraries such as OpenSSL.
#
CRYPTO_LIB         := $(OUTPUT_FOLDER)/libmultiap_crypto.a
CRYPTO_LIB_SHARED  := $(OUTPUT_FOLDER)/libmultiap_crypto.so
CRYPTO_INC         := $(SRC_FOLDER)/crypto/interfaces

# "DPP" library. Includes DPP functions (such as dpp_auth_int)
# that build and parse DPP action frames.
#
DPP_LIB         := $(OUTPUT_FOLDER)/libmultiap_dpp.a
DPP_LIB_SHARED  := $(OUTPUT_FOLDER)/libmultiap_dpp.so
DPP_INC         := $(SRC_FOLDER)/dpp/interfaces

# "Factory" library. Includes "pure" 1905 related functions to parse/build 1905
# packets. This library is also used by both AL and HLE but most (if not all) of
# it is platform independent.
#
FACTORY_LIB   := $(OUTPUT_FOLDER)/libfactory.a
FACTORY_INC   := $(SRC_FOLDER)/factory/interfaces
FACTORY_INC   += $(sort $(dir $(wildcard $(SRC_FOLDER)/factory/interfaces/extensions/*/)))

# The actual binaries that we are going to build: one for the AL and another one
# for the HLE entity.
#
# AL_EXE        := $(OUTPUT_FOLDER)/al_entity
HLE_EXE       := $(OUTPUT_FOLDER)/hle_entity

AL_LIB        := $(OUTPUT_FOLDER)/libmultiap.a
AL_LIB_SHARED := $(OUTPUT_FOLDER)/libmultiap.so

# BUILD_AL_SHARED := 1

# Compilation will need to create new directories during the process
#
MKDIR         := mkdir -p

# Export all these variable to child Makefiles
#
export


################################################################################
# Targets
################################################################################

.PHONY: all
ifeq ($(AL_SUPPORTED),yes)
all: al
endif
ifeq ($(HLE_SUPPORTED),yes)
all: $(HLE_EXE)
endif

.PHONY: ADDON_LIB
ADDON_LIB:
	$(MAKE) -C src/addon

.PHONY: COMMON_LIB
COMMON_LIB:
	$(MAKE) -C src/common

.PHONY: CRYPTO_LIB
CRYPTO_LIB:
	$(MAKE) -C src/crypto

.PHONY: DPP_LIB
DPP_LIB:
	$(MAKE) -C src/dpp

.PHONY: FACTORY_LIB
FACTORY_LIB:
	$(MAKE) -C src/factory

.PHONY: al
al: ADDON_LIB COMMON_LIB FACTORY_LIB CRYPTO_LIB DPP_LIB
	$(MAKE) -C src/al

$(HLE_EXE): COMMON_LIB FACTORY_LIB
	$(MAKE) -C src/hle

$(AL_LIB): $(COMMON_LIB) FACTORY_LIB
	$(MAKE) -C src/al lib

.PHONY: unit_tests
unit_tests: all
	$(MAKE) -C src/crypto unit_tests
	$(MAKE) -C src/factory unit_tests


.PHONY: clean
clean:
	$(MAKE) -C src/addon   clean
	$(MAKE) -C src/common  clean
	$(MAKE) -C src/crypto  clean
	$(MAKE) -C src/dpp     clean
	$(MAKE) -C src/factory clean
	$(MAKE) -C src/al      clean
	$(MAKE) -C src/hle     clean


.PHONY: static-analysis
static-analysis: clean
	scan-build $(MAKE) all
	cppcheck --enable=warning,style,performance,portability --inconclusive --xml-version=2 --error-exitcode=2 --suppress=memleakOnRealloc:* src 2> cppcheck-result.xml || ( cppcheck-htmlreport --source-encoding="iso8859-1" --title="multi-ap-core" --source-dir=. --report-dir=cppcheck-report --file=cppcheck-result.xml && rm cppcheck-result.xml )
