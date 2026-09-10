/**
  ******************************************************************************
  * @file    main.c
  * @brief   OTA Bootloader 主程序（Bootloader 区，0x08000000~0x08004FFF）
  * @note    上电流程（BootLoader_Branch 分流）：
  *          ① 2s 内敲 'w' → 命令行模式，菜单 [1]~[7] 手动操作
  *          ② W25Q64 元数据里 OTA_flag==0xAABB1122 → 主循环执行固件搬运
  *          ③ 否则 → LOAD_A 跳转 App 区（0x08005000）
  *          全局控制块 OTA_Info / UpDataA / BootStaFlag 定义在本文件。
  ******************************************************************************
  */
#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"
#include "my_spi.h"
#include "w25q64.h"
#include "flash.h"
#include "main.h"
#include "boot.h"

OTA_InfoCB OTA_Info;
UpDataA_CB UpDataA;
uint32_t BootStaFlag = 0;           

uint8_t aaa[5]={1,2,3,4,5}; 

int main(void)
{
    uint8_t i;

    Usart1_Init(115200);               
    W25Q64_Init();                      

    W25Q64_ReadOTAInfo();   
	
    BootLoader_Branch();

    /* ===== 命令行模式：开机500ms内敲 'w' 进入（提取自课程代码，U0CB → U1CB） =====
     * 进命令行后不再往下走，敲 [7] 重启是唯一出口；*/

    while(1)
    {
		Delay_ms(10);
		if(U1CB.URxDataOUT != U1CB.URxDataIN)
		{
			BootLoader_Event(U1CB.URxDataOUT->start,
							 U1CB.URxDataOUT->end - U1CB.URxDataOUT->start + 1);
			U1CB.URxDataOUT++;
			if(U1CB.URxDataOUT == U1CB.URxDataEND)
			{
				U1CB.URxDataOUT = &U1CB.URxDataPtr[0];
			}
		}
		if(BootStaFlag & IAP_XMODEMC_FLAG){        // 已通过菜单[2]进入Xmodem接收模式
			if(UpDataA.XmodemTimer >= 100)
			{      
				u1_printf("C");                    // 发邀请字符
				UpDataA.XmodemTimer = 0;
			}
			UpDataA.XmodemTimer++;                // 每圈+1
		}

		
        if(BootStaFlag & UPDATA_A_FLAG)
        {
            /* 更新A区：把 W25Q64 里 BlockNB 号 64KB 块的固件搬进内部 Flash App 区 */
            u1_printf("长度%d字节\r\n", OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]);

            if(OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 4 == 0)   /* 长度必须按字(4字节)对齐 */
            {
                /* 擦整个 App 区（44页），保证 Flash"先擦后写" */
                STM32_EraseFlash(STM32_A_START_PAGE, STM32_A_PAGE_NUM);

                /* 整页部分：Firelen/1024 个完整页，逐页从 W25Q64 读出写入 */
                for(i = 0; i < OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] / STM32_PAGE_SIZE; i++)
                {
                    W25Q64_Read(UpDataA.Updatabuff,
                                i*1024 + UpDataA.W25Q64_BlockNB*64*1024,
                                STM32_PAGE_SIZE);
                    STM32_WriteFlash(STM32_A_SADDR + i*STM32_PAGE_SIZE,
                                     (uint32_t *)UpDataA.Updatabuff,
                                     STM32_PAGE_SIZE);
                }

                /* 不足一页的尾巴（for 结束后 i 正好 = 完整页数） */
                if(OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 1024 != 0)
                {
                    W25Q64_Read(UpDataA.Updatabuff,
                                i*1024 + UpDataA.W25Q64_BlockNB*64*1024,
                                OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 1024);
                    STM32_WriteFlash(STM32_A_SADDR + i*STM32_PAGE_SIZE,
                                     (uint32_t *)UpDataA.Updatabuff,
                                     OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 1024);
                }

                /* 更新 0 号块（主固件）后清 OTA 标志：复位后就直接跳 A 区跑新固件 */
                if(UpDataA.W25Q64_BlockNB == 0)
                {
                    OTA_Info.OTA_flag = 0;
                    W25Q64_WriteOTAInfo();       /* M24C02_WriteOTAInfo → W25Q64 版 */
                }
                u1_printf("更新完成\r\n");
                NVIC_SystemReset();              /* 复位重启。原课程代码此 if 块缺一个右括号导致配对错乱，已修正 */
            }
            else
            {
                u1_printf("长度错误\r\n");
                BootStaFlag &= ~UPDATA_A_FLAG;   /* 长度非法，退出更新状态停在原地 */
            }
        }
    }
}
