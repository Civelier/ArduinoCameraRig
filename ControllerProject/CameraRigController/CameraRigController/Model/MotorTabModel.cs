using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.Model
{
    public class MotorTabModel : INotifyPropertyChanged
    {
        private string _motorChannelName;

        public string MotorChannelName
        {
            get { return _motorChannelName; }
            set 
            { 
                _motorChannelName = value;
                OnPropertyChanged(nameof(MotorChannelName));
            }
        }

        private string _testReadonly;

        [DisplayName("Test value")]
        [ReadOnly(true)]
        public string TestReadonly
        {
            get => _testReadonly;
            set
            {
                _testReadonly = value;
                OnPropertyChanged(nameof(TestReadonly));
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public MotorTabModel()
        {
            MotorChannelName = "Motor";
            TestReadonly = "Test readonly value";
        }

        private void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

    }
}
