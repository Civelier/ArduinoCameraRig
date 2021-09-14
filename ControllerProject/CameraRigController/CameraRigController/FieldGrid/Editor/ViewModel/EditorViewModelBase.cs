using GalaSoft.MvvmLight.CommandWpf;
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
        public abstract string DisplayName { get; set; }
        public abstract object ObjectValue { get; set; }

        public abstract bool IsSelected { get; set; }

        public Type InjectedType { get; set; }

        public IEnumerable<Attribute> PropertyAttributes { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;

        public void InitializeViewModel()
        {
            DisplayName = GetAttribute<DisplayNameAttribute>()?.DisplayName ?? FieldGridUtillities.NicifyName(PropertyName);
            AdditionnalInitialization();
        }

        protected virtual void AdditionnalInitialization()
        {
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            OnPropertyChanged(e.Property.Name);
        }

        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public T GetAttribute<T>() where T : Attribute
        {
            var f = PropertyAttributes.FirstOrDefault(a => a.GetType() == typeof(T));
            if (f == null) return null;
            return f as T;
        }

        public T[] GetAttributes<T>() where T : Attribute
        {
            var f = PropertyAttributes.Where(a => a.GetType() == typeof(T)).Select(a =>
                a as T);
            return f.ToArray();
        }

        public virtual void OnSelected()
        {
            IsSelected = true;
        }

        public virtual void OnUnselected()
        {
            IsSelected = false;
        }


    }   

    public abstract class EditorViewModelBase<TEditor> : EditorViewModelBase 
        where TEditor : EditorViewModelBase<TEditor>
    {


        public override bool IsSelected
        {
            get { return (bool)GetValue(IsSelectedProperty); }
            set { SetValue(IsSelectedProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsSelected.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsSelectedProperty =
            DependencyProperty.Register("IsSelected", typeof(bool), typeof(TEditor), new PropertyMetadata(false));



        public override string DisplayName
        {
            get { return (string)GetValue(DisplayNameProperty); }
            set { SetValue(DisplayNameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for DisplayName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DisplayNameProperty =
            DependencyProperty.Register("DisplayName", typeof(string),
                typeof(TEditor),
                new PropertyMetadata("Display name"));

        public override object ObjectValue
        {
            get { return GetValue(ObjectValueProperty); }
            set { SetValue(ObjectValueProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ObjectValueProperty =
            DependencyProperty.Register("ObjectValue", typeof(object), typeof(TEditor),
                new PropertyMetadata("Sample text"));
    }

    public abstract class EditorViewModelBase<TEditor, TValue> : EditorViewModelBase<TEditor> 
        where TEditor : EditorViewModelBase<TEditor, TValue>
    {
        public virtual TValue Value
        {
            get => (TValue)GetValue(ValueProperty);
            set => SetValue(ValueProperty, value);
        }

        // Using a DependencyProperty as the backing store for Value.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ValueProperty =
            DependencyProperty.Register("Value", typeof(TValue), typeof(TEditor),
                new PropertyMetadata(default(TValue)));

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            try
            {
                if (e.Property.Name == nameof(Value)) ObjectValue = e.NewValue;
                if (e.Property.Name == nameof(ObjectValue)) Value = (TValue)e.NewValue;
            }
            catch (InvalidCastException)
            {

            }
        }

        
    }
}
