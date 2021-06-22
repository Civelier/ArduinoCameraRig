using CameraRigController.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public class AnimChannel
    {
        public IReadOnlyList<Keyframe> Keyframes { get; }
        public MotorTabModel MotorInfo { get; }

        public AnimChannel(List<Keyframe> keyframes, MotorTabModel motorInfo)
        {
            Keyframes = keyframes;
            MotorInfo = motorInfo;
        }
    }
}
