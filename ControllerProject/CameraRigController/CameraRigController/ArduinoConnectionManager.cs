using CameraRigController.Properties;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace CameraRigController
{
    class ArduinoConnectionManager : IDisposable
    {
        ArduinoRecievePacket _lastPacket;
        private bool _newPacket;
        public string ComPort 
        {
            get => _comPort;
            set
            {
                _comPort = value;
                Port.Dispose();
            }
        }
        public SerialPort Port { get; private set; }
        Thread _arduinoPlay;
        private List<AnimChannel> _data;
        private bool _abort;
        private bool _run;
        private bool _running;
        private StringBuilder _buffer = new StringBuilder();
        private string _comPort;

        public ArduinoConnectionManager()
        {
            OpenPort();
            _arduinoPlay = new Thread(PlayRoutine);
        }

        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                var s = Port.ReadLine();
                var status = new ArduinoRecievePacket(s);
                if (status.Status == ArduinoStatusCode.Debug)
                {
                    Debug.Write("<Arduino debug> ");
                    Debug.WriteLine(Port.ReadLine());
                }
                else
                {
                    _lastPacket = status;
                    _newPacket = true;
                }
            }
            catch (IOException ex)
            {
                Debug.WriteLine(ex.Message);
            }
            
        }

        private bool AttemptConnection()
        {
            if (!Port.IsOpen)
            {
                try
                {
                    Port.Open();
                }
                catch (UnauthorizedAccessException e)
                {
                    _run = false;
                    MessageBox.Show(e.Message);
                    return false;
                }
                catch (InvalidOperationException e)
                {
                    _run = false;
                    MessageBox.Show(e.Message);
                    return false;
                }
                catch (IOException e)
                {
                    _run = false;
                    MessageBox.Show("Invalid port. Please select a valid port.\n" + e.Message);
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// Starts a thread to upload to the arduino and waits until thread has started.
        /// </summary>
        /// <param name="channels">Channels to send to the arduino.</param>
        public void Load(List<AnimChannel> channels)
        {
            _data = channels;
            if (!_arduinoPlay.IsAlive)
            {
                _arduinoPlay.Start();
            }
            _run = false;
            while (_running) Thread.Sleep(50);
            _run = true;
        }

        public bool TryConnect()
        {
            if (!Port.IsOpen)
            {
                if (AttemptConnection())
                {
                    var timeout = DateTime.Now + TimeSpan.FromSeconds(2);
                    while (DateTime.Now < timeout)
                    {
                        //if (Port.BytesToRead > 0)
                        //{
                        //    _lastPacket = new ArduinoRecievePacket(Port.ReadLine());
                        //    return _lastPacket.Status == ArduinoStatusCode.Ready;
                        //}
                        if (_newPacket)
                        {
                            _newPacket = false;
                            return _lastPacket.Status == ArduinoStatusCode.Ready;
                        }
                    }
                }
            }
            else
            {
                Port.Close();
                return false;
            }
            return false;
        }

        public bool ResetArduino()
        {
            if (Port.IsOpen) Port.Dispose();
            return TryConnect();
        }

        private void OpenPort()
        {
            Port = new SerialPort();
            if (!string.IsNullOrEmpty(ComPort)) Port.PortName = ComPort;
            Port.BaudRate = 115200;
            Port.DtrEnable = true;
            Port.RtsEnable = true;
            Port.DataReceived += Port_DataReceived;
            Port.Disposed += Port_Disposed;
        }

        private void Port_Disposed(object sender, EventArgs e)
        {
            Port.DataReceived -= Port_DataReceived;
            Port.Disposed -= Port_Disposed;
            OpenPort();
        }

        public ArduinoStatusCode SendStatusRequest()
        {
            SendDataPacket(ArduinoSendRequestPacket.StatusRequest.ToString());
            var res = ReadNextLine();
            if (res == null) return ArduinoStatusCode.None;
            return new ArduinoRecievePacket(res).Status;
        }

        private void SendFirstKeyframeData(Keyframe keyframe, int id)
        {
            SendDataPacket(new ArduinoSendKeyframePacket((ushort)id, 0, keyframe.Value).ToString());
        }

        private void SendKeyframeData(Keyframe keyframe, int id)
        {
            SendDataPacket(new ArduinoSendKeyframePacket((ushort)id, keyframe.MS + 1500, keyframe.Value).ToString());
        }

        private string ReadNextLine()
        {
            if (Port.IsOpen)
            {
                return Port.ReadLine();
            }
            return null;
        }

        private void SendDataPacket(string packet)
        {
            if (Port.IsOpen)
            {
                //if (_buffer.Length != 0)_buffer.Append(' ');
                //_buffer.Append(packet);
                Port.WriteLine(packet);
                Debug.WriteLine(packet);
            }
        }

        private void FlushBuffer()
        {
            //if (Port.IsOpen)
            //{
            //    Port.WriteLine(_buffer.ToString());
            //    Debug.WriteLine($"SerialPort: {_buffer}");
            //    _buffer.Clear();
            //}
        }

        private readonly struct KeyframeInfo : IComparable<KeyframeInfo>
        {
            public readonly Keyframe Keyframe;
            public readonly int MotorChannelID;

            public KeyframeInfo(Keyframe keyframe, int motorChannelID)
            {
                Keyframe = keyframe;
                MotorChannelID = motorChannelID;
            }

            public int CompareTo(KeyframeInfo other)
            {
                int timeComparisson = Keyframe.MS.CompareTo(other.Keyframe.MS);
                if (timeComparisson == 0) return MotorChannelID.CompareTo(other.MotorChannelID);
                return timeComparisson;
            }
        }

        private void PlayRoutine()
        {
            while (!_abort)
            {
                _running = false;
                while (!_run)
                {
                    if (_abort) return;
                    Thread.Sleep(50);
                }
                _running = true;
                //if (ResetArduino())
                if (TryConnect())
                {
                    Thread.Sleep(500);
                    Debug.WriteLine("Arduino connected");
                    while (_run)
                    {
                        //var status = SendStatusRequest();
                        //if (status == ArduinoStatusCode.Ready)
                        //{
                        int size = 0;
                        foreach (var channel in _data)
                        {
                            size += channel.Keyframes.Count;
                        }
                        var keyframeInfos = new List<KeyframeInfo>(size);

                        foreach (var channel in _data)
                        {
                            foreach (var keyframe in channel.Keyframes)
                            {
                                keyframeInfos.Add(new KeyframeInfo(keyframe, channel.MotorInfo.MotorChannelID));
                            }
                        }


                        keyframeInfos.Sort();
                        Debug.WriteLine($"Buffer length: {keyframeInfos.Count}");
                        foreach (var channel in _data)
                        {
                            SendFirstKeyframeData(channel.Keyframes.FirstOrDefault(), channel.MotorInfo.MotorChannelID);
                            Thread.Sleep(1);
                            if (!_run) break;
                            if (_abort) return;
                        }
                        foreach (var keyframeInfo in keyframeInfos)
                        {
                            SendKeyframeData(keyframeInfo.Keyframe, keyframeInfo.MotorChannelID);
                            Thread.Sleep(1);
                            if (!_run) break;
                            if (_abort) return;
                        }
                        
                        if (!_run) break;
                        if (_abort) return;
                        //}
                        //status = SendStatusRequest();
                        //if (status == ArduinoStatusCode.Ready)
                        //{
                        //FlushBuffer();
                        //}
                        SendDataPacket(ArduinoSendRequestPacket.StartRequest.ToString());
                        //while (status != ArduinoStatusCode.Done && !_abort)
                        //{
                        //    Thread.Sleep(1000);
                        //    status = SendStatusRequest();
                        //}
                        Debug.WriteLine("Upload complete");
                        _run = false;
                        //while (!_abort)
                        //{
                        //    if (_newPacket)
                        //    {
                        //        _newPacket = false;
                        //        if (_lastPacket.Status == ArduinoStatusCode.Done) break;
                        //    }
                        //}
                    }
                }
                //Thread.Sleep(1000);
                //if (Port.IsOpen)
                //{
                //    Port.Dispose();
                //    //while (!_abort)
                //    //{
                //    //    if (_newPacket)
                //    //    {
                //    //        if (_lastPacket.Status == ArduinoStatusCode.Done) break;
                //    //    }
                //    //}
                //    //Debug.WriteLine("Playback done");
                //}
            }
        }

        public void Dispose()
        {
            _abort = true;
            _arduinoPlay.Abort();
            Port.Dispose();
        }
    }
}
