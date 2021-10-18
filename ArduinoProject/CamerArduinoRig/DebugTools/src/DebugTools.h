/*
 Name:		DebugTools.h
 Created:	8/25/2021 12:24:24 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#ifndef _DebugTools_h
#define _DebugTools_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//#include <stack>
//#include <array>
#ifndef ARDUINO_ARCH_MEGAAVR
#include "malloc.h"
#endif
//#include <wdt.h>
#define DEBUG_TOOLS_FILE_NAME_LENGTH 32
#define DEBUG_TOOLS_STEP_NAME_LENGTH 32
#define DEBUG_TOOLS_FUNC_NAME_LENGTH 32
#define DEBUG_TOOLS_STACK_LENGTH 15
#define WDT_KEY (0xA5)

enum Timeout {
	TIMEOUT_15MS = 15,
	TIMEOUT_30MS = 30,
	TIMEOUT_60MS = 60,
	TIMEOUT_120MS = 120,
	TIMEOUT_250MS = 250,
	TIMEOUT_500MS = 500,
	TIMEOUT_1S = 1000,
	TIMEOUT_2S = 2000,
	TIMEOUT_4S = 4000,
	TIMEOUT_8S = 8000
};
	
//Call often to reset the watchdog and indicate a new step.
//stepName must be less than 32 characters.
#define DebugToolsStep(stepName) DebugTools.ResetWatchdog(__LINE__, __FILE__, __FUNCTION__, stepName)

#define CONCAT_TOKEN1(x, y) x##y

#define CONCAT_TOKEN2(x, y) CONCAT_TOKEN1(x, y)

#define DebugToolsFunctionBegin() auto CONCAT_TOKEN2(_reserved_var, __COUNTER__) = TraceObject(__LINE__, __FILE__, __FUNCTION__, false)


#define DebugToolsFunctionBeginAlloc(memoryAllowed) auto CONCAT_TOKEN2(_reserved_var, __COUNTER__) = TraceObject(__LINE__, __FILE__, __FUNCTION__, false, memoryAllowed)

#define DebugToolsFunctionBeginNoWarn() auto CONCAT_TOKEN2(_reserved_var, __COUNTER__) = TraceObject(__LINE__, __FILE__, __FUNCTION__, true)

#define DeclareNew(type, ...) new type(__VA_ARGS__); DebugTools.NotifyMemoryAllocation(sizeof(type))

#define DeclareDelete(type) DebugTools.NotifyMemoryFree(sizeof(type)); delete 

#define DeclareDeleteSize(size) DebugTools.NotifyMemoryFree(size); delete 

#define DeclareNewArr(type, size) new type[size]{}; DebugTools.NotifyMemoryAllocation(sizeof(type) * size)

#define DeclareDeleteArr(type, size) DebugTools.NotifyMemoryFree(sizeof(type) * size); delete[] 

enum DebugInfoDisplayFlags
{
	MentionFile =	0b00000001,
	MentionLine =	0b00000010,
	MentionName =	0b00000100,
	MentionFunc =	0b00001000,
	MentionCount=	0b00010000,
	MentionTrace=	0b00100000,
	MentionMemory=	0b01000000,
};

enum DebugInfoTypeFlags
{
	HasName =		0b00000111,
	HasFunc =		0b00001011,
	HasCount =		0b00010011,
	IsTrace =		0b00100011, // If meant to be used as a call trace
};

class ISizeable
{
public:
	virtual size_t Size() const = 0;
};

struct DebugInfo
{
	DebugInfoTypeFlags TypeFlags;
	uint16_t Line;
	uint16_t Count;
	int MemLeft;
#ifdef ARDUINO_ARCH_MEGAAVR
	bool DisplayOnNextRun;
#endif

	char FileName[DEBUG_TOOLS_FILE_NAME_LENGTH];
	char StepName[DEBUG_TOOLS_STEP_NAME_LENGTH];
	char FunctionName[DEBUG_TOOLS_FUNC_NAME_LENGTH];

	void Display(Print& print, DebugInfoDisplayFlags displayFlags);
};

extern const size_t MaxMem;	

template <typename T>
class Stack
{
private:
	T* m_arr;
	size_t m_count;
	size_t m_capacity;
public:
	Stack(size_t maxLength);
	void push_back(const T& item);
	void pop_back();
	T& back();
	T& operator[](size_t index);
	size_t size();
	T* begin();
	T* end();
	~Stack();
};

template<typename T>
inline Stack<T>::Stack(size_t maxLength)
{
	m_arr = new T[maxLength];
}

template<typename T>
inline void Stack<T>::push_back(const T& item)
{
	m_arr[m_count] = item;
	m_count++;
}

template<typename T>
inline void Stack<T>::pop_back()
{
	--m_count;
}

template<typename T>
inline T& Stack<T>::back()
{
	return m_arr[m_count];
}

template<typename T>
inline T& Stack<T>::operator[](size_t index)
{
	return m_arr[index];
}

template<typename T>
inline size_t Stack<T>::size()
{
	return m_count;
}

template<typename T>
inline T* Stack<T>::begin()
{
	return m_arr;
}

template<typename T>
inline T* Stack<T>::end()
{
	return m_arr + m_count + 1;
}

template<typename T>
inline Stack<T>::~Stack()
{
	delete[] m_arr;
}

class DebugToolsClass
{
private:
	//using Alloc_t = std::array<DebugInfo, DEBUG_TOOLS_STACK_LENGTH>;
	using Stack_t = Stack<DebugInfo>;

	Stream* m_debugStream{&Serial};
	DebugInfo* m_debugInfo;
	//Alloc_t* m_array;
	Stack_t* m_callStack;
	int m_allocated;
public:
	DebugToolsClass();
	void SetDebugOutput(Stream* output);
	Stream* GetDebugOutput();
	size_t GetFreeMem();
	void PrintCurrentlyAllocatedMemory();
	void SetupWatchdog(uint32_t ms);
	void FunctionEnter(uint16_t line, const char* fileName, const char* funcName);
	void FunctionExit(int memLeft, bool suppressWarning = true, int memAllowed = 0);
	//void LogStep(uint32_t line, const char* fileName, );
	void ResetWatchdog(uint16_t line, const char* fileName, const char* funcName, const char* stepName);
	bool WasLastResetFromWatchdog();
	void PrintStack();
	/// <summary>
	/// Warning, this will override atexit(), exit the program and wipe the entire ram (flash, aka program, will not be erased)
	/// </summary>
	void CleanMemory();		
	void NotifyMemoryAllocation(int size);
	void NotifyMemoryFree(int size);
	int GetAllocatedMemory();
	/// <summary>
	/// Prints the last debug update from <see cref="ResetWatchdog"/>
	/// </summary>
	void PrintDebugInfo(DebugInfoDisplayFlags displayFlags = (DebugInfoDisplayFlags)(DebugInfoDisplayFlags::MentionFile | DebugInfoDisplayFlags::MentionCount | DebugInfoDisplayFlags::MentionFunc | DebugInfoDisplayFlags::MentionLine | DebugInfoDisplayFlags::MentionName | DebugInfoDisplayFlags::MentionMemory));
};

extern DebugToolsClass DebugTools;

struct TraceObject
{
	int MemLeft;
	int MemAllowed;
	bool SuppressWarning;
	TraceObject(uint16_t line, const char* fileName, const char* funcName, bool suppressWarning = true, int memoryAllowed = 0)
	{
		DebugTools.FunctionEnter(line, fileName, funcName);
		MemLeft = DebugTools.GetAllocatedMemory();
		MemAllowed = memoryAllowed;
		SuppressWarning = suppressWarning;
	}

	~TraceObject()
	{
		DebugTools.FunctionExit(MemLeft, SuppressWarning, MemAllowed);
	}
};




#endif