/***************************************************
Copyright (c) 2017 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
#ifndef _ASYNCSTEPPERLIB_h
#define _ASYNCSTEPPERLIB_h

#include <Arduino.h>

typedef void(*StepCallback)();

class AsyncStepper
{

public:
	typedef enum
	{
		CW,
		CCW,
	}  StepDirection;

	AsyncStepper(uint16_t motorSteps, int pinCW, int pinCCW);
	AsyncStepper(uint16_t motorSteps, StepCallback actionCW, StepCallback actionCCW);

	float speedRpm;
	unsigned long microsByStep;

	void Rotate(float angleDelta, StepDirection direction);
	void Rotate(float angleDelta, StepDirection direction, StepCallback callback);
	void RotateToAngle(float angle, StepDirection direction, StepCallback callback);
	void RotateToAngle(float angle, StepDirection direction);
	void RotateContinuos(StepDirection direction);

	void SetSpeedRpm(float rpm);
	void SetSpeedDegreesBySecond(float degreesBySecond);
	void SetSpeedRadiansBySecong(float radiansBySecond);
	void SetMetersBySecond(float metersBySecond, float radius);

	void Stop();
	float GetCurrentAngle() const;

	bool Update();

	StepCallback ActionCW;
	StepCallback ActionCCW;

	int PinCW;
	int PinCCW;


private:
	uint16_t _motorSteps;

	StepDirection _direction;
	bool _stopped;

	unsigned int _currentStep;
	unsigned int _goalStep;

	unsigned long _startime;
	StepCallback _callback;

	void Step();
	void StepCW();
	void StepCCW();
	static void DigitalPulse(int pin);
};


#endif

