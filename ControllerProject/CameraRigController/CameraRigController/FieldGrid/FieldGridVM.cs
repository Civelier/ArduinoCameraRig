    using CameraRigController.FieldGrid.Editor.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
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
            DependencyProperty.Register("Target", typeof(object), typeof(FieldGridVM), new PropertyMetadata(null));



        public ObservableCollection<EditorViewModelBase> Fields
        {
            get { return (ObservableCollection<EditorViewModelBase>)GetValue(FieldsProperty); }
            set { SetValue(FieldsProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Fields.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FieldsProperty =
            DependencyProperty.Register("Fields", typeof(ObservableCollection<EditorViewModelBase>), typeof(FieldGridVM), new PropertyMetadata(null));

        public class SampleObject : INotifyPropertyChanged
        {
            private int _txtBoxInt;

            public int TextBoxInt
            {
                get => _txtBoxInt;
                set
                {
                    _txtBoxInt = value;
                    OnPropertyChanged(nameof(TextBoxInt));
                }
            }

            private string _readonlyString;

            public string ReadonlyString
            {
                get => _readonlyString;
                set
                {
                    _readonlyString = value;
                    OnPropertyChanged(nameof(ReadonlyString));
                }
            }

            private string _txtBoxString;

            public string TextBoxString
            {
                get => _txtBoxString;
                set
                {
                    _txtBoxString = value;
                    OnPropertyChanged(nameof(TextBoxString));
                }
            }

            public enum SampleEnum
            {
                Value1,
                Something,
                SomethingElse,
            }

            private SampleEnum _enumValue;

            public SampleEnum EnumValue
            {
                get => _enumValue;
                set
                {
                    _enumValue = value;
                    OnPropertyChanged(nameof(EnumValue));
                }
            }


            public event PropertyChangedEventHandler PropertyChanged;

            private void OnPropertyChanged(string propertyName)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public FieldGridVM()
        {
            if (GalaSoft.MvvmLight.ViewModelBase.IsInDesignModeStatic)
            {
                var sample = new SampleObject();
                Target = sample;
            }
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            if (e.Property != TargetProperty) return;
            if (e.NewValue != null) SetValue(FieldsProperty, FieldGridUtillities.ToVMCollection(e.NewValue));
        }

    }
}
