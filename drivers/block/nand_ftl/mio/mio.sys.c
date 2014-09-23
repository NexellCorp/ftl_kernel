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
            STATE_NONE = 0x00,

            STATE_CMD       = 0x10000000,
            STATE_CMD_DEBUG = STATE_CMD+0x1000,
            STATE_CMD_CTRL  = STATE_CMD+0x2000,
            STATE_CMD_SMART = STATE_CMD+0x3000,

            STATE_CMD_DEBUG_NFC_SCHE = STATE_CMD_DEBUG+1,
            STATE_CMD_DEBUG_NFC_PHY = STATE_CMD_DEBUG+2,

            STATE_MAX = 0xFFFFFFFF

        } state = STATE_NONE;

        char delim[] = " -";
        char * token = strtok(cmd_buf, delim);
        int breaker = 0;

        while ((token != NULL) && !breaker)
        {
            printk("token = %s\n", token);

            switch (state)
            {
                case STATE_NONE:
                {
                    if (!memcmp((const void *)token, (const void *)"cmd", strlen("cmd")))
                    {
                        printk("STATE_NONE -> STATE_CMD\n");
                        state = STATE_CMD;
                    }
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case STATE_CMD:
                {
                    if (!memcmp((const void *)token, (const void *)"debug", strlen("debug")))
                    {
                        printk("STATE_CMD -> STATE_CMD_DEBUG\n");
                        state = STATE_CMD_DEBUG;
                    }
                    else if (!memcmp((const void *)token, (const void *)"ctrl", strlen("ctrl")))
                    {
                        printk("STATE_CMD -> STATE_CMD_CTRL\n");
                        state = STATE_CMD_CTRL;
                    }
                    else if (!memcmp((const void *)token, (const void *)"smart", strlen("smart")))
                    {
                        printk("STATE_CMD -> STATE_CMD_SMART\n");
                        state = STATE_CMD_SMART;
                    }
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case STATE_CMD_DEBUG:
                {
                    if (!memcmp((const void *)token, (const void *)"nfc.sche", strlen("nfc.sche")))
                    {
                        printk("STATE_CMD_DEBUG -> STATE_CMD_DEBUG_NFC_SCHE\n");
                        state = STATE_CMD_DEBUG_NFC_SCHE;
                    }
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case STATE_CMD_DEBUG_NFC_SCHE:
                {
                    printk("STATE_CMD_DEBUG_NFC_SCHE\n");
                    if (!memcmp((const void *)token, (const void *)"enable", strlen("enable")))
                    {
                        printk("NFC.PHY.Debug : Enabled\n");
                        Exchange.debug.nfc.sche.operation = 1;
                    }
                    else if (!memcmp((const void *)token, (const void *)"disable", strlen("disable")))
                    {
                        printk("NFC.PHY.Debug : Disabled\n");
                        Exchange.debug.nfc.sche.operation = 0;
                    }
                    else
                    {
                        breaker = 1;
                    }

                } break;

                case STATE_CMD_CTRL:
                {
                    printk("STATE_CMD_CTRL\n");

                } break;

                case STATE_CMD_SMART:
                {
                    unsigned int channel = 0;
                    unsigned int way = 0;

                    printk("EWS.FTL Smart\n");
                    printk("\n");
                    printk(" - Power On Time : %d days, %02d:%02d:%2d:%02\n", Exchange.statistics.por_time.day, Exchange.statistics.por_time.hour, Exchange.statistics.por_time.minute, Exchange.statistics.por_time.second, Exchange.statistics.por_time.msecond);
                    printk(" - Current  Read Command Count : %d\n", Exchange.statistics.ios.cur.read);
                    printk(" - Current Write Command Count : %d\n", Exchange.statistics.ios.cur.write);
                    printk(" - Current  Read  Sector Count : %d\n", Exchange.statistics.ios.cur.read_seccnt);
                    printk(" - Current Write  Sector Count : %d\n", Exchange.statistics.ios.cur.write_seccnt);
                    printk("\n");

                    for (channel = 0; channel < *Exchange.ftl.Channel; channel++)
                    {
                        for (way = 0; way < *Exchange.ftl.Way; way++)
                        {

                        }
                    }


                
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