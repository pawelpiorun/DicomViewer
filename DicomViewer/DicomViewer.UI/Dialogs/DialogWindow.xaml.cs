using System;
using System.Windows;
using DicomViewer.Lib.ViewModels;
using DicomViewer.UI.Views;

namespace DicomViewer.UI.Dialogs
{
    /// <summary>
    /// Interaction logic for DialogWindow.xaml
    /// </summary>
    public partial class DialogWindow : WindowBase
    {
        public DialogWindow()
        {
            InitializeComponent();
            Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnLoaded;
            this.SizeToContent = SizeToContent.WidthAndHeight;

            var dc = DataContext as DialogBaseViewModel;
            if (dc != null)
                dc.CloseRequested += OnDataContextCloseRequested;

            Unloaded += OnUnloaded;
        }

        private void OnUnloaded(object sender, RoutedEventArgs e)
        {
            Unloaded -= OnUnloaded;
            var dc = DataContext as DialogBaseViewModel;
            if (dc != null)
                dc.CloseRequested -= OnDataContextCloseRequested;
        }

        private void OnDataContextCloseRequested(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
