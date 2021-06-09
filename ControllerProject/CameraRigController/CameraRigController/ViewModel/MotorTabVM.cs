using CameraRigController.Model;
using GalaSoft.MvvmLight;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.ViewModel
{
    public class MotorTabsVM : ViewModelBase
    {
        private ObservableCollection<MotorTabVM> _tabs;

        public ObservableCollection<MotorTabVM> Tabs
        {
            get => _tabs;
            set 
            {
                _tabs = value;
                RaisePropertyChanged(nameof(Tabs));
            }
        }

        public MotorTabsVM()
        {
            _tabs = new ObservableCollection<MotorTabVM>();
            var count = 2;
            for (int i = 0; i < count; i++)
            {
                Tabs.Add(new MotorTabVM() { Data = new MotorTabModel() { MotorChannelName = $"Motor {i}" } });
            }
        }
    }

    public class MotorTabVM : DependencyObject
    {
        public MotorTabModel Data
        {
            get { return (MotorTabModel)GetValue(MotorChannelNameProperty); }
            set { SetValue(MotorChannelNameProperty, value); }
        }

        // Using a DependencyProperty as the backing store for MotorChannelName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty MotorChannelNameProperty =
            DependencyProperty.Register("Data", typeof(MotorTabModel), typeof(MotorTabVM), new PropertyMetadata(null));

        public MotorTabVM()
        {
        }

    }
}
