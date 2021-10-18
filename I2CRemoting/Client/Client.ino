/*
 Name:		Client.ino
 Created:	10/13/2021 4:11:31 PM
 Author:	civel
*/

#include <Wire.h>
#include "I2CRemoting.h"
#include "Setup.h"
#include "DebugTools.h"


void OnTestIntChanged(IntProp_t& sender, const IntEventArgs_t& args)
{
	Serial.print("Int changed to: ");
	Serial.println(args.NewValue);
}

void OnTestFloatChanged(FloatProp_t& sender, const FloatEventArgs_t& args)
{

}


void SerialCommand()
{
	if (!Serial.available()) return;
	int cmd = Serial.parseInt();
	switch (cmd)
	{
	case 1: // Set value
	{
		int value = Serial.parseInt();
		TestInt->SetValue(value);
	}
	break;
	case 2: // Print value
	{
		Serial.print("Int = ");
		Serial.println(TestInt->GetValue());
	}
	break;
	default:
		break;
	}
}

void setup()
{
	Serial.begin(9600);

	CmdReg.SetupClient();
	CmdSetup();
	if (DebugTools.WasLastResetFromWatchdog())
	{
		DebugTools.PrintStack();
		DebugTools.PrintDebugInfo();
	}

	DebugTools.SetupWatchdog(1000);
}


// the loop function runs over and over again until power down or reset
void loop()
{
	DebugToolsFunctionBeginNoWarn();
	DebugToolsStep("Loop");
	SerialCommand();
}
