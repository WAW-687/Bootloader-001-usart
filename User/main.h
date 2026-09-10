#ifndef __MAIN_H
#define __MAIN_H

/**
  ******************************************************************************
  * @file    main.h
  * @brief   OTA Bootloader 全局配置：Flash 分区 / W25Q64 存储布局 / 标志位 / 控制块
  * @note    内存布局总览：
  *          内部Flash：[0x08000000~0x08004FFF] Bootloader 20页
  *                     [0x08005000~0x0800FFFF] App 区 44页
  *          W25Q64　：[0x000000~]            固件块区（9个64KB块，块0=主固件）
  *                     [0x7FF000~0x7FFFFF]   元数据扇区（OTA_InfoCB，最后4KB）
  ******************************************************************************
  */

/* ===================== STM32 内部 Flash 分区（OTA 用） ===================== */
#define STM32_FLASH_SADDR      0x08000000                          /* 内部 Flash 首地址 */
#define STM32_PAGE_SIZE        1024                                /* 页大小：C8T6 中容量 1KB/页 */
#define STM32_PAGE_NUM         64                                  /* 总页数：64KB = 64 页 */
#define STM32_B_PAGE_NUM       20                                  /* Bootloader 区页数（照课程约定 20KB） */
#define STM32_A_PAGE_NUM       (STM32_PAGE_NUM - STM32_B_PAGE_NUM) /* App 区页数 = 44 页 */
#define STM32_A_START_PAGE     (STM32_B_PAGE_NUM)                    /* App 起始页号 = 20 */
#define STM32_A_SADDR          (STM32_FLASH_SADDR + STM32_A_START_PAGE * STM32_PAGE_SIZE)  /* App 起始地址 0x08005000 */

/* ===================== W25Q64 存储布局 ===================== */
#define W25Q64_TOTAL_SIZE      0x800000                            /* 总容量 8MB */
#define W25Q64_SECTOR_SIZE     4096                                /* 擦除最小单位 4KB 扇区 */
#define W25Q64_PAGE_SIZE       256                                 /* 编程页 256B（不可跨页写） */
#define W25Q64_SECTOR_NUM      (W25Q64_TOTAL_SIZE / W25Q64_SECTOR_SIZE)   /* 2048 个扇区 */
#define W25Q64_PAGE_NUM        (W25Q64_TOTAL_SIZE / W25Q64_PAGE_SIZE)     /* 32768 个页 */

/* ---------- 元数据区：最后一个扇区（代替 AT24C02） ---------- */
#define W25Q64_META_SADDR      (W25Q64_TOTAL_SIZE - W25Q64_SECTOR_SIZE)   /* 元数据扇区首地址 = 0x7FF000 */
#define W25Q64_META_PAGE       (W25Q64_META_SADDR / W25Q64_PAGE_SIZE)     /* 元数据页号 = 32760，与 META_SADDR 严格同址（0x7FF000） */
#define W25Q64_META_PADDR      (W25Q64_META_SADDR + 0)                    /* 元数据页首地址 = 0x7FF000 */

/* ---------- 固件下载区（OTA 备用，暂定从头开始） ---------- */
#define W25Q64_FW_SADDR        0x000000                            /* 固件在 W25Q64 的存放首地址 */

#define OTA_SET_FLAG      	0xAABB1122     /* OTA更新标志魔数 */
#define UPDATA_A_FLAG     	0x00000001     /* Bootloader状态位：正在更新A区（搬运固件中） */
#define IAP_XMODEMC_FLAG	0x00000002
#define IAP_XMODEMD_FLAG	0x00000004
#define SET_VERSION_FLAG	0x00000008
#define CMD_5_FLAG			0x00000010
#define CMD5_XMODEM_FLAG	0x00000020
#define CMD_6_FLAG			0x00000040


typedef struct{
    uint32_t OTA_flag;                    /* 0号成员固定在首位，老逻辑（判断魔数）不受影响 */
    uint32_t Firelen[11];                 /* 预留11个字：固件长度/CRC/版本号等后续课程填充 */
	uint8_t OTA_ver[32];
}OTA_InfoCB;                              /* 共 12 字 = 48 字节 */

#define OTA_INFOCB_SIZE    sizeof(OTA_InfoCB)

typedef struct{
    uint8_t  Updatabuff[STM32_PAGE_SIZE]; /* 一页数据的暂存缓冲（1KB），收满一页写一次 Flash */
    uint32_t W25Q64_BlockNB;              /* 当前下载内容要写入 W25Q64 的块号 */
	uint32_t XmodemTimer;
	uint32_t XmodemNum;
	uint32_t XmodemCRC;
}UpDataA_CB;

extern OTA_InfoCB OTA_Info;
extern UpDataA_CB UpDataA;
extern uint32_t  BootStaFlag;        /* Bootloader 状态标志，定义在 main.c */
extern uint32_t BootStaFlag;



#endif
