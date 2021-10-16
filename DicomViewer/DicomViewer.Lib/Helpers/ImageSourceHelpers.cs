using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace DicomViewer.Lib.Helpers
{
    public static class ImageSourceHelpers
    {
        public static ImageSource ImageSourceFromBytes(byte[] bytes)
        {
            using (var ms = new MemoryStream(bytes))
            {
                return ImageSourceFromStream(ms);
            }
        }

        public static ImageSource ImageSourceFromStream(MemoryStream stream)
        {
            var src = new BitmapImage();
            src.BeginInit();
            src.CacheOption = BitmapCacheOption.OnLoad;
            src.CreateOptions = BitmapCreateOptions.IgnoreColorProfile;
            src.StreamSource = stream;
            src.EndInit();

            src.Freeze();
            return src;
        }
    }
}
