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
#include <vector>

//#include <wdt.h>
#define DEBUG_TOOLS_FILE_NAME_LENGTH 32
#define DEBUG_TOOLS_STEP_NAME_LENGTH 32
#define DEBUG_TOOLS_FUNC_NAME_LENGTH 32
#define DEBUG_TOOLS_STACK_LENGTH 12
#define WDT_KEY (0xA5)

//Call often to reset the watchdog and indicate a new step.
//stepName must be less than 32 characters.
#define DebugToolsStep(stepName) DebugTools.ResetWatchdog(__LINE__, __FILE__, __FUNCTION__, stepName)

#define DebugToolsFunctionBegin() auto _reserved_var = TraceObject(__LINE__, __FILE__, __FUNCTION__)


enum DebugInfoDisplayFlags
{
	MentionFile =	0b00000001,
	MentionLine =	0b00000010,
	MentionName =	0b00000100,
	MentionFunc =	0b00001000,
	MentionCount=	0b00010000,
	MentionTrace=	0b00100000,
};

enum DebugInfoTypeFlags
{
	HasName =		0b00000111,
	HasFunc =		0b00001011,
	HasCount =		0b00010011,
	IsTrace =		0b00100011, // If meant to be used as a call trace
};

struct DebugInfo
{
	DebugInfoTypeFlags TypeFlags;
	uint32_t Line;
	uint32_t Count;

	char FileName[DEBUG_TOOLS_FILE_NAME_LENGTH];
	char StepName[DEBUG_TOOLS_STEP_NAME_LENGTH];
	char FunctionName[DEBUG_TOOLS_FUNC_NAME_LENGTH];

	void Display(Print& print, DebugInfoDisplayFlags displayFlags);
	
};

struct TraceObject
{
	TraceObject(uint32_t line, const char* fileName, const char* funcName);
	~TraceObject();
};

class DebugToolsClass
{
private:
	//using Alloc_t = std::array<DebugInfo, DEBUG_TOOLS_STACK_LENGTH>;
	using Stack_t = std::vector<DebugInfo>;

	Print* m_debugStream{&Serial};
	DebugInfo* m_debugInfo;
	//Alloc_t* m_array;
	Stack_t* m_callStack;
public:
	DebugToolsClass();
	void SetDebugOutput(Print* output);
	void SetupWatchdog(uint32_t ms);
	void FunctionEnter(uint32_t line, const char* fileName, const char* funcName);
	void FunctionExit();
	//void LogStep(uint32_t line, const char* fileName, );
	void ResetWatchdog(uint32_t line, const char* fileName, const char* funcName, const char* stepName);
	bool WasLastResetFromWatchdog();
	void PrintStack();
	/// <summary>
	/// Prints the last debug update from <see cref="ResetWatchdog"/>
	/// </summary>
	void PrintDebugInfo(DebugInfoDisplayFlags displayFlags = (DebugInfoDisplayFlags)(DebugInfoDisplayFlags::MentionFile | DebugInfoDisplayFlags::MentionCount | DebugInfoDisplayFlags::MentionFunc | DebugInfoDisplayFlags::MentionLine | DebugInfoDisplayFlags::MentionName));
};

extern DebugToolsClass DebugTools;

#endif

