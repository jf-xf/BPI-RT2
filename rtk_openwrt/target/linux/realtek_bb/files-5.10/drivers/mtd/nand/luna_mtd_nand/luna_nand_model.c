/*
 * =====================================================================================
 *
 *       Filename:  luna_nand_model.c
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


#include "luna_mtd_nand.h"
#include "luna_nand_model.h"


#if IS_ENABLED(CONFIG_MTD_SPI_NAND_RTK) || IS_ENABLED(CONFIG_MTD_DUALIF_NAND_RTK)
static void
luna_spi_pio_read(
	struct luna_nand_t *chip,
	void *addr,
	uint32_t wr_bytes,
	uint32_t blk_pge_addr,
	uint32_t col_addr
)
{
	chip->_spi_nand_info->_model_info->_pio_read(chip->_spi_nand_info, addr, wr_bytes, blk_pge_addr, col_addr);
}

static int32_t
luna_spi_pio_write(
	struct luna_nand_t *chip,
	void *addr,
	uint32_t wr_bytes,
	uint32_t blk_pge_addr,
	uint32_t col_addr
)
{
	chip->_spi_nand_info->_model_info->_pio_write(chip->_spi_nand_info, addr, wr_bytes, blk_pge_addr, col_addr);
}

static void
luna_spi_page_read(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr)
{
	chip->_spi_nand_info->_model_info->_page_read(chip->_spi_nand_info, dma_addr, blk_pge_addr);
}

static int32_t
luna_spi_page_write(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr)
{
	return chip->_spi_nand_info->_model_info->_page_write(chip->_spi_nand_info, dma_addr, blk_pge_addr);
}

static int32_t
luna_spi_page_read_ecc(
	struct luna_nand_t *chip,
	void *dma_addr,
	uint32_t blk_pge_addr,
	void *p_eccbuf
)
{
	return chip->_spi_nand_info->_model_info->_page_read_ecc(chip->_spi_nand_info, dma_addr, blk_pge_addr, p_eccbuf);
}

static int32_t
luna_spi_page_write_ecc(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr, void *p_eccbuf)
{
	return chip->_spi_nand_info->_model_info->_page_write_ecc(chip->_spi_nand_info, dma_addr, blk_pge_addr, p_eccbuf);
}

static int32_t
luna_spi_erase_block (struct luna_nand_t *chip, uint32_t blk_pge_addr)
{
	return chip->_spi_nand_info->_model_info->_block_erase(chip->_spi_nand_info, blk_pge_addr);
}

static void
luna_spi_ecc_encode(struct luna_nand_t *chip, uint32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
  return chip->_spi_nand_info->_ecc_encode(ecc_ability, dma_addr, p_eccbuf);
}

static int32_t
luna_spi_ecc_decode(struct luna_nand_t *chip, uint32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
  return chip->_spi_nand_info->_ecc_decode(ecc_ability, dma_addr, p_eccbuf);
}

static void
luna_spi_nand_reset(struct luna_nand_t *chip, uint32_t cs)
{
#if IS_ENABLED(CONFIG_RTK_SPI_NAND_GEN3)
  chip->_spi_nand_info->_reset(cs);
#else
  chip->_spi_nand_info->_reset();
#endif
}

static const char*
luna_spi_mtd_part_name(struct luna_nand_t *chip)
{
  static char *mtd_part_str = SPINAND_NAND_MTD_PARTS_NAME;
  return mtd_part_str;
}

static const char*
luna_spi_proc_name(struct luna_nand_t *chip)
{
	static const char *spi_proc_str = SPINAND_NAND_PROC_NAME;
	return spi_proc_str;
}


static uint8_t
luna_spi_get_status(struct luna_nand_t *chip)
{
#if IS_ENABLED(CONFIG_RTK_SPI_NAND_GEN3)
  return (nsc_get_feature_register(chip->selected_chip, 0xC0) | 0x80);
#else
  return (nsc_get_feature_register(0xC0) | 0x80);
#endif

}

static void
luna_spi_model_init (struct luna_nand_t *chip)
{
  chip->model->man_id          = chip->_spi_nand_info->man_id;
  chip->model->dev_id          = chip->_spi_nand_info->dev_id;
  chip->model->blk_num         = SNAF_NUM_OF_BLOCK(chip->_spi_nand_info);
  chip->model->pge_sz          = SNAF_PAGE_SIZE(chip->_spi_nand_info);
  chip->model->pge_num_per_blk = SNAF_NUM_OF_PAGE_PER_BLK(chip->_spi_nand_info);
  chip->model->spare_size      = SNAF_SPARE_SIZE(chip->_spi_nand_info);
  chip->model->oob_size        = SNAF_OOB_SIZE(chip->_spi_nand_info);
  chip->model->ecc_ability     = ECC_CORRECT_BITS(chip->_spi_nand_info);
#ifdef PGE_BLK_SHF
  chip->model->pge_blk_shift   = PGE_BLK_SHF(chip->_spi_nand_info->_num_block);
#endif
  printk("[%s:%d]\n\
      \tpage size is %d\n\
      \tman id is %x\n\
      \tdev id is %x\n\
      \tpage number per block %d\n\
      ", __FUNCTION__, __LINE__, chip->model->pge_sz,
      chip->model->man_id,
      chip->model->dev_id,
      chip->model->pge_num_per_blk);
#ifdef PGE_BLK_SHF
	printk("\tpage block shift is %d\n", chip->model->pge_blk_shift);
#endif
}


struct luna_nand_model_t luna_nand_spi_model = {
	._pio_read       = luna_spi_pio_read,
	._pio_write      = luna_spi_pio_write,
	._page_read      = luna_spi_page_read,
	._page_write     = luna_spi_page_write,
	._page_read_ecc  = luna_spi_page_read_ecc,
	._page_write_ecc = luna_spi_page_write_ecc,
	._block_erase    = luna_spi_erase_block,
	._ecc_encode     = luna_spi_ecc_encode,
	._ecc_decode     = luna_spi_ecc_decode,
	._reset          = luna_spi_nand_reset,
	._init           = luna_spi_model_init,
	._mtd_part_name  = luna_spi_mtd_part_name,
	._proc_name      = luna_spi_proc_name,
	._get_status     = luna_spi_get_status,

};

#endif

#if IS_ENABLED(CONFIG_MTD_ONFI_NAND_RTK) || IS_ENABLED(CONFIG_MTD_DUALIF_NAND_RTK)
static void
luna_onfi_pio_read(
	struct luna_nand_t *chip,
	void *addr,
	uint32_t wr_bytes,
	uint32_t blk_pge_addr,
	uint32_t col_addr
)
{
	chip->_onfi_nand_info->_model_info->_pio_read(chip->_onfi_nand_info, addr, wr_bytes, blk_pge_addr, col_addr);
}

static int32_t
luna_onfi_pio_write(
	struct luna_nand_t *chip,
	void *addr,
	uint32_t wr_bytes,
	uint32_t blk_pge_addr,
	uint32_t col_addr
)
{
	chip->_onfi_nand_info->_model_info->_pio_write(chip->_onfi_nand_info, addr, wr_bytes, blk_pge_addr, col_addr);
}

static void
luna_onfi_page_read(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr)
{
	chip->_onfi_nand_info->_model_info->_page_read(chip->_onfi_nand_info, dma_addr, blk_pge_addr);
}

static int32_t
luna_onfi_page_write(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr)
{
	return chip->_onfi_nand_info->_model_info->_page_write(chip->_onfi_nand_info, dma_addr, blk_pge_addr);
}

static int32_t
luna_onfi_page_read_ecc(
	struct luna_nand_t *chip,
	void *dma_addr,
	uint32_t blk_pge_addr,
	void *p_eccbuf
)
{
	return chip->_onfi_nand_info->_model_info->_page_read_ecc(chip->_onfi_nand_info, dma_addr, blk_pge_addr, p_eccbuf);
}

static int32_t
luna_onfi_page_write_ecc(struct luna_nand_t *chip, void *dma_addr, uint32_t blk_pge_addr, void *p_eccbuf)
{
	return chip->_onfi_nand_info->_model_info->_page_write_ecc(chip->_onfi_nand_info, dma_addr, blk_pge_addr, p_eccbuf);
}

static int32_t
luna_onfi_erase_block (struct luna_nand_t *chip, uint32_t blk_pge_addr)
{
	return chip->_onfi_nand_info->_model_info->_block_erase(chip->_onfi_nand_info, blk_pge_addr);
}

static void
luna_onfi_ecc_encode(struct luna_nand_t *chip, uint32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
  return chip->_onfi_nand_info->_ecc_encode(ecc_ability, dma_addr, p_eccbuf);
}

static int32_t
luna_onfi_ecc_decode(struct luna_nand_t *chip, uint32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
  return chip->_onfi_nand_info->_ecc_decode(ecc_ability, dma_addr, p_eccbuf);
}


static void
luna_onfi_nand_reset(struct luna_nand_t *chip, uint32_t cs)
{
  chip->_onfi_nand_info->_reset();
}

static const char*
luna_onfi_mtd_part_name(struct luna_nand_t *chip)
{
  static char *mtd_part_str = ONFI_NAND_MTD_PARTS_NAME;
  return mtd_part_str;
}

static const char*
luna_onfi_proc_name(struct luna_nand_t *chip)
{
	static char *onfi_proc_str = ONFI_NAND_PROC_NAME;
	return onfi_proc_str;
}

static uint8_t
luna_onfi_get_status(struct luna_nand_t *chip)
{
  return onfi_get_status_register();
}


static void
luna_onfi_model_init (struct luna_nand_t *chip)
{
  chip->model->man_id          = chip->_onfi_nand_info->man_id;
  chip->model->dev_id          = chip->_onfi_nand_info->dev_id;
  chip->model->blk_num         = ONFI_NUM_OF_BLOCK(chip->_onfi_nand_info);
  chip->model->pge_sz          = ONFI_PAGE_SIZE(chip->_onfi_nand_info);
  chip->model->pge_num_per_blk = ONFI_NUM_OF_PAGE_PER_BLK(chip->_onfi_nand_info);
  chip->model->spare_size      = ONFI_SPARE_SIZE(chip->_onfi_nand_info);
  chip->model->oob_size        = ONFI_OOB_SIZE(chip->_onfi_nand_info);
  chip->model->ecc_ability     = ECC_CORRECT_BITS(chip->_onfi_nand_info);
#ifdef PGE_BLK_SHF
  chip->model->pge_blk_shift   = PGE_BLK_SHF(chip->_spi_nand_info->_num_block);
#endif
  printk("[%s:%d]\n\
      \tpage size is %d\n\
      \tman id is %x\n\
      \tdev id is %x\n\
      \tpage number per block %d\n\
      \tpage block shift is %d\n", __FUNCTION__, __LINE__, chip->model->pge_sz,
      chip->model->man_id,
      chip->model->dev_id,
      chip->model->pge_num_per_blk,
      chip->model->pge_blk_shift);
}

struct luna_nand_model_t luna_nand_onfi_model = {
	._pio_read       = luna_onfi_pio_read,
	._pio_write      = luna_onfi_pio_write,
	._page_read      = luna_onfi_page_read,
	._page_write     = luna_onfi_page_write,
	._page_read_ecc  = luna_onfi_page_read_ecc,
	._page_write_ecc = luna_onfi_page_write_ecc,
	._block_erase    = luna_onfi_erase_block,
	._ecc_encode     = luna_onfi_ecc_encode,
	._ecc_decode     = luna_onfi_ecc_decode,
	._reset          = luna_onfi_nand_reset,
	._init           = luna_onfi_model_init,
	._mtd_part_name  = luna_onfi_mtd_part_name,
	._proc_name      = luna_onfi_proc_name,
	._get_status     = luna_onfi_get_status,
};

#endif
