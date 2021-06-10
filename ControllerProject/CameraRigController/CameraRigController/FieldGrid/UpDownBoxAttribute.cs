using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    [System.AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
    sealed class UpDownBoxAttribute : Attribute
    {
        public UpDownBoxAttribute(double min, double max, int decimals, double increment)
        {
            Min = min;
            Max = max;
            Decimals = decimals;
            Increment = increment;
        }

        public double Min { get; }
        public double Max { get; }
        public int Decimals { get; }
        public double Increment { get; }
    }
}
