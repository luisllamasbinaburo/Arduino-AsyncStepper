/***************************************************
Copyright (c) 2017 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

#include "AsyncStepperLib.h"
#include <Stepper.h>

const int motorPin1 = 8;  
const int motorPin2 = 9;  
const int motorPin3 = 10; 
const int motorPin4 = 11; 
const int numSteps = 8;
const int stepsLookup[8] = { B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001 };
int stepCounter = 0; 

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

void setOutput(int step)
{
	digitalWrite(motorPin1, bitRead(stepsLookup[step], 0));
	digitalWrite(motorPin2, bitRead(stepsLookup[step], 1));
	digitalWrite(motorPin3, bitRead(stepsLookup[step], 2));
	digitalWrite(motorPin4, bitRead(stepsLookup[step], 3));
}

const int stepsPerRevolution = 4076;
AsyncStepper stepper1(stepsPerRevolution,
	[]() {clockwise(); },
	[]() {anticlockwise(); }
);

void rotateCW()
{
	stepper1.Rotate(90, AsyncStepper::CW, rotateCCW);
}

void rotateCCW()
{
	stepper1.Rotate(90, AsyncStepper::CCW, rotateCW);
}

void setup()
{
	Serial.begin(9600);

	pinMode(motorPin1, OUTPUT);
	pinMode(motorPin2, OUTPUT);
	pinMode(motorPin3, OUTPUT);
	pinMode(motorPin4, OUTPUT);

	stepper1.SetSpeedRpm(10);
	stepper1.RotateContinuos(AsyncStepper::CCW);
}

void loop()
{
	stepper1.Update();
}
