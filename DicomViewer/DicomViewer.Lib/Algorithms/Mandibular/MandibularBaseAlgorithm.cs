using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Native.Dicom;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Algorithms
{
    public abstract class MandibularBaseAlgorithm : DicomAlgorithm, IChangesInput
    {
        #region Native

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern int MandibularBaseAlgorithm_Run(AlgorithmHandle instance, IntPtr dicomSeries);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern float MandibularBaseAlgorithm_GetProgress(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void MandibularBaseAlgorithm_GetLeftCondylePosition(AlgorithmHandle instance, double[] buff);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void MandibularBaseAlgorithm_GetLeftCondyleRotation(AlgorithmHandle instance, double[] buff);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void MandibularBaseAlgorithm_GetRightCondylePosition(AlgorithmHandle instance, double[] buff);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void MandibularBaseAlgorithm_GetRightCondyleRotation(AlgorithmHandle instance, double[] buff);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern bool MandibularBaseAlgorithm_GetSuccess(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        protected static extern string MandibularBaseAlgorithm_Run(AlgorithmHandle instance);
        #endregion

        MandibularBaseAlgorithm()
        {
            SupportsProgress = true;
        }
        protected MandibularBaseAlgorithm(Func<IntPtr> nativeConstructor, Action<AlgorithmHandle> nativeDestructor)
        {
            this.nativeConstructor = nativeConstructor;
            this.nativeDestructor = nativeDestructor;
            SupportsProgress = true;
        }
        readonly Func<IntPtr> nativeConstructor;
        readonly Action<AlgorithmHandle> nativeDestructor;
        protected AlgorithmHandle instance;

        protected abstract void PrepareResult();

        public override void Run(DicomSeries dicomSeries, bool dispose = true, Action onDone = null, Action<double> onProgress = null)
        {
            bool isFinished = false;
            try
            {
                StartTime = DateTime.Now;
                instance = new AlgorithmHandle(nativeConstructor, nativeDestructor);
                Threading.Threads.Background(() =>
                {
                    while (!isFinished)
                    {
                        lock (locker)
                        {
                            var progress = (double)MandibularBaseAlgorithm_GetProgress(instance);
                            onProgress?.Invoke(progress);
                        }
                        Thread.Sleep(100);
                    }
                    onProgress?.Invoke(1.0d);
                });

                var err = MandibularBaseAlgorithm_Run(instance, dicomSeries.GetPointer());

                PrepareResult();

                Duration = (int)((DateTime.Now - StartTime).TotalMilliseconds);
                Success = MandibularBaseAlgorithm_GetSuccess(instance);
                PatientName = dicomSeries.PatientName;
                PatientId = dicomSeries.PatientId;
                AcquisitionDate = dicomSeries.AcquisitionDate;
                Path = dicomSeries.FullPath;

                if (err == 0)
                    Threading.Threads.UI(() => dicomSeries.RequestRender());
                onDone?.Invoke();
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to run ManidublarBaseAlgorithm");
                Success = false;
            }
            finally
            {
                isFinished = true;
                if (dispose)
                {
                    lock (locker)
                    {
                        instance?.Dispose();
                    }
                }
            }
        }
        readonly object locker = new object();

        protected DateTime StartTime { get; private set; }
        protected string PatientName { get; private set; }
        protected string PatientId { get; private set; }
        protected string AcquisitionDate { get; private set; }
        protected string Path { get; private set; }

        #region IDisposable

        bool isDisposed;
        public override void Dispose()
        {
            Dispose(true);
        }

        protected override void Dispose(bool disposing)
        {
            if (isDisposed) return;

            if (disposing)
            {
                lock (locker)
                {
                    instance?.Dispose();
                }
            }

            isDisposed = true;
        }

        #endregion
    }

}
