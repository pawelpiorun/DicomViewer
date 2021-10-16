using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.ViewModels;
using Serilog;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace DicomViewer.Lib.Algorithms
{
    public class AirVolumeAlgorithm : DicomAlgorithm, IBatchAlgorithm, IChangesInput
    {
        #region Native

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr AirVolumeAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_Dispose(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_Run(AlgorithmHandle instance, IntPtr dicomSeries);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern long AirVolumeAlgorithm_GetVoxelCount(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetCalculatedVolume(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern float AirVolumeAlgorithm_GetProgress(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_DumpIntermediates(AlgorithmHandle instance, [MarshalAs(UnmanagedType.LPStr)] string folder);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetMinimalCrossSection(AlgorithmHandle instance);
      
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetMinimalCrossSection2(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetMinimalCrossSection3(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetMinimalCrossSection4(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_GetMinimalCrossSections2(AlgorithmHandle instance, out IntPtr ptr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_GetMinimalCrossSections(AlgorithmHandle instance, out IntPtr ptr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_GetMinimalCrossSections3(AlgorithmHandle instance, out IntPtr ptr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_GetMinimalCrossSections4(AlgorithmHandle instance, out IntPtr ptr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetCalculatedPharynx(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double AirVolumeAlgorithm_GetCalculatedNasalCavity(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_GetMinimalCrossSectionCoord(AlgorithmHandle instance, int i, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void AirVolumeAlgorithm_GetMinimalCrossSectionNormal(AlgorithmHandle instance, int i, out IntPtr arr, out int size);
        #endregion

        public AirVolumeAlgorithm()
        {
            SupportsProgress = true;
            Title = "Air volume";
            Description = "Calculate volume of air passages of part between nasal cavity and epiglottis";
        }
        AlgorithmHandle instance;

        public override void Run(DicomSeries dicomSeries, bool dispose = true, Action onDone = null, Action<double> onProgress = null)
        {
            bool isFinished = false;
            try
            {
                StartTime = DateTime.Now;
                instance = new AlgorithmHandle(AirVolumeAlgorithm_CreateInstance, AirVolumeAlgorithm_Dispose);
                isDisposed = false;

                Threading.Threads.Background(() =>
                {
                    while (!isFinished)
                    {
                        lock (locker)
                        {
                            var progress = (double)AirVolumeAlgorithm_GetProgress(instance);
                            onProgress?.Invoke(progress);
                        }
                        Thread.Sleep(100);
                    }
                    onProgress?.Invoke(1.0d);
                });

                AirVolumeAlgorithm_Run(instance, dicomSeries.GetPointer());

                PrepareResult(dicomSeries);
               
                Threading.Threads.UI(() => dicomSeries.RequestRender());
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to run AirVolumeAlgorithm");
                Success = false;
            }
            finally
            {
                isFinished = true;
                onDone?.Invoke();
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

        private void PrepareResult(DicomSeries dicomSeries)
        {
            var calculatedVolume = AirVolumeAlgorithm_GetCalculatedVolume(instance);
            var minimalSection = AirVolumeAlgorithm_GetMinimalCrossSection(instance);
            var minimalSection2 = AirVolumeAlgorithm_GetMinimalCrossSection2(instance);
            var minimalSection3 = AirVolumeAlgorithm_GetMinimalCrossSection3(instance);
            var minimalSection4 = AirVolumeAlgorithm_GetMinimalCrossSection4(instance);
            var pharynxVolume = AirVolumeAlgorithm_GetCalculatedPharynx(instance);
            var nasalCavity = AirVolumeAlgorithm_GetCalculatedNasalCavity(instance);

            Result = string.Format(
                "Calculated volume: {0:0.000} cm3 \n" +
                "Where {1:0.000} cm3 is pharynx \n" +
                "and {2:0.000} cm3 is nasal cavity \n" +
                //"Minimal cross-section 1: {1:0.00} mm2 \n" +
                //"Minimal cross-section 2: {2:0.00} mm2 \n" +
                "Minimal cross-section: {3:0.00} mm2 ",
                calculatedVolume,
                pharynxVolume,
                nasalCavity,
                //minimalSection,
                //minimalSection2,
                minimalSection4);
            Duration = (int)((DateTime.Now - StartTime).TotalMilliseconds);
            Success = true;
            PatientName = dicomSeries.PatientName;
            PatientId = dicomSeries.PatientId;
            AcquisitionDate = dicomSeries.AcquisitionDate;
            Path = dicomSeries.FullPath;
            Volume = calculatedVolume;
            PharynxVolume = pharynxVolume;
            NasalCavityVolume = nasalCavity;
            MinimalSection = minimalSection;
            MinimalSection2 = minimalSection2;
            MinimalSection3 = minimalSection3;
            MinimalSection4 = minimalSection4;
            ImportSectionsProfiles();

            // insert sections profile as a tag in dicom series
            dicomSeries.Tags[DicomSeries.MinimalCrossSection] = string.Format("{0:0.00}", MinimalSection4);
            dicomSeries.Tags[DicomSeries.MinimalCrossSectionsTag] = FormatDoubleArray(MinimalSections4);

            MinimalSectionsCoords = new List<int[]>();
            for (int i = 0; i < MinimalSections4.Length; i++)
                MinimalSectionsCoords.Add(GetMinimalSectionsCoord(i));

            dicomSeries.Tags[DicomSeries.MinimalCrossSectionsCoordsTag] = FormatCrossSectionsCoords(MinimalSectionsCoords);

            MinimalSectionsNormals = new List<double[]>();
            for (int i = 0; i < MinimalSections4.Length; i++)
                MinimalSectionsNormals.Add(GetMinimalSectionsNormal(i));

            dicomSeries.Tags[DicomSeries.MinimalCrossSectionsNormalsTag] = FormatCrossSectionsNormals(MinimalSectionsNormals);
        }

        protected DateTime StartTime { get; private set; }
        protected double Volume { get; set; }
        protected double PharynxVolume { get; set; }
        protected double NasalCavityVolume { get; set; }
        protected double MinimalSection { get; set; }
        protected string PatientName { get; set; }
        protected string PatientId { get; set; }
        protected string AcquisitionDate { get; set; }
        protected string Path { get; set; }
        protected double MinimalSection2 { get; set; }
        protected double MinimalSection3 { get; set; }
        protected double MinimalSection4 { get; set; }

        protected double[] MinimalSections { get; set; }
        protected double[] MinimalSections2 { get; set; }
        protected double[] MinimalSections3 { get; set; }
        protected double[] MinimalSections4 { get; set; }

        protected List<int[]> MinimalSectionsCoords { get; set; }
        protected List<double[]> MinimalSectionsNormals { get; set; }

        void ImportSectionsProfiles()
        {
            try
            {
                AirVolumeAlgorithm_GetMinimalCrossSections(instance, out IntPtr arr, out int size1);
                AirVolumeAlgorithm_GetMinimalCrossSections2(instance, out IntPtr arr2, out int size2);
                AirVolumeAlgorithm_GetMinimalCrossSections3(instance, out IntPtr arr3, out int size3);
                AirVolumeAlgorithm_GetMinimalCrossSections4(instance, out IntPtr arr4, out int size4);

                MinimalSections = new double[size1];
                MinimalSections2 = new double[size2];
                MinimalSections3 = new double[size3];
                MinimalSections4 = new double[size4];
                Marshal.Copy(arr, MinimalSections, 0, size1);
                Marshal.Copy(arr2, MinimalSections2, 0, size2);
                Marshal.Copy(arr3, MinimalSections3, 0, size3);
                Marshal.Copy(arr4, MinimalSections4, 0, size4);
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to import sections profiles");
            }
        }

        private int[] GetMinimalSectionsCoord(int i)
        {
            try
            {
                AirVolumeAlgorithm_GetMinimalCrossSectionCoord(instance, i, out IntPtr arr, out int size);
                var coord = new int[size];
                Marshal.Copy(arr, coord, 0, size);
                return coord;
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to get minimal sections coords.");
            }

            return null;
        }

        private double[] GetMinimalSectionsNormal(int i)
        {
            try
            {
                AirVolumeAlgorithm_GetMinimalCrossSectionNormal(instance, i, out IntPtr arr, out int size);
                var normal = new double[size];
                Marshal.Copy(arr, normal, 0, size);
                return normal;
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to get minimal sections normals.");
            }

            return null;
        }

        public string GetCsvFormattedHeader()
        {
            return string.Format("{0};{1};{2};{3};{4};{5};{6};{7};{8};{9};{10};{11}\n",
                "PatientName",
                "PatientId",
                "Date",
                "Success",
                "Duration",
                "Volume",
                "Pharynx",
                "Nasal cavity",
                "Cross-section",
                //"Cross-section 2",
                //"Cross-section 3",
                "Result",
                "Path",
                "All cross-sections"
                //"All cross-sections 2",
                //"All cross-sections 3"
                );
        }

        public string GetCsvFormattedResult()
        {
            return string.Format("{0};{1};{2};{3};{4};{5};{6};{7};{8};{9};{10};{11}\n",
                  PatientName,
                  PatientId,
                  AcquisitionDate,
                  Success,
                  TimeSpan.FromMilliseconds(Duration).Format(),
                  string.Format("{0:0.000}", Volume),
                  string.Format("{0:0.000}", PharynxVolume),
                  string.Format("{0:0.000}", NasalCavityVolume),
                  //string.Format("{0:0.00}", MinimalSection),
                  //string.Format("{0:0.00}", MinimalSection2),
                  string.Format("{0:0.00}", MinimalSection4),
                  Result.Replace('\n', ' '),
                  Path,
                  //FormatCrossSections(MinimalSections),
                  //FormatCrossSections(MinimalSections2),
                  FormatDoubleArray(MinimalSections4)
                  );
        }

        string FormatDoubleArray(double [] arr, int precision = 2)
        {
            if (arr is null || arr.Length == 0) return null;
            var sb = new StringBuilder();
            sb.Append("{");
            for (int i = 0; i < arr.Length; i++)
            {
                sb.Append(string.Format("{0}", Math.Round(arr[i], precision, MidpointRounding.AwayFromZero)));
                if (i != arr.Length - 1)
                    sb.Append(",");
            }
            sb.Append("}");
            return sb.ToString();
        }

        private string FormatCrossSectionsCoords(List<int[]> minimalSectionsCoords)
        {
            if (minimalSectionsCoords is null || minimalSectionsCoords.Count == 0) return null;
            var sb = new StringBuilder();
            sb.Append("{");
            for (int i = 0; i < minimalSectionsCoords.Count; i++)
            {
                sb.Append(FormatCoord(minimalSectionsCoords[i]));
                if (i != minimalSectionsCoords.Count - 1)
                    sb.Append(",");
            }
            sb.Append("}");
            return sb.ToString();
        }

        private string FormatCoord(int[] arr)
        {
            if (arr is null || arr.Length == 0) return null;
            var sb = new StringBuilder();
            sb.Append("{");
            for (int i = 0; i < arr.Length; i++)
            {
                sb.Append(arr[i].ToString());
                if (i != arr.Length - 1)
                    sb.Append(",");
            }
            sb.Append("}");
            return sb.ToString();
        }

        private string FormatCrossSectionsNormals(List<double[]> minimalSectionsNormals)
        {
            if (minimalSectionsNormals is null || minimalSectionsNormals.Count == 0) return null;
            var sb = new StringBuilder();
            sb.Append("{");
            for (int i = 0; i < minimalSectionsNormals.Count; i++)
            {
                sb.Append(FormatDoubleArray(minimalSectionsNormals[i], 3));
                if (i != minimalSectionsNormals.Count - 1)
                    sb.Append(",");
            }
            sb.Append("}");
            return sb.ToString();
        }


        public void DumpIntermediates(string outputFolder)
        {
            AirVolumeAlgorithm_DumpIntermediates(instance, outputFolder);
        }

        #region IDisposable

        ~AirVolumeAlgorithm() => Dispose(false);

        bool isDisposed;
        public override void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        // if disposing, it's from consumer, otherwise from finalizer
        protected override void Dispose(bool disposing)
        {
            if (isDisposed) return;
            if (disposing)
            {
                // free managed resources
                lock (locker)
                {
                    instance?.Dispose();
                }
            }

            // free unmanaged resources

            isDisposed = true;
        }

        #endregion

    }
}
