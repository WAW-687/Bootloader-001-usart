#include "stm32f10x.h"                  // Device header
#include "my_spi.h"
#include "w25q64.h"


void W25Q64_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* PA4 - CS：推挽输出，软件控制片选 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CS_DISENABLE;       /* 片选默认拉高（不选中） */
    SPI1_Init();
}

/**
 * @brief  等待芯片空闲（读状态寄存器1的BUSY位）
 * @retval W25Q64_OK 空闲 / W25Q64_ERR_TIMEOUT 超时（芯片未接或异常）
 */
uint8_t W25Q64_WaitBusy(void)
{
    uint32_t timeout = W25Q64_BUSY_TIMEOUT;
    uint8_t  res;

    do{
        CS_ENABLE;
        SPI1_ReadWriteByte(0x05);          /* Read Status Register-1 */
        res = SPI1_ReadWriteByte(0xff);
        CS_DISENABLE;
        timeout--;
    }while(((res & 0x01) == 0x01) && (timeout != 0));

    if(timeout == 0)
        return W25Q64_ERR_TIMEOUT;
    return W25Q64_OK;
}

/**
 * @brief  写使能（所有擦写操作前必须调用）
 */
uint8_t W25Q64_Enable(void)
{
    uint8_t ret;

    ret = W25Q64_WaitBusy();
    if(ret != W25Q64_OK)
        return ret;

    CS_ENABLE;
    SPI1_ReadWriteByte(0x06);              /* Write Enable */
    CS_DISENABLE;
    return W25Q64_OK;
}

/**
 * @brief  读 JEDEC ID（上电自检用）
 * @param  id 读出的24位ID，正常应为 0xEF4017
 */
uint8_t W25Q64_ReadID(uint32_t *id)
{
    uint8_t ret;

    *id = 0;

    ret = W25Q64_WaitBusy();
    if(ret != W25Q64_OK)
        return ret;

    CS_ENABLE;
    SPI1_ReadWriteByte(0x9F);              /* JEDEC ID */
    *id  = (uint32_t)SPI1_ReadWriteByte(0xff) << 16;   /* Manufacturer: 0xEF */
    *id |= (uint32_t)SPI1_ReadWriteByte(0xff) << 8;    /* Memory Type:  0x40 */
    *id |= (uint32_t)SPI1_ReadWriteByte(0xff);         /* Capacity:     0x17 */
    CS_DISENABLE;
    return W25Q64_OK;
}

/**
 * @brief  64KB 块擦除
 * @param  blockNB 块号 0~127
 */
uint8_t W25Q64_Erase64K(uint8_t blockNB)
{
    uint8_t wdata[4];
    uint8_t ret;

    wdata[0] = 0xD8;                       /* Block Erase 64KB */
    wdata[1] = (blockNB * 64 * 1024) >> 16;
    wdata[2] = (blockNB * 64 * 1024) >> 8;
    wdata[3] = (blockNB * 64 * 1024) >> 0;

    ret = W25Q64_Enable();                 /* 擦除前必须先 Write Enable */
    if(ret != W25Q64_OK)
        return ret;

    CS_ENABLE;
    SPI1_Write(wdata, 4);
    CS_DISENABLE;

    return W25Q64_WaitBusy();              /* 64KB 擦除典型 150ms，等完 */
}

/**
 * @brief  4KB 扇区擦除
 * @param  addr 扇区首地址（须 4096 对齐，如 0x7FF000）
 */
uint8_t W25Q64_Erase4K(uint32_t addr)
{
    uint8_t wdata[4];
    uint8_t ret;

    wdata[0] = 0x20;                       /* Sector Erase 4KB */
    wdata[1] = addr >> 16;
    wdata[2] = addr >> 8;
    wdata[3] = addr;

    ret = W25Q64_Enable();
    if(ret != W25Q64_OK)
        return ret;

    CS_ENABLE;
    SPI1_Write(wdata, 4);
    CS_DISENABLE;

    return W25Q64_WaitBusy();              /* 4KB 擦除典型 45ms */
}

/**
 * @brief  页编程（一次写满一页 256 字节，页内不能跨页）
 * @param  wbuff 数据缓冲区（至少256字节）
 * @param  pageNB 页号 0~32767
 */
uint8_t W25Q64_PageWrite(uint8_t *wbuff, uint16_t pageNB)
{
    uint8_t wdata[4];
    uint8_t ret;

    wdata[0] = 0x02;                       /* Page Program */
    wdata[1] = (pageNB * 256) >> 16;       /* 24位地址：页号x256 = 页首地址 */
    wdata[2] = (pageNB * 256) >> 8;
    wdata[3] = (pageNB * 256) >> 0;

    ret = W25Q64_Enable();                 /* 写操作前必须 Write Enable */
    if(ret != W25Q64_OK)
        return ret;

    CS_ENABLE;
    SPI1_Write(wdata, 4);                  /* 命令 + 24位地址 */
    SPI1_Write(wbuff, 256);                /* 一整页 256 字节数据 */
    CS_DISENABLE;

    return W25Q64_WaitBusy();              /* 页编程典型 0.4ms */
}

/**
 * @brief  读数据
 * @param  rbuff   读出缓冲区
 * @param  addr    起始地址（0~0x7FFFFF）
 * @param  datalen 读取长度（最大 65535，一次连续读不超过 64KB）
 */
uint8_t W25Q64_Read(uint8_t *rbuff, uint32_t addr, uint16_t datalen)
{
    uint8_t wdata[4];
    uint8_t ret;

    wdata[0] = 0x03;                       /* Read Data */
    wdata[1] = addr >> 16;
    wdata[2] = addr >> 8;
    wdata[3] = addr;

    ret = W25Q64_WaitBusy();
    if(ret != W25Q64_OK)
        return ret;

    CS_ENABLE;
    SPI1_Write(wdata, 4);                  /* 命令 + 24位地址 */
    SPI1_Read(rbuff, datalen);             /* 连续读数据 */
    CS_DISENABLE;
    return W25Q64_OK;
}



