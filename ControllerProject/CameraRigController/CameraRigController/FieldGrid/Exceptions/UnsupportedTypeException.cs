using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid.Exceptions
{
    public class UnsupportedTypeException : Exception
    {
        public UnsupportedTypeException(Type unsupportedType) : base($"Type \'{unsupportedType.Name}\' is not supported by the fieldgrid")
        {
        }
    }
}
