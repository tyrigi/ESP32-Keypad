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
#define GAMEPAD_ID 0x03

static const uint8_t _hidReportDescriptor[] = {
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x06,        // Usage (Keyboard)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x01,        //   Report ID (1)
  0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
  0x19, 0xE0,        //   Usage Minimum (0xE0)
  0x29, 0xE7,        //   Usage Maximum (0xE7)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x08,        //   Report Count (8)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x95, 0x01,        //   Report Count (1)
  0x75, 0x08,        //   Report Size (8)
  0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x95, 0x05,        //   Report Count (5)
  0x75, 0x01,        //   Report Size (1)
  0x05, 0x08,        //   Usage Page (LEDs)
  0x19, 0x01,        //   Usage Minimum (Num Lock)
  0x29, 0x05,        //   Usage Maximum (Kana)
  0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  0x95, 0x01,        //   Report Count (1)
  0x75, 0x03,        //   Report Size (3)
  0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  0x95, 0x06,        //   Report Count (6)
  0x75, 0x08,        //   Report Size (8)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x65,        //   Logical Maximum (101)
  0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
  0x19, 0x00,        //   Usage Minimum (0x00)
  0x29, 0x65,        //   Usage Maximum (0x65)
  0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              // End Collection
  //0x05, 0x0C,        // Usage Page (Consumer)
  //0x09, 0x01,        // Usage (Consumer Control)
  //0xA1, 0x01,        // Collection (Application)
  //0x85, 0x02,        //   Report ID (2)
  //0x05, 0x0C,        //   Usage Page (Consumer)
  //0x15, 0x00,        //   Logical Minimum (0)
  //0x25, 0x01,        //   Logical Maximum (1)
  //0x75, 0x01,        //   Report Size (1)
  //0x95, 0x10,        //   Report Count (16)
  //0x09, 0xB5,        //   Usage (Scan Next Track)
  //0x09, 0xB6,        //   Usage (Scan Previous Track)
  //0x09, 0xB7,        //   Usage (Stop)
  //0x09, 0xCD,        //   Usage (Play/Pause)
  //0x09, 0xE2,        //   Usage (Mute)
  //0x09, 0xE9,        //   Usage (Volume Increment)
  //0x09, 0xEA,        //   Usage (Volume Decrement)
  //0x0A, 0x23, 0x02,  //   Usage (AC Home)
  //0x0A, 0x94, 0x01,  //   Usage (AL Local Machine Browser)
  //0x0A, 0x92, 0x01,  //   Usage (AL Calculator)
  //0x0A, 0x2A, 0x02,  //   Usage (AC Bookmarks)
  //0x0A, 0x21, 0x02,  //   Usage (AC Search)
  //0x0A, 0x26, 0x02,  //   Usage (AC Stop)
  //0x0A, 0x24, 0x02,  //   Usage (AC Back)
  //0x0A, 0x83, 0x01,  //   Usage (AL Consumer Control Configuration)
  //0x0A, 0x8A, 0x01,  //   Usage (AL Email Reader)
  //0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  //0xC0,              // End Collection
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x05,        // Usage (Game Pad)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x03,        //   Report ID (3)
  0x09, 0x01,        //   Usage (Pointer)
  0xA1, 0x00,        //   Collection (Physical)
  0x05, 0x09,        //     Usage Page (Button)
  0x19, 0x01,        //     Usage Minimum (0x01)
  0x29, 0x20,        //     Usage Maximum (0x20)
  0x15, 0x00,        //     Logical Minimum (0)
  0x25, 0x01,        //     Logical Maximum (1)
  0x75, 0x01,        //     Report Size (1)
  0x95, 0x20,        //     Report Count (32)
  0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
  0xA1, 0x00,        //     Collection (Physical)
  0x09, 0x30,        //       Usage (X)
  0x09, 0x31,        //       Usage (Y)
  0x09, 0x32,        //       Usage (Z)
  0x09, 0x35,        //       Usage (Rz)
  0x16, 0x01, 0x80,  //       Logical Minimum (-32767)
  0x26, 0xFF, 0x7F,  //       Logical Maximum (32767)
  0x75, 0x10,        //       Report Size (16)
  0x95, 0x04,        //       Report Count (4)
  0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x09, 0x33,        //       Usage (Rx)
  0x09, 0x34,        //       Usage (Ry)
  0x15, 0x81,        //       Logical Minimum (-127)
  0x25, 0x7F,        //       Logical Maximum (127)
  0x75, 0x08,        //       Report Size (8)
  0x95, 0x02,        //       Report Count (2)
  0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              //     End Collection
  0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
  0x09, 0x39,        //     Usage (Hat switch)
  0x09, 0x39,        //     Usage (Hat switch)
  0x15, 0x01,        //     Logical Minimum (1)
  0x25, 0x08,        //     Logical Maximum (8)
  0x75, 0x04,        //     Report Size (4)
  0x95, 0x02,        //     Report Count (2)
  0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              //   End Collection
  0xC0,              // End Collection
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
  //Serial.begin(115200);
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


// Note: This is only needed to get the gamepad stuff to keep sending data after a reconnect. 
// Suggesstion taken from https://github.com/nkolban/esp32-snippets/issues/632
  BLEDescriptor *desc = bleKeypadInstance->inputGamepad->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  uint8_t val[] = {0x01, 0x00};
  desc->setValue(val, 2);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}

void BleKeypad::sendUpdate()
{
  this->inputKeyboard->notify();
  this->inputGamepad->notify();
}

void BleKeypad::sendReport(KeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
    this->inputKeyboard->notify();
  }
}

void BleKeypad::sendMediaReport(uint8_t key)
{
  if (this->isConnected())
  {
    this->inputMediaKeys->setValue((uint8_t*)key, sizeof(key));
    this->inputMediaKeys->notify();
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

void BleKeypad::sendReport(void)
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
		//this->inputGamepad->notify();
    //Serial.print(m[5], HEX);
    //Serial.print(m[4], HEX);
    //Serial.print("X\n");
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
  //Serial.print("Pressing key ");
  //Serial.print(k, HEX);
  //Serial.print("\n");
    
	if (k >= 128 && k <= 135) {			// it's a modifier key
		_keyReport.modifiers |= (1<<k);
    k = 0;
    //Serial.print("modifier key pressed\n");
	}

  if (k >= 0xE9){
    switch (k){
      case 0xE9: 
        this->presskey(NEXT_TRACK);
        k=0;
        break;
      case 0xEA: 
        this->presskey(PREVIOUS_TRACK);
        k=0;
        break;
      case 0xEB: 
        this->presskey(MEDIA_STOP);
        k=0;
        break;
      case 0xEC: 
        this->presskey(PLAY_PAUSE);
        k=0;
        break;
      case 0xED: 
        this->presskey(MEDIA_MUTE);
        k=0;
        break;
      case 0xEE: 
        this->presskey(VOLUME_UP);
        k=0;
        break;
      case 0xEF: 
        this->presskey(VOLUME_DOWN);
        k=0;
        break;
    }
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
  //Serial.print("Releasing key ");
  //Serial.print(k, HEX);
  //Serial.print("\n");

	if (k >= 128 && k <= 135) {			// it's a modifier key
		_keyReport.modifiers &= ~(1<<k);
    k = 0;
    //Serial.print("Modifier Key released\n");
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
    //Serial.print("media key released\n");
  }

	// Test the key report to see if k is present.  Clear it if it exists.
	// Check all positions in case the key is present more than once (which it shouldn't be)
	for (i=0; i<6; i++) {
		if (0 != k && _keyReport.keys[i] == k) {
			_keyReport.keys[i] = 0x00;
		}
	}

	sendReport(&_keyReport);
  //Serial.print("Rleased key sent!\n");
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
	
	if(_autoReport){ sendReport(); }
}

void BleKeypad::buttons(uint32_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
	
	if(_autoReport){ sendReport(); }
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
	
	if(_autoReport){ sendReport(); }
}
void BleKeypad::setRightThumb(int16_t z, int16_t rZ)
{
	_z = z;
	_rZ = rZ;
	
	if(_autoReport){ sendReport(); }
}

void BleKeypad::setLeftTrigger(char rX)
{
	_rX = rX;
	
	if(_autoReport){ sendReport(); }
}

void BleKeypad::setRightTrigger(char rY)
{
	_rY = rY;
	
	if(_autoReport){ sendReport(); }
}

void BleKeypad::setHat(signed char hat)
{
	_hat = hat;
	
	if(_autoReport){ sendReport(); }
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