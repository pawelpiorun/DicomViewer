using DicomViewer.Lib.Algorithms;
using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.Threading;
using DicomViewer.Lib.ViewModels.Specific;
using Microsoft.WindowsAPICodePack.Dialogs;
using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class DicomSeriesSurfaceViewModel : SurfaceViewModel, IWithAlgorithms
    {
        public DicomSeriesSurfaceViewModel(MainViewModel main) : base(main) { }

        public DicomSeries DicomSeries
        {
            get { return dicomSeries; }
            set
            {
                if (dicomSeries == value) return;
                dicomSeries = value;
                OnPropertyChanged();
            }
        }
        DicomSeries dicomSeries;

        public bool IsSeriesLoaded
        {
            get { return isSeriesLoaded; }
            set
            {
                if (isSeriesLoaded == value) return;
                isSeriesLoaded = value;
                OnPropertyChanged();
            }
        }
        bool isSeriesLoaded;

        public void OpenDicomSeries(string path = null)
        {
            string folderPath;
            if (!string.IsNullOrEmpty(path) && Directory.Exists(path))
                folderPath = path;
            else
            {
                var dialog = new CommonOpenFileDialog() { IsFolderPicker = true };
                var result = dialog.ShowDialog();

                if (result == CommonFileDialogResult.Ok && !string.IsNullOrEmpty(dialog.FileName))
                {
                    folderPath = dialog.FileName;
                }
                else return;

            }

            if (IsSeriesLoaded)
                IsSeriesLoaded = false;

            OpenDicomSeriesPriv(folderPath);
        }

        private void OpenDicomSeriesPriv(string folderPath)
        {
            if (string.IsNullOrEmpty(folderPath)) return;

            if (dicomSeries != null)
            {
                dicomSeries.Cleanup();
                dicomSeries = null;
            }
            dicomSeries = new DicomSeries();
            AirVolumeResult.Cleanup();
            OnPropertyChanged(nameof(DicomSeries));

            var progressViewModel = new ProgressViewModel()
            {
                SubTitle = "Reading DICOM series...",
                Message = folderPath
            };
            progressViewModel.Show();

            dicomSeries.Open(folderPath,
                async: true,
                onProgress: progressViewModel.SetProgress,
                onDone: () =>
                {
                    Threads.UI(() =>
                    {
                        progressViewModel.Close();
                        IsSeriesLoaded = dicomSeries.IsLoaded;
                    });
                });
        }

        public void SaveDicomSeries()
        {
            var folderPath = "c:\\test\\dicom";
            DicomSeries.Save(folderPath);
        }

        public DicomSeriesSurfaceType SurfaceType
        {
            get { return currentSurface; }
            set
            {
                if (currentSurface == value) return;
                currentSurface = value;
                OnPropertyChanged();
            }
        }
        DicomSeriesSurfaceType currentSurface = DicomSeriesSurfaceType.Volumetric;


        public ObservableCollection<IAlgorithm> Algorithms
        {
            get => algorithms ?? (algorithms = new ObservableCollection<IAlgorithm>(Lib.Algorithms.Algorithms.GetDicomAlgorithmsWithOutput()));
        }
        ObservableCollection<IAlgorithm> algorithms;

        public void RefreshAlgorithms()
        {
            algorithms = null;
            OnPropertyChanged(nameof(Algorithms));
        }

        public ICommand RunAlgorithmCommand
        {
            get => runAlgorithmCommand ?? (runAlgorithmCommand = new ActionCommand<DicomAlgorithm>(RunAlgorithm));
        }
        ICommand runAlgorithmCommand;

        void RunAlgorithm(DicomAlgorithm algorithm)
        {
            var progressViewModel = new ProgressViewModel()
            {
                Title = "Processing...",
                SubTitle = "Running algorithm:",
                Message = algorithm.Title
            };

            Action<double> onProgress = null;
            if (algorithm.SupportsProgress)
            {
                progressViewModel.Show();
                onProgress = progressViewModel.SetProgress;
            }

            Threads.Background(() =>
            {
                void onDone()
                {
                    progressViewModel?.Close();
                    if (algorithm is AirVolumeAlgorithm airVolume)
                    {
                        PresetsViewModel.SelectPreset(PresetType.AirVolume);
                        AirVolumeResult.PrepareResult(dicomSeries);
                    }
                    else if (algorithm is MandibularBaseAlgorithm)
                        PresetsViewModel.SelectPreset(PresetType.CTAAA2);
                }
                algorithm.Run(DicomSeries, true, onDone, onProgress);
                ResultsViewModel.AddResults(algorithm);
            });
        }

        public ICommand SwitchSurfaceCommand
        {
            get => switchSurfaceCommand ?? (switchSurfaceCommand = new ActionCommand(SwitchSurface));
        }
        ICommand switchSurfaceCommand;

        void SwitchSurface()
        {
            if (SurfaceType == DicomSeriesSurfaceType.Flat)
                SurfaceType = DicomSeriesSurfaceType.Volumetric;
            else if (SurfaceType == DicomSeriesSurfaceType.Volumetric)
                SurfaceType = DicomSeriesSurfaceType.Flat;
        }

        public ICommand SwitchCursorsCommand
        {
            get => switchCursorsCommand ?? (switchCursorsCommand = new ActionCommand(SwitchCursors));
        }
        ICommand switchCursorsCommand;

        void SwitchCursors()
        {
            ShowResliceCursors = !ShowResliceCursors;
        }

        public ICommand ResetCursorsCommand
        {
            get => resetCursorsCommand ?? (resetCursorsCommand = new ActionCommand(ResetCursors));
        }
        ICommand resetCursorsCommand;

        void ResetCursors()
        {
            ResetReslicers?.Invoke(this, EventArgs.Empty);
        }
        public event EventHandler ResetReslicers;

        public bool ShowResliceCursors
        {
            get { return showResliceCursors; }
            set
            {
                if (showResliceCursors == value) return;
                showResliceCursors = value;
                OnPropertyChanged();
            }
        }
        bool showResliceCursors = true;


        public int[] ResliceCenter
        {
            get { return resliceCenter; }
            set
            {
                if (resliceCenter == value) return;
                resliceCenter = value;
                OnPropertyChanged();
            }
        }
        int[] resliceCenter;

        public double[] ResliceNormal
        {
            get { return resliceNormal; }
            set
            {
                if (resliceNormal == value) return;
                resliceNormal = value;
                OnPropertyChanged();
            }
        }
        double[] resliceNormal;

        public ResultsViewModel ResultsViewModel
        {
            get { return resultsViewModel ?? (resultsViewModel = new ResultsViewModel()); }
        }
        ResultsViewModel resultsViewModel;

        public PresetsViewModel PresetsViewModel
        {
            get { return presetsViewModel ?? (presetsViewModel = new PresetsViewModel()); }
        }
        PresetsViewModel presetsViewModel;

        public AirVolumeResultViewModel AirVolumeResult
        {
            get { return airVolumeResult ?? (airVolumeResult = new AirVolumeResultViewModel(this)); }
        }
        AirVolumeResultViewModel airVolumeResult;
    }

    public enum DicomSeriesSurfaceType
    {
        Flat,
        Volumetric
    }
}
