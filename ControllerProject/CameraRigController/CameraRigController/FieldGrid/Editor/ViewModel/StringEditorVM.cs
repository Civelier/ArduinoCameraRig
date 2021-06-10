using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    [FieldGridEditor(typeof(string))]
    public class StringEditorVM : EditorViewModelBase
    {
        public override string DisplayName
        {
            get { return (string)GetValue(DisplayNameProperty); }
            set { SetValue(DisplayNameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for DisplayName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DisplayNameProperty =
            DependencyProperty.Register("DisplayName", typeof(string), typeof(StringEditorVM), new PropertyMetadata("String display name"));



        public string Value
        {
            get => (string)ObjectValue;
            set => ObjectValue = value;
        }

        public override object ObjectValue 
        {
            get { return GetValue(ObjectValueProperty); }
            set 
            { 
                SetValue(ObjectValueProperty, value);
                OnPropertyChanged(nameof(ObjectValue));
            }
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ObjectValueProperty =
            DependencyProperty.Register("Value", typeof(object), typeof(StringEditorVM), new PropertyMetadata("Sample text"));


    }
}
