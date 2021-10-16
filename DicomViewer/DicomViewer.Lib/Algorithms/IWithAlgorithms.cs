using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Input;

namespace DicomViewer.Lib.Algorithms
{
    public interface IWithAlgorithms
    {
        ObservableCollection<IAlgorithm> Algorithms { get; }
        ICommand RunAlgorithmCommand { get; }
        void RefreshAlgorithms();
    }
}
