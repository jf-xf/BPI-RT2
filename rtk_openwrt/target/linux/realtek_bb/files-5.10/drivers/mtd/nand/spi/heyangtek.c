// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022, Realtek Semiconductor Corp.
 * [RTK] 2022/10/03: Support HYF1GQ4UDACAE, HYF1GQ4UPACAE, HYF2GQ4UHCCAE, HYF2GQ4UAACAE, HYF4GQ4UAACBE
 */


#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_HEYANGTEK		0xC9

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int hyf1gq4udacae_ooblayout_ecc(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 8;

	return 0;
}

static int hyf1gq4udacae_ooblayout_free(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 2;
	region->length = 6;

	return 0;
}

static const struct mtd_ooblayout_ops hyf1gq4udacae_ooblayout = {
	.ecc = hyf1gq4udacae_ooblayout_ecc,
	.free = hyf1gq4udacae_ooblayout_free,
};

static int hyf1gq4upacae_ooblayout_ecc(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
    if (section)
		return -ERANGE;
        
	region->offset = mtd->oobsize / 2;
	region->length = mtd->oobsize / 2;

	return 0;
}

static int hyf1gq4upacae_ooblayout_free(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = (mtd->oobsize / 2) - 2;

	return 0;
}

static const struct mtd_ooblayout_ops hyf1gq4upacae_ooblayout = {
	.ecc = hyf1gq4upacae_ooblayout_ecc,
	.free = hyf1gq4upacae_ooblayout_free,
};

static int hyf4gq4uaacbe_ooblayout_ecc(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 24;

	return 0;
}

static int hyf4gq4uaacbe_ooblayout_free(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (24 * section) + 2;
	region->length = 6;

	return 0;
}

static const struct mtd_ooblayout_ops hyf4gq4uaacbe_ooblayout = {
	.ecc = hyf4gq4uaacbe_ooblayout_ecc,
	.free = hyf4gq4uaacbe_ooblayout_free,
};

static int heyangtek_ecc_get_status_4bit(struct spinand_device *spinand, u8 status)
{
    u8 eccs = (status & STATUS_ECC_MASK) >> 4;

    if (eccs == 0) return 0;
    else if (eccs == 1) return 3;
    else if (eccs == 3) return 4;
    else return -EBADMSG;
}

static int heyangtek_ecc_get_status_1bit(struct spinand_device *spinand, u8 status)
{
    u8 eccs = (status & STATUS_ECC_MASK) >> 4;

    if (eccs == 0) return 0;
    else if (eccs == 1) return 1;
    else return -EBADMSG;
}

static int heyangtek_ecc_get_status_14bit(struct spinand_device *spinand, u8 status)
{
    u8 eccs = (status & STATUS_ECC_MASK) >> 4;

    if (eccs == 0) return 0;
    else if (eccs == 1) return 13;
    else if (eccs == 3) return 14;
    else return -EBADMSG;
}

static const struct spinand_info heyangtek_spinand_table[] = {
	SPINAND_INFO("HYF1GQ4UDACAE",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&hyf1gq4udacae_ooblayout, heyangtek_ecc_get_status_4bit)),

	SPINAND_INFO("HYF1GQ4UPACAE",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0xA1),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&hyf1gq4upacae_ooblayout, heyangtek_ecc_get_status_1bit)),

	SPINAND_INFO("HYF2GQ4UHCCAE",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x5A),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&hyf1gq4udacae_ooblayout, heyangtek_ecc_get_status_4bit)),

	SPINAND_INFO("HYF2GQ4UAACAE",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x52),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&hyf1gq4udacae_ooblayout, heyangtek_ecc_get_status_4bit)),

	SPINAND_INFO("HYF4GQ4UAACBE",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0xD4),
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(14, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&hyf4gq4uaacbe_ooblayout, heyangtek_ecc_get_status_14bit)),
};

static const struct spinand_manufacturer_ops heyangtek_spinand_manuf_ops = {
};

const struct spinand_manufacturer heyangtek_spinand_manufacturer = {
	.id = SPINAND_MFR_HEYANGTEK,
	.name = "HeYangTek",
	.chips = heyangtek_spinand_table,
	.nchips = ARRAY_SIZE(heyangtek_spinand_table),
	.ops = &heyangtek_spinand_manuf_ops,
};
