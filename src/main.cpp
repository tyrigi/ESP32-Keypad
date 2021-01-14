#include <Arduino.h>
#include "BleKeypad.h"
#include "Bounce2.h"

#define BTN1 14
#define BTN2 26
#define LED1 12
#define LED2 25
#define DEBOUNCE_INTERVAL 5
#define SCAN_DELAY 10

BleKeypad bleKeypad;
Bounce btn1;
Bounce btn2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting BLE Service");
  bleKeypad.begin();
  Serial.println("BLE Service Started");
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  btn1.attach(BTN1, INPUT_PULLUP);
  btn2.attach(BTN2, INPUT_PULLUP);
  btn1.interval(DEBOUNCE_INTERVAL);
  btn2.interval(DEBOUNCE_INTERVAL);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(bleKeypad.isConnected()) {
    //Serial.println("Sending 'Hello world'...");
    //bleKeypad.print("Hello world");
    btn1.update();
    btn2.update();
    if(btn1.fell())
    {
      digitalWrite(LED1, HIGH);
      bleKeypad.presskey(KEY_LEFT_ARROW);
    }
    else if (btn1.rose())
    {
      digitalWrite(LED1, LOW);
      bleKeypad.releasekey(KEY_LEFT_ARROW);
    }
    if(btn2.fell())
    {
      digitalWrite(LED2, HIGH);
      bleKeypad.presskey(KEY_RIGHT_ARROW);
    }
    else if (btn2.rose())
    {
      digitalWrite(LED2, LOW);
      bleKeypad.releasekey(KEY_RIGHT_ARROW);
    }    
    delay(SCAN_DELAY);
  }

  //Serial.println("Waiting 5 seconds...");
  //delay(5000);
}