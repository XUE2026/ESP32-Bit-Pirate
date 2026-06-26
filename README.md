# ESP32 Bit Pirate

![Logo banner of the ESP32 Bit Pirate firmware](images/logo_protocols_banner_small.png)


**ESP32 Bit Pirate** is an open-source firmware that turns your device into a multi-protocol hacker's tool, inspired by the [legendary Bus Pirate](https://buspirate.com/).

It supports sniffing, sending, scripting, and interacting with various digital protocols (I2C, UART, 1-Wire, SPI, etc.) via a serial terminal or web-based CLI. It also communicates with radio protocols like Bluetooth, Wi-Fi, Sub-GHz and RFID.

Use the [ESP32 Bit Pirate Web Flasher](https://geo-tp.github.io/ESP32-Bit-Pirate/webflasher/) to install the firmware in one click. See the [Wiki](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki) for step-by-step guides on every mode and command. Check [ESP32 Bit Pirate Scripts](https://github.com/geo-tp/ESP32-Bit-Pirate-Scripts) for a collection of scripts.

For hardware extensions, see the [ESP32 Bus Expander](https://github.com/geo-tp/ESP32-Bus-Expander) for additional radio interfaces, and the [ESP32 Bit Pirate Dock](https://github.com/AndreiVladescu/ESP32-Bit-Pirate-Dock) to use original [Bus Pirate](https://buspirate.com/) adapters and accessories.

![Demo showing the different mode of the ESP32 Bit Pirate firmware](images/bit_pirate_uart.gif)
![Demo showing the LittleFS file system of the ESP32 Bit Pirate firmware](images/pirate_assistant.gif)

## Features

- Interactive command-line interface (CLI) via **USB Serial or WiFi Web**.
- **Modes for:**
   - [HiZ](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/01-HiZ) (default)
   - [I2C](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/05-I2C) (scan, glitch, slave mode, dump, eeprom)
   - [SPI](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/06-SPI) (eeprom, flash, sdcard, slave mode)
   - [UART](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/03-UART) / [Half-Duplex UART](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/04-HDUART) (bridge, read, write)
   - [1WIRE](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/02-1WIRE) (ibutton, eeprom)
   - [2WIRE](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/07-2WIRE) (sniff, smartcard) / [3WIRE](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/08-3WIRE) (eeprom)
   - [DIO](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/09-DIO) (Digital I/O, read, pullup, set, pwm)
   - [Infrared](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/11-INFRARED) (send, record, universal remote)
   - [USB](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/12-USB) (HID, flashrom, storage, usb-uart)
   - [Bluetooth](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/13-BLUETOOTH) (BLE HID, scan, spoofing, sniffing)
   - [Wi-Fi](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/14-WIFI) / [Ethernet](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/18-ETHERNET) (sniff, deauth, nmap, netcat)
   - [JTAG](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/15-JTAG) (scan, SWD, openOCD)
   - [LED](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/10-LED) (animations, set LEDs)
   - [I2S](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/16-I2S) (test speakers, mic, play sound)
   - [CAN](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/17-CAN) (sniff, send and receive frames)
   - [SUBGHZ](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/19-SUBGHZ) (analyze, record, replay)
   - [RFID](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/20-RFID) (read, write, clone)
   - [RF24](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/21-RF24) (scan, send, receive)
   - [FM](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/22-FM) (analyze, broadcast)
   - [CELL](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/23-CELL) (dump sim card, sms, call)
   - **[MIDI](wiki/24-MIDI)** (UART MIDI 5-pin DIN, USB MIDI device, WiFi MIDI API on ports 72/73)


- **Protocol sniffers** I2C, UART, SPI, 1Wire, 2wire, CAN, Wi-Fi, Bluetooth, SubGhz.
- Baudrate **auto-detection**, AT commands and various tools for UART.
- Registers manipulation, **EEPROM dump tools**, identify devices for I2C.
- Read all sort of **EEPROM, Flash** and various others tools for SPI.
- Scripting using **Bus Pirate-style bytecode** instructions or **Python**.
- Device-B-Gone command with more than **80 supported INFRARED protocols**.
- Direct I/O management, **PWM, servo, GPIOs state**.
- Analyze radio signals and frequencies **on every bands**.
- Near than **50 addressable LEDs protocols** supported.
- **Ethernet and WiFi** are supported to access networks.
- Import and export data with the **LittleFS over HTTP.**
- **Pirate assistant** to help you with the firmware.
- **USB-Uart dongle, SPI programmer, logic analyzer** and more.
- [**Web Serial tools**](https://geo-tp.github.io/ESP32-Bit-Pirate/web-tools/) to use USB Serial over a web browser.

## Supported Devices


| Device               |                                     | Description                       |
|-----------------------|------------------------------------------|---------------------------------------------------|
| **ESP32 S3 Dev Kit**  | ![Photo of the ESP32 S3 Dev Kit](/images/s3-devkit_s.jpg)     | More than 20 available GPIO, 1 button |
| **LILYGO T-Display** | ![Photo of the T-Display-S3](/images/t_displays3_s.jpg) | 13 GPIO (1 Qwicc), screen, 2 buttons |
| **LILYGO T-Embed**    | ![Photo of the LILYGO T-Embed](/images/tembed_s.jpg)          | 9 GPIO (Grove, Header), screen, encoder, speaker, mic, SD card                                         |
| **LILYGO T-Embed CC1101** | ![Photo of the LILYGO T-Embed CC1101](/images/tembedcc1101_s.jpg) | 4 GPIO (2x Qwiic), screen, encoder, speaker, mic, SD Card, CC1101, PN532, IR TX, IR RX , battery                                 |
| **LILYGO T-Embed CC1101 Plus** | ![Photo of the LILYGO T-Embed CC1101 Plus](/images/tembedcc1101_s.jpg) | 4 GPIO (2x Qwiic), screen, encoder, speaker, mic, SD Card, CC1101, NRF24, PN532, IR TX, IR RX , battery                                 |
| **M5 AtomS3 Lite**    | ![Photo of the M5 Atom S3 Lite](/images/atom_s.jpg)            | 8 GPIO (Grove, Header), IR TX, 1 buttton                  |
| **M5 Cardputer**      | ![Photo of the M5 Cardputer](/images/cardputer_s.png)            | 2 GPIO (Grove), screen, keyboard, mic, speaker, IR TX, SD card, battery, [standalone mode](#standalone-mode-for-the-cardputer)            |
| **M5 Cardputer ADV**  | ![Photo of the M5 Cardputer ADV](/images/cardputer-adv_s.jpg)    | 12 GPIO (Grove, Header), screen, keyboard, mic, speaker, IR TX, SD card, IMU, battery, [standalone mode](#standalone-mode-for-the-cardputer)                  |
| **M5 StampS3**        | ![Photo of the M5 StampS3](/images/stamps3_s.jpg)             | 9 GPIO (exposed pins), 1 button                       |
| **M5 Stick S3** | ![Photo of the M5 Stick S3](/images/m5sticks3_s.jpg)      | 13 GPIO (Grove, Header), screen, mic, speaker, IR TX, IR RX, IMU, 3 buttons, battery                 |
| **Seeed Studio Xiao S3** | ![Photo of the Seeed Studio Xiao ESP32-S3](/images/xiaos3_s.jpg)        | 9 GPIO (exposed pins), 1 button |

- **Other ESP32-S3-based Boards**

  - All boards based on the **ESP32-S3 can be supported**, provided they have at least **8 MB of flash.**

  - You can **flash the s3 dev-kit firmware onto any ESP32-S3 board.**

  - Keep in mind that the **default pin mapping in the firmware may not match** your specific board.

## Getting Started

[![Banner of the ESP32 Bit Pirate web flasher](images/flasher.jpg)](https://geo-tp.github.io/ESP32-Bit-Pirate/webflasher/)

1. 🔧 Flash the firmware
   - Use the [ESP32 Bit Pirate Web Flasher](https://geo-tp.github.io/ESP32-Bit-Pirate/webflasher/) to burn the firmware directly from a web browser.
   - You can also burn it on [M5Burner](https://docs.m5stack.com/en/download), in the StickS3, AtomS3, M5StampS3 or Cardputer category.

2. 🔌 Connect via Serial or Web
   - Serial: any terminal app, or the [free browser-based Web Serial terminal](https://geo-tp.github.io/ESP32-Bit-Pirate/web-tools/web-serial-terminal/) (see [Connect via Serial](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/99-Serial))
   - Web: configure Wi-Fi and access the CLI via browser (see [Wi-Fi Connection](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/00-Terminal))

3. 🧪 Use commands like:
   ```
   mode
   help
   scan
   sniff
   ...
    ```

## MIDI (Musical Instrument Digital Interface)

**⚠️ 硬件说明**: 本MIDI实现仅针对 **ESP32-S3 N16R8**（16MB Flash / 8MB PSRAM）进行测试和优化。其他硬件平台未经测试，不保证功能完整。

MIDI模式将ESP32 Bit Pirate转变为功能完整的MIDI接口工具，支持三种MIDI通路：

### 1. UART MIDI（5-Pin DIN，标准MIDI硬件接口）

基于ESP32 UART2，31250 baud 8N1标准MIDI速率。

**命令：**
| 命令 | 说明 |
|------|------|
| `config` | 配置MIDI TX/RX GPIO引脚 |
| `send <hex>` | 发送原始MIDI字节 |
| `receive` | 持续监听并解码MIDI消息，按[ENTER]停止 |
| `sniff` | MIDI流量原始十六进制转储 |
| `note <ch> <n> [vel]` | 发送Note On/Off（ch=1-16, note=0-127, vel=0-127，vel=0→Note Off） |
| `cc <ch> <c> [val]` | 发送Control Change（ctrl=0-127, val=0-127） |
| `pgm <ch> <p>` | 发送Program Change（prog=0-127，如0=Grand Piano） |
| `pitch <ch> [val]` | 发送Pitch Bend（val=0-16383，中心=8192） |
| `clock` | 持续发送实时时钟消息，按[ENTER]停止 |
| `start` | 发送Start实时消息 |
| `stop` | 发送Stop实时消息 |
| `continue` | 发送Continue实时消息 |
| `thru [on/off]` | 开/关MIDI Thru（RX输入透明转发到TX输出） |
| `reset` | 重置MIDI接口 |

### 2. USB MIDI 设备类（Type-C口）

通过ESP32-S3原生USB Type-C口暴露为USB MIDI设备。连接电脑后，DAW（如FL Studio、Ableton Live、MuseScore等）可直接识别为MIDI输入/输出设备。

- 基于TinyUSB实现，需构建时启用`CONFIG_TINYUSB_MIDI_ENABLED`
- 不占用内置USB Serial/JTAG（`Serial`仍可用）
- USB MIDI设备与WiFi API服务共用MidiService数据通路

**命令：**
| 命令 | 说明 |
|------|------|
| `usb` | 查看USB MIDI状态 |
| `usb start` | 启动MIDI API服务（同时启用USB MIDI功能） |
| `usb stop` | 停止MIDI API服务 |

### 3. WiFi MIDI API（端口72 HTTP + 端口73 WebSocket）

当WiFi连接成功后，可自动在端口72和73启动MIDI API服务。

**端口72 — REST API (HTTP/JSON)：**

| 端点 | 方法 | 说明 |
|------|------|------|
| `/info` | GET | 设备信息、MIDI状态、API配置 |
| `/status` | GET | 当前MIDI运行状态 |
| `/send` | POST | 发送MIDI消息（JSON body） |

**`POST /send` 支持的消息格式：**

```json
{"note":{"channel":1,"note":60,"velocity":100}}
{"cc":{"channel":1,"controller":7,"value":100}}
{"program":{"channel":1,"program":0}}
{"pitch":{"channel":1,"value":8192}}
{"raw":[144,60,127]}
{"sysex":[126,127,9,1]}
{"clock":true}
{"transport":"start"}
```

**端口73 — WebSocket (`/midi`)：**

- 双向MIDI流传输
- 支持所有MIDI消息类型的JSON格式收发
- 接收到的MIDI消息实时推送到WebSocket客户端
- 客户端可通过WebSocket发送MIDI命令（格式同REST API）
- 适合MuseScore等软件通过局域网进行MIDI输入/输出

**API管理命令：**
| 命令 | 说明 |
|------|------|
| `api` | 查看API管理菜单 |
| `api start` | 启动API服务 |
| `api stop` | 停止API服务 |
| `api config` | 配置白名单/黑名单模式 |
| `api whitelist add <ip>` | 添加IP到白名单 |
| `api whitelist del <ip>` | 从白名单移除IP |
| `api blacklist add <ip>` | 添加IP到黑名单 |
| `api blacklist del <ip>` | 从黑名单移除IP |
| `api autostart on` | 启用WiFi连接自动启动 |
| `api autostart off` | 禁用WiFi连接自动启动 |

**安全特性：**
- **单IP并发限制**：同一时间最多1个客户端使用API，防止资源耗尽
- **白名单模式**：仅允许指定IP访问
- **黑名单模式**：阻止指定IP访问
- **默认开机自启动**：WiFi连接成功后自动启动API（可通过`api autostart off`关闭）

### 快速开始

```
1. 进入MIDI模式: mode midi
2. 配置引脚: config  (设置TX/RX GPIO)
3. 连接5-pin DIN MIDI设备
4. 发送测试: note 1 60 100  (通道1, 中央C, 力度100)
5. 监听消息: receive       (按ENTER停止)
```

WiFi API使用：
```
1. 配置WiFi并连接成功
2. 启动API: api start
3. 电脑访问: curl http://esp32-ip:72/info
4. WebSocket: ws://esp32-ip:73/midi
```

## Wiki

[![Banner of the ESP32 Bit Pirate Wiki page](images/bus_pirate_wiki.png)](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/)

📚 **[Visit the Wiki](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki)** for detailed documentation on every mode and command.

Includes:
- [Terminal mode](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/00-Terminal) - About serial and web terminal.
- [Mode overviews](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki) - Browse supported modes.
- [Serial setup](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/99-Serial) - Serial access via USB.

The wiki is the best place to learn how everything works.

## Scripting

[![Banner of the ESP32 Bit Pirate Scripts page](images/bus_pirate_scripts.png)](https://github.com/geo-tp/ESP32-Bit-Pirate-Scripts/)

🛠️ [**Automate interactions with the ESP32 Bit Pirate**](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/99-Python) using **Python scripts over serial.**

**Examples and ready-to-use scripts** are available in the repository: [ESP32 Bit Pirate Scripts](https://github.com/geo-tp/ESP32-Bit-Pirate-Scripts).

**Including:** Logging data in a file, eeprom and flash dump, interracting with GPIOs, LED animation...

## Expander
[![Banner of the ESP32 Bit Pirate Expander page](images/bus_pirate_exp.png)](https://github.com/geo-tp/ESP32-Bus-Expander)


🔌 **[Expand the capabilities of the ESP32 Bit Pirate](https://github.com/geo-tp/ESP32-Bus-Expander)** with additional hardware modules.
The Expander adds support for the **WiFi 5 GhZ** or other radio protocols.

![A Cardputer connected to an expander C5](images/cardputer_with_c5.jpg)


## Dock
[![Banner of the ESP32 Bit Pirate Dock page](images/bus_pirate_dock.png)](https://github.com/AndreiVladescu/ESP32-Bit-Pirate-Dock)

🔧 **[A docking station for the ESP32 S3 DevKit](https://github.com/AndreiVladescu/ESP32-Bit-Pirate-Dock) designed to work with original Bus Pirate adapters.**
It allows you to plug and use the original [Bus Pirate](https://buspirate.com/) ecosystem of adapters and accessories.

![The ESP32 Bit Pirate dock board](images/bus_pirate_dock_board.png)

(Coming soon)

[![PCBWay Logo](images/pcbway_logo.png)](https://www.pcbway.com)



## Command-Line Interfaces

The ESP32 Bit Pirate firmware provides three command-line interface (CLI) modes:

| Interface         | Advantages                                                                 | Ideal for...                          |
|------------------|-----------------------------------------------------------------------------|----------------------------------------|
| **Web Interface** | - Accessible from any browser<br>- PC, tablets, mobiles<br>- Works over Wi-Fi<br>- No cables needed | Quick tests, demos, headless setups   |
| **Serial Interface** | - Faster performance<br>- Instant responsiveness<br>- Handles large data smoothly | Intensive sessions, frequent interactions |
| **Standalone** | - Only for the Cardputer<br>- On device keyboard<br>- On device screen | Portable sessions, Quick tests |


All interfaces share the same command structure and can be used interchangeably ([more details](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/00-Terminal)).

## Mobile Web Interface over WiFi
![An iPhone screenshot showing the Bit Pirate firmware web interface](images/presentation_mobile.png)

## Standalone Mode for the Cardputer
![A Cardputer running the ESP32 Bit Pirate in standalone mode](images/standalonemode_s.png)

## Browser-Based Web Serial Tools

The [ESP32 Bit Pirate Web Serial Tools](https://geo-tp.github.io/ESP32-Bit-Pirate/web-tools/) provides direct access to the Serial CLI from a compatible browser, without installing PuTTY, minicom, or another terminal application.

![A demo Using the ESP32 Bit Pirate with Web Serial Tools](images/web_tools_demo.gif)

## Contribute
See [How To Contribute](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/99-Contribute) section, which outlines a **simple way to add a new command** to any mode.

## Visuals Assets

#### [![Small logo of the ESP32 Bit Pirate firmware](images/logo_square_small.png)](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/99-Visual-Assets)

See [images, logo, presentations, photo, video, illustrations](https://github.com/geo-tp/ESP32-Bit-Pirate/wiki/99-Visual-Assets). These visuals can be **freely used in blog posts, documentation, videos, or articles** to help explain and promote the firmware.


## Warning
> ⚠️ **Voltage Warning**: Devices should only operate at **3.3V** or **5V**.
> - Do **not** connect peripherals using other voltage levels — doing so may **damage your ESP32**.

> ⚠️ **Usage Warning**: This firmware is provided for **educational, diagnostic, and interoperability testing purposes only**.
> - Do not use it to interfere with, probe, or manipulate devices without proper authorization.
> - Avoid any unauthorized RF transmissions (e.g., sub-GHz) that could violate local regulations or disrupt networks and communications.
> - The authors are not responsible for any misuse of this software or hardware, including legal consequences resulting from unauthorized access or signal emission.
> - Always stay within the bounds of your country’s laws and responsible disclosure policies.
