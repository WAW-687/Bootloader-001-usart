/**
  ******************************************************************************
  * @file    flash.c
  * @brief   STM32F103C8T6 内部 Flash 读写驱动（OTA 用，操作 App 区 0x08005000~）
  * @note    中容量 C8T6：64KB Flash、1KB/页、共 64 页。
  *          分区约定见 main.h：Bootloader 占前 20 页，App 区占后 44 页。
  *          编程单位是"字"（32位），必须先整页擦除（擦=全FF）再编程（只能1→0）。
  ******************************************************************************
  */
#include "stm32f10x.h"                  // Device header
#include "flash.h"


/**
 * @brief  擦除内部 Flash 指定起始页开始的 num 个页
 * @param  start 起始页号（0~63，App 区从 20 开始）
 * @param  num   擦除的页数（App 区为 44）
 * @note   每页擦除约 20~40ms，擦 44 页约 1~2 秒
 */
void STM32_EraseFlash(uint16_t start, uint16_t num)
{
    uint16_t i;

    FLASH_Unlock();
    for(i = 0; i < num; i++)
    {
        FLASH_ErasePage((0x08000000 + start * 1024) + (1024 * i));
    }
    FLASH_Lock();
}

/**
 * @brief  向内部 Flash 指定地址连续编程
 * @param  saddr 起始地址（须 4 字节对齐，App 区为 0x08005000 起）
 * @param  wdata 数据缓冲区（按 uint32_t 字取数）
 * @param  wnum  字节数（隐含要求为 4 的倍数：循环按"每次4字节"递减，
 *               非 4 倍数会下溢 0xFFFFFFFE 导致狂写飞掉——调用方必须保证对齐）
 * @note   编程前目标区必须已擦除（本驱动不自动擦除，先擦后写由调用方负责）
 */
void STM32_WriteFlash(uint32_t saddr, uint32_t *wdata, uint32_t wnum)
{
    FLASH_Unlock();
    while(wnum)
    {
        FLASH_ProgramWord(saddr, *wdata);
        wnum -= 4;               /* wnum 按字节数计：每次编程写 4 字节 */
        saddr += 4;
        wdata++;
    }
    FLASH_Lock();
}










    
