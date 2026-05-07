


ifeq ($(CONFIG_RTK_SPI_NAND_GEN3), y)
SPI_NAND_PATH  := spi_nand_gen3
else
SPI_NAND_PATH  := spi_nand
endif

ifeq ($(CONFIG_RTK_ONFI_GEN2), y)
ONFI_NAND_PATH := onfi_gen2
else
ONFI_NAND_PATH := onfi_gen1
endif

ifeq ($(CONFIG_RTK_ECC_GEN2), y)
ECC_PATH       := ecc_gen2
else
ECC_PATH       := ecc
endif

#EXTRA_CFLAGS = -I$(src) -I$(src)/spi_nand -I$(src)/ecc
ccflags-y := -I$(src) -I$(src)/$(ECC_PATH) -I../ -D__LUNA_KERNEL__
ccflags-$(CONFIG_MTD_SPI_NAND_RTK) += -I$(src)/$(SPI_NAND_PATH)
ccflags-$(CONFIG_MTD_ONFI_NAND_RTK) += -I$(src)/$(ONFI_NAND_PATH)
ccflags-$(CONFIG_MTD_DUALIF_NAND_RTK) += -I$(src)/$(SPI_NAND_PATH) -I$(src)/$(ONFI_NAND_PATH)

spin_flist := spi_nand_common.o
spin_flist += spi_nand_ctrl.o
spin_flist += spi_nand_winbond.o
spin_flist += spi_nand_gd.o
spin_flist += spi_nand_zentel.o
spin_flist += spi_nand_toshiba.o
spin_flist += spi_nand_mxic.o
spin_flist += spi_nand_esmt.o
ifeq ($(SPI_NAND_PATH), spi_nand_gen3)
spin_flist += spi_nand_micron.o
spin_flist += spi_nand_ato.o
spin_flist += spi_nand_heyangtek.o
spin_flist += spi_nand_longsys.o
spin_flist += spi_nand_dosilicon.o
spin_flist += spi_nand_etron.o
spin_flist += spi_nand_fmsh.o
spin_flist += spi_nand_xtx.o
ifeq ($(CONFIG_RTK_SOC_RTL9607C), y)
spin_flist += spi_nand_dhc.o
endif
endif
spin_flist := $(addprefix $(SPI_NAND_PATH)/,$(spin_flist))


onfi_flist := onfi_common.o
onfi_flist += onfi_ctrl.o
onfi_flist += onfi_toshiba.o
ifeq ($(ONFI_NAND_PATH), onfi_gen2)
onfi_flist += onfi_mxic.o
onfi_flist += onfi_micron.o
onfi_flist += onfi_winbond.o
onfi_flist += onfi_samsung.o
onfi_flist += onfi_esmt.o
endif
onfi_flist += onfi_common_chip.o
onfi_flist := $(addprefix $(ONFI_NAND_PATH)/,$(onfi_flist))

ecc_flist := ecc_ctrl.o
ifeq ($(SPI_NAND_PATH), spi_nand)
ecc_flist += bch6_software_encode.o
endif
ecc_flist := $(addprefix $(ECC_PATH)/,$(ecc_flist))

obj-y += $(ecc_flist)
obj-$(CONFIG_MTD_SPI_NAND_RTK)  += $(spin_flist)
obj-$(CONFIG_MTD_ONFI_NAND_RTK) += $(onfi_flist)
obj-$(CONFIG_MTD_DUALIF_NAND_RTK) += $(spin_flist) $(onfi_flist)
obj-$(CONFIG_MTD_DUALIF_NAND_RTK) += luna_nand_model.o
obj-y += luna_mtd_protect.o
obj-y += luna_mtd_usage_count.o
obj-y += luna_mtd_proc_bbt2.o
ifeq ($(CONFIG_MTD_DUALIF_NAND_RTK), y)
	obj-y += luna_mtd_nand_dualif.o
else
	obj-y += luna_mtd_nand.o
endif

