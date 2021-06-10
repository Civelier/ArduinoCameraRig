using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    [System.AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
    sealed class SliderAttribute : Attribute
    {
        public SliderAttribute(double min, double max)
        {
            Min = min;
            Max = max;
        }

        public double Min { get; }
        public double Max { get; }
    }
}
