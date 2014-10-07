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

/******************************************************************************
 * 
 ******************************************************************************/
static ssize_t miosys_read(struct file * file, char * buf, size_t count, loff_t * ppos);
static ssize_t miosys_write(struct file * file, const char * buf, size_t count, loff_t * ppos);

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