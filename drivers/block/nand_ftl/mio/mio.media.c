/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.media.c
 * Date         : 2014.06.30
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.06.30 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#define __MIO_MEDIA_GLOBAL__
#include "mio.media.h"

#include "mio.definition.h"

#if defined (__BUILD_MODE_X86_LINUX_DEVICE_DRIVER__)
#define __MEDIA_ON_RAM__
#elif defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#define __MEDIA_ON_NAND__
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#define __MEDIA_ON_NAND__
#endif

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__MEDIA_ON_RAM__)
#define MEDIA_ON_RAM_SIZE __MB(100)
#include "mio.block.h"

static u8 * media_on_ram;

#elif defined (__MEDIA_ON_NAND__)
#include "media/exchange.h"
#include "mio.block.h"
#include "mio.smart.h"
#endif

/******************************************************************************
 *
 ******************************************************************************/
void media_indicator_init(void);
void media_indicator_busy0(void);
void media_indicator_idle0(void);
void media_indicator_busy1(void);
void media_indicator_idle1(void);

/******************************************************************************
 *
 ******************************************************************************/
int media_open(void)
{
    int capacity = -1;

#if defined (__MEDIA_ON_RAM__)
    {
        media_on_ram = (u8 *)vmalloc(MEDIA_ON_RAM_SIZE);

        if (media_on_ram == NULL)
        {
            return -1;
        }

        capacity =  __SECTOR_OF_BYTE(MEDIA_ON_RAM_SIZE);
    }

#elif defined (__MEDIA_ON_NAND__)
    /**************************************************************************
     * FTL Need Leaner Buffer, Do Not Use kmalloc(...)
     **************************************************************************/
    DBG_MEDIA(KERN_INFO "media_open: Memory Pool Pre Alloc:\n");
    Exchange.buffer.mpool_size  = 0;
    Exchange.buffer.mpool_size += 1 * 4 * (4<<20); // 1CH x 4WAY x 4MB (Page Map Table per Lun)
    Exchange.buffer.mpool_size += 1 * 4 * (1<<20); // 1CH x 4WAY x 1MB (Update Map Table per Lun)
    Exchange.buffer.mpool_size += (1<<20);         // 1MB (Misc)
    Exchange.buffer.mpool = (unsigned char *)vmalloc(Exchange.buffer.mpool_size);

    if (!Exchange.buffer.mpool)
    {
        DBG_MEDIA(KERN_INFO "media_open: Memory Pool Pre Alloc: fail\n");
        return -1;
    }

    DBG_MEDIA(KERN_INFO "media_open: EXCHANGE_init:\n");
    EXCHANGE_init();

    /**************************************************************************
     * MIO Sys Options
     **************************************************************************/
    Exchange.sys.gpio.io_req = 32*2+0;  // Asign GPIOC.00
    Exchange.sys.gpio.bg_job = 32*2+1;  // Asign GPIOC.01
    Exchange.sys.gpio.nfc_wp = 32*2+27; // Asign GPIOC.27

    Exchange.sys.support_list.led_indicator = 1;
    Exchange.sys.support_list.spor = 1;

    Exchange.sys.fnIndicatorReqBusy = media_indicator_busy0;
    Exchange.sys.fnIndicatorReqIdle = media_indicator_idle0;
    Exchange.sys.fnIndicatorNfcBusy = media_indicator_busy1;
    Exchange.sys.fnIndicatorNfcIdle = media_indicator_idle1;

    media_indicator_init();

    /**************************************************************************
     * MIO Debug Options
     **************************************************************************/
  //Exchange.debug.misc.block = 1;
  //Exchange.debug.misc.media = 1;

    Exchange.debug.ftl.format = 1;
    Exchange.debug.ftl.format_progress = 1;
    Exchange.debug.ftl.configurations = 1;
    Exchange.debug.ftl.open = 1;
    Exchange.debug.ftl.memory_usage = 1;
    Exchange.debug.ftl.boot = 1;
    Exchange.debug.ftl.block_summary = 1;
    Exchange.debug.ftl.error = 1;
  //Exchange.debug.ftl.boot_read_retry = 1;
  //Exchange.debug.ftl.read_retry = 1;

  //Exchange.debug.nfc.sche.operation = 1;

  //Exchange.debug.nfc.phy.operation = 1;
    Exchange.debug.nfc.phy.info_feature = 1;
  //Exchange.debug.nfc.phy.info_ecc = 1;
  //Exchange.debug.nfc.phy.info_ecc_correction = 1;
  //Exchange.debug.nfc.phy.info_ecc_corrected = 1;
    Exchange.debug.nfc.phy.warn_prohibited_block_access = 1;
  //Exchange.debug.nfc.phy.warn_ecc_uncorrectable = 1;
  //Exchange.debug.nfc.phy.warn_ecc_uncorrectable_show = 1;
    Exchange.debug.nfc.phy.err_ecc_uncorrectable = 1;

    /**************************************************************************
     * Intial EWS FTL
     **************************************************************************/
#if defined (__COMPILE_MODE_FORMAT__)
    DBG_MEDIA(KERN_INFO "media_open: Exchange.ftl.fnFormat()\n");
    Exchange.ftl.fnFormat("NXP4330", 0xF0067000, 0);
#endif

    DBG_MEDIA(KERN_INFO "media_open: Exchange.ftl.fnOpen()\n");
    if (Exchange.ftl.fnOpen("NXP4330", 0xF0067000, 0) < 0)
    {
        DBG_MEDIA(KERN_INFO "media_open: Exchange.ftl.fnOpen(): fail\n");
        return -1;
    }

    DBG_MEDIA(KERN_INFO "media_open: Exchange.ftl.fnBoot(0)\n");
    if (Exchange.ftl.fnBoot(0) < 0)
    {
        DBG_MEDIA(KERN_INFO "media_open: Exchange.ftl.fnBoot(0): fail\n");
        return -1;
    }

    capacity = *Exchange.ftl.Capacity;

#endif

    return capacity;
}

/******************************************************************************
 *
 ******************************************************************************/
void media_close(void)
{
    printk(KERN_INFO "media_close: Start\n");

#if defined (__MEDIA_ON_RAM__)
    vfree(media_on_ram);
#elif defined (__MEDIA_ON_NAND__)
    DBG_MEDIA(KERN_INFO "media_close: Exchange.ftl.fnClose:\n");
    Exchange.ftl.fnClose();
    vfree(Exchange.buffer.mpool);
#endif

    printk(KERN_INFO "media_close: Done\n");
}

/******************************************************************************
 *
 ******************************************************************************/
void media_suspend(void)
{
#if defined (__MEDIA_ON_RAM__)
#elif defined (__MEDIA_ON_NAND__)
    media_powerdown(NULL);
    while (!media_is_idle(NULL));
    Exchange.nfc.fnSuspend();
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void media_resume(void)
{
#if defined (__MEDIA_ON_RAM__)
#elif defined (__MEDIA_ON_NAND__)
    Exchange.nfc.fnResume();
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void media_write(sector_t _lba, unsigned int _seccnt, u8 * _buffer, void * _io_state)
{
    sector_t lba = _lba;
    unsigned int seccnt = _seccnt;
    u8 * buffer = _buffer;
    struct mio_state * io_state = _io_state;

#if defined (__MEDIA_ON_RAM__)
    memcpy(media_on_ram + lba * __SECTOR_SIZEOF(1), buffer, seccnt * __SECTOR_SIZEOF(1));
#elif defined (__MEDIA_ON_NAND__)
    int wcidxfar = 0;
    int wcidx = 0;

    // Test Put Command To FTL
    while (1)
    {
        wcidxfar = Exchange.ftl.fnPrePutCommand(IO_CMD_WRITE, 0, lba, seccnt);
        wcidx = wcidxfar & (*Exchange.buffer.SectorsOfWriteCache - 1);

        if (-1 == wcidxfar)
        {
            media_super();
        }
        else
        {
            break;
        }
    }

    // Adjust Write Index
    *Exchange.buffer.WriteBlkIdx = (unsigned int)wcidxfar;

    // Copy buffer to WCache
    {
        unsigned int dest = 0;
        unsigned int src  = 0;
        unsigned int size = 0;

        // Write Cache Roll Over
        if ((wcidx + seccnt) > *Exchange.buffer.SectorsOfWriteCache)
        {
            dest = *Exchange.buffer.BaseOfWriteCache + (wcidx << 9);
            src  = (unsigned int)buffer;
            size = (*Exchange.buffer.SectorsOfWriteCache - wcidx) << 9;
            memcpy((void *)dest, (const void *)src, size);

            dest = *Exchange.buffer.BaseOfWriteCache;
            src  = (unsigned int)buffer + size;
            size = (seccnt << 9) - size;
            memcpy((void *)dest, (const void *)src, size);
        }
        else
        {
            dest = *Exchange.buffer.BaseOfWriteCache + (wcidx << 9);
            src  = (unsigned int)buffer;
            size = seccnt << 9;
            memcpy((void *)dest, (const void *)src, size);
        }
    }

    // Adjust Write Index
    *Exchange.buffer.WriteBlkIdx += (unsigned int)seccnt;

    // Put Command to FTL
    Exchange.ftl.fnPutCommand(IO_CMD_WRITE, 0, lba, seccnt);

    if (io_state->transaction.trigger.e.force_flush)
    {
        Exchange.ftl.fnPrePutCommand(IO_CMD_WCACHE_OFF, 0, 0, 0);
        Exchange.ftl.fnPutCommand(IO_CMD_WCACHE_OFF, 0, 0, 0);
        io_state->transaction.trigger.e.force_flush = 0;
    }

    // IO Summary
    Exchange.statistics.ios.cur.write += (seccnt << 9);
    Exchange.statistics.ios.cur.write_seccnt += seccnt;
    Exchange.statistics.ios.accumulate.write += (seccnt << 9);
    Exchange.statistics.ios.accumulate.write_seccnt += seccnt;

    {
		media_super();
#if 0
        unsigned int super_loop = 40;

        for (; super_loop; super_loop--)
        {
            media_super();
        }
#endif
    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void media_read(sector_t _lba, unsigned int _seccnt, u8 * _buffer, void * _io_state)
{
    sector_t lba = _lba;
    unsigned int seccnt = _seccnt;
    unsigned int req_seccnt = _seccnt;
    unsigned int req_trseccnt = 0;
    u8 * buffer = _buffer;
  //struct mio_state * io_state = _io_state;

#if defined (__MEDIA_ON_RAM__)
    memcpy(buffer, media_on_ram + lba * __SECTOR_SIZEOF(1), seccnt * __SECTOR_SIZEOF(1));
#elif defined (__MEDIA_ON_NAND__)
    int rbidxfar = 0;

    // Test Put Command to FTL
    while (1)
    {
        rbidxfar = Exchange.ftl.fnPrePutCommand(IO_CMD_READ, 0, lba, seccnt);

        if (-1 == rbidxfar)
        {
            media_super();
        }
        else
        {
            break;
        }
    }

    // Adjust Read Index
    *Exchange.buffer.ReadBlkIdx = (unsigned int)rbidxfar;

    // Put Command to FTL
    Exchange.ftl.fnPutCommand(IO_CMD_READ, 0, lba, seccnt);

    // IO Summary
    Exchange.statistics.ios.cur.read += (seccnt << 9);
    Exchange.statistics.ios.cur.read_seccnt += seccnt;
    Exchange.statistics.ios.accumulate.read += (seccnt << 9);
    Exchange.statistics.ios.accumulate.read_seccnt += seccnt;

    // Copy DATA From "FTL Read Buffer" to "BIO Read Buffer"
    while (1)
    {
        unsigned int rbidx  = 0;
        unsigned int readed = 0;

        rbidx  = *Exchange.buffer.ReadBlkIdx & (*Exchange.buffer.SectorsOfReadBuffer - 1);
        readed = Exchange.buffer.fnGetRequestReadSeccnt();

        if (readed)
        {
            /**********************************************************************
             * Case By Case
             *
             *   Case 1) "FTL Readed Buffer" is Linear
             *
             *     a) "FTL Readed Buffer" > "BIO Read Buffer" : Cause "Read-Look-Ahead" or "Another BIO Requested" or ...
             *                                             readed
             *       FTL Read Buffer :   base +---------+----------+------------------------------------------+ end
             *                                |         | ++++++++ |                                          |
             *                                +---------+----------+------------------------------------------+
             *       BIO Read Buffer :                b +------+
             *                                          | ++++ |
             *                                          +------+
             *
             *     b) "FTL Readed Buffer" < "BIO Read Buffer"
             *                                                        readed
             *       FTL Read Buffer :   base +--------------------+----------+--------------------------------+ end
             *                                |                    | ++++++++ |                                |
             *                                +--------------------+----------+--------------------------------+
             *       BIO Read Buffer :                           b +----------------+
             *                                                     | ++++++++       |
             *                                                     +----------------+
             *
             *     c) "FTL Readed Buffer" == "BIO Read Buffer"
             *                                                                   readed
             *       FTL Read Buffer :   base +-------------------------------+----------+---------------------+ end
             *                                |                               | ++++++++ |                     |
             *                                +-------------------------------+----------+---------------------+
             *       BIO Read Buffer :                                      b +----------+
             *                                                                | ++++++++ |
             *                                                                +----------+
             *
             *
             *   Case 2) "FTL Readed Buffer" is Roll Over : refer a), b), c)
             *
             *       FTL Read Buffer :   base +-----+-----------------------------------------------------+----+ end
             *                                | +++ |                                                     | ++ |
             *                                +-----+-----------------------------------------------------+----+
             *
             **********************************************************************/
            unsigned int IsLinear = 1;

            unsigned int dest = 0;
            unsigned int src  = 0;
            unsigned int size = 0;
            unsigned int trseccnt = 0;

            // Is Roll Over ?
            if ((rbidx + readed) > *Exchange.buffer.SectorsOfReadBuffer)
            {
                IsLinear = 0;
            }

            // "FTL Readed Buffer" > "BIO Buffer"
            if (readed > seccnt)
            {
                // "FTL Readed Buffer" is Linear
                if (IsLinear)
                {
                    dest = (unsigned int)buffer;
                    src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                    size = seccnt << 9;
                    trseccnt += size >> 9;
                    memcpy((void *)dest, (const void *)src, size);
                }
                // "FTL Readed Buffer" is Roll Over
                else
                {
                    // But 'seccnt' is Linear
                    if ((*Exchange.buffer.SectorsOfReadBuffer - rbidx) >= seccnt)
                    {
                        dest = (unsigned int)buffer;
                        src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                        size = seccnt << 9;
                        trseccnt += size >> 9;
                        memcpy((void *)dest, (const void *)src, size);
                    }
                    else
                    {
                        dest = (unsigned int)buffer;
                        src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                        size = (*Exchange.buffer.SectorsOfReadBuffer - rbidx) << 9;
                        trseccnt += size >> 9;
                        memcpy((void *)dest, (const void *)src, size);

                        dest = (unsigned int)buffer + size;
                        src  = *Exchange.buffer.BaseOfReadBuffer;
                        size = (seccnt << 9) - size;
                        trseccnt += size >> 9;
                        memcpy((void *)dest, (const void *)src, size);
                    }
                }
            }
            // "FTL Readed Buffer" <= "BIO Buffer"
            else if (readed <= seccnt)
            {
                // "FTL Readed Buffer" is Linear
                if (IsLinear)
                {
                    dest = (unsigned int)buffer;
                    src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                    size = readed << 9;
                    trseccnt += size >> 9;
                    memcpy((void *)dest, (const void *)src, size);
                }
                // "FTL Readed Buffer" is Roll Over
                else
                {
                    dest = (unsigned int)buffer;
                    src  = *Exchange.buffer.BaseOfReadBuffer + (rbidx << 9);
                    size = (*Exchange.buffer.SectorsOfReadBuffer - rbidx) << 9;
                    trseccnt += size >> 9;
                    memcpy((void *)dest, (const void *)src, size);

                    dest = (unsigned int)buffer + size;
                    src  = *Exchange.buffer.BaseOfReadBuffer;
                    size = (readed << 9) - size;
                    trseccnt += size >> 9;
                    memcpy((void *)dest, (const void *)src, size);
                }
            }

            seccnt -= trseccnt;
            buffer += trseccnt << 9;
            req_trseccnt += trseccnt;

            // Adjust Read Index
            *Exchange.buffer.ReadBlkIdx += trseccnt;

            // Current Request Completed
            if (req_trseccnt == req_seccnt)
            {
                break;
            }
        }
        else
        {
            media_super();
        }
    }

    media_super();
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
int media_super(void)
{
    int ret = -1;

#if defined (__MEDIA_ON_RAM__)
#elif defined (__MEDIA_ON_NAND__)
    if (Exchange.ftl.fnMain)
    {
        Exchange.ftl.fnMain();
        ret = 0;
    }
#endif

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
void media_flush(void * _io_state)
{
    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_FLUSH, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_FLUSH, 0, 0, 0);
                break;
            }
        }
    }
}

void media_background(void * _io_state)
{
    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_BACKGROUND, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_BACKGROUND, 0, 0, 0);
                break;
            }
        }
    }
}

void media_standby(void * _io_state)
{
    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_STANDBY, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_STANDBY, 0, 0, 0);
                break;
            }
        }
    }
}

void media_powerdown(void * _io_state)
{
    while (1)
    {
        media_super();

        if (Exchange.ftl.fnIsReady())
        {
            if (Exchange.ftl.fnPrePutCommand(IO_CMD_POWER_DOWN, 0, 0, 0) >= 0)
            {
                Exchange.ftl.fnPutCommand(IO_CMD_POWER_DOWN, 0, 0, 0);
                break;
            }
        }
    }
}

int media_is_idle(void * _io_state)
{
    if (!Exchange.ftl.fnIsIdle())
    {
        media_super();
        return 0;
    }

    return 1;
}

/******************************************************************************
 * LED Indicator
 ******************************************************************************/
extern int /* -1 = invalid gpio, 0 = gpio's input mode, 1 = gpio's output mode. */ nxp_soc_gpio_get_io_dir(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/);
extern void nxp_soc_gpio_set_io_dir(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/, int /* '1' is output mode, '0' is input mode */);
extern void nxp_soc_gpio_set_out_value(unsigned int /* gpio pad number, 32*n + bit (n= GPIO_A:0, GPIO_B:1, GPIO_C:2, GPIO_D:3, GPIO_E:4, ALIVE:5, bit= 0 ~ 32)*/, int /* '1' is high level, '0' is low level */);

void media_indicator_init(void)
{
    nxp_soc_gpio_set_io_dir(Exchange.sys.gpio.io_req, 1);
    nxp_soc_gpio_set_io_dir(Exchange.sys.gpio.bg_job, 1);
}

void media_indicator_busy0(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.io_req, 1);
}

void media_indicator_idle0(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.io_req, 0);
}

void media_indicator_busy1(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.bg_job, 1);
}

void media_indicator_idle1(void)
{
    nxp_soc_gpio_set_out_value(Exchange.sys.gpio.bg_job, 0);
}
