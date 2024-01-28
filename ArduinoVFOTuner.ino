/*

    Arduino VFO Tuner for Harris RF-550

    Author: d3rail0
    
	Testers:
		Keith Densmore
		Bryan Kim

    Completion date: Sunday the 5th of November, 2017 

*/

#include "src/bconverter.h"
#include <EEPROM.h>

const double EPSILON = 0.0001;

const int encoder0PinA = 3;
const int encoder0PinB = 2;

const int SAVE_STATE_BUTTON = 11;
const int SPEED_BUTON       = 12;
const int TUNE_SWITCH       = 17;

// Radio Configuration
const long MAX_FREQUENCY          = 30000000;  // Maximum frequency limit
const long MIN_FREQUENCY          = 0;         // Minimum frequency limit
const double STANDARD_MULTIPLIER  = 1.0;
const double FAST_MULTIPLIER      = 0.5;

const char EEPROM_SIGNATURE[] = "dat1";  // Constant for EEPROM signature

struct TunerState {
    char signature[5];  // A signature to check if valid data is stored
    long frequency;
    short tuneSpeedID;
};

//Rotary encoder
volatile int encoder0Pos = 0;

//Button for selecting tune speed
int buttonSaveState 	= LOW;
int oldButtonSaveState  = LOW;
int speedButtonState    = 0;
int tuneSwitchPrevState = LOW;
int state               = HIGH;
int tuneSwitchReading;

//Tune speeds
TunerState tunerState;
bool FAST_MODE = false;
unsigned long tuneAmount = 100;
const unsigned long tuneSpeeds[4] = {100, 1000, 10000, 100000};

long bcdNum;
unsigned long timer;
const short diff = 200;

void readEncoder();
void tuneFrequency(bool up);
bool debounceButton(int buttonPin, bool oldState);

// Initialize the tuner state
void initState() {
    strcpy(tunerState.signature, EEPROM_SIGNATURE);
    tunerState.frequency = 7000000;
    tunerState.tuneSpeedID = 0;
}

// Save current tuner state to EEPROM
void saveState() {
	EEPROM.put(0, tunerState);
	Serial.println("State saved!");
}

// Load last tuner state from EEPROM
void loadState() {

	EEPROM.get(0, tunerState);

	if (strcmp(tunerState.signature, EEPROM_SIGNATURE) == 0) {

		// Check if the tuneSpeedID is within a valid range
        if (tunerState.tuneSpeedID < 0 || tunerState.tuneSpeedID > 3) {
            Serial.println("Incorrect tuneSpeedID reading from EEPROM.");
            initState();  // Reset to initial state
        }

		tuneAmount = tuneSpeeds[tunerState.tuneSpeedID];

		Serial.println("Tuner state loaded from EEPROM:");
		Serial.print("frequency = ");
		Serial.println(tunerState.frequency);
		Serial.print("tuneAmount = ");
		Serial.println(tuneAmount);

	} else {
		// Set default values and save them
        initState();
        saveState();
        Serial.println("Default tuner state saved to EEPROM.");
	}

}

void setup() {
	// Configure input pins
	pinMode(encoder0PinA, INPUT);
	pinMode(encoder0PinB, INPUT);

	pinMode(SAVE_STATE_BUTTON, INPUT);
	pinMode(SPEED_BUTON, INPUT);
	pinMode(TUNE_SWITCH, INPUT);

	// turn on pull-up resistor
	digitalWrite(encoder0PinA, HIGH);
	digitalWrite(encoder0PinB, HIGH);

	// BCD requires 22 lines for tuning
	for (int i = 0; i < 22; i++)
		pinMode(22 + i, OUTPUT);
	
	// encoder pin on interrupt 0 - pin 2
	attachInterrupt(0, readEncoder, CHANGE);

	Serial.begin(115200);
	Serial.println("start");

	loadState();
}

void loop() {

	// Update state if button is active
	buttonSaveState = debounceButton(SAVE_STATE_BUTTON, oldButtonSaveState);
	if (buttonSaveState == HIGH && oldButtonSaveState == LOW) {
		saveState();
	}
	oldButtonSaveState = buttonSaveState;

	//interrupts will take care of themselves
	speedButtonState = digitalRead(SPEED_BUTON);
	FAST_MODE = !(speedButtonState == HIGH);

	tuneSwitchReading = digitalRead(TUNE_SWITCH);

	if (tuneSwitchReading == LOW && tuneSwitchPrevState == HIGH && millis() - timer > diff)
	{

		state = !state;

		if (state == LOW) {
			tunerState.tuneSpeedID = (tunerState.tuneSpeedID + 1) % 4;
			tuneAmount = tuneSpeeds[tunerState.tuneSpeedID];
		}

		tuneSwitchReading = !state;
		timer = millis();
	}

	tuneSwitchPrevState = tuneSwitchReading;
	
}

bool debounceButton(int buttonPin, bool oldState) {
	bool stateNow = digitalRead(buttonPin);
	if(oldState!=stateNow) {
		delay(10);
		stateNow = digitalRead(buttonPin);
	}
	return stateNow;
}

void updateFrequency() {
	bcdNum = bcv::mConvertToBCD(tunerState.frequency / 100);

	//DEBUGGING ---------------|
	//Serial.println("");
	//Serial.print(frequency);
	//Serial.print(" >> ");
	//Serial.println(bcdNum);
	//-------------------------|

	for (int i = 21; i >= 0; i--)
		digitalWrite(43 - i, (bcdNum >> i) & 1);
}

void tuneFrequency(bool up) {
	double multiplier = (FAST_MODE) ? FAST_MULTIPLIER : STANDARD_MULTIPLIER;
	long frequencyChange = static_cast<long>(tuneAmount * multiplier) * (up ? 1 : -1);
    tunerState.frequency += frequencyChange;

    if (tuneAmount == 1000) {
        tunerState.frequency -= (tunerState.frequency % 1000);
    } else if (tuneAmount == 10000) {
        tunerState.frequency -= (tunerState.frequency % 10000) - (tunerState.frequency % 1000);
    }

    // Handle edge frequencies
    if (tunerState.frequency < MIN_FREQUENCY) {
		/*
			Change to frequency=MAX_FREQUENCY if you want 
			the frequency to go from MIN_FREQUENCY to MAX_FREQUENCY when 
			RF goes below MIN_FREQUENCY
		*/
        tunerState.frequency = MIN_FREQUENCY;
    } else if (tunerState.frequency > MAX_FREQUENCY) {
        tunerState.frequency = MIN_FREQUENCY;
    }
}

void readEncoder() {
  	if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
    	tuneFrequency(false);
	}
	else {
    	tuneFrequency(true);
	}

	updateFrequency();
}