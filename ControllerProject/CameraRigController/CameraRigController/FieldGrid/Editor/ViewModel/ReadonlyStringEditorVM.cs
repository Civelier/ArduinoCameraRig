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
    public class ReadonlyStringEditorVM : EditorViewModelBase
    {
        public override string DisplayName
        {
            get { return (string)GetValue(DisplayNameProperty); }
            set { SetValue(DisplayNameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for DisplayName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DisplayNameProperty =
            DependencyProperty.Register("DisplayName", typeof(string), 
                typeof(ReadonlyStringEditorVM), 
                new PropertyMetadata("Readonly string display name"));
        
        public string Value
        {
            get => ObjectValue.ToString();
        }

        public override object ObjectValue
        {
            get { return GetValue(ObjectValueProperty); }
            set { SetValue(ObjectValueProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ObjectValueProperty =
            DependencyProperty.Register("ObjectValue", typeof(object), 
                typeof(ReadonlyStringEditorVM), 
                new PropertyMetadata("Sample text", OnObjectValueChanged));
    }
}
