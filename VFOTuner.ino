/*

    Arduino VFO Tuner for Harris RF-550

    Author: d3rail0
    Tester: Keith Densmore

    Completion date: Sunday the 5th of November, 2017 

*/

#include "src/Helper.h"

#define encoder0PinA 3
#define encoder0PinB 2
#define SPEED_BUTON 12
#define TUNE_SWITCH 17
#define EPSILON 0.0001

//Rotary encoder
volatile int encoder0Pos = 0;

//Button for selecting tune speed
int buttonState         = 0;
int tuneSwitchPrevState = LOW;
int state               = HIGH;
int tuneSwitchReading;

//Tune frequency
long frequency  = 100;
long tuneAmount = 100;
bool FAST_MODE  = false;

//Tune speeds
short tuneSpeed_ID = 0;
const unsigned long tune_speeds[4] = {100, 1000, 10000, 100000};

long timer;
long diff = 200;

// Forward declare functions
void readEncoder();
void tuneFrequency(bool up);

Helper 	helper;
long 	bcdNum;

void setup()
{

	// Configure input pins
	pinMode(encoder0PinA, INPUT);
	pinMode(encoder0PinB, INPUT);
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
}

void loop()
{

	//interrupts will take care of themselves
	buttonState = digitalRead(SPEED_BUTON);
	FAST_MODE = !(buttonState == HIGH);

	tuneSwitchReading = digitalRead(TUNE_SWITCH);

	if (tuneSwitchReading == LOW && tuneSwitchPrevState == HIGH && millis() - timer > diff)
	{

		state = !state;

		if (state == LOW)
		{
			tuneSpeed_ID = (tuneSpeed_ID + 1) % 4;
			tuneAmount = tune_speeds[tuneSpeed_ID];
		}

		tuneSwitchReading = !state;
		timer = millis();
	}

	tuneSwitchPrevState = tuneSwitchReading;

	bcdNum = helper.ConvertToBCD(frequency / 100);

	//DEBUGGING ---------------|
	//Serial.println("");
	//Serial.print(frequency);
	//Serial.print(" >> ");
	//Serial.println(bcdNum);
	//-------------------------|

	for (int i = 21; i >= 0; i--)
		digitalWrite(22 + i, (bcdNum >> i) & 1);
}

void tuneFrequency(bool up)
{

	double multiplier = (FAST_MODE == false) / 2;
	if (abs(multiplier) < EPSILON)
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
}