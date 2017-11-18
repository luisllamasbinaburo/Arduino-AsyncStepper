/***************************************************
Copyright (c) 2017 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

#include "AsyncStepperLib.h"

AsyncStepper::AsyncStepper(uint16_t motorSteps, int pinCW, int pinCCW)
{
	_motorSteps = motorSteps;
	PinCW = pinCW;
	PinCCW = pinCCW;
	pinMode(OUTPUT, PinCW);
	pinMode(OUTPUT, PinCCW);
	ActionCW = nullptr;
	ActionCCW = nullptr;
	_callback = nullptr;
}

AsyncStepper::AsyncStepper(uint16_t motorSteps, StepCallback actionCW, StepCallback actionCCW)
{
	_motorSteps = motorSteps;
	ActionCW = actionCW;
	ActionCCW = actionCCW;
	_callback = nullptr;
}

void AsyncStepper::Rotate(float angleDelta, StepDirection direction)
{
	Rotate(angleDelta, direction, nullptr);
}

void AsyncStepper::Rotate(float angleDelta, StepDirection direction, StepCallback callback)
{
	_stopped = false;
	_direction = direction;
	_goalStep = angleDelta / 360.f * _motorSteps;
	_callback = callback;
	_startime = micros();
}

void AsyncStepper::RotateToAngle(float angle, StepDirection direction)
{
	RotateToAngle(angle, direction, nullptr);
}

void AsyncStepper::RotateToAngle(float angle, StepDirection direction, StepCallback callback)
{
	float currentAngle = GetCurrentAngle();
	float angleDelta = direction == CW ? currentAngle - angle : angle - currentAngle;
	if (angleDelta < 0) angleDelta = -angleDelta;
	_callback = callback;
	Rotate(angleDelta, direction);
}

void AsyncStepper::RotateContinuos(StepDirection direction)
{
	_stopped = false;
	_direction = direction;
	_goalStep = 0;
	_startime = micros();
}

void AsyncStepper::SetSpeedRpm(float rpm)
{
	SetSpeedDegreesBySecond(rpm * 360 / 60);
}

void AsyncStepper::SetSpeedDegreesBySecond(float degreesBySecond)
{
	float degreeByStep = 360.0f / _motorSteps;
	microsByStep = degreeByStep / degreesBySecond * 1000000;
}

void AsyncStepper::SetSpeedRadiansBySecong(float radiansBySecond)
{
	SetSpeedDegreesBySecond(360.0f / 2 / PI * radiansBySecond);
}

void AsyncStepper::SetMetersBySecond(float metersBySecond, float radius)
{
	SetSpeedRadiansBySecong(metersBySecond / radius);
}

void AsyncStepper::Stop()
{
	_stopped = true;
}

float AsyncStepper::GetCurrentAngle() const
{
	return 360.0f * _currentStep / _motorSteps;
}

bool AsyncStepper::Update()
{
	if (_stopped) return false;

	if (static_cast<unsigned long>(micros() - _startime) >= microsByStep)
	{
		_startime += microsByStep;
		Step();

		if(_goalStep > 0)
		{
			_goalStep--;
			if (_goalStep == 0)
			{
				_stopped = true;
				if (_callback != nullptr) _callback();
			}
		}
		return true;
	}
	return false;
}

void AsyncStepper::Step()
{
	if (_direction == 0) StepCW();
	else StepCCW();
}

void AsyncStepper::StepCW()
{
	if (ActionCW != nullptr) ActionCW();
	else DigitalPulse(PinCW);
	_currentStep++;
	if (_currentStep > _motorSteps) _currentStep = 0;
}

void AsyncStepper::StepCCW()
{
	if (ActionCCW != nullptr) ActionCCW();
	else DigitalPulse(PinCCW);
	//_currentStep--;
	//if (_currentStep > _motorSteps) _currentStep = _motorSteps;
	_currentStep = _currentStep == 0 ? _motorSteps : _currentStep - 1;
}

void AsyncStepper::DigitalPulse(int pin)
{
	digitalWrite(HIGH, pin);
	digitalWrite(LOW, pin);
}
