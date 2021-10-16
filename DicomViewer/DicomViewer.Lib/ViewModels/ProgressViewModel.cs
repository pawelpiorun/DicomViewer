using DicomViewer.Lib.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.ViewModels
{
    /// <summary>
    /// Singleton progress class, which opens up in a dialog.
    /// </summary>
    public class ProgressViewModel : DialogBaseViewModel
    {
        public ProgressViewModel()
        {
            Title = "Progress";
        }

        public double Progress
        {
            get { return progress; }
            set
            {
                if (Math.Abs(progress - value) < 0.01) return;
                progress = value;
                OnPropertyChanged();
            }
        }
        double progress;

        public void SetProgress(double progress)
        {
            Progress = progress;
        }
    }
}
