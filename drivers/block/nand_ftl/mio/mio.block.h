/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.block.h
 * Date         : 2014.06.30
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.06.30 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#ifdef __MIO_BLOCK_GLOBAL__
#define MIO_BLOCK_EXT
#else
#define MIO_BLOCK_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
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
#include <linux/mm.h>

/******************************************************************************
 *
 ******************************************************************************/
#define MIO_FIRST_MINOR     (0)
#define MIO_MINOR_CNT       (16)

/******************************************************************************
 *
 ******************************************************************************/
#define MIO_TIME_DIFF_MAX(J64)  (0xFFFFFFFFFFFFFFFF - (J64))
#define MIO_TIME_MSEC(MS)       (((MS)*(HZ))/1000)
#define MIO_TIME_SEC(S)         (( (S)*(HZ))/1)

/******************************************************************************
 *
 ******************************************************************************/
struct mio_state
{
    // MIO Background Jobs
    struct
    {
        struct task_struct * thread;
        unsigned int thread_sleep;

        // time
        struct
        {
            u64 ecccheck;
            u64 statistics;
            u64 flush;
            u64 standby;
            u64 bgjobs;

        } t;

        // event
        struct
        {
            unsigned int ecccheck;
            unsigned int statistics;
            unsigned int flush;
            unsigned int standby;
            unsigned int bgjobs;

        } e;

    } background;

    // MIO Data Transaction Jobs
    struct
    {
        struct task_struct * thread;
        struct request_queue * rq;
        spinlock_t queue_lock;

        spinlock_t lock;
        struct mutex thread_mutex;

        struct
        {
            int cnt;
            u64 time;

        } wake;

        struct
        {
            struct
            {
                u64 ioed;

            } t;

            struct
            {
                unsigned int written_flush;
                unsigned int written_standby;
                unsigned int written_bgjobs;

                unsigned int force_flush;

            } e;

        } trigger;

    } transaction;
};

struct mio_device
{
    struct semaphore * mutex;

    /**************************************************************************
     * Device Information
     **************************************************************************/
    unsigned int capacity;
    struct gendisk * disk;
    struct mio_state * io_state;
    struct miscdevice * miosys;
};

