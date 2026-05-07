// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022, Realtek Semiconductor Corp.
 * [RTK] 2022/06/29: Support F35SQA002G.
 * [RTK] 2022/08/18: Support FS35ND01G-S1Y2, FS35ND02G-S3Y2, FS35ND04G-S2Y2
 */


#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_LONGSYS		0xCD

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

static int fs35nd0xg_ooblayout_ecc(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
    return -ERANGE;
}

static int fs35nd0xg_ooblayout_free(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = mtd->oobsize - 2;

	return 0;
}

static const struct mtd_ooblayout_ops fs35nd0xg_ooblayout = {
	.ecc = fs35nd0xg_ooblayout_ecc,
	.free = fs35nd0xg_ooblayout_free,
};

static int fs35nd0xg_ecc_get_status(struct spinand_device *spinand, u8 status)
{
    u8 eccs = (status & STATUS_ECC_MASK) >> 4;

    if (eccs == 0) return 0;
    else if (eccs == 1) return 4;
    else return -EBADMSG;
}


static int f35sqa002g_ooblayout_ecc(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
    return -ERANGE;
}

static int f35sqa002g_ooblayout_free(struct mtd_info *mtd, int section, struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = mtd->oobsize - 2;

	return 0;
}

static const struct mtd_ooblayout_ops f35sqa002g_ooblayout = {
	.ecc = f35sqa002g_ooblayout_ecc,
	.free = f35sqa002g_ooblayout_free,
};

static int f35sqa002g_ecc_get_status(struct spinand_device *spinand, u8 status)
{
    u8 eccs = (status & STATUS_ECC_MASK) >> 4;

    if (eccs == 0) return 0;
    else if (eccs == 1) return 1;
    else return -EBADMSG;
}

static const struct spinand_info longsys_spinand_table[] = {
	SPINAND_INFO("FS35ND01G-S1Y2",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xEA),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fs35nd0xg_ooblayout, fs35nd0xg_ecc_get_status)),

	SPINAND_INFO("FS35ND02G-S3Y2",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xEB),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fs35nd0xg_ooblayout, fs35nd0xg_ecc_get_status)),

	SPINAND_INFO("FS35ND04G-S2Y2",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xEC),
		     NAND_MEMORG(1, 2048, 64, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fs35nd0xg_ooblayout, fs35nd0xg_ecc_get_status)),

	SPINAND_INFO("F35SQA002G",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x72),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                                      &write_cache_variants,
                                      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&f35sqa002g_ooblayout, f35sqa002g_ecc_get_status)),
};

static const struct spinand_manufacturer_ops longsys_spinand_manuf_ops = {
};

const struct spinand_manufacturer longsys_spinand_manufacturer = {
	.id = SPINAND_MFR_LONGSYS,
	.name = "Longsys",
	.chips = longsys_spinand_table,
	.nchips = ARRAY_SIZE(longsys_spinand_table),
	.ops = &longsys_spinand_manuf_ops,
};
