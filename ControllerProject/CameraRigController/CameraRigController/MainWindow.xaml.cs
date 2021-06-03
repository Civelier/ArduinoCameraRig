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
        public MainWindow()
        {
            InitializeComponent();
            var tab = new ChannelTab();
            _tabs.Add(tab);
            ChannelFrame.Content = tab;
        }

        /// <summary>
        /// Connection->Play
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click(object sender, RoutedEventArgs e)
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
                animChannels.Add(ComputeChannel(file.AnimFileInfo.Channels[index - 1],
                    tab.MotorInfo, file.AnimFileInfo));
            }

            _connectionManager.Play(animChannels);
        }

        /// <summary>
        /// Edit->Preferences
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MenuItem_Click_1(object sender, RoutedEventArgs e)
        {

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
                if (_openFiles.Exists((file) => file.File.FullName == f.FileName))
                {
                    MessageBox.Show("File already open!", "Already open", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return;
                }
                var fi = new FileInfo(f.FileName);
                var manager = new AnimFileManager(fi);
                _openFiles.Add(manager);
                FileNameLabel.Content = $"Filename: {fi.Name}";
                manager.ProcessFile();
                ChannelNumberLabel.Content = $"Number of channels: {manager.AnimFileInfo.ChannelCount}";

                foreach (var tab in _tabs)
                {
                    tab.ChannelComboBox.Items.Clear();
                    tab.ChannelComboBox.Items.Add(new ComboBoxItem() { Content = "None" });
                    for (int i = 0; i < manager.AnimFileInfo.ChannelCount; i++)
                    {
                        tab.ChannelComboBox.Items.Add(new ComboBoxItem() { Content = i.ToString() });
                    }
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
            _connectionManager.Port.PortName = radio.Content.ToString();
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            _connectionManager.Dispose();
        }
    }
}
