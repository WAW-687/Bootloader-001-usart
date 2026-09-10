#ifndef __FLASH_H
#define __FLASH_H

/* ============ STM32 内部 Flash 驱动对外接口（OTA 用） ============ */
void STM32_EraseFlash(uint16_t start, uint16_t num);                    /* 擦除 start 页起 num 个页 */
void STM32_WriteFlash(uint32_t saddr, uint32_t *wdata, uint32_t wnum);  /* 从 saddr 起编程 wnum 字节（须4对齐、先擦后写） */


#endif
