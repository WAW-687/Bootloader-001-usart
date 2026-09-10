#ifndef __BOOT_H
#define __BOOT_H

#include <stdint.h>

/**
  ******************************************************************************
  * @file    boot.h
  * @brief   OTA Bootloader 对外接口（跳转App / 命令行 / Xmodem下载 / 元数据）
  * @note    分区与标志宏见 main.h；命令行菜单 [1]~[7] 见 BootLoader_Info
  ******************************************************************************
  */

/* App 复位函数指针类型：接收一个占位参数（复位向量不接收参数，传0） */
typedef void (*load_a)(uint32_t);

void BootLoader_Branch(void);                               /* 开机分流：'w'进命令行 / OTA魔数→搬运 / 直接跳App */
uint8_t BootLoader_Enter(uint8_t timeout);                  /* 开机拦截窗口：timeout*100ms内敲'w'返回1 */
void BootLoader_Info(void);                                 /* 打印命令行菜单 [1]~[7] */
void BootLoader_Event(uint8_t *data, uint16_t datalen);     /* 命令行事件分发（含Xmodem包解析） */
void W25Q64_ReadOTAInfo(void);                              /* 从W25Q64元数据扇区读 OTA_Info */
void W25Q64_WriteOTAInfo(void);                             /* 先擦后写，把 OTA_Info 存入元数据扇区 */
__asm void MSR_SP(uint32_t addr);                           /* 设置主堆栈指针MSP后返回（跳转App用） */
void BootLoader_Clear(void);                                /* 注销Bootloader用过的外设（跳转前调用） */
void LOAD_A(uint32_t addr);                                 /* 校验并跳转到App区（addr=0x08005000） */
uint16_t Xmodem_CRC16(uint8_t *data, uint16_t datalen);     /* Xmodem CRC16-CCITT 校验（多项式0x1021） */


#endif
