using System;

namespace CameraRigController
{
    public readonly struct Keyframe
    {
        public UInt32 MS { get; }
        public Int32 Value { get; }

        public Keyframe(UInt32 ms, Int32 value)
        {
            MS = ms;
            Value = value;
        }
    }
}