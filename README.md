# ESP32-Keypad

Update: I've started a [Hackaday.io project](https://hackaday.io/project/177226-esp32-bluetooth-gamepad)!

This is a modification to the [Sherbet gaming keypad](https://www.billiam.org/2019/05/29/sherbet-an-ergonomic-keypad) designed by [Billiam](https://www.billiam.org/). The intent is to turn it into a battery-powered bluetooth peripheral. I'm using the ESP32 Arduino libraries, but I'm working in VSCode with [esp-idf](https://github.com/espressif/esp-idf) and [PlatformIO](https://platformio.org/install/ide?install=vscode).

The main reason for not working in the Arduino IDE is the library screwy-ness that is required here. The Sherbet gamepad has an analog stick in addition to regular keyboard inputs, which requires the use of Keyboard HID and Gamepad HID. Both of these options are widely available and pretty easy to implement, but I've been unsuccessful in finding a solution that implements both. Due to how the ESP32-Arduino libraries for both of these options work, the libraries need to be combined into one freakish combination. 

The library I'm using is a combination of the following two libraries:

[ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) written by [T-vK](https://github.com/T-vK)

[ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad) written by [lemmingDev](https://github.com/lemmingDev)

I've created a breakout board with an ESP32-WROOM-32E chip that has support for a Li-Ion battery. The Eagle design files, datasheets for critical parts, and a DigiKey BOM have been included for anyone wanting to make one of these for themselves!

Notes:

There have been some issues with my board files. As a disclaimer, older versions of the PCB have pin 32 connected directly to Battery +. I wanted to use the ADC on pin 32 to measure battery voltage, and from there remaining battery capacity. I forgot however that a fully charged Li-Ion battery is 4.2V, which exceeds the ESP32's max input voltage by 0.5V. Not enough to fry the chip, but more than enough to damage the pin. The end effect is that this blasts the UART RX channel in the chip with 4V, rendering the ESP unable to receive new code over the UART interface. So if you're using an old board version, make sure to cut the trace going to I/O pin 32. You've been warned. 

I've since changed this to be fed through a voltage divider to measure the battery. It's constantly draining the battery, but the divider only draws about 9 micro amps, so it shouldn't impact lifespan.
