/* 
 * This is the master code for the ESP32 Wireless Sherbet Keypad, based on Billiam's
 * design (https://www.billiam.org/).
 * 
 * ToDo:
 *  - Add a timeout to deep sleep after a period of inactivity for additional power savings
 *  - Implement user-definable layouts imported from a text file on an attached SD card.
 */

#include <Arduino.h>
#include "BleSherbet.h"
#include "Bounce2.h"

const int ROW_COUNT = 4;
const int COL_COUNT = 6;
const int BTN_COUNT = 1;
const int MODE_COUNT = 3;

const int DEBOUNCE = 3;
const int SCAN_DELAY = 20;

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
static float JOY_DEVIATION = 0.03;
static int prev_x = 0;
static int prev_y = 0;

const int BAT_PIN = 32;
const int BAT_MAX = 3960;
const int BAT_MIN = 2925;
const int LEVEL_SAMPLES = 128;
static int BAT_LEVEL = 100;
static int PREV_LEVEL = 0;
static int AVG_BAT_LEVEL = 0;
static int BAT_ADC = 3960;
static int MEASURE_COUNT = 0;

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

static bool update = false;
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

/*
Scan the key matrix for changes in key status. The row pins are have
alread been set as inputs with a weak pullup, so now the columns are 
set to a logic LOW state. When a button in the currently LOW column is 
pressed, the cooresponding row pin detects a change from HIGH to LOW.
When the switch is released, the opposite is true. 

We're only interested in changes in key state (unpressed -> pressed or 
pressed -> unpressed), so we only look for rising or falling edges. When
one is detected, the key code from the layout matrix cooresponding to the
correct row and column is transmitted to the computer.

The update variable is only set true when a key press or a key release
event is logged.
*/
void keyScanner() {
	for (int col = 0; col < COL_COUNT; col++){
		//Set the current column to GND state
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], LOW);
		
		//Scan all buttons in the column for changes
		for (int row = 0; row < ROW_COUNT; row++){
			switches[row][col].update();
			if (switches[row][col].fell()){
				if (keyLayer != 1){
					bleKeypad.presskey(keyMap[keyLayer][row][col]);
					update = true;
					return;
				// Layer 1 is gamepad buttons, which uses a different "press" method
				} else if (keyLayer == 1){
					bleKeypad.press(keyMap[keyLayer][row][col]);
					update = true;
					bleKeypad.sendReport();
					return;
				}
			} else if (switches[row][col].rose()){
				if (keyLayer !=1){
					bleKeypad.releasekey(keyMap[keyLayer][row][col]);
					update = true;
					return;
				// Layer 1 is gamepad buttons, which uses a different "press" method
				} else if (keyLayer == 1){
					bleKeypad.release(keyMap[keyLayer][row][col]);
					update = true;
					bleKeypad.sendReport();
					return;
				}
			}
		}
		
		//Switch off the current to a HIGH logic state 
		digitalWrite(colPins[col], HIGH);
	}
}


/*
This function scans the Joystick for changes. We read the voltage
at the X axis and Y axis, which gives a value from 0 to 4096. This
value is then re-mapped to the joystick outputs of -32767 to 32767.

To save power by supressing unnecessary updates, the X and Y values
of the Joystick are only updated if the value changes by a certain
percentage from the previous reading. This percentage is set by 
JOY_DEVIATION.
*/
void joyScanner(){
	x_read = analogRead(JOY_X_PIN);
	y_read = analogRead(JOY_Y_PIN);

	float tempX = (float)abs(x_read - prev_x) / (float)prev_x;
	float tempY = (float)abs(y_read - prev_y) / (float)prev_y;

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
	} else {
		//Reversed X axis
		axes_current[0] = map(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MAX, JOY_MIN);
	}
	if (!REV_Y){
		//Update analog stick values with current deflection
		axes_current[1] = map(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MIN, JOY_MAX);
	} else {
		//Reversed y axis
		axes_current[1] = map(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MAX, JOY_MIN);
	}

	if (tempX > JOY_DEVIATION || tempY > JOY_DEVIATION){
		prev_x = x_read;
		prev_y = y_read;
		bleKeypad.setX(axes_current[0]);
		bleKeypad.setY(axes_current[1]);
		bleKeypad.sendReport();
		update = true;
	}
}


/*
This function checks for a button press to switch between
layouts. The only 'button' I've set up on this keypad is
the Joystick push button (when you press on the stick). 

No update needs to be logged for this event since the 
computer doesn't need to be informed of layout changes.
*/
void modeScanner(){
	btnDebounce[0].update();
	if (btnDebounce[0].fell()){
		keyLayer++;
	}
	if (keyLayer>1){
		keyLayer = 0;
	}
}


/*
This function reads the voltage level of the battery,
then scales it relative to the minimum and maximum 
battery voltages for reporting a percentage. The
keypad uses a single Li-Ion cell, which has a max 
voltage of 4.2V (when fully charged). I've set the
minimum to 3.0V to keep the battery from being fully
discharged. This will impact battery lifespan a bit,
but it should make the battery last longer before
needing to be replaced.

To save on unnecessary updates (which saves power), 
an update is only logged if the battery percentage
changes by 5%. And only increments of 5% are logged,
since the voltage reading can fluctuate by 1-2%.
*/
void updateBatLevel(){
	MEASURE_COUNT++;
	BAT_ADC = analogRead(BAT_PIN);
	BAT_LEVEL = map(constrain(BAT_ADC, BAT_MIN, BAT_MAX), BAT_MIN, BAT_MAX, 0, 100);
	AVG_BAT_LEVEL += BAT_LEVEL;
	if (MEASURE_COUNT == LEVEL_SAMPLES){
		AVG_BAT_LEVEL = AVG_BAT_LEVEL/LEVEL_SAMPLES;
		AVG_BAT_LEVEL = (AVG_BAT_LEVEL/5)*5;
		if (AVG_BAT_LEVEL != PREV_LEVEL){
			PREV_LEVEL = AVG_BAT_LEVEL;
			bleKeypad.setBatteryLevel(AVG_BAT_LEVEL);
			update = true;
		}
		MEASURE_COUNT = 0;
		AVG_BAT_LEVEL = 0;
	}
}


/*
Set up the hardware and instantiate all necessary objects/variables.
*/
void setup() {
  	//Configure Serial port for debug messages
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
	//Serial.print("Setup complete\n");
}

void loop() {
	//Don't bother doing anything if we're not connected to anything.
  	if(bleKeypad.isConnected()) {
		//Check for a layer change
      	modeScanner();
		//Check for keypresses
	  	keyScanner();
		//Check for joystick movement
	  	joyScanner();
		//Check for a change in battery level
	  	updateBatLevel();
		//If any changes have been logged, send an update
	  	if (update==true){
		  	bleKeypad.sendUpdate();
		  	update = false;
		}
		//No need to free-wheel, that just uses unnecessary power.
	  	delay(SCAN_DELAY);
  	}
}