#pragma once
#ifndef _IDriverInstruction_h
#define _IDriverInstruction_h

#include "Stepperdriver.h"
#include "LogarithmicStepDistribution.h"
#include "Curves.h"

enum DriverInstructionResult
{
	Success,
	Done,
};

class IDriverInstruction
{
protected:
	
	int StepFactor(MicroStep ms)
	{
		switch (ms)
		{
		case MSStep:
			return 16;
		case MSHalf:
			return 8;
		case MSQuarter:
			return 4;
		case MSEighth:
			return 2;
		case MSSixteenth:
			return 1;
		default:
			break;
		}
	}
public:
	virtual DriverInstructionResult Execute(StepperDriver* driver) = 0;
};


#endif // !_IDriverInstruction_h