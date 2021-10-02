#ifndef _KeyframeInstruction_h
#define _KeyframeInstruction_h

#include "Keyframe.h"
#include "IDriverInstruction.h"
#include "TimeSync.h"
#include "SpeedBuffer.h"


class KeyframeDriverInstruction : public IDriverInstruction
{
private:
    uint32_t nextPulse = 0;
    uint32_t pulseDelay = 0;
    int32_t steps = 0;
    MicroStep lastStepSize = MicroStep::MSStep;
    bool dir = false;
    Keyframe m_start = Keyframe();
    Keyframe m_end = Keyframe();
    TimeSync* m_sync = nullptr;
    bool m_firstRun = true;
    SpeedBuffer m_buffer = SpeedBuffer(10);
    curve_t m_curve = nullptr;
    int32_t stepsLeft = 0;
    int32_t lastStep = 0;
private:
    int8_t Repeat(int8_t step, int8_t length);
public:
    KeyframeDriverInstruction(TimeSync* sync, Keyframe start, Keyframe end, curve_t curve);
    DriverInstructionResult Execute(StepperDriver* driver) override;
    size_t Size() const override;
    virtual ~KeyframeDriverInstruction();
};

#endif // !_KeyframeInstruction_h

