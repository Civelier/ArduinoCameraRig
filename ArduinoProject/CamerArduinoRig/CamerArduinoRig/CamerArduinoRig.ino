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
#include "DebugTools.h"

#ifndef max
#define max(x,y) (x > y ? x : y)
#endif // !max


#define CHANNEL_COUNT 2
#define BUFFERS_SIZE 50

#define PrintDebugTracePin 53

using smallSize_t = uint16_t;


StepperDriver* Motors[CHANNEL_COUNT]
{
	new StepperDriver(200, 23, 22, 24, 25, 26),
	new StepperDriver(200, 28, 27, 29, 30, 31),
};



enum class StatusCode : uint16_t
{
	Ready = 1,
	Running = 2,
	Done = 3,
	Debug = 4,
	ReadyForInstruction = 5,
	Value = 6,
	DebugBlockBegin = 7,
	DebugBlockEnd = 8,
	Error = 0b10000000,
	MotorChannelOutOfRangeError = Error | 1,
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


#define DebugStep() USB.println((uint16_t)StatusCode::Debug); USB.print('<'); USB.print(__LINE__); USB.print("> ")

#define Adr(value) DebugStep(); USB.print("&"); USB.print(#value); USB.print(" = "); USB.println((int)&value, HEX); USB.flush()

#define DebugValue(value) DebugStep(); USB.print(#value); USB.print(" = "); USB.println(value); USB.flush()

#define DebugBegin() USB.println((uint16_t)StatusCode::DebugBlockBegin); USB.print('<'); USB.print(__LINE__); USB.print("> ")

#define DebugEnd() USB.println((uint16_t)StatusCode::DebugBlockEnd)

//Use before printing an error message. This is ONLY valid for the next line of text printed.
#define PrintErrorStep(message) DebugStep(); USB.print("Error at line <"); USB.print(__LINE__); USB.print(">: ")

struct CircularBuffer
{
	Keyframe* buffer;
	smallSize_t Count;
	smallSize_t Index;
	smallSize_t ReadIndex;
	smallSize_t Size;
	CircularBuffer(size_t size)
	{
		buffer = DeclareNewArr(Keyframe, size);
		Count = 0;
		Index = 0;
		ReadIndex = 0;
		Size = size;
	}

	void Write(const Keyframe& kf)
	{
		DebugToolsFunctionBegin();
		if (kf.ChannelID > CHANNEL_COUNT)
		{
			DebugTools.PrintStack();
		}
		buffer[Index] = kf;
		/*DBGValue(Index);
		Adr(buffer[Index]);*/
		//kf.Print();
		Count = max(Count, Index + 1);
		Index = (Index + 1) % Size;
	}

	int AvailableForWrite()
	{
		DebugToolsFunctionBegin();
		if (Index >= ReadIndex) return Size - Index + ReadIndex;
		return  ReadIndex - Index;
	}

	int Available()
	{
		DebugToolsFunctionBegin();
		/*
		* When Index (I) is in front of ReadIndex (R) available (+) is the difference between the two
		* ***R+++I*
		*/
		if (Index >= ReadIndex) return Index - ReadIndex;
		/*
		* Otherwise the remainder (Count - ReadIndex) plus what Index has written
		* +++I***R+
		*/
		return Count - ReadIndex + Index;
	}

	void Clear()
	{
		DebugToolsFunctionBegin();
		Count = 0;
		ReadIndex = 0;
		Index = 0;
	}

	const Keyframe& Read()
	{
		DebugToolsFunctionBegin();
		const Keyframe& kf = buffer[ReadIndex];
		/*DBGValue(ReadIndex);
		Adr(kf);*/
		ReadIndex = (ReadIndex + 1) % Size;
		return kf;
	}

	~CircularBuffer()
	{
		DeclareDeleteArr(Keyframe, Size) buffer;
	}
};

//Use before printing a debug message. This is ONLY valid for the next line of text printed.


StatusCode status = StatusCode::Ready;
CircularBuffer Buffers[] =
{
	CircularBuffer(BUFFERS_SIZE),
	CircularBuffer(BUFFERS_SIZE),
};


Keyframe last[CHANNEL_COUNT]{};
TimeSync* sync = new TimeSync();
bool Running = false;
bool IsDone[CHANNEL_COUNT]{};
uint32_t NextInstructionRequestTime[CHANNEL_COUNT]{};

CircularBuffer* GetBuffer(uint16_t channelID)
{
	DebugToolsFunctionBegin();
	if (channelID < CHANNEL_COUNT)
	{
		return &Buffers[channelID];
	}
	PrintErrorStep();
	USB.print("Attempt to get a buffer from an undefined channel ID <id: ");
	USB.print(channelID);
	USB.println(">");
	USB.flush();
	exit(EXIT_FAILURE);
}

StepperDriver* GetMotor(uint16_t channelID)
{
	DebugToolsFunctionBegin();
	if (channelID < CHANNEL_COUNT)
	{
		return Motors[channelID];
	}
	PrintErrorStep();
	USB.print("Attempt to get a motor from an undefined channel ID <id: ");
	USB.print(channelID);
	USB.println(">");
	DebugTools.PrintStack();
	USB.flush();
	//exit(EXIT_FAILURE);
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

void ComputeInstruction(const Keyframe& start, const Keyframe& end)
{
	DebugToolsFunctionBeginAlloc(120); 
	// allow allocation of 120 bytes because sizeof(KeyframeDriverInstruction) = 80
	// and sizeof(KeyframeDriverInstruction.SpeedBuffer.Buffer) = 40
	// So 120 bytes
	
	/*Adr(start);
	Adr(end);

	DBGValue(start.ChannelID);*/
	auto motor = GetMotor(start.ChannelID);
	if (!motor) return;
	KeyframeDriverInstruction* instruction = DeclareNew(KeyframeDriverInstruction, sync, start, end, LinearCurve);
	motor->SetInstruction(instruction);
}

void InstructionCallback(uint16_t channelID, DriverInstructionResult result)
{
	DebugToolsFunctionBeginNoWarn(); // Because of ComputeInstruction
	if (!Running) return;
	auto buffer = GetBuffer(channelID);
	if (!sync->Started) return;
	//if (result == DriverInstructionResult::Done) DBGValue(Buffer.Available());
	if (!buffer->Available() && result == DriverInstructionResult::Done)
	{
		DebugStep();
		USB.print("Channel '");
		USB.print(channelID);
		USB.println("' is done!");
		USB.flush();
		return;
	}
	
	if (buffer->Available() && result == DriverInstructionResult::Done)
	{
		DebugToolsStep("New instruction");
		/*DebugStep();
		USB.print("New instruction for channel: ");
		USB.println(channelID);
		USB.flush();*/
		auto kf = buffer->Read();
		ComputeInstruction(last[channelID], kf);
		last[channelID] = kf;
		if (buffer->AvailableForWrite() > (buffer->Size / 2) && NextInstructionRequestTime[channelID] < millis())
		{
			USB.print((uint16_t)StatusCode::ReadyForInstruction);
			USB.print(' ');
			USB.println(channelID);
			USB.flush();
			DebugStep();
			DebugTools.PrintCurrentlyAllocatedMemory();
			NextInstructionRequestTime[channelID] = millis() + 1500;
		}
		//Serial.println(StatusCode::ReadyForInstruction);
	}
	for (size_t i = 0; i < CHANNEL_COUNT; i++)
	{
		if (!IsDone[i]) return;
		auto buff = GetBuffer(i);
		if (buff && buff->Available()) return;
	}
	Running = false;
	status = StatusCode::Ready;
	USB.println((uint16_t)StatusCode::Done);
	//Serial.println(StatusCode::Debug);
	//Serial.println("Done");
	buffer->Clear();
	sync->Stop();
	//Motor1->SetInstruction(nullptr);
	return;
}

void serialEvent()
{
	DebugToolsFunctionBeginNoWarn();
	int cmd = USB.parseInt();
	switch (cmd)
	{
	case 1: // Status request packet
		USB.println((uint16_t)status);
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
		status = StatusCode::Running;
		Running = true;
		for (size_t i = 0; i < CHANNEL_COUNT; i++)
		{
			auto buff = GetBuffer(i);
			auto start = buff->Read();
			auto end = buff->Read();
			last[i] = end;
			DebugToolsFunctionBeginAlloc(120);
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
		DebugToolsFunctionBegin();
		GetBuffer(id)->Write(Keyframe{ id, ms, steps });
	}
		break;
	case 8: // Request available for write on specific buffer
	{
		int channelID = USB.parseInt();
		USB.print((uint16_t)StatusCode::Value);
		USB.print(' ');
		USB.println(GetBuffer(channelID)->AvailableForWrite());
		USB.flush();
		/*DebugStep();
		USB.print("Requested for buffer length of 'channel ");
		USB.print(channelID);
		USB.print("' = ");
		USB.println(GetBuffer(channelID).AvailableForWrite());*/
	}
		break;
	case 9: // Request available to read on specific buffer
	{
		int channelID = USB.parseInt();
		USB.print((uint16_t)StatusCode::Value);
		USB.print(' ');
		USB.println(GetBuffer(channelID)->Available());
		USB.flush();
	}
		break;
	case 10: // Request to print a keyframe from specific buffer at index
	{
		int channelID = USB.parseInt();
		int index = USB.parseInt();
		DebugStep();
		USB.print("Channel: ");
		USB.print(channelID);
		USB.print(" Index: ");
		USB.print(index);
		USB.print(" Keyframe: ");
		GetBuffer(channelID)->buffer[index].Print();
	}
		break;
	case 11: // Instruct that the specific channel is done recieving keyframes
	{
		int channelID = USB.parseInt();
		IsDone[channelID] = true;
	}
		break;
	case 12:
		DebugTools.CleanMemory();
		break;
	default:
		break;
	}
}

void AtExit()
{
	DebugBegin();
	USB.println("Program exitted due to an error");
	DebugTools.PrintStack();
	DebugEnd();
	USB.flush();

}


// the setup function runs once when you press reset or power the board
void setup()
{
	atexit(AtExit);
	USB.begin(115200);

	DebugTools.PrintCurrentlyAllocatedMemory();
	DebugTools.SetDebugOutput(&USB);
	pinMode(PrintDebugTracePin, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);

	DBGValue(digitalRead(PrintDebugTracePin));

	if (digitalRead(PrintDebugTracePin))
	{
		DebugBegin();
		DebugTools.PrintDebugInfo();
		/*USB.println();
		DebugTools.PrintStack();*/
		DebugEnd();
	}

	DebugTools.SetupWatchdog(3000);

	pinMode(LED_BUILTIN, OUTPUT);
	while (!USB)
	{
		DebugToolsStep("Waiting for usb connection");
		digitalWrite(LED_BUILTIN, millis() % 500 < 250);
	}

	DebugToolsStep("Flashing LED");
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	DebugToolsStep("Flashing LED");
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	DebugToolsStep("Flashing LED");
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	DebugToolsStep("Flashing LED");
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	DebugToolsStep("Flashing LED");
	digitalWrite(LED_BUILTIN, LOW);
	for (size_t i = 0; i < CHANNEL_COUNT; i++)
	{
		MStep.AttachDriver(Motors[i]);
	}

	MStep.AttachCallback(&InstructionCallback);
	USB.println((uint16_t)status);
	DebugTools.PrintCurrentlyAllocatedMemory();

	/*USB.println("Memory test");
	{
		DebugToolsFunctionBegin();
		USB.println("Test1");
		Keyframe kf1{ 0, 0, 0 };
		Keyframe kf2{ 0, 1500, 0 };
		auto instruct = DeclareNew(KeyframeDriverInstruction, sync, kf1, kf2, LinearCurve);
		DebugTools.PrintCurrentlyAllocatedMemory();
		delete instruct;
		DebugTools.PrintCurrentlyAllocatedMemory();
	}*/


	DebugToolsStep("End of setup");
}



// the loop function runs over and over again until power down or reset
void loop()
{
	DebugToolsFunctionBeginNoWarn();
	DebugToolsStep("Loop update");
	//for (;;);
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
7 0 0 0 7 1 0 0 7 0 1500 0 7 1 1500 0 7 0 5670 442 7 1 5670 442 7 0 14429 -344 7 1 14429 -344 7 0 19434 0 7 1 19434 0 4 11 0 11 1
*/

