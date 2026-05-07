/*
 * =====================================================================================
 *
 *       Filename:  luna_nand_model.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  C. Y. Yang (),
 *        Company:
 *
 * =====================================================================================
 */

#include <linux/types.h>

#if IS_ENABLED(CONFIG_MTD_SPI_NAND_RTK) || IS_ENABLED(CONFIG_MTD_DUALIF_NAND_RTK)
#include "spi_nand_ctrl.h"
#include "spi_nand_common.h"
#include "spi_nand_struct.h"
#endif

#if IS_ENABLED(CONFIG_MTD_ONFI_NAND_RTK) || IS_ENABLED(CONFIG_MTD_DUALIF_NAND_RTK)
#include "onfi_struct.h"
#include "onfi_common.h"
#endif

#ifndef LUNA_NAND_MODEL_H__
#define LUNA_NAND_MODEL_H__

struct luna_nand_t;

typedef void (luna_nand_pio_read_t)(struct luna_nand_t *chip, void *addr, uint32_t wr_bytes, uint32_t blk_pge_addr, uint32_t col_addr);
typedef int32_t (luna_nand_pio_write_t)(struct luna_nand_t *chip, void *addr, uint32_t wr_bytes, uint32_t blk_pge_addr, uint32_t col_addr);
typedef void (luna_nand_page_read_t)(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr);
typedef int32_t (luna_nand_page_write_t)(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr);
typedef int32_t (luna_nand_page_read_ecc_t)(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr, void *p_eccbuf);
typedef int32_t (luna_nand_page_write_ecc_t)(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr, void *p_eccbuf);
typedef int32_t (luna_nand_erase_block_t) (struct luna_nand_t *chip, uint32_t blk_pge_addr);
typedef void (luna_nand_model_init_t) (struct luna_nand_t *chip);
typedef void  (luna_ecc_encode_t)(struct luna_nand_t *chip, uint32_t ecc_ability, void *dma_addr, void *p_eccbuf);
typedef int32_t (luna_ecc_decode_t)(struct luna_nand_t *chip, uint32_t ecc_ability, void *dma_addr, void *p_eccbuf);
typedef void (luna_nand_reset_t)(struct luna_nand_t *chip, uint32_t cs);
typedef const char* (luna_nand_mtd_part_name_t)(struct luna_nand_t *chip);
typedef const char* (luna_nand_proc_name_t)(struct luna_nand_t *chip);
typedef uint8_t (luna_nand_get_status_t)(struct luna_nant_t *chip);


struct luna_nand_model_t {
	luna_nand_pio_read_t       *_pio_read;
	luna_nand_pio_write_t      *_pio_write;
	luna_nand_page_read_t      *_page_read;
	luna_nand_page_write_t     *_page_write;
	luna_nand_page_read_ecc_t  *_page_read_ecc;
	luna_nand_page_write_ecc_t *_page_write_ecc;
	luna_nand_erase_block_t    *_block_erase;
	luna_nand_model_init_t     *_init;
	luna_ecc_encode_t          *_ecc_encode;
	luna_ecc_decode_t          *_ecc_decode;
	luna_nand_reset_t          *_reset;
	luna_nand_mtd_part_name_t  *_mtd_part_name;
	luna_nand_proc_name_t      *_proc_name;
	luna_nand_get_status_t     *_get_status;
	uint32_t blk_num; // block number of flash
	uint32_t pge_sz;  // page size
	uint32_t pge_num_per_blk; // how many page on a block
	uint32_t spare_size; // spare size
	uint32_t oob_size;   // avaiable size except ECC
	uint32_t ecc_ability;
	uint32_t man_id;
	uint32_t dev_id;
	uint32_t pge_blk_shift; // shift of page and block
};

extern struct luna_nand_model_t luna_nand_spi_model;
extern struct luna_nand_model_t luna_nand_onfi_model;
#endif
