#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"
#include "my_spi.h"
#include "w25q64.h"

uint8_t wdata[256];
uint8_t rdata[256];

int main(void)
{
    uint16_t i, j;

    Usart1_Init(115200);        /* Usart0_Init → Usart1_Init，按你串口助手当前 115200 */
    W25Q64_Init();

//    W25Q64_Erase64K(0);         /* 擦 0 号块：地址 0~65535，正好覆盖下面要写的 256 页 */

    for(i = 0; i < 256; i++)
    {
        for(j = 0; j < 256; j++)
            wdata[j] = i;               /* 第 i 页整页填 i */
        W25Q64_PageWrite(wdata, i);
    }

    Delay_ms(50);

    for(i = 0; i < 256; i++)
    {
        W25Q64_Read(rdata, i * 256, 256);
        for(j = 0; j < 256; j++)
            u1_printf("地址%d=%x\r\n", i * 256 + j, rdata[j]);
    }

    while(1)
    {
    }
}

