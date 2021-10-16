using DicomViewer.Lib.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class TestsViewModel : NotifyPropertyChanged
    {
        public TestsViewModel(MainViewModel mainViewModel)
        {
            this.mainViewModel = mainViewModel;
        }
        MainViewModel mainViewModel;

        public ICommand LoadTestImageCommand
        {
            get { return loadTestImageCommand ?? (loadTestImageCommand = new ActionCommand(LoadTestImage)); }
        }
        ICommand loadTestImageCommand;

        public ICommand LoadTestDicomCommand
        {
            get { return loadTestDicomCommand ?? (loadTestDicomCommand = new ActionCommand(LoadTestDicom)); }
        }
        ICommand loadTestDicomCommand;

        public ICommand SaveDicomImageCommand
        {
            get { return saveDicomImageCommand ?? (saveDicomImageCommand = new ActionCommand(SaveDicomImage)); }
        }
        ICommand saveDicomImageCommand;

        public ICommand SaveDicomSeriesCommand
        {
            get { return saveDicomSeriesCommand ?? (saveDicomSeriesCommand = new ActionCommand(SaveDicomSeries)); }
        }
        ICommand saveDicomSeriesCommand;

        private void LoadTestImage()
        {
            var imageSurface = mainViewModel.GetImageSurface() as ImageSurfaceViewModel;
            if (imageSurface != null)
            {
                imageSurface.LoadTestImage();
                mainViewModel.SurfaceViewModel = imageSurface;
            }
        }

        private void LoadTestDicom()
        {
            var dicomSurface = mainViewModel.GetDicomSurface() as DicomImageSurfaceViewModel;
            if (dicomSurface != null)
            {
                dicomSurface.LoadTestDicomImage();
                mainViewModel.SurfaceViewModel = dicomSurface;
            }
        }

        private void SaveDicomImage()
        {
            var dicomSurface = mainViewModel.GetDicomSurface() as DicomImageSurfaceViewModel;
            if (dicomSurface != null)
            {
                dicomSurface.SaveDicomImage();
                mainViewModel.SurfaceViewModel = dicomSurface;
            }
        }

        private void SaveDicomSeries()
        {
            var dicomSurface = mainViewModel.GetDicomSurface(false) as DicomSeriesSurfaceViewModel;
            if (dicomSurface != null)
            {
                dicomSurface.SaveDicomSeries();
                mainViewModel.SurfaceViewModel = dicomSurface;
            }
        }
    }
}
