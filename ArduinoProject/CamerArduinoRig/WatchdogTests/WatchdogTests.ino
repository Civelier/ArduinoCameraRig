/*
 Name:		WatchdogTests.ino
 Created:	8/25/2021 1:16:26 PM
 Author:	civel
*/

#include "DebugTools.h"




int*& Func1()
{
	DebugToolsFunctionBegin();

	auto x = new int{ 10 }; // memory leak
	return x;
	DebugToolsStep("Inside Func1!");
}

void Func2(int*& ptr)
{
	DebugToolsFunctionBegin();
	delete ptr; // memory freed
	ptr = nullptr;
}

void Func3()
{
	DebugToolsFunctionBegin();
	auto x = new int{ 30 };

	delete x;
	x = nullptr;
}

int*& Func4()
{
	DebugToolsFunctionBeginAlloc(); // suppress warning
	auto x = (int*)malloc(sizeof(int)); // create a pointer

	return x; // valid use of suppress
}

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	delay(2000);

	int x;
	if (x != 0xFFFFFFFF && DebugTools.WasLastResetFromWatchdog())
	{
		DebugTools.PrintDebugInfo();
		Serial.println();
		DebugTools.PrintStack();
	}
	x = 0;
}

void serialEvent()
{
	int c = Serial.parseInt();
	if (c == 1)
	{
		while (Serial.available()) Serial.read();
		//DebugTools.CleanMemory();
	}
}

// the loop function runs over and over again until power down or reset
void loop()
{
	DebugToolsFunctionBegin();
	Serial.println("Loop begin!");
	DebugToolsStep("Loop begin");

	Func1();
	auto alloc = new int{ 20 };
	Func2(alloc);
	Func3();
	auto x_ptr = Func4();
	delete x_ptr;
	Serial.flush();
	// Stall the execition for watchdog reset
	delay(500);
}
