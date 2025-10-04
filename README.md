| Support Targets | ![alt text][esp32] | ![alt text][Arduino] |
| --- | --- | --- |

| Boards Used | ![alt text][esp32-WROOM-32]|
| --- | --- |

[esp32]: https://img.shields.io/badge/ESP32-green "ESP32"
[Arduino]: https://img.shields.io/badge/Arduino-blue "Arduino"
[esp32-WROOM-32]: https://img.shields.io/badge/ESP32--WROOM--32-orange "ESP32-WROOM-32"

CW keyer with BT keyboard on ESP32-WROOM-32 module   This is very basic feature set used mainly to test integrating a BT keyboard. It sends out text to a LED and buzzer.

Compiled with Arduino IDE 2.3.5

This merges 2 projects.

1. The BT Keyboad Class was derived largely intact from https://github.com/turgu1/bt-keyboard.   

2. "The Arduino Morse Keyer" project from K6HX (code location TBD) 

I am using a Rii model i8+ (aka K08) mini dual mode keyboard with touchpad and buttons.  The bt_keyboard code was looking for appearance == 0x031c and UUID == 0x1812 to initiate connection to the keyboard.  The Rii keyboard showed up as UUID=0x0000.  I bypassed the UUID match and just use appearance value. The code scans BLE thgen BT to build a device list.  It then scans the list and connects to the first device that matches the appearance number.

Accepts 0-9, a-z, and space character and most oither characters in cluding SHIFT_KEY.  Letters are converted to upper case.  The characters are sent out real time as morse code on a PWM I/O pin.  I hooked up a standard piezo buzzer to pin 25.  The speed and frequency are set in the code.  An onboard user LED (blue in my case) on pin 2 also lights to show BT connected status.  There is also a pin (LED_Pin) that can be used to see the dits and dahs.

Hardware is a ESP32-WROOM-32 module from https://www.amazon.com/dp/B0C5MJ6CPL.   Use care, some are older or even single core versions, many listing make it hard to tell.

Chip is ESP32-D0WD-V3 (revision v3.1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz

I am using esp-idf 5.5.1 underneath. 

The bt_keyboard .cpp and .hpp are in a library format and you copy then to your documents/Arduino/libraries folder.

There is a simple workaround included for a BT controller_init error at startup under Arduino IDE.  btStarted() is called in program setup before calling bt_keyboard.setup().  It forces a certain esp bt api .c file to get linked in and it magically makes things work.  So I can now compile and run this under Arduino IDE.  I also was able to integrate the bt_keyboard library into a 2022 era fork of K3NG Keyer, modified to run on ESP32 hardware, now included initial work to use a BT keyboard. That is a separate repo here https://github.com/K7MDL2/k3ng_cw_keyer-master_2022.

These are some direct links to the Wiki pages form my esp-idf version of this.
    
https://github.com/K7MDL2/BT-Keyboard-CW-Keyer/wiki/CPU-Module

For your Arduino IDE configuration make these settings in the Tools menu:

Board: uPesy ESP32 WroomDevKit
port – your com port
patition Scheme: Huge App












