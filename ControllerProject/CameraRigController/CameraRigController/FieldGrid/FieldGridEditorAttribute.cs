using CameraRigController.FieldGrid.Editor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    [System.AttributeUsage(AttributeTargets.Class, Inherited = false, AllowMultiple = true)]
    sealed class FieldGridEditorAttribute : Attribute
    {
        public FieldGridEditorAttribute(Type editedObjectType, params Type[] attributeTypes)
        {
            EditedObjectType = editedObjectType;
            AttributeTypes = attributeTypes;
        }

        public Type EditedObjectType { get; }
        public Type[] AttributeTypes { get; }
    }
}
