using CameraRigController.FieldGrid.Editor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    [System.AttributeUsage(AttributeTargets.Class, Inherited = false, AllowMultiple = false)]
    sealed class FieldgridTemplateAttribute : Attribute
    {
        public FieldgridTemplateAttribute(IDataTemplateProvider provider)
        {
            Provider = provider;
        }

        public FieldgridTemplateAttribute(string resourceTemplateName) : 
            this(new StringDataTemplateProvider(resourceTemplateName))
        { }

        public IDataTemplateProvider Provider { get; }
    }
}
