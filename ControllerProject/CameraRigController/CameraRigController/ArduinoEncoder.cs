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
        Error =     0b10000000,
        SpecificError = Error | 1,
    }

    public readonly struct ArduinoSendRequestPacket
    {
        public static readonly ArduinoSendRequestPacket StatusRequest = new ArduinoSendRequestPacket(1);
        public static readonly ArduinoSendRequestPacket ErrorClearRequest = new ArduinoSendRequestPacket(2);
        public static ArduinoSendRequestPacket MotorResetRequest(double[] initialStates)
        {
            return new ArduinoSendRequestPacket(3, initialStates.Cast<string>());
        }
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
            if (int.TryParse(packet, out int s))
            {
                Status = (ArduinoStatusCode)s;
                Data = "";
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
