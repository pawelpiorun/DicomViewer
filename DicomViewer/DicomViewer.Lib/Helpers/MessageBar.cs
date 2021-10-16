using DicomViewer.Lib.Threading;
using DicomViewer.Lib.ViewModels;

namespace DicomViewer.Lib.Helpers
{
    public static class MessageBar
    {
        public static void Init()
        {
            messageBar = MessageBarViewModel.Instance;
            messageBar.StatusMessage = "Ready... ";
        }

        static MessageBarViewModel messageBar;

        public static void Message(string message)
        {
            Threads.UI(() => messageBar.StatusMessage = message);
        }

        public static void Start()
        {
            StartPriv();
        }

        public static void Start(string message)
        {
            Message(message);
            StartPriv();
        }

        static void StartPriv()
        {
            Threads.UI(() =>
            {
                messageBar.IsBusy = true;
                messageBar.Progress = 0;
            });
        }

        public static void SetProgress(double progress)
        {
            Threads.UI(() => messageBar.Progress = progress);
        }

        public static void Finish()
        {
            Threads.UI(() =>
            {
                messageBar.IsBusy = false;
                messageBar.Progress = 0;
                messageBar.StatusMessage = "Ready...";
            });
        }
    }
}
