using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public struct AnimFileInfo
    {
        public FileInfo File { get; }
        public int ChannelCount => Channels.Count;
        public int FrameCount { get; }
        public IReadOnlyList<RawAnimChannel> Channels { get; }
        public double FPS { get; }

        public AnimFileInfo(FileInfo file, List<RawAnimChannel> channels, double fps)
        {
            File = file;
            Channels = channels;
            FrameCount = 0;
            FPS = fps;
        }
    }
}
