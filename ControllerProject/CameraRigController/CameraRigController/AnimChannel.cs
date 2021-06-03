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
        public ushort ChannelID { get; }
        public MotorInfo MotorInfo { get; }

        public AnimChannel(List<Keyframe> keyframes, ushort id, MotorInfo motorInfo)
        {
            Keyframes = keyframes;
            ChannelID = id;
            MotorInfo = motorInfo;
        }
    }
}
