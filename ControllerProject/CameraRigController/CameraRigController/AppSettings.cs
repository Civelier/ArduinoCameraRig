using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.Serialization.Formatters;
using Newtonsoft.Json;
using System.Collections.ObjectModel;
using CameraRigController.ViewModel;
using System.Collections.Specialized;
using System.ComponentModel;
using CameraRigController.Model;
using System.Windows;
using System.Diagnostics;

namespace CameraRigController
{
    public class AppSettings : INotifyPropertyChanged
    {
        private const string _preferencesFileName = "Preferences.prop";
        private const string _defaultConfigName = "Default";
        private string _defaultTempName = "­temp";
        public const string ConfigExtension = ".cfg";

        private static AppSettings _instance;

        public static AppSettings Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new AppSettings();
                }
                return _instance;
            }
        }

        private DirectoryInfo _settingsDir;
        private FileInfo _prefFile;
        private JsonSerializer Serializer;
        private List<FileInfo> _fileInfos;

        public Preferences Preferences;
        public ObservableCollection<MotorTabsVM> MotorConfigs;
        public MotorTabsVM DefaultConfig = new MotorTabsVM() { Name = _defaultConfigName };

        private MotorTabsVM _currentConfig = null;

        public event PropertyChangedEventHandler PropertyChanged;

        public MotorTabsVM CurrentConfig
        {
            get
            {
                if (_currentConfig != null) return _currentConfig;
                var name = Preferences.CurrentConfigName;
                var res = MotorConfigs.FirstOrDefault((c) => c.Name == name);
                if (res == null) return CurrentConfig = DefaultConfig;
                return CurrentConfig = res;
            }
            set
            {
                if (_currentConfig != null) _currentConfig.PropertyChanged -= Value_PropertyChanged;
                Preferences.CurrentConfigName = value.Name;
                _currentConfig = value;
                OnPropertyChanged(nameof(CurrentConfig));
                if (value != null) value.PropertyChanged += Value_PropertyChanged;
            }
        }

        private void Value_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            OnPropertyChanged(nameof(CurrentConfig));
        }

        AppSettings()
        {
            Serializer = JsonSerializer.CreateDefault(new JsonSerializerSettings() 
            {
                Formatting = Formatting.Indented
            });
            _fileInfos = new List<FileInfo>();
            MotorConfigs = new ObservableCollection<MotorTabsVM>();
            LoadOrCreate();
            SetupEvents();
        }

        void SetupEvents()
        {
            MotorConfigs.CollectionChanged += MotorConfigs_CollectionChanged;
            Preferences.PropertyChanged += Preferences_PropertyChanged;
        }

        private void Preferences_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            using (var preferencesStream = _prefFile.CreateText())
            {
                Serializer.Serialize(preferencesStream, Preferences);
            }
            OnPropertyChanged(nameof(Preferences));
        }

        void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void MotorConfigs_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add)
            {
                foreach (var item in e.NewItems)
                {
                    var config = (MotorTabsVM)item;
                    config.PropertyChanged += Config_PropertyChanged;
                }
            }
            if (e.Action == NotifyCollectionChangedAction.Remove)
            {
                foreach (var item in e.OldItems)
                {
                    var config = (MotorTabsVM)item;
                    config.PropertyChanged -= Config_PropertyChanged;
                }
            }
            OnPropertyChanged(nameof(MotorConfigs));
        }

        void LoadOrCreate()
        {
            _settingsDir = new DirectoryInfo(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)  + "\\CameraRigController");
            if (!_settingsDir.Exists)
            {
                _settingsDir.Create();
            }
            _prefFile = new FileInfo(_settingsDir.FullName + "\\" + _preferencesFileName);
            if (!_prefFile.Exists)
            {
                using (var prefStream = _prefFile.CreateText())
                {
                    Preferences = new Preferences();
                    Serializer.Serialize(prefStream, Preferences);
                }
            }
            else
            {
                using (var prefStream = _prefFile.OpenText())
                {
                    Preferences = (Preferences)Serializer.Deserialize(prefStream, typeof(Preferences));
                }
            }
            var configFiles = _settingsDir.GetFiles('*' + ConfigExtension);
            var defaultConfigFile = new FileInfo($"{_settingsDir.FullName}\\{_defaultConfigName}{ConfigExtension}");
            //_fileInfos.Add(defaultConfigFile);
            var defaultConfig = new MotorTabsVM() { Name = _defaultConfigName };
            MotorConfigs.Add(defaultConfig);
            _fileInfos.Add(defaultConfigFile);
            if (configFiles.Length <= 0)
            {
                using (var configStream = defaultConfigFile.CreateText())
                {
                    Serializer.Serialize(configStream, defaultConfig);
                }
            }

            foreach (var configFile in configFiles)
            {
                if (configFile.Name != (_defaultConfigName + ConfigExtension))
                {
                    try
                    {
                        using (var configStream = configFile.OpenText())
                        {

                            var config = (MotorTabsVM)Serializer.Deserialize(configStream, typeof(MotorTabsVM));
                            config.PropertyChanged += Config_PropertyChanged;
                            MotorConfigs.Add(config);
                            _fileInfos.Add(configFile);
                        }
                    }
                    catch (InvalidCastException)
                    {
                        configFile.Delete();
                        Debug.WriteLine($"Deleted '{configFile.Name}' because of invalid cast exception!");
                    }
                    catch (Exception e)
                    {
                        MessageBox.Show(e.Message, "Config file read error");
                    }
                }
            }
            if (MotorConfigs.FirstOrDefault((MotorTabsVM item) => item.Name == Preferences.CurrentConfigName) == null)
            {
                Preferences.CurrentConfigName = _defaultConfigName;
            }
        }

        private void Config_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var config = (MotorTabsVM)sender;
            WriteConfig(config);
            OnPropertyChanged(nameof(MotorConfigs));
        }

        private void WriteConfig(MotorTabsVM config)
        {
            var configFileInfo = _fileInfos.Find((f) => f.Name == config.Name + ConfigExtension);
            if (configFileInfo == null) return;
            using (var configStream = configFileInfo.CreateText())
            {
                Serializer.Serialize(configStream, config);
            }
        }

        public MotorTabsVM CreateTempConfig(MotorTabsVM basedOn)
        {
            var temp = new MotorTabsVM();
            basedOn.CopyTo(temp);

            temp.Name = basedOn.Name + _defaultTempName;
            MotorConfigs.Add(temp);
            _fileInfos.Add(new FileInfo($"{_settingsDir.FullName}\\{temp.Name + ConfigExtension}"));
            temp.PropertyChanged += Config_PropertyChanged;
            WriteConfig(temp);
            return temp;
        }
    }
}
