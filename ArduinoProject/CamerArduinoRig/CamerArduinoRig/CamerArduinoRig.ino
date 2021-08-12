/*
 Name:		MultiStepperTest.ino
 Created:	5/27/2021 6:45:08 PM
 Author:	civel
*/

//#define MSTEP_DEBUG
#define USB Serial

#include "MultiStepper.h"
#include "Keyframe.h"
#include "KeyframeInstruction.h"

#define CHANNEL_COUNT 1


StepperDriver* Motor1 = new StepperDriver(200, 23, 22, 24, 25, 26);
StepperDriver* Motor2 = new StepperDriver(200, 28, 27, 29, 30, 31);

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
		ReadIndex = 0;
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
		return Index - ReadIndex;
	}

	int AvailableForWrite()
	{
		return Size - Index;
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

#define DebugValue(value) USB.println(StatusCode::STDebug); USB.print(#value); USB.print(" = "); USB.println(value)

StatusCode status = StatusCode::STReady;
CircularBuffer Buffer = CircularBuffer(10);
Keyframe last;
TimeSync* sync = new TimeSync();
bool Running;

void ComputeInstruction(Keyframe start, Keyframe end)
{
	if (start.ChannelID == 0)
	{
		Motor1->SetInstruction(new KeyframeDriverInstruction(sync, start, end, &QuadraticInOutCurve));
	}
}

void InstructionCallback(uint16_t channelID, DriverInstructionResult result)
{
	if (!sync->Started) return;
	//if (result == DriverInstructionResult::Done) DBGValue(Buffer.Available());
	if (!Buffer.Available() && Running && result == DriverInstructionResult::Done)
	{
		Running = false;
		status = StatusCode::STReady;
		Serial.println(STDone);
		//Serial.println(StatusCode::STDebug);
		//Serial.println("Done");
		Buffer.Clear();
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
	USB.begin(115200);
	while (!USB);

	MStep.AttachDriver(Motor1);
	MStep.AttachDriver(Motor2);
	MStep.AttachCallback(&InstructionCallback);
	USB.println(status);
}



void ReadSerial()
{
	if (!USB.available()) return;
	int cmd = USB.parseInt();
	switch (cmd)
	{
	case 1: // Status request packet
		USB.println(status);
		break;
	case 2: // Error clear packet
		break;
	case 3: // Motor reset packet
	{
		int steps[CHANNEL_COUNT];
		for (size_t i = 0; i < CHANNEL_COUNT; i++)
		{
			steps[i] = USB.parseInt();
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
		uint16_t id = USB.parseInt();
		uint32_t ms = USB.parseInt();
		uint32_t steps = USB.parseInt();
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

/*
7 0 0 0 7 0 1500 0 7 0 5670 442 7 0 14429 -344 7 0 19434 0 4
*/

