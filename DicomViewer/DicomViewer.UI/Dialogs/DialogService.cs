using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.ViewModels;

namespace DicomViewer.UI.Dialogs
{
    public class DialogService : IDialogService
    {
        private DialogService() { }

        public static IDialogService Instance
        {
            get => instance ?? (instance = new DialogService());
        }
        static IDialogService instance;

        public void OpenDialog(DialogBaseViewModel dataContext)
        {
            var window = new DialogWindow();
            window.DataContext = dataContext;
            window.ShowDialog();
        }

        public T OpenDialog<T>(DialogBaseViewModel<T> dataContext)
        {
            var window = new DialogWindow();
            window.DataContext = dataContext;
            window.ShowDialog();
            return dataContext.Result;
        }
    }

}
