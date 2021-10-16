using DicomViewer.Lib.Algorithms;
using DicomViewer.Lib.Common;
using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.Threading;
using Microsoft.WindowsAPICodePack.Dialogs;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class BatchProcessingViewModel : SurfaceViewModel
    {
        public BatchProcessingViewModel(MainViewModel main) : base(main)
        {
            var algorithms = Algorithms.Algorithms.GetDicomAlgorithms().Where(x =>
                x is IBatchAlgorithm && x is DicomAlgorithm).Select(x => x as DicomAlgorithm);

            foreach (var algorithm in algorithms)
                Targets.Add(new KeyValue<DicomAlgorithm, bool>(algorithm, false));

            Targets.ListChanged += OnTargetsListChanged;
            OnPropertyChanged(nameof(CanStart));

            LoadSettings();
        }

        public ICommand BrowseForFoldersCommand
        {
            get => browseForFoldersCommand ?? (browseForFoldersCommand = new ActionCommand(BrowseForFolders));
        }
        ICommand browseForFoldersCommand;

        void BrowseForFolders()
        {
            var dialog = new CommonOpenFileDialog() { IsFolderPicker = true, Multiselect = true };
            var result = dialog.ShowDialog();

            if (result != CommonFileDialogResult.Ok) return;

            int addedCount = 0;
            foreach (var path in dialog.FileNames)
            {
                if (!InputFiles.Contains(path))
                {
                    InputFiles.Add(path);
                    addedCount++;
                }
            }
            if (addedCount > 0)
                HasPendingChanges = true;

            OnPropertyChanged(nameof(CanStart));
        }

        public ICommand ClearCommand
        {
            get => clearCommand ?? (clearCommand = new ActionCommand(Clear));
        }
        ICommand clearCommand;

        void Clear()
        {
            InputFiles.Clear();
            HasPendingChanges = true;
        }

        public ICommand RemoveCommand
        {
            get => removeCommand ?? (removeCommand = new ActionCommand<string>(Remove));
        }
        ICommand removeCommand;

        void Remove(string path)
        {
            InputFiles.Remove(path);
            OnPropertyChanged(nameof(CanStart));
        }

        public ObservableCollection<string> InputFiles
        {
            get => inputFiles ?? (inputFiles = new ObservableCollection<string>());
        }
        ObservableCollection<string> inputFiles;

        public BindingList<KeyValue<DicomAlgorithm, bool>> Targets
        {
            get => targets ?? (targets = new BindingList<KeyValue<DicomAlgorithm, bool>>());
        }
        BindingList<KeyValue<DicomAlgorithm, bool>> targets;

        private void OnTargetsListChanged(object sender, ListChangedEventArgs e)
        {
            OnPropertyChanged(nameof(CanStart));
            HasPendingChanges = true;
        }

        public string OutputFolder
        {
            get { return outputFolder; }
            set
            {
                if (outputFolder == value) return;
                outputFolder = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(CanStart));
                HasPendingChanges = true;
            }
        }
        string outputFolder;

        public string FinalFolder
        {
            get { return finalFolder; }
            set
            {
                if (finalFolder == value) return;
                finalFolder = value;
                OnPropertyChanged();
            }
        }
        string finalFolder;

        public ICommand BrowseForOutputFolderCommand
        {
            get => browseForOutputFolderCommand ?? (browseForOutputFolderCommand = new ActionCommand(BrowseForOutputFolder));
        }
        ICommand browseForOutputFolderCommand;

        void BrowseForOutputFolder()
        {
            var dialog = new CommonOpenFileDialog() { IsFolderPicker = true };
            var result = dialog.ShowDialog();

            if (result != CommonFileDialogResult.Ok) return;

            OutputFolder = dialog.FileName;
        }

        public ICommand StartCommand
        {
            get => startCommand ?? (startCommand = new ActionCommand(Start));
        }
        ICommand startCommand;

        public ICommand StopCommand
        {
            get => stopCommand ?? (stopCommand = new ActionCommand(Stop));
        }
        ICommand stopCommand;

        public bool CanStart
        {
            get
            {
                var canStart = !string.IsNullOrEmpty(OutputFolder)
                && Targets.Any(x => x.Value)
                && InputFiles.Count > 0;

                if (canStart)
                {
                    LocalMessage = "Waiting for start";
                    TotalMessage = "Files processed: ";
                }
                else
                {
                    LocalMessage = null;
                    TotalMessage = null;
                }

                return canStart;
            }
        }

        void Start()
        {
            if (string.IsNullOrEmpty(OutputFolder)) return;

            var filesCopy = InputFiles.ToList();
            var targetsCopy = targets.ToList();

            Reset();
            IsBusy = true;

            Threads.Background(() =>
            {
                ProcessPriv(filesCopy, targetsCopy);
                Threads.UI(() => IsBusy = false);
            });

        }
        //readonly object locker = new object();

        void ProcessPriv(List<string> filesCopy, List<KeyValue<DicomAlgorithm, bool>> targetsCopy)
        {
            var algorithmsToRun = targetsCopy.Where(x => x.Value).Select(x => x.Key);
            var totalCount = algorithmsToRun.Count() * filesCopy.Count;
            int counter = 0;
            TotalMessage = string.Format("Files processed: 0 / {0}", totalCount);

            var timeStamp = DateTime.Now.ToFileFormat();
            FinalFolder = Path.Combine(OutputFolder, timeStamp);
            Directory.CreateDirectory(FinalFolder);

            foreach (var algorithm in targetsCopy.Where(x => x.Value).Select(x => x.Key))
            {
                if (stopForced) return;

                timeStamp = DateTime.Now.ToFileFormat();
                OutputFilePath = Path.Combine(finalFolder, timeStamp + " " + algorithm.Title + ".csv");

                var header = (algorithm as IBatchAlgorithm)?.GetCsvFormattedHeader();
                var bytesToWrite = Encoding.UTF8.GetBytes(header);
                using (var fileStream = new FileStream(OutputFilePath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
                    fileStream.Write(bytesToWrite, 0, bytesToWrite.Length);

                if (stopForced) return;

                foreach (var dicomFile in filesCopy)
                {
                    LocalProgress = 0.0d;

                    if (stopForced) return;

                    Threads.UI(() => LocalMessage = string.Format("Opening DICOM: {0}", dicomFile));

                    var dicomSeries = new DicomSeries();
                    dicomSeries.Open(dicomFile,
                        async: false,
                        onProgress: progress =>
                        {
                            LocalProgress = progress;
                        });

                    if (stopForced) return;

                    Threads.UI(() =>
                    {
                        LocalMessage = string.Format("Processing DICOM: {0}", dicomFile);
                        LocalProgress = 0.0d;
                    });

                    algorithm.Run(dicomSeries, dispose: false, onDone: null, onProgress: progress => LocalProgress = progress);
                    (algorithm as IBatchAlgorithm)?.DumpIntermediates(FinalFolder);

                    algorithm.Dispose();
                    dicomSeries.Cleanup();
                    dicomSeries = null;

                    if (stopForced) return;

                    Threads.UI(() => LocalMessage = "Writing results...");

                    var result = (algorithm as IBatchAlgorithm)?.GetCsvFormattedResult();
                    var resultBytes = Encoding.UTF8.GetBytes(result);
                    using (var fileStream = new FileStream(OutputFilePath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
                        fileStream.Write(resultBytes, 0, resultBytes.Length);

                    if (stopForced) return;

                    counter++;
                    Threads.UI(() =>
                    {
                        LocalMessage = null;
                        TotalProgress = (double)counter / totalCount;
                        TotalMessage = string.Format("Files processed: {0} / {1}", counter, totalCount);
                    });
                }

                Threads.UI(() =>
                {
                    if (!singleResult)
                        SingleResult = true;
                    else
                        MultipleResults = true;

                    ProcessingDone?.Invoke(this, EventArgs.Empty);
                });
            }
        }
        public event EventHandler ProcessingDone;

        private void Reset()
        {
            FinalFolder = null;
            OutputFilePath = null;
            SingleResult = false;
            MultipleResults = false;
            LocalMessage = null;
            TotalMessage = null;
            LocalProgress = 0.0d;
            TotalProgress = 0.0d;
        }

        void Stop()
        {
            // TODO: Force stop in unmanaged processing
            stopForced = true;
            Threads.UI(() => IsBusy = false);
        }
        bool stopForced;

        public double LocalProgress
        {
            get { return localProgress; }
            set
            {
                if (localProgress == value) return;
                localProgress = value;
                OnPropertyChanged();
            }
        }
        double localProgress;

        public string LocalMessage
        {
            get { return localMessage; }
            set
            {
                if (localMessage == value) return;
                localMessage = value;
                OnPropertyChanged();
            }
        }
        string localMessage;

        public double TotalProgress
        {
            get { return totalProgress; }
            set
            {
                if (totalProgress == value) return;
                totalProgress = value;
                OnPropertyChanged();
            }
        }
        double totalProgress;

        public string TotalMessage
        {
            get { return totalMessage; }
            set
            {
                if (totalMessage == value) return;
                totalMessage = value;
                OnPropertyChanged();
            }
        }
        string totalMessage;

        public bool IsBusy
        {
            get { return isBusy; }
            set
            {
                if (isBusy == value) return;
                isBusy = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(CanStart));
            }
        }
        bool isBusy;

        public string OutputFilePath
        {
            get { return outputFilePath; }
            set
            {
                if (outputFilePath == value) return;
                outputFilePath = value;
                OnPropertyChanged();
            }
        }
        string outputFilePath;

        public bool SingleResult
        {
            get { return singleResult; }
            set
            {
                if (singleResult == value) return;
                singleResult = value;
                OnPropertyChanged();
            }
        }
        bool singleResult;

        public bool MultipleResults
        {
            get { return mutipleResults; }
            set
            {
                if (mutipleResults == value) return;
                mutipleResults = value;
                OnPropertyChanged();
            }
        }
        bool mutipleResults;

        public ICommand OpenCommand
        {
            get => openCommand ?? (openCommand = new ActionCommand(Open));
        }
        ICommand openCommand;

        public ICommand OpenFolderCommand
        {
            get => openFolderCommand ?? (openFolderCommand = new ActionCommand<string>(x => Open(FinalFolder)));
        }
        ICommand openFolderCommand;

        void Open(string path)
        {
            if (File.Exists(path) || Directory.Exists(path))
                Process.Start(path);
            else
                Reset();
        }
        void Open()
        {
            if (SingleResult)
            {
                if (File.Exists(OutputFilePath))
                    Process.Start(OutputFilePath);
                else
                    Reset();
            }
            else if (MultipleResults)
            {
                if (Directory.Exists(OutputFolder))
                    Process.Start(OutputFolder);
                else
                    Reset();
            }
        }

        private static string GetSettingsPath()
        {
            var folder = PathHelpers.GetAppDataOutputPath();
            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            return Path.Combine(folder, "batchprocessing.ini");
        }

        private void LoadSettings()
        {
            var settingsPath = GetSettingsPath();
            if (!File.Exists(settingsPath)) return;

            var components = File.ReadAllLines(settingsPath);
            if (components is null || components.Length != 3) return;

            var paths = components[0].Split(';');
            var algorithms = components[1].Split(';');
            var outputPath = components[2];

            inputFiles = new ObservableCollection<string>(paths);
            OnPropertyChanged(nameof(InputFiles));

            var toConsider = Targets.Where(t => algorithms.Any(x => string.Equals(x, t.Key.Title)));
            foreach (var target in toConsider)
                target.Value = true;

            OutputFolder = outputPath;
            HasPendingChanges = false;
        }

        public ICommand StoreSettingsCommand
        {
            get => storeSettingsCommand ?? (storeSettingsCommand = new ActionCommand(StoreSettings));
        }
        ICommand storeSettingsCommand;

        private void StoreSettings()
        {
            var settingsPath = GetSettingsPath();

            var sb = new StringBuilder();

            var files = InputFiles.ToList();
            foreach (var file in files)
            {
                sb.Append(file);
                if (files.IndexOf(file) != files.Count - 1)
                    sb.Append(";");
            }
            sb.AppendLine();

            var algorithms = Targets.Where(x => x.Value).ToList();
            foreach (var algorithm in algorithms)
            {
                sb.Append(algorithm.Key.Title);
                if (algorithms.IndexOf(algorithm) != algorithms.Count - 1)
                    sb.Append(";");
            }
            sb.AppendLine();

            sb.Append(OutputFolder);

            var bytes = Encoding.UTF8.GetBytes(sb.ToString());
            using (var fileStream = new FileStream(settingsPath, FileMode.Create, FileAccess.Write, FileShare.None))
            {
                fileStream.Write(bytes, 0, bytes.Length);
            }

            HasPendingChanges = false;
        }

        public bool HasPendingChanges
        {
            get { return hasPendingChanges; }
            set
            {
                if (hasPendingChanges == value) return;
                hasPendingChanges = value;
                OnPropertyChanged();
            }
        }
        bool hasPendingChanges;

    }
}
