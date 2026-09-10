/**
  ******************************************************************************
  * @file    usart.c
  * @brief   USART1 驱动（115200 8N1）+ DMA1_Channel5 不定长接收 + u1_printf
  * @note    引脚：PA9-TX（复用推挽）、PA10-RX（浮空输入）。
  *          接收方案：DMA 循环写入 U1_RxBuff，USART1 空闲中断（IDLE）判定一帧
  *          结束，帧的起止位置记入 U1CB 环形帧队列，主循环从 OUT 指针取帧。
  *          由 GD32 USART0+DMA 课程代码移植而来（CH4→CH5，函数名对应替换）。
  ******************************************************************************
  */
#include "stm32f10x.h"                  // Device header
#include "usart.h"

uint8_t U1_RxBuff[U1_RX_SIZE];
uint8_t U1_TxBuff[U1_TX_SIZE];
UCB_CB U1CB;

/**
 * @brief  串口1初始化：GPIO + USART1(8N1) + NVIC + DMA1通道5 + 空闲中断
 * @param  bandrate 波特率（本工程固定传 115200）
 */
void Usart1_Init(uint32_t bandrate)
{
	/* 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	/* GPIO 初始化 */
    /* PA9  - USART1_TX：复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* PA10 - USART1_RX：浮空输入 */
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	 /* USART1 参数配置（8N1、无流控、收发都开） */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate 				= bandrate;
    USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode 					= USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity 				= USART_Parity_No;
    USART_InitStructure.USART_StopBits 				= USART_StopBits_1;
    USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

	/* 5. NVIC 配置：PRE2_SUB2 = 2 位抢占优先级 + 2 位响应优先级 */
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	/* DMA1_Channel5 接收配置（USART1_RX 对应 DMA1 Channel5） */
    DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t)U1_RxBuff;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_BufferSize         = U1_RX_MAX + 1;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure.DMA_PeripheralInc 	 = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
	
	/* 使能 USART1 的 DMA 接收请求 + 启动 DMA */
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);
	
	/* 使能空闲中断（配合 DMA 实现不定长接收） */
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	
	U1Rx_PtrInit();
	
	/* 使能 USART1 */
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief  帧队列指针初始化：IN/OUT 都归零号槽，游标清零
 * @note   在 Usart1_Init 末尾调用一次；上电后第一帧数据落在 U1_RxBuff[0]
 */
void U1Rx_PtrInit(void)
{
	U1CB.URxDataIN = &U1CB.URxDataPtr[0];
	U1CB.URxDataOUT = &U1CB.URxDataPtr[0];
	U1CB.URxDataEND = &U1CB.URxDataPtr[NUM - 1];
	U1CB.URxDataIN->start = U1_RxBuff;
	U1CB.URxCounter = 0;
}


/**
 * @brief  串口1格式化打印（类似 printf，阻塞发送）
 * @note   vsprintf 先在 U1_TxBuff 里拼好字符串，再逐字节查 TXE 标志发送，
 *         最后等 TC 发送完成标志——保证函数返回后数据已完整发出。
 *         注意缓冲区 2048 字节上限，超长格式化会溢出。
 */
void u1_printf(char *format, ...)
{
    uint16_t i;
    va_list listdata;

    va_start(listdata, format);
    vsprintf((char *)U1_TxBuff, format, listdata);
    va_end(listdata);

    for (i = 0; i < strlen((const char *)U1_TxBuff); i++)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, U1_TxBuff[i]);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}










    
