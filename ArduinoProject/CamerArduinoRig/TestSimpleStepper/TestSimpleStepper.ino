/*
 Name:		TestSimpleStepper.ino
 Created:	6/4/2021 11:38:18 AM
 Author:	civel
*/

#include <AccelStepper.h>

#define DIR_PIN 2
#define STEP_PIN 3
#define MS1_PIN 4
#define MS2_PIN 5
#define MS3_PIN 6

//AccelStepper Motor1(AccelStepper::MotorInterfaceType::DRIVER, 2, 3, 4, 5);



#define MS1_BIT 2
#define MS2_BIT 1
#define MS3_BIT 0
enum MicroStep
{
	MSStep		= 0b000,
	MSHalf		= 0b100,
	MSQuarter	= 0b010,
	MSEighth	= 0b110,
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
		if (sps >= Values.Middle2_1) return MicroStep::MSStep;
		if (sps >= Values.Middle4_2) return MicroStep::MSHalf;
		if (sps >= Values.Middle8_4) return MicroStep::MSQuarter;
		if (sps >= Values.Middle16_8) return MicroStep::MSEighth;
		return MicroStep::MSSixteenth;
	}
};

typedef float(*curve_t)(float);

float LinearCurve(float v)
{
	return max(0, min(1, v));
}

float QuadraticInOutCurve(float v)
{
	v = max(0, min(1, v));
	if (v < 0.5f)
		return 2.0f * (v * v);
	return -2.0f * powf(v - 1.0f, 2.0f) + 1.0f;
}

void SetMicroStepPins(MicroStep ms)
{
	digitalWrite(MS1_PIN, bitRead(ms, MS1_BIT));
	digitalWrite(MS2_PIN, bitRead(ms, MS2_BIT));
	digitalWrite(MS3_PIN, bitRead(ms, MS3_BIT));
}

void Step(MicroStep ms, int32_t steps, float stepsPerSecond)
{
	bool dir = steps < 0;
	int32_t stepsLeft = abs(steps);
	digitalWrite(DIR_PIN, dir);
	uint32_t pulseMicro = (uint32_t)(1000000.0f / stepsPerSecond);
	SetMicroStepPins(ms);
	Serial.print("Pulse delay: ");
	Serial.println(pulseMicro);
	uint32_t now;
	uint32_t nextPulse = micros();
	uint32_t start = micros();
	while (stepsLeft)
	{
		now = micros();
		if (nextPulse <= now)
		{
			digitalWrite(STEP_PIN, HIGH);
			delayMicroseconds(1);
			digitalWrite(STEP_PIN, LOW);
			delayMicroseconds(1);
			nextPulse += pulseMicro;
			stepsLeft--;
		}
	}
	uint32_t end = micros();
	Serial.print("Average micros per step: ");
	Serial.println((end - start) / abs(steps));
}

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

struct SpeedBuffer
{
	size_t Count;
	size_t Index;
	size_t Size;
	uint32_t* Deltas;
	SpeedBuffer(size_t size)
	{
		Size = size;
		Deltas = (uint32_t*)malloc(size * sizeof(uint32_t));
	}

	void Write(uint32_t delta)
	{
		Deltas[Index] = delta;
		Index = (Index + 1) % Size;
		if (Count < Size) Count++;
	}

	uint32_t operator[](size_t index)
	{
		size_t i = (Index + index) % Count;
		return Deltas[i];
	}

	float AverageSpeed()
	{
		if (Count < 2) return Count == 0 ? 0 : operator[](0);
		return (float)(operator[](Count - 1) - operator[](0)) / (float)Count;
	}

	~SpeedBuffer()
	{
		free(Deltas);
	}
};

LogarithmicStepDistribution LogDistribution;

void StepWithAccellerationCurve(curve_t curve, int32_t steps, uint32_t time)
{
	bool dir = steps < 0;
	int32_t stepsLeft = abs(steps);
	digitalWrite(DIR_PIN, dir);
	uint32_t now;
	uint32_t nextPulse = micros();
	uint32_t start = nextPulse;
	uint32_t lastPulse = nextPulse;
	int hit = 0;
	MicroStep lastStep;
	SpeedBuffer buff(20);
	while (stepsLeft > 0)
	{
		now = micros();
		
		int32_t step = abs(steps) * curve((float)(now - start) / (float)(time * 1000.0f));
		if (step > (abs(steps) - stepsLeft))
		{
			buff.Write(now);
			float sps = nextPulse == lastPulse ? 0 : (1.0f / buff.AverageSpeed()) * 1000000.0f;
			MicroStep ms = LogDistribution.MicroStepForSpeed(sps);
			/*if (ms != lastStep)
			{
				lastStep = ms;
				Serial.print("Changed step: ");
				Serial.println(StepFactor(ms));
			}*/
			SetMicroStepPins(ms);
			digitalWrite(STEP_PIN, HIGH);
			delayMicroseconds(1);
			digitalWrite(STEP_PIN, LOW);
			delayMicroseconds(1);
			stepsLeft -= StepFactor(ms);
		}
	}
}

MicroStep SerialInputToMicroStep(int input)
{
	/*int base = 1;
	int result = 0;
	for (size_t i = 0; i < 3; i++)
	{
		if ((input & base) == base) bitWrite(result, 2 - i, 1);
		base *= 10;
	}
	return (MicroStep)result;*/
	switch (input)
	{
	case 1:
		return MicroStep::MSStep;
	case 2:
		return MicroStep::MSHalf;
	case 4:
		return MicroStep::MSQuarter;
	case 8:
		return MicroStep::MSEighth;
	case 16:
		return MicroStep::MSSixteenth;
	default:
		break;
	}
}

enum SerialCommand
{
	None = 0,
	MS_Steps_StpPerSec = 1,
	Steps_Time = 2,
};

void ReadSerial()
{
	if (!Serial.available()) return;
	SerialCommand cmd = (SerialCommand)Serial.parseInt();
	switch (cmd)
	{
	case None:
		break;
	case MS_Steps_StpPerSec:
	{
		int microstep = Serial.parseInt();
		MicroStep ms = SerialInputToMicroStep(microstep);
		Serial.print("MS: ");
		Serial.println((int)ms, 2);
		int steps = Serial.parseInt();
		float sps = Serial.parseFloat();
		Step(ms, steps, sps);
		Serial.println("Done");
		while (Serial.available()) Serial.read();
	}
		break;
	case Steps_Time:
	{
		int steps = Serial.parseInt();
		uint32_t timeMS = Serial.parseFloat();
		StepWithAccellerationCurve(&QuadraticInOutCurve, steps, timeMS);
		Serial.println("Done");
	}
		break;
	default:
		break;
	}
}


// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	
	LogDistribution.MaxStepPerSec = 800;
	LogDistribution.StepScaling = 1050;
	LogDistribution.Factor = 4.5f;
	LogDistribution.Base = 1.07f;

	LogDistribution.ComputeValues();
	LogDistribution.Display();
	pinMode(DIR_PIN, PinMode::OUTPUT);
	pinMode(STEP_PIN, PinMode::OUTPUT);
	pinMode(MS1_PIN, PinMode::OUTPUT);
	pinMode(MS2_PIN, PinMode::OUTPUT);
	pinMode(MS3_PIN, PinMode::OUTPUT);
}

// the loop function runs over and over again until power down or reset
void loop()
{
	ReadSerial();
}
