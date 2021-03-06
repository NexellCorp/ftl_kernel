/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : exchange.h
 * Date         : 2014.07.11
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.11 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#ifdef __EXCHANGE_GLOBAL__
#define EXCHANGE_EXT
#else
#define EXCHANGE_EXT extern
#endif

/******************************************************************************
 *
 ******************************************************************************/
#include "exchange.config.h"

/******************************************************************************
 *
 ******************************************************************************/
#pragma pack(1)
typedef union __NAND__
{
    unsigned char _c[512-4*4/*CRCs*/];

    struct
    {
        unsigned char manufacturer[16];
        unsigned char modelname[32];
        unsigned char id[16];
        unsigned char generation[32];

#define NAND_INTERFACE_ASYNC        1
#define NAND_INTERFACE_ONFI_ASYNC   2
#define NAND_INTERFACE_ONFI_SYNC    3
#define NAND_INTERFACE_TOGGLE_ASYNC 4
#define NAND_INTERFACE_TOGGLE_DDR   5
        // timing config
        unsigned char interfacetype;
        unsigned char onfi_detected;
        unsigned char onfi_timing_mode;

        struct
        {
            struct
            {
                unsigned int tClk;  // Hz
                unsigned int tRWC;  // ns, tRC: RE# cycle time, tWC: WE# cycle time
                unsigned int tR;    // ns, READ PAGE operation time
                unsigned int tWB;   // ns, WE# HIGH to R/B# LOW
                unsigned int tCCS;  // ns, Change column setup time to data in/out or next command
                unsigned int tADL;  // ns, ALE to data start
                unsigned int tRHW;  // ns, RE# HIGH to WE# LOW
                unsigned int tWHR;  // ns, WE# HIGH to RE# LOW
                unsigned int tWW;   // ns, WP# transition to WE# LOW
                unsigned int tRR;   // ns, Ready to RE# LOW
                unsigned int tFEAT; // ns, Busy time for SET FEATURES and GET FEATURES operations

                unsigned int tCS;   // ns, CE# setup
                unsigned int tCH;   // ns, CE# hold
                unsigned int tCLS;  // ns, CLE setup time
                unsigned int tALS;  // ns, ALE setup time
                unsigned int tCLH;  // ns, CLE hold time
                unsigned int tALH;  // ns, ALE hold time
                unsigned int tWP;   // ns, WE# pulse width
                unsigned int tWH;   // ns, WE# HIGH hold time
                unsigned int tWC;   // ns, WE# cycle time
                unsigned int tDS;   // ns, Data setup time
                unsigned int tDH;   // ns, Data In hold

                unsigned int tCEA;  // ns, CE# access time
                unsigned int tREA;  // ns, RE# access time
                unsigned int tRP;   // ns, RE# pulse width
                unsigned int tREH;  // ns, RE# HIGH hold time
                unsigned int tRC;   // ns, RE# cycle time
                unsigned int tCOH;  // ns, CE# HIGH to output hold

            } async;

            struct
            {
                unsigned char _rsvd;

            } sync;

            struct
            {
                unsigned char _rsvd;

            } ddr;

        } timing;

        // cell config
        unsigned int luns_per_ce;
        unsigned int databytes_per_page;
        unsigned int sparebytes_per_page;
        unsigned int number_of_planes;
        unsigned int pages_per_block;
        unsigned int mainblocks_per_lun;
        unsigned int extendedblocks_per_lun;
        unsigned int next_lun_address;
        unsigned int over_provisioning; // 8000 ~ 9313
        unsigned int bits_per_cell;
        unsigned int number_of_bits_ecc_correctability;
        unsigned int maindatabytes_per_eccunit;
        unsigned int eccbits_per_maindata;
        unsigned int eccbits_per_blockinformation;
        unsigned int block_endurance;
        unsigned int factorybadblocks_per_nand;

        // operation config
        struct
        {
            unsigned int slc_mode            : 1;
            unsigned int multiplane_read     : 1;
            unsigned int multiplane_write    : 1;
            unsigned int cache_read          : 1;
            unsigned int cache_write         : 1;
            unsigned int interleave          : 1;
            unsigned int randomize           : 1;

            unsigned int _rsvd0 : (32-7);

        } support_list;

        struct
        {
            unsigned char multiplane_read;
            unsigned char multiplane_write;
            unsigned char cache_read;
            unsigned char cache_write;
            unsigned char interleave;
            unsigned char paired_page_mapping;
            unsigned char block_indicator;
            unsigned char paired_plane;
            unsigned char multiplane_erase;

#define NAND_READRETRY_TYPE_MICRON_20NM             (1)
#define NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE    (10)
#define NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE   (11)
#define NAND_READRETRY_TYPE_HYNIX_1xNM_MLC          (20)
#define NAND_READRETRY_TYPE_TOSHIBA_A19NM           (40)
            unsigned char read_retry;

            unsigned char _rsvd0[6];

        } support_type;

        unsigned int step_of_static_wear_leveling;
        unsigned int max_channel;
        unsigned int max_way;

        // operating drive strength config
        union
        {
            unsigned int _i;

            struct
            {
                unsigned int micron_a  : 1; // L73A, L74A, L83A, L84A, L85A
                unsigned int intel_a   : 1; // ??
                unsigned int samsung_a : 1; // ??
                unsigned int hynix_a   : 1; // ??
                unsigned int thosiba_a : 1; // ??

                unsigned int _rsvd0 : (32-5);

            } _f;

        } ds_support_list;

        struct
        {
            unsigned char micron_a;  // L73A, L74A, L83A, L84A, L85A
            unsigned char intel_a;   // ??
            unsigned char samsung_a; // ??
            unsigned char hynix_a;   // ??
            unsigned char thosiba_a; // ??

            unsigned char _rsvd0[16-5];

        } ds_support_type;

    } _f;

} NAND;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/

#pragma pack(1)
typedef struct __CONFIGURATIONS__
{
    NAND nand;

    unsigned int crc;

} CONFIGURATIONS;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/

#pragma pack(1)
typedef union __WARN_LIST__
{
    struct
    {
        unsigned int   bitHour   : 24;
        unsigned int   bitMinute : 8;

        unsigned char  ucSecond;
        unsigned char  ucState;
        unsigned short usReportType;

        unsigned short usFuncID;
        unsigned char  bitChannel : 4;
        unsigned char  bitWay     : 4;
        unsigned char  ucPart;

        unsigned short usPhyPage;
        unsigned short usPhyBlock;

    } s;

    struct
    {
        unsigned int uiWord0;
        unsigned int uiWord1;
        unsigned int uiWord2;
        unsigned int uiWord3;

    } w;

} WARN_LIST;
#pragma pack()

#pragma pack(1)
typedef struct __WARN__
{
    WARN_LIST * pstList[2];
    unsigned char ucBankHead;
    unsigned char ucBankTail;
    unsigned short usListHead;
    unsigned short usListTail;
    unsigned short usLists;
    unsigned int uiAddress;

} WARN;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/

#pragma pack(1)
typedef struct __ExFTL__
{
    // Main Method
    void (*fnConfig)(void * _config);
    int (*fnFormat)(unsigned char * _chip_name, unsigned int _chip_id_base, unsigned char _option);
    int (*fnOpen)(unsigned char * _chip_name, unsigned int _chip_id_base, unsigned int _format_open);
    int (*fnClose)(void);
    int (*fnBoot)(unsigned char _mode);
    int (*fnMain)(void);

    // Command Method
#define IO_CMD_NONE                                 (0x0000)
#define IO_CMD_READ                                 (0x0101)
#define IO_CMD_READ_DIRECT                          (0x0102)    // Do Not Use
#define IO_CMD_WRITE                                (0x0201)
#define IO_CMD_WRITE_DIRECT                         (0x0202)    // Do Not Use
#define IO_CMD_BACKGROUND                           (0x1101)
#define IO_CMD_DATA_SET_MANAGEMENT                  (0x0801)
#define IO_CMD_FLUSH                                (0x0F01)
#define IO_CMD_STANDBY                              (0x0F02)
#define IO_CMD_SWITCH_PARTITION                     (0x0F03)    // Do Not Use
#define IO_CMD_WCACHE_OFF                           (0x0F04)
#define IO_CMD_POWER_DOWN                           (0x0F0F)
#define IO_CMD_DATA_SET_MANAGEMENT_FEATURE_NONE     (0x00)
#define IO_CMD_DATA_SET_MANAGEMENT_FEATURE_TRIM     (0x01)
#define IO_CMD_FEATURE_WRITE_CONTINUE               (0x01)
#define IO_CMD_MAX_READ_SECTORS                     (0x10000)   //  32 MB
#define IO_CMD_MAX_WRITE_SECTORS                    (0x100)     // 128 KB
    int (*fnPrePutCommand)(unsigned short _command, unsigned char _feature, unsigned int _address, unsigned int _length);
    int (*fnPutCommand)(unsigned short _command, unsigned char _feature, unsigned int _address, unsigned int _length);
    void (*fnCancelCommand)(unsigned char _slot, unsigned int _length);
    void (*fnAdjustCommand)(unsigned char _slot, unsigned int _length);

    // Status Method
    unsigned int (*fnIsBooted)(void);
    unsigned int (*fnIsReady)(void);
    unsigned int (*fnIsIdle)(void);
    unsigned int (*fnIsBusy)(void);
    unsigned int (*fnIsAdminBusy)(void);

    // Misc Method
    unsigned int (*fnSetTime)(void);
    WARN * (*fnGetWarnList)(void);
    int (*fnGetNandInfo)(NAND *info);
    int (*fnGetWearLevelData)(unsigned char _channel, unsigned char _way, void *buff, unsigned int buff_size);

#define BLOCK_TYPE_MAPLOG           (0x8)
#define BLOCK_TYPE_FREE             (0x9)
#define BLOCK_TYPE_DATA_HOT         (0x4)
#define BLOCK_TYPE_DATA_HOT_BAD     (0x5)
#define BLOCK_TYPE_DATA_COLD        (0x6)
#define BLOCK_TYPE_DATA_COLD_BAD    (0x7)
#define BLOCK_TYPE_UPDATE_SEQUENT   (0x2)
#define BLOCK_TYPE_UPDATE_RANDOM    (0x3)
#define BLOCK_TYPE_FBAD             (0xB)
#define BLOCK_TYPE_IBAD             (0xA)
#define BLOCK_TYPE_RBAD             (0xC)
#define BLOCK_TYPE_ROOT             (0xD)
#define BLOCK_TYPE_ENED             (0xE)
#define BLOCK_TYPE_FIRM             (0xF)
    unsigned int (*fnGetBlocksCount)(unsigned char _channel, unsigned char _way, unsigned int _1st_blocktype, ...);

    // Property
    unsigned int * Capacity;
    unsigned char * Way;
    unsigned char * Channel;
    unsigned int * ReadRetryCount;

} ExFTL;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/

#pragma pack(1)
typedef struct __ExBUFFER__
{
    unsigned char * mpool;
    unsigned int mpool_size;

    unsigned int (*fnGetRequestReadSeccnt)(void);

    // Read Buffer
    unsigned int * BaseOfReadBuffer;
    unsigned int * SectorsOfReadBuffer;
    unsigned int * ReadNfcIdx;
    unsigned int * ReadBlkIdx;

    // Write Cache
    unsigned int * BaseOfWriteCache;
    unsigned int * SectorsOfWriteCache;
    unsigned int * WriteNfcIdx;
    unsigned int * WriteBlkIdx;

    // Direct Read/Write Cache
    unsigned int * BaseOfDirectReadCache;
    unsigned int * SectorsOfDirectReadCache;
    unsigned int * BaseOfDirectWriteCache;
    unsigned int * SectorsOfDirectWriteCache;

    // Admin buffer
    unsigned int * BaseOfAdmin1;
    unsigned int * SectorsOfAdmin1;
    unsigned int * BaseOfAdmin2;
    unsigned int * SectorsOfAdmin2;

} ExBUFFER;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/
#pragma pack(1)
 typedef struct __DEVICE_SUMMARY__
{
    unsigned int uiAccumEccError;
    unsigned int uiAccumEccLevel;
    unsigned int uiAccumEccSector;
    unsigned int uiAccumEccCount;
    unsigned char ucAccumEccMax;
    unsigned char reserved[3];
    unsigned short usAccumWriteFail;
    unsigned short usAccumEraseFail;
    unsigned int uiAccumReadRetry;

} DEVICE_SUMMARY;
#pragma pack()

#pragma pack(1)
typedef struct __ExSTATISTICS__
{
    struct
    {
        unsigned int  day;
        unsigned char hour;
        unsigned char minute;
        unsigned char second;
        unsigned char msecond;

    } por_time;

    struct
    {
        struct
        {
            unsigned long long read;
            unsigned long long write;

            unsigned long long read_seccnt;
            unsigned long long write_seccnt;

        } accumulate;

        struct
        {
            unsigned long long read;
            unsigned long long write;

            unsigned long long read_seccnt;
            unsigned long long write_seccnt;

        } cur;

    } ios;

    DEVICE_SUMMARY **device_summary; // [FTL_WAYS][FTL_CHANNELS];

} ExSTATISTICS;
#pragma pack()

#pragma pack(1)
typedef struct __ExNFC__
{
    unsigned int (*fnInit)(unsigned int _scan_format);
    void (*fnDeInit)(void);
    void (*fnSuspend)(void);
    void (*fnResume)(void);

    int  (*fnEccInfoInit)(unsigned int _max_channels, unsigned int _max_ways, const unsigned char *_way_map);
    void (*fnEccInfoDeInit)(void);

    void (*fnGetFeatures)(unsigned int * _max_channel, unsigned int * _max_way, void * _nand_config);
    void (*fnAdjustFeatures)(void);
    void (*fnSetFeatures)(unsigned int _max_channel, unsigned int _max_way, void * _nand_config);

    void (*fnDelay)(unsigned int _tDelay);

    int (*fnReadId)(unsigned int _channel, unsigned int _way, char * _id, char * _onfi_id, char * _jedec_id);

    void (*fnSetOnfiFeature)(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned int _parameter);
    void (*fnGetOnfiFeature)(unsigned int _channel, unsigned int _way, unsigned char _feature_address, unsigned int * _parameter);

    int (*fnStatusIsFAIL)(unsigned char _status);
    int (*fnStatusIsFAILC)(unsigned char _status);
    int (*fnStatusIsARDY)(unsigned char _status);
    int (*fnStatusIsRDY)(unsigned char _status);
    int (*fnStatusIsWP)(unsigned char _status);
    unsigned char (*fnStatusRead)(unsigned int _channel, unsigned int _way);

    int (*fn1stRead)(unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col);
    int (*fn2ndReadDataNoEcc)(unsigned int _channel, unsigned int _way, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, void * _data_buffer, unsigned int _bytes_spare, void * _spare_buffer);
    int (*fn2ndReadLog)(unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col, unsigned int _log_loop_count, unsigned int _bytes_per_log_ecc, unsigned int _bytes_per_log_parity, unsigned int _log_ecc_bits, void * _log_buffer, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer, unsigned int _retryable);
    int (*fn2ndReadData)(unsigned int _stage, unsigned int _channel, unsigned int _way, unsigned int _row, unsigned int _col, unsigned int _fake_spare_row, unsigned int _fake_spare_col, unsigned int _data_loop_count, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer, unsigned int _bytes_per_spare_ecc, unsigned int _bytes_per_spare_parity, unsigned int _spare_ecc_bits, void * _spare_buffer, unsigned int _retryable);
    int (*fn3rdRead)(unsigned int _channel, unsigned int _way);

    int (*fn1stWriteLog)(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _col, unsigned int _multi_plane_write_cmd, unsigned int _cache_write_cmd, unsigned int _log_loop_count, unsigned int _bytes_per_log_ecc, unsigned int _bytes_per_log_parity, unsigned int _log_ecc_bits, void * _log_buffer, unsigned int _data_loop_count0, unsigned int _data_loop_count1, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer0, void * _data_buffer1);
    int (*fn1stWriteRoot)(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _col, unsigned int _root_loop_unit, unsigned int _bytes_per_root_ecc, unsigned int _bytes_per_root_parity, unsigned int _root_ecc_bits, void * _root_buffer);
    int (*fn1stWriteData)(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _col, unsigned int _multi_plane_write_cmd, unsigned int _cache_write_cmd, unsigned int _data_loop_count0, unsigned int _data_loop_count1, unsigned int _bytes_per_data_ecc, unsigned int _bytes_per_data_parity, unsigned int _data_ecc_bits, void * _data_buffer0, void * _data_buffer1, unsigned int _bytes_per_spare_ecc, unsigned int _bytes_per_spare_parity, unsigned int _spare_ecc_bits, void * _spare_buffer);
    int (*fn2ndWrite)(unsigned int _channel, unsigned int _way);
    unsigned char (*fn3rdWrite)(unsigned int _channel, unsigned int _way);

    int (*fn1stErase)(unsigned int _channel, unsigned int _way, unsigned int _row0, unsigned int _row1, unsigned int _multi_plane_erase_cmd);
    int (*fn2ndErase)(unsigned int _channel, unsigned int _way);
    unsigned char (*fn3rdErase)(unsigned int _channel, unsigned int _way);

    int  (*fnReadRetry_Init)(unsigned int _max_channels, unsigned int _max_ways, const unsigned char *_way_map, unsigned char _readretry_type);
    void (*fnReadRetry_DeInit)(void);
    int  (*fnReadRetry_MakeRegAll)(void);
    void (*fnReadRetry_SetParameter)(unsigned int _channel, unsigned int _way);
    int  (*fnReadRetry_GetTotalReadRetryCount)(unsigned int _channel, unsigned int _way);
    void *(*fnReadRetry_GetAddress)(void);
    void *(*fnReadRetry_GetRegDataAddress)(unsigned int _channel, unsigned int _way);
    void (*fnReadRetry_ClearAllCurrReadRetryCount)(void);
    void (*fnReadRetry_PrintTable)(void);
    void (*fnReadRetry_Post)(unsigned int _channel, unsigned int _way);

    int (*fnRandomize_Init)(int _buf_size);
    void (*fnRandomize_DeInit)(void);
    void (*fnRandomize_Enable)(unsigned char _enable);

    struct
    {
        unsigned char timing_mode;
        unsigned char output_drive_strength0;
        unsigned char output_drive_strength1;
        unsigned char rb_pull_down_strength;
        unsigned char read_retry;
        unsigned char array_operation_mode;

    } onfi_feature_address;

    struct
    {
        unsigned int ways;
        unsigned int channels;

        unsigned int * level0;           // Data 512B or 1024B
        unsigned int * level1;           // Information 8B
        unsigned int ** level_error;
        unsigned int ** error;
        unsigned int ** correct_sector;
        unsigned int ** correct_bit;
        unsigned int ** max_correct_bit;

    } ecc;

} ExNFC;
#pragma pack()

#pragma pack(1)
typedef struct __ExSTD__
{
    int (*__print)(const char *, ...);
    int (*__sprintf)(char *, const char *, ...);
    unsigned int (*__strlen)(const char *);
    void * (*__memset)(void *, int, unsigned int);
    void * (*__memcpy)(void *, const void *, unsigned int);
    int (*__memcmp)(const void *, const void *, unsigned int);

    unsigned long long (*__div64)(unsigned long long _dividend, unsigned long long _divisor);

    unsigned short (*__get_crc16)(unsigned short _initial, void * _buffer, unsigned int _length);
    unsigned int (*__get_crc32)(unsigned int _initial, void * _buffer, unsigned int _length);

} ExSTD;
#pragma pack()

#pragma pack(1)
typedef struct __ExSYS__
{
    struct
    {
        unsigned int io_req;
        unsigned int bg_job;
        unsigned int nfc_wp;

    } gpio;

    struct
    {
        unsigned int spor          : 1;
        unsigned int led_indicator : 1;

        unsigned int _rsvd0        : 32 -2;

    } support_list;

    // SPOR
    unsigned int lvd_detected;
    void (*fnSpor)(void);

    // Indicator
    void (*fnIndicatorReqBusy)(void);
    void (*fnIndicatorReqIdle)(void);
    void (*fnIndicatorNfcBusy)(void);
    void (*fnIndicatorNfcIdle)(void);

} ExSYS;
#pragma pack()

#pragma pack(1)
typedef struct __ExDEBUG__
{
    struct
    {
        unsigned int block  : 1;
        unsigned int media  : 1;

        unsigned int _rsvd0 : 32 - 2;

    } misc;

    struct
    {
        unsigned int cp_data;
        unsigned int cp_size;

        unsigned char pattern[8192];

    } misc_sub;

    struct
    {
        unsigned int format          : 1;
        unsigned int format_progress : 1;
        unsigned int configurations  : 1;
        unsigned int open            : 1;
        unsigned int memory_usage    : 1;
        unsigned int boot            : 1;
        unsigned int boot_read_retry : 1;
        unsigned int read_retry      : 1;
        unsigned int block_summary   : 1;
        unsigned int smart_log       : 1;
        unsigned int _rsvd0          : 16 - 10;

        // Error, Warnning
        unsigned int error  : 1;
        unsigned int _rsvd1 : 16 - 1;

    } ftl;

    struct
    {
        struct
        {
            unsigned int operation : 1;
            unsigned int _rsvd0    : 32 - 1;

        } sche;

        struct
        {
            unsigned int operation : 1;
            unsigned int _rsvd0    : 8 - 1;

            unsigned int read_data  : 1;
            unsigned int write_data : 1;
            unsigned int _rsvd1     : 8 - 2;

            unsigned int info_feature        : 1;
            unsigned int info_ecc            : 1;
            unsigned int info_ecc_correction : 1;
            unsigned int info_ecc_corrected  : 1;
            unsigned int info_randomizer     : 1;
            unsigned int _rsvd2              : 8 - 5;

            // Error, Warnning
            unsigned int warn_prohibited_block_access : 1;
            unsigned int warn_ecc_uncorrectable       : 1;
            unsigned int warn_ecc_uncorrectable_show  : 1;
            unsigned int err_ecc_uncorrectable        : 1;
            unsigned int _rsvd3                       : 8 - 4;

        } phy;

    } nfc;

} ExDEBUG;
#pragma pack()

/******************************************************************************
 *
 ******************************************************************************/
typedef struct __EXCHANGES__
{
    unsigned long ewsftl_start_offset;
    unsigned int  ewsftl_start_page;
    unsigned int  ewsftl_start_block;

    ExFTL ftl;
    ExBUFFER buffer;
    ExSTATISTICS statistics;

    ExNFC nfc;

    ExSTD std;
    ExSYS sys;
    ExDEBUG debug;

} EXCHANGES;

EXCHANGE_EXT EXCHANGES Exchange;

/******************************************************************************
 *
 ******************************************************************************/
EXCHANGE_EXT void EXCHANGE_init(void);
