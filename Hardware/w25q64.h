#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f10x.h"

/* ===================== 片选宏 ===================== */
#define CS_ENABLE     GPIO_ResetBits(GPIOA, GPIO_Pin_4)   /* 拉低选中 */
#define CS_DISENABLE  GPIO_SetBits(GPIOA, GPIO_Pin_4)     /* 拉高释放 */

/* ===================== 错误码 ===================== */
#define W25Q64_OK            0    /* 操作成功 */
#define W25Q64_ERR_TIMEOUT   1    /* WaitBusy 超时：芯片未接/接触不良/擦写异常 */

/* ===================== 超时参数 ===================== */
/* WaitBusy 轮询次数上限：每次轮询约 3~5us，20万次 ≈ 0.6~1s，
   覆盖 64KB 块擦除的最大规格（400ms），留有余量 */
#define W25Q64_BUSY_TIMEOUT  200000u

/* ===================== 函数接口 ===================== */
void    W25Q64_Init(void);
uint8_t W25Q64_WaitBusy(void);                               /* 0=空闲 1=超时 */
uint8_t W25Q64_Enable(void);                                 /* 写使能 */
uint8_t W25Q64_Erase64K(uint8_t blockNB);                    /* 64KB 块擦除 */
uint8_t W25Q64_Erase4K(uint32_t addr);                       /* 4KB 扇区擦除 */
uint8_t W25Q64_PageWrite(uint8_t *wbuff, uint16_t pageNB);   /* 页编程（固定256字节，不可跨页） */
uint8_t W25Q64_Read(uint8_t *rbuff, uint32_t addr, uint16_t datalen);  /* 读数据 */
uint8_t W25Q64_ReadID(uint32_t *id);                         /* 读 JEDEC ID，正常应为 0xEF4017 */

#endif
