using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Windows;

namespace CameraRigController
{
    [Serializable]
    public class MotorInfo
    {
        public string Name { get; set; }

        [Description("The number of steps the motor has per revolution.")]
        [DisplayName("Steps per revolutions")]
        public UInt16 StepsPerRevolution { get; set; }
        //[DisplayName("Samples per second")]
        //[Description("Number of instruction per second.")]
        //public UInt32 SamplesPerSecond { get; set; }
    }
}
