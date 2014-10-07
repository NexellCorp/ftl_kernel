/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.readretry.h
 * Date         : 2014.08.29
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.25, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#pragma once

#ifdef __NFC_PHY_READRETRY_GLOBAL__
#define NFC_PHY_READRETRY_EXT
#else
#define NFC_PHY_READRETRY_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
#define NAND_PHY_HYNIX_READRETRY_TOTAL_CNT	    (12)
#define NAND_PHY_HYNIX_READRETRY_REG_CNT	    (10)

typedef struct _NAND_HYNIX_READRETRY_REG_ADDRESS_
{
    unsigned int size_of_this;

    unsigned char addr[NAND_PHY_HYNIX_READRETRY_REG_CNT];
    unsigned char reserved[4 - NAND_PHY_HYNIX_READRETRY_REG_CNT%4]; // padding for 4 bytes align

}NAND_HYNIX_READRETRY_REG_ADDRESS;

typedef struct _NAND_HYNIX_READRETRY_REG_DATA_
{
    unsigned int size_of_this;

    unsigned char total_readretry_cnt;
    unsigned char readretry_reg_cnt;
    unsigned char curr_readretry_cnt;
    unsigned char reserved0;

    unsigned char table[NAND_PHY_HYNIX_READRETRY_TOTAL_CNT][NAND_PHY_HYNIX_READRETRY_REG_CNT];
    unsigned char reserved1[4 - (NAND_PHY_HYNIX_READRETRY_TOTAL_CNT*NAND_PHY_HYNIX_READRETRY_REG_CNT)%4]; // padding for 4 bytes align

}NAND_HYNIX_READRETRY_REG_DATA;

typedef struct _NAND_HYNIX_READRETRY_
{
    unsigned int size_of_this;

    unsigned char max_channels;
    unsigned char max_ways;
    unsigned char phyway_map[8];

    unsigned char readretry_type;
    unsigned char reserved;

    NAND_HYNIX_READRETRY_REG_ADDRESS reg_addr;
    NAND_HYNIX_READRETRY_REG_DATA **reg_data; // [max_ways][max_channels]

} NAND_HYNIX_READRETRY;

/******************************************************************************
 * Global variables
 ******************************************************************************/
NFC_PHY_READRETRY_EXT NAND_HYNIX_READRETRY hynix_readretry;

/******************************************************************************
 * Global functions
 ******************************************************************************/
NFC_PHY_READRETRY_EXT int NFC_PHY_HYNIX_READRETRY_Init(unsigned int _max_channels, unsigned int _max_ways, const unsigned char *_way_map, unsigned char _readretry_type);
NFC_PHY_READRETRY_EXT void NFC_PHY_HYNIX_READRETRY_DeInit(void);
NFC_PHY_READRETRY_EXT int NFC_PHY_HYNIX_READRETRY_MakeRegAll(void);

NFC_PHY_READRETRY_EXT void NFC_PHY_HYNIX_READRETRY_SetParameter(unsigned int _channel, unsigned int _phyway);
NFC_PHY_READRETRY_EXT int NFC_PHY_HYNIX_READRETRY_GetTotalReadRetryCount(unsigned int _channel, unsigned int _way);

NFC_PHY_READRETRY_EXT void *NFC_PHY_HYNIX_READRETRY_GetAddress(void);
NFC_PHY_READRETRY_EXT void *NFC_PHY_HYNIX_READRETRY_GetRegDataAddress(unsigned int _channel, unsigned int _way);

NFC_PHY_READRETRY_EXT void NFC_PHY_HYNIX_READRETRY_PrintTable(void);
