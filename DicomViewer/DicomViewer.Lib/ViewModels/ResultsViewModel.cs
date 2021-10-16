using DicomViewer.Lib.Algorithms;
using DicomViewer.Lib.Common;
using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Threading;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class ResultsViewModel : NotifyPropertyChanged
    {
        public void AddResults(AlgorithmBase algorithm)
        {
            if (algorithm is null) return;

            var message = algorithm.Result;

            var result = new AlgorithmResult
            {
                Title = algorithm.Title,
                Index = resultsCounter,
                Success = algorithm.Success,
                TimeStamp = DateTime.Now.ToString(),
                Duration = TimeSpan.FromMilliseconds(algorithm.Duration).Format(),
                Message = message
            };

            resultsCounter++;
            Threads.UI(() =>
            {
                Results.Add(result);
                if (!AreResultsPresent)
                    AreResultsPresent = true;
            });
        }
        int resultsCounter = 0;

        public ObservableCollection<AlgorithmResult> Results
        {
            get => results ?? (results = new ObservableCollection<AlgorithmResult>());
        }
        ObservableCollection<AlgorithmResult> results;

        public ICommand ClearResultsCommand
        {
            get => clearResultsCommand ?? (clearResultsCommand = new ActionCommand(ClearResults));
        }
        ICommand clearResultsCommand;

        void ClearResults()
        {
            Results.Clear();
            resultsCounter = 0;
            AreResultsPresent = false;
        }

        public bool AreResultsPresent
        {
            get { return areResultsPresent; }
            set
            {
                if (areResultsPresent == value) return;
                areResultsPresent = value;
                OnPropertyChanged();
            }
        }
        bool areResultsPresent;
    }

    public class AlgorithmResult : NotifyPropertyChanged
    {
        public int Index { get; set; }
        public string Title { get; set; }
        public bool Success { get; set; }
        public string Duration { get; set; }
        public string TimeStamp { get; set; }
        public string Message { get; set; }
    }
}
