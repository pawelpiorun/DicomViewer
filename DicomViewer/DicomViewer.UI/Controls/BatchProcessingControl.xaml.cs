using DicomViewer.Lib.ViewModels;
using DicomViewer.UI.Views;
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

namespace DicomViewer.UI.Controls
{
    /// <summary>
    /// Interaction logic for BatchProcessingControl.xaml
    /// </summary>
    public partial class BatchProcessingControl : UserControl
    {
        public BatchProcessingControl()
        {
            InitializeComponent();
            Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnLoaded;

            var dc = RootGrid.DataContext as BatchProcessingViewModel;
            if (dc is null) return;

            dc.ProcessingDone += OnProcessingDone;
        }

        private void OnProcessingDone(object sender, EventArgs e)
        {
            var window = Window.GetWindow(this) as WindowBase;
            if (window is null) return;

            window.Flash();
        }
    }
}
