#ifndef ESP32_BLE_SHERBET_H
#define ESP32_BLE_SHERBET_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BleConnectionStatus.h"
#include "BLEHIDDevice.h"
#include "BLECharacteristic.h"
#include "Print.h"
#include <Arduino.h>

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

enum XInputControl : uint8_t {
	BUTTON_LOGO = 0,
	BUTTON_A  = 1,
	BUTTON_B = 2,
	BUTTON_X = 3,
	BUTTON_Y = 4,
	BUTTON_LB = 5,
	BUTTON_RB = 6,
	BUTTON_BACK = 7,
	BUTTON_START = 8,
	BUTTON_L3 = 9,
	BUTTON_R3 = 10,
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	TRIGGER_LEFT,
	TRIGGER_RIGHT,
	JOY_LEFT,
	JOY_RIGHT,
};

enum class XInputReceiveType {
	Rumble = 0x00,
	LEDs = 0x01,
};

//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

class BleKeypad : public Print
{
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
  void press(uint8_t button);
	void release(uint8_t button);
	void setButton(uint8_t button, boolean state);

	void setDpad(XInputControl pad, boolean state);
	void setDpad(boolean up, boolean down, boolean left, boolean right, boolean useSOCD = true);

	void setTrigger(XInputControl trigger, int32_t val);

	void setJoystick(XInputControl joy, int32_t x, int32_t y);
	void setJoystick(XInputControl joy, boolean up, boolean down, boolean left, boolean right, boolean useSOCD = true);
	void setJoystickX(XInputControl joy, int32_t x);
	void setJoystickY(XInputControl joy, int32_t y);

	void gamepadReleaseAll();

	// Auto-Send Data
	void setAutoSend(boolean a);

	// Read Control Surfaces
	boolean getButton(uint8_t button) const;
	boolean getDpad(XInputControl dpad) const;
	uint8_t getTrigger(XInputControl trigger) const;
	int16_t getJoystickX(XInputControl joy) const;
	int16_t getJoystickY(XInputControl joy) const;

	// Received Data
	uint8_t getPlayer() const;  // Player # assigned to the controller (0 is unassigned)

	uint16_t getRumble() const;  // Rumble motors. MSB is large weight, LSB is small
	uint8_t  getRumbleLeft() const;  // Large rumble motor, left grip
	uint8_t  getRumbleRight() const; // Small rumble motor, right grip

	//XInputLEDPattern getLEDPattern() const;  // Returns LED pattern type

	// Received Data Callback
	using RecvCallbackType = void(*)(uint8_t packetType);
	void setReceiveCallback(RecvCallbackType);

	// USB IO
	boolean connected();
	void send();
	int receive();

	// Control Input Ranges
	struct Range { int32_t min; int32_t max; };

	void setTriggerRange(int32_t rangeMin, int32_t rangeMax);
	void setJoystickRange(int32_t rangeMin, int32_t rangeMax);
	void setRange(XInputControl ctrl, int32_t rangeMin, int32_t rangeMax);

	// Setup
	void reset();

	// Debug
	void printDebug(Print& output = Serial) const;
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
  // Sent Data
	uint8_t tx[20];  // USB transmit data
	boolean newData;  // Flag for tx data changed
	boolean autoSendOption;  // Flag for automatically sending data
	
	void setJoystickDirect(XInputControl joy, int16_t x, int16_t y);

	void inline autosend() {
		if (autoSendOption) { send(); }
	}

	// Received Data
	volatile uint8_t player;  // Gamepad player #, buffered
	volatile uint8_t rumble[2];  // Rumble motor data in, buffered
	//volatile XInputLEDPattern ledPattern;  // LED pattern data in, buffered
	RecvCallbackType recvCallback;  // User-set callback for received data

	void parseLED(uint8_t leds);  // Parse LED data and set pattern/player data

	// Control Input Ranges
	Range rangeTrigLeft, rangeTrigRight, rangeJoyLeft, rangeJoyRight;
	Range * getRangeFromEnum(XInputControl ctrl);
	int32_t rescaleInput(int32_t val, Range in, Range out);
protected:
  virtual void onStarted(BLEServer *pServer) { };
};

class XInputKeypad
{
public:
	XInputKeypad();

	void begin();

	// Set Control Surfaces
	void press(uint8_t button);
	void release(uint8_t button);
	void setButton(uint8_t button, boolean state);

	void setDpad(XInputControl pad, boolean state);
	void setDpad(boolean up, boolean down, boolean left, boolean right, boolean useSOCD = true);

	void setTrigger(XInputControl trigger, int32_t val);

	void setJoystick(XInputControl joy, int32_t x, int32_t y);
	void setJoystick(XInputControl joy, boolean up, boolean down, boolean left, boolean right, boolean useSOCD = true);
	void setJoystickX(XInputControl joy, int32_t x);
	void setJoystickY(XInputControl joy, int32_t y);

	void releaseAll();

	// Auto-Send Data
	void setAutoSend(boolean a);

	// Read Control Surfaces
	boolean getButton(uint8_t button) const;
	boolean getDpad(XInputControl dpad) const;
	uint8_t getTrigger(XInputControl trigger) const;
	int16_t getJoystickX(XInputControl joy) const;
	int16_t getJoystickY(XInputControl joy) const;

	// Received Data
	uint8_t getPlayer() const;  // Player # assigned to the controller (0 is unassigned)

	uint16_t getRumble() const;  // Rumble motors. MSB is large weight, LSB is small
	uint8_t  getRumbleLeft() const;  // Large rumble motor, left grip
	uint8_t  getRumbleRight() const; // Small rumble motor, right grip

	//XInputLEDPattern getLEDPattern() const;  // Returns LED pattern type

	// Received Data Callback
	using RecvCallbackType = void(*)(uint8_t packetType);
	void setReceiveCallback(RecvCallbackType);

	// USB IO
	boolean connected();
	int send();
	int receive();

	// Control Input Ranges
	struct Range { int32_t min; int32_t max; };

	void setTriggerRange(int32_t rangeMin, int32_t rangeMax);
	void setJoystickRange(int32_t rangeMin, int32_t rangeMax);
	void setRange(XInputControl ctrl, int32_t rangeMin, int32_t rangeMax);

	// Setup
	void reset();

	// Debug
	void printDebug(Print& output = Serial) const;

private:
	// Sent Data
	uint8_t tx[20];  // USB transmit data
	boolean newData;  // Flag for tx data changed
	boolean autoSendOption;  // Flag for automatically sending data
	
	void setJoystickDirect(XInputControl joy, int16_t x, int16_t y);

	void inline autosend() {
		if (autoSendOption) { send(); }
	}

	// Received Data
	volatile uint8_t player;  // Gamepad player #, buffered
	volatile uint8_t rumble[2];  // Rumble motor data in, buffered
	//volatile XInputLEDPattern ledPattern;  // LED pattern data in, buffered
	RecvCallbackType recvCallback;  // User-set callback for received data

	void parseLED(uint8_t leds);  // Parse LED data and set pattern/player data

	// Control Input Ranges
	Range rangeTrigLeft, rangeTrigRight, rangeJoyLeft, rangeJoyRight;
	Range * getRangeFromEnum(XInputControl ctrl);
	int32_t rescaleInput(int32_t val, Range in, Range out);
};

extern XInputKeypad XInput;

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_SHERBET_H
