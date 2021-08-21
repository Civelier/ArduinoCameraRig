/*
 Name:		MultiStepper.cpp
 Created:	5/27/2021 6:43:09 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#include "MultiStepper.h"

MultiStepperClass::MultiStepperClass()
{
	//m_drivers = (StepperDriver**)malloc(MSTEP_MAX_COUNT * sizeof(StepperDriver*));
	m_count = 0;
	for (size_t i = 0; i < MSTEP_MAX_COUNT; i++)
	{
		m_drivers[i] = nullptr;
	}
}

void MultiStepperClass::AttachCallback(instructionCB_t callback)
{
	m_callback = callback;
}

void MultiStepperClass::AttachDriver(StepperDriver* driver)
{
#ifdef MSTEP_DEBUG
	MSTEP_DEBUG_STREAM.print("Attaching Driver: ");
	MSTEP_DEBUG_STREAM.print(this->m_count);
	MSTEP_DEBUG_STREAM.println("...\t");
	MSTEP_DEBUG_STREAM.flush();
#endif
	this->m_drivers[m_count] = driver;
#ifdef MSTEP_DEBUG
	MSTEP_DEBUG_STREAM.println("Attached driver");
	MSTEP_DEBUG_STREAM.flush();
#endif


	pinMode(driver->Dir, OUTPUT);
	pinMode(driver->Step, OUTPUT);
	if (driver->MS1 != 255) pinMode(driver->MS1, OUTPUT);
	if (driver->MS2 != 255) pinMode(driver->MS2, OUTPUT);
	if (driver->MS3 != 255) pinMode(driver->MS3, OUTPUT);

#ifdef MSTEP_DEBUG
	MSTEP_DEBUG_STREAM.println("Initialized");
	MSTEP_DEBUG_STREAM.flush();
#endif

	this->m_count++;
}

void MultiStepperClass::UpdateDrivers()
{
#ifdef MSTEP_DEBUG
	static int hit = 0;
	static uint32_t nextWakeup = 0;
	uint32_t refresh = 2000;
	hit++;
	if (millis() > nextWakeup)
	{
		nextWakeup = millis() + refresh;
		MSTEP_DEBUG_STREAM.print("Updated '");
		MSTEP_DEBUG_STREAM.print(hit);
		MSTEP_DEBUG_STREAM.print("' times in the last '");
		MSTEP_DEBUG_STREAM.print(refresh);
		MSTEP_DEBUG_STREAM.println("' milliseconds!");
		hit = 0;
		MSTEP_DEBUG_STREAM.flush();
	}
#endif // MSTEP_DEBUG

	for (size_t i = 0; i < m_count; i++)
	{
		if (m_drivers[i] == nullptr)
		{
#ifndef MSTEP_DEBUG
			MSTEP_DEBUG_STREAM.println(4);
#endif
			MSTEP_DEBUG_STREAM.print("-Error: Driver '");
			MSTEP_DEBUG_STREAM.print(i);
			MSTEP_DEBUG_STREAM.println("' was nullptr");
			MSTEP_DEBUG_STREAM.flush();
			return;
		}
		if (m_drivers[i]->Instruction != nullptr)
		{
#ifdef MSTEP_DEBUG
			static bool status[MSTEP_MAX_COUNT]{};
			if (!status[i])
			{
				MSTEP_DEBUG_STREAM.print("Driver '");
				MSTEP_DEBUG_STREAM.print(i);
				MSTEP_DEBUG_STREAM.println("' has an instruction!");
				MSTEP_DEBUG_STREAM.flush();
				status[i] = 1;
			}
#endif
			auto result = m_drivers[i]->Instruction->Execute(m_drivers[i]);
			if (result == DriverInstructionResult::Done) m_drivers[i]->SetInstruction(nullptr);
			if (m_callback != nullptr)
			{
				m_callback(i, result);
#ifdef MSTEP_DEBUG
				MSTEP_DEBUG_STREAM.print("Driver '");
				MSTEP_DEBUG_STREAM.print(i);
				MSTEP_DEBUG_STREAM.println("' completed an instruction!");
				MSTEP_DEBUG_STREAM.flush();
				status[i] = 0;
#endif // MSTEP_DEBUG

			}
		}
	}
}

size_t MultiStepperClass::Count()
{
	return m_count;
}


MultiStepperClass MStep;