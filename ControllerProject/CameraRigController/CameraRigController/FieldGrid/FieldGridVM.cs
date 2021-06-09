using CameraRigController.FieldGrid.Editor.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid
{
    public class FieldGridVM : DependencyObject
    {


        public ObservableCollection<EditorViewModelBase> Fields
        {
            get { return (ObservableCollection<EditorViewModelBase>)GetValue(FieldsProperty); }
            set { SetValue(FieldsProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Fields.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FieldsProperty =
            DependencyProperty.Register("Fields", typeof(ObservableCollection<EditorViewModelBase>), typeof(FieldGridVM), new PropertyMetadata(null));

        public FieldGridVM()
        {
            Fields = new ObservableCollection<EditorViewModelBase>();
            Fields.Add(new StringEditorVM() { DisplayName = "String1", Value = "Hello world" });
        }
    }
}
