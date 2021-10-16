using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.Threading;
using Microsoft.Win32;
using Serilog;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace DicomViewer.Lib.ViewModels
{
    public class DicomImageSurfaceViewModel : SurfaceViewModel
    {
        public DicomImageSurfaceViewModel(MainViewModel main) : base(main) { }

        private static string TestImagePath => Path.Combine(PathHelpers.GetAppDataOutputPath(), "test.dcm");

        public string Title
        {
            get { return title; }
            set
            {
                if (title == value) return;
                title = value;
                OnPropertyChanged();
            }
        }
        string title;

        public DicomImage DicomImage
        {
            get { return dicomImage; }
            set
            {
                if (dicomImage == value) return;
                dicomImage = value;
                OnPropertyChanged();
            }
        }
        DicomImage dicomImage;

        public void OpenDicom(string filePath = null)
        {
            if (string.IsNullOrEmpty(filePath))
            {
                var dialog = new OpenFileDialog();
                dialog.DefaultExt = ".dcm";
                dialog.Filter = "DICOM files (*.dcm)|*.dcm;";
                var result = dialog.ShowDialog();

                if (result.HasValue && !string.IsNullOrEmpty(dialog.FileName))
                {
                    filePath = dialog.FileName;
                }
                else return;

            }
            OpenDicomImagePriv(filePath);
        }

        void OpenDicomImagePriv(string filePath)
        {
            if (dicomImage == null)
                dicomImage = new DicomImage();
            else
                dicomImage.Cleanup();

            dicomImage.Open(filePath);
        }

        public void LoadTestDicomImage()
        {
            if (!File.Exists(TestImagePath))
            {
                var directory = Path.GetDirectoryName(TestImagePath);
                if (!Directory.Exists(directory))
                    Directory.CreateDirectory(directory);

                try
                {
                    var assembly = this.GetType().Assembly;
                    using (var stream = assembly.GetManifestResourceStream(assembly.GetName().Name + ".test.dcm"))
                    {
                        using (var fs = new FileStream(TestImagePath, FileMode.Create, FileAccess.ReadWrite, FileShare.ReadWrite))
                        {
                            stream.CopyTo(fs);
                        }
                    }
                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to load test dicom image.");
                }
            }

            OpenDicomImagePriv(TestImagePath);
        }

        public void SaveDicomImage()
        {
            if (DicomImage is null) return;
            var filePath = "savedImage.dcm";
            DicomImage.Save(filePath);
        }
    }
}
