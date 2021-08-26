/*
 Name:		DebugTools.cpp
 Created:	8/25/2021 12:24:24 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#include "DebugTools.h"
#include "watchdog.h"

void watchdogSetup(void) {/*** watchdogDisable (); ***/ }

TraceObject::TraceObject(uint32_t line, const char* fileName, const char* funcName)
{
    DebugTools.FunctionEnter(line, fileName, funcName);
}

TraceObject::~TraceObject()
{
    DebugTools.FunctionExit();
}

void DebugToolsClass::SetDebugOutput(Print* output)
{
	m_debugStream = output;
}

void DebugToolsClass::FunctionEnter(uint32_t line, const char* fileName, const char* funcName)
{
    DebugInfo debugInfo{ (DebugInfoTypeFlags)(DebugInfoTypeFlags::HasFunc | DebugInfoTypeFlags::IsTrace) };
    char* file_ptr = (char*)fileName;
    debugInfo.Line = { line };

    for (char* c = file_ptr; *c != 0; c++)
    {
        if (*c == '\\') file_ptr = c;
    }

    strlcpy(debugInfo.FileName, file_ptr, DEBUG_TOOLS_FILE_NAME_LENGTH);
    strlcpy(debugInfo.FunctionName, funcName, DEBUG_TOOLS_STEP_NAME_LENGTH);
    *m_debugInfo = debugInfo;
    m_callStack->push_back(debugInfo);
}

void DebugToolsClass::FunctionExit()
{
    m_callStack->pop_back();
    *m_debugInfo = m_callStack->back();
}

void DebugToolsClass::SetupWatchdog(uint32_t ms)
{
    watchdogEnable(ms);
    
    //m_array = new Alloc_t(DEBUG_TOOLS_STACK_LENGTH, {});
    *m_callStack = Stack_t(0);
}

void DebugToolsClass::ResetWatchdog(uint32_t line, const char* fileName, const char* funcName, const char* stepName)
{
    watchdogReset();
    
    m_debugInfo->Line = { line };
    m_debugInfo->TypeFlags = (DebugInfoTypeFlags)(DebugInfoTypeFlags::HasFunc | DebugInfoTypeFlags::HasName);
    
    char* file_ptr = (char*)fileName;

    for (char* c = file_ptr; *c != 0; c++)
    {
        if (*c == '\\') file_ptr = c;
    }

    strlcpy(m_debugInfo->FileName, file_ptr, DEBUG_TOOLS_FILE_NAME_LENGTH);
    strlcpy(m_debugInfo->FunctionName, funcName, DEBUG_TOOLS_STEP_NAME_LENGTH);
    strlcpy(m_debugInfo->StepName, stepName, DEBUG_TOOLS_STEP_NAME_LENGTH);

    if ((m_debugInfo->TypeFlags & DebugInfoTypeFlags::IsTrace) != DebugInfoTypeFlags::IsTrace && strcmp(m_debugInfo->StepName, stepName))
    {
        m_debugInfo->Count++;
        m_debugInfo->TypeFlags = (DebugInfoTypeFlags)(m_debugInfo->TypeFlags | DebugInfoTypeFlags::HasCount);
        m_debugInfo->Line = line;
    }
    
}

bool DebugToolsClass::WasLastResetFromWatchdog()
{
    uint32_t status = (RSTC->RSTC_SR & RSTC_SR_RSTTYP_Msk) >> 8;
    return status == 0b010;
}

static void PrintIndent(Print& print, int indent)
{
    for (size_t i = 0; i < indent; i++)
    {
        print.print('\t');
    }
}

static void PrintTrace(Print& print, std::vector<DebugInfo>::iterator it, int indent, int count)
{
    auto v = *it;
    if (indent < count - 1)
    {
        PrintIndent(print, indent); print.print('<');
        v.Display(print, (DebugInfoDisplayFlags)(DebugInfoDisplayFlags::MentionFile | DebugInfoDisplayFlags::MentionCount | DebugInfoDisplayFlags::MentionFunc | DebugInfoDisplayFlags::MentionLine | DebugInfoDisplayFlags::MentionName));
        print.println('>');
        PrintTrace(print, ++it, indent + 1, count);
        PrintIndent(print, indent); print.print("</");
        v.Display(print, (DebugInfoDisplayFlags)(DebugInfoDisplayFlags::MentionFunc | DebugInfoDisplayFlags::MentionLine | DebugInfoDisplayFlags::MentionName));
        print.println('>');
    }
    else
    {
        PrintIndent(print, indent);
        v.Display(print, (DebugInfoDisplayFlags)(DebugInfoDisplayFlags::MentionFile | DebugInfoDisplayFlags::MentionCount | DebugInfoDisplayFlags::MentionFunc | DebugInfoDisplayFlags::MentionLine | DebugInfoDisplayFlags::MentionName));
        print.println();
    }
    
}

void DebugToolsClass::PrintStack()
{
    PrintTrace(*m_debugStream, m_callStack->begin(), 0, m_callStack->size());
    m_debugStream->println();
}

void DebugToolsClass::PrintDebugInfo(DebugInfoDisplayFlags displayFlags)
{
    m_debugInfo->Display(*m_debugStream, displayFlags);
    m_debugStream->println();
}

void DebugInfo::Display(Print& print, DebugInfoDisplayFlags displayFlags)
{
    print.print("Debug info { ");

    int flags = (DebugInfoDisplayFlags)(TypeFlags & displayFlags);
    if ((flags & DebugInfoDisplayFlags::MentionFile) == DebugInfoDisplayFlags::MentionFile)
    {
        print.print("File: '");
        print.print(FileName);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionLine) == DebugInfoDisplayFlags::MentionLine)
    {
        print.print("Line: '");
        print.print(Line);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionFunc) == DebugInfoDisplayFlags::MentionFunc)
    {
        print.print("Func: '");
        print.print(FunctionName);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionName) == DebugInfoDisplayFlags::MentionName)
    {
        print.print("Step name: '");
        print.print(StepName);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionCount) == DebugInfoDisplayFlags::MentionCount)
    {
        print.print("Count: '");
        print.print(Count);
        print.print("' ");
    }
    print.print("}");
}

DebugToolsClass::DebugToolsClass()
{
    m_debugInfo = (DebugInfo*)malloc(sizeof(DebugInfo));
    m_callStack = (Stack_t*)malloc(sizeof(Stack_t));
}

DebugToolsClass DebugTools;

