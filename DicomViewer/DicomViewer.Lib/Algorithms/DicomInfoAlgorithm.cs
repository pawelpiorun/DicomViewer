using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DicomViewer.Lib.Native.Dicom;

namespace DicomViewer.Lib.Algorithms
{
    public class DicomInfoAlgorithm : DicomAlgorithm, IBatchAlgorithm
    {
        public DicomInfoAlgorithm()
        {
            SupportsProgress = false;
            Title = "Dicom tags info";
            Description = "Dumps information about a DICOM format image";
        }

        public override void Run(DicomSeries dicomSeries, bool dispose = true, Action onDone = null, Action<double> onProgress = null)
        {
            Path = dicomSeries.FullPath;
            PatientName = dicomSeries.PatientName;
            PatientId = dicomSeries.PatientId;
            Date = dicomSeries.StudyDate;
            ImageId = dicomSeries.ImageId;
            onDone?.Invoke();

        }
        protected override void Dispose(bool disposing) { }
        public override void Dispose() { }

        public string Path { get; private set; }
        public string PatientName { get; private set; }
        public string PatientId { get; private set; }
        public string ImageId { get; private set; }
        public string Date { get; private set; }

        public string GetCsvFormattedHeader()
        {
            return string.Format("{0};{1};{2};{3};{4}\n",
                   "Path",
                   "PatientName",
                   "PatientId",
                   "ImageId",
                   "Date");
        }

        public string GetCsvFormattedResult()
        {
            return string.Format("{0};{1};{2};{3};{4}\n",
                  Path,
                  PatientName,
                  PatientId,
                  ImageId,
                  Date);
        }

        public void DumpIntermediates(string outputFolder)
        {
            // no op
        }
    }
}
