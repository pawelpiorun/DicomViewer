using DicomViewer.Lib.Common;
using DicomViewer.Lib.Threading;
using Serilog;
using System;
using System.Runtime.InteropServices;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace DicomViewer.Lib.Native.Images
{
    public class Image : NotifyPropertyChanged
    {
        #region Extern

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Image_CreateInstance();

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Image_Open(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string imagePath);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Image_GetData(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Image_GetWidth(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Image_GetHeight(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Image_Dispose(IntPtr instance);

        #endregion

        public Image()
        {

        }
        IntPtr instance;

        public byte[] ImageBytes
        {
            get { return imageBytes; }
            set
            {
                if (imageBytes == value) return;
                imageBytes = value;
                OnPropertyChanged();
            }
        }
        byte[] imageBytes;

        public ImageSource ImageSource
        {
            get { return imageSource; }
            set
            {
                if (imageSource == value) return;
                imageSource = value;
                OnPropertyChanged();
            }
        }
        private ImageSource imageSource;


        public int Width
        {
            get { return width; }
            set
            {
                if (width == value) return;
                width = value;
                OnPropertyChanged();
            }
        }
        int width;

        public int Height
        {
            get { return height; }
            set
            {
                if (height == value) return;
                height = value;
                OnPropertyChanged();
            }
        }
        int height;

        public int Depth => 3;

        public ImageType ImageType => ImageType.Rgb;


        public string FilePath
        {
            get { return filePath; }
            set
            {
                if (filePath == value) return;
                filePath = value;
                OnPropertyChanged();
            }
        }
        string filePath;


        public void Open(string filePath)
        {
            Threads.Background(() =>
            {
                try
                {
                    if (instance == IntPtr.Zero)
                        instance = Image_CreateInstance();
                    Image_Open(instance, filePath);
                    IntPtr res = Image_GetData(instance);
                    Width = Image_GetWidth(instance);
                    Height = Image_GetHeight(instance);
                    var imageSize = Width * Height * Depth;

                    imageBytes = new byte[imageSize];
                    Marshal.Copy(res, imageBytes, 0, imageSize);
                    Load(imageBytes);
                    this.FilePath = filePath;
                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to open image.");
                }
            });
        }

        private void Load(byte[] imageBytes)
        {
            Threads.UI(() =>
            {
                ImageSource = BitmapSource.Create(Width, Height, 96, 96, PixelFormats.Bgr24, null, imageBytes, Depth * Width);
            });
        }

        public void Cleanup()
        {
            ImageBytes = null;
            ImageSource = null;
            try
            {
                Image_Dispose(instance);
                instance = IntPtr.Zero;
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to cleanup image.");
            }
        }
    }

    public enum ImageType
    {
        Rgb,
        GrayScale
    }
}
