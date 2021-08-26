/*
 Name:		WatchdogTests.ino
 Created:	8/25/2021 1:16:26 PM
 Author:	civel
*/

#include "DebugTools.h"

void ExitCode()
{
	DebugTools.PrintDebugInfo();
}


void Func1()
{
	DebugToolsFunctionBegin();

	DebugToolsStep("Inside Func1!");
	//for (;;);
}

// the setup function runs once when you press reset or power the board
void setup()
{
	atexit(ExitCode);
	Serial.begin(115200);
	delay(2000);

	if (DebugTools.WasLastResetFromWatchdog())
	{
		DebugTools.PrintDebugInfo();
		Serial.println();
		DebugTools.PrintStack();
	}
	DebugTools.SetupWatchdog(1000);
}

// the loop function runs over and over again until power down or reset
void loop()
{
	DebugToolsFunctionBegin();
	Serial.println("Loop begin!");
	DebugToolsStep("Loop begin");

	Func1();

	// Stall the execition for watchdog reset
	for (;;)
	{
		DebugToolsStep("Inside loop");
		for (;;);
	}
}
