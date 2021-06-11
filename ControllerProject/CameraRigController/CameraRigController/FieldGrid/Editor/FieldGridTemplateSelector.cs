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
    public interface IDataTemplateProvider
    {
        DataTemplate GetDataTemplate(UserControl control);
    }

    public struct DataTemplateProvider : IDataTemplateProvider
    {
        private readonly DataTemplate template;

        public DataTemplateProvider(DataTemplate template)
        {
            this.template = template;
        }
        public DataTemplate GetDataTemplate(UserControl control)
        {
            return template;
        }
    }

    public struct StringDataTemplateProvider : IDataTemplateProvider
    {
        private readonly string resource;

        public StringDataTemplateProvider(string resource)
        {
            this.resource = resource;
        }
        public DataTemplate GetDataTemplate(UserControl control)
        {
            return (DataTemplate)control.Resources[resource];
        }
    }

    public class FieldGridTemplateSelector : DataTemplateSelector
    {
        private static Dictionary<Type, IDataTemplateProvider> _editorResourceDictionary = 
            new Dictionary<Type, IDataTemplateProvider>();
        public FieldGridTemplateSelector(UserControl control)
        {
            Control = control;
        }

        public UserControl Control { get; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (item == null) return null;
            if (_editorResourceDictionary.TryGetValue(item.GetType(), out IDataTemplateProvider provider))
            {
                return provider.GetDataTemplate(Control);
            }
            return null;
        }

        public static void RegisterEditorResource(Type editorType, IDataTemplateProvider provider)
        {
            _editorResourceDictionary.Add(editorType, provider);
        }

        public static void RegisterEditorResource(Type editorType, string resourceName)
        {
            _editorResourceDictionary.Add(editorType, new StringDataTemplateProvider(resourceName));
        }
    }
}
