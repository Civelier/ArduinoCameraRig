using CameraRigController.FieldGrid;
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

        private int _itg1;
        [DisplayName("Number")]
        public int Itg1
        {
            get => _itg1;
            set
            {
                _itg1 = value;
                OnPropertyChanged(nameof(Itg1));
            }
        }

        private int _itg2;

        [Slider(0, 10)]
        public int Itg2
        {
            get => _itg2;
            set
            {
                _itg2 = value;
                OnPropertyChanged(nameof(Itg2));
            }
        }

        private StringComparison _enumVar;

        public StringComparison EnumVar
        {
            get => _enumVar;
            set
            {
                _enumVar = value;
                OnPropertyChanged(nameof(EnumVar));
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
