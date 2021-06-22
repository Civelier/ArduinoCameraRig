using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    [System.AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
    public sealed class SliderAttribute : Attribute
    {
        public SliderAttribute(object min, object max)
        {
            Min = min as IComparable;
            Max = max as IComparable;
        }

        public IComparable Min { get; }
        public IComparable Max { get; }
    }
}
