using CameraRigController.ViewModel;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace CameraRigController
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<AnimFileManager> _openFiles = new List<AnimFileManager>();
        private string[] _lastPorts = null;
        private ArduinoConnectionManager _connectionManager = new ArduinoConnectionManager();
        private List<ChannelTab> _tabs = new List<ChannelTab>();
        private readonly MainViewModel _modelView;

        public MainWindow()
        {
            InitializeComponent();
            _modelView = CommonServiceLocator.ServiceLocator.Current.GetInstance<MainViewModel>();
            _modelView.PropertyChanged += _modelView_PropertyChanged;
            var tab = new ChannelTab();
            tab.MotorInfo.MotorID = 0;
            tab.MotorInfo.Name = $"Motor {tab.MotorInfo.MotorID}";
            _tabs.Add(tab);
            ChannelFrame.Content = tab;
        }

        private void _modelView_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            
        }

        /// <summary>
        /// Connection->Play
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            _connectionManager.Play();
        }

        /// <summary>
        /// Edit->Preferences
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click_1(object sender, RoutedEventArgs e)
        {

        }

        private void CloseFile(string path)
        {
            if (_openFiles.Exists((file) => file.File.FullName == path))
            {
                var file = _openFiles.Find((f) => f.File.FullName == path);
                _openFiles.Remove(file);
                //FileNameLabel.Content = "Filename:";
                //ChannelNumberLabel.Content = "Number of channels:";
                //FPSLabel.Content = "FPS:";

                foreach (var tab in _tabs)
                {
                    tab.ChannelComboBox.Items.Clear();
                    tab.ChannelComboBox.Items.Add(new ComboBoxItem() { Content = "None" });
                    tab.ChannelComboBox.SelectedIndex = 0;
                }
            }
        }

        /// <summary>
        /// File->Open
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click_2(object sender, RoutedEventArgs e)
        {
            e.Handled = true;
            var f = new OpenFileDialog();
            f.Filter = "Text files|*";
            f.Multiselect = false;
            f.Title = "Open an animation file";
            f.CheckFileExists = true;
            f.CheckPathExists = true;
            var res = f.ShowDialog();
            if (res.HasValue && res.Value)
            {
                if (_openFiles.Count > 0)
                {
                    CloseFile(_openFiles.Last().File.FullName);
                }
                if (_openFiles.Exists((file) => file.File.FullName == f.FileName))
                {
                    MessageBox.Show("File already open!", "Already open", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return;
                }
                var fi = new FileInfo(f.FileName);
                var manager = _modelView.FileManager = new AnimFileManager(fi);
                _openFiles.Add(manager);
                //FileNameLabel.Content = $"Filename: {fi.Name}";
                manager.ProcessFile();
                //ChannelNumberLabel.Content = $"Number of channels: {manager.AnimFileInfo.Value.ChannelCount}";
                //FPSLabel.Content = $"FPS: {manager.AnimFileInfo.Value.FPS}";
                int tabID = 0;
                foreach (var tab in _tabs)
                {
                    tabID++;
                    tab.ChannelComboBox.Items.Clear();
                    tab.ChannelComboBox.Items.Add(new ComboBoxItem() { Content = "None" });
                    for (int i = 0; i < manager.AnimFileInfo.Value.ChannelCount; i++)
                    {
                        tab.ChannelComboBox.Items.Add(new ComboBoxItem() { Content = i.ToString() });
                    }
                    tab.ChannelComboBox.SelectedIndex = tabID < tab.ChannelComboBox.Items.Count ? tabID : 0;
                }
            }
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

        /// <summary>
        /// Connection->Port
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_GotFocus_1(object sender, RoutedEventArgs e)
        {
            if (_lastPorts == null) _lastPorts = SerialPort.GetPortNames();
            else if (_lastPorts.SequenceEqual(SerialPort.GetPortNames())) return;
            _lastPorts = SerialPort.GetPortNames();
            PortMenuItem.Items.Clear();
            foreach (var port in _lastPorts)
            {
                var radio = new RadioButton() { Content = port };
                radio.Checked += Radio_Checked;
                PortMenuItem.Items.Add(radio);
            }
        }

        private void Radio_Checked(object sender, RoutedEventArgs e)
        {
            var radio = (RadioButton)sender;
            _connectionManager.ComPort = radio.Content.ToString();
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            _connectionManager.Dispose();
        }

        /// <summary>
        /// Connection->Initialize motors
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click_3(object sender, RoutedEventArgs e)
        {

        }

        /// <summary>
        /// Connection->Load
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click_4(object sender, RoutedEventArgs e)
        {
            var file = _openFiles.LastOrDefault();
            if (file == null)
            {
                MessageBox.Show("No open file. Open a file to play the animation: File->Open");
                return;
            }
            List<AnimChannel> animChannels = new List<AnimChannel>();
            foreach (var tab in _tabs)
            {
                if (tab.ChannelComboBox.SelectedIndex < 1)
                {
                    MessageBox.Show("Select an animation channel!");
                    return;
                }
                int index = tab.ChannelComboBox.SelectedIndex;
                animChannels.Add(ComputeChannel(file.AnimFileInfo.Value.Channels[index - 1],
                    tab.MotorInfo, file.AnimFileInfo.Value));
            }
            _connectionManager.Load(animChannels);
        }
    }
}
