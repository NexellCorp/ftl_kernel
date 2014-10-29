/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.sys.c
 * Date         : 2014.07.02
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.02 SD.LEE)
 *
 * Description  : API
 *
 ******************************************************************************/
#define __MIO_BLOCK_SYSFS_GLOBAL__
#include "mio.sys.h"
#include "mio.block.h"
#include "media/exchange.h"
#include "mio.definition.h"
#include "mio.smart.h"

/******************************************************************************
 * 
 ******************************************************************************/
static ssize_t miosys_read(struct file * file, char * buf, size_t count, loff_t * ppos);
static ssize_t miosys_write(struct file * file, const char * buf, size_t count, loff_t * ppos);
static int miosys_print_wearleveldata(void);
static int miosys_print_smart(void);

struct file_operations miosys_fops =
{
    .owner = THIS_MODULE,
    .read = miosys_read,
    .write = miosys_write,
};

struct miscdevice miosys =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "miosys",
    .fops = &miosys_fops,
};

/******************************************************************************
 * 
 ******************************************************************************/
char kbuf[16];

static ssize_t miosys_read(struct file * file, char * buf, size_t count, loff_t * ppos)
{
  //DBG_MIOSYS(KERN_INFO "miosys_read(file:0x%08x buf:0x%08x count:%d ppos:0x%08x)",
  //        (unsigned int)file, (unsigned int)buf, count, (unsigned int)ppos);


    if (count < 256) { return -EINVAL; }
    if (*ppos != 0) { return 0; }
    memset((void *)buf, 0, count);
    if (copy_to_user(buf, kbuf, 16)) { return -EINVAL; }
    *ppos = count;

    return count;
}

char * strtok(register char *s, register const char *delim);

static ssize_t miosys_write(struct file * file, const char * buf, size_t count, loff_t * ppos)
{
    char * cmd_buf = (char *)vmalloc(count);
    if (copy_from_user(cmd_buf, buf, count)) { return -EINVAL; }

  //DBG_MIOSYS(KERN_INFO "miosys_write(file:0x%08x buf:0x%08x count:%d ppos:0x%08x)",
  //          (unsigned int)file, (unsigned int)buf, count, (unsigned int)ppos);

    // Command Parse
    {
        enum
        {
            MIOSYS_NONE = 0x00000000,

            MIOSYS_REQUEST_SMART     = 0x10000000,
            MIOSYS_REQUEST_WEARLEVEL = 0x20000000,

            MIOSYS_REQEUST_DEBUG = 0x30000000,

            // # echo "debug block enable" > /dev/miosys
            // # echo "debug block disable" > /dev/miosys
            MIOSYS_REQUEST_DEBUG_BLOCK = MIOSYS_REQEUST_DEBUG+1,

            // # echo "debug media enable" > /dev/miosys
            // # echo "debug media disable" > /dev/miosys
            MIOSYS_REQUEST_DEBUG_MEDIA = MIOSYS_REQEUST_DEBUG+2,

            // # echo "debug nfc.phy.operation enable" > /dev/miosys
            // # echo "debug nfc.phy.operation disable" > /dev/miosys
            MIOSYS_REQUEST_DEBUG_NFC_PHY_OPERATION = MIOSYS_REQEUST_DEBUG+3,

            // # echo "debug nfc.phy.rdata enable" > /dev/miosys
            // # echo "debug nfc.phy.rdata disable" > /dev/miosys
            MIOSYS_REQUEST_DEBUG_NFC_PHY_READ_DATA = MIOSYS_REQEUST_DEBUG+4,

            // # echo "debug nfc.phy.wdata enable" > /dev/miosys
            // # echo "debug nfc.phy.wdata disable" > /dev/miosys
            MIOSYS_REQUEST_DEBUG_NFC_PHY_WRITE_DATA = MIOSYS_REQEUST_DEBUG+5,

            MIOSYS_MAX = 0xFFFFFFFF

        } state = MIOSYS_NONE;

        char delim[] = " ";
        char * token = strtok(cmd_buf, delim);
        int breaker = 0;

        while ((token != NULL) && !breaker)
        {
            printk("token = %s\n", token);

            switch (state)
            {
                case MIOSYS_NONE:
                {
                    if (!memcmp((const void *)token, (const void *)"smart", strlen("smart")))
                    {
                        state = MIOSYS_REQUEST_SMART;
                    }
                    else if (!memcmp((const void *)token, (const void *)"wearlevel", strlen("wearlevel")))
                    {
                        state = MIOSYS_REQUEST_WEARLEVEL;
                    }
                    else if (!memcmp((const void *)token, (const void *)"debug", strlen("debug")))
                    {
                        state = MIOSYS_REQEUST_DEBUG;
                    }
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case MIOSYS_REQUEST_SMART: { breaker = 1; } break;
                case MIOSYS_REQUEST_WEARLEVEL: { breaker = 1; } break;

                case MIOSYS_REQEUST_DEBUG:
                {
                         if (!memcmp((const void *)token, (const void *)"block",             strlen("block")))             { state = MIOSYS_REQUEST_DEBUG_BLOCK; }
                    else if (!memcmp((const void *)token, (const void *)"media",             strlen("media")))             { state = MIOSYS_REQUEST_DEBUG_MEDIA; }
                    else if (!memcmp((const void *)token, (const void *)"nfc.phy.operation", strlen("nfc.phy.operation"))) { state = MIOSYS_REQUEST_DEBUG_NFC_PHY_OPERATION; }
                    else if (!memcmp((const void *)token, (const void *)"nfc.phy.rdata",     strlen("nfc.phy.rdata")))     { state = MIOSYS_REQUEST_DEBUG_NFC_PHY_READ_DATA; }
                    else if (!memcmp((const void *)token, (const void *)"nfc.phy.wdata",     strlen("nfc.phy.wdata")))     { state = MIOSYS_REQUEST_DEBUG_NFC_PHY_WRITE_DATA; }
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case MIOSYS_REQUEST_DEBUG_BLOCK:
                {
                         if (!memcmp((const void *)token, (const void *)"enable", strlen("enable")))   { Exchange.debug.misc.block = 1; }
                    else if (!memcmp((const void *)token, (const void *)"disable", strlen("disable"))) { Exchange.debug.misc.block = 0; }
                    else                                                                               { breaker = 1; }

                } break;

                case MIOSYS_REQUEST_DEBUG_MEDIA:
                {
                         if (!memcmp((const void *)token, (const void *)"enable", strlen("enable")))   { Exchange.debug.misc.media = 1; }
                    else if (!memcmp((const void *)token, (const void *)"disable", strlen("disable"))) { Exchange.debug.misc.media = 0; }
                    else                                                                               { breaker = 1; }

                } break;

                case MIOSYS_REQUEST_DEBUG_NFC_PHY_OPERATION:
                {
                         if (!memcmp((const void *)token, (const void *)"enable", strlen("enable")))   { Exchange.debug.nfc.phy.operation = 1; }
                    else if (!memcmp((const void *)token, (const void *)"disable", strlen("disable"))) { Exchange.debug.nfc.phy.operation = 0; }
                    else                                                                               { breaker = 1; }

                } break;

                case MIOSYS_REQUEST_DEBUG_NFC_PHY_READ_DATA:
                {
                         if (!memcmp((const void *)token, (const void *)"enable", strlen("enable")))   { Exchange.debug.nfc.phy.read_data = 1; }
                    else if (!memcmp((const void *)token, (const void *)"disable", strlen("disable"))) { Exchange.debug.nfc.phy.read_data = 0; }
                    else                                                                               { breaker = 1; }

                } break;

                case MIOSYS_REQUEST_DEBUG_NFC_PHY_WRITE_DATA:
                {
                         if (!memcmp((const void *)token, (const void *)"enable", strlen("enable")))   { Exchange.debug.nfc.phy.write_data = 1; }
                    else if (!memcmp((const void *)token, (const void *)"disable", strlen("disable"))) { Exchange.debug.nfc.phy.write_data = 0; }
                    else                                                                               { breaker = 1; }

                } break;

                default:
                {
                    breaker = 1;
                }
            }

            token = strtok(NULL, delim);
        }

        // execute
        switch (state)
        {
            case MIOSYS_REQUEST_SMART:
            {
                miosys_print_smart();
            } break;

            case MIOSYS_REQUEST_WEARLEVEL:
            {
                miosys_print_wearleveldata();
            } break;
        }
    }

    *ppos = count;
    return count;
}


char * strtok(register char *s, register const char *delim)
{
    register char *spanp;
    register int c, sc;
    char *tok;
    static char *last;

    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /* Skip (span) leading delimiters (s += strspn(s, delim), sort of). */
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;)
    {
        if (c == sc)
            goto cont;
    }

    if (c == 0)
    {
        /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /* Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too. */
    for (;;)
    {
        c = *s++;
        spanp = (char *)delim;

        do
        {
            if ((sc = *spanp++) == c)
            {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;

                last = s;

                return (tok);
            }

        } while (sc != 0);
    }
}

#if 0
int miosys_print_smart(void)
{
    // ECC corrected
    unsigned int *pcurrent = 0;
    unsigned int channel = 0, way = 0;

    // ECC info
    DBG_MIOSYS(KERN_INFO "NAND CH%02d-WAY%02d - ECC Count", channel, way);

    for (way=0; way < *Exchange.ftl.Way; way++)
    {
        for (channel=0; channel < *Exchange.ftl.Channel; channel++)
        {
            // ECC corrected
            pcurrent = &(Exchange.statistics.ecc_sector.corrected[way][channel]);
            DBG_MIOSYS(KERN_INFO " ECC corrected count: current:%d", *pcurrent);

            // ECC leveldetected
            pcurrent = &(Exchange.statistics.ecc_sector.leveldetected[way][channel]);
            DBG_MIOSYS(KERN_INFO " ECC leveldetected count: current:%d", *pcurrent);

            // ECC uncorrectable
            pcurrent = &(Exchange.statistics.ecc_sector.uncorrectable[way][channel]);
            DBG_MIOSYS(KERN_INFO " ECC uncorrectable count: current:%d", *pcurrent);

        }
    }

    return 0;
}
#else
int miosys_print_smart(void)
{
    // ECC corrected
    unsigned int *pcurrent = 0;
  //unsigned int *paccumulate = 0;
    MIO_SMART_CE_DATA *pnand_accumulate = 0;
    MIO_SMART_CE_DATA *pnand_current = 0;
    unsigned int channel = 0, way = 0;
    unsigned int sum_erasecount = 0, sum_usableblocks = 0;
    unsigned int min_erasecount = 0xFFFFFFFF, max_erasecount = 0;
    unsigned int average_erasecount[2] = {123,456};

    {
        miosmart_init(*Exchange.ftl.Channel, *Exchange.ftl.Way);
        miosmart_update();
    }

    DBG_MIOSYS(KERN_INFO "< SMART INFORMATION >");

    miosmart_get_erasecount(&min_erasecount, &max_erasecount, &sum_erasecount, average_erasecount);
    sum_usableblocks = miosmart_get_total_usableblocks();

    DBG_MIOSYS(KERN_INFO " sum of erasecount:\t%d", sum_erasecount);
    DBG_MIOSYS(KERN_INFO " sum of usable blocks:\t%d", sum_usableblocks);
    DBG_MIOSYS(KERN_INFO " max erasecount:\t\t%d", max_erasecount);
    DBG_MIOSYS(KERN_INFO " min erasecount:\t\t%d", min_erasecount);
    DBG_MIOSYS(KERN_INFO " average erasecount:\t%d.%02d", average_erasecount[0], average_erasecount[1]);
    DBG_MIOSYS(KERN_INFO " sum of readretrycount:\t%d", *Exchange.ftl.ReadRetryCount);

    // ECC info
    for (way=0; way < *Exchange.ftl.Way; way++)
    {
        for (channel=0; channel < *Exchange.ftl.Channel; channel++)
        {
            DBG_MIOSYS(KERN_INFO " NAND channel:%02d way:%02d's", channel, way);

            pnand_accumulate = &(MioSmartInfo.nand_accumulate[way][channel]);
            pnand_current = &(MioSmartInfo.nand_current[way][channel]);
            if (!pnand_accumulate || !pnand_current)
            {
                return -1;
            }

            // ECC corrected
            pcurrent = &(pnand_current->ecc_sector.corrected);
          //paccumulate = &(pnand_accumulate->ecc_sector.corrected);
          //DBG_MIOSYS(KERN_INFO "  ECC corrected sectors: current:%d, accumulated:%d", *pcurrent, *paccumulate);
            DBG_MIOSYS(KERN_INFO "  ECC corrected sectors:     \t%d", *pcurrent);

            // ECC leveldetected
            pcurrent = &(pnand_current->ecc_sector.leveldetected);
          //paccumulate = &(pnand_accumulate->ecc_sector.leveldetected);
          //DBG_MIOSYS(KERN_INFO "  ECC leveldetected sectors: current:%d accumulated:%d", *pcurrent, *paccumulate);
            DBG_MIOSYS(KERN_INFO "  ECC leveldetected sectors: \t%d", *pcurrent);

            // ECC uncorrectable
            pcurrent = &(pnand_current->ecc_sector.uncorrectable);
          //paccumulate = &(pnand_accumulate->ecc_sector.uncorrectable);
          //DBG_MIOSYS(KERN_INFO "  ECC uncorrectable sectors: current:%d, accumulated:%d", *pcurrent, *paccumulate);
            DBG_MIOSYS(KERN_INFO "  ECC uncorrectable sectors: \t%d", *pcurrent);

            // read retry count
            pcurrent = &(pnand_current->readretry_count);
          //paccumulate = &(pnand_accumulate->readretry_count);
          //DBG_MIOSYS(KERN_INFO "  Read retry count: current:%d, accumulated:%d", *pcurrent, *paccumulate);
            DBG_MIOSYS(KERN_INFO "  Read retry count:        \t%d", *pcurrent);

        }
    }
    DBG_MIOSYS(KERN_INFO " \n");

    {
        miosmart_deinit();
    }

    return 0;
}
#endif

int miosys_print_wearleveldata(void)
{
    unsigned char channel = 0;
    unsigned char way = 0;
    unsigned int *buff = 0;
    unsigned int buff_size = 0;
    NAND nand;
    int blockindex = 0;
    unsigned int entrydata = 0;
    unsigned int attribute = 0, sub_attribute = 0;
    unsigned int erasecount = 0, partition = 0;
    unsigned int badblock_count = 0;
    unsigned int min_erasecount = 0xFFFFFFFF, max_erasecount = 0;
    unsigned int average_erasecount[2] = {0,0};
    unsigned char isvalid_erasecount = 0;
    unsigned int validnum_erasecount = 0, sum_erasecount = 0;

    // create buffer
    Exchange.ftl.fnGetNandInfo(&nand);
    buff_size = nand._f.luns_per_ce * nand._f.mainblocks_per_lun * sizeof(unsigned int);

  //DBG_MIOSYS(KERN_INFO "buffer size:%d", buff_size);

    buff = (unsigned int *)vmalloc(buff_size);
    if (!buff)
    {
        DBG_MIOSYS(KERN_INFO "mio.sys: wearlevel: memory alloc: fail");
        return -1;
    }

    for (channel=0; channel < *Exchange.ftl.Channel; channel++)
    {
        for (way=0; way < *Exchange.ftl.Way; way++)
        {
            DBG_MIOSYS(KERN_INFO "NAND CH%02d-WAY%02d", channel, way);

            // Get Wearlevel data
            Exchange.ftl.fnGetWearLevelData(channel, way, buff, buff_size);

            // print wearlevel data
            for (blockindex = 0; blockindex < nand._f.mainblocks_per_lun; blockindex++)
            {
                isvalid_erasecount = 0;
                entrydata = *(buff + blockindex);
                attribute = (entrydata & 0xF0000000) >> 28;
                partition = (entrydata & 0x0F000000) >> 24;
                sub_attribute = (entrydata & 0x00F00000) >> 20;
                erasecount = (entrydata & 0x000FFFFF);

                switch (attribute)
                {
                    case 0x2:
                    case 0x3:
                    case 0x4:
                    case 0x5:
                    case 0x6:
                    case 0x7:
                    {
                        if (!partition) { DBG_MIOSYS(KERN_INFO " BLOCK %5d, EraseCount(%5d), DATA",         blockindex, erasecount); isvalid_erasecount = 1; }
                        else            { DBG_MIOSYS(KERN_INFO " BLOCK %5d, EraseCount(%5d), DATA (ADMIN)", blockindex, erasecount); isvalid_erasecount = 1; }
                    } break;

                    case 0x8: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, EraseCount(%5d), SYSTEM (M)", blockindex, erasecount); isvalid_erasecount = 1; } break;
                    case 0x9: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, EraseCount(%5d), FREE",       blockindex, erasecount); isvalid_erasecount = 1; } break;
                    case 0xA:
                    {
                        switch (sub_attribute)
                        {
                            case 0x7: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, PROHIBIT",       blockindex); } break;
                            case 0x8: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, CONFIG",         blockindex); } break;
                            case 0x9: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, SYSTEM (RETRY)", blockindex); } break;
                            case 0xA: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, BAD (Initial)",  blockindex); badblock_count++; } break;
                            case 0xB: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, BAD (Factory)",  blockindex); badblock_count++; } break;
                            case 0xC: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, BAD (Runtime)",  blockindex); badblock_count++; } break;
                            case 0xD: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, EraseCount(%5d), SYSTEM (ROOT)",  blockindex, erasecount); isvalid_erasecount = 1; } break;
                            case 0xE: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, SYSTEM (ENED)",  blockindex); } break;
                            case 0xF: { DBG_MIOSYS(KERN_INFO " BLOCK %5d, SYSTEM (FW)",    blockindex); } break;
                            default:  { DBG_MIOSYS(KERN_INFO " BLOCK %5d, SYSTEM (0x%x)",  blockindex, sub_attribute); } break;
                        }
                    } break;
                    default:  { DBG_MIOSYS(KERN_INFO " BLOCK %5d, unknown", blockindex); } break;
                }

                // erasecount: max/min/average
                if (isvalid_erasecount)
                {
                    if (erasecount < min_erasecount)
                    {
                        min_erasecount = erasecount;
                    }

                    if (erasecount > max_erasecount)
                    {
                        max_erasecount = erasecount;
                    }

                    sum_erasecount += erasecount;
                    validnum_erasecount += 1;
                }
            }
       
            if (validnum_erasecount)
            {
                average_erasecount[0] = sum_erasecount / validnum_erasecount;
                average_erasecount[1] = ((sum_erasecount % validnum_erasecount) * 100) / validnum_erasecount;
            }
 
            DBG_MIOSYS(KERN_INFO "max erasecount %5d", max_erasecount);
            DBG_MIOSYS(KERN_INFO "min erasecount %5d", min_erasecount);
            DBG_MIOSYS(KERN_INFO "average erasecount %d.%02d", average_erasecount[0], average_erasecount[1]);
        }
    }
    DBG_MIOSYS(KERN_INFO " \n");

    // destory buffer
    vfree(buff);

    return 0;
}


