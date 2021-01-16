# ESP32-Keypad

This is a modification to the [Sherbet gaming keypad](https://www.billiam.org/2019/05/29/sherbet-an-ergonomic-keypad) designed by [Billiam]. The intent is to turn it into a battery-powered bluetooth peripheral. I'm using the ESP32 Arduino libraries, but I'm working in VSCode with [esp-idf](https://github.com/espressif/esp-idf) and [PlatformIO](https://platformio.org/install/ide?install=vscode).

The main reason for not working in the Arduino IDE is the library screwy-ness that is required here. The Sherbet gamepad has an analog stick in addition to regular keyboard inputs, which requires the use of Keyboard HID and Gamepad HID. Both of these options are widely available and pretty easy to implement, but I've been unsuccessful in finding a solution that implements both. Due to how the ESP32-Arduino libraries for both of these options work, the libraries need to be combined into one freakish combination. 

The library I'm using is a combination of the following two libraries:

[ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) written by [T-vK](https://github.com/T-vK)

[ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad) written by [lemmingDev](https://github.com/lemmingDev)

I've created a breakout board with an ESP32-WROOM-32E chip that has support for a Li-Ion battery. The Eagle design files, datasheets for critical parts, and a DigiKey BOM have been included for anyone wanting to make one of these for themselves!
