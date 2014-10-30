/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.smart.c
 * Date         : 2014.10.23
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.10.23)
 *
 * Description  :
 *
 ******************************************************************************/
#define __MIO_SMART_GLOBAL__
#include "mio.smart.h"

#include "mio.definition.h"
#include "media/exchange.h"
#include "mio.block.h"
#include "media/nfc/phy/nfc.phy.h"

#define DBG_MIOSMART(fmt, args...) __PRINT(fmt, ##args)
//#define DBG_MIOSMART(fmt, args...)

/******************************************************************************
 * local variable
 ******************************************************************************/
static struct
{
    unsigned char *rwbuff;
    unsigned int rwbuff_size;
    
    unsigned int diff_cnt;

    unsigned int adminlow_size;
    unsigned int adminmiddle_size;
    unsigned int adminhigh_size;

    NAND *nand;

} miosmart;

/******************************************************************************
 * local functions
 ******************************************************************************/
static int miosmart_writeadmin(unsigned char region, void *buff, unsigned int sectors);
static int miosmart_readadmin(unsigned char region, void * buff, unsigned int sectors, unsigned char sequent);

/******************************************************************************
 * extern functions
 ******************************************************************************/
int miosmart_init(unsigned int _max_channels, unsigned int _max_ways)
{
    int resp = 0;
    unsigned int max_channels = _max_channels;
    unsigned int max_ways = _max_ways;
    unsigned int way=0, channel=0;
    unsigned int size=0;
    unsigned int page_size = 0;
    
    miosmart_deinit();
    miosmart.nand = &phy_features.nand_config;

    // io_current, io_accumulate
    MioSmartInfo.io_accumulate.this_size = sizeof(MIO_SMART_COMMON_DATA);
    MioSmartInfo.io_current.this_size = sizeof(MIO_SMART_COMMON_DATA);

    // nand_current, nand_accumulate
    size = max_ways * sizeof(MIO_SMART_CE_DATA *);
    MioSmartInfo.nand_accumulate = (MIO_SMART_CE_DATA **)vmalloc(size);
    MioSmartInfo.nand_current = (MIO_SMART_CE_DATA **)vmalloc(size);

    if (MioSmartInfo.nand_accumulate && MioSmartInfo.nand_current)
    {
        MioSmartInfo.max_ways = max_ways;

        for (way = 0; way < max_ways; way++)
        {
            size = max_channels * sizeof(MIO_SMART_CE_DATA);

            MioSmartInfo.nand_accumulate[way] = (MIO_SMART_CE_DATA *)vmalloc(size);
            MioSmartInfo.nand_current[way] = (MIO_SMART_CE_DATA *)vmalloc(size);

            if (MioSmartInfo.nand_accumulate[way] && MioSmartInfo.nand_current[way])
            {
                MioSmartInfo.max_channels = max_channels;
                memset((void *)(MioSmartInfo.nand_accumulate[way]), 0x00, size);
                memset((void *)(MioSmartInfo.nand_current[way]), 0x00, size);

                for (channel=0; channel < max_channels; channel++)
                {
                    MioSmartInfo.nand_accumulate[way][channel].this_size = sizeof(MIO_SMART_CE_DATA);
                    MioSmartInfo.nand_current[way][channel].this_size = sizeof(MIO_SMART_CE_DATA);
                }
            }
            else
            {
              //DBG_MIOSMART(KERN_WARNING "miosmart_init: memory alloc: fail");
                resp = -1;
            }
        }
    }
    else
    {
      //DBG_MIOSMART(KERN_WARNING "miosmart_init: memory alloc: fail");
        resp = -1;
    }

#if 0
    // wearlevel_data
    size = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun * sizeof(unsigned int);
    MioSmartInfo.wearlevel_data = (unsigned int *)vmalloc(size);
    if (!MioSmartInfo.wearlevel_data)
    {
      //DBG_MIOSMART(KERN_INFO "mio.sys: wearlevel: memory alloc: fail");
        resp = -1;
    }
#endif

#if 0
    // load/save
    page_size = (miosmart.nand->_f.databytes_per_page)? miosmart.nand->_f.databytes_per_page: 8*1024;
    miosmart.adminlow_size = page_size;
    miosmart.adminmiddle_size = page_size;
    miosmart.adminhigh_size = page_size;

    miosmart.rwbuff = (unsigned char *)vmalloc(page_size);
    if (miosmart.rwbuff)
    {
        miosmart.rwbuff_size = page_size;
    }
    else
    {
      //DBG_MIOSMART(KERN_WARNING "miosmart_init: memory alloc: fail");
        resp = -1;
    }
#endif

    return resp;
}

void miosmart_deinit(void)
{
    unsigned int max_ways = MioSmartInfo.max_ways;
    unsigned int way=0;

    // nand_accumulate
    if (MioSmartInfo.nand_accumulate)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (MioSmartInfo.nand_accumulate[way])
            {
                vfree(MioSmartInfo.nand_accumulate[way]);
                MioSmartInfo.nand_accumulate[way] = 0;
            }
            else
            {
                break;
            }
        }
        vfree(MioSmartInfo.nand_accumulate);
        MioSmartInfo.nand_accumulate = 0;
    }

    // nand_current
    if (MioSmartInfo.nand_current)
    {
        for (way = 0; way < max_ways; way++)
        {
            if (MioSmartInfo.nand_current[way])
            {
                vfree(MioSmartInfo.nand_current[way]);
                MioSmartInfo.nand_current[way] = 0;
            }
            else
            {
                break;
            }
        }
        vfree(MioSmartInfo.nand_current);
        MioSmartInfo.nand_current = 0;
    }

#if 0
    // wearlevel_data
    if (MioSmartInfo.wearlevel_data)
    {
        vfree(MioSmartInfo.wearlevel_data);
        MioSmartInfo.wearlevel_data = 0;
    }
#endif

#if 0
    // load/save
    if (miosmart.rwbuff)
    {
        vfree(miosmart.rwbuff);
        miosmart.rwbuff = 0;
    }
#endif
}

int miosmart_update(void)
{
    int diff_cnt = 0;
    unsigned int current_new = 0;
    unsigned int *pcurrent = 0;
    unsigned int *paccumulate = 0;
    MIO_SMART_CE_DATA *pnand_accumulate = 0;
    MIO_SMART_CE_DATA *pnand_current = 0;
    unsigned int channel = 0, way = 0;

    // read retry count
    // ...
    // ...

    // ECC info
    for (way=0; way < *Exchange.ftl.Way; way++)
    {
        for (channel=0; channel < *Exchange.ftl.Channel; channel++)
        {
            pnand_accumulate = &(MioSmartInfo.nand_accumulate[way][channel]);
            pnand_current = &(MioSmartInfo.nand_current[way][channel]);

            if (!pnand_accumulate || !pnand_current)
            {
                return -1;
            }

            // ECC corrected
            current_new = Exchange.statistics.ecc_sector.corrected[way][channel];
            pcurrent    = &(pnand_current->ecc_sector.corrected);
            paccumulate = &(pnand_accumulate->ecc_sector.corrected);
            if (*pcurrent < current_new)
            {
                *paccumulate = current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // ECC leveldetected
            current_new = Exchange.statistics.ecc_sector.leveldetected[way][channel];
            pcurrent    = &(pnand_current->ecc_sector.leveldetected);
            paccumulate = &(pnand_accumulate->ecc_sector.leveldetected);
            if (*pcurrent < current_new)
            {
                *paccumulate = current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // ECC uncorrectable
            current_new = Exchange.statistics.ecc_sector.uncorrectable[way][channel];
            pcurrent    = &(pnand_current->ecc_sector.uncorrectable);
            paccumulate = &(pnand_accumulate->ecc_sector.uncorrectable);
            if (*pcurrent < current_new)
            {
                *paccumulate = current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

            // Read retry count
            current_new = Exchange.statistics.readretry_count[way][channel];
            pcurrent    = &(pnand_current->readretry_count);
            paccumulate = &(pnand_accumulate->readretry_count);
            if (*pcurrent < current_new)
            {
                *paccumulate = current_new - *pcurrent;
                *pcurrent = current_new;
                diff_cnt += 1;
            }

        }
    }

    miosmart.diff_cnt = diff_cnt;
    return diff_cnt;
}

int miosmart_load(void)
{
#if 0
    void * src_buff = 0;
    unsigned int region = 1;
    unsigned int region_size = miosmart.adminhigh_size;
    unsigned char try_count = 0;
    MIO_SMART_COMMON_DATA *io_data;
    MIO_SMART_CE_DATA     *nand_data;

    for (try_count=0; try_count < 2; try_count++)
    {
        memset((void *)miosmart.rwbuff, 0, miosmart.rwbuff_size);
        if (miosmart_readadmin(region, (void *)miosmart.rwbuff, region_size, try_count) < 0)
        {
            continue;
        }

        src_buff = (void *)miosmart.rwbuff;

        // io_accumulate
        io_data = (MIO_SMART_COMMON_DATA *)&MioSmartInfo.io_accumulate;
        if (io_data->crc32 != Exchange.std.__get_crc32(0, (void *)(src_buff), (io_data->this_size - 4)))
        {
            continue;
        }
        memcpy((void *)io_data, src_buff, io_data->this_size);
        src_buff += io_data->this_size;

        // nand_accumulate
        for (way = 0; way < MioSmartInfo.max_ways; way++)
        {
            for (channel = 0; channel < MioSmartInfo.max_channels; channel++)
            {
                nand_data = (MIO_SMART_CE_DATA *)&MioSmartInfo.nand_accumulate[way][channel];

                if (nand_data->crc32 != Exchange.std.__get_crc32(0, (void *)nand_data, (nand_data->this_size - 4)))
                {
                    continue;
                }
                
                memcpy((void *)nand_data, src_buff, nand_data->this_size);
                src_buff += nand_data->this_size;
            }
        }


        // add here
        // ..

        break;
    }
#endif

    miosmart.diff_cnt = 0;
    return 0;
}

int miosmart_save(void)
{
    int resp = -1;
    void * dest_buff = 0;
    unsigned char try_count = 0;
    unsigned char region = 1;
    unsigned int region_size = miosmart.adminhigh_size;
	unsigned char channel = 0, way = 0;
    MIO_SMART_COMMON_DATA *io_data;
    MIO_SMART_CE_DATA     *nand_data;
	
    if (!miosmart.diff_cnt)
    {
        return 0;
    }

    /**********************************************************************
     * Construct Data
     **********************************************************************/
    memset((void *)miosmart.rwbuff, 0, miosmart.rwbuff_size);
    {
        dest_buff = (void *)miosmart.rwbuff;

        // io_accumulate
        io_data = (MIO_SMART_COMMON_DATA *)&MioSmartInfo.io_accumulate;
        
        io_data->crc32 = Exchange.std.__get_crc32(0, (void *)io_data, (io_data->this_size - 4));
        memcpy(dest_buff, (void *)io_data, io_data->this_size);
        dest_buff += io_data->this_size;

        // nand_accumulate
        for (way = 0; way < MioSmartInfo.max_ways; way++)
        {
            for (channel = 0; channel < MioSmartInfo.max_channels; channel++)
            {
                nand_data = (MIO_SMART_CE_DATA *)&MioSmartInfo.nand_accumulate[way][channel];

                nand_data->crc32 = Exchange.std.__get_crc32(0, (void *)nand_data, (nand_data->this_size - 4));
                memcpy(dest_buff, (void *)nand_data, nand_data->this_size);
                dest_buff += nand_data->this_size;
            }
        }
    }

    /**********************************************************************
     * Write Condition Data
     **********************************************************************/
    for (try_count=0; try_count < 2; try_count++)
    {
        if (miosmart_writeadmin(region, (void *)miosmart.rwbuff, region_size) < 0)
        {
            continue;
        }

        memset((void *)miosmart.rwbuff, 0, region_size);
        if (miosmart_readadmin(region, (void *)miosmart.rwbuff, region_size, 0) < 0)
        {
            continue;
        }

        dest_buff = (void *)miosmart.rwbuff;
        if (MioSmartInfo.io_accumulate.crc32 != Exchange.std.__get_crc32(0, (void *)(dest_buff), (MioSmartInfo.io_accumulate.this_size - 4)))
        {
            continue;
        }

        // add here
        // ...
        
    }

  //gstAta.RapidUpdateHigh = 0;

    miosmart.diff_cnt = 0;
    return resp;
}

/******************************************************************************
 * local functions
 ******************************************************************************/
int miosmart_writeadmin(unsigned char region, void *buff, unsigned int sectors)
{
    int resp = -1;
    unsigned char sequent = 0;
    unsigned int address = 0, feature = 0;
    int ext_index = 0;
    unsigned int page_size = __POW(16,10);   // 16 KB

    if (region) // admin (update high/middle)
    {
        unsigned int *dest_buff = 0;
        unsigned int ofs_adminhigh = 0;
        unsigned int ofs_adminmiddle = 0;
        unsigned int addr_adminhigh[2] = {0,0};
        unsigned int addr_adminmiddle[2] = {0,0};

        ofs_adminhigh = ((miosmart.adminhigh_size + page_size - 1) / page_size) * page_size;
        ofs_adminmiddle = ((miosmart.adminmiddle_size + page_size - 1) / page_size) * page_size;

        addr_adminhigh[0] = *Exchange.buffer.BaseOfAdmin2;
        addr_adminhigh[1] = addr_adminhigh[0] + __SECTOR_OF_BYTE(ofs_adminhigh);
        addr_adminmiddle[0] = addr_adminhigh[1] + __SECTOR_OF_BYTE(ofs_adminhigh);
        addr_adminmiddle[1] = addr_adminmiddle[0] + __SECTOR_OF_BYTE(ofs_adminmiddle);

        feature = MIOSMART_PART_ADMIN2;

        // write to NAND
        for (sequent=0; sequent < 2; sequent++)
        {
            switch (region)
            {
                case 1: { address = addr_adminhigh[sequent]; } break;
                case 2: { address = addr_adminmiddle[sequent]; } break;
            }

            while(1)
            {
                Exchange.ftl.fnMain();

                resp = Exchange.ftl.fnPrePutCommand(IO_CMD_WRITE_DIRECT, __POW(feature,0), address, sectors);
                if (resp >= 0)
                {
                    ext_index = resp;
                    resp = Exchange.ftl.fnPutCommand(IO_CMD_WRITE_DIRECT, __POW(feature,0), address, sectors);
                    if (resp >= 0)
                    {
                        dest_buff = (unsigned int*)(*Exchange.buffer.BaseOfDirectWriteCache + __SECTOR_SIZEOF(ext_index));
                        memcpy (dest_buff, buff, __SECTOR_SIZEOF(sectors));

                        break;
                    }
                }
            } 

            do
            {
                Exchange.ftl.fnMain();
            }while(Exchange.ftl.fnIsAdminBusy());
        }
    }
    else  // admin (update low)
    {
        unsigned char *dest_buff = 0, *end_of_buff = 0;
        unsigned int diffbytes = 0;
        unsigned int ofs_adminlow = 0;
        unsigned int addr_adminlow[4] = {0,};
        unsigned int mask_of_max_writeindex = (*Exchange.buffer.SectorsOfWriteCache-1);

        ofs_adminlow = ((miosmart.adminlow_size + page_size - 1) / page_size) * page_size;
        addr_adminlow[0] = *Exchange.buffer.BaseOfAdmin1;
        addr_adminlow[1] = addr_adminlow[0] + __SECTOR_OF_BYTE(ofs_adminlow);  
        addr_adminlow[2] = addr_adminlow[1] + __SECTOR_OF_BYTE(ofs_adminlow);
        addr_adminlow[3] = addr_adminlow[2] + __SECTOR_OF_BYTE(ofs_adminlow);

        feature = MIOSMART_PART_ADMIN1;

        // switch partition
        while(1)
        {
            Exchange.ftl.fnMain();

            resp = Exchange.ftl.fnPrePutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
            if (resp >= 0)
            {
                resp = Exchange.ftl.fnPutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
                if (resp >= 0)
                {
                    do
                    {
                        Exchange.ftl.fnMain();
                    }while(Exchange.ftl.fnIsBusy());

                    break;
                }
            }
        } 

        // write
        for (sequent = 0; sequent < 4; sequent++)
        {
            address = addr_adminlow[sequent];

            while(1)
            {
                Exchange.ftl.fnMain();

                resp = Exchange.ftl.fnPrePutCommand(IO_CMD_WRITE, 0, address, sectors);
                if (resp >= 0)
                {
                    ext_index = resp;
                    resp = Exchange.ftl.fnPutCommand(IO_CMD_WRITE, 0, address, sectors);
                    if (resp >= 0)
                    {
                        *Exchange.buffer.WriteBlkIdx = (unsigned int)ext_index;
                        break;
                    }
                }
            } 

            if (resp >= 0)
            {
                dest_buff = (unsigned char *)(*Exchange.buffer.BaseOfWriteCache);
                dest_buff += __SECTOR_SIZEOF(ext_index & mask_of_max_writeindex);

                end_of_buff = (unsigned char *)(*Exchange.buffer.BaseOfWriteCache) + __SECTOR_SIZEOF(*Exchange.buffer.SectorsOfWriteCache);
                diffbytes = end_of_buff - dest_buff;
                if (diffbytes < sectors)
                {
                    memcpy((void *)dest_buff, (void *)buff, diffbytes);
                    memcpy((void *)(*Exchange.buffer.BaseOfWriteCache), (void *)((unsigned int)buff + diffbytes), (__SECTOR_SIZEOF(sectors) - diffbytes));
                }
                else
                {
                    memcpy(dest_buff, buff, __SECTOR_SIZEOF(sectors));
                }

                *Exchange.buffer.WriteBlkIdx += sectors;
                *Exchange.buffer.WriteBlkIdx &= mask_of_max_writeindex;

            }
        }

        // switch partition
        while(1)
        {
            Exchange.ftl.fnMain();

            resp = Exchange.ftl.fnPrePutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
            if (resp >= 0)
            {
                resp = Exchange.ftl.fnPutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
                if (resp >= 0)
                {
                    do
                    {
                        Exchange.ftl.fnMain();
                    }while(Exchange.ftl.fnIsBusy());

                    break;
                }
            }
        } 

    }

    return resp;
}

int miosmart_readadmin(unsigned char region, void * buff, unsigned int sectors, unsigned char sequent)
{
    int resp = 0;
    unsigned char feature = 0;
    unsigned int address = 0;
    int ext_index = -1;
    unsigned int page_size = __POW(16,10);   // 16 KB

    if (region)  // Admin(UpdateHigh/Middle)
    {
        unsigned int *src_buff = 0;

        unsigned int ofs_adminhigh = 0;
        unsigned int ofs_adminmiddle = 0;
        unsigned int addr_adminhigh[2] = {0,};
        unsigned int addr_adminmiddle[2] = {0,};

        ofs_adminhigh = ((miosmart.adminhigh_size + page_size - 1) / page_size) * page_size;
        ofs_adminmiddle = ((miosmart.adminmiddle_size + page_size - 1) / page_size) * page_size;
        addr_adminhigh[0] = *Exchange.buffer.BaseOfAdmin2;
        addr_adminhigh[1] = addr_adminhigh[0] + __SECTOR_OF_BYTE(ofs_adminhigh);
        addr_adminmiddle[0] = addr_adminhigh[1] + __SECTOR_OF_BYTE(ofs_adminhigh);
        addr_adminmiddle[1] = addr_adminmiddle[0] + __SECTOR_OF_BYTE(ofs_adminmiddle);

        feature = MIOSMART_PART_ADMIN2;

        switch (region)
        {
            case 1: { address = addr_adminhigh[sequent]; } break;
            case 2: { address = addr_adminmiddle[sequent]; } break;
        }

        while(1)
        {
            Exchange.ftl.fnMain();

            resp = Exchange.ftl.fnPrePutCommand(IO_CMD_READ_DIRECT, __POW(feature,0), address, sectors);
            if (resp >= 0)
            {
                ext_index = resp;
                resp = Exchange.ftl.fnPutCommand(IO_CMD_READ_DIRECT, __POW(feature,0), address, sectors);
                if (resp >= 0)
                {
                    do
                    {
                        Exchange.ftl.fnMain();
                    }while(Exchange.ftl.fnIsAdminBusy());

                    src_buff = (unsigned int *)(*Exchange.buffer.BaseOfDirectReadCache + __SECTOR_SIZEOF(ext_index));
                    memcpy (buff, src_buff, __SECTOR_SIZEOF(sectors));

                    break;
                }
            }
        }
    }
    else  // Admin(UpdateLow)
    {
        unsigned char *src_buff = 0, *end_of_buff = 0;
        unsigned int diffbytes = 0;
        unsigned int ofs_adminlow = 0;
        unsigned int addr_adminlow[4] = {0,};
        unsigned int mask_of_max_readindex = (*Exchange.buffer.SectorsOfReadBuffer-1);

        ofs_adminlow = ((miosmart.adminlow_size + page_size - 1) / page_size) * page_size;
        addr_adminlow[0] = *Exchange.buffer.BaseOfAdmin1;
        addr_adminlow[1] = addr_adminlow[0] + __SECTOR_OF_BYTE(ofs_adminlow);
        addr_adminlow[2] = addr_adminlow[1] + __SECTOR_OF_BYTE(ofs_adminlow);
        addr_adminlow[3] = addr_adminlow[2] + __SECTOR_OF_BYTE(ofs_adminlow);

        feature = MIOSMART_PART_ADMIN1;

        address = addr_adminlow[sequent];

        // switch partition
        while(1)
        {
            Exchange.ftl.fnMain();

            resp = Exchange.ftl.fnPrePutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
            if (resp >= 0)
            {
                resp = Exchange.ftl.fnPutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
                if (resp >= 0)
                {
                    do
                    {
                        Exchange.ftl.fnMain();
                    }while(Exchange.ftl.fnIsBusy());

                    break;
                }
            }
        } 

        // read
        while(1)
        {
            Exchange.ftl.fnMain();

            resp = Exchange.ftl.fnPrePutCommand(IO_CMD_READ, 0, address, sectors);
            if (resp >= 0)
            {
                ext_index = resp;
                resp = Exchange.ftl.fnPutCommand(IO_CMD_READ, 0, address, sectors);
                if (resp >= 0)
                {
                    while (1)
                    {
                        Exchange.ftl.fnMain();

                        if (((*Exchange.buffer.ReadNfcIdx - ext_index) & mask_of_max_readindex) >= sectors)
                        {
                        #if 1
                            if ((*Exchange.buffer.ReadNfcIdx & mask_of_max_readindex) != ((ext_index + sectors) & mask_of_max_readindex))
                            {
                                while(1)
                                {
                                    printk(KERN_INFO "miosmart_readadmin: DBG AAA");
                                }
                            }
                        #endif
                        
                            break;
                        }
                    }
                    *Exchange.buffer.ReadBlkIdx = (*Exchange.buffer.ReadNfcIdx & mask_of_max_readindex);

                    src_buff = (unsigned char *)*Exchange.buffer.BaseOfReadBuffer;
                    src_buff += __SECTOR_SIZEOF(ext_index & mask_of_max_readindex);

                    end_of_buff = (unsigned char *)*Exchange.buffer.BaseOfReadBuffer + __SECTOR_SIZEOF(*Exchange.buffer.SectorsOfReadBuffer);
                    diffbytes = end_of_buff - src_buff;
                    if (diffbytes < sectors)
                    {
                        memcpy(buff, src_buff, diffbytes);
                        memcpy(buff, (void *)*Exchange.buffer.BaseOfReadBuffer, (__SECTOR_SIZEOF(sectors) - diffbytes));
                    }
                    else
                    {
                        memcpy(buff, src_buff, __SECTOR_SIZEOF(sectors));
                    }

                    break;
                }
            }
        } 

        // switch partition
        while(1)
        {
            Exchange.ftl.fnMain();

            resp = Exchange.ftl.fnPrePutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
            if (resp >= 0)
            {
                resp = Exchange.ftl.fnPutCommand(IO_CMD_SWITCH_PARTITION, feature, 0, 0);
                if (resp >= 0)
                {
                    do
                    {
                        Exchange.ftl.fnMain();
                    }while(Exchange.ftl.fnIsBusy());

                    break;
                }
            }
        } 
    }

    return resp;
}

void miosmart_get_erasecount(unsigned int *min, unsigned int *max, unsigned int *sum, unsigned int average[])
{
    unsigned int sum_erasecount = 0, erasecount = 0;
    unsigned int min_erasecount = 0xFFFFFFFF, max_erasecount = 0;
    unsigned char channel = 0, way = 0;
    unsigned char max_channels = MioSmartInfo.max_channels;
    unsigned char max_ways = MioSmartInfo.max_ways;
    unsigned int block = 0;
  //unsigned char bits_per_cell = miosmart.nand->_f.bits_per_cell;
    unsigned int max_blocks = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun;
    unsigned int buff_size = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun * sizeof(unsigned int);
    unsigned int entrydata = 0;    
    unsigned int *wearlevel_data = 0;

    wearlevel_data = (unsigned int *)vmalloc(buff_size);
    if (!wearlevel_data)
    {
        DBG_MIOSMART(KERN_WARNING "miosmart_get_erasecount: memory alloc: fail");
        return;
    }
    
    for (channel = 0; channel < max_channels; channel++)
    {
        for (way = 0; way < max_ways; way++)
        {
            Exchange.ftl.fnGetWearLevelData(channel, way, wearlevel_data, buff_size);

            for (block = 0; block < max_blocks; block++)
            {
                entrydata = *(wearlevel_data + block);
                erasecount = (entrydata & 0x000FFFFF);
              //erasecount = __NROOT(erasecount, (bits_per_cell - 1));

                // max/min/average erasecount
                if (erasecount < min_erasecount)
                {
                    min_erasecount = erasecount;
                }

                if (erasecount > max_erasecount)
                {
                    max_erasecount = erasecount;
                }

                sum_erasecount += erasecount;
            }
        }
    }

    if (min) { *min = min_erasecount; }
    if (max) { *max = max_erasecount; }
    if (sum) { *sum = sum_erasecount; }

    if (average)
    {
        unsigned int total_usableblocks = miosmart_get_total_usableblocks();

        average[0] = sum_erasecount / total_usableblocks;
        average[1] = ((sum_erasecount % total_usableblocks) * 100) / total_usableblocks;
    }

    if (wearlevel_data)
    {
        vfree(wearlevel_data);
    }

    return;
}

unsigned int miosmart_get_total_usableblocks(void)
{
    unsigned int sum_usableblocks = 0, badblocks = 0;
    unsigned char max_channels = MioSmartInfo.max_channels;
    unsigned char max_ways = MioSmartInfo.max_ways;
    unsigned int max_blocks = miosmart.nand->_f.luns_per_ce * miosmart.nand->_f.mainblocks_per_lun;
    unsigned char channel = 0, way = 0;

    for (channel = 0; channel < max_channels; channel++)
    {
        for (way = 0; way < max_ways; way++)
        {
            badblocks = Exchange.ftl.fnGetBlocksCount(channel, way,
                BLOCK_TYPE_DATA_HOT_BAD, BLOCK_TYPE_DATA_COLD_BAD, BLOCK_TYPE_FBAD, BLOCK_TYPE_IBAD, BLOCK_TYPE_RBAD, 0xFF);
            sum_usableblocks += max_blocks - badblocks;
        }
    }

    return sum_usableblocks;
}

