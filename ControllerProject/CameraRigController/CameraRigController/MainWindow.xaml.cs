using CameraRigController.FieldGrid;
using CameraRigController.ViewModel;
using System.Windows;
using System.Windows.Controls;

namespace CameraRigController
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            Loaded += MainWindow_Loaded;
            Initial­izeComponent();
            
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            for (int i = 0; i < TabCTL.Items.Count; i++)
            {
                TabCTL.SelectedIndex = i;
                TabCTL.Refresh();
            }
            if (TabCTL.Items.Count != 0) TabCTL.SelectedIndex = 0;
        }
    }
}
