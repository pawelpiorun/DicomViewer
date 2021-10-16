using DicomViewer.Lib.ViewModels;
using System.Windows;

namespace DicomViewer.UI.Views
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : WindowBase
    {
        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnLoaded;

            var dataContext = DataContext as MainViewModel;
            dataContext.NotifyViewLoaded();

            Unloaded += OnUnloaded;
        }

        private void OnUnloaded(object sender, RoutedEventArgs e)
        {
            Cleanup();
        }

        private void Cleanup()
        {

        }
    }
}
