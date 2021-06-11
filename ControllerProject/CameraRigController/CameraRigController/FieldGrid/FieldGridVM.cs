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
        public object Target
        {
            get { return (object)GetValue(TargetProperty); }
            set { SetValue(TargetProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Target.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty TargetProperty =
            DependencyProperty.Register("Target", typeof(object), typeof(FieldGridVM), new PropertyMetadata(null, TargetChangedCallback));



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
        }

        static void TargetChangedCallback(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        {
            if (args.NewValue != null) sender.SetValue(FieldsProperty, FieldGridUtillities.ToVMCollection(args.NewValue));
        }
    }
}
