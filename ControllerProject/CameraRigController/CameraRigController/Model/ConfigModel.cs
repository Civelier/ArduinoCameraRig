using CameraRigController.ViewModel;
using GalaSoft.MvvmLight.CommandWpf;
using GalaSoft.MvvmLight.Ioc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CameraRigController.Model
{
    public class ConfigModel : DependencyObject
    {
        public MotorTabsVM Tabs
        {
            get { return (MotorTabsVM)GetValue(TabsProperty); }
            set { SetValue(TabsProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Tabs.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty TabsProperty =
            DependencyProperty.Register("Tabs", typeof(MotorTabsVM), typeof(ConfigModel), new PropertyMetadata(null));


        public RelayCommand SelectCommand { get; set; }

        public ConfigModel()
        {
            SelectCommand = new RelayCommand(Select);
        }

        void Select()
        {
            var instance = SimpleIoc.Default.GetInstance<MainViewModel>();
            instance.SelectVM.SelectedConfig = this;
        }
    }
}
