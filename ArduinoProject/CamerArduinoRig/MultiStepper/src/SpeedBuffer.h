#ifndef _SpeedBuffer_h
#define _SpeedBuffer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

struct SpeedBuffer
{
	size_t Count;
	size_t Index;
	size_t Size;
	uint32_t* Deltas;
	SpeedBuffer(size_t size)
	{
		Size = size;
		Deltas = (uint32_t*)malloc(size * sizeof(uint32_t));
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
		free(Deltas);
	}
};

#endif // !_SpeedBuffer_h