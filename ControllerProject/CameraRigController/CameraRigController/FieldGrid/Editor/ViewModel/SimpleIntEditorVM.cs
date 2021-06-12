using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    [FieldGridEditor(typeof(int))]
    [FieldgridTemplate("StringTemplate")]
    public class SimpleIntEditorVM : EditorViewModelBase
    {
        public override string DisplayName
        {
            get { return (string)GetValue(DisplayNameProperty); }
            set { SetValue(DisplayNameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for DisplayName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DisplayNameProperty =
            DependencyProperty.Register("DisplayName", typeof(string),
                typeof(SimpleIntEditorVM),
                new PropertyMetadata("String display name"));

        public string Value
        {
            get => (string)GetValue(ValueProperty);
            set => SetValue(ValueProperty, value);
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ValueProperty =
            DependencyProperty.Register("Value", typeof(object), typeof(SimpleIntEditorVM),
                new PropertyMetadata("0", OnValueChanged));

        static void OnValueChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        {
            var obj = sender as SimpleIntEditorVM;
            if (int.TryParse((string)args.NewValue, out int result))
            {
                obj.ObjectValue = result;
            }
            else obj.Value = (string)args.OldValue;
        }

        public override object ObjectValue
        {
            get { return GetValue(ObjectValueProperty); }
            set { SetValue(ObjectValueProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ObjectValueProperty =
            DependencyProperty.Register("ObjectValue", typeof(object), typeof(SimpleIntEditorVM),
                new PropertyMetadata("Sample text", OnObjectValueChanged));
    }
}
