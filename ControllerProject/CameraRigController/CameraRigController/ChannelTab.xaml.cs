using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace CameraRigController
{
    /// <summary>
    /// Interaction logic for ChannelTab.xaml
    /// </summary>
    public partial class ChannelTab : Page
    {
        public MotorInfo MotorInfo;
        public PropertyGrid Grid;
        public ChannelTab()
        {
            InitializeComponent();
            MotorInfo = new MotorInfo() { StepsPerRevolution = 1000 };
        }
    }
}
