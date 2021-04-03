/***************************************************
Copyright (c) 2021 Luis Llamas
(www.luisllamas.es)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses
****************************************************/

#include <AsyncStepper.hpp>

const int motorPin1 = 8;  
const int motorPin2 = 9;  
const int motorPin3 = 10; 
const int motorPin4 = 11; 

int stepCounter = 0; 
const int numSteps = 8;
const int stepsLookup[8] = { B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001 };

// Code to drive the NoDrive stepper
void setOutput(int step)
{
	digitalWrite(motorPin1, bitRead(stepsLookup[step], 0));
	digitalWrite(motorPin2, bitRead(stepsLookup[step], 1));
	digitalWrite(motorPin3, bitRead(stepsLookup[step], 2));
	digitalWrite(motorPin4, bitRead(stepsLookup[step], 3));
}

void clockwise()
{
	stepCounter++;
	if (stepCounter >= numSteps) stepCounter = 0;
	setOutput(stepCounter);
}

void anticlockwise()
{
	stepCounter--;
	if (stepCounter < 0) stepCounter = numSteps - 1;
	setOutput(stepCounter);
}

const int stepper_steps = 4076;

AsyncStepper stepper1(stepper_steps,
	[]() { clockwise(); },
	[]() { anticlockwise(); }
);

void setup()
{
	Serial.begin(115200);

	pinMode(motorPin1, OUTPUT);
	pinMode(motorPin2, OUTPUT);
	pinMode(motorPin3, OUTPUT);
	pinMode(motorPin4, OUTPUT);

	stepper1.SetSpeedRpm(10);
	stepper1.RotateContinuous(AsyncStepper::CCW);
}

void loop()
{
	stepper1.Update();
}
