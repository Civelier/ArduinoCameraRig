/*
 Name:		MultiStepperTest.ino
 Created:	5/27/2021 6:45:08 PM
 Author:	civel
*/

//#define MSTEP_DEBUG
#include "MultiStepper.h"
#include "Keyframe.h"
#include "KeyframeInstruction.h"

#define CHANNEL_COUNT 1

StepperDriver* Motor1 = new StepperDriver(200, 2, 3, 4, 5, 6);

enum StatusCode
{
	STReady = 1,
	STRunning = 2,
	STDone = 3,
	STDebug = 4,
	STReadyForInstruction = 5,
	STError = 0b10000000,
	STError1 = StatusCode::STError | 1,
};

struct KeyframeBuffer
{
	Keyframe* buffer;
	size_t Count;
	size_t Index;
	size_t ReadIndex;
	size_t Size;
	KeyframeBuffer(size_t size)
	{
		Size = size;
		buffer = (Keyframe*)malloc(size * sizeof(Keyframe));
	}

	void ResetRead()
	{
		Index = 0;
	}

	void Write(Keyframe kf)
	{
		buffer[Index] = kf;
		//kf.Print();
		Count++;
		Index++;
	}

	int Available()
	{
		return Count - Index;
	}

	void Clear()
	{
		Count = 0;
		ReadIndex = 0;
		Index = 0;
	}

	Keyframe Read()
	{
		Keyframe kf = buffer[ReadIndex];
		//kf.Print();
		ReadIndex++;
		return kf;
	}

	~KeyframeBuffer()
	{
		free(buffer);
	}
};

struct CircularBuffer
{
	Keyframe* buffer;
	size_t Count;
	size_t Index;
	size_t ReadIndex;
	size_t Size;
	CircularBuffer(size_t size)
	{
		Size = size;
		buffer = (Keyframe*)malloc(size * sizeof(Keyframe));
	}

	void Write(Keyframe kf)
	{
		buffer[Index] = kf;
		//kf.Print();
		Count = max(Count, Index + 1);
		Index = (Index + 1) % Size;
	}

	int Available()
	{
		if (Index >= ReadIndex) return Index - ReadIndex;
		return Count - ReadIndex + Index;
	}

	void Clear()
	{
		Count = 0;
		ReadIndex = 0;
		Index = 0;
	}

	Keyframe Read()
	{
		Keyframe kf = buffer[ReadIndex];
		//kf.Print();
		ReadIndex = (ReadIndex + 1) % Size;
		return kf;
	}

	~CircularBuffer()
	{
		free(buffer);
	}
};

#define DebugValue(value) Serial.println(StatusCode::STDebug); Serial.print(#value); Serial.print(" = "); Serial.println(value)

StatusCode status = StatusCode::STReady;
KeyframeBuffer Buffer = KeyframeBuffer(10);
Keyframe last;
TimeSync* sync = new TimeSync();
bool Running;

void ComputeInstruction(Keyframe start, Keyframe end)
{
	if (start.ChannelID == 0)
	{
		Motor1->SetInstruction(new KeyframeDriverInstruction(sync, start, end));
	}
}

void InstructionCallback(uint16_t channelID, DriverInstructionResult result)
{
	if (!sync->Started) return;
	if (!Buffer.Available() && Running)
	{
		Running = false;
		status = StatusCode::STReady;
		Serial.println(STDone);
		Serial.println(StatusCode::STDebug);
		Serial.println("Done");
		Buffer.ResetRead();
		sync->Stop();
		//Motor1->SetInstruction(nullptr);
		return;
	}
	if (channelID == 0 && result == DriverInstructionResult::Done)
	{
		auto kf = Buffer.Read();
		ComputeInstruction(last, kf);
		last = kf;
		//Serial.println(StatusCode::STReadyForInstruction);
	}
}

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	MStep.AttachDriver(Motor1);
	MStep.AttachCallback(&InstructionCallback);
	Serial.println(status);
}

void ReadSerial()
{
	if (!Serial.available()) return;
	int cmd = Serial.parseInt();
	switch (cmd)
	{
	case 1: // Status request packet
		Serial.println(status);
		break;
	case 2: // Error clear packet
		break;
	case 3: // Motor reset packet
	{
		int steps[CHANNEL_COUNT];
		for (size_t i = 0; i < CHANNEL_COUNT; i++)
		{
			steps[i] = Serial.parseInt();
		}
	}
	break;
	case 4: // Start packet
	{
		auto start = Buffer.Read();
		auto end = Buffer.Read();
		last = end;
		status = StatusCode::STRunning;
		Running = true;
		ComputeInstruction(start, end);
	}
	break;
	case 5:
		DebugValue(sync->CurrentMicros());
		break;
	case 6:
		sync->Start();
		break;
	case 7: // Keyframe packet
	{
		uint16_t id = Serial.parseInt();
		uint32_t ms = Serial.parseInt();
		uint32_t steps = Serial.parseInt();
		Buffer.Write(Keyframe{ id, ms, steps });
	}
	break;
	default:
		break;
	}
}

// the loop function runs over and over again until power down or reset
void loop()
{
	/*static int hit = 0;
	if (hit++ == 500 && Serial.available())
	{
		int32_t steps = Serial.parseInt();
		uint32_t speed = Serial.parseInt();
		Serial.print("Moving: ");
		Serial.print(steps);
		Serial.print("steps at speed: ");
		Serial.println(speed);
		Motor1->SetInstruction(new StepLinearDriverInstruction(speed, steps));
		hit = 0;
	}*/
	ReadSerial();

	MStep.UpdateDrivers();
}
