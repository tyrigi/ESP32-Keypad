#include "KeypadOutputCallbacks.h"

#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif

KeypadOutputCallbacks::KeypadOutputCallbacks(void) {
}

void KeypadOutputCallbacks::onWrite(BLECharacteristic* me) {
  uint8_t* value = (uint8_t*)(me->getValue().c_str());
  ESP_LOGI(LOG_TAG, "special keys: %d", *value);
}

/*void KeypadOutputCallbacks::onConnect(BLEServer* pServer){
  BLEDescriptor *desc = blekeypadInstance->inputGamepad->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  uint8_t val[] = {0x01, 0x00};
  desc->setValue(val, 2);
}*/
