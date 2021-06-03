using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public readonly struct RawKeyframe
    {
        public uint Frame { get; }
        public double Value { get; }
        public RawKeyframe(uint frame, double value)
        {
            Frame = frame;
            Value = value;
        }
    }
}
