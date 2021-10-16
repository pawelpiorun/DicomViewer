using System;
using System.IO;

namespace DicomViewer.Lib.Helpers
{
    public static class PathHelpers
    {
        public static string GetDocumentsOutputPath()
        {
            return Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
                "DicomViewer");
        }

        public static string GetAppDataOutputPath()
        {
            return Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "DicomViewer");
        }
    }
}
