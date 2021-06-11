using CameraRigController.FieldGrid.Editor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    [System.AttributeUsage(AttributeTargets.Class, Inherited = true, AllowMultiple = true)]
    sealed class FieldGridEditorAttribute : Attribute
    {
        public FieldGridEditorAttribute(Type editedObjectType, IDataTemplateProvider provider, params Type[] attributeTypes)
        {
            EditedObjectType = editedObjectType;
            Provider = provider;
            AttributeTypes = attributeTypes;
        }
        public FieldGridEditorAttribute(Type editedObjectType, string templateResourceName, params Type[] attributeTypes) :
            this(editedObjectType, new StringDataTemplateProvider(templateResourceName), attributeTypes) { }

        public Type EditedObjectType { get; }
        public Type[] AttributeTypes { get; }
        public IDataTemplateProvider Provider { get; }
    }
}
