using DicomViewer.Lib.Common;
using DicomViewer.Lib.Threading;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace DicomViewer.Lib.Native.Dicom
{
    public class DicomImage : NotifyPropertyChanged
    {
        #region Extern
        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DicomImage_CreateInstance();

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomImage_Open(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string imagePath);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomImage_Save(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string imagePath);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DicomImage_GetData(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DicomImage_GetImageBytes(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int DicomImage_GetWidth(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int DicomImage_GetHeight(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomImage_Dispose(IntPtr instance);

        #endregion

        public DicomImage()
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

        public int Depth => 1;

        public void Open(string filePath)
        {
            Threads.Background(() =>
            {
                try
                {
                    if (instance == IntPtr.Zero)
                        instance = DicomImage_CreateInstance();

                    DicomImage_Open(instance, filePath);

                    IntPtr res = DicomImage_GetImageBytes(instance);
                    Width = DicomImage_GetWidth(instance);
                    Height = DicomImage_GetHeight(instance);
                    var imageSize = width * height;

                    Width = width;
                    Height = height;

                    imageBytes = new byte[imageSize];
                    Marshal.Copy(res, imageBytes, 0, imageSize);

                    Load(imageBytes);

                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to open dicom image.");
                }
            });
        }

        private void Load(byte[] imageBytes)
        {
            Threads.UI(() =>
            {
                try
                {
                    ImageSource = BitmapSource.Create(Width, Height, 96, 96, PixelFormats.Gray8, null, imageBytes, Depth * Width);
                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to load dicom image bytes.");
                }
            });
        }

        public void Save(string filePath)
        {
            //Run.Background(() => DicomImage_Save(instance, filePath));
            DicomImage_Save(instance, filePath);
        }

        public void Cleanup()
        {
            ImageBytes = null;
            ImageSource = null;
            try
            {
                DicomImage_Dispose(instance);
                instance = IntPtr.Zero;
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to cleanup dicom image.");
            }
        }
    }
}
