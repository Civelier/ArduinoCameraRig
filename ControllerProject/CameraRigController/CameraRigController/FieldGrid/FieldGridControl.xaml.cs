using CameraRigController.FieldGrid.Editor;
using CameraRigController.FieldGrid.Editor.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace CameraRigController.FieldGrid
{






    /// <summary>
    /// Interaction logic for FieldGridControl.xaml
    /// </summary>
    public partial class FieldGridControl : UserControl
    {
        public ObservableCollection<EditorViewModelBase> Fields
        {
            get { return (ObservableCollection<EditorViewModelBase>)GetValue(FieldsProperty); }
            set { SetValue(FieldsProperty, value); }
        }

        // Using a DependencyProperty as the backing store for editorViewModelBases.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FieldsProperty =
            DependencyProperty.Register("Fields",
                typeof(ObservableCollection<EditorViewModelBase>), 
                typeof(FieldGridControl),
                new PropertyMetadata(new ObservableCollection<EditorViewModelBase>(), FieldsChangedCallback));

        public FieldGridControl()
        {
            Resources["FieldTemplateSelector"] = new FieldGridTemplateSelector(this);
            InitializeComponent();
        }

        static void FieldsChangedCallback(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        {
            if (args.NewValue != null)
            {
                var fg = sender as FieldGridControl;
                fg.PropertyListBox.ItemsSource = fg.Fields;
            }
        }
    }
}
