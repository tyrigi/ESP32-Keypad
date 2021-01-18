#include <Arduino.h>
#include "BleSherbet.h"
#include "Bounce2.h"

const int ROW_COUNT = 4;
const int COL_COUNT = 6;
const int BTN_COUNT = 1;
const int MODE_COUNT = 3;

const int DEBOUNCE = 3;
const int SCAN_DELAY = 1;

const int JOY_X_PIN = 12;
const int JOY_Y_PIN = 14;
const bool REV_X = true;
const bool REV_Y = true;
const int MIN_X = 183;
const int CNTR_X = 505;
const int MAX_X = 828;
const int MIN_Y = 223;
const int CNTR_Y = 513;
const int MAX_Y = 815;
const int JOY_MIN = 0;
const int JOY_MAX = 1023;

const int rowPins[] = {16, 17, 18, 19};
const int colPins[] = {27, 26, 25, 23, 22, 21};
const int buttonPins[] = {12};
const int modePins[] = {18, 19, 20};
const int ledPins[] = {16, 17, 21};

Bounce btnDebounce[BTN_COUNT];
Bounce switches[ROW_COUNT][COL_COUNT];
//Bounce rowDebounce[ROW_COUNT];
Bounce modeDebounce[MODE_COUNT];

boolean buttonStatus[BTN_COUNT];
boolean keyStatus[ROW_COUNT][COL_COUNT];

BleKeypad bleKeypad;

int keyLayer = 0;
const int totalKeyLayers = 3;

int x_read = 0;
int y_read = 0;
int axes_current[] = {512, 512};
int axes_prev[] = {0,0};
const int keyMap[ROW_COUNT][COL_COUNT] = {
	{KEY_ESC, KEY_O, KEY_I, KEY_Q, KEY_V, KEY_T},
	{KEY_MEDIA_PREVIOUS_TRACK, KEY_J, KEY_X, KEY_LEFT_ALT, KEY_E, KEY_R},
	{KEY_MEDIA_PLAY_PAUSE, KEY_LEFT_CTRL, KEY_TAB, KEY_LEFT_SHIFT, KEY_SPACE, KEY_F},
	{KEY_MEDIA_NEXT_TRACK, KEY_S, KEY_W, KEY_C, KEY_Z, KEY_M}
};

void keyScanner() {
	for (int col = 0; col < COL_COUNT; col++){
		//Set the current column to GND state
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], LOW);
		
		//Scan all buttons in the column for changes
		for (int row = 0; row < ROW_COUNT; row++){
			switches[row][col].update();
			//Serial.print("Checking keys...\n");
			if (switches[row][col].fell()){
				Serial.print(keyMap[row][col], HEX);
				Serial.print(" was pressed\n");
				bleKeypad.presskey(keyMap[row][col]);
				//if (keyLayer != 2){
				//	bleKeypad.presskey(keyMap[row][col]);
				//} else if (keyLayer == 2){
					//XInput.setButton(keyMap[keyLayer][row][col], 1);
					//buttonPress(keyMap[keyLayer][row][col]);
					//Joystick.button(keyMap[row][col], 1);
				//}
			} else if (switches[row][col].rose()){
				Serial.print(keyMap[row][col], HEX);
				Serial.print(" was released\n");
				bleKeypad.releasekey(keyMap[row][col]);
				//if (keyLayer !=2){
				//	bleKeypad.releasekey(keyMap[row][col]);
				//} else if (keyLayer == 2){
					//XInput.setButton(keyMap[keyLayer][row][col], 0);
					//buttonRelease(keyMap[keyLayer][row][col]);
					//Joystick.button(keyMap[keyLayer][row][col], 0);
				//}
			}
		}
		
		//Switch off the current column
		pinMode(colPins[col], INPUT);
	}
}

void setup() {
  //Configure serial port for debug messages
	Serial.begin(115200);
  	bleKeypad.begin();
	
	//Configure XInput joystick settings
	//XInput.setJoystickRange(JOY_MIN, JOY_MAX); //Set minimum and maximum values for joystick range
	//XInput.setAutoSend(false); //Update joystick position manually
	
	//Set all columns to INPUT (high-impedance)
	for (int col = 0; col < COL_COUNT; col++){
		pinMode(colPins[col], INPUT);
	}
	
	//Create one debounce object per key
	for (int row = 0; row < ROW_COUNT; row++){
		for (int col = 0; col < COL_COUNT; col++){
			Bounce debouncer = Bounce();
			debouncer.attach(rowPins[row], INPUT_PULLUP);
			debouncer.interval(DEBOUNCE);
			switches[row][col] = debouncer;
		}
	}
	
	//Create debounce objects for extra buttons
	for (int btn = 0; btn < BTN_COUNT; btn++){
		Bounce debouncer = Bounce();
		debouncer.attach(buttonPins[btn], INPUT_PULLUP);
		debouncer.interval(DEBOUNCE);
		btnDebounce[btn] = debouncer;
	}
	
	//Create debounce objects for mode buttons
	for (int mode = 0; mode < MODE_COUNT; mode++){
		Bounce debouncer = Bounce();
		debouncer.attach(modePins[mode], INPUT);
		debouncer.interval(DEBOUNCE);
		modeDebounce[mode] = debouncer;
	}

	//Set mode LED's to correct mode
	//pinMode(ledPins[0], OUTPUT);
	//digitalWrite(ledPins[0], LOW);
	//pinMode(ledPins[1], OUTPUT);
	//digitalWrite(ledPins[1], LOW);
	//pinMode(ledPins[2], OUTPUT);
	//digitalWrite(ledPins[2], LOW);

	//digitalWrite(ledPins[keyLayer], HIGH);
	//XInput.begin();
	Serial.print("Setup complete\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(bleKeypad.isConnected()) {
    //modeScanner();
	  keyScanner();
	  //joyScanner();
	  delay(SCAN_DELAY);
	  //Serial.print(millis());
	  //Serial.print("\n");
  }
}