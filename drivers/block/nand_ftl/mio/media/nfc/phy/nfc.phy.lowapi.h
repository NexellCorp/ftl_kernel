/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.lowapi.h
 * Date         : 2014.08.25
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.25, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_LOWAPI_GLOBAL__
#define NFC_PHY_LOWAPI_EXT
#else
#define NFC_PHY_LOWAPI_EXT extern
#endif

#include <linux/types.h>

typedef struct __MIO_NAND_INFO__
{
	unsigned char channel;
	unsigned char phyway;
	unsigned short pages_per_block;
	unsigned short bytes_per_page;
	unsigned short blocks_per_lun;

	unsigned short ecc_bits;
	unsigned short bytes_per_ecc;
	unsigned short bytes_per_parity;

	unsigned int readretry_type;

} MIO_NAND_INFO;

/******************************************************************************
 *
 ******************************************************************************/

NFC_PHY_LOWAPI_EXT int NFC_PHY_LOWAPI_nand_write(const MIO_NAND_INFO *info, loff_t ofs, size_t *len, u_char *buf);
NFC_PHY_LOWAPI_EXT int NFC_PHY_LOWAPI_nand_read(const MIO_NAND_INFO *info, loff_t ofs, size_t *len, u_char *buf);
NFC_PHY_LOWAPI_EXT int NFC_PHY_LOWAPI_nand_erase(const MIO_NAND_INFO *info, loff_t ofs, size_t size);

NFC_PHY_LOWAPI_EXT int NFC_PHY_LOWAPI_nand_raw_write(const MIO_NAND_INFO *info, loff_t ofs, size_t *len, u_char *buf);
NFC_PHY_LOWAPI_EXT int NFC_PHY_LOWAPI_nand_raw_read(const MIO_NAND_INFO *info, loff_t ofs, size_t *len, u_char *buf);
