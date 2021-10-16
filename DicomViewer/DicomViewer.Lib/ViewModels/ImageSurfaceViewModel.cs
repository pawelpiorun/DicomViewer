using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Native.Images;
using Microsoft.Win32;
using Serilog;
using System;
using System.IO;
using System.Linq;

namespace DicomViewer.Lib.ViewModels
{
    public class ImageSurfaceViewModel : SurfaceViewModel
    {
        public ImageSurfaceViewModel(MainViewModel main) : base(main) { }
        public Image Image
        {
            get => image;
            set
            {
                if (image == value) return;
                image = value;
                OnPropertyChanged();
            }
        }
        Image image;

        private static string TestImagePath => Path.Combine(PathHelpers.GetAppDataOutputPath(), "test.jpg");

        public void LoadTestImage()
        {
            if (!File.Exists(TestImagePath))
            {
                var directory = Path.GetDirectoryName(TestImagePath);
                if (!Directory.Exists(directory))
                    Directory.CreateDirectory(directory);

                try
                {
                    var assembly = this.GetType().Assembly;
                    using (var stream = assembly.GetManifestResourceStream(assembly.GetName().Name + ".test.jpg"))
                    {
                        using (var fs = new FileStream(TestImagePath, FileMode.Create, FileAccess.ReadWrite, FileShare.ReadWrite))
                        {
                            stream.CopyTo(fs);
                        }
                    }
                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to load test image.");
                }
            }

            OpenImagePriv(TestImagePath);
        }

        public void OpenImage(string filePath = null)
        {
            if (string.IsNullOrEmpty(filePath))
            {
                var dialog = new OpenFileDialog
                {
                    DefaultExt = ".jpg",
                    Filter = "Image files (*.jpg, *.jpeg, *.png, *.bmp)|*.jpg;*.jpeg;*.png;*.bmp;"
                };
                var result = dialog.ShowDialog();

                if (result.HasValue && !string.IsNullOrEmpty(dialog.FileName))
                {
                    filePath = dialog.FileName;
                }
                else return;
            }

            OpenImagePriv(filePath);
        }

        private void OpenImagePriv(string filePath)
        {
            if (string.IsNullOrEmpty(filePath)) return;

            if (image == null)
                image = new Image();
            else
                image.Cleanup();

            Image.Open(filePath);
        }

        public void LoadPreviousImage()
        {
            if (Image is null) return;
            
            var path = Image.FilePath.ToLower();
            var folder = Path.GetDirectoryName(path);
            if (!Directory.Exists(folder)) return;

            var files = Directory.EnumerateFiles(folder).Where(x => Path.GetExtension(x).ToLower().Contains("jpg"))
                .Select(x => x.ToLower()).ToList();
            var index = files.IndexOf(path);
            if (index > 0)
            {
                var previous = files[index - 1];
                OpenImagePriv(previous);
                main.UpdateSubtitle(previous);
            }
        }

        public void LoadNextImage()
        {
            if (Image is null) return;

            var path = Image.FilePath.ToLower();
            var folder = Path.GetDirectoryName(path);
            if (!Directory.Exists(folder)) return;

            var files = Directory.EnumerateFiles(folder).Where(x => Path.GetExtension(x).ToLower().Contains("jpg"))
                .Select(x => x.ToLower()).ToList();
            var index = files.IndexOf(path);
            if (index < files.Count - 1)
            {
                var next = files[index + 1];
                OpenImagePriv(next);
                main.UpdateSubtitle(next);
            }
        }
    }
}
