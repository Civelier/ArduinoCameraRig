using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid.Editor.ViewModel.Numerics
{
    public interface INumber<T> : IComparable, IFormattable, IConvertible, IComparable<T>, IEquatable<T>, IComparable<INumber<T>>, IEquatable<INumber<T>> where T : struct
    {
        T Value { get; set; }
    }
}
