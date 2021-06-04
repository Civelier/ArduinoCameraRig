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
        [ReadOnly(true)]
        [Description("The ID of the motor channel.")]
        [DisplayName("Motor channel ID")]
        public UInt16 MotorID { get; set; }

        [DisplayName("Name")]
        [Description("The name of the motor channel.")]
        public string Name { get; set; }

        [Description("The number of steps the motor has per revolution.")]
        [DisplayName("Steps per revolutions")]
        public UInt16 StepsPerRevolution { get; set; }
        //[DisplayName("Samples per second")]
        //[Description("Number of instruction per second.")]
        //public UInt32 SamplesPerSecond { get; set; }
    }
}
