#ifndef ESP32_BLE_KEYPAD_OUTPUT_CALLBACKS_H
#define ESP32_BLE_KEYPAD_OUTPUT_CALLBACKS_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <BLEServer.h>
#include "BLE2902.h"
#include "BLECharacteristic.h"

class KeypadOutputCallbacks : public BLECharacteristicCallbacks
{
public:
  KeypadOutputCallbacks(void);
  void onWrite(BLECharacteristic* me);
  //void onConnect(BLEServer* pServer);
};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_KEYPAD_OUTPUT_CALLBACKS_H
