# ESP32 Bit Pirate — 项目完整文档

> 本文档仅保存在 `i/` 目录，**不推送**到 GitHub。

---

## 1. 项目概述

**ESP32 Bit Pirate** 是一款开源固件，将 ESP32-S3 设备打造成多协议黑客工具，灵感来自 [Bus Pirate](https://buspirate.com/)。

支持通过 **串口终端** 或 **Web 浏览器** 进行交互式命令行操作，涵盖 20+ 种数字/无线协议。

---

## 2. 支持的硬件

| 型号 | 环境名 | 屏幕 | 输入 |
|------|--------|------|------|
| ESP32-S3 DevKit (N16R8) | `s3-devkit-n16-r8` | 无 | Boot 按钮 |
| ESP32-S3 DevKit (通用) | `s3-devkit` | 无 | Boot 按钮 |
| M5Stack Cardputer | `cardputer` / `cardputer-adv` | 有 | 键盘 |
| M5Stack Stick S3 | `m5stack-sticks3` | 有 | 按键 |
| M5Stack StampS3 / AtomS3 | `m5stack-stamps3` / `atom-lite-s3` | 无 | 按键 |
| T-Embed S3 | `t-embed-s3` / `t-embed-s3-cc1101` | 有 | 按键+编码器 |
| T-Display S3 | `t-display-s3` | 有 | 按键 |
| Waveshare ESP32-S3-GEEK | `waveshare-s3-geek` | 有 | 按键 |
| Xiao ESP32-S3 | `xiao-esp32s3` | 无 | 按键 |
| Heltec WiFi LoRa 32 V3 | `heltec_wifi_lora_32_V3` | 有 | 按键 |

**本项目仅针对 `s3-devkit-n16-r8` (ESP32-S3 N16R8) 进行测试。**

### ESP32-S3 N16R8 硬件规格

| 参数 | 值 |
|------|-----|
| CPU | XTensa LX7 双核 @ 240MHz |
| Flash | 16MB (Quad SPI) |
| PSRAM | 8MB (Octal PSRAM) |
| USB | USB OTG (TinyUSB) + USB Serial/JTAG |
| UART | 3 路 (UART0 调试, UART1/UART2 可用) |
| GPIO | 多个可用引脚 (具体见各模式配置) |

---

## 3. 项目目录结构

```
/workspace/
├── .github/workflows/       # GitHub Actions CI/CD
│   └── build.yml            # PlatformIO 编译工作流
├── partitions/              # 分区表配置
│   ├── app4M_spiffs_4M_8MB.csv
│   └── app4M_spiffs_12M_16MB.csv
├── lib/                     # 第三方库
│   ├── 93cx6/               # EEPROM 93Cx6 驱动
│   ├── EEPROM_SPI_WE/       # SPI EEPROM 库
│   ├── IRremote/            # 红外遥控库
│   ├── PN532/               # NFC/RFID 库
│   ├── RF24/                # nRF24L01 无线库
│   └── SmartRC-CC1101-Driver-Lib/  # CC1101 Sub-GHz 驱动
├── src/                     # 源代码
│   ├── main.cpp             # 程序入口点
│   ├── Abstracts/           # 抽象基类
│   ├── Adapters/            # USB/调试适配器
│   ├── Analyzers/           # 分析器
│   ├── Config/              # 启动配置
│   ├── Controllers/         # 命令处理器 (每个模式一个)
│   ├── Data/                # 静态数据/数据库
│   ├── Dispatchers/         # 命令分发
│   ├── Enums/               # 枚举定义
│   ├── Inputs/              # 输入处理
│   ├── Interfaces/          # 接口定义
│   ├── Managers/            # 管理器
│   ├── Models/              # 数据模型
│   ├── Providers/           # 依赖注入
│   ├── Selectors/           # UI 选择器
│   ├── Serial/              # 串口实现
│   ├── Servers/             # 网络服务器
│   ├── Services/            # 核心服务
│   ├── Shells/              # 辅助 Shell
│   ├── States/              # 全局状态
│   ├── Transformers/        # 数据转换器
│   ├── Vendors/             # 供应商辅助代码
│   └── Views/               # 显示视图
├── test/                    # 单元测试
├── webui/                   # Web UI 资源
│   ├── index.h              # HTML 页面
│   ├── scripts.h            # JS 脚本
│   └── style.h              # CSS 样式
├── images/                  # 文档图片
├── i/                       # 内部文档 (不推送)
├── platformio.ini           # PlatformIO 配置
├── README.md                # 项目 README
└── .gitignore
```

---

## 4. 源码结构详解

### 4.1 Abstracts (抽象基类)

| 文件 | 说明 |
|------|------|
| `ANetworkController.h/.cpp` | 网络控制器抽象类 (WiFi/Ethernet 共用) |

### 4.2 Adapters (适配器)

| 文件 | 说明 |
|------|------|
| `AvrDudeBusPirateAdapter` | AVR 烧录器 (Bus Pirate 协议) |
| `Bpio2Adapter` | BPIO2 协议适配 |
| `FlashromSerprogAdapter` | Flashrom 串行编程器 |
| `InfraredToyAdapter` | 红外玩具适配 |
| `OpenOcdBusPirateAdapter` | OpenOCD 调试适配器 |
| `SubGhzRawCdcAdapter` | Sub-GHz 原始 CDC 适配 |
| `SumpLogicAnalyzerAdapter` | SUMP 逻辑分析仪 |
| `UsbUartBridgeAdapter` | USB-UART 桥接 |

### 4.3 Analyzers (分析器)

| 文件 | 说明 |
|------|------|
| `BinaryAnalyzer` | 二进制数据分析 |
| `PinAnalyzer` | GPIO 引脚分析 |
| `SubGhzAnalyzer` | Sub-GHz 信号分析 |

### 4.4 Config (启动配置)

| 文件 | 说明 |
|------|------|
| `BootModeConfigurator` | 启动模式选择 (正常/USB 适配器) |
| `TerminalTypeConfigurator` | 终端类型选择 (串口/WiFi Web/独立) |
| `WifiTypeConfigurator` | WiFi 模式配置 (客户端/AP) |

### 4.5 Controllers (命令处理器) — **核心**

每个模式对应一个 Controller，负责解析和执行该模式下的 CLI 命令。

| Controller | 对应模式 | 主要命令 |
|------------|----------|----------|
| `UtilityController` | 全局 | `help`, `mode`, `system`, `profile`, `alias`, `hex`, `logic`, `P/p` |
| `OneWireController` | 1WIRE | `scan`, `ping`, `sniff`, `read`, `write`, `temp`, `ibutton`, `eeprom` |
| `UartController` | UART | `scan`, `autobaud`, `ping`, `sniff`, `read`, `write`, `bridge`, `at`, `emulator`, `spam`, `xmodem` |
| `HdUartController` | HDUART | `bridge` |
| `I2cController` | I2C | `scan`, `discovery`, `ping`, `identify`, `sniff`, `slave`, `read`, `write`, `dump`, `glitch`, `flood`, `eeprom` |
| `SpiController` | SPI | `sniff`, `sdcard`, `slave`, `flash`, `eeprom` |
| `TwoWireController` | 2WIRE | `sniff`, `smartcard` |
| `ThreeWireController` | 3WIRE | `eeprom` |
| `DioController` | DIO | `scan`, `pins`, `sniff`, `read`, `set`, `pulse`, `servo`, `pwm`, `toggle`, `jam` |
| `LedController` | LED | `fill`, `set`, `blink`, `rainbow`, `chase`, `cycle`, `wave` |
| `InfraredController` | INFRARED | `send`, `receive`, `devicebgone`, `remote`, `record`, `replay`, `load` |
| `UsbS3Controller` | USB | `storage`, `keyboard`, `mouse`, `gamepad`, `host`, `adapters` |
| `BluetoothController` | BLUETOOTH | `scan`, `pair`, `sniff`, `spoof`, `server`, `keyboard`, `mouse` |
| `WifiController` | WIFI | `scan`, `connect`, `ping`, `sniff`, `deauth`, `ap`, `spam`, `flood`, `ssh`, `telnet`, `nc`, `nmap`, `http` |
| `JtagController` | JTAG | `scan swd`, `scan jtag`, `openocd` |
| `I2sController` | I2S | `play`, `record`, `test` |
| `CanController` | CAN | `sniff`, `send`, `receive`, `status` |
| `EthernetController` | ETHERNET | `connect`, `ping`, `ssh`, `telnet`, `nc`, `nmap`, `modbus` |
| `SubGhzController` | SUBGHZ | `scan`, `sweep`, `send`, `receive`, `replay`, `jam`, `bruteforce`, `waterfall`, `record` |
| `RfidController` | RFID | `read`, `write`, `clone`, `erase` |
| `Rf24Controller` | RF24 | `scan`, `send`, `receive`, `sweep`, `jam`, `waterfall` |
| `FmController` | FM | `sweep`, `trace`, `waterfall`, `broadcast` |
| `CellController` | CELL | `modem`, `network`, `operator`, `sim`, `sms`, `call`, `ussd` |
| `ExpanderController` | EXPANDER | UART 桥接到扩展模块 |
| **`MidiController`** | **MIDI** | **详见 MIDI 章节** |

### 4.6 Services (核心服务)

| 服务 | 说明 |
|------|------|
| `WifiService` | WiFi 连接管理 |
| `BluetoothService` | 蓝牙 BLE/经典模式 |
| `I2cService` | I2C 总线通信 |
| `SpiService` | SPI 总线通信 |
| `UartService` | UART 串口通信 |
| `OneWireService` | 1-Wire 协议 |
| `TwoWireService` | 2-Wire 协议 (ISO 7816) |
| `ThreeWireService` | 3-Wire 协议 |
| `LedService` | LED 灯带控制 |
| `InfraredService` | 红外发送/接收 |
| `SubGhzService` | CC1101 Sub-GHz 无线 |
| `RfidService` | PN532 NFC/RFID |
| `Rf24Service` | nRF24L01 无线 |
| `CanService` | MCP2515 CAN 总线 |
| `EthernetService` | W5500 以太网 |
| `FmService` | Si4713 FM 广播 |
| `CellService` | SIM800/SIM7600 蜂窝模块 |
| `JtagService` | JTAG/SWD 调试 |
| `I2sService` | I2S 音频 |
| `HttpService` | HTTP 客户端 |
| `ICMPService` | ICMP Ping |
| `NmapService` | 端口扫描 |
| `NetcatService` | TCP 收发 |
| `SshService` | SSH 客户端 |
| `TelnetService` | Telnet 客户端 |
| `ModbusService` | Modbus TCP/RTU |
| `SdService` | SD 卡读写 |
| `PinService` | GPIO 引脚控制 |
| `SystemService` | 系统信息 |
| `NvsService` | NVS 非易失存储 |
| `LittleFsService` | LittleFS 文件系统 |
| `HdUartService` | 半双工 UART |
| `MidiService` | UART MIDI (5-Pin DIN, 31250 baud) |
| `MidiApiService` | WiFi MIDI API (端口 72/73) |
| `USBMidiService` | USB MIDI 设备类 (已弃用) |
| `DnsServer` | DNS 服务器 (Captive Portal) |
| `HttpServer` | HTTP 服务器 (Web UI + API) |
| `WebSocketServer` | WebSocket 服务器 |

### 4.7 Configurators (启动配置流)

```
Boot → 选择启动模式 (正常/USB 适配器)
         ↓
  选择终端类型 (串口 / WiFi Web / Cardputer 独立)
         ↓
  (若 WiFi) 选择 WiFi 模式 (客户端 / AP)
         ↓
  进入 ActionDispatcher 主循环
```

### 4.8 命令处理流程

```
用户输入命令
    ↓
TerminalInput (读字符)
    ↓
ActionDispatcher.dispatch()
    ├── Alias 展开
    ├── Bytecode 指令? → dispatchInstructions()
    ├── Macro 命令? (未实现)
    ├── Repeat 命令? → dispatchRepeatCommands()
    ├── Pipeline 命令? → dispatchPipelineCommands()
    └── 单命令 → dispatchCommand()
                    ├── "mode"/"m"? → 切换模式
                    ├── 全局命令? → UtilityController
                    └── 模式命令 → 当前模式的 Controller.handleCommand()
                                    ↓
                              Services 层 (硬件操作)
                                    ↓
                              TerminalView (显示输出)
```

### 4.9 终端类型

| 类型 | 说明 |
|------|------|
| `SerialPort` | 通过 USB 串口 (COM/tty) |
| `WiFiClient` | 通过 WiFi 连接到 Web UI |
| `WiFiAp` | ESP32 作为 AP，浏览器连接 |
| `Standalone` | Cardputer 自带键盘+屏幕 |

---

## 5. CLI 命令大全

### 5.1 全局命令 (所有模式可用)

```
help                 - 显示帮助
mode [name]          - 切换模式 (m)
man                  - 显示固件指南
system               - 显示系统信息
profile              - 保存/加载 GPIO 配置
alias                - 创建命令别名
hex [number]         - 十进制/十六进制/二进制转换
logic <gpio>         - 逻辑分析仪
analogic <gpio>      - 模拟信号绘图
wizard <gpio>        - GPIO 活动分析
listen <gpio>        - GPIO 活动转音频
repeat <n> <cmd>     - 重复执行命令
P                    - 启用上拉电阻
p                    - 禁用上拉电阻
```

### 5.2 模式特定命令

#### HIZ (高阻态)
```
help mode - 显示可用模式
```

#### 1WIRE
```
scan                 - 扫描 1-Wire 设备
ping                 - Ping 设备
sniff                - 监听 1-Wire 流量
read                 - 读取 ID + SP
write id [8字节]     - 写入设备 ID
write sp [8字节]     - 写入暂存器
temp                 - 读取温度
ibutton              - iButton 操作
eeprom               - EEPROM 操作
config               - 配置设置
```

#### UART
```
scan                 - 扫描 UART 引脚活动
autobaud             - 自动检测波特率
ping                 - 发送并期待回复
sniff                - 监听 ASCII UART 流量
sniff raw            - 监听 十六进制 UART 流量
read                 - 接收 ASCII 数据
raw                  - 接收原始十六进制数据
write [text]         - 发送数据
bridge               - 全双工桥接模式
at                   - AT 命令操作
emulator             - 模拟 UART 设备
trigger [pattern]    - 匹配模式时发送响应
spam [text] [ms]     - 定期发送数据
xmodem               - XMODEM 文件传输
config               - 配置设置
swap                 - 交换 RX/TX 引脚
```

#### HDUART (半双工 UART)
```
bridge               - 半双工 I/O
config               - 配置设置
```

#### I2C
```
scan                 - 扫描设备
discovery            - 设备报告
ping <addr>          - 检查 ACK
identify <addr>      - 识别设备类型
sniff                - 监听流量
slave <addr>         - 模拟 I2C 从设备
read <addr> [reg]    - 读寄存器
write <a> [r] [val]  - 写寄存器
dump <addr> [len]    - 读取所有寄存器
regs <addr> [len]    - 探测寄存器 R/W
glitch <addr>        - 攻击序列
flood <addr>         - 饱和目标 I/O
health <addr>        - 时序测试
monitor <addr> [ms]  - 监控寄存器变化
trace <a> [reg] [ms] - 监控单个寄存器
eeprom [addr]        - I2C EEPROM 操作
recover              - 总线恢复
jam                  - 噪声干扰
swap                 - 交换 SDA/SCL
config               - 配置设置
```

#### SPI
```
sniff                - 监听流量
sdcard               - SD 卡操作
slave                - 模拟 SPI 从设备
flash                - SPI Flash 操作
eeprom               - SPI EEPROM 操作
config               - 配置设置
```

#### 2WIRE
```
sniff                - 监听 2WIRE 流量
smartcard            - 智能卡操作
config               - 配置设置
```

#### 3WIRE
```
eeprom               - 3WIRE EEPROM 操作
config               - 配置设置
```

#### DIO (数字 I/O)
```
scan                 - 检测引脚活动
pins                 - 显示引脚状态
sniff <gpio>         - 跟踪切换状态
read <gpio>          - 获取引脚电平
set <gpio> <H/L/I/O> - 设置引脚
pullup <gpio>        - 设置上拉
pulldown <gpio>      - 设置下拉
pulse <gpio> <us>    - 发送脉冲
servo <gpio> <angle> - 设置舵机角度
pwm <gpio> [f d%]    - 设置 PWM
toggle <gpio> <ms>   - 定期切换
measure <gpio> [ms]  - 计算频率
jam <gpio> [min max] - 随机高低电平
reset <gpio>         - 重置为默认
```

#### LED
```
fill <color>         - 填充所有 LED
set <index> <color>  - 设置指定 LED
blink                - 闪烁
rainbow              - 彩虹动画
chase                - 追逐效果
cycle                - 颜色循环
wave                 - 波浪动画
reset                - 关闭所有 LED
setprotocol          - 选择 LED 协议
config               - 配置设置
```

#### INFRARED
```
send <dev> sub <cmd> - 发送红外信号
receive              - 接收红外信号
setprotocol          - 设置协议类型
devicebgone          - 万能关机
remote               - 万能遥控
replay [count]       - 重放录制的帧
record               - 录制到文件
load                 - 从文件系统加载 .ir 文件
jam                  - 随机红外干扰
config               - 配置设置
```

#### USB
```
storage              - 挂载 SD 卡为 USB 存储
keyboard [text]      - 键盘桥接
mouse [action]       - 鼠标控制
mouse jiggle [ms]    - 随机鼠标移动
gamepad [key]        - 手柄按键
sysctrl [action]     - 硬件控制
host                 - 连接 USB 设备
adapters             - USB 适配器
reset                - 重置接口
config               - 配置设置
```

#### BLUETOOTH
```
scan                 - 发现设备
pair <mac>           - 配对设备
sniff                - 监听蓝牙数据
spoof <mac>          - MAC 地址欺骗
status               - 显示状态
server               - 创建 HID 服务器
keyboard [text]      - 键盘桥接
mouse [action]       - 鼠标控制
mouse jiggle [ms]    - 随机鼠标移动
reset                - 重置接口
```

#### WIFI
```
scan                 - 扫描网络
connect              - 连接网络
ping <host>          - Ping 主机
discovery [timeout]  - 发现网络设备
sniff                - 监听 WiFi 数据包
waterfall            - 信道活动显示
probe                - 探测网络接入
repeater             - 转发 WiFi 流量
spoof ap <mac>       - 伪造 AP MAC
spoof sta <mac>      - 伪造站 MAC
status               - 显示状态
deauth [ssid]        - 取消认证攻击
disconnect           - 断开连接
ap <ssid> <pw>       - 设置 AP
spam                 - 随机信标洪水
flood [channel]      - 信道洪水攻击
ssh [h] [u] [pw] [p] - SSH 会话
telnet <host> [port] - Telnet 会话
nc <host> <port>     - Netcat 会话
nmap <h> [-p ports]  - 端口扫描
modbus <host> [port] - Modbus TCP
http get <url>       - HTTP GET 请求
http analyze <url>   - HTTP 分析报告
lookup mac|ip <addr> - MAC/IP 查询
webui                - 显示 Web UI IP
reset                - 重置接口
```

#### JTAG
```
scan swd             - 扫描 SWD 引脚
scan jtag            - 扫描 JTAG 引脚
openocd              - 重启为 OpenOCD 适配器
config               - 配置设置
```

#### I2S
```
play <freq> [ms]     - 播放正弦波
record               - 持续读取麦克风
test <speaker|mic>   - 音频测试
reset                - 重置
config               - 配置设置
```

#### CAN
```
sniff                - 打印所有接收帧
send [id]            - 发送指定 ID 帧
receive [id]         - 捕获指定 ID 帧
status               - CAN 控制器状态
config               - 配置 MCP2515
```

#### ETHERNET
```
connect              - DHCP 连接
status               - 显示状态
ping <host>          - Ping 主机
discovery [timeout]  - 发现设备
ssh [h] [u] [pw] [p] - SSH 会话
telnet <host> [port] - Telnet
nc <host> <port>     - Netcat
nmap <h> [-p ports]  - 端口扫描
modbus <host> [port] - Modbus TCP
http get <url>       - HTTP GET
http analyze <url>   - HTTP 分析
lookup mac|ip <addr> - 查找
reset                - 重置
config               - 配置 W5500
```

#### SUBGHZ
```
scan                 - 搜索最佳频率
sweep                - 分析频段
send <payload>       - 发送帧
receive              - 接收信号
replay               - 录制并重放
jam                  - 频率干扰
bruteforce           - 暴力破解 12 位密钥
trace                - RX 信号跟踪
waterfall            - 频率峰值显示
record               - 录制到 .sub 文件
load                 - 加载 .sub 文件
ear                  - RSSI 转音频
setfrequency         - 设置工作频率
config               - 配置 CC1101
```

#### RFID
```
read                 - 读取标签数据
write                - 写入 UID/块
clone                - 克隆 Mifare UID
erase                - 擦除标签
config               - 配置 PN532
```

#### RF24 (nRF24L01)
```
scan                 - 搜索活动信道
send                 - 发送帧
receive              - 接收帧
sweep                - 分析信道活动
jam                  - 干扰信道
waterfall            - 信道峰值
setchannel           - 设置工作信道
config               - 配置 NRF24
```

#### FM (Si4713)
```
sweep                - 分析频率
trace [freq]         - 频率信号跟踪
waterfall            - 频率峰值
broadcast            - 创建 FM 电台
reset                - 重置
config               - 配置 Si4713
```

#### CELL (蜂窝模块)
```
modem                - 显示调制解调器信息
network              - 网络信息
operator             - 可用运营商
sim                  - SIM 卡信息
unlock               - PIN 解锁
phonebook            - 电话本
sms                  - 短信操作
call                 - 通话操作
ussd [code]          - USSD 代码
setmode              - 设置模式
config               - 配置
```

#### EXPANDER
```
此模式通过 UART 桥接到 ESP32 Bus Expander 扩展模块。
```

#### **MIDI**
```
config               - 配置 MIDI UART GPIO 引脚
send <hex>           - 发送原始 MIDI 字节
receive              - 接收并解码 MIDI 消息
sniff                - 原始十六进制 MIDI 流量转储
note <ch> <n> [vel]  - 发送 Note (ch=1-16, note=0-127, vel=0-127)
cc <ch> <c> [val]    - 发送 Control Change (ch=1-16, ctrl=0-127, val=0-127)
pgm <ch> <p>         - 发送 Program Change (ch=1-16, prog=0-127)
pitch <ch> [val]     - 发送 Pitch Bend (ch=1-16, val=0-16383, center=8192)
clock                - 持续发送 Clock 实时消息
start                - 发送 Start 实时消息
stop                 - 发送 Stop 实时消息
continue             - 发送 Continue 实时消息
thru [on/off]        - 切换 MIDI Thru (RX 回显到 TX)
usb                  - 显示 USB MIDI (API) 状态
usb start            - 启动 MIDI API 服务 (端口 72 HTTP + 73 WS)
usb stop             - 停止 MIDI API 服务
api                  - 显示 API 管理菜单
api start            - 启动 API 服务器
api stop             - 停止 API 服务器
api config           - 配置黑白名单模式
api whitelist <cmd>  - 白名单管理 (add|del <ip>)
api blacklist <cmd>  - 黑名单管理 (add|del <ip>)
api autostart <on|of> - 切换 WiFi 连接后自动启动
reset                - 重置接口
```

---

## 6. MIDI 功能详解

### 6.1 架构

```
┌─────────────────────────────────────────────────┐
│                  MidiController                  │
│  (命令解析 + 参数校验 + 路由)                    │
└────────┬────────────┬────────────────┬──────────┘
         │            │                │
         ▼            ▼                ▼
   MidiService   MidiApiService    (USB MIDI)
   (UART MIDI)   (WiFi API)       (已弃用)
   5-Pin DIN     端口 72 HTTP
   31250 baud    端口 73 WS
   TX=17 RX=18   单 IP 限制
                 黑白名单
```

### 6.2 MidiService (UART 5-Pin DIN)

- 使用 Arduino `HardwareSerial` (Serial2)
- TX 引脚: GPIO17, RX 引脚: GPIO18
- 波特率: 31250 8N1 (MIDI 标准)
- **支持的消息类型**:
  - Note Off (0x80) / Note On (0x90)
  - Polyphonic Key Pressure (0xA0)
  - Control Change (0xB0)
  - Program Change (0xC0)
  - Channel Pressure (0xD0)
  - Pitch Bend (0xE0)
  - SysEx (0xF0-0xF7)
  - 实时消息 (0xF8-0xFF): Clock, Start, Stop, Continue, Active Sensing, Reset
- **Running Status**: 支持连续相同状态字节省略

### 6.3 MidiApiService (WiFi API)

| 端口 | 协议 | 说明 |
|------|------|------|
| 72 | HTTP REST | 设备信息 + MIDI 发送 |
| 73 | WebSocket | 双向 MIDI 流 |

**REST API** (端口 72):
- `GET /info` — 设备信息 (名称, 版本, MIDI 状态, API 状态)
- `POST /send` — 发送 MIDI 消息 (JSON 格式)

**WebSocket** (端口 73):
- `/midi` — 双向 MIDI 消息流

**JSON 消息格式**:
```json
// Note 消息
{"note":{"channel":1,"note":60,"velocity":100}}
// Control Change
{"cc":{"channel":1,"controller":7,"value":100}}
// Program Change
{"program":{"channel":1,"program":0}}
// Pitch Bend
{"pitch":{"channel":1,"value":8192}}
// 原始 MIDI 字节
{"raw":[144,60,127]}
// SysEx
{"sysex":[126,127,9,1]}
// 实时时钟
{"clock":true}
// 传输控制
{"transport":"start|stop|continue"}
```

**安全特性**:
- 单 IP 并发限制 (同一时间最多 1 个客户端)
- 白名单/黑名单访问控制
- 可通过终端或 Web 配置

### 6.4 硬件引脚

| 功能 | 引脚 |
|------|------|
| MIDI UART TX | GPIO17 |
| MIDI UART RX | GPIO18 |
| 板载 LED | GPIO48 (RGB) |

---

## 7. 构建与部署

### 7.1 构建环境

- 框架: PlatformIO + Arduino
- 平台: `https://github.com/pioarduino/platform-espressif32`
- 环境: `s3-devkit-n16-r8`

### 7.2 分区表

| 环境 | 分区文件 | App | SPIFFS |
|------|----------|-----|--------|
| 通用 | `app4M_spiffs_4M_8MB.csv` | 4MB | 4MB |
| N16R8 | `app4M_spiffs_12M_16MB.csv` | 4MB | 12MB |

### 7.3 GitHub Actions

- 文件: `.github/workflows/build.yml`
- 触发: 推送到 `pioarduino` 分支
- 步骤: Checkout → Python → PlatformIO → 编译 → 重命名 → 上传
- 产物命名:
  - `ESP32-S3-N16R8_BitPirate_firmware_0x0.bin` — **单文件烧录到 0x0**
  - `ESP32-S3-N16R8_bootloader.bin`
  - `ESP32-S3-N16R8_firmware_app.bin`
  - `ESP32-S3-N16R8_partitions.bin`
  - `firmware.elf`

### 7.4 烧录命令

```bash
# 方法 1: 单文件烧录 (推荐)
esptool.py --chip esp32s3 write_flash 0x0 ESP32-S3-N16R8_BitPirate_firmware_0x0.bin

# 方法 2: PlatformIO
pio run -e s3-devkit-n16-r8 --target upload
```

---

## 8. 依赖库

```ini
fastled/FastLED@^3.3.3              # LED 灯带
bblanchon/ArduinoJson@^7.3.0        # JSON 解析
paulstoffregen/OneWire@^2.3.8       # 1-Wire
hideakitai/ESP32SPISlave@^0.6.8     # SPI 从模式
gilman88/XModem@^1.0.3              # XMODEM
ewpa/LibSSH-ESP32@^5.6.0            # SSH 客户端
autowp/autowp-mcp2515@^1.2.1        # CAN 总线
sparkfun/SparkFun External EEPROM   # 外部 EEPROM
miq19/eModbus@^1.7.4                # Modbus
pstolarz/OneWireNg@^0.14.0          # 1-Wire (新)
adafruit/Adafruit Si4713 Library    # FM 广播
crankyoldgit/IRremoteESP8266@^2.9.0  # 红外
```

### 本地库 (`lib/`)
- `93cx6/` — 93Cx6 EEPROM 驱动
- `EEPROM_SPI_WE/` — SPI EEPROM 库
- `IRremote/` — 红外遥控
- `PN532/` — NFC/RFID
- `RF24/` — nRF24L01 无线
- `SmartRC-CC1101-Driver-Lib/` — CC1101 Sub-GHz

---

## 9. Critical Build Flags

```ini
-D ARDUINO_USB_MODE=0              # 使用 USB Serial (非 USB-OTG)
-D ARDUINO_USB_CDC_ON_BOOT=1       # 启动时启用 CDC 串口
-D CONFIG_TINYUSB_CDC_ENABLED=1    # TinyUSB CDC
-D CONFIG_TINYUSB_HID_ENABLED=1    # TinyUSB HID
-D CONFIG_TINYUSB_MSC_ENABLED=1    # TinyUSB MSC
-D CONFIG_TINYUSB_MIDI_ENABLED=1   # TinyUSB MIDI
-D CONFIG_CRC16_ENABLED=1          # CRC16 (OneWireNg)
-D INFRARED_IREMOTE_ESP8266        # 使用 IRremoteESP8266
-Wl,-zmuldefs                      # 允许多重定义 (WiFi deauth)
```

---

## 10. GitHub Actions 构建产物历史

| Run # | 日期 | 状态 | 产物位置 |
|-------|------|------|----------|
| 1-4 | 2026-06-26 | ❌ | - |
| 5 | 2026-06-26 | ✅ 首次成功 | `github/actions/5/产物/` |
| 6 | 2026-06-27 | ✅ 重命名后成功 | `github/actions/6/产物/` |