using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using Microsoft.WindowsAPICodePack.Dialogs;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class ImageComparationViewModel : SurfaceViewModel
    {
        public ImageComparationViewModel(MainViewModel main)
            : base(main)
        {
            LoadSettings();
        }

        public ICommand BrowseCommand
        {
            get => browseCommand ?? (browseCommand = new ActionCommand<string>(Browse));
        }
        ICommand browseCommand;

        private void Browse(string obj)
        {
            switch (obj)
            {
                case "1":
                    BrowseForFolder((path) => FirstPath = path, GetInitialFolder(FirstPath));
                    break;
                case "2":
                    BrowseForFolder((path) => SecondPath = path, GetInitialFolder(SecondPath));
                    break;
                case "3":
                    BrowseForFile(".csv", (file) => ResultsPath = file, GetInitialFolder(ResultsPath));
                    break;
                default:
                    break;
                    // run Forest!
            }
        }

        private string GetInitialFolder(string resultsPath)
        {
            if (string.IsNullOrEmpty(resultsPath)) return null;
            return Path.GetDirectoryName(resultsPath);
        }

        private void BrowseForFolder(Action<string> setter, string initialFolder = null)
        {
            var dialog = new CommonOpenFileDialog() { IsFolderPicker = true, Multiselect = false };
            if (!string.IsNullOrEmpty(initialFolder))
                dialog.InitialDirectory = initialFolder;
            var result = dialog.ShowDialog();

            if (result != CommonFileDialogResult.Ok) return;

            setter(dialog.FileName);
        }

        private void BrowseForFile(string extension, Action<string> setter, string initialFolder = null)
        {
            var dialog = new OpenFileDialog();

            if (!string.IsNullOrEmpty(initialFolder))
                dialog.InitialDirectory = initialFolder;
            dialog.DefaultExt = extension;
            dialog.Filter = "CSV (*" + extension + ") |*" + extension + ";";
            var result = dialog.ShowDialog();

            if (result != DialogResult.OK) return;

            setter(dialog.FileName);
        }

        public string FirstPath
        {
            get { return firstPath; }
            set
            {
                if (firstPath == value) return;
                firstPath = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(CanRun));
                HasPendingChanges = true;
            }
        }
        string firstPath;

        public string SecondPath
        {
            get { return secondPath; }
            set
            {
                if (secondPath == value) return;
                secondPath = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(CanRun));
                HasPendingChanges = true;
            }
        }
        string secondPath;

        public string ResultsPath
        {
            get { return resultsPath; }
            set
            {
                if (resultsPath == value) return;
                resultsPath = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(CanRun));
                HasPendingChanges = true;
            }
        }
        string resultsPath;

        public ICommand CompareCommand
        {
            get => compareCommand ?? (compareCommand = new ActionCommand(RunComparation));
        }
        ICommand compareCommand;

        public bool CanRun
        {
            get
            {
                return !string.IsNullOrEmpty(FirstPath)
                    && !string.IsNullOrEmpty(SecondPath)
                    && !string.IsNullOrEmpty(ResultsPath);
            }
        }

        private void RunComparation()
        {
            var dicomComp = new DicomFittingComparationViewModel(FirstPath, SecondPath, ResultsPath);
            dicomComp.Load(onDone: () => ComparationViewModel = dicomComp);
        }

        public NotifyPropertyChanged ComparationViewModel
        {
            get { return comparationViewModel; }
            set
            {
                if (comparationViewModel == value) return;
                comparationViewModel = value;
                OnPropertyChanged();
            }
        }
        NotifyPropertyChanged comparationViewModel;


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


        public ICommand SaveCommand
        {
            get => saveCommand ?? (saveCommand = new ActionCommand(SaveSettings));
        }
        ICommand saveCommand;

        private void SaveSettings()
        {
            var settingsPath = GetSettingsPath();

            var sb = new StringBuilder();

            sb.AppendLine(FirstPath);
            sb.AppendLine(SecondPath);
            sb.AppendLine(ResultsPath);

            var bytes = Encoding.UTF8.GetBytes(sb.ToString());
            using (var fileStream = new FileStream(settingsPath, FileMode.Create, FileAccess.Write, FileShare.None))
            {
                fileStream.Write(bytes, 0, bytes.Length);
            }

            HasPendingChanges = false;
        }

        private static string GetSettingsPath()
        {
            var folder = PathHelpers.GetAppDataOutputPath();
            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            return Path.Combine(folder, "imagecomparation.ini");
        }

        private void LoadSettings()
        {
            var settingsPath = GetSettingsPath();
            if (!File.Exists(settingsPath)) return;

            var components = File.ReadAllLines(settingsPath);
            if (components is null || components.Length != 3) return;

            firstPath = components[0];
            secondPath = components[1];
            resultsPath = components[2];
            OnPropertyChanged(nameof(FirstPath));
            OnPropertyChanged(nameof(SecondPath));
            OnPropertyChanged(nameof(ResultsPath));
        }
    }
}
