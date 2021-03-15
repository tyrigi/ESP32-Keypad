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

//#include "FS.h"
#include "SD.h"
#include <SPI.h>

//Abstracting out whether a key has been pressed or released
#define PRESSED(row, col) switches[row][col].fell()
#define RELEASED(row, col) switches[row][col].rose()

#define ISIN(CH, ARRY) std::find(std::begin(ARRY), std::end(ARRY), CH) != std::end(ARRY)
#define GETPOS(CH, ARRY) std::find(std::begin(ARRY), std::end(ARRY), CH)

//General info about the physical keypad
const int ROW_COUNT = 4; 							//Number of rows
const int COL_COUNT = 6; 							//Number of columns
const int BTN_COUNT = 1; 							//Number of buttons (for joystick stuff)
const int MODE_COUNT = 3; 							//Number of total Layouts - will be removed when layouts defined on SD Card
const int JOY_X_PIN = 35; 							//Joystick X Axis
const int JOY_Y_PIN = 34; 							//Joystick Y Axis
const int BAT_PIN = 32; 							//Battery level measurement 
const int rowPins[] = {4, 2, 17, 16};				//Row GPIO's
const int colPins[] = {21, 22, 33, 25, 26, 27}; 	//Column GPIO's
const int buttonPins[] = {15}; 						//Joystick button GPIO's
const int modePins[] = {2, 4, 5}; 					//Mode control buttons (carryover from teensy)
const int ledPins[] = {12, 13, 14}; 				//LED indicator pins (carryover from teensy)

//Descriptions of the scanning process
const int DEBOUNCE = 3; 							//Debounce period for button debouncing
const int SCAN_DELAY = 20; 							//Delay between scan cycles
static bool update = false;							//Indicator - cycle has registered a valid change that requires an update
const int COL_STATE = HIGH;							//State the column should be in when not being read
const int COL_MODE = OUTPUT;						//Mode that the column pins should be put in during setup
const int ROW_MODE = INPUT_PULLUP;					//Mode to initialize the rows to during startup

//Joystick information
const bool REV_X = true;							//Invert X Axis
const bool REV_Y = true;							//Invert Y Axis
const int MIN_ADC = 0;								//Minimum ADC reading for scaling output
const int MAX_ADC = 4096;							//Maximum ADC reading for scaling output
static int MIN_X = 400;								//Minimum typical X Axis ADC reading
static int MAX_X = 2800;							//Maximum typical X Axis ADC reading
static int MIN_Y = 400;								//Minimum typical Y Axis ADC reading
static int MAX_Y = 2800;							//Maximum typical Y Axis ADC reading
const int JOY_MIN = -32767;							//Minimum reportable Axis Value
const int JOY_MAX = 32767;							//Maximum reportable Axis Value
static float JOY_DEVIATION = 0.03;					//Minimum deviation from previous value before update
static int prev_x = 0;								//Previous X Axis reading
static int prev_y = 0;								//Previous Y Axis reading
int x_read = 0;										//Current cycle's X Axis ADC reading
int y_read = 0;										//Current cycle's Y Axis ADC reading
int axes_current[] = {512, 512};					//Current scaled Axis readings
int axes_prev[] = {0,0};							//Previous cycle's scaled Axis readings

//Battery measurement information
const int BAT_MAX = 3960;							//ADC value at maximum voltage (Battery @4.2V)
const int BAT_MIN = 2925;							//ADC value at minimum voltage (Battery @3.0V)
const int LEVEL_SAMPLES = 128;						//Number of samples to average over
static int BAT_LEVEL = 100;							//Battery level as percentage scaled from 4.2V to 3.0V (linear)
static int PREV_LEVEL = 0;							//Previous battery level reading
static int AVG_BAT_LEVEL = 0;						//Averaged battery level reading
static int BAT_ADC = 3960;							//Read ADC value (used for averaging)
static int MEASURE_COUNT = 0;						//Counted number of samples

//SD Card Stuff
#define SD_CS 5
static bool CARD_AVAILABLE = false;
const char LETTERS[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
							'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 
							'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 
							'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 
							'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
							'Y', 'Z'}; 
const char NUMBERS[] = {'0', '1', '2', '3', '4', '5', '6', 
							'7', '8', '9'};
const char SYMBOLS[] = {'_'};
const char COMMA[] = {','};

//Object instances
Bounce btnDebounce[BTN_COUNT];						//Joystic button debounce objects
Bounce switches[ROW_COUNT][COL_COUNT];				//Keypad key debounce objects
Bounce modeDebounce[MODE_COUNT];					//Mode button debounce objects

BleKeypad bleKeypad;								//The keypad object


//Temporary layout management variables
int keyLayer = 0;
const int totalKeyLayers = 2;
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

const int MAX_LAYERS = 5;
static String layout[MAX_LAYERS][ROW_COUNT][COL_COUNT];

/*
Determines whether a given character is a valid character for a key encoding.
*/
//bool charIsValid(char ch){
//	bool exists = std::find(std::begin(VALID_CHARS), std::end(VALID_CHARS), ch) != std::end(VALID_CHARS);
//	return exists;
//}


/*
Scan through all rows of a given column for state changes.
*/
void rowScan(int col) {
	for (int row = 0; row < ROW_COUNT; row++){
		switches[row][col].update();
	}

	for (int row = 0; row < ROW_COUNT; row++){
		if (PRESSED(row, col)){
			Serial.print(row, DEC);
			Serial.print(":");
			Serial.print(col, DEC);
			Serial.print(" Pressed\n");
			bleKeypad.presskey(keyMap[keyLayer][row][col]);
			update = true;
			return;
			//press the button of the right layer
		} else if (RELEASED(row, col)){
			Serial.print(row, DEC);
			Serial.print(":");
			Serial.print(col, DEC);
			Serial.print(" Released\n");
			bleKeypad.releasekey(keyMap[keyLayer][row][col]);
			update = true;
			return;
			//release the button of the right layer
		}
	}
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
		digitalWrite(colPins[col], !COL_STATE);

		//Flip row pins high and low really fast to dump any charge
		//accumulation due to internal bootstrapping pin hardware.
		for (int row = 0; row < ROW_COUNT; row++){
			pinMode(rowPins[row], OUTPUT);
			digitalWrite(rowPins[row], LOW);
			digitalWrite(rowPins[row], HIGH);
			pinMode(rowPins[row], INPUT_PULLUP);
		}

		//Scan all buttons in the column for changes
		rowScan(col);
		
		//Switch off the current column to a HIGH logic state 
		digitalWrite(colPins[col], COL_STATE);
	}
}

/*
The mapping of joystick values to adc values is sensitive to the range
reported by the adc. Essentially, if the read ADC value is outside the 
min/max range specified in the MAX_ and MIN_ variables, the output will 
wrap around. To maintain sensitivity, we want these bounds to be as 
closely mapped to what is reported as possible. 

Easy fix is to keep the bounds slightly elastic. If the read value goes 
out of bounds, move the bounds. This makes the joystic sensitive to 
drift, but since values don't drift very much in this particular setup, 
it doesn't cause too many problems.
*/
void joyRunCal(int adcX, int adcY){
	if (adcX > MAX_X){
		MAX_X = adcX;
	}
	if (adcX < MIN_X){
		MIN_X = adcX;
	}
	if (adcY > MAX_Y){
		MAX_Y = adcY;
	}
	if (adcY < MIN_Y){
		MIN_Y = adcY;
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
	//Read in the values. We'll be doing some massaging of these values, so we want them static.
	x_read = analogRead(JOY_X_PIN);
	y_read = analogRead(JOY_Y_PIN);
	joyRunCal(x_read, y_read);

	//Compare the current readings to the previous readings. If the readings differ by more
	//than a specified percentage, the new value will be sent over to the paired device.
	float tempX = (float)abs(x_read - prev_x) / (float)prev_x;
	float tempY = (float)abs(y_read - prev_y) / (float)prev_y;

	//If the joystic output has changed sufficiently, map output values and prepare an update
	if (tempX > JOY_DEVIATION || tempY > JOY_DEVIATION){
		//Map X axis ADC reading to joystick output values
		if (!REV_X){
			axes_current[0] = map(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MIN, JOY_MAX);
		} else {
			//Reversed X axis
			axes_current[0] = map(constrain(x_read, MIN_ADC, MAX_ADC), MIN_X, MAX_X, JOY_MAX, JOY_MIN);
		}

		//Map Y axis ADC reading to joystick output values
		if (!REV_Y){
			axes_current[1] = map(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MIN, JOY_MAX);
		} else {
			//Reversed y axis
			axes_current[1] = map(constrain(y_read, MIN_ADC, MAX_ADC), MIN_Y, MAX_Y, JOY_MAX, JOY_MIN);
		}

		//Use the current ADC values for next cycle's comparison
		//Only updating these if the value is updated prevents very slow,
		//small movements of the joystick being ignored as the per-cycle
		//change could cause the reference point to drift without triggering
		//an update.
		prev_x = x_read;
		prev_y = y_read;

		//Queue an update
		bleKeypad.setX(axes_current[0]);
		bleKeypad.setY(axes_current[1]);
		//Construct the packet to be sent as an update
		bleKeypad.sendReport();

		//Indicate that a change worthy of an update has occured
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

void readUntil(File* file, String& str, char* delim){
	
}

String * parseLine(String& str){
	
}

void parseLayout(File* file){
	
}


/*
Set up the hardware and instantiate all necessary objects/variables.
*/
void setup() {
  	//Configure Serial port for debug messages
	Serial.begin(115200);
  	bleKeypad.begin();

//Initialize and read SD Card
	pinMode(SD_CS, OUTPUT);
	digitalWrite(SD_CS, HIGH);
	pinMode(23, INPUT_PULLUP);
	CARD_AVAILABLE = SD.begin(SD_CS);
	if (CARD_AVAILABLE){
		Serial.println("SD Card Found!");
		uint8_t cardType = SD.cardType();
		if(cardType == CARD_NONE){
			CARD_AVAILABLE = false;
			Serial.println("ERROR -- No SD Card present.");
		}else{
			Serial.println("Initializing SD Card");
			File file = SD.open("/data.txt");
			file.seek(0);
			if (!file){
				Serial.println("ERROR -- File not found.");
			}else{
				Serial.println("Reading from file...");
				parseLayout(&file);
				Serial.println("Done reading from file.");
				
				file.close();
			}
		}
	}

//Start by configuring GPIO:
	
	//Set rows to initial state
	for (int row=0; row<ROW_COUNT; row++){
		pinMode(rowPins[row], ROW_MODE);
	}
	//Set columns to initial state
	for (int col=0; col<COL_COUNT; col++){
		pinMode(colPins[col], COL_MODE);
		digitalWrite(colPins[col], COL_STATE);
	}
	//Set button to initial state
	pinMode(buttonPins[0], INPUT_PULLUP);

	//Initially, this was a for loop, but we only have one button
	Bounce debouncer = Bounce();
	debouncer.attach(buttonPins[0], INPUT_PULLUP);
	debouncer.interval(DEBOUNCE);
	btnDebounce[0] = debouncer; //Doesn't need to be an array, legacy carryover
	//Set up Joystick



	//Create and associate debounce objects





	//for (int row = 0; row < ROW_COUNT; row++){
	//	pinMode(rowPins[row], INPUT_PULLUP);
	//}
	
	//Set all columns to INPUT (high-impedance)
	//for (int col = 0; col < COL_COUNT; col++){
	//	//pinMode(colPins[col], INPUT);
	//	pinMode(colPins[col], OUTPUT);
	//	digitalWrite(colPins[col], HIGH);
	//}
	
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