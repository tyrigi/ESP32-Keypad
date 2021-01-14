#ifndef ESP32_BLE_KEYPAD_H
#define ESP32_BLE_KEYPAD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BleConnectionStatus.h"
#include "BLEHIDDevice.h"
#include "BLECharacteristic.h"
#include "Print.h"

#define BUTTON_1    (1<<0)		// 1
#define BUTTON_2    (1<<1)		// 2
#define BUTTON_3    (1<<2)		// 4
#define BUTTON_4    (1<<3)		// 8
#define BUTTON_5    (1<<4)		// 16
#define BUTTON_6    (1<<5)		// 32
#define BUTTON_7    (1<<6)		// 64
#define BUTTON_8    (1<<7)		// 128
#define BUTTON_9    (1<<8)		// 256
#define BUTTON_10 	(1<<9)		// 512
#define BUTTON_11 	(1<<10)		// 1024
#define BUTTON_12 	(1<<11)		// 2048
#define BUTTON_13 	(1<<12)		// 4096
#define BUTTON_14 	(1<<13)		// 8192
#define BUTTON_15 	(1<<14)		// 16384
#define BUTTON_16 	(1<<15)		// 32768

#define BUTTON_17   (1<<16)		// 65536
#define BUTTON_18   (1<<17)		// 131072
#define BUTTON_19   (1<<18)		// 262144
#define BUTTON_20   (1<<19)		// 524288
#define BUTTON_21   (1<<20)		// 1048576
#define BUTTON_22   (1<<21)		// 2097152
#define BUTTON_23   (1<<22)		// 4194304
#define BUTTON_24   (1<<23)		// 8388608
#define BUTTON_25   (1<<24)		// 16777216
#define BUTTON_26   (1<<25)		// 33554432
#define BUTTON_27   (1<<26)		// 67108864
#define BUTTON_28   (1<<27)		// 134217728
#define BUTTON_29   (1<<28)		// 268435456
#define BUTTON_30   (1<<29)		// 536870912
#define BUTTON_31   (1<<30)		// 1073741824
#define BUTTON_32   (1<<31)		// 2147483648

#define DPAD_CENTERED 	0
#define DPAD_UP 		1
#define DPAD_UP_RIGHT 	2
#define DPAD_RIGHT 		3
#define DPAD_DOWN_RIGHT 4
#define DPAD_DOWN 		5
#define DPAD_DOWN_LEFT 	6
#define DPAD_LEFT 		7
#define DPAD_UP_LEFT 	8

const uint8_t KEY_LEFT_CTRL = 0x80;
const uint8_t KEY_LEFT_SHIFT = 0x81;
const uint8_t KEY_LEFT_ALT = 0x82;
const uint8_t KEY_LEFT_GUI = 0x83;
const uint8_t KEY_RIGHT_CTRL = 0x84;
const uint8_t KEY_RIGHT_SHIFT = 0x85;
const uint8_t KEY_RIGHT_ALT = 0x86;
const uint8_t KEY_RIGHT_GUI = 0x87;

const uint8_t KEY_UP_ARROW = 0xDA;
const uint8_t KEY_DOWN_ARROW = 0xD9;
const uint8_t KEY_LEFT_ARROW = 0xD8;
const uint8_t KEY_RIGHT_ARROW = 0xD7;
const uint8_t KEY_BACKSPACE = 0xB2;
const uint8_t KEY_TAB = 0xB3;
const uint8_t KEY_RETURN = 0xB0;
const uint8_t KEY_ESC = 0xB1;
const uint8_t KEY_INSERT = 0xD1;
const uint8_t KEY_DELETE = 0xD4;
const uint8_t KEY_PAGE_UP = 0xD3;
const uint8_t KEY_PAGE_DOWN = 0xD6;
const uint8_t KEY_HOME = 0xD2;
const uint8_t KEY_END = 0xD5;
const uint8_t KEY_CAPS_LOCK = 0xC1;
const uint8_t KEY_F1 = 0xC2;
const uint8_t KEY_F2 = 0xC3;
const uint8_t KEY_F3 = 0xC4;
const uint8_t KEY_F4 = 0xC5;
const uint8_t KEY_F5 = 0xC6;
const uint8_t KEY_F6 = 0xC7;
const uint8_t KEY_F7 = 0xC8;
const uint8_t KEY_F8 = 0xC9;
const uint8_t KEY_F9 = 0xCA;
const uint8_t KEY_F10 = 0xCB;
const uint8_t KEY_F11 = 0xCC;
const uint8_t KEY_F12 = 0xCD;
const uint8_t KEY_F13 = 0xF0;
const uint8_t KEY_F14 = 0xF1;
const uint8_t KEY_F15 = 0xF2;
const uint8_t KEY_F16 = 0xF3;
const uint8_t KEY_F17 = 0xF4;
const uint8_t KEY_F18 = 0xF5;
const uint8_t KEY_F19 = 0xF6;
const uint8_t KEY_F20 = 0xF7;
const uint8_t KEY_F21 = 0xF8;
const uint8_t KEY_F22 = 0xF9;
const uint8_t KEY_F23 = 0xFA;
const uint8_t KEY_F24 = 0xFB;

typedef uint8_t MediaKeyReport[2];

const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};
const MediaKeyReport KEY_MEDIA_WWW_HOME = {128, 0};
const MediaKeyReport KEY_MEDIA_LOCAL_MACHINE_BROWSER = {0, 1}; // Opens "My Computer" on Windows
const MediaKeyReport KEY_MEDIA_CALCULATOR = {0, 2};
const MediaKeyReport KEY_MEDIA_WWW_BOOKMARKS = {0, 4};
const MediaKeyReport KEY_MEDIA_WWW_SEARCH = {0, 8};
const MediaKeyReport KEY_MEDIA_WWW_STOP = {0, 16};
const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 32};
const MediaKeyReport KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION = {0, 64}; // Media Selection
const MediaKeyReport KEY_MEDIA_EMAIL_READER = {0, 128};


//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

class BleKeypad : public Print
{
private:
  uint32_t _buttons;
  int16_t _x;
  int16_t _y;
  int16_t _z;
  int16_t _rZ;
  int16_t _rX;
  int16_t _rY;
  int16_t _hat;
  bool _autoReport;
  BleConnectionStatus* connectionStatus;
  BLEHIDDevice* hid;
  BLECharacteristic* inputKeyboard;
  BLECharacteristic* outputKeyboard;
  BLECharacteristic* inputMediaKeys;
  BLECharacteristic* inputGamepad;
  KeyReport _keyReport;
  MediaKeyReport _mediaKeyReport;
  void buttons(uint32_t b);
  void rawAction(uint8_t msg[], char msgSize);
  static void taskServer(void* pvParameter);
public:
  BleKeypad(std::string deviceName = "ESP32 BLE Keypad", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100);
  void begin(bool autoReport = true);
  void end(void);
  void sendReport(KeyReport* keys);
  void sendReport(MediaKeyReport* keys);
  size_t presskey(uint8_t k);
  size_t presskey(const MediaKeyReport k);
  size_t releasekey(uint8_t k);
  size_t releasekey(const MediaKeyReport k);
  size_t write(uint8_t c);
  size_t write(const MediaKeyReport c);
  size_t write(const uint8_t *buffer, size_t size);
  void releaseAll(void);
  void setAxes(int16_t x, int16_t y, int16_t z = 0, int16_t rZ = 0, char rX = 0, char rY = 0, signed char hat = 0);
  void pressbutton(uint32_t b = BUTTON_1);   // press BUTTON_1 by default
  void releasebutton(uint32_t b = BUTTON_1); // release BUTTON_1 by default
  void setLeftThumb(int16_t x = 0, int16_t y = 0);
  void setRightThumb(int16_t z = 0, int16_t rZ = 0);
  void setLeftTrigger(char rX = 0);
  void setRightTrigger(char rY = 0);
  void setHat(signed char hat = 0);
  void setX(int16_t x = 0);
  void setY(int16_t y = 0);
  void setZ(int16_t z = 0);
  void setRZ(int16_t rZ = 0);
  void setRX(int16_t rX = 0);
  void setRY(int16_t rY = 0);
  void setAutoReport(bool autoReport = true);
  void sendReport();
  bool isPressed(uint32_t b = BUTTON_1); // check BUTTON_1 by default
  bool isConnected(void);
  void setBatteryLevel(uint8_t level);
  uint8_t batteryLevel;
  std::string deviceManufacturer;
  std::string deviceName;
protected:
  virtual void onStarted(BLEServer *pServer) { };
};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_KEYPAD_H
