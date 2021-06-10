using CameraRigController.FieldGrid.Editor.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid.Editor
{
    public class StringEditorSelector : EditorSelectorBase
    {
        public StringEditorSelector(MemberInfo memberInfo) : base(memberInfo)
        {
        }

        public override string GetDataTemplateResourceName()
        {
            throw new NotImplementedException();

        }

        public override EditorViewModelBase GetViewModel()
        {
            throw new NotImplementedException();
        }
    }
}
