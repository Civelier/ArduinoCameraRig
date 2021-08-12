#ifndef _LogarithmicStepDistribution_h
#define _LogarithmicStepDistribution_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define MS1_BIT 2
#define MS2_BIT 1
#define MS3_BIT 0
enum MicroStep
{
	MSStep = 0b000,
	MSHalf = 0b100,
	MSQuarter = 0b010,
	MSEighth = 0b110,
	MSSixteenth = 0b111,
};



struct LogarithmicStepValues
{
	double Middle2_1;
	double Middle4_2;
	double Middle8_4;
	double Middle16_8;
	void Display()
	{
		Serial.print("Middle2_1: ");
		Serial.println(Middle2_1);
		Serial.print("Middle4_2: ");
		Serial.println(Middle4_2);
		Serial.print("Middle8_4: ");
		Serial.println(Middle8_4);
		Serial.print("Middle16_8: ");
		Serial.println(Middle16_8);
	}
};

struct LogarithmicStepDistribution
{
	double MaxStepPerSec;
	double StepScaling;
	double Factor;
	double Base;
	LogarithmicStepValues Values;
	void Update()
	{
		MaxStepPerSec = Serial.parseFloat();
		StepScaling = Serial.parseFloat();
		Factor = Serial.parseFloat();
		Base = Serial.parseFloat();
		ComputeValues();
	}
	void Display()
	{
		Serial.print("Steps/s = ");
		Serial.println(MaxStepPerSec);
		Serial.print("Step scaling = ");
		Serial.println(StepScaling);
		Serial.print("Factor = ");
		Serial.println(Factor);
		Serial.print("Base = ");
		Serial.println(Base);
		Values.Display();
	}

	void ComputeValues()
	{
		Values =
		{
			StepScaling - pow(Base, 2.0) / Factor * StepScaling,
			StepScaling - pow(Base, 4.0) / Factor * StepScaling,
			StepScaling - pow(Base, 8.0) / Factor * StepScaling,
			StepScaling - pow(Base, 16.0) / Factor * StepScaling,
		};
	}

	MicroStep MicroStepForSpeed(float sps)
	{
		/*if (sps >= Values.Middle2_1) return MicroStep::MSStep;
		if (sps >= Values.Middle4_2) return MicroStep::MSHalf;
		if (sps >= Values.Middle8_4) return MicroStep::MSQuarter;
		if (sps >= Values.Middle16_8) return MicroStep::MSEighth;*/
		return MicroStep::MSSixteenth;
	}
};

#endif // !_LogarithmicStepDistribution_h