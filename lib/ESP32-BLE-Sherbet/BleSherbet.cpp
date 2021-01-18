#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
//#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"

#include "BleConnectionStatus.h"
#include "KeypadOutputCallbacks.h"
#include "BleSherbet.h"
#include <Arduino.h>

#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG "BLEKeypad"
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEKeypad";
#endif


// Report IDs:
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop Ctrls)
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  // ------------------------------------------------- Keyboard
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1)
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0)
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   Logical Maximum (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 1 byte (Reserved)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8)
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5) ; 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) ; Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) ; Kana
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 3 bits (Padding)
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3)
  HIDOUTPUT(1),       0x01,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6) ; 6 bytes (Keys)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- Media Keys
  USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
  USAGE(1),           0x01,          // USAGE (Consumer Control)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  REPORT_ID(1),       MEDIA_KEYS_ID, //   REPORT_ID (3)
  USAGE_PAGE(1),      0x0C,          //   USAGE_PAGE (Consumer)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x10,          //   REPORT_COUNT (16)
  USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
  USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
  USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
  USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
  USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
  USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
  USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
  USAGE(2),           0x23, 0x02,    //   Usage (WWW Home)            ; bit 7: 128
  USAGE(2),           0x94, 0x01,    //   Usage (My Computer) ; bit 0: 1
  USAGE(2),           0x92, 0x01,    //   Usage (Calculator)  ; bit 1: 2
  USAGE(2),           0x2A, 0x02,    //   Usage (WWW fav)     ; bit 2: 4
  USAGE(2),           0x21, 0x02,    //   Usage (WWW search)  ; bit 3: 8
  USAGE(2),           0x26, 0x02,    //   Usage (WWW stop)    ; bit 4: 16
  USAGE(2),           0x24, 0x02,    //   Usage (WWW back)    ; bit 5: 32
  USAGE(2),           0x83, 0x01,    //   Usage (Media sel)   ; bit 6: 64
  USAGE(2),           0x8A, 0x01,    //   Usage (Mail)        ; bit 7: 128
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0)                  // END_COLLECTION
};

BleKeypad::BleKeypad(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) : hid(0)
{
  this->deviceName = deviceName;
  this->deviceManufacturer = deviceManufacturer;
  this->batteryLevel = batteryLevel;
  this->connectionStatus = new BleConnectionStatus();
}

void BleKeypad::begin(bool autoReport)
{
  Serial.begin(115200);
  _autoReport = autoReport;
  xTaskCreate(this->taskServer, "server", 20000, (void *)this, 5, NULL);
}

void BleKeypad::end(void)
{
}

bool BleKeypad::isConnected(void) {
  return this->connectionStatus->connected;
}

void BleKeypad::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
    this->hid->setBatteryLevel(this->batteryLevel);
}

void BleKeypad::taskServer(void* pvParameter) {
  BleKeypad* bleKeypadInstance = (BleKeypad *) pvParameter; //static_cast<BleKeyboard *>(pvParameter);
  BLEDevice::init(bleKeypadInstance->deviceName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleKeypadInstance->connectionStatus);

  bleKeypadInstance->hid = new BLEHIDDevice(pServer);
  bleKeypadInstance->inputKeyboard = bleKeypadInstance->hid->inputReport(KEYBOARD_ID); // <-- input REPORTID from report map
  bleKeypadInstance->outputKeyboard = bleKeypadInstance->hid->outputReport(KEYBOARD_ID);
  bleKeypadInstance->inputMediaKeys = bleKeypadInstance->hid->inputReport(MEDIA_KEYS_ID);
  bleKeypadInstance->connectionStatus->inputKeyboard = bleKeypadInstance->inputKeyboard;
  bleKeypadInstance->connectionStatus->outputKeyboard = bleKeypadInstance->outputKeyboard;
  bleKeypadInstance->connectionStatus->inputMediaKeys = bleKeypadInstance->inputMediaKeys;

  bleKeypadInstance->outputKeyboard->setCallbacks(new KeypadOutputCallbacks());

  bleKeypadInstance->hid->manufacturer()->setValue(bleKeypadInstance->deviceManufacturer);

  bleKeypadInstance->hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  bleKeypadInstance->hid->hidInfo(0x00,0x01);

  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  bleKeypadInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleKeypadInstance->hid->startServices();

  bleKeypadInstance->onStarted(pServer);

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(bleKeypadInstance->hid->hidService()->getUUID());
  pAdvertising->setScanResponse(false);
  pAdvertising->start();
  bleKeypadInstance->hid->setBatteryLevel(bleKeypadInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}

void BleKeypad::sendReport(KeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
    this->inputKeyboard->notify();
  }
}

void BleKeypad::sendReport(MediaKeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
    this->inputMediaKeys->notify();
  }
}

uint8_t USBPutChar(uint8_t c);

// press() adds the specified key (printing, non-printing, or modifier)
// to the persistent key report and sends the report.  Because of the way
// USB HID works, the host acts like the key remains pressed until we
// call release(), releaseAll(), or otherwise clear the report and resend.
size_t BleKeypad::presskey(uint8_t k)
{
	uint8_t i;
  Serial.print("Pressing key ");
  Serial.print(k, HEX);
  Serial.print("\n");
    
	if (k >= 128 && k <= 135) {			// it's a modifier key
		_keyReport.modifiers |= (1<<k);
    k = 0;
    Serial.print("modifier key pressed\n");
	}

  if (k >= 0xE9){
    switch (k){
      case 0xE9: this->presskey(NEXT_TRACK);
      case 0xEA: this->presskey(PREVIOUS_TRACK);
      case 0xEB: this->presskey(MEDIA_STOP);
      case 0xEC: this->presskey(PLAY_PAUSE);
      case 0xED: this->presskey(MEDIA_MUTE);
      case 0xEE: this->presskey(VOLUME_UP);
      case 0xEF: this->presskey(VOLUME_DOWN);
    }
    Serial.print("Media Key Pressed\n");
  }

	// Add k to the key report only if it's not already present
	// and if there is an empty slot.
	if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
		_keyReport.keys[2] != k && _keyReport.keys[3] != k &&
		_keyReport.keys[4] != k && _keyReport.keys[5] != k) {

		for (i=0; i<6; i++) {
			if (_keyReport.keys[i] == 0x00) {
				_keyReport.keys[i] = k;
				break;
			}
		}
		if (i == 6) {
			setWriteError();
			return 0;
		}
	}
	sendReport(&_keyReport);
  Serial.print("Keypress sent!\n");
	return 1;
}

size_t BleKeypad::presskey(const MediaKeyReport k)
{
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);

    mediaKeyReport_16 |= k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

	sendReport(&_mediaKeyReport);
	return 1;
}

// release() takes the specified key out of the persistent key report and
// sends the report.  This tells the OS the key is no longer pressed and that
// it shouldn't be repeated any more.
size_t BleKeypad::releasekey(uint8_t k)
{
	uint8_t i;
  Serial.print("Releasing key ");
  Serial.print(k, HEX);
  Serial.print("\n");

	if (k >= 128 && k <= 135) {			// it's a modifier key
		_keyReport.modifiers &= ~(1<<k);
    k = 0;
    Serial.print("Modifier Key released\n");
	}

  if (k >= 0xE9){
    switch (k){
      case 0xE9: this->releasekey(NEXT_TRACK);
      case 0xEA: this->releasekey(PREVIOUS_TRACK);
      case 0xEB: this->releasekey(MEDIA_STOP);
      case 0xEC: this->releasekey(PLAY_PAUSE);
      case 0xED: this->releasekey(MEDIA_MUTE);
      case 0xEE: this->releasekey(VOLUME_UP);
      case 0xEF: this->releasekey(VOLUME_DOWN);
    }
    Serial.print("media key released\n");
  }

	// Test the key report to see if k is present.  Clear it if it exists.
	// Check all positions in case the key is present more than once (which it shouldn't be)
	for (i=0; i<6; i++) {
		if (0 != k && _keyReport.keys[i] == k) {
			_keyReport.keys[i] = 0x00;
		}
	}

	sendReport(&_keyReport);
  Serial.print("Rleased key sent!\n");
	return 1;
}

size_t BleKeypad::releasekey(const MediaKeyReport k)
{
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
    mediaKeyReport_16 &= ~k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

	sendReport(&_mediaKeyReport);
	return 1;
}

void BleKeypad::releaseAll(void)
{
	_keyReport.keys[0] = 0;
	_keyReport.keys[1] = 0;
	_keyReport.keys[2] = 0;
	_keyReport.keys[3] = 0;
	_keyReport.keys[4] = 0;
	_keyReport.keys[5] = 0;
	_keyReport.modifiers = 0;
    _mediaKeyReport[0] = 0;
    _mediaKeyReport[1] = 0;
	sendReport(&_keyReport);
}

size_t BleKeypad::write(uint8_t c)
{
	uint8_t p = presskey(c);  // Keydown
	releasekey(c);            // Keyup
	return p;              // just return the result of press() since release() almost always returns 1
}

size_t BleKeypad::write(const MediaKeyReport c)
{
	uint16_t p = presskey(c);  // Keydown
	releasekey(c);            // Keyup
	return p;              // just return the result of press() since release() almost always returns 1
}

size_t BleKeypad::write(const uint8_t *buffer, size_t size) {
	size_t n = 0;
	while (size--) {
		if (*buffer != '\r') {
			if (write(*buffer)) {
			  n++;
			} else {
			  break;
			}
		}
		buffer++;
	}
	return n;
}

struct XInputMap_Button {
	constexpr XInputMap_Button(uint8_t i, uint8_t o)
		: index(i), mask(BuildMask(o)) {}
	const uint8_t index;
	const uint8_t mask;

private:
	constexpr static uint8_t BuildMask(uint8_t offset) {
		return (1 << offset);  // Bitmask of bit to flip
	}
};

static const XInputMap_Button Map_DpadUp(2, 0);
static const XInputMap_Button Map_DpadDown(2, 1);
static const XInputMap_Button Map_DpadLeft(2, 2);
static const XInputMap_Button Map_DpadRight(2, 3);
static const XInputMap_Button Map_ButtonStart(2, 4);
static const XInputMap_Button Map_ButtonBack(2, 5);
static const XInputMap_Button Map_ButtonL3(2, 6);
static const XInputMap_Button Map_ButtonR3(2, 7);

static const XInputMap_Button Map_ButtonLB(3, 0);
static const XInputMap_Button Map_ButtonRB(3, 1);
static const XInputMap_Button Map_ButtonLogo(3, 2);
static const XInputMap_Button Map_ButtonA(3, 4);
static const XInputMap_Button Map_ButtonB(3, 5);
static const XInputMap_Button Map_ButtonX(3, 6);
static const XInputMap_Button Map_ButtonY(3, 7);

const XInputMap_Button * getButtonFromEnum(XInputControl ctrl) {
	switch (ctrl) {
	case(DPAD_UP):      return &Map_DpadUp;
	case(DPAD_DOWN):    return &Map_DpadDown;
	case(DPAD_LEFT):    return &Map_DpadLeft;
	case(DPAD_RIGHT):   return &Map_DpadRight;
	case(BUTTON_A):     return &Map_ButtonA;
	case(BUTTON_B):     return &Map_ButtonB;
	case(BUTTON_X):     return &Map_ButtonX;
	case(BUTTON_Y):     return &Map_ButtonY;
	case(BUTTON_LB):    return &Map_ButtonLB;
	case(BUTTON_RB):    return &Map_ButtonRB;
	case(JOY_LEFT):
	case(BUTTON_L3):    return &Map_ButtonL3;
	case(JOY_RIGHT):
	case(BUTTON_R3):    return &Map_ButtonR3;
	case(BUTTON_START): return &Map_ButtonStart;
	case(BUTTON_BACK):  return &Map_ButtonBack;
	case(BUTTON_LOGO):  return &Map_ButtonLogo;
	default: return nullptr;
	}
}

// --------------------------------------------------------
// XInput Trigger Maps                                    |
// (Matches ID to tx index)                               |
// --------------------------------------------------------

struct XInputMap_Trigger {
	constexpr XInputMap_Trigger(uint8_t i)
		: index(i) {}
	static const BleKeypad::Range range;
	const uint8_t index;
};

const BleKeypad::Range XInputMap_Trigger::range = { 0, 255 };  // uint8_t

static const XInputMap_Trigger Map_TriggerLeft(4);
static const XInputMap_Trigger Map_TriggerRight(5);

const XInputMap_Trigger * getTriggerFromEnum(XInputControl ctrl) {
	switch (ctrl) {
	case(TRIGGER_LEFT): return &Map_TriggerLeft;
	case(TRIGGER_RIGHT): return &Map_TriggerRight;
	default: return nullptr;
	}
}

// --------------------------------------------------------
// XInput Joystick Maps                                   |
// (Matches ID to tx x/y high/low indices)                |
// --------------------------------------------------------

struct XInputMap_Joystick {
	constexpr XInputMap_Joystick(uint8_t xl, uint8_t xh, uint8_t yl, uint8_t yh)
		: x_low(xl), x_high(xh), y_low(yl), y_high(yh) {}
	static const BleKeypad::Range range;
	const uint8_t x_low;
	const uint8_t x_high;
	const uint8_t y_low;
	const uint8_t y_high;
};

const BleKeypad::Range XInputMap_Joystick::range = { -32768, 32767 };  // int16_t

static const XInputMap_Joystick Map_JoystickLeft(6, 7, 8, 9);
static const XInputMap_Joystick Map_JoystickRight(10, 11, 12, 13);

const XInputMap_Joystick * getJoyFromEnum(XInputControl ctrl) {
	switch (ctrl) {
	case(JOY_LEFT): return &Map_JoystickLeft;
	case(JOY_RIGHT): return &Map_JoystickRight;
	default: return nullptr;
	}
}

// --------------------------------------------------------
// XInput Rumble Maps                                     |
// (Stores rx index and buffer index for each motor)      |
// --------------------------------------------------------

struct XInputMap_Rumble {
	constexpr XInputMap_Rumble(uint8_t rIndex, uint8_t bIndex)
		: rxIndex(rIndex), bufferIndex(bIndex) {}
	const uint8_t rxIndex;
	const uint8_t bufferIndex;
};

static const XInputMap_Rumble RumbleLeft(3, 0);   // Large motor
static const XInputMap_Rumble RumbleRight(4, 1);  // Small motor

// --------------------------------------------------------
// XInput USB Receive Callback                            |
// --------------------------------------------------------

#ifdef USB_XINPUT
static void XInputLib_Receive_Callback() {
	XInput.receive();
}
#endif


// --------------------------------------------------------
// XInputController Class (API)                           |
// --------------------------------------------------------

//BleKeypad::BleKeypad() :
//	tx(), rumble() // Zero initialize arrays
//{
//	reset();
//#ifdef USB_XINPUT
//	XInputUSB::setRecvCallback(XInputLib_Receive_Callback);
//#endif
//}

//void BleKeypad::begin() {
	// Empty for now
//}

void BleKeypad::press(uint8_t button) {
	setButton(button, true);
}

void BleKeypad::release(uint8_t button) {
	setButton(button, false);
}

void BleKeypad::setButton(uint8_t button, boolean state) {
	const XInputMap_Button * buttonData = getButtonFromEnum((XInputControl) button);
	if (buttonData != nullptr) {
		if (getButton(button) == state) return;  // Button hasn't changed

		if (state) { tx[buttonData->index] |= buttonData->mask; }  // Press
		else { tx[buttonData->index] &= ~(buttonData->mask); }  // Release
		newData = true;
		autosend();
	}
	else {
		Range * triggerRange = getRangeFromEnum((XInputControl) button);
		if (triggerRange == nullptr) return;  // Not a trigger (or joystick, but the trigger function will ignore that)
		setTrigger((XInputControl) button, state ? triggerRange->max : triggerRange->min);  // Treat trigger like a button
	}
}

void BleKeypad::setDpad(XInputControl pad, boolean state) {
	setButton(pad, state);
}

void BleKeypad::setDpad(boolean up, boolean down, boolean left, boolean right, boolean useSOCD) {
	// Simultaneous Opposite Cardinal Directions (SOCD) Cleaner
	if (useSOCD) {
		if (up && down) { down = false; }  // Up + Down = Up
		if (left && right) { left = false; right = false; }  // Left + Right = Neutral
	}

	const boolean autoSendTemp = autoSendOption;  // Save autosend state
	autoSendOption = false;  // Disable temporarily

	setDpad(DPAD_UP, up);
	setDpad(DPAD_DOWN, down);
	setDpad(DPAD_LEFT, left);
	setDpad(DPAD_RIGHT, right);

	autoSendOption = autoSendTemp;  // Re-enable from option
	autosend();
}

void BleKeypad::setTrigger(XInputControl trigger, int32_t val) {
	const XInputMap_Trigger * triggerData = getTriggerFromEnum(trigger);
	if (triggerData == nullptr) return;  // Not a trigger

	val = rescaleInput(val, *getRangeFromEnum(trigger), XInputMap_Trigger::range);
	if (getTrigger(trigger) == val) return;  // Trigger hasn't changed

	tx[triggerData->index] = val;
	newData = true;
	autosend();
}

void BleKeypad::setJoystick(XInputControl joy, int32_t x, int32_t y) {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return;  // Not a joystick

	x = rescaleInput(x, *getRangeFromEnum(joy), XInputMap_Joystick::range);
	y = rescaleInput(y, *getRangeFromEnum(joy), XInputMap_Joystick::range);

	setJoystickDirect(joy, x, y);
}

void BleKeypad::setJoystickX(XInputControl joy, int32_t x) {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return;  // Not a joystick

	x = rescaleInput(x, *getRangeFromEnum(joy), XInputMap_Joystick::range);

	if (getJoystickX(joy) == x) return;  // Axis hasn't changed

	tx[joyData->x_low] = lowByte(x);
	tx[joyData->x_high] = highByte(x);

	newData = true;
	autosend();
}

void BleKeypad::setJoystickY(XInputControl joy, int32_t y) {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return;  // Not a joystick

	y = rescaleInput(y, *getRangeFromEnum(joy), XInputMap_Joystick::range);

	if (getJoystickY(joy) == y) return;  // Axis hasn't changed

	tx[joyData->y_low] = lowByte(y);
	tx[joyData->y_high] = highByte(y);

	newData = true;
	autosend();
}

void BleKeypad::setJoystick(XInputControl joy, boolean up, boolean down, boolean left, boolean right, boolean useSOCD) {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return;  // Not a joystick

	const Range & range = XInputMap_Joystick::range;

	int16_t x = 0;
	int16_t y = 0;

	// Simultaneous Opposite Cardinal Directions (SOCD) Cleaner
	if (useSOCD) {
		if (up && down) { down = false; }  // Up + Down = Up
		if (left && right) { left = false; right = false; }  // Left + Right = Neutral
	}
	
	// Analog axis means directions are mutually exclusive. Only change the
	// output from '0' if the per-axis inputs are different, in order to
	// avoid the '-1' result from adding the int16 extremes
	if (left != right) {
		if (left == true) { x = range.min; }
		else if (right == true) { x = range.max; }
	}
	if (up != down) {
		if (up == true) { y = range.max; }
		else if (down == true) { y = range.min; }
	}

	setJoystickDirect(joy, x, y);
}

void BleKeypad::setJoystickDirect(XInputControl joy, int16_t x, int16_t y) {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return;  // Not a joystick

	if (getJoystickX(joy) == x && getJoystickY(joy) == y) return;  // Joy hasn't changed

	tx[joyData->x_low]  = lowByte(x);
	tx[joyData->x_high] = highByte(x);

	tx[joyData->y_low]  = lowByte(y);
	tx[joyData->y_high] = highByte(y);

	newData = true;
	autosend();
}

void BleKeypad::gamepadReleaseAll() {
	const uint8_t offset = 2;  // Skip message type and packet size
	memset(tx + offset, 0x00, sizeof(tx) - offset);  // Clear TX array
	newData = true;  // Data changed and is unsent
	autosend();
}

void BleKeypad::setAutoSend(boolean a) {
	autoSendOption = a;
}

boolean BleKeypad::getButton(uint8_t button) const {
	const XInputMap_Button * buttonData = getButtonFromEnum((XInputControl) button);
	if (buttonData == nullptr) return 0;  // Not a button
	return tx[buttonData->index] & buttonData->mask;
}

boolean BleKeypad::getDpad(XInputControl dpad) const {
	return getButton(dpad);
}

uint8_t BleKeypad::getTrigger(XInputControl trigger) const {
	const XInputMap_Trigger * triggerData = getTriggerFromEnum(trigger);
	if (triggerData == nullptr) return 0;  // Not a trigger
	return tx[triggerData->index];
}

int16_t BleKeypad::getJoystickX(XInputControl joy) const {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return 0;  // Not a joystick
	return (tx[joyData->x_high] << 8) | tx[joyData->x_low];
}

int16_t BleKeypad::getJoystickY(XInputControl joy) const {
	const XInputMap_Joystick * joyData = getJoyFromEnum(joy);
	if (joyData == nullptr) return 0;  // Not a joystick
	return (tx[joyData->y_high] << 8) | tx[joyData->y_low];
}

uint8_t BleKeypad::getPlayer() const {
	return player;
}

uint16_t BleKeypad::getRumble() const {
	return rumble[RumbleLeft.bufferIndex] << 8 | rumble[RumbleRight.bufferIndex];
}

uint8_t BleKeypad::getRumbleLeft() const {
	return rumble[RumbleLeft.bufferIndex];
}

uint8_t BleKeypad::getRumbleRight() const {
	return rumble[RumbleRight.bufferIndex];
}

//XInputLEDPattern BleKeypad::getLEDPattern() const {
//	return ledPattern;
//}

void BleKeypad::setReceiveCallback(RecvCallbackType cback) {
	recvCallback = cback;
}

//boolean BleKeypad::connected() {
//
//}

//Send an update packet to the PC
void BleKeypad::send() {
  if (this->isConnected()){
    this->inputGamepad->setValue(this->tx, sizeof(this->tx));
    this->inputGamepad->notify();
  }
/*	if (!newData) return 0;  // TX data hasn't changed
#ifdef USB_XINPUT
	newData = false;
	return XInputUSB::send(tx, sizeof(tx));
#else
	printDebug();
	return sizeof(tx);
#endif*/
}

int BleKeypad::receive() {
#ifdef USB_XINPUT
	if (XInputUSB::available() == 0) {
		return 0;  // No packet available
	}

	// Grab packet and store it in rx array
	uint8_t rx[8];
	const int bytesRecv = XInputUSB::recv(rx, sizeof(rx));

	// Only process if received 3 or more bytes (min valid packet size)
	if (bytesRecv >= 3) {
		const uint8_t PacketType = rx[0];

		// Rumble Packet
		if (PacketType == (uint8_t)XInputReceiveType::Rumble) {
			rumble[RumbleLeft.bufferIndex] = rx[RumbleLeft.rxIndex];   // Big weight (Left grip)
			rumble[RumbleRight.bufferIndex] = rx[RumbleRight.rxIndex];  // Small weight (Right grip)
		}
		// LED Packet
		else if (PacketType == (uint8_t)XInputReceiveType::LEDs) {
			parseLED(rx[2]);
		}

		// User-defined receive callback
		if (recvCallback != nullptr) {
			recvCallback(PacketType);
		}
	}

	return bytesRecv;
#else
	return 0;
#endif
}

/*void BleKeypad::parseLED(uint8_t leds) {
	if (leds > 0x0D) return;  // Not a known pattern

	ledPattern = (XInputLEDPattern) leds;  // Save pattern
	switch (ledPattern) {
	case(XInputLEDPattern::Off):
	case(XInputLEDPattern::Blinking):
		player = 0;  // Not connected
		break;
	case(XInputLEDPattern::On1):
	case(XInputLEDPattern::Flash1):
		player = 1;
		break;
	case(XInputLEDPattern::On2):
	case(XInputLEDPattern::Flash2):
		player = 2;
		break;
	case(XInputLEDPattern::On3):
	case(XInputLEDPattern::Flash3):
		player = 3;
		break;
	case(XInputLEDPattern::On4):
	case(XInputLEDPattern::Flash4):
		player = 4;
		break;
	default: return;  // Pattern doesn't affect player #
	}
}*/

BleKeypad::Range * BleKeypad::getRangeFromEnum(XInputControl ctrl) {
	switch (ctrl) {
	case(TRIGGER_LEFT): return &rangeTrigLeft;
	case(TRIGGER_RIGHT): return &rangeTrigRight;
	case(JOY_LEFT): return &rangeJoyLeft;
	case(JOY_RIGHT): return &rangeJoyRight;
	default: return nullptr;
	}
}

int32_t BleKeypad::rescaleInput(int32_t val, Range in, Range out) {
	if (val <= in.min) return out.min;  // Out of range -
	if (val >= in.max) return out.max;  // Out of range +
	if (in.min == out.min && in.max == out.max) return val;  // Ranges identical
	return map(val, in.min, in.max, out.min, out.max);
}

void BleKeypad::setTriggerRange(int32_t rangeMin, int32_t rangeMax) {
	setRange(TRIGGER_LEFT, rangeMin, rangeMax);
	setRange(TRIGGER_RIGHT, rangeMin, rangeMax);
}

void BleKeypad::setJoystickRange(int32_t rangeMin, int32_t rangeMax) {
	setRange(JOY_LEFT, rangeMin, rangeMax);
	setRange(JOY_RIGHT, rangeMin, rangeMax);
}

void BleKeypad::setRange(XInputControl ctrl, int32_t rangeMin, int32_t rangeMax) {
	if (rangeMin >= rangeMax) return;  // Error: Max < Min

	Range * range = getRangeFromEnum(ctrl);
	if (range == nullptr) return;  // Not an addressable range

	range->min = rangeMin;
	range->max = rangeMax;
}

// Resets class back to initial values
void BleKeypad::reset() {
	// Reset control data (tx)
	releaseAll();  // Clear TX buffer
	tx[0] = 0x00;  // Set tx message type
	tx[1] = 0x14;  // Set tx packet size (20)

	// Reset received data (rx)
	player = 0;  // Not connected, no player
	memset((void*) rumble, 0x00, sizeof(rumble));  // Clear rumble values
	//ledPattern = XInputLEDPattern::Off;  // No LEDs on

	// Reset rescale ranges
	setTriggerRange(XInputMap_Trigger::range.min, XInputMap_Trigger::range.max);
	setJoystickRange(XInputMap_Joystick::range.min, XInputMap_Joystick::range.max);

	// Clear user-set options
	recvCallback = nullptr;
	autoSendOption = true;
}

static void fillBuffer(char* buff, const char fill) {
	uint8_t i = 0;
	while (true) {
		if (buff[i] == 0) break;
		buff[i] = fill;
		i++;
	}
}

void BleKeypad::printDebug(Print &output) const {
	const char fillCharacter = '_';
	char buffer[34];

	output.print("XInput Debug: ");

	// Left Side Controls
	char leftBumper[3] = "LB";
	char leftJoyBtn[3] = "L3";

	if (!getButton(BUTTON_LB)) fillBuffer(leftBumper, fillCharacter);
	if (!getButton(BUTTON_L3)) fillBuffer(leftJoyBtn, fillCharacter);

	sprintf(buffer,
		"LT: %3u %s L:(%6d, %6d, %s)",

		getTrigger(TRIGGER_LEFT),
		leftBumper,
		getJoystickX(JOY_LEFT), getJoystickY(JOY_LEFT),
		leftJoyBtn
	);
	output.print(buffer);

	// Face Buttons
	const char dpadLPrint = getButton(DPAD_LEFT)  ? '<' : fillCharacter;
	const char dpadUPrint = getButton(DPAD_UP)    ? '^' : fillCharacter;
	const char dpadDPrint = getButton(DPAD_DOWN)  ? 'v' : fillCharacter;
	const char dpadRPrint = getButton(DPAD_RIGHT) ? '>' : fillCharacter;

	const char aButtonPrint = getButton(BUTTON_A) ? 'A' : fillCharacter;
	const char bButtonPrint = getButton(BUTTON_B) ? 'B' : fillCharacter;
	const char xButtonPrint = getButton(BUTTON_X) ? 'X' : fillCharacter;
	const char yButtonPrint = getButton(BUTTON_Y) ? 'Y' : fillCharacter;

	const char startPrint = getButton(BUTTON_START) ? '>' : fillCharacter;
	const char backPrint  = getButton(BUTTON_BACK)  ? '<' : fillCharacter;

	const char logoPrint = getButton(BUTTON_LOGO) ? 'X' : fillCharacter;

	sprintf(buffer,
		" %c%c%c%c | %c%c%c | %c%c%c%c ",

		dpadLPrint, dpadUPrint, dpadDPrint, dpadRPrint,
		backPrint, logoPrint, startPrint,
		aButtonPrint, bButtonPrint, xButtonPrint, yButtonPrint
	);
	output.print(buffer);

	// Right Side Controls
	char rightBumper[3] = "RB";
	char rightJoyBtn[3] = "R3";

	if (!getButton(BUTTON_RB)) fillBuffer(rightBumper, fillCharacter);
	if (!getButton(BUTTON_R3)) fillBuffer(rightJoyBtn, fillCharacter);

	sprintf(buffer,
		"R:(%6d, %6d, %s) %s RT: %3u",
		
		getJoystickX(JOY_RIGHT), getJoystickY(JOY_RIGHT),
		rightJoyBtn,
		rightBumper,
		getTrigger(TRIGGER_RIGHT)
	);
	output.println(buffer);
}

//BleKeypad XInput;