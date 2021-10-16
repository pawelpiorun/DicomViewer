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
    /// Interaction logic for DicomImageSurface.xaml
    /// </summary>
    public partial class DicomImageSurface : UserControl
    {
        public DicomImageSurface()
        {
            InitializeComponent();
            Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (ApplyRotation)
                ApplyImageRotation();
        }

        public bool ApplyRotation
        {
            get { return (bool)GetValue(ApplyRotationProperty); }
            set { SetValue(ApplyRotationProperty, value); }
        }

        public static readonly DependencyProperty ApplyRotationProperty =
            DependencyProperty.Register("ApplyRotation", typeof(bool), typeof(DicomImageSurface), new PropertyMetadata(true, OnApplyRotationChanged));

        private static void OnApplyRotationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var surface = d as DicomImageSurface;
            var apply = (bool) e.NewValue;

            if (surface != null && apply)
                surface.ApplyImageRotation();
        }

        void ApplyImageRotation()
        {
            SingleImage.RenderTransformOrigin = new Point(0.5, 0.5);
            SingleImage.RenderTransform = new RotateTransform(180);
        }
    }
}
