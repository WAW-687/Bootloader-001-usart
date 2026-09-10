#ifndef __USART_H
#define __USART_H

#include "stdarg.h"
#include "stdio.h"
#include "string.h"

/* ===================== 串口接收参数 ===================== */
#define U1_RX_SIZE		2048		//串口接收缓冲区总长度（环形存放多帧数据）
#define U1_TX_SIZE		2048		//发送格式化缓冲区长度（u1_printf 用）
#define U1_RX_MAX		256			//单帧最大接收量（DMA 单次搬运上限）
#define NUM				10			//帧描述符槽位数（环形队列深度）

/* 帧描述符：记录一帧数据在 U1_RxBuff 里的起止位置 */
typedef struct{
	uint8_t *start;                     //本帧首字节地址
	uint8_t *end;                       //本帧末字节地址（含）
}UCB_URxBuffptr;

/* 串口控制块：DMA+空闲中断不定长接收的环形帧队列 */
typedef struct{
	uint16_t URxCounter;                //累计写入字节数（也是下一帧的写入游标）
	UCB_URxBuffptr URxDataPtr[NUM];     //帧描述符数组（环形）
	UCB_URxBuffptr *URxDataIN;          //写指针：中断里指向下一空槽
	UCB_URxBuffptr *URxDataOUT;         //读指针：主循环从这里取完整帧
	UCB_URxBuffptr *URxDataEND;         //环形回卷点（指向最后一个槽）
}UCB_CB;

extern UCB_CB	U1CB;                   //串口1控制块（定义在 usart.c）
extern uint8_t U1_RxBuff[U1_RX_SIZE];   //接收数据总缓冲（DMA 目标区）

void Usart1_Init(uint32_t bandrate);    //串口1初始化（115200 8N1 + DMA + 空闲中断）
void U1Rx_PtrInit(void);                //帧队列指针初始化
void u1_printf(char *format,...);       //串口1格式化打印（阻塞发送）



#endif
