using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public class ArduinoConfig
    {
        public Dictionary<int, MotorInfo> MotorChannels { get; set; }

        public ArduinoConfig()
        {

        }
    }
}
