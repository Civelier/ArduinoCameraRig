using CameraRigController.Model;
using GalaSoft.MvvmLight.CommandWpf;
using GalaSoft.MvvmLight.Ioc;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.ViewModel
{
    public class ConfigSelectVM : DependencyObject, INotifyPropertyChanged
    {
        public IList<ConfigModel> Configs { get; private set; }
        public RelayCommand<Window> SelectCommand { get; private set; }
        public RelayCommand<Window> CancelCommand { get; private set; }



        public ConfigModel SelectedConfig
        {
            get { return (ConfigModel)GetValue(SelectedConfigProperty); }
            set { SetValue(SelectedConfigProperty, value); }
        }

        // Using a DependencyProperty as the backing store for SelectedConfig.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty SelectedConfigProperty =
            DependencyProperty.Register("SelectedConfig", typeof(ConfigModel), typeof(ConfigSelectVM), new PropertyMetadata());

        public event PropertyChangedEventHandler PropertyChanged;

        public ConfigSelectVM()
        {
            Configs = GetConfigModels();
            SelectCommand = new RelayCommand<Window>(Select);
            CancelCommand = new RelayCommand<Window>(Cancel);
            SelectedConfig = Configs.First((c) => c.Tabs == AppSettings.Instance.CurrentConfig);
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(e.Property.Name));
        }


        IList<ConfigModel> GetConfigModels()
        {
            return AppSettings.Instance.MotorConfigs.Select((item) => new ConfigModel() {
                Tabs = item
            }).ToList();
        }
        void Select(Window window)
        {
            AppSettings.Instance.CurrentConfig = SelectedConfig.Tabs;
            window.Hide();
        }

        void Cancel(Window window)
        {
            SelectedConfig = Configs.First((c) => c.Tabs == AppSettings.Instance.CurrentConfig);
            window.Hide();
        }
    }
}
