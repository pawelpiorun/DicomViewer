using DicomViewer.Lib.ViewModels;

namespace DicomViewer.Lib.Interfaces
{
    public interface IDialogService
    {
        void OpenDialog(DialogBaseViewModel viewModel);
        T OpenDialog<T>(DialogBaseViewModel<T> viewModel);
    }
}
