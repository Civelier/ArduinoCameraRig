using CameraRigController.FieldGrid.Editor.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid.Editor
{
    public abstract class EditorSelectorBase
    {
        public MemberInfo MemberInfo { get; }

        protected EditorSelectorBase(MemberInfo memberInfo)
        {
            MemberInfo = memberInfo;
        }

        public abstract string GetDataTemplateResourceName();
        public abstract EditorViewModelBase GetViewModel();
    }
}
