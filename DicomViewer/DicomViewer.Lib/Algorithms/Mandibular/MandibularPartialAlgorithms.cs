using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Algorithms
{
    public class MandibularGeneralThresholdAlgorithm : MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr MandibularGeneralThresholdAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularGeneralThresholdAlgorithm_Dispose(AlgorithmHandle instance);

        public MandibularGeneralThresholdAlgorithm()
            : base(MandibularGeneralThresholdAlgorithm_CreateInstance, MandibularGeneralThresholdAlgorithm_Dispose)
        {
            Title = "Mandibular general threshold";
            Description = "Segmentation of bones (HU > 450)";
        }

        protected override void PrepareResult() { }
    }

    public class MandibularTeethThresholdAlgorithm: MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr MandibularTeethThresholdAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularTeethThresholdAlgorithm_Dispose(AlgorithmHandle instance);

        public MandibularTeethThresholdAlgorithm()
            : base(MandibularTeethThresholdAlgorithm_CreateInstance, MandibularTeethThresholdAlgorithm_Dispose)
        {
            Title = "Mandibular teeth threshold";
            Description = "Segmentation of teeth (HU 2300 - 2600)";
        }

        protected override void PrepareResult() { }
    }

    public class MandibularTeethDilateAlgorithm : MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr MandibularTeethDilateAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularTeethDilateAlgorithm_Dispose(AlgorithmHandle instance);

        public MandibularTeethDilateAlgorithm()
            : base(MandibularTeethDilateAlgorithm_CreateInstance, MandibularTeethDilateAlgorithm_Dispose)
        {
            Title = "Mandibular teeth dilate / segmentation";
            Description = "Segmentation of teeth (dilate + keep max segment)";
        }

        protected override void PrepareResult() { }
    }

    public class MandibularSubtractAlgorithm : MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr MandibularSubtractAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularSubtractAlgorithm_Dispose(AlgorithmHandle instance);

        public MandibularSubtractAlgorithm()
            : base(MandibularSubtractAlgorithm_CreateInstance, MandibularSubtractAlgorithm_Dispose)
        {
            Title = "Mandibular subtraction";
            Description = "Result of (general threshold - teeth segmentation)";
        }

        protected override void PrepareResult() { }
    }

    public class MandibularExtractionAlgorithm : MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr MandibularExtractionAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularExtractionAlgorithm_Dispose(AlgorithmHandle instance);

        public MandibularExtractionAlgorithm()
            : base(MandibularExtractionAlgorithm_CreateInstance, MandibularExtractionAlgorithm_Dispose)
        {
            Title = "Mandibular extraction";
            Description = "Extracts mandibular bone";
        }

        protected override void PrepareResult() { }
    }

    public class MandibularUpperSkullAlgorithm : MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr UpperSkullExtractionAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void UpperSkullExtractionAlgorithm_Dispose(AlgorithmHandle instance);

        public MandibularUpperSkullAlgorithm()
            : base(UpperSkullExtractionAlgorithm_CreateInstance,
                  UpperSkullExtractionAlgorithm_Dispose)
        {
            Title = "Upper skull extraction";
            Description = "Subtracts mandibula from the whole skull";
        }

        protected override void PrepareResult() { }
    }

    public class ZygomaticArchsExtractionAlgorithm : MandibularBaseAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr ZygomaticArchsExtractionAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void ZygomaticArchsExtractionAlgorithm_Dispose(AlgorithmHandle instance);

        public ZygomaticArchsExtractionAlgorithm()
            : base(ZygomaticArchsExtractionAlgorithm_CreateInstance,
                  ZygomaticArchsExtractionAlgorithm_Dispose)
        {
            Title = "Zygomatic archs extraction";
            Description = "Extracts zygomatic archs";
        }

        protected override void PrepareResult() { }
    }
}
