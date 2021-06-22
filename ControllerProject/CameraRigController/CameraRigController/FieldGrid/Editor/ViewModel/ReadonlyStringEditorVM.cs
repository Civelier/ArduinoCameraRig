using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    //[FieldGridEditor(typeof(int), typeof(ReadOnlyAttribute))]
    [FieldGridEditor(typeof(object), typeof(ReadOnlyAttribute))]
    [FieldgridTemplate("ReadonlyStringTemplate")]
    public class ReadonlyStringEditorVM : EditorViewModelBase<ReadonlyStringEditorVM>
    {
        public object Value
        {
            get => GetValue(ValueProperty).ToString();
            set => SetValue(ValueProperty, value.ToString());
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ValueProperty =
            DependencyProperty.Register("Value", typeof(string), typeof(ReadonlyStringEditorVM),
                new PropertyMetadata(""));
    }
}
