#ifndef _SpeedBuffer_h
#define _SpeedBuffer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "DebugTools.h"

struct SpeedBuffer
{
	size_t Count = 0;
	size_t Index = 0;
	size_t Size = 0;
	uint32_t* Deltas = nullptr;
	SpeedBuffer(size_t size)
	{
		Size = size;
		Deltas = DeclareNewArr(uint32_t, size);
	}

	void Write(uint32_t delta)
	{
		Deltas[Index] = delta;
		Index = (Index + 1) % Size;
		if (Count < Size) Count++;
	}

	uint32_t operator[](size_t index)
	{
		size_t i = (Index + index) % Count;
		return Deltas[i];
	}

	float AverageSpeed()
	{
		if (Count < 2) return Count == 0 ? 0 : operator[](0);
		return (float)(operator[](Count - 1) - operator[](0)) / (float)Count;
	}

	~SpeedBuffer()
	{
		DeclareDeleteArr(uint32_t, Size) Deltas;
	}
};

#endif // !_SpeedBuffer_h