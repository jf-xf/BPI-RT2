// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 exceet electronics GmbH
 * Copyright (c) 2018 Kontron Electronics GmbH
 *
 * Author: Frieder Schrempf <frieder.schrempf@kontron.de>
 *
 *
 * Copyright (c) 2022, Realtek Semiconductor Corp.
 * [RTK] 2022/08/31: Support EM73D044VCO-H, EM73E044VCE-H
*/

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_ETRON		0xD5
#define ETRON_STATUS_ECC_HAS_BITFLIPS_T	(3 << 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));


static SPINAND_OP_VARIANTS(write_cache_x4_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_x4_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int em73x044vcfh_ooblayout_ecc(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section > 0)
		return -ERANGE;

	region->offset = mtd->oobsize / 2;
	region->length = mtd->oobsize / 2;

	return 0;
}

static int em73x044vcfh_ooblayout_free(struct mtd_info *mtd, int section,
					 struct mtd_oob_region *region)
{
	if (section > 0)
		return -ERANGE;

	/* 2 bytes reserved for BBM */
	region->offset = 2;
	region->length = (mtd->oobsize / 2) - 2;

	return 0;
}

static const struct mtd_ooblayout_ops em73x044vcfh_ooblayout = {
	.ecc = em73x044vcfh_ooblayout_ecc,
	.free = em73x044vcfh_ooblayout_free,
};

static int em73x044vcfh_ecc_get_status(struct spinand_device *spinand,
					 u8 status)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case STATUS_ECC_HAS_BITFLIPS:
		return (nanddev_get_ecc_conf(nand)->strength - 1);

	case ETRON_STATUS_ECC_HAS_BITFLIPS_T:
		return nanddev_get_ecc_conf(nand)->strength;

	default:
		break;
	}

	return -EINVAL;
}

static const struct spinand_info etron_spinand_table[] = {
	SPINAND_INFO("EM73C044VCF-H",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x25),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_x4_variants,
                                      &update_cache_x4_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&em73x044vcfh_ooblayout, em73x044vcfh_ecc_get_status)),

	SPINAND_INFO("EM73D044VCL-H",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x2E),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_x4_variants,
                                      &update_cache_x4_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&em73x044vcfh_ooblayout, em73x044vcfh_ecc_get_status)),

	SPINAND_INFO("EM73E044VCB-H",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x2F),
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_x4_variants,
                                      &update_cache_x4_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&em73x044vcfh_ooblayout, em73x044vcfh_ecc_get_status)),

	SPINAND_INFO("EM73F044VCA-H",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x2D),
		     NAND_MEMORG(1, 4096, 256, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_x4_variants,
                                      &update_cache_x4_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&em73x044vcfh_ooblayout, em73x044vcfh_ecc_get_status)),

	SPINAND_INFO("EM73D044VCO-H",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x3A),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_x4_variants,
                                      &update_cache_x4_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&em73x044vcfh_ooblayout, em73x044vcfh_ecc_get_status)),

	SPINAND_INFO("EM73E044VCE-H",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x3B),
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_x4_variants,
                                      &update_cache_x4_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&em73x044vcfh_ooblayout, em73x044vcfh_ecc_get_status)),
};

static const struct spinand_manufacturer_ops etron_spinand_manuf_ops = {
};

const struct spinand_manufacturer etron_spinand_manufacturer = {
	.id = SPINAND_MFR_ETRON,
	.name = "Etron",
	.chips = etron_spinand_table,
	.nchips = ARRAY_SIZE(etron_spinand_table),
	.ops = &etron_spinand_manuf_ops,
};
