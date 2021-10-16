using DicomViewer.Lib.Common;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.Threading;
using Serilog;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace DicomViewer.Lib.Native.Dicom
{
    public class DicomSeries : NotifyPropertyChanged, IRenderContent
    {
        #region Extern

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DicomSeries_CreateInstance();

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomSeries_InitializeReader(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DicomSeries_Open(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string folderPath);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomSeries_Save(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string folderPath);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DicomSeries_GetDataBuffer(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DicomSeries_GetImagesBuffer(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int DicomSeries_GetWidth(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int DicomSeries_GetHeight(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int DicomSeries_GetLayers(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern double DicomSeries_GetProgress(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string DicomSeries_GetPatientName(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string DicomSeries_GetPatientId(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string DicomSeries_GetImageId(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string DicomSeries_GetStudyDate(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string DicomSeries_GetAcquisitionDate(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomSeries_DisposeReader(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DicomSeries_Dispose(IntPtr instance);

        #endregion

        public DicomSeries()
        {
            instance = DicomSeries_CreateInstance();
        }
        readonly IntPtr instance;

        public string PatientName { get; private set; }
        public string PatientId { get; private set; }
        public string ImageId { get; private set; }
        public string StudyDate { get; private set; }
        public string AcquisitionDate { get; private set; }
        public string FullPath { get; private set; }
        public bool IsLoaded { get; private set; }

        public Dictionary<string, string> Tags { get; private set; } = new Dictionary<string, string>();

        public void Open(string folderPath, bool async = false, Action<double> onProgress = null, Action onDone = null)
        {
            void a()
            {
                bool isFinished = false;
                try
                {
                    DicomSeries_InitializeReader(instance);

                    Threads.Background(() =>
                    {
                        while (!isFinished)
                        {
                            var progress = DicomSeries_GetProgress(instance);
                            Thread.Sleep(100);
                            onProgress?.Invoke(progress);
                        }
                        onDone?.Invoke();
                        Thread.Sleep(100);
                        Threads.UI(() => ContentReady?.Invoke(this, EventArgs.Empty));
                    });

                    var isFine = DicomSeries_Open(instance, folderPath);
                    IsLoaded = isFine;
                    if (isFine)
                    {
                        void setResult()
                        {
                            FullPath = folderPath;
                            PatientName = DicomSeries_GetPatientName(instance);
                            PatientId = DicomSeries_GetPatientId(instance);
                            StudyDate = DicomSeries_GetStudyDate(instance);
                            AcquisitionDate = DicomSeries_GetAcquisitionDate(instance);
                            ImageId = DicomSeries_GetImageId(instance);
                        }

                        if (async)
                            Threads.UI(setResult);
                        else
                            setResult();
                    }
                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to open dicom series.");
                }
                finally
                {
                    isFinished = true;
                }
            }

            if (async)
                Threads.Background(a);
            else
                a();
        }

        public event EventHandler ContentReady;

        public void RequestRender()
        {
            RenderRequested?.Invoke(this, EventArgs.Empty);
        }
        public event EventHandler RenderRequested;

        public void Save(string folderPath)
        {
            //Run.Background(() => DicomImage_Save(instance, filePath));
            DicomSeries_Save(instance, folderPath);
        }

        public IntPtr GetPointer()
        {
            return instance;
        }

        public void Cleanup()
        {
            CleaningUp?.Invoke(this, EventArgs.Empty);
            try
            {
                DicomSeries_DisposeReader(instance);
                DicomSeries_Dispose(instance);
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to cleanup dicom series.");
            }
        }

        public event EventHandler CleaningUp;

        public static readonly string MinimalCrossSectionsTag = "MinimalCrossSections";
        public static readonly string MinimalCrossSectionsCoordsTag = "MinimalCrossSectionsCoords";
        public static readonly string MinimalCrossSection = "MinimalCrossSection";
        public static readonly string MinimalCrossSectionsNormalsTag = "MinimalCrossSectionsNormals";
    }
}
