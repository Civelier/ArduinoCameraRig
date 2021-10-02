using CameraRigController.Model;
using CameraRigController.Properties;
using GalaSoft.MvvmLight;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.ViewModel
{
    [Serializable]
    public class MotorTabsVM : ViewModelBase, ISerializable
    {
        private string _name;

        public string Name
        {
            get
            {
                return _name;
            }
            set
            {
                _name = value;
                RaisePropertyChanged(nameof(Name));
            }
        }



        private ObservableCollection<MotorTabVM> _tabs;

        public ObservableCollection<MotorTabVM> Tabs
        {
            get => _tabs;
            private set 
            {
                _tabs = value;
                foreach (var tab in value)
                {
                    tab.Data.PropertyChanged += Data_PropertyChanged;
                }
                RaisePropertyChanged(nameof(Tabs));
            }
        }

        public MotorTabsVM()
        {
            Tabs = new ObservableCollection<MotorTabVM>();
            var count = Settings.Default.MotorChannelCount;
            for (int i = 0; i < count; i++)
            {
                var tab = new MotorTabVM() {
                    Data = new MotorTabModel() {
                        MotorChannelName = $"Motor {i}",
                        AnnimationChannelID = i,
                        StepsPerRevolution = 200,
                        MotorChannelID = i,
                    }
                };
                //tab.Data.PropertyChanged += (sender, e) => RaisePropertyChanged(nameof(Tabs)); 
                Tabs.Add(tab);
            }
        }

        private void _tabs_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add)
            {
                foreach (var item in e.NewItems)
                {
                    var tab = (MotorTabVM)item;
                    tab.Data.PropertyChanged += Data_PropertyChanged;
                }
            }
            if (e.Action == NotifyCollectionChangedAction.Remove ||
                e.Action == NotifyCollectionChangedAction.Reset)
            {
                foreach (var item in e.OldItems)
                {
                    var tab = (MotorTabVM)item;
                    tab.Data.PropertyChanged -= Data_PropertyChanged;
                }
            }
            RaisePropertyChanged(nameof(Tabs));
        }

        private void Data_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            RaisePropertyChanged(nameof(Tabs));
        }



        /// <summary>
        /// Makes a deep copy
        /// </summary>
        /// <param name="other">Instance to copy to</param>
        public void CopyTo(MotorTabsVM other)
        {
            other.Name = Name;
            other.Tabs.Clear();
            foreach (var tab in Tabs)
            {
                var newTab = new MotorTabVM(tab.Data.Clone());
                other.Tabs.Add(newTab);
            }
        }


        public MotorTabsVM(SerializationInfo info, StreamingContext context)
        {
            Name = (string)info.GetValue(nameof(Name), typeof(string));
            Tabs = (ObservableCollection<MotorTabVM>)info.GetValue(nameof(Tabs), typeof(ObservableCollection<MotorTabVM>));

        }
        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue(nameof(Name), Name);
            info.AddValue(nameof(Tabs), Tabs);
        }
    }

    [Serializable]
    public class MotorTabVM : DependencyObject, ISerializable
    {
        public MotorTabModel Data
        {
            get { return (MotorTabModel)GetValue(DataProperty); }
            set 
            {
                if (Data != null) Data.PropertyChanged -= Value_PropertyChanged;
                SetValue(DataProperty, value);
                if (value != null) value.PropertyChanged += Value_PropertyChanged;
            }
        }

        private void Value_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            OnPropertyChanged(new DependencyPropertyChangedEventArgs(DataProperty, null, Data));
        }

        public MotorTabVM(SerializationInfo info, StreamingContext context)
        {
            Data = (MotorTabModel)info.GetValue(nameof(Data), typeof(MotorTabModel));
        }

        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue(nameof(Data), Data);
        }

        // Using a DependencyProperty as the backing store for MotorChannelName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DataProperty =
            DependencyProperty.Register("Data", typeof(MotorTabModel), typeof(MotorTabVM), new PropertyMetadata(null));

        public MotorTabVM()
        {
        }


        public MotorTabVM(MotorTabModel data)
        {
            Data = data;
        }
    }
}
