/**
  ******************************************************************************
  * @file    my_spi.c
  * @brief   SPI1 硬件驱动（主机模式，驱动外部 Flash W25Q64）
  * @note    引脚分配（SPI1 默认复用）：
  *          PA5 - SCK   复用推挽
  *          PA6 - MISO  浮空输入
  *          PA7 - MOSI  复用推挽
  *          PA4 - CS    普通推挽（软件片选，在 w25q64.c 中控制）
  ******************************************************************************
  */
#include "stm32f10x.h"                  // Device header
#include "my_spi.h"


/**
 * @brief  SPI1 初始化（主机、8位、模式0、MSB先行、2分频=36MHz）
 * @note   NSS 用软件控制（PA4 普通GPIO），不占用硬件 NSS 引脚
 */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* PA5 SCK、PA7 MOSI：复用推挽 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA6 MISO：浮空输入 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_I2S_DeInit(SPI1);

    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; 
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;                
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;              
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;                 
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;                 
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;                    
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;        
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;               
    SPI_InitStructure.SPI_CRCPolynomial     = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

/**
 * @brief  SPI1 收发一个字节（全双工交换）
 * @param  txdata 发送的字节
 * @retval 收到的字节（SPI 是全双工的，发的同时必然收）
 * @note   读操作时传 0xFF 哑字节，只为产生时钟把数据移进来
 */
uint8_t SPI1_ReadWriteByte(uint8_t txdata)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   /* 等发送缓冲空 */

    SPI_I2S_SendData(SPI1, txdata);

    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);  /* 等接收缓冲非空 */

    return SPI_I2S_ReceiveData(SPI1);
}

/**
 * @brief  连续发送 datalen 个字节（用于下发命令/地址/数据）
 */
void SPI1_Write(uint8_t *wdata, uint16_t datalen)
{
    uint16_t i;
    for(i = 0; i < datalen; i++)
    {
        SPI1_ReadWriteByte(wdata[i]);
    }
}

/**
 * @brief  连续接收 datalen 个字节（发 0xFF 哑字节产生时钟）
 */
void SPI1_Read(uint8_t *rdata, uint16_t datalen)
{
    uint16_t i;
    for(i = 0; i < datalen; i++)
    {
        rdata[i] = SPI1_ReadWriteByte(0xFF);   /* 读时发哑字节产生时钟 */
    }
}







    
