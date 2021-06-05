#ifndef _StepperDriver_h
#define _StepperDriver_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "MultiStepperUtillities.h"
#include "LogarithmicStepDistribution.h"

class IDriverInstruction;


struct StepperDriver
{
	uint32_t NumberOfSteps;
	pin_t Step;
	pin_t Dir;
	pin_t MS1 = NOT_A_PIN;
	pin_t MS2 = NOT_A_PIN;
	pin_t MS3 = NOT_A_PIN;
	LogarithmicStepDistribution LogDistribution;
	StepType StepCompatibility;
	IDriverInstruction* Instruction;
	int8_t thisStep;
	StepperDriver() { }
	StepperDriver(uint32_t numberOfSteps, pin_t step, pin_t dir)
	{
		NumberOfSteps = numberOfSteps;
		Step = step;
		Dir = dir;
		StepCompatibility = StepType::Step;
	}
	StepperDriver(uint32_t numberOfSteps, pin_t step, pin_t dir, pin_t ms1, pin_t ms2)
	{
		NumberOfSteps = numberOfSteps;
		Step = step;
		Dir = dir;
		MS1 = ms1;
		MS2 = ms2;
		StepCompatibility = StepType::Eigth;
	}
	StepperDriver(uint32_t numberOfSteps, pin_t step, pin_t dir, pin_t ms1, pin_t ms2, pin_t ms3)
	{
		NumberOfSteps = numberOfSteps;
		Step = step;
		Dir = dir;
		MS1 = ms1;
		MS2 = ms2;
		MS3 = ms3;
		StepCompatibility = StepType::Sixteenth;
		LogDistribution.MaxStepPerSec = 800;
		LogDistribution.StepScaling = 1050;
		LogDistribution.Factor = 4.5f;
		LogDistribution.Base = 1.07f;

		LogDistribution.ComputeValues();
	}

	void SetInstruction(IDriverInstruction* instruction)
	{
		if (Instruction != nullptr) delete(Instruction);
		Instruction = instruction;
	}
	
	void SetMicroStepPins(MicroStep ms)
	{
		digitalWrite(MS1, bitRead(ms, MS1_BIT));
		digitalWrite(MS2, bitRead(ms, MS2_BIT));
		digitalWrite(MS3, bitRead(ms, MS3_BIT));
	}
};

#endif // !_StepperDriver_h
