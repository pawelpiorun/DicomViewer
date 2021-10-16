using DicomViewer.Lib.Common;
using System;

namespace DicomViewer.Lib.ViewModels
{
    /// <summary>
    /// Singleton message bar class, which is shown as status bar with message and progress./>
    /// </summary>
    public class MessageBarViewModel : NotifyPropertyChanged
    {
        private MessageBarViewModel() { }

        public static MessageBarViewModel Instance
        {
            get => instance ?? (instance = new MessageBarViewModel());
        }
        static MessageBarViewModel instance;

        public bool IsBusy
        {
            get { return isBusy; }
            set
            {
                if (isBusy == value) return;
                isBusy = value;
                OnPropertyChanged();
            }
        }
        bool isBusy;

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

        public string StatusMessage
        {
            get { return statusMessage; }
            set
            {
                if (string.Equals(statusMessage, value)) return;
                statusMessage = value;
                OnPropertyChanged();
            }
        }
        string statusMessage;
    }
}
