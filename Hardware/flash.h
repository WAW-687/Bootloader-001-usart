#ifndef __FLASH_H
#define __FLASH_H

void STM32_EraseFlash(uint16_t start, uint16_t num);
void STM32_WriteFlash(uint32_t saddr, uint32_t *wdata, uint32_t wnum);


#endif
