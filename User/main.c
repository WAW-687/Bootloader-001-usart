#include "stm32f10x.h"                  // Device header
#include "usart.h"

uint16_t i;

int main(void)
{
	Usart1_Init(115200);
	u1_printf("%d %c %x",0x30,0x30,0x30);
	
	while (1)
	{
		if (U1CB.URxDataOUT != U1CB.URxDataIN)
		{
			u1_printf("本次接收了%d字节数据\r\n", U1CB.URxDataOUT->end - U1CB.URxDataOUT->start + 1);
			for (i = 0; i < U1CB.URxDataOUT->end - U1CB.URxDataOUT->start + 1; i++)
				u1_printf("%c", U1CB.URxDataOUT->start[i]);
			u1_printf("\r\n\r\n");

			U1CB.URxDataOUT++;
			if (U1CB.URxDataOUT == U1CB.URxDataEND)
			{
				U1CB.URxDataOUT = &U1CB.URxDataPtr[0];
			}
		}
	}

}

