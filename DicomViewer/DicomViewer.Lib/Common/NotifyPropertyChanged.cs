using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace DicomViewer.Lib.Common
{
    public class NotifyPropertyChanged : INotifyPropertyChanged
    {
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public event PropertyChangedEventHandler PropertyChanged;
    }
}
