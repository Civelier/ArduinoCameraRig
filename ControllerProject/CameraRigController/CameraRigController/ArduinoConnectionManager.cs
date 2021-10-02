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
        private bool _debugging = false;

        public ArduinoConnectionManager()
        {
            SetPortSettings();
            _arduinoPlay = new Thread(PlayRoutine);
        }

        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                var s = Port.ReadLine();
                Debug.WriteLine($"Raw Arduino: {s}");
                var status = new ArduinoRecievePacket(s);
                switch (status.Status)
                {
                    case ArduinoStatusCode.Debug:
                        Debug.Write("<Arduino debug> ");
                        Debug.WriteLine(Port.ReadLine());
                        break;
                    case ArduinoStatusCode.DebugBlockBegin:
                        _debugging = true;
                        while (true)
                        {
                            var line = Port.ReadLine();
                            if (int.TryParse(line, out int code))
                            {
                                if (code == (int)ArduinoStatusCode.DebugBlockEnd) return;
                            }
                            Debug.WriteLine(line);
                        }
                    default:
                        _lastPacket = status;
                        _newPacket = true;
                        break;
                }
            }
            catch (IOException ex)
            {
                Debug.WriteLine(ex.Message);
            }
            catch (InvalidOperationException ex)
            {
                Debug.WriteLine($"Port was closed: {ex.Message}");
            }
            
        }

        private bool AttemptConnection()
        {
            if (!Port.IsOpen)
            {
                try
                {
                    SetPortSettings();
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
                    Debug.WriteLine("Calling arduino");
                    var timeout = DateTime.Now + TimeSpan.FromSeconds(5);
                    while (DateTime.Now < timeout)
                    {
                        Thread.Sleep(100);
                        //if (Port.BytesToRead > 0)
                        //{
                        //    _lastPacket = new ArduinoRecievePacket(Port.ReadLine());
                        //    return _lastPacket.Status == ArduinoStatusCode.Ready;
                        //}
                        if (_newPacket)
                        {
                            _newPacket = false;
                            Debug.WriteLine("Arduino responded");
                            return _lastPacket.Status == ArduinoStatusCode.Ready;
                        }
                    }
                }
            }
            else
            {
                Debug.WriteLine("Closed port to attempt a new connection");
                Port.Close();
                return false;
            }
            Debug.WriteLine("Try connection failed");
            return false;
        }

        public bool ResetArduino()
        {
            if (Port.IsOpen) Port.Dispose();
            return TryConnect();
        }

        private void SetPortSettings()
        {
            Debug.WriteLine("Openning port");
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
            Debug.WriteLine("Port disposed");
            Port.DataReceived -= Port_DataReceived;
            Port.Disposed -= Port_Disposed;
            //OpenPort();
        }

        public ArduinoStatusCode SendStatusRequest()
        {
            SendDataPacket(ArduinoSendRequestPacket.StatusRequest.ToString());
            var res = ReadNextLine();
            if (res == null) return ArduinoStatusCode.None;
            return new ArduinoRecievePacket(res).Status;
        }

        /// <summary>
        /// Send a request for the available length of the buffer to write
        /// </summary>
        /// <param name="channelID">Arduino keyframe buffer (motor) channelID</param>
        /// <returns>The number of keyframes that can be written.
        /// Returns null if invalid value or </returns>
        private int? SendBufferAvailableForWriteRequest(UInt16 channelID)
        {
            _newPacket = false;
            SendDataPacket(ArduinoSendRequestPacket.BufferAvailableToWriteRequest(channelID).ToString());
            while (!_abort && _run)
            {
                if (_newPacket)
                {
                    if (_lastPacket.Status == ArduinoStatusCode.Value)
                    {
                        if (int.TryParse(_lastPacket.Data, out int count))
                        {
                            _newPacket = false;
                            return count;
                        }
                        return null;
                    }
                }
                Thread.Sleep(100);
            }
            return null;
        }

        private void SendEndOfBufferRequest(UInt16 channelID)
        {
            SendDataPacket(ArduinoSendRequestPacket.BufferDoneRequest(channelID).ToString());
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
                    Thread.Sleep(100);
                }
                _running = true;
                //if (ResetArduino())
                if (TryCon­nect())
                {
                    Thread.Sleep(500);
                    Debug.WriteLine("Arduino connected");
                    while (_run)
                    {
                        //var status = SendStatusRequest();
                        //if (status == ArduinoStatusCode.Ready)
                        //{
                        var keyframeInfos = new List<Queue<KeyframeInfo>>(Settings.Default.MotorChannelCount);

                        foreach (var channel in _data)
                        {
                            var infos = new List<KeyframeInfo>(channel.Keyframes.Count);
                            foreach (var keyframe in channel.Keyframes)
                            {
                                infos.Add(new KeyframeInfo(keyframe, channel.MotorInfo.MotorChannelID));
                            }
                            infos.Sort();
                            Debug.WriteLine($"Buffer[{channel.MotorInfo.MotorChannelID}] length: {infos.Count}");
                            keyframeInfos.Add(new Queue<KeyframeInfo>(infos));
                        }

                        //Debug.WriteLine($"Buffer length: {keyframeInfos.Count}");
                        foreach (var channel in _data)
                        {
                            SendFirstKeyframeData(channel.Keyframes.FirstOrDefault(), channel.MotorInfo.MotorChannelID);
                            Thread.Sleep(1);
                            if (!_run) break;
                            if (_abort) return;
                        }


                        //var bufferAvailable = new int[Settings.Default.MotorChannelCount];
                        //// Get buffer sizes
                        //for (int i = 0; i < Settings.Default.MotorChannelCount; i++)
                        //{
                        //    var count = SendBufferAvailableForWriteRequest((UInt16)i);
                        //    Thread.Sleep(1);
                        //    if (count.HasValue)
                        //    {
                        //        bufferAvailable[i] = count.Value;
                        //    }
                        //    if (!_run) break;
                        //    if (_abort) return;
                        //}
                        bool firstRun = true; // To send the start command to the arduino
                        while (!_abort && _run)
                        {
                            // If all queues are empty, then upload is complete
                            if (keyframeInfos.All((q) => q.Count == 0)) break;

                            // Send keyframes to buffers
                            for (int i = 0; i < keyframeInfos.Count; i++)
                            {
                                var infos = keyframeInfos[i];

                                // Verify if there is still data to be sent on this channel
                                if (infos.Count <= 0) break;

                                // Keep track of the buffer's available length
                                for (;infos.Count > 0;)
                                {
                                    //if (infos.Count == 0) break;
                                    // Send next keyframe
                                    var info = infos.Dequeue();
                                    SendKeyframeData(info.Keyframe, info.MotorChannelID);
                                    Thread.Sleep(1);

                                    // Ensure that if execution needs to stop, it can exit
                                    if (!_run) break;
                                    if (_abort) return;
                                }
                                //SendEndOfBufferRequest((UInt16)i);
                            }

                            if (!_run) break;
                            if (_abort) return;

                            if (firstRun) // Send the start command on first run
                            {
                                SendDataPacket(ArduinoSendRequestPacket.StartRequest.ToString());
                                firstRun = false;
                            }
                            break; // Because were not filling the buffer multiple times anymore

                            // Wait for either 'Done' or 'ReadyForInstruction' status from arduino
                            //while (!_abort && _run)
                            //{
                            //    if (_newPacket)
                            //    {
                            //        if (_lastPacket.Status == ArduinoStatusCode.Done)
                            //        {
                            //            _newPacket = false;
                            //            break;
                            //        }
                            //        if (_lastPacket.Status == ArduinoStatusCode.ReadyForInstruction)
                            //        {
                            //            _newPacket = false;
                            //            if (UInt16.TryParse(_lastPacket.Data, out UInt16 channelID))
                            //            {
                            //                if (channelID >= 0 && channelID < Settings.Default.MotorChannelCount)
                            //                {
                            //                    if (keyframeInfos[channelID].Count <= 0)
                            //                    {
                            //                        SendEndOfBufferRequest(channelID);
                            //                        break;
                            //                    }
                            //                    var count = SendBufferAvailableForWriteRequest(channelID);
                            //                    if (count.HasValue)
                            //                    {
                            //                        bufferAvailable[channelID] = count.Value;
                            //                    }
                            //                    else if (_run && !_abort) throw new Exception("Possible communication error");
                            //                }
                            //                else throw new Exception($"ChannelID was out of range: {channelID}");
                            //            }
                            //            break;
                            //        }
                            //    }
                            //}
                        }

                        
                        if (!_run) break;
                        if (_abort) return;
                        //}
                        //status = SendStatusRequest();
                        //if (status == ArduinoStatusCode.Ready)
                        //{
                        //FlushBuffer();
                        //}
                        
                        //while (status != ArduinoStatusCode.Done && !_abort)
                        //{
                        //    Thread.Sleep(1000);
                        //    status = SendStatusRequest();
                        //}
                        Debug.WriteLine("Upload complete");

                        while (_run && !_abort)
                        {
                            if (_newPacket)
                            {
                                _newPacket = false;
                                if (_lastPacket.Status == ArduinoStatusCode.Done)
                                {
                                    Debug.WriteLine("Playback complete");
                                    Port.Dispose();
                                    break;
                                }
                            }
                        }

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
