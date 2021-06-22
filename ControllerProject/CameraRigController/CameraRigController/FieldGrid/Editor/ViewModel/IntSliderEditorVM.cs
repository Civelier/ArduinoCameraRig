﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.FieldGrid.Editor.ViewModel
{
    [FieldGridEditor(typeof(int), typeof(SliderAttribute))]
    [FieldgridTemplate("SliderTemplate")]
    public class IntSliderEditorVM : SliderNumberEditorVM<IntSliderEditorVM, int>
    {
    }
}
