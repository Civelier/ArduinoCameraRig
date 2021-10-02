using GalaSoft.MvvmLight.CommandWpf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    public abstract class TextBoxEditorVM<TEditor, TValue> : EditorViewModelBase<TEditor, TValue> 
        where TEditor : TextBoxEditorVM<TEditor, TValue>
    {
        public RelayCommand<TextBox> Confirm { get; set; }

        protected TextBoxEditorVM()
        {
            Confirm = new RelayCommand<TextBox>(ConfirmCommand);
        }

        protected virtual void ConfirmCommand(TextBox textBox)
        {
            SetValue(ObjectValueProperty, textBox.Text);
        }
    }
}
