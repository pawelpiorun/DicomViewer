using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class PlotViewModel : NotifyPropertyChanged, ICleanup
    {
        public PlotViewModel()
        {
            Points.ListChanged += OnPointsChanged;
            AppState.AppStateChanged += (s, e) => OnPropertyChanged(nameof(IsAlpha));
        }

        private void OnPointsChanged(object sender, ListChangedEventArgs e)
        {
            OnPropertyChanged(nameof(HasAnyPoints));
        }

        public bool HasAnyPoints
        {
            get { return Points.Count > 0; }
        }
        
        public BindingList<Point> Points
        {
            get { return points ?? (points = new BindingList<Point>()); }
            set
            {
                if (points == value) return;
                points = value;
                OnPropertyChanged();
            }
        }
        BindingList<Point> points;

        public Rect Bounds
        {
            get { return bounds; }
            set
            {
                if (bounds == value) return;
                bounds = value;
                OnPropertyChanged();
            }
        }
        Rect bounds;

        public double XTick
        {
            get { return xTick; }
            set
            {
                if (xTick == value) return;
                xTick = value;
                OnPropertyChanged();
            }
        }
        double xTick = 1.0d;

        public double YTick
        {
            get { return yTick; }
            set
            {
                if (yTick == value) return;
                yTick = value;
                OnPropertyChanged();
            }
        }
        double yTick = 1.0d;

        public int CursorPosition
        {
            get { return cursorPosition; }
            set
            {
                if (cursorPosition == value) return;
                cursorPosition = value;
                OnPropertyChanged();
            }
        }
        int cursorPosition;

        public void Cleanup()
        {
            Points.Clear();
        }

        public bool IsAlpha
        {
            get { return AppState.IsAlpha; }
        }

    }
}
