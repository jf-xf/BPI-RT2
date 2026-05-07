// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Cortina Access
 *
 * Author: PengPeng Chen <pengpeng.chen@cortina-access.com>
 *
 * Copyright (c) 2021, Realtek Semiconductor Corp.
 * [RTK] 2021/05/21: Support DS35Q2GB-IB, DS35Q2GB-IB
 * [RTK] 2022/02/08: Enabled Dosilison into probe list.
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_DOSILICON		0xE5

#define DOSILICON_STATUS_ECC_MASK		GENMASK(7, 4)
#define DOSILICON_STATUS_ECC_NO_BITFLIPS	(0 << 4)
#define DOSILICON_STATUS_ECC_1TO3_BITFLIPS	(1 << 4)
#define DOSILICON_STATUS_ECC_1TO4_BITFLIPS	(1 << 4)
#define DOSILICON_STATUS_ECC_4TO6_BITFLIPS	(3 << 4)
#define DOSILICON_STATUS_ECC_7TO8_BITFLIPS	(5 << 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int ds35qxga_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int ds35qxga_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = mtd->oobsize - 2;

	return 0;
}

static const struct mtd_ooblayout_ops ds35qxga_ooblayout = {
	.ecc = ds35qxga_ooblayout_ecc,
	.free = ds35qxga_ooblayout_free,
};

static int ds35qxgb_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
    if (section)
        return -ERANGE;

    region->offset = mtd->oobsize / 2;
    region->length = mtd->oobsize / 2;

    return 0;
}

static int ds35qxgb_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = (mtd->oobsize / 2) - 2;

	return 0;
}

static const struct mtd_ooblayout_ops ds35qxgb_ooblayout = {
	.ecc = ds35qxgb_ooblayout_ecc,
	.free = ds35qxgb_ooblayout_free,
};


static int ds35qxga_ecc_get_status(struct spinand_device *spinand, u8 status)
{
    switch (status & STATUS_ECC_MASK) {
        case STATUS_ECC_NO_BITFLIPS:
            return 0;

        case DOSILICON_STATUS_ECC_1TO4_BITFLIPS:
            return 4;

        case STATUS_ECC_UNCOR_ERROR:
            return -EBADMSG;

        default:
            break;
    }
    return -EINVAL;
}

static int ds35qxgb_ecc_get_status(struct spinand_device *spinand, u8 status)
{
    switch (status & DOSILICON_STATUS_ECC_MASK) {
        case STATUS_ECC_NO_BITFLIPS:
            return 0;

        case DOSILICON_STATUS_ECC_1TO3_BITFLIPS:
            return 3;

        case STATUS_ECC_UNCOR_ERROR:
            return -EBADMSG;

        case DOSILICON_STATUS_ECC_4TO6_BITFLIPS:
            return 6;

        case DOSILICON_STATUS_ECC_7TO8_BITFLIPS:
            return 8;

        default:
            break;
    }

    return -EINVAL;
}


static const struct spinand_info dosilicon_spinand_table[] = {
	SPINAND_INFO("DS35Q1GA_IB",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x71),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&ds35qxga_ooblayout, ds35qxga_ecc_get_status)),
	SPINAND_INFO("DS35Q2GA_IB",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x72),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 2, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&ds35qxga_ooblayout, ds35qxga_ecc_get_status)),
    SPINAND_INFO("DS35Q2GB-IB",
             SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xF2),
             NAND_MEMORG(1, 2048, 128, 64, 2048, 40, 2, 1, 1),
             NAND_ECCREQ(8, 512),
             SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                          &write_cache_variants,
                          &update_cache_variants),
             SPINAND_HAS_QE_BIT,
             SPINAND_ECCINFO(&ds35qxgb_ooblayout, ds35qxgb_ecc_get_status)),
    SPINAND_INFO("DS35Q4GM-IB",
             SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xF4),
             NAND_MEMORG(1, 2048, 128, 64, 4096, 80, 2, 1, 1),
             NAND_ECCREQ(8, 512),
             SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
                          &write_cache_variants,
                          &update_cache_variants),
             SPINAND_HAS_QE_BIT,
             SPINAND_ECCINFO(&ds35qxgb_ooblayout, ds35qxgb_ecc_get_status)),
};

static const struct spinand_manufacturer_ops dosilicon_spinand_manuf_ops = {
};

const struct spinand_manufacturer dosilicon_spinand_manufacturer = {
	.id = SPINAND_MFR_DOSILICON,
	.name = "Dosilicon",
    .chips = dosilicon_spinand_table,
    .nchips = ARRAY_SIZE(dosilicon_spinand_table),
	.ops = &dosilicon_spinand_manuf_ops,
};

