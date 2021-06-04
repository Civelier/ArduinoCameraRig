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

struct LogarithmicStepDistribution
{
	float MaxStepPerSec;
	float StepScaling;
	float Factor;
	float Base;
	void Update()
	{
		MaxStepPerSec = Serial.parseFloat();
		StepScaling = Serial.parseFloat();
		Factor = Serial.parseFloat();
		Base = Serial.parseFloat();
	}
};

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
	default:
		break;
	}
}


// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
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
