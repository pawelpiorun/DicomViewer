using DicomViewer.Lib.Native.Dicom;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Algorithms
{
    public class ThresholdBetweenAlgorithm : DicomAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void DicomProcessing_ThresholdBetween(IntPtr dicomSeries, short lower, short upper);

        public ThresholdBetweenAlgorithm()
        {
            Title = "Threshold between";
            Description = "Threshold above lower and below upper value";
        }

        public override void Run(DicomSeries dicomSeries, bool dispose = true, Action onDone = null, Action<double> onProgress = null) => Run(dicomSeries);

        void Run(DicomSeries dicomSeries)
        {
            try
            {
                if (dicomSeries != null && Lower.HasValue && Upper.HasValue)
                {
                    ThresholdBetween(dicomSeries, Lower.Value, Upper.Value);
                    dicomSeries.RequestRender();
                    Success = true;
                }
            }
            catch (Exception x)
            {
                Success = false;
                Log.Fatal(x, "Failed to run ThresholdBetweenAlgorithm");
            }
        }

        void ThresholdBetween(DicomSeries dicomSeries, short lower, short upper)
        {
            var timeStamp = DateTime.Now;
            DicomProcessing_ThresholdBetween(dicomSeries.GetPointer(), lower, upper);
            Duration = (int)((DateTime.Now - timeStamp).TotalMilliseconds);
        }

        public short? Lower { get; set; }
        public short? Upper { get; set; }

        public override void Dispose() => Dispose(true);

        protected override void Dispose(bool disposing)
        {
            // TODO
        }
    }
}
