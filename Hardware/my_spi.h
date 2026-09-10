#ifndef __MYSPI_H
#define __MYSPI_H

/* ===================== SPI1 硬件驱动对外接口 ===================== */
void SPI1_Init(void);                                       /* SPI1 初始化（主机/模式0/36MHz） */
uint8_t SPI1_ReadWriteByte(uint8_t txdata);                 /* 收发一个字节（读时传0xFF哑字节） */
void SPI1_Write(uint8_t *wdata, uint16_t datalen);          /* 连续发送 */
void SPI1_Read(uint8_t *rdata, uint16_t datalen);           /* 连续接收（发哑字节出时钟） */



#endif
