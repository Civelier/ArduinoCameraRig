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

        private int _motorChannelID;
        [ReadOnly(true)]
        public int MotorChannelID
        {
            get => _motorChannelID;
            set
            {
                _motorChannelID = value;
                OnPropertyChanged(nameof(MotorChannelID));
            }
        }

        private int _animationChannelID;

        [UpDownBox(0, 3, 0, 1)]
        public int AnnimationChannelID
        {
            get => _animationChannelID;
            set
            {
                _animationChannelID = value;
                OnPropertyChanged(nameof(AnnimationChannelID));
            }
        }

        private int _stepsPerRervs;

        public int StepsPerRevolution
        {
            get => _stepsPerRervs;
            set
            {
                _stepsPerRervs = value;
                OnPropertyChanged(nameof(StepsPerRevolution));
            }
        }


        public event PropertyChangedEventHandler PropertyChanged;

        public MotorTabModel()
        {
            MotorChannelName = "Motor";
        }

        private void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
