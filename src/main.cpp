#include <Arduino.h>
#include "BleSherbet.h"
#include "Bounce2.h"

const int ROW_COUNT = 4;
const int COL_COUNT = 6;
const int BTN_COUNT = 1;
const int MODE_COUNT = 3;

const int DEBOUNCE = 3;
const int SCAN_DELAY = 1;

const int JOY_X_PIN = 35;
const int JOY_Y_PIN = 34;
const bool REV_X = true;
const bool REV_Y = true;
const int MIN_ADC = 0;
const int MAX_ADC = 4096;
static int MIN_X = 400;
static int CNTR_X = 1350;
static int MAX_X = 2800;
static int MIN_Y = 400;
static int CNTR_Y = 1250;
static int MAX_Y = 2800;
const int JOY_MIN = -32767;
const int JOY_MAX = 32767;

const int rowPins[] = {16, 17, 18, 19};
const int colPins[] = {21, 22, 23, 25, 26, 27};
const int buttonPins[] = {15};
const int modePins[] = {2, 4, 5};
const int ledPins[] = {12, 13, 14};

Bounce btnDebounce[BTN_COUNT];
Bounce switches[ROW_COUNT][COL_COUNT];
//Bounce rowDebounce[ROW_COUNT];
Bounce modeDebounce[MODE_COUNT];

boolean buttonStatus[BTN_COUNT];
boolean keyStatus[ROW_COUNT][COL_COUNT];

BleKeypad bleKeypad;

int keyLayer = 0;
const int totalKeyLayers = 2;

int x_read = 0;
int y_read = 0;
int axes_current[] = {512, 512};
int axes_prev[] = {0,0};
const int keyMap[totalKeyLayers][ROW_COUNT][COL_COUNT] = {
{
	{KEY_ESC, KEY_O, KEY_I, KEY_Q, KEY_V, KEY_T},
	{KEY_P, KEY_J, KEY_X, KEY_LEFT_ALT, KEY_E, KEY_R},
	{KEY_L, KEY_LEFT_CTRL, KEY_TAB, KEY_LEFT_SHIFT, KEY_SPACE, KEY_F},
	{KEY_K, KEY_S, KEY_W, KEY_C, KEY_Z, KEY_M}
},
{
	{BUTTON_1,BUTTON_2,BUTTON_3,BUTTON_4,BUTTON_5,BUTTON_6},
	{BUTTON_7,BUTTON_8,BUTTON_9,BUTTON_10,BUTTON_11,BUTTON_12},
	{BUTTON_13,BUTTON_14,BUTTON_15,BUTTON_16,BUTTON_17,BUTTON_18},
	{BUTTON_19,BUTTON_20,BUTTON_21,BUTTON_22,BUTTON_23,BUTTON_24}
}};

void stickCal(){
	CNTR_X = analogRead(JOY_X_PIN);
	CNTR_Y = analogRead(JOY_Y_PIN);
}

int reMap(int in, int in_low, int in_high, int out_low, int out_high, int cntr){
	if (in<cntr){
		float in_ratio = (cntr-in)/(cntr-in_low);
		float out_cntr = ((out_high - out_low)/2) + out_low;
		int out = ((out_cntr - out_low) * in_ratio ) + out_low;
		return out;
	} else {
		float in_ratio = (in_high - in)/(in_high - cntr);
		float out_cntr = ((out_high - out_low)/2) + out_low;
		int out = ((out_high - out_cntr)* in_ratio);
		return out;
	}
	return 0;
}

void keyScanner() {
	for (int col = 0; col < COL_COUNT; col++){
		//Set the current column to GND state
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], LOW);
		
		//Scan all buttons in the column for changes
		for (int row = 0; row < ROW_COUNT; row++){
			switches[row][col].update();
			if (switches[row][col].fell()){
				Serial.print(keyMap[keyLayer][row][col], HEX);
				Serial.print(" was pressed\n");
				if (keyLayer != 1){
					bleKeypad.presskey(keyMap[keyLayer][row][col]);
					return;
				} else if (keyLayer == 1){
					bleKeypad.press(keyMap[keyLayer][row][col]);
					bleKeypad.sendReport();
					return;
				}
			} else if (switches[row][col].rose()){
				Serial.print(keyMap[keyLayer][row][col], HEX);
				Serial.print(" was released\n");
				if (keyLayer !=1){
					bleKeypad.releasekey(keyMap[keyLayer][row][col]);
					return;
				} else if (keyLayer == 1){
					bleKeypad.release(keyMap[keyLayer][row][col]);
					bleKeypad.sendReport();
					return;
				}
			}
		}
		
		//Switch off the current column
		//pinMode(colPins[col], INPUT);
		digitalWrite(colPins[col], HIGH);
	}
}

void joyScanner(){
	x_read = analogRead(JOY_X_PIN);
	y_read = analogRead(JOY_Y_PIN);
	if (x_read > MAX_X){
		MAX_X = x_read;
	}
	if (x_read < MIN_X){
		MIN_X = x_read;
	}
	if (y_read > MAX_Y){
		MAX_Y = y_read;
	}
	if (y_read < MIN_Y){
		MIN_Y = y_read;
	}
	if (!REV_X){
		//Update analog stick values with current deflection
		axes_current[0] = map(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MIN, JOY_MAX);
		//axes_current[0] = reMap(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MIN, JOY_MAX, CNTR_X);
	} else {
		//Reversed X axis
		axes_current[0] = map(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MAX, JOY_MIN);
		//axes_current[0] = reMap(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MAX, JOY_MIN, CNTR_X);
	}
	if (!REV_Y){
		//Update analog stick values with current deflection
		axes_current[1] = map(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MIN, JOY_MAX);
		//axes_current[1] = reMap(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MIN, JOY_MAX, CNTR_Y);
	} else {
		//Reversed y axis
		axes_current[1] = map(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MAX, JOY_MIN);
		//axes_current[1] = reMap(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MAX, JOY_MIN, CNTR_Y);
	}
	//bleKeypad.setAxes(axes_current[0], axes_current[1],0,0,0,0, DPAD_CENTERED);
	bleKeypad.setX(axes_current[0]);
	bleKeypad.setY(axes_current[1]);
	bleKeypad.sendReport();
}

void modeScanner(){
	btnDebounce[0].update();
	if (btnDebounce[0].fell()){
		keyLayer++;
	}
	if (keyLayer>1){
		keyLayer = 0;
	}
}

void setup() {
  //Configure serial port for debug messages
	Serial.begin(115200);
  	bleKeypad.begin();
	
	//Set all columns to INPUT (high-impedance)
	for (int col = 0; col < COL_COUNT; col++){
		//pinMode(colPins[col], INPUT);
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], HIGH);
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
	stickCal();
	Serial.print("Setup complete\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(bleKeypad.isConnected()) {
      modeScanner();
	  keyScanner();
	  joyScanner();
	  delay(SCAN_DELAY);
  }
}