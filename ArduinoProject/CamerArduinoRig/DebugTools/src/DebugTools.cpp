/*
 Name:		DebugTools.cpp
 Created:	8/25/2021 12:24:24 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#include "DebugTools.h"
#include "watchdog.h"

void watchdogSetup(void) {/*** watchdogDisable (); ***/ }

const size_t MaxMem = 98304;

void DebugToolsClass::SetDebugOutput(Stream* output)
{
	m_debugStream = output;
}

Stream* DebugToolsClass::GetDebugOutput()
{
    return m_debugStream;
}   

typedef void (*func_t)();

func_t ClearMemFunc;

void ClearMemory()
{
    uint32_t* ptr = nullptr;

    for (ptr += 1; ptr < (uint32_t*)(MaxMem / 4); ptr++)
    {
        if (ptr != (uint32_t*)ClearMemFunc && ptr != (uint32_t*)&ptr && ptr != (uint32_t*)MaxMem && ptr != (uint32_t*)&ClearMemFunc)
        {
            *ptr = 0xFFFFFFFF;
        }
    }
}

void DebugToolsClass::CleanMemory()
{
    atexit(ClearMemory);
    m_debugStream->println("Wiping memory!");
    m_debugStream->flush();
    delay(100);
    exit(0);
}

void DebugToolsClass::NotifyMemoryAllocation(int size)
{
    m_allocated += size;
}

void DebugToolsClass::NotifyMemoryFree(int size)
{
    m_allocated -= size;
}

int DebugToolsClass::GetAllocatedMemory()
{
    return m_allocated;
}

void DebugToolsClass::PrintCurrentlyAllocatedMemory()
{
    m_debugStream->print("Memory: ");
    m_debugStream->print(m_allocated);
    m_debugStream->println(" bytes");
    m_debugStream->flush();
}

size_t DebugToolsClass::GetFreeMem()
{
    size_t mem = 0;
    for (uint8_t* ptr = (uint8_t*)1; ptr < (uint8_t*)MaxMem; ++ptr)
        if (*ptr == 0xFF) ++mem;
    return mem;
}

void DebugToolsClass::FunctionEnter(uint16_t line, const char* fileName, const char* funcName)
{
    DebugInfo debugInfo{ (DebugInfoTypeFlags)(DebugInfoTypeFlags::HasFunc | DebugInfoTypeFlags::IsTrace) };
    char* file_ptr = (char*)fileName;
    debugInfo.Line = { line };
    debugInfo.MemLeft = GetAllocatedMemory();

    for (char* c = file_ptr; *c != 0; c++)
    {
        if (*c == '\\') file_ptr = c;
    }

    strlcpy(debugInfo.FileName, file_ptr, DEBUG_TOOLS_FILE_NAME_LENGTH);
    strlcpy(debugInfo.FunctionName, funcName, DEBUG_TOOLS_STEP_NAME_LENGTH);
    *m_debugInfo = DebugInfo(debugInfo);
    m_callStack->push_back(debugInfo);
}

void DebugToolsClass::FunctionExit(int memLeft, bool suppressWarning, int memAllowed)
{
    auto endMem = GetAllocatedMemory();
    auto delta = endMem - memLeft; // + means allocated, - means freed
    int allowed = delta - memAllowed;
    if (!suppressWarning && allowed)
    {
        auto back = m_callStack->operator[](m_callStack->size() != 0 ? m_callStack->size() - 1 : 0);
        if (allowed > 0) // Less mem free now than before - allocated memory
        {
            m_debugStream->println(4);
            m_debugStream->print("[Warning] Possible memory leak in '");
            m_debugStream->print(back.FunctionName);
            m_debugStream->print("', bytes leaked: ");
            m_debugStream->print(allowed);
            m_debugStream->print(" over leak budget of: ");
            m_debugStream->println(memAllowed);
            m_debugStream->flush();
        }
        if (allowed < 0) // More mem free than before - freed memory
        {
            m_debugStream->println(4);
            m_debugStream->print("[Warning] Possible unwanted freeing of memory in '");
            m_debugStream->print(back.FunctionName);
            m_debugStream->print("', bytes freed: ");
            m_debugStream->print(-allowed);
            m_debugStream->print(" over freeing budget of: ");
            m_debugStream->println(-memAllowed);
            m_debugStream->flush();
        }
    }
    m_callStack->pop_back();
    *m_debugInfo = m_callStack->back();
}

void DebugToolsClass::SetupWatchdog(uint32_t ms)
{
    watchdogEnable(ms);
    //m_array = new Alloc_t(DEBUG_TOOLS_STACK_LENGTH, {});
    m_callStack = new Stack_t(1);
}

void DebugToolsClass::ResetWatchdog(uint16_t line, const char* fileName, const char* funcName, const char* stepName)
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
    m_callStack->back().MemLeft = GetAllocatedMemory();
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
    m_debugStream->flush();
}


void DebugToolsClass::PrintDebugInfo(DebugInfoDisplayFlags displayFlags)
{
    m_debugInfo->Display(*m_debugStream, displayFlags);
    m_debugStream->println();
    m_debugStream->flush();
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

    if ((flags & DebugInfoDisplayFlags::MentionMemory) == DebugInfoDisplayFlags::MentionMemory)
    {
        print.print("Memory: '");
        print.print(MemLeft);
        print.print("' ");
    }

    print.print("}");
}

DebugToolsClass::DebugToolsClass()
{
    //watchdogDisable();
    m_debugInfo = (DebugInfo*)malloc(sizeof(DebugInfo));
    //NotifyMemoryAllocation(sizeof(DebugInfo));
    //m_callStack = (Stack_t*)malloc(sizeof(Stack_t));
    //NotifyMemoryAllocation(sizeof(Stack_t));
}

DebugToolsClass DebugTools;

