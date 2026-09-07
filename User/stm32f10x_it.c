/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "usart.h"


/**
 * @brief  USART1 空闲中断 + DMA 接收不定长数据
 */
void USART1_IRQHandler(void)   /* GD32 的 USART0_IRQHandler → STM32 的 USART1_IRQHandler */
{
    /* usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE) */
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        /* usart_flag_get(USART0, USART_FLAG_IDLEF) : 先读 SR */
        (void)USART_GetFlagStatus(USART1, USART_FLAG_IDLE);

        /* usart_data_receive(USART0) : 再读 DR，SR+DR 各读一次即清除 IDLE 标志 */
        (void)USART_ReceiveData(USART1);

        /* dma_transfer_number_get(DMA0, DMA_CH4) */
        U1CB.URxCounter += (U1_RX_MAX + 1) - DMA_GetCurrDataCounter(DMA1_Channel5);

        U1CB.URxDataIN->end = &U1_RxBuff[U1CB.URxCounter - 1];
        U1CB.URxDataIN++;
        if (U1CB.URxDataIN == U1CB.URxDataEND)
        {
            U1CB.URxDataIN = &U1CB.URxDataPtr[0];
        }

        if (U1_RX_SIZE - U1CB.URxCounter >= U1_RX_MAX)
        {
            U1CB.URxDataIN->start = &U1_RxBuff[U1CB.URxCounter];
        }
        else
        {
            U1CB.URxDataIN->start = U1_RxBuff;
            U1CB.URxCounter = 0;
        }

        /* dma_channel_disable(DMA0, DMA_CH4) */
        DMA_Cmd(DMA1_Channel5, DISABLE);

        /* dma_transfer_number_config(DMA0, DMA_CH4, U0_RX_MAX+1) */
        DMA_SetCurrDataCounter(DMA1_Channel5, U1_RX_MAX + 1);

        /* dma_memory_address_config(DMA0, DMA_CH4, ...) : STM32 标准库没有对应封装，直接写 CMAR 寄存器 */
        DMA1_Channel5->CMAR = (uint32_t)U1CB.URxDataIN->start;

        /* dma_channel_enable(DMA0, DMA_CH4) */
        DMA_Cmd(DMA1_Channel5, ENABLE);
    }
}















/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
