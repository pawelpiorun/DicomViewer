using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.Native.Dicom;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Algorithms
{
    public interface IAlgorithm
    {
        string Title { get; set; }
        string Description { get; set; }
        bool Success { get; }
        string Result { get; }

        /// <summary>
        /// Algorithm duration in milliseconds.
        /// </summary>
        int Duration { get; }
    }

    public abstract class AlgorithmBase : IAlgorithm, IDisposable
    {
        public string Title { get; set; }
        public string Description { get; set; }
        public bool Success { get; protected set; }
        public int Duration { get; protected set; }
        public string Result { get; protected set; }
        public bool SupportsProgress { get; protected set; }
        public abstract void Dispose();
        protected abstract void Dispose(bool disposing);
    }

    public abstract class DicomAlgorithm : AlgorithmBase
    {
        public abstract void Run(DicomSeries dicomSeries, bool dispose = true, Action onDone = null, Action<double> onProgress = null);
    }

    public static class Algorithms
    {
        public static List<IAlgorithm> GetDicomAlgorithms()
        {
            var list = new List<IAlgorithm>()
            {
                new AirVolumeAlgorithm(),
                new MandibularCondylesAlgorithm()
            };

            if (AppState.IsAlpha)
            {
                list.Insert(0, new TestAlgorithm());
                list.Insert(2, new DicomInfoAlgorithm());
                list.AddRange(new IAlgorithm[]
                {
                    new MandibularGeneralThresholdAlgorithm(),
                    new MandibularTeethThresholdAlgorithm(),
                    new MandibularTeethDilateAlgorithm(),
                    new MandibularSubtractAlgorithm(),
                    new MandibularExtractionAlgorithm(),
                    new MandibularUpperSkullAlgorithm(),
                    new ZygomaticArchsExtractionAlgorithm()
                });
            }

            return list;
        }

        public static List<IAlgorithm> GetDicomAlgorithmsWithOutput()
        {
            return GetDicomAlgorithms().Where(x => x is IChangesInput).ToList();
        }
    }

    public sealed class AlgorithmHandle : System.Runtime.InteropServices.SafeHandle
    {
        public AlgorithmHandle(Func<IntPtr> ctor, Action<AlgorithmHandle> dctor)
            : base(ctor(), true)
        {
            this.dctor = dctor;
        }
        Action<AlgorithmHandle> dctor;

        public override bool IsInvalid => handle == IntPtr.Zero;

        protected override bool ReleaseHandle()
        {
            return true;
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                try
                {
                    dctor?.Invoke(this);
                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to dispose algorithm " + this.GetType().ToString());
                }
            }
            base.Dispose(disposing);
        }
    }
}