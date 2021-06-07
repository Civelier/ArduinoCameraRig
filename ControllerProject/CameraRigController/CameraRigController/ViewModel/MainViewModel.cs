using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.CommandWpf;
using Microsoft.Win32;
using System.IO;

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
            FileManager = null;
            OpenFileCommand = new RelayCommand(OpenFile);
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

        public RelayCommand OpenFileCommand { get; set; }

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
                Set(nameof(FileManager), ref _fileManager, new AnimFileManager(fi), true);
                FileManager.ProcessFile();
            }
        }
    }
}