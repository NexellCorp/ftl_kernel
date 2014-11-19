/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.readretry.c
 * Date         : 2014.08.29
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.25, TW.KIM)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#define __NFC_PHY_READRETRY_GLOBAL__
#include "nfc.phy.readretry.h"

#include "nfc.phy.h"


/******************************************************************************
 * to use: printf(), malloc(), memset(), free(), __TURE, __FALSE
 ******************************************************************************/
#include "../../../mio.definition.h"
#include "../../exchange.h"

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/cpufreq.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/gfp.h>

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <common.h>
#include <malloc.h>

#else
#error "nfc.phy.readretry.c: error! not defined build mode!"
#endif

/******************************************************************************
 *
 ******************************************************************************/
//#define DBG_PHY_READRETRY(fmt, args...) __PRINT(fmt, ##args)
#define DBG_PHY_READRETRY(fmt, args...)

/******************************************************************************
 * local
 ******************************************************************************/
static void NFC_PHY_HYNIX_READRETRY_OpenOtp(unsigned int _channel, unsigned int _phyway);
static void NFC_PHY_HYNIX_READRETRY_CloseOtp(unsigned int _channel, unsigned int _phyway);
static void NFC_PHY_HYNIX_READRETRY_ReadData(unsigned int _channel, unsigned int _phyway, void *_buf);

static int NFC_PHY_HYNIX_READRETRY_MakeReg(unsigned int _channel, unsigned int _phyway, void *_buf);
static void NFC_PHY_HYNIX_READRETRY_MajorityVote(void *_destbuf, void *_srcbuf, unsigned int entry_size, unsigned int entry_cnt);
static void NFC_PHY_HYNIX_READRETRY_MakeRegAddr(NAND_HYNIX_READRETRY_REG_ADDRESS *_reg_addr);
static int NFC_PHY_HYNIX_READRETRY_MakeRegData(NAND_HYNIX_READRETRY_REG_DATA *_reg_data, void *_majority_buf, const void *_otp_buf);

/******************************************************************************
 * extern functions
 ******************************************************************************/
int NFC_PHY_HYNIX_READRETRY_Init(unsigned int _max_channels, unsigned int _max_ways, const unsigned char *_way_map, unsigned char _readretry_type)
{
    int resp = 0;
    unsigned int max_channels = _max_channels;
    unsigned int max_ways = _max_ways;
    unsigned int way=0, channel=0;
    unsigned int size=0;
    unsigned int i=0;

    Exchange.nfc.fnReadRetry_MakeRegAll = NFC_PHY_HYNIX_READRETRY_MakeRegAll;
    Exchange.nfc.fnReadRetry_SetParameter = NFC_PHY_HYNIX_READRETRY_SetParameter;
    Exchange.nfc.fnReadRetry_GetTotalReadRetryCount = NFC_PHY_HYNIX_READRETRY_GetTotalReadRetryCount;
    Exchange.nfc.fnReadRetry_GetAddress = NFC_PHY_HYNIX_READRETRY_GetAddress;
    Exchange.nfc.fnReadRetry_GetRegDataAddress = NFC_PHY_HYNIX_READRETRY_GetRegDataAddress;
    Exchange.nfc.fnReadRetry_PrintTable = NFC_PHY_HYNIX_READRETRY_PrintTable;

    NFC_PHY_HYNIX_READRETRY_DeInit();

    hynix_readretry.readretry_type = _readretry_type;

    for (i=0; i < max_ways; i++)
    {
        hynix_readretry.phyway_map[i] = (_way_map)? _way_map[i]: 0;
    }

    size = max_ways * sizeof(NAND_HYNIX_READRETRY_REG_DATA *);

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    hynix_readretry.reg_data = (NAND_HYNIX_READRETRY_REG_DATA **)vmalloc(size);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    hynix_readretry.reg_data = (NAND_HYNIX_READRETRY_REG_DATA **)malloc(size);
#endif

    if (hynix_readretry.reg_data)
    {
        hynix_readretry.max_ways = max_ways;

        for (way = 0; way < max_ways; way++)
        {
            size = max_channels * sizeof(NAND_HYNIX_READRETRY_REG_DATA);

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
            hynix_readretry.reg_data[way] = (NAND_HYNIX_READRETRY_REG_DATA *)vmalloc(size);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
            hynix_readretry.reg_data[way] = (NAND_HYNIX_READRETRY_REG_DATA *)malloc(size);
#endif
            if (hynix_readretry.reg_data[way])
            {
                hynix_readretry.max_channels = max_channels;
                memset((void *)(hynix_readretry.reg_data[way]), 0x00, size);

                for (channel=0; channel < max_channels; channel++)
                {
                    hynix_readretry.reg_data[way][channel].this_size = sizeof(NAND_HYNIX_READRETRY_REG_DATA);
                    hynix_readretry.reg_data[way][channel].this_size_inverse = ~(hynix_readretry.reg_data[way][channel].this_size);
                }

            }
            else
            {
                DBG_PHY_READRETRY("NFC_PHY_HYNIX_READRETRY_Init: error! reg_data[%d]:0x%08x = malloc(%d)\n", way, (unsigned int)hynix_readretry.reg_data[way], size);
                resp = -1;
            }
        }

    }
    else
    {
        DBG_PHY_READRETRY("NFC_PHY_HYNIX_READRETRY_Init: error! reg_data:0x%08x = malloc(%d);\n", (unsigned int)hynix_readretry.reg_data, size);
        resp = -1;
    }

    if (resp >= 0)
    {
        hynix_readretry.this_signature[0] = 'R';
        hynix_readretry.this_signature[1] = 'T';
        hynix_readretry.this_signature[2] = 'R';
        hynix_readretry.this_signature[3] = 'Y';
    
        hynix_readretry.reg_addr.this_size = sizeof(hynix_readretry.reg_addr);
        hynix_readretry.reg_addr.this_size_inverse = ~(hynix_readretry.reg_addr.this_size);
        hynix_readretry.this_size = sizeof(hynix_readretry) - sizeof(NAND_HYNIX_READRETRY_REG_DATA **);
        hynix_readretry.this_size_inverse = ~(hynix_readretry.this_size);
    }

    return resp;
}

void NFC_PHY_HYNIX_READRETRY_DeInit(void)
{
    unsigned int max_ways = hynix_readretry.max_ways;
    unsigned int way=0;

    if (hynix_readretry.reg_data)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (hynix_readretry.reg_data[way])
            {
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
                vfree(hynix_readretry.reg_data[way]);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
                free(hynix_readretry.reg_data[way]);
#endif
                hynix_readretry.reg_data[way] = 0;
            }
            else
            {
                break;
            }
        }
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
        vfree(hynix_readretry.reg_data);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
        free(hynix_readretry.reg_data);
#endif
        hynix_readretry.reg_data = 0;
    }

}

void *NFC_PHY_HYNIX_READRETRY_GetAddress(void)
{
    return (void *)(&hynix_readretry);
}

void *NFC_PHY_HYNIX_READRETRY_GetRegDataAddress(unsigned int _channel, unsigned int _way)
{
    return (void *)(&(hynix_readretry.reg_data[_way][_channel]));
}

int NFC_PHY_HYNIX_READRETRY_GetTotalReadRetryCount(unsigned int _channel, unsigned int _way)
{
    return (int)hynix_readretry.reg_data[_way][_channel].total_readretry_cnt;
}

int NFC_PHY_HYNIX_READRETRY_MakeRegAll(void)
{
    int resp = -1;
    unsigned int max_channels = hynix_readretry.max_channels;
    unsigned int max_ways = hynix_readretry.max_ways;
    unsigned int channel=0, way=0, phyway=0;

    NAND_HYNIX_READRETRY_REG_ADDRESS *reg_addr = &hynix_readretry.reg_addr;
    NAND_HYNIX_READRETRY_REG_DATA *reg_data = 0;

    NFC_PHY_HYNIX_READRETRY_MakeRegAddr(reg_addr);

    for (way=0; way < max_ways; way++)
    {
        phyway = hynix_readretry.phyway_map[way];

        for(channel=0; channel < max_channels; channel++)
        {
            reg_data = &hynix_readretry.reg_data[way][channel];
            resp = NFC_PHY_HYNIX_READRETRY_MakeReg(channel, phyway, reg_data);
            if (resp < 0)
            {
                return -1;
            }
        }
    }

    return resp;
}

void NFC_PHY_HYNIX_READRETRY_SetParameter(unsigned int _channel, unsigned int _phyway)
{
    unsigned int channel = _channel;
    unsigned int way=0;
    unsigned int phyway = hynix_readretry.phyway_map[way];
    unsigned int reg_idx = 0;
    unsigned char addr=0, data=0;

    NAND_HYNIX_READRETRY_REG_ADDRESS *reg_addr = &hynix_readretry.reg_addr;
    NAND_HYNIX_READRETRY_REG_DATA *reg_data = 0;

    while(hynix_readretry.phyway_map[way] != phyway)
    {
        way += 1;
    }
    reg_data = &hynix_readretry.reg_data[way][channel];

    reg_data->curr_readretry_cnt++;
    if (reg_data->curr_readretry_cnt >= reg_data->total_readretry_cnt)
    {
        reg_data->curr_readretry_cnt = 0;
    }

    DBG_PHY_READRETRY(" ReadRetrySetParam(addr,data) - %08d: ", reg_data->curr_readretry_cnt);

    NFC_PHY_ChipSelect(channel, phyway, __TRUE);
    {
        NFC_PHY_Cmd(0x36);              // CMD:  0x36
        for (reg_idx=0; reg_idx < reg_data->readretry_reg_cnt; reg_idx++)
        {
            addr = reg_addr->addr[reg_idx];
            data = reg_data->table[reg_data->curr_readretry_cnt][reg_idx];

            NFC_PHY_Addr(addr);         // ADDR:
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(data);        // DATA:

            DBG_PHY_READRETRY("(%02x,%02x) ", addr, data);
        }
        NFC_PHY_Cmd(0x16);              // CMD:  0x16
    }
    NFC_PHY_ChipSelect(channel, phyway, __FALSE);

    DBG_PHY_READRETRY("\n");

}

/******************************************************************************
 * local functions
 ******************************************************************************/
void NFC_PHY_HYNIX_READRETRY_OpenOtp(unsigned int _channel, unsigned int _phyway)
{
    unsigned int channel = _channel;
    unsigned int phyway = _phyway;
    unsigned char readretry_type = hynix_readretry.readretry_type;

    NFC_PHY_NandReset(channel, phyway);

    NFC_PHY_ChipSelect(channel, phyway, __TRUE);
    {
        switch(readretry_type)
        {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
        {
            NFC_PHY_Cmd(0x36);              // CMD : 0x36
            NFC_PHY_Addr(0xFF);             // ADDR: 0xFF
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(0x40);            // DATA: 0x40
            NFC_PHY_Addr(0xCC);             // ADDR: 0xCC
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(0x4D);            // DATA: 0x4D
        } break;

        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        {
            NFC_PHY_Cmd(0x36);              // CMD : 0x36
            NFC_PHY_Addr(0xAE);             // ADDR: 0xAE
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(0x00);            // DATA: 0x00
            NFC_PHY_Addr(0xB0);             // ADDR: 0xB0
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(0x4D);            // DATA: 0x4D
        } break;

        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            NFC_PHY_Cmd(0x36);              // CMD : 0x36
            NFC_PHY_Addr(0x38);             // ADDR: 0xAE
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(0x52);            // DATA: 0x00
        } break;
        }

        NFC_PHY_Cmd(0x16);      // CMD:  0x16
        NFC_PHY_Cmd(0x17);      // CMD:  0x17
        NFC_PHY_Cmd(0x04);      // CMD:  0x04
        NFC_PHY_Cmd(0x19);      // CMD:  0x19
    }
    NFC_PHY_ChipSelect(channel, phyway, __FALSE);

}

void NFC_PHY_HYNIX_READRETRY_CloseOtp(unsigned int _channel, unsigned int _phyway)
{
    unsigned int channel = _channel;
    unsigned int phyway = _phyway;
    unsigned char readretry_type = hynix_readretry.readretry_type;

    NFC_PHY_NandReset(channel, phyway);

    NFC_PHY_ChipSelect(channel, phyway, __TRUE);
    {
        switch(readretry_type)
        {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        {
            NFC_PHY_Cmd(0x38);              // CMD:  0x38
        } break;

        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            NFC_PHY_Cmd(0x36);              // CMD:  0x36
            NFC_PHY_Addr(0x38);             // ADDR: 0x38
            NFC_PHY_tDelay(NfcTime.tADL);
            NFC_PHY_WData(0x00);            // DATA: 0x00
            NFC_PHY_Cmd(0x16);              // CMD:  0x16
            NFC_PHY_Cmd(0x00);              // CMD:  0x00
            NFC_PHY_Addr(0x00);             // ADDR: 0xXX
            NFC_PHY_Cmd(0x30);              // CMD:  0x30
        } break;
        }

        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
        while (!NFC_PHY_StatusIsRDY(NFC_PHY_RData()));
    }
    NFC_PHY_ChipSelect(channel, phyway, __FALSE);

  //NFC_PHY_NandReset(channel, way);

}

void NFC_PHY_HYNIX_READRETRY_ReadData(unsigned int _channel, unsigned int _phyway, void *_buf)
{
    unsigned int channel = _channel;
    unsigned int phyway = _phyway;
    unsigned char readretry_type = hynix_readretry.readretry_type;
    unsigned char *buf = (unsigned char *)_buf;
    unsigned int row = 0x000200;
    unsigned int readloop=0;
    unsigned int i=0;

    switch(readretry_type)
    {
    case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
    case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        readloop = 1026;
        break;
    case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        readloop = 528;
        break;
    default:
        return;
    }

    {
        unsigned char warn_prohibited_block_access = Exchange.debug.nfc.phy.warn_prohibited_block_access;
    
        Exchange.debug.nfc.phy.warn_prohibited_block_access = 0;
        NFC_PHY_1stRead(channel, phyway, row, 0);
        Exchange.debug.nfc.phy.warn_prohibited_block_access = warn_prohibited_block_access;
    }

    NFC_PHY_ChipSelect(channel, phyway, __TRUE);
    {
        NFC_PHY_Cmd(NF_CMD_READ_STATUS);
        NFC_PHY_tDelay(NfcTime.tWHR);
        while (!NFC_PHY_StatusIsRDY(NFC_PHY_RData()));
        NFC_PHY_Cmd(NF_CMD_READ_1ST);
        NFC_PHY_tDelay(NfcTime.tWHR);

        for (i=0; i < readloop; i++)
        {
            *buf = NFC_PHY_RData();
            buf++;
        }
    }
    NFC_PHY_ChipSelect(channel, phyway, __FALSE);

}

void NFC_PHY_HYNIX_READRETRY_MajorityVote(void *_destbuf, void *_srcbuf, unsigned int entry_size, unsigned int entry_cnt)
{
    unsigned char *destbuf = (unsigned char *)_destbuf;
    unsigned char *srcbuf = (unsigned char *)_srcbuf;
    unsigned int entry_idx=0, byte_idx=0;
    unsigned char bit_idx=0;
    unsigned char cnt_of_setbit[8] = {0,};
    unsigned char value=0;

    memset((void *)destbuf, 0x00, entry_size);

    for (byte_idx=0; byte_idx < entry_size; byte_idx++)
    {
        for (bit_idx=0; bit_idx < 8; bit_idx++)
        {
            for (entry_idx=0; entry_idx < entry_cnt; entry_idx++)
            {
                value = srcbuf[entry_idx * entry_size + byte_idx];
                if (value & (1 << bit_idx))
                {
                    cnt_of_setbit[bit_idx]++;
                    if (cnt_of_setbit[bit_idx] > (entry_cnt / 2))
                    {
                        destbuf[byte_idx] |= (1 << bit_idx);
                        break;
                    }
                }
            }
        }
    }
}

void NFC_PHY_HYNIX_READRETRY_MakeRegAddr(NAND_HYNIX_READRETRY_REG_ADDRESS *_reg_addr)
{
    unsigned char readretry_type = hynix_readretry.readretry_type;
    unsigned char reg_idx = 0;

    NAND_HYNIX_READRETRY_REG_ADDRESS *reg_addr = _reg_addr;

    memset((void *)reg_addr, 0x00, sizeof(*reg_addr));
    reg_addr->this_size = sizeof(*reg_addr);
    reg_addr->this_size_inverse = ~(reg_addr->this_size);
    switch (readretry_type)
    {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
        {
            reg_addr->addr[reg_idx++] = 0xCC;
            reg_addr->addr[reg_idx++] = 0xBF;
            reg_addr->addr[reg_idx++] = 0xAA;
            reg_addr->addr[reg_idx++] = 0xAB;
            reg_addr->addr[reg_idx++] = 0xCD;
            reg_addr->addr[reg_idx++] = 0xAD;
            reg_addr->addr[reg_idx++] = 0xAE;
            reg_addr->addr[reg_idx++] = 0xAF;
        } break;

        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        {
            reg_addr->addr[reg_idx++] = 0xB0;
            reg_addr->addr[reg_idx++] = 0xB1;
            reg_addr->addr[reg_idx++] = 0xB2;
            reg_addr->addr[reg_idx++] = 0xB3;
            reg_addr->addr[reg_idx++] = 0xB4;
            reg_addr->addr[reg_idx++] = 0xB5;
            reg_addr->addr[reg_idx++] = 0xB6;
            reg_addr->addr[reg_idx++] = 0xB7;
            reg_addr->addr[reg_idx++] = 0xD4;
            reg_addr->addr[reg_idx++] = 0xD5;
        } break;

        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            reg_addr->addr[reg_idx++] = 0x38;
            reg_addr->addr[reg_idx++] = 0x39;
            reg_addr->addr[reg_idx++] = 0x3A;
            reg_addr->addr[reg_idx++] = 0x3B;
        } break;
    }

}

int NFC_PHY_HYNIX_READRETRY_MakeRegData(NAND_HYNIX_READRETRY_REG_DATA *_reg_data, void *_majority_buf, const void *_otp_buf)
{
    unsigned char readretry_type = hynix_readretry.readretry_type;
    unsigned char *majority_buf = (unsigned char *)_majority_buf;
    unsigned char *otp_buf = (unsigned char *)_otp_buf;
    const unsigned char MAX_RRT_COPYS = 8;
    unsigned int entry_size=0, entry_srtart_ofs=0;
    unsigned int entry_idx=0, data_idx=0, inverse_idx=0, byte_idx=0, rr_idx=0, reg_idx=0;
    NAND_HYNIX_READRETRY_REG_DATA *reg_data = (NAND_HYNIX_READRETRY_REG_DATA *)_reg_data;

    switch (readretry_type)
    {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:  { reg_idx =  8; } break;
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE: { reg_idx = 10; } break;
        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:        { reg_idx =  4; } break;
    }
    memset((void *)reg_data, 0x00, sizeof(*reg_data));
    reg_data->this_size = sizeof(*reg_data);
    reg_data->this_size_inverse = ~(reg_data->this_size);

    switch (readretry_type)
    {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        {
            reg_data->total_readretry_cnt = otp_buf[0];
            reg_data->readretry_reg_cnt = otp_buf[1];
          //reg_data->total_readretry_cnt = 8;
          //reg_data->readretry_reg_cnt = 8;

            entry_srtart_ofs = 2;
        } break;

        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            // Make the header of RRT
            NFC_PHY_HYNIX_READRETRY_MajorityVote(majority_buf, otp_buf, 1, 8);
            reg_data->total_readretry_cnt = majority_buf[0];
            NFC_PHY_HYNIX_READRETRY_MajorityVote(majority_buf, otp_buf+8, 1, 8);
            reg_data->readretry_reg_cnt = majority_buf[0];
            entry_srtart_ofs = 8 + 8;
        } break;

        default: { return -1; }
    }

    if (!reg_data->total_readretry_cnt || (reg_data->total_readretry_cnt > NAND_PHY_HYNIX_READRETRY_TOTAL_CNT) ||
        !reg_data->readretry_reg_cnt || (reg_data->readretry_reg_cnt > NAND_PHY_HYNIX_READRETRY_REG_CNT) ||
        (reg_data->readretry_reg_cnt > reg_idx))
    {
        DBG_PHY_READRETRY("NFC_PHY_HYNIX_READRETRY_MakeRegData: error! reg_data->total_readretry_cnt:%d, reg_data->readretry_reg_cnt:%d, reg_idx:%d \n",  reg_data->total_readretry_cnt, reg_data->readretry_reg_cnt, reg_idx);

        memset((void *)reg_data, 0x00, sizeof(*reg_data));
        reg_data->this_size = sizeof(*reg_data);
        reg_data->this_size_inverse = ~(reg_data->this_size);

        return -1;
    }

    switch (readretry_type)
    {
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
        case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        {
            // set data of RRT with it's inverse
            entry_size = reg_data->total_readretry_cnt * reg_data->readretry_reg_cnt * 2;

            for(entry_idx=0; entry_idx < MAX_RRT_COPYS; entry_idx++)
            {
                data_idx = (entry_idx * entry_size) + entry_srtart_ofs;
                inverse_idx = data_idx + (entry_size / 2);

                for (byte_idx=0; byte_idx < (entry_size/2); byte_idx++)
                {
                    if (otp_buf[data_idx + byte_idx] + otp_buf[inverse_idx + byte_idx] == 0xFF)
                    {
                        rr_idx = byte_idx / reg_data->readretry_reg_cnt;
                        reg_idx = byte_idx % reg_data->readretry_reg_cnt;
                        if (!reg_data->table[rr_idx][reg_idx])
                        {
                            reg_data->table[rr_idx][reg_idx] = otp_buf[data_idx + byte_idx];
                        }
                    }
                }
            }

            // Majority vote for empty data of RRT
            NFC_PHY_HYNIX_READRETRY_MajorityVote(majority_buf+entry_srtart_ofs, otp_buf+entry_srtart_ofs, entry_size, MAX_RRT_COPYS);
            for(rr_idx=0; rr_idx < reg_data->total_readretry_cnt; rr_idx++)
            {
                for (reg_idx=0; reg_idx < reg_data->readretry_reg_cnt; reg_idx++)
                {
                    if (!reg_data->table[rr_idx][reg_idx])
                    {
                        reg_data->table[rr_idx][reg_idx] = (majority_buf+entry_srtart_ofs)[(rr_idx * reg_data->readretry_reg_cnt) + reg_idx];
                    }
                }
            }
        } break;

        case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            entry_size = reg_data->total_readretry_cnt * reg_data->readretry_reg_cnt * 2;

            // Majority vote for empty data of RRT
            NFC_PHY_HYNIX_READRETRY_MajorityVote(majority_buf+entry_srtart_ofs, otp_buf+entry_srtart_ofs, entry_size, MAX_RRT_COPYS);
            for(rr_idx=0; rr_idx < reg_data->total_readretry_cnt; rr_idx++)
            {
                for (reg_idx=0; reg_idx < reg_data->readretry_reg_cnt; reg_idx++)
                {
                    reg_data->table[rr_idx][reg_idx] = (majority_buf+entry_srtart_ofs)[(rr_idx * reg_data->readretry_reg_cnt) + reg_idx];
                }
            }
        } break;
    }

    return 0;

}

int NFC_PHY_HYNIX_READRETRY_MakeReg(unsigned int _channel, unsigned int _phyway, void *_buf)
{
    int resp = -1;
    unsigned int channel = _channel;
    unsigned int phyway = _phyway;
    unsigned char readretry_type = hynix_readretry.readretry_type;
    unsigned char *otp_buf=0;
    unsigned char *majority_buf=0;
    unsigned int otp_size = 0;
    unsigned int majority_size = NAND_PHY_HYNIX_READRETRY_TOTAL_CNT*NAND_PHY_HYNIX_READRETRY_REG_CNT*2;
    NAND_HYNIX_READRETRY_REG_DATA *reg_data = (NAND_HYNIX_READRETRY_REG_DATA *)_buf;

    if (!reg_data)
        return -1;

    switch(readretry_type)
    {
    case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
    case NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
        otp_size = 1026;
        break;
    case NAND_READRETRY_TYPE_HYNIX_1xNM_MLC:
        otp_size = 528;
        break;
    default:
        return -1;
    }

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    otp_buf = (unsigned char *)vmalloc(otp_size);
    majority_buf = (unsigned char *)vmalloc(majority_size);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    otp_buf = (unsigned char *)malloc(otp_size);
    majority_buf = (unsigned char *)malloc(majority_size);
#endif

    if (otp_buf && majority_buf)
    {
        NFC_PHY_HYNIX_READRETRY_OpenOtp(channel, phyway);
        NFC_PHY_HYNIX_READRETRY_ReadData(channel, phyway, otp_buf);
        NFC_PHY_HYNIX_READRETRY_CloseOtp(channel, phyway);

        resp = NFC_PHY_HYNIX_READRETRY_MakeRegData(reg_data, majority_buf, otp_buf);

        if (Exchange.debug.ftl.read_retry)
        {
            int i=0;

            DBG_PHY_READRETRY(" ##################################################\n");
            DBG_PHY_READRETRY(" #       otp data for HYNIX READ RETRY TABLE       ");

            for (i=0; i < otp_size; i++)
            {
                if (!(i%32))
                {
                    DBG_PHY_READRETRY("\n # [0x%03X]", i);
                }
                DBG_PHY_READRETRY("%02x ", otp_buf[i]);
            }
            DBG_PHY_READRETRY("\n");
            DBG_PHY_READRETRY(" ##################################################\n");
        }

        NFC_PHY_HYNIX_READRETRY_PrintTable();
    }
    else
    {
        DBG_PHY_READRETRY("NFC_PHY_HYNIX_READRETRY_Make: error! otp_buf:0x%08x, majority_buff:0x%08x\n", (unsigned int)otp_buf, (unsigned int)majority_buf);
    }

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
    if (otp_buf)
        vfree(otp_buf);
    if (majority_buf)
        vfree(majority_buf);
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
    if (otp_buf)
        free(otp_buf);
    if (majority_buf)
        free(majority_buf);
#endif

    return resp;
}


void NFC_PHY_HYNIX_READRETRY_PrintTable(void)
{
    if (Exchange.debug.ftl.boot_read_retry)
    {
        int i = 0;
        int j = 0;

        DBG_PHY_READRETRY(" ##################################################\n");
        DBG_PHY_READRETRY(" #             HYNIX READ RETRY TABLE              \n");
        DBG_PHY_READRETRY(" # max_channels:%d, max_ways:%d\n", hynix_readretry.max_channels, hynix_readretry.max_ways);
        DBG_PHY_READRETRY(" # reg_addr.addr: ");
        for (i=0; i < NAND_PHY_HYNIX_READRETRY_REG_CNT; i++)
            DBG_PHY_READRETRY("%02x ", hynix_readretry.reg_addr.addr[i]);
        DBG_PHY_READRETRY("\n");

        for (i=0; i < hynix_readretry.max_channels; i++)
        {
            for (j=0; j < hynix_readretry.max_ways; j++)
            {
                NAND_HYNIX_READRETRY_REG_DATA *reg_data = &hynix_readretry.reg_data[j][i];
                int m=0, n=0;

                DBG_PHY_READRETRY(" #\n");
                DBG_PHY_READRETRY(" # table of channel(%d), way(%d)\n", i, j);
                DBG_PHY_READRETRY(" #  total_readretry_cnt:%d, readretry_reg_cnt:%d, curr_readretry_cnt:%d\n", reg_data->total_readretry_cnt, reg_data->readretry_reg_cnt, reg_data->curr_readretry_cnt);
                DBG_PHY_READRETRY(" #           reg1 reg2 reg3 reg4 reg5 reg6 reg7 reg8 reg9 reg10\n");
                for (m=0; m < reg_data->total_readretry_cnt; m++)
                {
                    DBG_PHY_READRETRY(" #  [Step%2d]", m);
                    for (n=0; n < reg_data->readretry_reg_cnt; n++)
                    {
                        DBG_PHY_READRETRY("  %02x ", reg_data->table[m][n]);
                    }
                    DBG_PHY_READRETRY("\n");
                }
            }
        }
        DBG_PHY_READRETRY(" ##################################################\n");
    }
}



