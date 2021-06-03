using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public class AnimFileManager
    {
        public readonly FileInfo File;
        public AnimFileInfo AnimFileInfo { get; private set; }
        public AnimFileManager(FileInfo file)
        {
            File = file;
        }
        public void ProcessFile()
        {
            var rawChannels = new List<RawAnimChannel>();
            double fps;
            using (var reader = File.OpenText())
            {
                fps = double.Parse(reader.ReadLine());
                RawAnimChannel channel = null;
                while (!reader.EndOfStream)
                {
                    var s = reader.ReadLine().Split(',');
                    if (s.Length == 1) // Idication of a new channel ID
                    {
                        channel = new RawAnimChannel(ushort.Parse(s[0]));
                        rawChannels.Add(channel);
                    }
                    else if (s.Length == 2)
                    {
                        uint frame = (uint)Math.Truncate(double.Parse(s[0]));
                        double value = double.Parse(s[1]);
                        channel?.AddRawKeyframe(new RawKeyframe(frame, value));
                    }
                }
            }
            AnimFileInfo = new AnimFileInfo(File, rawChannels, fps);
        }
    }
}
