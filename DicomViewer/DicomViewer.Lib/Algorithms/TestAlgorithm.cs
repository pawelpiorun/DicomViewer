using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Native.Dicom;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Algorithms
{
    public class TestAlgorithm : DicomAlgorithm, IBatchAlgorithm
    {
        public TestAlgorithm()
        {
            Title = "Test Algorithm";
            Description = "Testing algorithms. After running it, there should be something visible in Results panel.";
        }

        public override void Run(DicomSeries dicomSeries, bool dispose = true, Action onDone = null,
            Action<double> onProgress = null) => Run();

        void Run()
        {
            DateTime startTime = DateTime.Now;
            Success = true;
            Thread.Sleep(100);
            Result = "And we have a result!";
            Duration = (int)((DateTime.Now - startTime).TotalMilliseconds);
        }

        public string GetCsvFormattedResult()
        {
            return string.Format("{0};{1};{2};{3}\n",
                "TestAlggg",
                Success,
                TimeSpan.FromMilliseconds(Duration).Format(),
                Result);
        }

        public string GetCsvFormattedHeader()
        {
            return string.Format("{0};{1};{2};{3}\n",
                "Algorithm",
                "Success",
                "Duration",
                "Result");
        }

        public override void Dispose() => Dispose(true);

        protected override void Dispose(bool disposing)
        {

        }

        public void DumpIntermediates(string outputFolder)
        {
            // no op
        }
    }
}
