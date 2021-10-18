#ifndef _RemotingUtillities_h
#define _RemotingUtillities_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Print.h"
#include "DebugTools.h"

enum class Endpoint
{
	None,
	Local,
	Remote,
};

#define DEBUG_ 1

#if defined(DEBUG_) && DEBUG_ >= 1

#define DebugError(errorMessage) Serial.print("[ERROR]");\
Serial.print(__FUNCTION__);\
Serial.print(" Line ");\
Serial.print(__LINE__);\
Serial.print(" ");\
Serial.println(errorMessage);\
Serial.flush()

#define DebugMemAddress(value) Serial.print("[MEM]");\
Serial.print(__FUNCTION__);\
Serial.print(" address ");\
Serial.print(#value);\
Serial.print(": ");\
Serial.println(reinterpret_cast<uint32_t>(value));\
Serial.flush()

#define DebugValue(value) Serial.print("[DEBUG]");\
Serial.print(__FUNCTION__);\
Serial.print(" ");\
Serial.print(#value);\
Serial.print(": ");\
Serial.println(value);\
Serial.flush()

#else
#define DebugError(errorMessage)
#define DebugMemAddress(value)
#define DebugValue(value)
#endif

void PrintBuffer(uint8_t* buff, uint8_t count);

namespace RemotingUtils
{
	

	template<typename T>
	void SendPacketOpenning(Print& print, T packet)
	{
	}

	template<typename T>
	void SendPacket(Print& print, T packet)
	{

	}
	constexpr uint8_t IntReply = 1;

}


#endif // !_RemotingUtillities_h