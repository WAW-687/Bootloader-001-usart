#ifndef __USART_H
#define __USART_H

#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#define U1_RX_SIZE		2048		//串口接收缓冲区长度
#define U1_TX_SIZE		2048		
#define U1_RX_MAX		256			//单次接受最大量
#define NUM				10
typedef struct{
	uint8_t *start;
	uint8_t *end;
}UCB_URxBuffptr;

typedef struct{
	uint16_t URxCounter;
	UCB_URxBuffptr URxDataPtr[NUM];
	UCB_URxBuffptr *URxDataIN;
	UCB_URxBuffptr *URxDataOUT;
	UCB_URxBuffptr *URxDataEND;
}UCB_CB;

extern UCB_CB	U1CB;
extern uint8_t U1_RxBuff[U1_RX_SIZE];

void Usart1_Init(uint32_t bandrate);
void U1Rx_PtrInit(void);
void u1_printf(char *format,...);



#endif
