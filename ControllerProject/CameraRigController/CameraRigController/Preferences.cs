using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController
{
    [Serializable]
    public class Preferences : INotifyPropertyChanged
    {
        private string _currentConfigName = "Default";

        public string CurrentConfigName
        {
            get => _currentConfigName;
            set
            {
                _currentCon­figName = value;
                OnPropertyChanged(nameof(CurrentConfigName));
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
