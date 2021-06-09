using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.Model
{
    public class MotorTabModel : DependencyObject
    {
        public string MotorChannelName
        {
            get { return (string)GetValue(MotorChannelNameProperty); }
            set { SetValue(MotorChannelNameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for MotorChannelName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty MotorChannelNameProperty =
            DependencyProperty.Register("MotorChannelName", typeof(string), typeof(MotorTabModel), new PropertyMetadata("Motor1"));

        public MotorTabModel()
        {
            MotorChannelName = "Motor";
        }

    }
}
