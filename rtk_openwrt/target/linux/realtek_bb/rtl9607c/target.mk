ARCH:=mips
SUBTARGET:=rtl9607c
BOARDNAME:=RTL9607C based boards
RTK_HW_VERSION:=skip
FEATURES+=pci

CPU_SUBTYPE:=nomips16

define Target/Description
	Build firmware image for Realtek 9607C MIPS-based devices.
endef
