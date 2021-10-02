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
#define BUFFERS_SIZE 500

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
		return  ReadIndex - Index - 1;
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
	KeyframeDriverInstruction* instruction = DeclareNew(KeyframeDriverInstruction, sync, start, end, QuadraticInOutCurve);
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
		IsDone[channelID] = true;
		/*DebugStep();
		USB.print("Channel '");
		USB.print(channelID);
		USB.println("' is done!");
		USB.flush();*/

	}
	else if (buffer->Available() && result == DriverInstructionResult::Done)
	{
		DebugToolsStep("New instruction");
		/*DebugStep();
		USB.print("New instruction for channel: ");
		USB.println(channelID);
		USB.flush();*/
		auto kf = buffer->Read();
		ComputeInstruction(last[channelID], kf);
		last[channelID] = kf;
		//if (buffer->AvailableForWrite() > (buffer->Size / 2) && NextInstructionRequestTime[channelID] < millis())
		//{
		//	USB.print((uint16_t)StatusCode::ReadyForInstruction);
		//	USB.print(' ');
		//	USB.println(channelID);
		//	USB.flush();
		//	/*DebugStep();
		//	DebugTools.PrintCurrentlyAllocatedMemory();*/
		//	NextInstructionRequestTime[channelID] = millis() + 1500;
		//}
		if (buffer->Available() <= 0)
		{
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

	DebugStep();
	USB.println("Done!");
	//Serial.println(StatusCode::Debug);
	//Serial.println("Done");
	buffer->Clear();
	sync->Stop();
	//Motor1->SetInstruction(nullptr);
	return;
};

void ReadSerial()
{
	DebugToolsFunctionBeginNoWarn();
	if (!USB.available()) return;
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
	case 13: // Print allocated memory
		DebugTools.PrintCurrentlyAllocatedMemory();
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

	DebugTools.SetDebugOutput(&USB);
	pinMode(PrintDebugTracePin, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);

	//DBGValue(digitalRead(PrintDebugTracePin));

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
	/*DebugStep();
	DebugTools.PrintCurrentlyAllocatedMemory();*/

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

	static int hit = 0;

	/*if (hit != millis())
	{
		ReadSerial();
		hit = millis();
	}*/

	ReadSerial();

	//ReadSerial();
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
/*
7 0 0 800 7 1 0 0 8 0 8 1 7 0 1500 800 7 0 1770 1066 7 0 1905 1066 7 0 2067 533 7 0 2148 400 7 0 2229 533 7 0 2391 1066 7 0 2472 1200 7 0 2554 1066 7 0 2716 533 7 0 2797 400 7 0 2878 533 7 0 3040 1066 7 0 3121 1200 7 0 3202 1066 7 0 3364 533 7 0 3445 400 7 0 3527 533 7 0 3689 1066 7 0 3770 1200 7 0 3851 1066 7 0 4013 533 7 0 4094 400 7 0 4175 533 7 0 4337 1066 7 0 4418 1200 7 0 4500 1066 7 0 4716 1066 7 0 4824 800 7 0 4959 800 7 0 5067 1435 7 0 5202 1435 7 0 5310 557 7 0 5445 557 7 0 5554 1412 7 0 5689 1412 7 0 5797 800 7 0 5932 800 7 0 6040 1435 7 0 6175 1435 7 0 6283 557 7 0 6418 557 7 0 6527 1412 7 0 6662 1412 7 0 6770 800 7 0 6905 800 7 0 7013 1435
7 0 7148 1435 7 0 7256 557 7 1 1500 0 7 1 1770 266 7 1 1905 266 7 1 2067 -266 7 1 2229 266 7 1 2391 -266 7 1 2554 266 7 1 2716 -266 7 1 2878 266 7 1 3040 -266 7 1 3202 266 7 1 3364 -266 7 1 3527 266 7 1 3689 -266 7 1 3851 266 7 1 4013 -266 7 1 4175 266 7 1 4337 -266 7 1 4500 266 7 1 4716 266 7 1 4824 0 7 1 4959 0 7 1 5067 -581 7 1 5202 -581 7 1 5310 747 7 1 5445 747 7 1 5554 -858 7 1 5689 -858 7 1 5797 0 7 1 5932 0 7 1 6040 -581 7 1 6175 -581 7 1 6283 747 7 1 6418 747 7 1 6527 -858 7 1 6662 -858 7 1 6770 0 7 1 6905 0 7 1 7013 -581 7 1 7148 -581 7 1 7256 747 7 1 7391 747 7 1 7500 -858 7 1 7635 -858 7 1 7743 0 7 1 7878 0 7 1 7986 -581 7 1 8121 -581 7 1 8229 747 7 0 7391 557 7 0 7500 1412 7 0 7635 1412 7 0 7743 800 7 0 7878 800 7 0 7986 1435 7 0 8121 1435 7 0 8229 557 7 0 8364 557 7 0 8472 1412 7 0 8608 1412 7 0 8716 800 7 0 8851 800 4
*/

/*
7 0 0 800 7 1 0 0 7 0 1500 800 7 0 2334 1093 7 0 3168 618 7 0 4836 1095 7 0 6505 800 7 1 1500 0 7 1 2334 -400 7 1 4002 384 7 1 5670 -409 7 1 6505 0 4
*/