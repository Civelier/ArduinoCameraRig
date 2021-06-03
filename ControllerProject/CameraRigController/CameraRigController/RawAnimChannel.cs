using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public class RawAnimChannel
    {
        private List<RawKeyframe> _keyframes = new List<RawKeyframe>();
        public IReadOnlyList<RawKeyframe> Keyframes { get => _keyframes; }
        public ushort ChannelID { get; }

        public RawAnimChannel(ushort id)
        {
            ChannelID = id;
        }

        public void AddRawKeyframe(RawKeyframe rawKeyframe)
        {
            _keyframes.Add(rawKeyframe);
        }
    }
}
