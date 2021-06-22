using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    [FieldGridEditor(typeof(Enum))]
    [FieldgridTemplate("EnumTemplate")]
    public class EnumEditorVM : EditorViewModelBase<EnumEditorVM, Enum>
    {


        public ObservableCollection<string> Values
        {
            get { return (ObservableCollection<string>)GetValue(ValuesProperty); }
            set { SetValue(ValuesProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Values.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ValuesProperty =
            DependencyProperty.Register("Values", typeof(ObservableCollection<string>), typeof(EnumEditorVM), new PropertyMetadata(new ObservableCollection<string>()));



        protected override void AdditionnalInitialization()
        {
            base.AdditionnalInitialization();
            Values = new ObservableCollection<string>(Enum.GetNames(InjectedType));
        }
    }
}
