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
    public class MainViewModel : ViewModelBase
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
            if (IsInDesignMode)
            {
                Title = "Hello MVVM Light (Design Mode)";
            }
            else
            {
                Title = "Hello MVVM Light";
            }
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
            get => _fileManager;
            set
            {
                var old = _fileManager;
                _fileManager = value;
                RaisePropertyChanged(nameof(FileManager), old, value);
                Filename = $"Filename: {FileManager?.AnimFileInfo?.File.Name ?? ""}";
                NumberOfChannels = $"Number of channels: {FileManager?.AnimFileInfo?.ChannelCount.ToString() ?? ""}";
                FPS = $"FPS: {FileManager?.AnimFileInfo?.FPS.ToString() ?? ""}";
            }
        }
        public string Filename
        {
            get => _filename; 
            set
            {
                var old = _filename;
                _filename = value;
                RaisePropertyChanged(nameof(Filename), old, value, true);
            }
        }
        public string NumberOfChannels
        {
            get => _numberOfChannels;
            set
            {
                var old = _numberOfChannels;
                _numberOfChannels = value;
                RaisePropertyChanged(nameof(NumberOfChannels), old, value, true);
            }
        }
        public string FPS
        {
            get => _fps;
            set
            {
                var old = _fps;
                _fps = value;
                RaisePropertyChanged(nameof(FPS), old, value, true);
            }
        }

        public ObservableCollection<RadioButton> Ports
        {
            get => _ports;
            set
            {
                var old = _ports;
                _ports = value;
                RaisePropertyChanged(nameof(Ports), old, value, true);
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
                RaisePropertyChanged(nameof(Tabs), old, value, true);
            }
        }

        public RelayCommand OpenFileCommand { get; set; }
        public RelayCommand PlayCommand { get; set; }
        public RelayCommand RefreshPortsCommand { get; set; }
        public RelayCommand RadioChecked { get; set; }
        public RelayCommand CloseCommand { get; set; }


        void Play()
        {
            _connectionManager.Play();
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
            rdo.IsChecked = true;
        }

        private void Close()
        {
            _connectionManager.Dispose();
        }

        private void CloseFile()
        {
        }

        private AnimChannel ComputeChannel(RawAnimChannel rawAnimChannel, MotorInfo motorInfo, AnimFileInfo fileInfo)
        {
            var keyframes = new List<Keyframe>();

            foreach (var kf in rawAnimChannel.Keyframes)
            {
                UInt32 ms = (UInt32)(kf.Frame / fileInfo.FPS * 1000.0);
                Int32 value = (Int32)(kf.Value / (2.0 * Math.PI) * motorInfo.StepsPerRevolution * 16.0);
                keyframes.Add(new Keyframe(ms, value));
            }
            return new AnimChannel(keyframes, rawAnimChannel.ChannelID, motorInfo);
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
                FileManager = new AnimFileManager(fi);
                FileManager.ProcessFile();
                
            }
        }
    }
}