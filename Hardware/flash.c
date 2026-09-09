#include "stm32f10x.h"                  // Device header
#include "flash.h"


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










    
