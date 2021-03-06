using DicomViewer.Lib.ViewModels;
using DicomViewer.UI.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
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
using System.Windows.Shapes;

namespace DicomViewer.UI.Controls
{
    /// <summary>
    /// Interaction logic for DicomSeriesFlatSurface.xaml
    /// </summary>
    public partial class DicomSeriesFlatSurface : UserControl
    {
        public DicomSeriesFlatSurface()
        {
            InitializeComponent();

            this.Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            this.Loaded -= OnLoaded;

            var vm = DataContext as DicomSeriesSurfaceViewModel;
            if (vm is null) return;

            //vm.PropertyChanged += OnVmPropertyChanged;
            vm.ResetReslicers += OnResetReslicers;
        }

        public VtkRenderSync Sync
        {
            get { return (VtkRenderSync)GetValue(SyncProperty); }
        }

        public static readonly DependencyProperty SyncProperty =
            DependencyProperty.Register(
                nameof(Sync),
                typeof(VtkRenderSync),
                typeof(DicomSeriesFlatSurface),
                new PropertyMetadata(CreateSync()));

        private static VtkRenderSync CreateSync()
        {
            return new VtkRenderSync();
        }

        //private void OnVmPropertyChanged(object sender, PropertyChangedEventArgs e)
        //{
        //    var vm = sender as DicomSeriesSurfaceViewModel;
        //    if (vm is null) return;

        //    if (e.PropertyName == nameof(vm.XNormal))
        //        Sync.SetAxisNormal(0, vm.XNormal);
        //    else if (e.PropertyName == nameof(vm.YNormal))
        //        Sync.SetAxisNormal(1, vm.YNormal);
        //    else if (e.PropertyName == nameof(vm.ZNormal))
        //        Sync.SetAxisNormal(2, vm.ZNormal);
        //    else if (e.PropertyName == nameof(vm.ResliceCenter))
        //        Sync.SetResliceCenter(vm.ResliceCenter);
        //}
        private void OnResetReslicers(object sender, EventArgs e)
        {
            Sync.ResetReslicers();
        }
    }
}
