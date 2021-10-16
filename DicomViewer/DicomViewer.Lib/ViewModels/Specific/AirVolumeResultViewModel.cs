using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.Threading;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels.Specific
{
    public class AirVolumeResultViewModel : NotifyPropertyChanged, ICleanup
    {
        public AirVolumeResultViewModel(DicomSeriesSurfaceViewModel dicomViewModel)
        {
            this.dicomViewModel = dicomViewModel;
            AppState.AppStateChanged += (s, e) => OnPropertyChanged(nameof(CanShowResults));
        }
        readonly DicomSeriesSurfaceViewModel dicomViewModel;

        public ICommand ShowAirVolumeResultsCommand
        {
            get { return showAirVolumeResultsCommand ?? (showAirVolumeResultsCommand = new ActionCommand(ToggleShowResults)); }
        }
        ICommand showAirVolumeResultsCommand;

        void ToggleShowResults()
        {
            ShouldShowResult = !ShouldShowResult;
            dicomViewModel.SurfaceType = ShouldShowResult ? DicomSeriesSurfaceType.Flat
                : DicomSeriesSurfaceType.Volumetric;
        }

        public bool ShouldShowResult
        {
            get { return shouldShowResult; }
            set
            {
                if (shouldShowResult == value) return;
                shouldShowResult = value;
                OnPropertyChanged();
            }
        }
        bool shouldShowResult;


        public bool CanShowResults
        {
            get { return IsAlpha || canShowResults; }
            set
            {
                if (canShowResults == value) return;
                canShowResults = value;
                OnPropertyChanged();
            }
        }
        bool canShowResults;


        public PlotViewModel PlotViewModel
        {
            get { return plotViewModel ?? (plotViewModel = new PlotViewModel()); }
        }
        PlotViewModel plotViewModel;

        internal void PrepareResult(DicomSeries dicomSeries)
        {
            Threads.UI(() =>
            {
                PlotViewModel.Cleanup();

                var values = ParseCrossSections(dicomSeries);
                var max = values.Max();
                PlotViewModel.Bounds = new Rect(0, 0, values.Count, max);
                PlotViewModel.YTick = (int)(max / 20);
                PlotViewModel.XTick = (int)(values.Count / 25);

                for (int i = 0; i < values.Count; i++)
                    PlotViewModel.Points.Add(new Point(i, values[i]));

                CurrentMinimalSection = ParseMinimalSection(dicomSeries);
                SectionsCoords = ParseSectionsCoords(dicomSeries);
                SectionsNormals = ParseSectionsNormals(dicomSeries);

                SectionsMaxIndex = values.Count - 1;
                CurrentSectionIndex = values.IndexOf(CurrentMinimalSection);

                CanShowResults = true;
            });
        }

        private List<double[]> ParseSectionsNormals(DicomSeries dicomSeries)
        {
            var tag = dicomSeries.Tags[DicomSeries.MinimalCrossSectionsNormalsTag];

            var coordsStrings = tag.Split('}').Select(x => x?.Trim('{', '}', ','))
                .Where(x => !string.IsNullOrEmpty(x));
            List<double[]> finalCoords = new List<double[]>();
            foreach (var txt in coordsStrings)
            {
                var values = txt.Split(',').Select(x => double.Parse(x, NumberStyles.Any)).ToArray();
                finalCoords.Add(values);
            }
            return finalCoords;
        }

        private List<int[]> ParseSectionsCoords(DicomSeries dicomSeries)
        {
            var tag = dicomSeries.Tags[DicomSeries.MinimalCrossSectionsCoordsTag];

            var coordsStrings = tag.Split('}').Select(x => x?.Trim('{', '}',','))
                .Where(x => !string.IsNullOrEmpty(x));
            List<int[]> finalCoords = new List<int[]>();
            foreach (var txt in coordsStrings)
            {
                var values = txt.Split(',').Select(x => int.Parse(x)).ToArray();
                finalCoords.Add(values);
            }
            return finalCoords;
        }

        private double ParseMinimalSection(DicomSeries dicomSeries)
        {
            return double.Parse(
                dicomSeries.Tags[DicomSeries.MinimalCrossSection], NumberStyles.Any);
        }

        private List<double> ParseCrossSections(DicomSeries dicomSeries)
        {
            var tag = dicomSeries.Tags[DicomSeries.MinimalCrossSectionsTag];
            tag = tag.Trim(new char[] { '{', '}' });
            return tag.Split(',').Select(x => double.Parse(x, NumberStyles.Any)).ToList();
        }

        public List<int[]> SectionsCoords
        {
            get { return sectionsCoords; }
            set
            {
                if (sectionsCoords == value) return;
                sectionsCoords = value;
                OnPropertyChanged();
            }
        }
        List<int[]> sectionsCoords;


        public List<double[]> SectionsNormals
        {
            get { return sectionsNormals; }
            set
            {
                if (sectionsNormals == value) return;
                sectionsNormals = value;
                OnPropertyChanged();
            }
        }
        List<double[]> sectionsNormals;


        public int CurrentSectionIndex
        {
            get { return currentSectionIndex; }
            set
            {
                if (currentSectionIndex == value) return;
                currentSectionIndex = value;
                OnPropertyChanged();
                CurrentMinimalSection = PlotViewModel.Points[value].Y;
                PlotViewModel.CursorPosition = value;
                if (SectionsCoords != null && SectionsCoords.Count > value)
                    dicomViewModel.ResliceCenter = SectionsCoords[value];
                if (SectionsNormals != null && SectionsNormals.Count > value)
                    dicomViewModel.ResliceNormal = SectionsNormals[value];
            }
        }
        int currentSectionIndex;

        public double CurrentMinimalSection
        {
            get { return currentMinimalSection; }
            set
            {
                if (currentMinimalSection == value) return;
                currentMinimalSection = value;
                OnPropertyChanged();
            }
        }
        double currentMinimalSection;

        public int SectionsMaxIndex
        {
            get { return sectionsMaxIndex; }
            set
            {
                if (sectionsMaxIndex == value) return;
                sectionsMaxIndex = value;
                OnPropertyChanged();
            }
        }
        int sectionsMaxIndex;

        public ICommand SimulatePlotCommand
        {
            get { return simulatePlotCommand ?? (simulatePlotCommand = new ActionCommand(SimulatePlot)); }
        }
        ICommand simulatePlotCommand;

        void SimulatePlot()
        {
            if (!IsAlpha) return;

            PlotViewModel.Bounds = new Rect(0, 0, 60, 900 / 4);
            PlotViewModel.XTick = 5;
            PlotViewModel.YTick = 20;

            for (int i = 0; i < 30; i++)
            {
                PlotViewModel.Points.Add(new Point(i, 0.25d * Math.Pow(i, 2)));
            }
            for (int i = 30; i <= 60; i++)
            {
                PlotViewModel.Points.Add(new Point(i, 0.25d * Math.Pow(i - 60, 2)));
            }

            SectionsCoords = new List<int[]>();
            for (int i = 0; i <= 60; i++)
                SectionsCoords.Add(new int[] { 250, 250, 2 * i });
            SectionsNormals = new List<double[]>();

            double step = 0.1;
            for (int i = 0; i < 10; i++)
                SectionsNormals.Add(new double[] { 0, i * step, 1 }); // y up
            for (int i = 0; i < 10; i++)
                SectionsNormals.Add(new double[] { 0, 1 - i * step, 1 }); // y down
            for (int i = 0; i < 10; i++)
                SectionsNormals.Add(new double[] { i * step, 0, 1 }); // x up
            for (int i = 0; i < 10; i++)
                SectionsNormals.Add(new double[] { 1 - i * step, 0, 1 }); // x down
            for (int i = 0; i < 10; i++)
                SectionsNormals.Add(new double[] { i * step, i * step, 1 }); // both up
            for (int i = 0; i <= 10; i++)
                SectionsNormals.Add(new double[] { 1 - i * step, 1 - i * step, 1 }); // both down

            SectionsMaxIndex = 60;
            CurrentSectionIndex = 30;
        }

        public bool IsAlpha
        {
            get { return AppState.IsAlpha; }
        }

        public void Cleanup()
        {
            PlotViewModel.Cleanup();
            SectionsCoords?.Clear();
            SectionsNormals?.Clear();
            ShouldShowResult = false;
            CanShowResults = false;
        }
    }
}
