/*
  In App.xaml:
  <Application.Resources>
      <vm:ViewModelLocator xmlns:vm="clr-namespace:CameraRigController"
                           x:Key="Locator" />
  </Application.Resources>
  
  In the View:
  DataContext="{Binding Source={StaticResource Locator}, Path=ViewModelName}"

  You can also use Blend to do all this with the tool's support.
  See http://www.galasoft.ch/mvvm
*/

using CameraRigController.FieldGrid;
using CameraRigController.FieldGrid.Editor.ViewModel;
using CameraRigController.Model;
using CommonServiceLocator;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Ioc;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace CameraRigController.ViewModel
{
    /// <summary>
    /// This class contains static references to all the view models in the
    /// application and provides an entry point for the bindings.
    /// </summary>
    public class ViewModelLocator
    {
        /// <summary>
        /// Initializes a new instance of the ViewModelLocator class.
        /// </summary>
        public ViewModelLocator()
        {
            ServiceLocator.SetLocatorProvider(() => SimpleIoc.Default);

            ////if (ViewModelBase.IsInDesignModeStatic)
            ////{
            ////    // Create design time view services and models
            ////    SimpleIoc.Default.Register<IDataService, DesignDataService>();
            ////}
            ////else
            ////{
            ////    // Create run time view services and models
            ////    SimpleIoc.Default.Register<IDataService, DataService>();
            ////}

            SimpleIoc.Default.Register<MainViewModel>();
            SimpleIoc.Default.Register<MotorTabsVM>();
            SimpleIoc.Default.Register<MotorTabModel>();
            SimpleIoc.Default.Register<FieldGridVM>(() => new FieldGridVM() { Target = Motor1 });
        }

        public MainViewModel Main
        {
            get
            {
                return ServiceLocator.Current.GetInstance<MainViewModel>();
            }
        }

        public MotorTabsVM MotorTabs
        {
            get
            {
                return ServiceLocator.Current.GetInstance<MotorTabsVM>();
            }
        }

        public FieldGridVM FieldGrid1
        {
            get
            {
                return ServiceLocator.Current.GetInstance<FieldGridVM>();
            }
        }

        public MotorTabModel Motor1
        {
            get
            {
                return ServiceLocator.Current.GetInstance<MotorTabModel>();
            }
        }


        public static void Cleanup()
        {
            // TODO Clear the ViewModels
        }
    }
}