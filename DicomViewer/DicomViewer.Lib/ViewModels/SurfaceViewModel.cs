using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using System;

namespace DicomViewer.Lib.ViewModels
{
    public class SurfaceViewModel : NotifyPropertyChanged, ISurface, IDisposable
    {
        public SurfaceViewModel(MainViewModel main)
        {
            AppState.AppStateChanged += OnAppStateChanged;
            this.main = main;
        }
        protected MainViewModel main;

        private void OnAppStateChanged(object sender, EventArgs e)
        {
            OnPropertyChanged(nameof(IsAlpha));
        }

        public bool IsAlpha 
        {
            get => AppState.IsAlpha;
        }

        public void Dispose()
        {
            AppState.AppStateChanged -= OnAppStateChanged;
        }
    }
}
