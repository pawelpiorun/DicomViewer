using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.ViewModels;
using DicomViewer.UI.Dialogs;
using DicomViewer.UI.Views;
using Serilog;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;

namespace DicomViewer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            InitializeComponent();

            CheckExperimental();
            RegisterServices();

            InitializeLog();
            StartMainWindow();
        }

        private void CheckExperimental()
        {
            var commandLineArgs = Environment.GetCommandLineArgs();
            bool isExperimental = commandLineArgs.Any(x => string.Equals(x, "ALPHA", StringComparison.OrdinalIgnoreCase));
            AppState.IsAlpha = isExperimental;
        }

        private void RegisterServices()
        {
            ServiceRegister.Register(typeof(IDialogService), DialogService.Instance);
        }

        private void InitializeLog()
        {
            var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            var logDir = Path.Combine(appDataPath, "DicomViewer", "Serilog");
            var logPath = Path.Combine(logDir, $"DVLog_{DateTime.Now.ToString("yymmdd_hhmmss")}.txt");

#if DEBUG
            var logInterval = TimeSpan.FromSeconds(10);
#else
            var logInterval = TimeSpan.FromMinutes(5);
#endif
            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Debug()
                .WriteTo.File(logPath, retainedFileCountLimit: 500, flushToDiskInterval: TimeSpan.FromMinutes(5))
                .CreateLogger();

            Log.Information($"Log initialized for DicomViewer {Assembly.GetEntryAssembly().ImageRuntimeVersion}");
        }

        internal static void StartMainWindow()
        {
            var mainViewModel = new MainViewModel();
            var mainWindow = new MainWindow { DataContext = mainViewModel };
            mainWindow.Show();
        }
    }
}
