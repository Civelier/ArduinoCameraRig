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

#define CHANNEL_COUNT 2


StepperDriver* Motor0 = new StepperDriver(200, 23, 22, 24, 25, 26);
StepperDriver* Motor1 = new StepperDriver(200, 28, 27, 29, 30, 31);
StepperDriver* Motors[2]
{
	Motor0,
	Motor1,
};

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
		buffer = (Keyframe*)malloc(size * sizeof(Keyframe));
		Count = 0;
		Index = 0;
		ReadIndex = 0;
		Size = size;
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
	size_t ReadIndices[CHANNEL_COUNT]{};
	size_t Size;
	CircularBuffer(size_t size)
	{
		buffer = (Keyframe*)malloc(size * sizeof(Keyframe));
		Count = 0;
		Index = 0;
		Size = size;
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
		size_t minReadIndex{ Count };
		for (size_t i = 0; i < CHANNEL_COUNT; i++)
		{
			minReadIndex = min(minReadIndex, ReadIndices[i]);
		}
		if (Index >= minReadIndex) return Index - minReadIndex;
		return Count - minReadIndex + Index;
	}

	void Clear()
	{
		Count = 0;
		for (size_t i = 0; i < CHANNEL_COUNT; i++)
		{
			ReadIndices[i] = 0;
		}
		Index = 0;
	}

	Keyframe Read(uint16_t channelID)
	{
		while (ReadIndices[channelID] < Index)
		{
			if (buffer[ReadIndices[channelID]].ChannelID == channelID) break;
			ReadIndices[channelID] = (ReadIndices[channelID] + 1) % Size;
		}
		Keyframe kf = buffer[ReadIndices[channelID]];
		//kf.Print();
		while (ReadIndices[channelID] < Index)
		{
			ReadIndices[channelID] = (ReadIndices[channelID] + 1) % Size;
			if (buffer[ReadIndices[channelID]].ChannelID == channelID) break;
		}
		return kf;
	}

	~CircularBuffer()
	{
		free(buffer);
	}
};

//Use before printing a debug message. This is ONLY valid for the next line of text printed.
#define DebugStep() USB.println(StatusCode::STDebug)


#define DebugValue(value) DebugStep(); USB.print(#value); USB.print(" = "); USB.println(value); USB.flush()

//Use before printing an error message. This is ONLY valid for the next line of text printed.
#define PrintErrorStep(message) DebugStep(); USB.print("Error at line <"); USB.print(__LINE__); USB.print(">: ")

StatusCode status = StatusCode::STReady;
CircularBuffer Buffer = CircularBuffer(500);
Keyframe last[CHANNEL_COUNT]{};
TimeSync* sync = new TimeSync();
bool Running = false;

StepperDriver* GetMotor(uint16_t channelID)
{
	if (channelID < CHANNEL_COUNT)
	{
		return Motors[channelID];
	}
	PrintErrorStep();
	USB.print("Attempt to get a motor from an undefined channel ID <id: ");
	USB.print(channelID);
	USB.println(">");
	USB.flush();
	exit(EXIT_FAILURE);
	return nullptr;
	/*switch (channelID)
	{
	case 0:
		return Motor0;
	case 1:
		return Motor1;
	default:
		PrintErrorStep();
		USB.print("Attempt to get a motor from an undefined channel ID <id: ");
		USB.print(channelID);
		USB.println(">");
		USB.flush();
		exit(EXIT_FAILURE);
		break;
	}*/
}

void ComputeInstruction(Keyframe start, Keyframe end)
{
	auto motor = GetMotor(start.ChannelID);
	motor->SetInstruction(new KeyframeDriverInstruction(sync, start, end, &QuadraticInOutCurve));
}

void InstructionCallback(uint16_t channelID, DriverInstructionResult result)
{
	if (!sync->Started) return;
	//if (result == DriverInstructionResult::Done) DBGValue(Buffer.Available());
	if (!Buffer.Available() && Running && result == DriverInstructionResult::Done)
	{
		Running = false;
		status = StatusCode::STReady;
		USB.println(STDone);
		//Serial.println(StatusCode::STDebug);
		//Serial.println("Done");
		Buffer.Clear();
		sync->Stop();
		//Motor1->SetInstruction(nullptr);
		return;
	}
	if (result == DriverInstructionResult::Done)
	{
		auto kf = Buffer.Read(channelID);
		ComputeInstruction(last[channelID], kf);
		last[channelID] = kf;
		//Serial.println(StatusCode::STReadyForInstruction);
	}
	
}

void serialEvent()
{
	int cmd = USB.parseInt();
	switch (cmd)
	{
	case 1: // Status request packet
		USB.println(status);
		USB.flush();
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
		status = StatusCode::STRunning;
		Running = true;
		for (size_t i = 0; i < CHANNEL_COUNT; i++)
		{
			auto start = Buffer.Read(i);
			auto end = Buffer.Read(i);
			last[i] = end;
			ComputeInstruction(start, end);
		}
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

void AtExit()
{
	DebugStep();
	USB.println("Program exitted due to an error");
	USB.flush();
}

// the setup function runs once when you press reset or power the board
void setup()
{
	atexit(AtExit);
	USB.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);
	while (!USB)
	{
		digitalWrite(LED_BUILTIN, millis() % 500 < 250);
	}

	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);

	MStep.AttachDriver(Motor0);
	MStep.AttachDriver(Motor1);
	MStep.AttachCallback(&InstructionCallback);
	USB.println(status);
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
	
	MStep.UpdateDrivers();
}

/*
7 0 0 0 7 0 1500 0 7 0 5670 442 7 0 14429 -344 7 0 19434 0 4
*/

