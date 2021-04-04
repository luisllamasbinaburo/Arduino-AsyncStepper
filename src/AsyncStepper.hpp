/***************************************************
Copyright (c) 2021 Luis Llamas
(www.luisllamas.es)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses
****************************************************/

#ifndef _ASYNCSTEPPER_h
#define _ASYNCSTEPPER_h

#include <Arduino.h>

typedef void(*StepperCallback)();

class AsyncStepper
{

public:
	enum StepperMode
	{
		Constant,
		Linear
	};

	enum StepperDirection
	{
		CW,
		CCW,
	};

	enum StepperState
	{
		Stopped,
		Accelerating,
		Running,
		Breaking
	};


	AsyncStepper(uint16_t motorSteps, StepperCallback actionCW, StepperCallback actionCCW)
	{
		MotorSteps = motorSteps;

		ActionCW = actionCW;
		ActionCCW = actionCCW;
		OnFinish = nullptr;
	}

	AsyncStepper(uint16_t motorSteps, int pinDir, int pinStep)
	{
		MotorSteps = motorSteps;

		PinDir = pinDir;
		PinStep = pinStep;
		OnFinish = nullptr;

		pinMode(PinDir, OUTPUT);
		pinMode(PinStep, OUTPUT);
	}



	void Rotate(long steps, StepperDirection direction, StepperCallback onFinish = nullptr)
	{
		Direction = direction;
		OnFinish = onFinish;

		ContinuousMove = false;
		TravelSteps = labs(steps);

		InitRotation(MaxSpeed);
	}

	void RotateAngle(float angle, StepperDirection direction, StepperCallback onFinish = nullptr)
	{
		Rotate(angle / 360.0f * 200, direction, onFinish);
	}

	void RotateToAngle(float angle, StepperDirection direction, StepperCallback onFinish = nullptr)
	{
		auto currentAngle = GetCurrentAngle();
		float angleDelta = direction == CW ? currentAngle - angle : angle - currentAngle;
		if (angleDelta < 0) angleDelta = -angleDelta;
		RotateAngle(angleDelta, direction, OnFinish);
	}

	void RotateInTime(long steps, float time, StepperDirection direction, StepperCallback onFinish = nullptr)
	{
		auto requiredTime = GetTimeForMove(steps);

		auto speed = requiredTime > time ? MaxSpeed : GetSpeedForMove(steps, time);

		Direction = direction;
		OnFinish = onFinish;

		ContinuousMove = false;
		TravelSteps = labs(steps);

		InitRotation(speed);
	}

	void RotateAngleInTime(float angle, float time, StepperDirection direction, StepperCallback onFinish = nullptr)
	{
		RotateInTime(angle / 360.0f * 200, time, direction, onFinish);
	}

	void RotateToAngleInTime(float angle, float time, StepperDirection direction, StepperCallback onFinish = nullptr)
	{
		auto currentAngle = GetCurrentAngle();
		float angleDelta = direction == CW ? currentAngle - angle : angle - currentAngle;
		if (angleDelta < 0) angleDelta = -angleDelta;
		RotateAngleInTime(angleDelta, time, direction, OnFinish);
	}


	void RotateContinuous(StepperDirection direction)
	{
		Direction = direction;
		OnFinish = nullptr;

		ContinuousMove = true;
		TravelSteps = 0;

		InitRotation(MaxSpeed);
	}

	void Stop()
	{
		State = StepperState::Stopped;
	}

	void Break()
	{
		State = StepperState::Breaking;
	}


	uint16_t Update()
	{
		if (State == StepperState::Stopped) return false;

		uint16_t stepsDone = 0;
		while (static_cast<unsigned long>(micros() - LastStepTime) >= Interval)
		{
			LastStepTime += Interval;
			Step();

			if (stepsDone > 0) delayMicroseconds(PulseOffWidth);
			stepsDone++;
		}
		return stepsDone;
	}

	void SetSpeed(long speed)
	{
		MaxSpeed = speed;
	}

	void SetSpeedRpm(float rpm)
	{
		SetSpeed(rpm / 60.0f * MotorSteps);
	}

	void SetSpeedDegreesBySecond(float degreesBySecond)
	{
		SetSpeed(degreesBySecond / 360.0f * MotorSteps);
	}

	void SetAcceleration(long acceleration)
	{
		Acceleration = acceleration;
		Deceleration = acceleration;
	}

	void SetAcceleration(long acceleration, long deceleration)
	{
		Acceleration = acceleration;
		Deceleration = deceleration;
	}



	long GetRemainSteps() const
	{
		return TravelSteps - TravelCurrentStep;
	}

	unsigned long GetTimeTraveling() const
	{
		return micros() - TravelStartTime;
	}

	float GetCurrentAngle() const
	{
		return 360.0f * (AbsoluteStep % MotorSteps) / MotorSteps;
	}

	long GetCurrentInterval() const
	{
		return Interval;
	}

	long GetTravelCurrentStep() const
	{
		return TravelCurrentStep;
	}

	long GetTravelSteps() const
	{
		return TravelSteps;
	}

	long GetAbsoluteStep() const
	{
		return AbsoluteStep;
	}

	long GetMaxSpeed() const
	{
		return MaxSpeed;
	}

	long GetCurrentSpeed() const
	{
		if (State == StepperState::Stopped) return 0;
		return 1e+6 / Interval;
	}

	float GetCurrentSpeedRpm() const
	{
		return GetCurrentSpeed() / MotorSteps * 60.0f;
	}

	float GetCurrentSpeedDegreesBySecond() const
	{
		return GetCurrentSpeed() / MotorSteps * 360.0f;
	}

	float GetSpeedForMove(long steps, float time)
	{
		float speed = 0;

		if (Mode == AsyncStepper::Linear)
		{
			float a_2 = 1.0 / Acceleration + 1.0 / Deceleration;
			float discriminant = time * time - 2 * a_2 * steps;
			if (discriminant >= 0)
			{
				speed = (time - (float)sqrt(discriminant)) / a_2;
			};
		}
		else
		{
			speed = steps / time;
		}

		return speed;
	}

	float GetTimeForMove(long steps)
	{
		return GetTimeForMove(steps, MaxSpeed);
	}

	float GetTimeForMove(long steps, unsigned long speed)
	{
		float time;
		if (steps == 0)
		{
			return 0;
		}
		else
		{
			if (Mode == AsyncStepper::Linear)
			{
				auto accSteps = (float)GetRampSteps(MaxSpeed, Acceleration);
				auto decSteps = (float)GetRampSteps(MaxSpeed, Deceleration);

				float ramps = accSteps + decSteps;
				float runSteps = 0;

				if (steps < ramps) {
					accSteps = steps * Deceleration / (Acceleration + Deceleration);
					decSteps = steps - accSteps;
				}
				else
				{
					runSteps = steps - ramps;
				}
				time = (runSteps / speed) + sqrt(2.0 * accSteps / Acceleration) + sqrt(2.0 * decSteps / Deceleration);
			}
			else
			{
				time = steps / speed;
			}
		}
		return time;
	}

	int PinDir;
	int PinStep;

	uint16_t MotorSteps;

	StepperCallback ActionCW = nullptr;
	StepperCallback ActionCCW = nullptr;

	StepperCallback OnFinish = nullptr;

	StepperMode Mode = StepperMode::Linear;
	StepperState State = StepperState::Stopped;
	StepperDirection Direction = StepperDirection::CW;

private:
	unsigned long Speed = 200;
	unsigned long Acceleration = 100;
	unsigned long Deceleration = 100;
	unsigned long MaxSpeed = 200;

	bool ContinuousMove;

	long AbsoluteStep = 0;

	unsigned long TravelSteps;
	unsigned long AccSteps;
	unsigned long DecSteps;
	unsigned long TravelCurrentStep;

	long Interval = 0;
	long IntervalRest = 0;
	long RunInterval = 0;

	unsigned long TravelStartTime;
	unsigned long LastStepTime;

	unsigned long PulseOnWidth = 10;
	unsigned long PulseOffWidth = 250;

	void InitRotation(unsigned long speed)
	{
		State = Mode == StepperMode::Constant ? StepperState::Running : StepperState::Accelerating;

		AccSteps = GetRampSteps(speed, Acceleration);
		Interval = 1e+6f * 0.956f * sqrt(1.0f / Acceleration);
		RunInterval = 1e+6f / speed;
		IntervalRest = 0;

		TravelCurrentStep = 0;
		TravelStartTime = micros();
		LastStepTime = TravelStartTime;
	}

	void Step()
	{
		if (Direction == StepperDirection::CW) StepCW();
		else StepCCW();

		TravelCurrentStep++;

		UpdateState();
		UpdateInterval();
	}

	void UpdateState()
	{
		if (State == StepperState::Stopped) return;

		const auto remaining = GetRemainSteps();
		if (ContinuousMove == false && remaining <= 0)
		{
			State = StepperState::Stopped;
			if (OnFinish != nullptr) OnFinish();
		}
		else
		{
			if (Mode == StepperMode::Linear)
			{
				if (State == StepperState::Breaking) return;

				DecSteps = GetRampSteps(GetCurrentSpeed(), Deceleration);
				if (ContinuousMove == false && remaining <= DecSteps)
				{
					State = StepperState::Breaking;
				}
				else if (TravelCurrentStep <= AccSteps)
				{
					State = StepperState::Accelerating;
				}
				else
				{
					State = StepperState::Running;
				}
			}
		}
	}

	void UpdateInterval()
	{
		if (State == StepperState::Accelerating && TravelCurrentStep > 0)
		{
			Interval = Interval - (2 * Interval + IntervalRest) / (4 * TravelCurrentStep + 1);
			IntervalRest = (TravelCurrentStep < AccSteps) ? (2 * Interval + IntervalRest) % (4 * TravelCurrentStep + 1) : 0;
		}

		else if (State == StepperState::Breaking && TravelCurrentStep > 0)
		{
			const long remain = GetRemainSteps();
			Interval = Interval - (2 * Interval + IntervalRest) / (-4 * remain + 1);
			IntervalRest = (2 * Interval + IntervalRest) % (-4 * remain + 1);
		}

		else if (State == StepperState::Running)
		{
			Interval = RunInterval;
		}

		if (Interval < RunInterval)
		{
			Interval = RunInterval;
		}
	}

	static unsigned long GetRampSteps(unsigned long speed, unsigned long acceleration)
	{
		return (speed * speed) / (2.0f * acceleration);
	}

	void StepCW()
	{
		if (ActionCW != nullptr) ActionCW();
		else
		{
			digitalWrite(PinDir, HIGH);
			DigitalPulse(PinStep);
		}

		AbsoluteStep++;
	}

	void StepCCW()
	{
		if (ActionCCW != nullptr) ActionCCW();
		else
		{
			digitalWrite(PinDir, LOW);
			DigitalPulse(PinStep);
		}

		AbsoluteStep--;
	}

	void DigitalPulse(int pin) const
	{
		digitalWrite(pin, HIGH);
		delayMicroseconds(PulseOnWidth);
		digitalWrite(pin, LOW);
	}
};
#endif

