#include "RemotingUtillities.h"

void PrintBuffer(uint8_t* buff, uint8_t count)
{
	DebugToolsFunctionBegin();
	for (uint8_t i = 0; i < count; i++)
	{
		Serial.print((int)(buff[i]));
		Serial.print(' ');
	}
	Serial.println();
}
