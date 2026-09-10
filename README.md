# STM32F103 OTA Bootloader 项目（B-002-SPI）

基于 STM32F103C8T6 + W25Q64 的串口 IAP / OTA Bootloader 项目，支持命令行交互、
Xmodem 协议下载固件、内部 Flash / 外部 Flash 双存储搬运更新。
由 GD32工程移植到 STM32F103 标准外设库，已退役 AT24C02（I2C 方案），
元数据统一存放在 W25Q64。

---

## 1. 硬件环境

### 1.1 主控与外设

| 器件 | 型号/规格 | 说明 |
|---|---|---|
| MCU | STM32F103C8T6 | 64KB Flash（64页×1KB）、20KB RAM、72MHz |
| 外部 Flash | W25Q64 | 8MB SPI NOR Flash，存固件镜像与 OTA 元数据 |
| 调试器 | ST-Link V2 | 烧录 / 调试 |
| 串口 | USART1（PA9-TX / PA10-RX） | 115200-8-N-1，USB 转 TTL 连电脑 |

### 1.2 引脚分配

| 引脚 | 功能 | 模式 |
|---|---|---|
| PA4 | W25Q64 CS（片选） | 推挽输出（软件控制） |
| PA5 | SPI1 SCK | 复用推挽 |
| PA6 | SPI1 MISO | 浮空输入 |
| PA7 | SPI1 MOSI | 复用推挽 |
| PA9 | USART1 TX | 复用推挽 |
| PA10 | USART1 RX | 浮空输入 |

---

## 2. 软件编译环境

| 项目 | 版本/说明 |
|---|---|
| IDE | Keil MDK 5（µVision） |
| 编译器 | ARM Compiler 5（AC5）；中文警告已用 `--diag_suppress=870` 屏蔽 |
| 固件库 | STM32F10x 标准外设库 V3.5 |
| 器件包 | Keil.STM32F1xx_DFP（STM32F103C8） |
| bin 生成 | `fromelf.exe`（随 Keil 安装，位于 `ARM\ARMCC\bin\`） |
| 串口终端 | SecureCRT（Xterm 模式，需勾选 New line mode）或任意串口助手 |
| 版本管理 | Git + GitHub（Watt Toolkit 网络环境下 push 需 `git -c http.sslVerify=false push`） |

### 2.1 bin 文件生成设置（App 工程需要）

`Options for Target → User → After Build/Rebuild → Run #1` 勾选并填入：

```
C:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin -o $L@L.bin #L
```

编译后在 Output 目录生成与 .axf 同名的 .bin，用于 Xmodem 下载。

---

## 3. Flash 分区布局

### 3.1 内部 Flash（STM32，64KB）

| 分区 | 地址范围 | 大小 | 用途 |
|---|---|---|---|
| Bootloader 区 | 0x08000000 ~ 0x08004FFF | 20 页（20KB） | 本工程，开机引导 |
| App 区 | 0x08005000 ~ 0x0800FFFF | 44 页（44KB） | 用户应用程序 |

App 工程设置：IROM1 起始 `0x8005000`、大小 `0xB000`；
`system_stm32f10x.c` 中 `VECT_TAB_OFFSET = 0x5000`（中断向量表重定位）。

### 3.2 外部 Flash（W25Q64，8MB）

| 区域 | 地址范围 | 说明 |
|---|---|---|
| 固件块区 | 0x000000 ~ 0x0EFFFF | 9 个 64KB 块（块 0~8），块号 1-9 供命令行选用 |
| 元数据扇区 | 0x7FF000 ~ 0x7FFFFF | 最后 4KB，存放 OTA_InfoCB（OTA_flag / Firelen / OTA_ver） |

---

## 4. 工程结构

```
B-002-SPI/
├── User/
│   ├── main.c / main.h        主程序：标志位、分区宏、OTA搬运主循环
│   └── stm32f10x_it.c         USART1空闲中断（DMA不定长收帧）
├── Hardware/
│   ├── usart.c/h              USART1驱动 + DMA收帧队列 + u1_printf
│   ├── my_spi.c/h             SPI1 硬件驱动（主机模式）
│   ├── w25q64.c/h             W25Q64 命令层（读ID/擦除/页编程/读数据）
│   ├── flash.c/h              内部 Flash 擦除与按字编程
│   └── boot.c/h               Bootloader核心：分流/命令行/Xmodem/CRC16/跳转
├── System/
│   └── Delay.c/h              SysTick 阻塞延时（us/ms/s）
└── Output/                    编译输出（.axf / .hex / .bin）
```

---

## 5. 编译与烧录

1. 用 Keil 打开工程，确认编译器为 **AC5**（Options → Target → ARM Compiler: Use default compiler version 5）；
2. `F7` 编译，`F8` 全部重建，0 Error 通过；
3. ST-Link 连接后 `F8` 旁的 Load 按钮下载（Bootloader 工程直接烧 0x08000000）；
4. App 工程编译生成 bin（见 2.1 节），**不要直接下载覆盖 Bootloader**。

> 注意：App 烧录调试时需在 Debug 设置里勾选 "Erase Sectors" 而非整片擦除，
> 并在 Flash Download 里只保留 0x08005000 起的 App 区编程算法，避免抹掉 Bootloader。

---

## 6. 串口命令行使用

### 6.1 连接

SecureCRT：Serial → COMx → 115200、8-N-1、**流控全部关闭**；
Terminal → Emulation → Modes → 勾选 **New line mode**（否则 \r\n 显示/发送异常）。

### 6.2 命令流程

1. 复位或上电，串口输出 `2000ms内，输入小写字母w，进入BootLoader命令行`；
2. 立即敲 `w` 进入命令行菜单：

```
[1]擦除A区            清空 App 区（危险操作，擦后需重下固件）
[2]串口IAP下载A区程序  Xmodem-CRC 直接下载到内部 App 区，完成后自动重启
[3]设置OTA版本号       发送 VER=1.0.0-2026/09/10-12:00（26字节，含回车）
[4]查询OTA版本号       读回元数据里的版本字符串
[5]向外部Flash下载程序 输入块号(1-9) → Xmodem-CRC 下载到 W25Q64 对应 64KB 块
[6]使用外部Flash内程序 输入块号(1-9) → 把 W25Q64 块内容搬运到 App 区并重启
[7]重启               软复位
```

### 6.3 Xmodem 下载操作（[2]/[5] 通用）

1. 选菜单后板子每秒输出一个 `C`（Xmodem-CRC 邀请字符）；
2. SecureCRT 菜单 `Transfer → Send Xmodem...`，勾选 **CRC 模式**，选择 App 工程生成的 **.bin 文件**；
3. 传输中每包 133 字节（SOH+包号+补码+128数据+CRC16），CRC 校验失败自动 NAK 重传；
4. 收到 EOT 后板子回 ACK，[2] 路径自动重启进新固件，[5] 路径回菜单。

### 6.4 典型 OTA 流程（[5] + [6]）

```
烧录新App.bin → [5]选块号下载到W25Q64 → 断电随便玩 → [6]选同一块号
→ 搬运到内部App区 → 自动复位 → 新固件运行
```

---

## 7. 已知注意事项

- **App 区被擦后** LOAD_A 校验失败会提示"跳转A区失败"，属正常保护，[2] 重下即可；
- **同一 W25Q64 块重复下载**会先整块擦除（约1~2秒），串口有"正在擦除"提示；
- `OTA_flag == 0xAABB1122`（OTA_SET_FLAG）时上电自动进搬运流程；正常使用请保持该魔数不在元数据中；
- [5] 下载中断后该块 `Firelen=0`，直接 [6] 选它会把 App 区擦光——下载失败请重新 [5]；
- W25Q64 与 ESP-01S 共用 3.3V 电源时，ESP 发射瞬间可能拉低电压干扰 DHT11 采样，建议各自加大电容或分路供电。

---

## 8. 后续规划

- ESP-01S WiFi 接收固件（HTTP/自定义协议 → W25Q64）；
- OTA 版本比对与自动更新策略；
- 低功耗模式与 P-MOS 电源开关（STM32 GPIO 控制 ESP-01S 上下电）。
