using CameraRigController.Model;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.CommandWpf;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace CameraRigController.ViewModel
{
    /// <summary>
    /// This class contains properties that the main View can data bind to.
    /// <para>
    /// Use the <strong>mvvminpc</strong> snippet to add bindable properties to this ViewModel.
    /// </para>
    /// <para>
    /// You can also use Blend to data bind with the tool's support.
    /// </para>
    /// <para>
    /// See http://www.galasoft.ch/mvvm
    /// </para>
    /// </summary>
    public class MainViewModel : DependencyObject
    {
        /// <summary>
        /// Initializes a new instance of the MainViewModel class.
        /// </summary>
        public MainViewModel()
        {
            Tabs = new MotorTabsVM();
            FileManager = null;
            OpenFileCommand = new RelayCommand(OpenFile);
            PlayCommand = new RelayCommand(Play);
            RefreshPortsCommand = new RelayCommand(RefreshPorts);
            CloseCommand = new RelayCommand(Close);
            _lastPorts = new string[0];
            Title = "Camerarduino controller interface";
        }


        public string Title { get; set; }
        private AnimFileManager _fileManager;
        private string _filename;
        private string _numberOfChannels;
        private string _fps;
        private ArduinoConnectionManager _connectionManager = new ArduinoConnectionManager();
        private ObservableCollection<RadioButton> _ports = new ObservableCollection<RadioButton>();
        private IEnumerable<string> _lastPorts;




        public AnimFileManager FileManager
        {
            get { return (AnimFileManager)GetValue(FileManagerProperty); }
            set { SetValue(FileManagerProperty, value); }
        }

        // Using a DependencyProperty as the backing store for FileManager.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FileManagerProperty =
            DependencyProperty.Register("FileManager", typeof(AnimFileManager), typeof(MainViewModel), new PropertyMetadata(null, OnFileManagerChanged));

        static void OnFileManagerChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            var instance = sender as MainViewModel;
            instance.Filename = $"Filename: {instance.FileManager?.AnimFileInfo?.File.Name ?? ""}";
            instance.NumberOfAnnimationChannels = $"Number of annimation channels: {instance.FileManager?.AnimFileInfo?.ChannelCount.ToString() ?? ""}";
            instance.FPS = $"FPS: {instance.FileManager?.AnimFileInfo?.FPS.ToString() ?? ""}";
        }



        public string Filename
        {
            get { return (string)GetValue(FilenameProperty); }
            set { SetValue(FilenameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Filename.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FilenameProperty =
            DependencyProperty.Register("Filename", typeof(string), typeof(MainViewModel), new PropertyMetadata("Filename:"));




        public string NumberOfAnnimationChannels
        {
            get { return (string)GetValue(NumberOfAnnimationChannelsProperty); }
            set { SetValue(NumberOfAnnimationChannelsProperty, value); }
        }

        // Using a DependencyProperty as the backing store for NumberOfChannels.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty NumberOfAnnimationChannelsProperty =
            DependencyProperty.Register("NumberOfAnnimationChannels", typeof(string), typeof(MainViewModel), new PropertyMetadata("Number of annimation channels:"));


        public string FPS
        {
            get { return (string)GetValue(FPSProperty); }
            set { SetValue(FPSProperty, value); }
        }

        // Using a DependencyProperty as the backing store for FPS.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FPSProperty =
            DependencyProperty.Register("FPS", typeof(string), typeof(MainViewModel), new PropertyMetadata("FPS:"));



        public ObservableCollection<RadioButton> Ports
        {
            get => _ports;
            set
            {
                var old = _ports;
                _ports = value;
            }
        }

        private MotorTabsVM _tabs;

        public MotorTabsVM Tabs
        {
            get => _tabs;
            set
            {
                var old = _tabs;
                _tabs = value;
            }
        }

        public RelayCommand OpenFileCommand { get; set; }
        /// <summary>
        /// Command to call <see cref="Play"/>
        /// </summary>
        public RelayCommand PlayCommand { get; set; }
        public RelayCommand RefreshPortsCommand { get; set; }
        public RelayCommand RadioChecked { get; set; }
        public RelayCommand CloseCommand { get; set; }

        /// <summary>
        /// Computes the channels and sends to the arduino using <see cref="ArduinoConnectionManager.Load(List{AnimChannel})"/>
        /// </summary>
        void Play()
        {
            _connectionManager.Load(ComputeChannels().ToList());
        }

        void RefreshPorts()
        {
            var ports = SerialPort.GetPortNames();
            if (ports.SequenceEqual(_lastPorts)) return;
            _lastPorts = ports;
            int i = 0;
            while (i < Ports.Count)
            {
                var port = Ports[i];
                if (!ports.Contains((string)port.Content))
                {
                    port.Click -= Rdo_Click;
                    //port.Checked -= Rdo_Checked;
                    Ports.Remove(port);
                }
                else i++;
            }
            foreach (var port in ports)
            {
                var rdo = new RadioButton() { Content = port };
                rdo.Click += Rdo_Click;
                //rdo.Checked += Rdo_Checked;
                Ports.Add(rdo);
            }
        }

        private void Rdo_Checked(object sender, RoutedEventArgs e)
        {
            var rdo = (RadioButton)sender;
            _connectionManager.ComPort = (string)rdo.Content;
        }

        private void Rdo_Click(object sender, RoutedEventArgs e)
        {
            var rdo = (RadioButton)sender;
            if ((rdo.IsChecked ?? false))
            {
                _connectionManager.ComPort = (string)rdo.Content;
            }
        }

        private void Close()
        {
            _connectionManager.Dispose();
        }

        private void CloseFile()
        {
        }

        /// <summary>
        /// Computes the <see cref="RawKeyframe"/> from <see cref="FileManager"/> into <see cref="Keyframe"/> based on this motor channel's configuration.
        /// </summary>
        /// <param name="motorInfo">Information on the motor channel (from the UI model)</param>
        /// <returns>An animation channel containing the converted information ready to be sent to the arduino.
        /// Returns <see cref="null"/> if <see cref="FileManager"/> has not been properly openned.</returns>
        private AnimChannel ComputeChannel(MotorTabModel motorInfo)
        {
            // If file manager failed to open or has not decoded the data yet, 
            // exit the method.
            if (!FileManager.AnimFileInfo.HasValue) return null;

            // Initialize some variables
            var fileInfo = FileManager.AnimFileInfo.Value;
            var keyframes = new List<Keyframe>();
            
            // Get the raw annimation channel for this specific motor channel
            var rawAnimChannel = fileInfo.Channels[motorInfo.AnnimationChannelID];

            // Compute the raw keyframes to convert them into animation keyframes based on 
            // this specific motor channel configuration.
            foreach (var kf in rawAnimChannel.Keyframes)
            {
                // Get the keyframe's position in time in milliseconds.
                UInt32 ms = (UInt32)(kf.Frame / fileInfo.FPS * 1000.0);

                // Get the keyframe's position value in motor steps 
                // (for the actual stepper motor).
                Int32 value = (Int32)(kf.Value / (2.0 * Math.PI) * motorInfo.StepsPerRevolution * 16.0);
                keyframes.Add(new Keyframe(ms, value));
            }
            return new AnimChannel(keyframes, motorInfo);
        }

        IEnumerable<AnimChannel> ComputeChannels()
        {
            foreach (var motorInfo in Tabs.Tabs)
            {
                var channel = ComputeChannel(motorInfo.Data);
                if (channel != null) yield return channel;
            }
        }

        void OpenFile()
        {
            var openFile = new OpenFileDialog();
            openFile.Filter = "Text files|*";
            openFile.Multiselect = false;
            openFile.Title = "Open an animation file";
            openFile.CheckFileExists = true;
            openFile.CheckPathExists = true;
            var res = openFile.ShowDialog();
            if (res.HasValue && res.Value)
            {
                //if (_openFiles.Exists((file) => file.File.FullName == f.FileName))
                //{
                //    MessageBox.Show("File already open!", "Already open", MessageBoxButton.OK, MessageBoxImage.Warning);
                //    return;
                //}
                var fi = new FileInfo(openFile.FileName);
                var fm = new AnimFileManager(fi);
                fm.ProcessFile();
                FileManager = fm;
            }
        }
    }
}