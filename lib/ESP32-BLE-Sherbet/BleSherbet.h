#ifndef ESP32_BLE_SHERBET_H
#define ESP32_BLE_SHERBET_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BleConnectionStatus.h"
#include "BLEHIDDevice.h"
#include "BLECharacteristic.h"
#include "Print.h"

const uint8_t KEY_LEFT_CTRL = 0x80;
const uint8_t KEY_LEFT_SHIFT = 0x81;
const uint8_t KEY_LEFT_ALT = 0x82;
const uint8_t KEY_LEFT_GUI = 0x83;
const uint8_t KEY_RIGHT_CTRL = 0x84;
const uint8_t KEY_RIGHT_SHIFT = 0x85;
const uint8_t KEY_RIGHT_ALT = 0x86;
const uint8_t KEY_RIGHT_GUI = 0x87;

const uint8_t KEY_A = 0x04;
const uint8_t KEY_B = 0x05;
const uint8_t KEY_C = 0x06;
const uint8_t KEY_D = 0x07;
const uint8_t KEY_E = 0x08;
const uint8_t KEY_F = 0x09;
const uint8_t KEY_G = 0x0A;
const uint8_t KEY_H = 0x0B;
const uint8_t KEY_I = 0x0C;
const uint8_t KEY_J = 0x0D;
const uint8_t KEY_K = 0x0E;
const uint8_t KEY_L = 0x0F;
const uint8_t KEY_M = 0x10;
const uint8_t KEY_N = 0x11;
const uint8_t KEY_O = 0x12;
const uint8_t KEY_P = 0x13;
const uint8_t KEY_Q = 0x14;
const uint8_t KEY_R = 0x15;
const uint8_t KEY_S = 0x16;
const uint8_t KEY_T = 0x17;
const uint8_t KEY_U = 0x18;
const uint8_t KEY_V = 0x19;
const uint8_t KEY_W = 0x1A;
const uint8_t KEY_X = 0x1B;
const uint8_t KEY_Y = 0x1C;
const uint8_t KEY_Z = 0x1D;

const uint8_t KEY_1 = 0x1E;
const uint8_t KEY_2 = 0x1F;
const uint8_t KEY_3 = 0x20;
const uint8_t KEY_4 = 0x21;
const uint8_t KEY_5 = 0x22;
const uint8_t KEY_6 = 0x23;
const uint8_t KEY_7 = 0x24;
const uint8_t KEY_8 = 0x25;
const uint8_t KEY_9 = 0x26;
const uint8_t KEY_0 = 0x27;
const uint8_t KEY_HYPHEN = 0x2D;
const uint8_t KEY_EQUALS = 0x2E;
const uint8_t KEY_LEFT_BRACKET = 0x2F;
const uint8_t KEY_RIGHT_BRACKET = 0x30;
const uint8_t KEY_BACKSLASH = 0x31;
const uint8_t KEY_FWDSLASH = 0x38;
const uint8_t KEY_POUND = 0x32;
const uint8_t KEY_SEMICOLON = 0x33;
const uint8_t KEY_APOSTROPHE = 0x34;
const uint8_t KEY_GRAVE = 0x35;
const uint8_t KEY_COMMA = 0x36;
const uint8_t KEY_PERIOD = 0x37;
const uint8_t KEY_PRINT_SCREEN = 0x46;
const uint8_t KEY_SCROLL_LOCK = 0x47;
const uint8_t KEY_PAUSE = 0x48;
const uint8_t KEY_NUM_LOCK = 0x53;
const uint8_t KEY_SPACE = 0x2C;

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

// Using Reserved Usage ID's to prevent overlap with something strange
const uint8_t KEY_MEDIA_NEXT_TRACK = 0xE9; 
const MediaKeyReport NEXT_TRACK = {1, 0};
const uint8_t KEY_MEDIA_PREVIOUS_TRACK = 0xEA; 
const MediaKeyReport PREVIOUS_TRACK = {2, 0};
const uint8_t KEY_MEDIA_STOP = 0xEB; 
const MediaKeyReport MEDIA_STOP = {4, 0};
const uint8_t KEY_MEDIA_PLAY_PAUSE = 0xEC; 
const MediaKeyReport PLAY_PAUSE = {8, 0};
const uint8_t KEY_MEDIA_MUTE = 0xED; 
const MediaKeyReport MEDIA_MUTE = {16, 0};
const uint8_t KEY_MEDIA_VOLUME_UP = 0xEE; 
const MediaKeyReport VOLUME_UP = {32, 0};
const uint8_t KEY_MEDIA_VOLUME_DOWN = 0xEF; 
const MediaKeyReport VOLUME_DOWN = {64, 0};
const uint8_t KEY_MEDIA_WWW_HOME = 0xF1; //{128, 0};
const uint8_t KEY_MEDIA_LOCAL_MACHINE_BROWSER = 0xF2; //{0, 1}; // Opens "My Computer" on Windows
const uint8_t KEY_MEDIA_CALCULATOR = 0xF3; //{0, 2};
const uint8_t KEY_MEDIA_WWW_BOOKMARKS = 0xF4; //{0, 4};
const uint8_t KEY_MEDIA_WWW_SEARCH = 0xF5; //{0, 8};
const uint8_t KEY_MEDIA_WWW_STOP = 0xF6; //{0, 16};
const uint8_t KEY_MEDIA_WWW_BACK = 0xF7; //{0, 32};
const uint8_t KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION = 0xF8; //{0, 64}; // Media Selection
const uint8_t KEY_MEDIA_EMAIL_READER = 0xF9; //{0, 128};


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
  BleKeypad(std::string deviceName = "ESP32 Sherbet Gamepad", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100);
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
  bool isConnected(void);
  void setBatteryLevel(uint8_t level);
  uint8_t batteryLevel;
  std::string deviceManufacturer;
  std::string deviceName;
protected:
  virtual void onStarted(BLEServer *pServer) { };
};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_SHERBET_H