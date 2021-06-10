using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    public abstract class EditorViewModelBase : DependencyObject, INotifyPropertyChanged
    {
        public string PropertyName { get; set; }
        public abstract string DisplayName
        {
            get;
            set;
        }
        public abstract object ObjectValue { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
