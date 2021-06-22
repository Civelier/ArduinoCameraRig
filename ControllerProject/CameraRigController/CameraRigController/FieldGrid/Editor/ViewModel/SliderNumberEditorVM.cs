using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using GalaSoft.MvvmLight.CommandWpf;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    public abstract class SliderNumberEditorVM<TEditor, TValue> : 
        EditorViewModelBase<TEditor, TValue> 
        where TEditor : SliderNumberEditorVM<TEditor, TValue>
        where TValue : struct, IComparable, IComparable<TValue>
    {

        public TValue Minimum
        {
            get { return (TValue)GetValue(MinimumProperty); }
            set { SetValue(MinimumProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Minimum.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty MinimumProperty =
            DependencyProperty.Register("Minimum", typeof(TValue), 
                typeof(TEditor), new PropertyMetadata(0));


        public TValue Maximum
        {
            get { return (TValue)GetValue(MaximumProperty); }
            set { SetValue(MaximumProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Maximum.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty MaximumProperty =
            DependencyProperty.Register("Maximum", typeof(TValue), 
                typeof(TEditor), new PropertyMetadata(0));

        protected override void AdditionnalInitialization()
        {
            var slider = GetAttribute<SliderAttribute>();
            if (slider == null) return;
            Minimum = (TValue)slider.Min;
            Maximum = (TValue)slider.Max;
        }
    }
}
