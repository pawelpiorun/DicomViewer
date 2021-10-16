using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.ViewModels;
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
    /// Interaction logic for ImageSurface.xaml
    /// </summary>
    public partial class ImageSurface : UserControl
    {
        public ImageSurface()
        {
            InitializeComponent();
            this.Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            this.Loaded -= OnLoaded;
            this.Unloaded += OnUnloaded;

            KeyboardHelpers.Instance.KeyDown += OnKeyDown;
        }

        private void OnKeyDown(object sender, KeyEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftCtrl))
            {
                var dc = DataContext as ImageSurfaceViewModel;
                if (dc is null) return;

                if (Keyboard.IsKeyDown(Key.Left))
                {
                    dc.LoadPreviousImage();
                }
                else if (Keyboard.IsKeyDown(Key.Right))
                {
                    dc.LoadNextImage();
                }
            }
        }

        private void OnUnloaded(object sender, RoutedEventArgs e)
        {
            this.Unloaded -= OnUnloaded;
            KeyboardHelpers.Instance.KeyDown -= OnKeyDown;
        }
    }
}
