using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid.Editor
{
    public class Any
    {
        private readonly Type type;

        internal Any(Type type)
        {
            this.type = type;
        }
        public Type GetInnerType() => type;
    }

    public static class AnyExtensions
    {
        public static Any ToAny<T>(this T obj)
        {
            return new Any(typeof(T));
        }
    }
}
