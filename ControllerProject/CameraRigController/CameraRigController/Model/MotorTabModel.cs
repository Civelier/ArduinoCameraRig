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
    [Serializable]
    public class MotorTabModel : INotifyPropertyChanged
    {
        private string _motorChannelName;
        [Description("The name of this motor channel configuration (this is purely for ease of use)")]
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
        [Description("The underlying Arduino motor channel this motor configuration applies to.")]
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
        [Description("Blender animation channel ID")]
        public int AnnimationChannelID
        {
            get => _animationChannelID;
            set
            {
                _animationChannelID = value;
                OnPropertyChanged(nameof(AnnimationChannelID));
            }
        }

        private int _stepsPerRevs;
        [Description("Steps per revolution of this specific motor (commonly 200)")]
        public int StepsPerRevolution
        {
            get => _stepsPerRevs;
            set
            {
                _stepsPerRevs = value;
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


        /// <summary>
        /// Makes a shallow copy of the object (the invocation list of <see cref="PropertyChanged"/> is not copied)
        /// </summary>
        /// <returns>The cloned instance</returns>
        public MotorTabModel Clone()
        {
            var clone = new MotorTabModel() {

                _animationChannelID = _animationChannelID,
                _motorChannelID = _motorChannelID,
                _motorChannelName = _motorChannelName,
                _stepsPerRevs = _stepsPerRevs,
            };
            return clone;
        }
    }
}
