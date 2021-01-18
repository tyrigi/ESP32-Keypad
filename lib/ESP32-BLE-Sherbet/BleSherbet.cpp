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
#define GAMEPAD_ID 0x04

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
  END_COLLECTION(0),                  // END_COLLECTION
  // ------------------------------------------------- Gamepad
  //USAGE_PAGE(1),       0x01, // USAGE_PAGE (Generic Desktop)
  USAGE(1),            0x05, // USAGE (Gamepad)
  COLLECTION(1),       0x01, // COLLECTION (Application)
  USAGE(1),            0x01, //   USAGE (Pointer)
  COLLECTION(1),       0x00, //   COLLECTION (Physical)
  REPORT_ID(1),       GAMEPAD_ID,   //   REPORT_ID (1)

  // ------------------------------------------------- Buttons (1 to 32)
  USAGE_PAGE(1),       0x09, //     USAGE_PAGE (Button)
  USAGE_MINIMUM(1),    0x01, //     USAGE_MINIMUM (Button 1)
  USAGE_MAXIMUM(1),    0x20, //     USAGE_MAXIMUM (Button 32)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1),  0x01, //     LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),      0x01, //     REPORT_SIZE (1)
  REPORT_COUNT(1),     0x20, //     REPORT_COUNT (32)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute) ;32 button bits
  // ------------------------------------------------- X/Y position, Z/rZ position
  USAGE_PAGE(1), 	   0x01, //		USAGE_PAGE (Generic Desktop)
  COLLECTION(1), 	   0x00, //		COLLECTION (Physical)
  USAGE(1), 		   0x30, //     USAGE (X)
  USAGE(1), 		   0x31, //     USAGE (Y)
  USAGE(1), 		   0x32, //     USAGE (Z)
  USAGE(1), 		   0x35, //     USAGE (rZ)
  0x16, 			   0x01, 0x80,//LOGICAL_MINIMUM (-32767)
  0x26, 			   0xFF, 0x7F,//LOGICAL_MAXIMUM (32767)
  REPORT_SIZE(1), 	   0x10, //		REPORT_SIZE (16)
  REPORT_COUNT(1), 	   0x04, //		REPORT_COUNT (4)
  HIDINPUT(1), 		   0x02, //     INPUT (Data,Var,Abs)
  // ------------------------------------------------- Triggers
  USAGE(1),            0x33, //     USAGE (rX) Left Trigger
  USAGE(1),            0x34, //     USAGE (rY) Right Trigger
  LOGICAL_MINIMUM(1),  0x81, //     LOGICAL_MINIMUM (-127)
  LOGICAL_MAXIMUM(1),  0x7f, //     LOGICAL_MAXIMUM (127)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x02, //     REPORT_COUNT (2)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute) ;4 bytes (X,Y,Z,rZ)
  END_COLLECTION(0),		 //     END_COLLECTION
  
  USAGE_PAGE(1),       0x01, //     USAGE_PAGE (Generic Desktop)
  USAGE(1),            0x39, //     USAGE (Hat switch)
  USAGE(1),            0x39, //     USAGE (Hat switch)
  LOGICAL_MINIMUM(1),  0x01, //     LOGICAL_MINIMUM (1)
  LOGICAL_MAXIMUM(1),  0x08, //     LOGICAL_MAXIMUM (8)
  REPORT_SIZE(1),      0x04, //     REPORT_SIZE (4)
  REPORT_COUNT(1),     0x02, //     REPORT_COUNT (2)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute) ;1 byte Hat1, Hat2

  END_COLLECTION(0)//,         //     END_COLLECTION
  //END_COLLECTION(0)          //     END_COLLECTION
};

BleKeypad::BleKeypad(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) : 
	hid(0), 
	_buttons(0),
	_x(0),
  	_y(0),
  	_z(0),
  	_rZ(0),
  	_rX(0),
  	_rY(0),
  	_hat(0),
  	_autoReport(false)
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
  //reset();
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
  bleKeypadInstance->inputGamepad = bleKeypadInstance->hid->inputReport(GAMEPAD_ID);
  bleKeypadInstance->connectionStatus->inputKeyboard = bleKeypadInstance->inputKeyboard;
  bleKeypadInstance->connectionStatus->outputKeyboard = bleKeypadInstance->outputKeyboard;
  bleKeypadInstance->connectionStatus->inputMediaKeys = bleKeypadInstance->inputMediaKeys;
  bleKeypadInstance->connectionStatus->inputGamepad = bleKeypadInstance->inputGamepad;

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
  pAdvertising->setAppearance(HID_GAMEPAD);
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
//-----------------------------------------------------------------------------
// Gamepad methods
//-----------------------------------------------------------------------------
void BleKeypad::setAxes(int16_t x, int16_t y, int16_t z, int16_t rZ, char rX, char rY, signed char hat)
{
	if(x == -32768) { x = -32767; }
	if(y == -32768) { y = -32767; }
	if(z == -32768) { z = -32767; }
	if(rZ == -32768) { rZ = -32767; }

	_x = x;
	_y = y;
	_z = z;
	_rZ = rZ;
	_rX = rX;
	_rY = rY;
	_hat = hat;
	
	if(_autoReport){ sendGamepadReport(); }
}

void BleKeypad::sendGamepadReport(void)
{
	if (this->isConnected())
	{
		uint8_t m[15];
		m[0] = _buttons;
		m[1] = (_buttons >> 8);
		m[2] = (_buttons >> 16);
		m[3] = (_buttons >> 24);
		m[4] = _x;
		m[5] = (_x >> 8);
		m[6] = _y;
		m[7] = (_y >> 8);
		m[8] = _z;
		m[9] = (_z >> 8);
		m[10] = _rZ;
		m[11] = (_rZ >> 8);
		m[12] = (signed char)(_rX - 128);
		m[13] = (signed char)(_rY - 128);
		m[14] = _hat;
		if (m[12] == -128) { m[12] = -127; }
		if (m[13] == -128) { m[13] = -127; }
		this->inputGamepad->setValue(m, sizeof(m));
		this->inputGamepad->notify();
	}
}
void BleKeypad::buttons(uint32_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
	
	if(_autoReport){ sendGamepadReport(); }
  }
}

void BleKeypad::press(uint32_t b)
{
  buttons(_buttons | b);
}

void BleKeypad::release(uint32_t b)
{
  buttons(_buttons & ~b);
}

void BleKeypad::setLeftThumb(int16_t x, int16_t y)
{
	_x = x;
	_y = y;
	
	if(_autoReport){ sendGamepadReport(); }
}
void BleKeypad::setRightThumb(int16_t z, int16_t rZ)
{
	_z = z;
	_rZ = rZ;
	
	if(_autoReport){ sendGamepadReport(); }
}

void BleKeypad::setLeftTrigger(char rX)
{
	_rX = rX;
	
	if(_autoReport){ sendGamepadReport(); }
}

void BleKeypad::setRightTrigger(char rY)
{
	_rY = rY;
	
	if(_autoReport){ sendGamepadReport(); }
}

void BleKeypad::setHat(signed char hat)
{
	_hat = hat;
	
	if(_autoReport){ sendGamepadReport(); }
}

void BleKeypad::setX(int16_t x)
{
	_x = x;
}

void BleKeypad::setY(int16_t y)
{
	_y = y;
}

void BleKeypad::setZ(int16_t z)
{
	_z = z;
}

void BleKeypad::setRZ(int16_t rZ)
{
	_rZ = rZ;
}

void BleKeypad::setRX(int16_t rX)
{
	_rX = rX;
}

void BleKeypad::setRY(int16_t rY)
{
	_rY = rY;
}

void BleKeypad::setAutoReport(bool autoReport)
{
	_autoReport = autoReport;
}

bool BleKeypad::isPressed(uint32_t b)
{
  if ((b & _buttons) > 0)
    return true;
  return false;
}