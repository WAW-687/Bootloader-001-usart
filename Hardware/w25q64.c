#include "stm32f10x.h"                  // Device header
#include "my_spi.h"
#include "w25q64.h"


void W25Q64_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* rcu_periph_clock_enable(RCU_GPIOA) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* PA4 - CS：推挽输出，软件控制片选 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CS_DISENABLE;       /* 片选默认拉高（不选中） */
    SPI1_Init();        /* SPI0_Init → SPI1_Init（上一次转好的那个） */
}

void W25Q64_WaitBusy(void)
{
    uint8_t res;
    do{
        CS_ENABLE;
        SPI1_ReadWriteByte(0x05);          /* Read Status Register-1 */
        res = SPI1_ReadWriteByte(0xff);
        CS_DISENABLE;
    }while((res & 0x01) == 0x01);          /* bit0 = BUSY 位 */
}

void W25Q64_Enable(void)
{
    W25Q64_WaitBusy();
    CS_ENABLE;
    SPI1_ReadWriteByte(0x06);              /* Write Enable */
    CS_DISENABLE;
}

void W25Q64_Erase64K(uint8_t blockNB)
{
    uint8_t wdata[4];

    wdata[0] = 0xD8;                       /* Block Erase 64KB */
    wdata[1] = (blockNB*64*1024) >> 16;
    wdata[2] = (blockNB*64*1024) >> 8;
    wdata[3] = (blockNB*64*1024) >> 0;

    W25Q64_WaitBusy();
    W25Q64_Enable();                       /* 擦除前必须先 Write Enable */
    CS_ENABLE;
    SPI1_Write(wdata, 4);
    CS_DISENABLE;
    W25Q64_WaitBusy();                     /* 64KB 擦除典型 150ms，靠 WaitBusy 等完 */
}

void W25Q64_Read(uint8_t *rbuff, uint32_t addr, uint32_t datalen)
{
    uint8_t wdata[4];

    wdata[0] = 0x03;                       /* Read Data */
    wdata[1] = (addr) >> 16;
    wdata[2] = (addr) >> 8;
    wdata[3] = (addr) >> 0;

    W25Q64_WaitBusy();
    CS_ENABLE;
    SPI1_Write(wdata, 4);                  /* 发命令+24位地址 */
    SPI1_Read(rbuff, datalen);             /* 连续读数据 */
    CS_DISENABLE;
}

void W25Q64_Erase4K(uint32_t addr)     /* addr 为扇区首地址，如 0x7FF000 */
{
    uint8_t wdata[4];

    wdata[0] = 0x20;                   /* Sector Erase 4KB */
    wdata[1] = (addr) >> 16;
    wdata[2] = (addr) >> 8;
    wdata[3] = (addr) >> 0;

    W25Q64_WaitBusy();
    W25Q64_Enable();
    CS_ENABLE;
    SPI1_Write(wdata, 4);
    CS_DISENABLE;
    W25Q64_WaitBusy();                 /* 4KB 擦除典型 45ms */
}

void W25Q64_PageWrite(uint8_t *wbuff, uint16_t pageNB)
{
    uint8_t wdata[4];

    wdata[0] = 0x02;                       /* Page Program */
    wdata[1] = (pageNB * 256) >> 16;       /* 24位地址：页号×256 = 页首地址 */
    wdata[2] = (pageNB * 256) >> 8;
    wdata[3] = (pageNB * 256) >> 0;

    W25Q64_WaitBusy();
    W25Q64_Enable();                       /* 写操作前必须 Write Enable */
    CS_ENABLE;
    SPI1_Write(wdata, 4);                  /* 命令 + 24位地址 */
    SPI1_Write(wbuff, 256);                /* 一整页 256 字节数据 */
    CS_DISENABLE;
}





    
