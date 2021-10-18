/*
 Name:		Master.ino
 Created:	10/2/2021 4:50:53 PM
 Author:	civel
*/

#include <Wire.h>
#include "I2CRemoting.h"
#include "Setup.h"

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

	CmdReg.SetupMaster();
	CmdSetup();

}

void loop()
{
	SerialCommand();
	CmdReg.MasterRefresh();
}
