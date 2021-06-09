using CameraRigController.Model;
using CameraRigController.ViewModel;
using CommonServiceLocator;
using GalaSoft.MvvmLight.Ioc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace CameraRigController.View
{
    /// <summary>
    /// Interaction logic for MotorTab.xaml
    /// </summary>
    public partial class MotorTab : UserControl
    {
        public MotorTabModel Data
        {
            get { return (MotorTabModel)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Data.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DataProperty =
            DependencyProperty.Register("Data", typeof(MotorTabModel), typeof(MotorTab), new PropertyMetadata(null));


        public MotorTab()
        {
            InitializeComponent();
        }
    }
}
