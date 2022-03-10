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

//Rotary encoder
volatile int encoder0Pos = 0;

//Button for selecting tune speed
int buttonSaveState 	= LOW;
int oldButtonSaveState  = LOW;
int speedButtonState    = 0;
int tuneSwitchPrevState = LOW;
int state               = HIGH;
int tuneSwitchReading;

//Tune frequency
unsigned long frequency  = 7000000;
unsigned long tuneAmount = 100;

//Tune speeds
bool  FAST_MODE   = false;
short tuneSpeedID = 0;
const unsigned long tuneSpeeds[4] = {100, 1000, 10000, 100000};

unsigned long timer;
const short diff = 200;
long bcdNum;

void readEncoder();
void tuneFrequency(bool up);
bool debounceButton(int buttonPin, bool oldState);

// Save current tuner state to EEPROM
void saveState() {
	EEPROM.put(4*sizeof(char), frequency);
	EEPROM.put(4*sizeof(char) + sizeof(long), tuneSpeedID);
	Serial.println("State saved!");
}

// Load last tuner state from EEPROM
void loadState() {
	// EEPROM:
	// char[4], long	 , short
	// temp   , frequency, tuneSpeed_ID

	char temp[5] = {0};

	for(short i=0; i<4; i++) 
		temp[i] = EEPROM.read(i);

	if(strcmp(temp, "data") == 0) {
		// Read freq and tuneSpeed from memory
		EEPROM.get(4*sizeof(char), frequency);
		EEPROM.get(4*sizeof(char) + sizeof(long), tuneSpeedID);

		if (tuneSpeedID<0 || tuneSpeedID>3) {
			Serial.println("Incorrect tuneSpeed_ID reading from EEPROM.");
			tuneSpeedID = 0;
			frequency   = 100;
		}

		tuneAmount = tuneSpeeds[tuneSpeedID];

		Serial.println("Tuner state loaded from EEPROM:");
		Serial.print("frequency = ");
		Serial.println(frequency);
		Serial.print("tuneAmount = ");
		Serial.println(tuneAmount);

	} else {
		// Set default values 
		EEPROM.put(0*sizeof(char), 'd');
		EEPROM.put(1*sizeof(char), 'a');
		EEPROM.put(2*sizeof(char), 't');
		EEPROM.put(3*sizeof(char), 'a');

		saveState();

		Serial.println("Default tuner state saved to EEPROM.");
	}

}

void setup()
{

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

void loop()
{

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

		if (state == LOW)
		{
			tuneSpeedID = (tuneSpeedID + 1) % 4;
			tuneAmount = tuneSpeeds[tuneSpeedID];
		}

		tuneSwitchReading = !state;
		timer = millis();
	}

	tuneSwitchPrevState = tuneSwitchReading;
	
}

bool debounceButton(int buttonPin, bool oldState)
{
	bool stateNow = digitalRead(buttonPin);
	if(oldState!=stateNow)
	{
		delay(10);
		stateNow = digitalRead(buttonPin);
	}
	return stateNow;
}

void updateFrequency() {
	bcdNum = bcv::mConvertToBCD(frequency / 100);

	//DEBUGGING ---------------|
	//Serial.println("");
	//Serial.print(frequency);
	//Serial.print(" >> ");
	//Serial.println(bcdNum);
	//-------------------------|

	for (int i = 21; i >= 0; i--)
		digitalWrite(43 - i, (bcdNum >> i) & 1);
}

void tuneFrequency(bool up)
{

	double multiplier = 0.5;
	if (FAST_MODE) 
		multiplier = 1.0;

	frequency += (tuneAmount * multiplier) * (up ? 1 : -1);

	if (tuneAmount == 1000)
		frequency -= (frequency % 1000);
	else if (tuneAmount == 10000)
	{
		frequency -= (frequency % 10000);
		frequency -= (frequency % 1000);
	}
	
	//Handle edge frequencies
	if (frequency < 0)
		/*
			Change to frequency=30000000 if you want 
			the frequency to go from 0 to 30MHz when 
			RF gets to below 0
		*/
		frequency = 0; 
	if (frequency > 30000000)
		frequency = 0;
}

void readEncoder() {
  	if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
    	tuneFrequency(false);
	else 
    	tuneFrequency(true);

	updateFrequency();
}