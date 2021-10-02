#include "KeyframeInstruction.h"

int8_t KeyframeDriverInstruction::Repeat(int8_t step, int8_t length)
{
    return step < 0 ? length + step : step % length;
}

KeyframeDriverInstruction::KeyframeDriverInstruction(TimeSync* sync, Keyframe start, Keyframe end, curve_t curve)
{
	m_curve = curve;
    m_sync = sync;
    m_start = start;
    m_end = end;
    float ms = end.MS - start.MS;
    //DebugValue(ms);
    int32_t stps = end.Steps - start.Steps;
    this->dir = stps > 0;
	this->steps = this->stepsLeft = abs(stps);
    //DebugValue(steps);
    /*Serial.println(4);
    Serial.print("Stps:");
    Serial.println(stps);
    Serial.println(4);
    Serial.print("Steps:");
    Serial.println(steps);
    Serial.println(4);
    Serial.print("Dir:");
    Serial.println(dir);*/
    /*Serial.println(4);
    Serial.print("ms / 1000:");
    Serial.println(ms / 1000.0f);*/

    /*Serial.println(4);
    Serial.println((1000000.0f / ((float)steps * (ms / 1000.0f))));*/
    if (steps != 0) pulseDelay = (uint32_t)(ms / (float)steps * 1000.0f);
    this->nextPulse = m_start.MS * 1000L + pulseDelay;
    m_firstRun = true;
}

//DriverInstructionResult KeyframeDriverInstruction::Execute(StepperDriver* driver)
//{
//    if (pulseDelay == 0) pulseDelay = 1;
//
//    /*Serial.print("New instruction: delay: ");
//    Serial.println(pulseDelay);*/
//    if (!m_sync->Started)
//    {
//        m_sync->Start();
//        /*Serial.println(4);
//        Serial.print("Sync time: ");
//        Serial.print(m_sync->CurrentMicros());
//        Serial.println("us");*/
//    }
//    /*if (m_firstRun)
//    {
//        Serial.println(4);
//        Serial.print("PulseDelay: ");
//        Serial.println(pulseDelay);
//        Serial.println(4);
//        Serial.print("Steps: ");
//        Serial.println(steps);
//        m_firstRun = false;
//    }*/
//    uint32_t now = m_sync->CurrentMicros();
//    // move only if the appropriate delay has passed:
//    if (steps > 0 && now >= nextPulse)
//    {
//        /*if (this->dir == 1)
//        {
//            thisStep++;
//            if (thisStep == driver->NumberOfSteps) {
//                thisStep = 0;
//            }
//        }
//        else
//        {
//            if (thisStep == 0) {
//                thisStep = driver->NumberOfSteps;
//            }
//            thisStep--;
//        }*/
//        if (driver->StepCompatibility == StepType::Sixteenth)
//        {
//            driver->thisStep = dir ? driver->thisStep - 1 : driver->thisStep + 1;
//            driver->thisStep = Repeat(driver->thisStep, 10);
//        }
//        else driver->thisStep %= 4;
//        nextPulse += pulseDelay;
//        /*if (driver->MS1 != NOT_A_PIN) digitalWrite(driver->MS1, bitRead(driver->StepCompatibility, 0));
//        if (driver->MS2 != NOT_A_PIN) digitalWrite(driver->MS2, bitRead(driver->StepCompatibility, 1));
//        if (driver->MS3 != NOT_A_PIN) digitalWrite(driver->MS3, bitRead(driver->StepCompatibility, 2));*/
//        if (driver->StepCompatibility == StepType::Step)
//        {
//            switch (driver->thisStep) {
//            case 0:  // 01
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->Dir, HIGH);
//                break;
//            case 1:  // 11
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->Dir, HIGH);
//                break;
//            case 2:  // 10
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->Dir, LOW);
//                break;
//            case 3:  // 00
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->Dir, LOW);
//                break;
//            }
//        }
//        if (driver->StepCompatibility == StepType::Sixteenth)
//        {
//            switch (driver->thisStep) {
//            case 0:  // 01101
//                digitalWrite(driver->Dir, LOW);
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->MS1, HIGH);
//                digitalWrite(driver->MS2, LOW);
//                digitalWrite(driver->MS3, HIGH);
//                break;
//            case 1:  // 01001
//                digitalWrite(driver->Dir, LOW);
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->MS1, LOW);
//                digitalWrite(driver->MS2, LOW);
//                digitalWrite(driver->MS3, HIGH);
//                break;
//            case 2:  // 01011
//                digitalWrite(driver->Dir, LOW);
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->MS1, LOW);
//                digitalWrite(driver->MS2, HIGH);
//                digitalWrite(driver->MS3, HIGH);
//                break;
//            case 3:  // 01010
//                digitalWrite(driver->Dir, LOW);
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->MS1, LOW);
//                digitalWrite(driver->MS2, HIGH);
//                digitalWrite(driver->MS3, LOW);
//                break;
//            case 4:  // 11010
//                digitalWrite(driver->Dir, HIGH);
//                digitalWrite(driver->Step, HIGH);
//                digitalWrite(driver->MS1, LOW);
//                digitalWrite(driver->MS2, HIGH);
//                digitalWrite(driver->MS3, LOW);
//                break;
//            case 5:  // 10010
//                digitalWrite(driver->Dir, HIGH);
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->MS1, LOW);
//                digitalWrite(driver->MS2, HIGH);
//                digitalWrite(driver->MS3, LOW);
//                break;
//            case 6:  // 10110
//                digitalWrite(driver->Dir, HIGH);
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->MS1, HIGH);
//                digitalWrite(driver->MS2, HIGH);
//                digitalWrite(driver->MS3, LOW);
//                break;
//            case 7:  // 10100
//                digitalWrite(driver->Dir, HIGH);
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->MS1, HIGH);
//                digitalWrite(driver->MS2, LOW);
//                digitalWrite(driver->MS3, LOW);
//                break;
//            case 8:  // 10101
//                digitalWrite(driver->Dir, HIGH);
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->MS1, HIGH);
//                digitalWrite(driver->MS2, LOW);
//                digitalWrite(driver->MS3, HIGH);
//                break;
//            case 9:  // 00101
//                digitalWrite(driver->Dir, LOW);
//                digitalWrite(driver->Step, LOW);
//                digitalWrite(driver->MS1, HIGH);
//                digitalWrite(driver->MS2, LOW);
//                digitalWrite(driver->MS3, HIGH);
//                break;
//            }
//        }
//        //thisStep = (thisStep + 1) % 10;
//        steps--;
//    }
//    if (m_end.MS * 1000L <= now)
//    {
//        return DriverInstructionResult::Done;
//    }
//    return DriverInstructionResult::Success;
//}


DriverInstructionResult KeyframeDriverInstruction::Execute(StepperDriver* driver)
{
    if (!m_sync->Started)
    {
        m_sync->Start();
        /*Serial.println(4);
        Serial.print("Sync time: ");
        Serial.print(m_sync->CurrentMicros());
        Serial.println("us");*/
    }
	

    //if (now < m_start.MS) return DriverInstructionResult::Success;

    if (m_firstRun)
    {
        m_firstRun = false;
        digitalWrite(driver->Dir, dir);
        
        //DBGValue(steps);
    }

    uint32_t now = m_sync->CurrentMicros();
    if (now < m_start.MS) return DriverInstructionResult::Success;
	if (stepsLeft > 0)
	{
        float startUS = now < m_start.MS * 1000.0f ? 0 : (float)(now - m_start.MS * 1000.0f);

		int32_t step = steps * m_curve(startUS / (float)((m_end.MS - m_start.MS) * 1000.0f));
        int32_t deltaStep = step - lastStep;
		if (step > (steps - stepsLeft))
		{
            /*DBGValue(now);
            DBGValue(m_start.MS);
            DBGValue(startMS);
            DBGValue(startMS / (float)((m_end.MS - m_start.MS) * 1000.0f));
            DBGValue(step);*/
			m_buffer.Write(now);
            float sps = (1.0f / m_buffer.AverageSpeed()) * 1000000.0f;// *(float)deltaStep;
            //DBGValue(sps);
			MicroStep ms = driver->LogDistribution.MicroStepForSpeed(sps);

            //if (ms != lastStepSize)
            //{
            //    MSTEP_DEBUG_STREAM.println(4);
            //    MSTEP_DEBUG_STREAM.print("Channel: ");
            //    MSTEP_DEBUG_STREAM.print(m_start.ChannelID);
            //    MSTEP_DEBUG_STREAM.print(" channged stepping to: ");
            //    switch (ms)
            //    {
            //    case MicroStep::MSStep:
            //        MSTEP_DEBUG_STREAM.println("Full step");
            //        break;
            //    case MicroStep::MSHalf:
            //        MSTEP_DEBUG_STREAM.println("Half step");
            //        break;
            //    case MicroStep::MSQuarter:
            //        MSTEP_DEBUG_STREAM.println("Quarter step");
            //        break;
            //    case MicroStep::MSEighth:
            //        MSTEP_DEBUG_STREAM.println("Eighth step");
            //        break;
            //    case MicroStep::MSSixteenth:
            //        MSTEP_DEBUG_STREAM.println("Sixteenth step");
            //        break;
            //    default:
            //        break;
            //    }

            //    // Stepping up to the next step size
            //    
            //    /*
            //    * From 1/16 to 1/1 step
            //    * Microstep 14 + 1/8 step
            //    * 
            //    */

            //    const int v1 = 0b10000;

            //    int32_t stepUp{ stepsLeft % 16 }; // How many microsteps 


            //    lastStepSize = ms;
            //}



            //DBGValue(ms);
            //DBGValue(stepsLeft);
			/*if (ms != lastStep)
			{
				lastStep = ms;
				Serial.print("Changed step: ");
				Serial.println(StepFactor(ms));
			}*/
			driver->SetMicroStepPins(ms);
			digitalWrite(driver->Step, HIGH);
			delayMicroseconds(1);
			digitalWrite(driver->Step, LOW);
            delayMicroseconds(1);
			stepsLeft -= StepFactor(ms);
            lastStep = step;
		}
	}
    if (m_end.MS * 1000L <= now)
    {
        return DriverInstructionResult::Done;
    }
    return DriverInstructionResult::Success;
}

size_t KeyframeDriverInstruction::Size() const
{
    return sizeof(KeyframeDriverInstruction);
}

KeyframeDriverInstruction::~KeyframeDriverInstruction()
{
}
