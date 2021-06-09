using CameraRigController.FieldGrid.Editor.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace CameraRigController.FieldGrid.Editor
{
    public class FieldGridTemplateSelector : DataTemplateSelector
    {
        public FieldGridTemplateSelector(UserControl control)
        {
            Control = control;
        }

        public UserControl Control { get; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (item == null) return null;
            if (item is StringEditorVM)
            {
                return Control.Resources["StringTemplate"] as DataTemplate;
            }
            return null;
        }
    }
}
