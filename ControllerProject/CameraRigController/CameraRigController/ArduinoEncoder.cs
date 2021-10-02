using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    public enum ArduinoStatusCode
    {
        None =      0,
        Ready =     1,
        Running =   2,
        Done =      3,
        Debug =     4,
        ReadyForInstruction = 5,
        Value = 6,
        DebugBlockBegin = 7,
        DebugBlockEnd = 8,
        Error =     0b10000000,
        SpecificError = Error | 1,
    }

    public readonly struct ArduinoSendRequestPacket
    {
        /// <summary>
        /// Request a status update of the arduino
        /// </summary>
        public static readonly ArduinoSendRequestPacket StatusRequest = new ArduinoSendRequestPacket(1);
        /// <summary>
        /// Reserved for further use
        /// </summary>
        public static readonly ArduinoSendRequestPacket ErrorClearRequest = new ArduinoSendRequestPacket(2);
        /// <summary>
        /// Request the arduino to set the position of the motors to the starting point
        /// </summary>
        public static readonly ArduinoSendRequestPacket MotorResetRequest = new ArduinoSendRequestPacket(3);
        /// <summary>
        /// Request to start playback of the animation
        /// </summary>
        public static readonly ArduinoSendRequestPacket StartRequest = new ArduinoSendRequestPacket(4);

        public readonly UInt16 Command;
        public readonly string[] Arguments;

        private ArduinoSendRequestPacket(UInt16 command, params string[] arguments)
        {
            Command = command;
            Arguments = arguments;
        }
        private ArduinoSendRequestPacket(UInt16 command, IEnumerable<string> arguments)
        {
            Command = command;
            Arguments = arguments.ToArray();
        }

        public static ArduinoSendRequestPacket BufferAvailableToWriteRequest(UInt16 channelID)
        {
            return new ArduinoSendRequestPacket(8, channelID.ToString());
        }

        public static ArduinoSendRequestPacket BufferDoneRequest(UInt16 channelID)
        {
            return new ArduinoSendRequestPacket(11, channelID.ToString());
        }

        public override string ToString()
        {
            if (Arguments.Length == 0) return Command.ToString();
            var sb = new StringBuilder();
            sb.Append(Command);
            for (int i = 0; i < Arguments.Length; i++)
            {
                sb.Append(' ');
                sb.Append(Arguments[i]);
            }
            return sb.ToString();
        }
    }

    public readonly struct ArduinoSendKeyframePacket
    {
        public readonly UInt16 Command;
        public readonly UInt16 MotorChannel;
        public readonly UInt32 MS;
        public readonly Int32 Steps;

        /// <summary>
        /// Generates a packet containing a keyframe for the arduino
        /// </summary>
        /// <param name="motorChannel">Channel of the motor</param>
        /// <param name="ms">Time of keyframe</param>
        /// <param name="steps">Absolute position in steps</param>
        public ArduinoSendKeyframePacket(UInt16 motorChannel, UInt32 ms, Int32 steps)
        {
            Command = 7;
            MotorChannel = motorChannel;
            MS = ms;
            Steps = steps;
        }

        public override string ToString()
        {
            return $"{Command} {MotorChannel} {MS} {Steps}";
        }
    }

    public readonly struct ArduinoRecievePacket
    {
        public readonly ArduinoStatusCode Status;
        public bool HasError => (Status & ArduinoStatusCode.Error) == ArduinoStatusCode.Error;
        public readonly string Data;
        public ArduinoRecievePacket(string packet)
        {
            var i = packet.IndexOf(' ');
            if (int.TryParse(i == -1 ? packet : packet.Substring(0, i + 1), out int s))
            {
                Status = (ArduinoStatusCode)s;
                Data = packet.Length >= i + 1  ? packet.Substring(i + 1) : "";
            }
            else
            {
                Status = ArduinoStatusCode.None;
                Debug.WriteLine(packet);
                Data = packet;
            }
        }
    }
}
