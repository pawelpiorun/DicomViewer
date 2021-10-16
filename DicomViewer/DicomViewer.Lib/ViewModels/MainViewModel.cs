using DicomViewer.Lib.Algorithms;
using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using Microsoft.Win32;
using Microsoft.WindowsAPICodePack.Dialogs;
using Serilog;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class MainViewModel : NotifyPropertyChanged
    {
        public MainViewModel()
        {
            //MessageBar.Init();
        }

        public string SubTitle
        {
            get { return subTitle; }
            set
            {
                if (subTitle == value) return;
                subTitle = value;
                OnPropertyChanged();
            }
        }
        string subTitle;

        internal void UpdateSubtitle(string filePath)
        {
            this.SubTitle = filePath;
        }

        public void NotifyViewLoaded()
        {
            // preload presets
            //Presets.Instance.GetDefaultPresets();

            CommandLineArgs();
        }

        void CommandLineArgs()
        {
            var commandLineArgs = Environment.GetCommandLineArgs();
            var seriesPath = string.Empty;
            var imagePath = string.Empty;
            foreach (var path in commandLineArgs)
            {
                if (string.Equals(path, Assembly.GetEntryAssembly().Location, StringComparison.OrdinalIgnoreCase)) continue;

                if (Directory.Exists(path))
                {
                    seriesPath = path;
                    break;
                }
                else if (File.Exists(path))
                {
                    imagePath = path;
                    break;
                }
            }

            if (!string.IsNullOrEmpty(seriesPath))
            {
                Log.Information($"Command line args: {seriesPath}");
                ProcessingMode = ProcessingMode.Single;
                OpenDicomSeries(seriesPath);
            }
            else if (!string.IsNullOrEmpty(imagePath))
            {
                Log.Information($"Command line args: {imagePath}");
                ProcessingMode = ProcessingMode.Single;
                if (string.Equals(Path.GetExtension(imagePath), "dcm"))
                    OpenDicom(imagePath);
                else
                    OpenImage(imagePath);
            }
        }

        // TODO: Show image properties
        public PanelViewModel PanelViewModel
        {
            get { return panelViewModel ?? (panelViewModel = new PanelViewModel()); }
        }
        PanelViewModel panelViewModel;

        #region Surfaces

        public ISurface SurfaceViewModel
        {
            get { return surfaceViewModel; }
            internal set
            {
                if (surfaceViewModel == value) return;
                surfaceViewModel = value;
                OnPropertyChanged(nameof(SurfaceViewModel));
            }
        }
        ISurface surfaceViewModel;

        public List<ISurface> Surfaces
        {
            get { return surfaces ?? (surfaces = new List<ISurface>()); }
        }
        List<ISurface> surfaces;

        internal ISurface GetImageSurface()
        {
            var imageSurface = Surfaces.Where(x => x is ImageSurfaceViewModel).FirstOrDefault();
            if (imageSurface == null)
            {
                var newSurface = new ImageSurfaceViewModel(this);
                Surfaces.Add(newSurface);
                imageSurface = newSurface;
            }

            return imageSurface;
        }

        internal ISurface GetDicomSurface(bool single = true)
        {
            var dicomSurface = single
                ? Surfaces.Where(x => x is DicomImageSurfaceViewModel).FirstOrDefault()
                : Surfaces.Where(x => x is DicomSeriesSurfaceViewModel).FirstOrDefault();
            if (dicomSurface == null)
            {
                if (single)
                    Surfaces.Add(dicomSurface = new DicomImageSurfaceViewModel(this));
                else
                    Surfaces.Add(dicomSurface = new DicomSeriesSurfaceViewModel(this));
            }

            return dicomSurface;
        }

        internal ISurface GetBatchProcessingSurface()
        {
            var surface = Surfaces.Where(x => x is BatchProcessingViewModel).FirstOrDefault();
            if (surface == null)
                Surfaces.Add(surface = new BatchProcessingViewModel(this));

            return surface;
        }

        #endregion

        #region Ribbon commands

        public ICommand OpenImageCommand
        {
            get { return openImageCommand ?? (openImageCommand = new ActionCommand(OpenImage)); }
        }
        ICommand openImageCommand;

        private void OpenImage() => OpenImage(null);
        private void OpenImage(string filePath = null)
        {
            if (GetImageSurface() is ImageSurfaceViewModel imageSurface)
            {
                imageSurface.OpenImage(filePath);
                SurfaceViewModel = imageSurface;
                SubTitle = filePath;
            }
        }

        public ICommand OpenDicomCommand
        {
            get { return openDicomCommand ?? (openDicomCommand = new ActionCommand(OpenDicom)); }
        }
        ICommand openDicomCommand;

        private void OpenDicom() => OpenDicom(null);
        private void OpenDicom(string filePath = null)
        {
            if (GetDicomSurface() is DicomImageSurfaceViewModel dicomSurface)
            {
                dicomSurface.OpenDicom(filePath);
                SurfaceViewModel = dicomSurface;
                SubTitle = filePath;
            }
        }

        public ICommand OpenDicomSeriesCommand
        {
            get { return openDicomSeriesCommand ?? (openDicomSeriesCommand = new ActionCommand(OpenDicomSeries)); }
        }
        ICommand openDicomSeriesCommand;

        private void OpenDicomSeries() => OpenDicomSeries(null);
        private void OpenDicomSeries(string path)
        {
            if (GetDicomSurface(false) is DicomSeriesSurfaceViewModel dicomSurface)
            {
                if (string.IsNullOrEmpty(path))
                    path = BrowseForFolder();
                if (string.IsNullOrEmpty(path)) return;

                dicomSurface.OpenDicomSeries(path);
                SurfaceViewModel = dicomSurface;
                SubTitle = path;
            }
        }

        string BrowseForFolder()
        {
            var dialog = new CommonOpenFileDialog() { IsFolderPicker = true };
            var result = dialog.ShowDialog();

            if (result == CommonFileDialogResult.Ok && !string.IsNullOrEmpty(dialog.FileName))
                return dialog.FileName;

            return null;
        }

        public ICommand TestVTKCommand
        {
            get => testVTKCommand ?? (testVTKCommand = new ActionCommand(TestVTK));
        }
        ICommand testVTKCommand;

        void TestVTK()
        {
            // TODO: VTK test
            //LoadDicomSurface();
            //var dicomSurface = Surfaces.Where(x => x is DicomSurfaceViewModel).FirstOrDefault() as DicomSurfaceViewModel;
            //if (dicomSurface != null)
            //{
            //    dicomSurface.TestVTK();
            //}
        }

        public ICommand OpenFileCommand
        {
            get => openFileCommand ?? (openFileCommand = new ActionCommand(OpenFile));
        }
        ICommand openFileCommand;

        void OpenFile()
        {
            var dialog = new OpenFileDialog
            {
                DefaultExt = ".dcm",
                Filter = "DICOM files (*.dcm)|*.dcm|Image files (*.jpg, *.jpeg, *.bmp, *.png)|*.jpg;*.jpeg;*.bmp;*.png|All files (*.*)|*.*"
            };
            var result = dialog.ShowDialog();

            string filePath;
            if (result.HasValue && !string.IsNullOrEmpty(dialog.FileName))
            {
                filePath = dialog.FileName;
            }
            else return;

            var extension = Path.GetExtension(filePath);
            if (string.Equals(extension, ".dcm", StringComparison.InvariantCultureIgnoreCase))
            {
                OpenDicom(filePath);
            }
            else
            {
                OpenImage(filePath);
            }
        }

        public ICommand ShowBatchProcessingCommand
        {
            get { return showBatchProcessingCommand ?? (showBatchProcessingCommand = new ActionCommand(ShowBatchProcessing)); }
        }
        ICommand showBatchProcessingCommand;

        public ICommand ShowSingleProcessingCommand
        {
            get { return showSingleProcessingCommand ?? (showSingleProcessingCommand = new ActionCommand(ShowSingleProcessing)); }
        }
        ICommand showSingleProcessingCommand;

        public ICommand BackToHomeCommand
        {
            get { return backToHomeCommand ?? (backToHomeCommand = new ActionCommand(GoBackToHome)); }
        }
        ICommand backToHomeCommand;

        void ShowBatchProcessing()
        {
            ProcessingMode = ProcessingMode.Batch;
        }

        public ISurface BatchProcessingViewModel
        {
            get => batchProcessingViewModel ?? (batchProcessingViewModel = GetBatchProcessingSurface());
        }
        ISurface batchProcessingViewModel;

        void ShowSingleProcessing()
        {
            ProcessingMode = ProcessingMode.Single;
        }

        void GoBackToHome()
        {
            ProcessingMode = ProcessingMode.Unknown;
        }

        public ICommand ShowImageComparationCommand
        {
            get => showImageComparationCommand ?? (showImageComparationCommand = new ActionCommand(ShowImageComparation));
        }
        ICommand showImageComparationCommand;
        
        private void ShowImageComparation()
        {
            ProcessingMode = ProcessingMode.Dual;
        }

        public ISurface ImageComparationViewModel
        {
            get => imageComparationViewModel ?? (imageComparationViewModel = new ImageComparationViewModel(this));
        }
        ISurface imageComparationViewModel;

        public ProcessingMode ProcessingMode
        {
            get { return processingMode; }
            set
            {
                if (processingMode == value) return;
                processingMode = value;
                OnPropertyChanged();
            }
        }
        ProcessingMode processingMode;

#endregion

        public TestsViewModel TestsViewModel
        {
            get => testsViewModel ?? (testsViewModel = new TestsViewModel(this));
        }
        TestsViewModel testsViewModel;

        public static bool ShowAlphaSwitch
        {
            get
            {
#if DEBUG
                return true;
#else
                return AppState.IsAlpha;
#endif
            }
        }

        public bool IsExperimental
        {
            get { return AppState.IsAlpha; }
            set
            {
                if (value == AppState.IsAlpha) return;
                AppState.IsAlpha = value;
                OnPropertyChanged();

                foreach (var surface in Surfaces.ToList())
                {
                    if (surface is IWithAlgorithms withAlgorithms)
                        withAlgorithms.RefreshAlgorithms();
                }
            }
        }

        public ICommand ChangeExperimentalModeCommand
        {
            get => changeExperimentalModeCommand ?? (changeExperimentalModeCommand = new ActionCommand(() => IsExperimental = !IsExperimental));
        }
        ICommand changeExperimentalModeCommand;
    }

    public enum ProcessingMode
    {
        Unknown,
        Single,
        Batch,
        Dual
    }
}
