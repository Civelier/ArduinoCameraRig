using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{

    public class PropertyChangingCancelEventArgs : CancelEventArgs
    {

        public PropertyChangingCancelEventArgs(string propertyName) : base()
        {
            PropertyName = propertyName;
        }

        public string PropertyName { get; }
    }

    public delegate void PropertyChangingCancelEventHandler(object sender, PropertyChangingCancelEventArgs args);

    public interface INotifyPropertyChangingCancel
    {
        event PropertyChangingCancelEventHandler PropertyChangingCancel;
    }
}
