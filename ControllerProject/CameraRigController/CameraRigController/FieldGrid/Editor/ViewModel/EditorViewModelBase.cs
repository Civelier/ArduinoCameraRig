using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    public abstract class EditorViewModelBase : DependencyObject
    {
        public abstract string DisplayName
        {
            get;
            set;
        }
    }
}
