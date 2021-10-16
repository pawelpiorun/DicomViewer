using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Algorithms
{
    public sealed class MandibularCondylesAlgorithm : MandibularBaseAlgorithm, IBatchAlgorithm
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr MandibularCondylesAlgorithm_CreateInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_Dispose(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_GetLeftCondylePosition(AlgorithmHandle instance, out IntPtr arr, out int size1);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_GetRightCondylePosition(AlgorithmHandle instance, out IntPtr arr2, out int size2);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_GetCondylesPositionDifference(AlgorithmHandle instance, out IntPtr arr3, out int size3);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double MandibularCondylesAlgorithm_GetCondylesDistance(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double MandibularCondylesAlgorithm_GetCondylesDistancePx(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_GetNasionPoint(AlgorithmHandle instance, out IntPtr arr, out int size3);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_GetLeft2NasionDifference(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void MandibularCondylesAlgorithm_GetRight2NasionDifference(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double MandibularCondylesAlgorithm_GetLeft2NasionDistancePx(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double MandibularCondylesAlgorithm_GetLeft2NasionDistance(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double MandibularCondylesAlgorithm_GetRight2NasionDistancePx(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double MandibularCondylesAlgorithm_GetRight2NasionDistance(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_DumpIntermediates(AlgorithmHandle instance, [MarshalAs(UnmanagedType.LPStr)] string folder);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetLeftZygomaticPoint(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetRightZygomaticPoint(AlgorithmHandle instance, out IntPtr arr, out int size);


        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetCenter(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetLeftToCenterDifferenceEx(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetRightToCenterDifferenceEx(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double MandibularCondylesAlgorithm_GetLeftToCenterDistance(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double MandibularCondylesAlgorithm_GetRightToCenterDistance(AlgorithmHandle instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetImageSpacing(AlgorithmHandle instance, out IntPtr arr, out int size);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetImageOrigin(AlgorithmHandle instance, out IntPtr arr, out int size);
        
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetLeftCondyleReal(AlgorithmHandle instance, out IntPtr arr, out int size);
        
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void MandibularCondylesAlgorithm_GetRightCondyleReal(AlgorithmHandle instance, out IntPtr arr, out int size);


        public MandibularCondylesAlgorithm()
            : base(MandibularCondylesAlgorithm_CreateInstance, MandibularCondylesAlgorithm_Dispose)
        {
            Title = "Mandibular condyles";
            Description = "Calculate position and distance of mandibular condyles";
        }

        private delegate void NativeGetter(AlgorithmHandle instance, out IntPtr arr, out int size);

        protected override void PrepareResult()
        {
            LeftCondylePosition = GetPoint(MandibularCondylesAlgorithm_GetLeftCondylePosition);
            RightCondylePosition = GetPoint(MandibularCondylesAlgorithm_GetRightCondylePosition);
            CondylesPositionDifference = GetPoint(MandibularCondylesAlgorithm_GetCondylesPositionDifference);
            NasionPosition = GetPoint(MandibularCondylesAlgorithm_GetNasionPoint);
            Left2NasionDifference = GetPoint(MandibularCondylesAlgorithm_GetLeft2NasionDifference);
            Right2NasionDifference = GetPoint(MandibularCondylesAlgorithm_GetRight2NasionDifference);
            LeftZygomaticPoint = GetPoint(MandibularCondylesAlgorithm_GetLeftZygomaticPoint);
            RightZygomaticPoint = GetPoint(MandibularCondylesAlgorithm_GetRightZygomaticPoint);
            CenterPosition = GetPoint(MandibularCondylesAlgorithm_GetCenter);
            Left2CenterDifference = GetPoint(MandibularCondylesAlgorithm_GetLeftToCenterDifferenceEx);
            Right2CenterDifference = GetPoint(MandibularCondylesAlgorithm_GetRightToCenterDifferenceEx);
            Spacing = GetPoint(MandibularCondylesAlgorithm_GetImageSpacing);
            Origin = GetPoint(MandibularCondylesAlgorithm_GetImageOrigin);
            LeftCondylePositionReal = GetPoint(MandibularCondylesAlgorithm_GetLeftCondyleReal);
            RightCondylePositionReal = GetPoint(MandibularCondylesAlgorithm_GetRightCondyleReal);

            CondylesDistance = MandibularCondylesAlgorithm_GetCondylesDistance(instance);
            CondylesDistancePx = MandibularCondylesAlgorithm_GetCondylesDistancePx(instance);
            Left2NasionDistance = MandibularCondylesAlgorithm_GetLeft2NasionDistance(instance);
            Right2NasionDistance = MandibularCondylesAlgorithm_GetRight2NasionDistance(instance);
            Left2NasionDistancePx = MandibularCondylesAlgorithm_GetLeft2NasionDistancePx(instance);
            Right2NasionDistancePx = MandibularCondylesAlgorithm_GetRight2NasionDistancePx(instance);
            Left2CenterDistance = MandibularCondylesAlgorithm_GetLeftToCenterDistance(instance);
            Right2CenterDistance = MandibularCondylesAlgorithm_GetRightToCenterDistance(instance);

            Result =
                string.Format("L [px]: {0}, {1}, {2} px \n", LeftCondylePosition[0], LeftCondylePosition[1], LeftCondylePosition[2]) +
                string.Format("R [px]: {0}, {1}, {2} px \n", RightCondylePosition[0], RightCondylePosition[1], RightCondylePosition[2]) +
                string.Format("Diff: {0}, {1}, {2} px \n", CondylesPositionDifference[0], CondylesPositionDifference[1], CondylesPositionDifference[2]) +
                string.Format("L [mm]: {0:0.000}, {1:0.000}, {2:0.000} \n", LeftCondylePositionReal[0], LeftCondylePositionReal[1], LeftCondylePositionReal[2]) +
                string.Format("R [mm]: {0:0.000}, {1:0.000}, {2:0.000} mm \n", RightCondylePositionReal[0], RightCondylePositionReal[1], RightCondylePositionReal[2]) +
                string.Format("Diff: {0:0.000}, {1:0.000}, {2:0.000} mm \n", RightCondylePositionReal[0] - LeftCondylePositionReal[0], RightCondylePositionReal[1] - LeftCondylePositionReal[1], RightCondylePositionReal[2] - LeftCondylePositionReal[2]) +
                string.Format("L-R: {0:0.00} ({1:0.000} mm) \n", CondylesDistancePx, CondylesDistance) +
                string.Format("L-N: {0:0.000} mm, R-N: {1:0.000} mm \n", Left2NasionDistance, Right2NasionDistance) +
                string.Format("L-C: {0:0.000} mm, R-C: {1:0.000} mm", Left2CenterDistance, Right2CenterDistance);

            // TODO: rotations?
            //MandibularBaseAlgorithm_GetLeftCondyleRotation(instance, LeftCondyleRotation);
            //MandibularBaseAlgorithm_GetRightCondyleRotation(instance, RightCondyleRotation);
        }

        double[] GetPoint(NativeGetter getter)
        {
            getter(instance, out IntPtr arr, out int size);
            var val = new double[size];
            Marshal.Copy(arr, val, 0, size);
            return val;
        }

        double[] LeftCondylePosition { get; set; }
        double[] RightCondylePosition { get; set; }
        double[] LeftCondyleRotation { get; set; }
        double[] RightCondyleRotation { get; set; }

        double[] CondylesPositionDifference { get; set; }
        double CondylesDistancePx { get; set; }
        double CondylesDistance { get; set; }

        double[] NasionPosition { get; set; }
        double[] Left2NasionDifference { get; set; }
        double[] Right2NasionDifference { get; set; }
        double Left2NasionDistance { get; set; }
        double Right2NasionDistance { get; set; }
        double Left2NasionDistancePx { get; set; }
        double Right2NasionDistancePx { get; set; }

        double[] LeftZygomaticPoint { get; set; }
        double[] RightZygomaticPoint { get; set; }
        double[] Left2CenterDifference { get; set; }
        double[] Right2CenterDifference { get; set; }
        double[] CenterPosition { get; set; }
        double Left2CenterDistance { get; set; }
        double Right2CenterDistance { get; set; }

        double[] Origin { get; set; }
        double[] Spacing { get; set; }

        double[] LeftCondylePositionReal { get; set; }
        double[] RightCondylePositionReal { get; set; }

        public string GetCsvFormattedHeader()
        {
            var sb = new StringBuilder();
            var props = GetProperties();
            for (int i = 0; i < props.Length; i++)
            {
                sb.Append(string.Format("{0}", props[i]));
                if (i != props.Length - 1)
                    sb.Append(";");
            }
            sb.AppendLine();
            return sb.ToString();
        }

        public string GetCsvFormattedResult()
        {
            var sb = new StringBuilder();
            sb.AppendFormat("{0};", PatientName);
            sb.AppendFormat("{0};", PatientId);
            sb.AppendFormat("{0};", AcquisitionDate);
            sb.AppendFormat("{0};", Success);
            sb.AppendFormat("{0};", TimeSpan.FromMilliseconds(Duration).Format());
            sb.AppendFormat("{0};", Math.Round(LeftCondylePosition[0], 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftCondylePosition[1], 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftCondylePosition[2], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(LeftCondyleRotation[0], 2, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(LeftCondyleRotation[1], 2, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(LeftCondyleRotation[2], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightCondylePosition[0], 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightCondylePosition[1], 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightCondylePosition[2], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(CondylesPositionDifference[0], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(CondylesPositionDifference[1], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(CondylesPositionDifference[2], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(CondylesDistancePx, 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(CondylesDistance, 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(NasionPosition[0], 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(NasionPosition[1], 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(NasionPosition[2], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Left2NasionDifference[0], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Left2NasionDifference[1], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Left2NasionDifference[2], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Right2NasionDifference[0], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Right2NasionDifference[1], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Right2NasionDifference[2], 0, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Left2NasionDistancePx, 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Left2NasionDistance, 3, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(Right2NasionDistancePx, 0, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Right2NasionDistance, 3, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(RightCondyleRotation[0], 2, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(RightCondyleRotation[1], 2, MidpointRounding.AwayFromZero));
            //sb.AppendFormat("{0};", Math.Round(RightCondyleRotation[2], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftZygomaticPoint[0], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftZygomaticPoint[1], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftZygomaticPoint[2], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightZygomaticPoint[0], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightZygomaticPoint[1], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightZygomaticPoint[2], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(CenterPosition[0], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(CenterPosition[1], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(CenterPosition[2], 2, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Left2CenterDistance, 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Right2CenterDistance, 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Origin[0], 6, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Origin[1], 6, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Origin[2], 6, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Spacing[0], 6, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Spacing[1], 6, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(Spacing[2], 6, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Result.Replace('\n', ' ').Trim());
            sb.AppendFormat("{0};", Path);
            sb.AppendFormat("{0};", Math.Round(LeftCondylePositionReal[0], 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftCondylePositionReal[1], 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(LeftCondylePositionReal[2], 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightCondylePositionReal[0], 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0};", Math.Round(RightCondylePositionReal[1], 3, MidpointRounding.AwayFromZero));
            sb.AppendFormat("{0}", Math.Round(RightCondylePositionReal[2], 3, MidpointRounding.AwayFromZero));
            sb.AppendLine();
            return sb.ToString();
        }

        static string[] GetProperties()
        {
            return properties ?? (properties = new string[]
            {
                "PatientName",
                "PatientId",
                "Date",
                "Success",
                "Duration",
                Mandibular.LeftCondyleXTag,
                Mandibular.LeftCondyleYTag,
                Mandibular.LeftCondyleZTag,
                //"LRotX",
                //"LRotY",
                //"LRotZ",
                Mandibular.RightCondyleXTag,
                Mandibular.RightCondyleYTag,
                Mandibular.RightCondyleZTag,
                //"RRotX",
                //"RRotY",
                //"RRotZ",
                //"XDiff",
                //"YDiff",
                //"ZDiff",
                //"Distance [px]",
                Mandibular.CondylesDistanceTag,
                Mandibular.NasionXTag,
                Mandibular.NasionYTag,
                Mandibular.NasionZTag,
                //"Left2NasionDiffX",
                //"Left2NasionDiffY",
                //"Left2NasionDiffZ",
                //"Right2NasionDiffX",
                //"Right2NasionDiffY",
                //"Right2NasionDiffZ",
                //"Left2NasionDist [px]",
                Mandibular.LeftConToNasionDistanceTag,
                //"Right2NasionDist [px]",
                Mandibular.RightConToNasionDistanceTag,
                Mandibular.LeftZygomaticXTag,
                Mandibular.LeftZygomaticYTag,
                Mandibular.LeftZygomaticZTag,
                Mandibular.RightZygomaticXTag,
                Mandibular.RightZygomaticYTag,
                Mandibular.RightZygomaticZTag,
                Mandibular.CenterXTag,
                Mandibular.CenterYTag,
                Mandibular.CenterZTag,
                Mandibular.LeftZygToCenterDistanceTag,
                Mandibular.RightZygToCenterDistanceTag,
                Mandibular.OriginXTag,
                Mandibular.OriginYTag,
                Mandibular.OriginZTag,
                Mandibular.SpacingXTag,
                Mandibular.SpacingYTag,
                Mandibular.SpacingZTag,
                "Result",
                "Path",
                Mandibular.LeftCondyleXRealTag,
                Mandibular.LeftCondyleYRealTag,
                Mandibular.LeftCondyleZRealTag,
                Mandibular.RightCondyleXRealTag,
                Mandibular.RightCondyleYRealTag,
                Mandibular.RightCondyleZRealTag
            });
        }
        static string[] properties;

        public void DumpIntermediates(string outputFolder)
        {
            MandibularCondylesAlgorithm_DumpIntermediates(instance, outputFolder);
        }
    }

    public static class Mandibular
    {
        public static string LeftZygomaticXTag = "LZygX";
        public static string LeftZygomaticYTag = "LZygY";
        public static string LeftZygomaticZTag = "LZygZ";

        public static string RightZygomaticXTag = "RZygX";
        public static string RightZygomaticYTag = "RZygY";
        public static string RightZygomaticZTag = "RZygZ";

        public static string NasionXTag = "NsX";
        public static string NasionYTag = "NsY";
        public static string NasionZTag = "NsZ";

        public static string CenterXTag = "CenterX";
        public static string CenterYTag = "CenterY";
        public static string CenterZTag = "CenterZ";

        public static string OriginXTag = "OriginX";
        public static string OriginYTag = "OriginY";
        public static string OriginZTag = "OriginZ";

        public static string SpacingXTag = "SpacingX";
        public static string SpacingYTag = "SpacingY";
        public static string SpacingZTag = "SpacingZ";

        public static string LeftCondyleXTag = "LPosX";
        public static string LeftCondyleYTag = "LPosY";
        public static string LeftCondyleZTag = "LPosZ";
        public static string RightCondyleXTag = "RPosX";
        public static string RightCondyleYTag = "RPosY";
        public static string RightCondyleZTag = "RPosZ";

        public static string CondylesDistanceTag = "Dst [mm]";

        public static string LeftConToNasionDistanceTag = "L2NDist [mm]";
        public static string RightConToNasionDistanceTag = "R2NDist [mm]";

        public static string LeftZygToCenterDistanceTag = "L2CDist [mm]";
        public static string RightZygToCenterDistanceTag = "R2CDist [mm]";

        public static string LeftCondyleXRealTag = "LPosXReal";
        public static string LeftCondyleYRealTag = "LPosYReal";
        public static string LeftCondyleZRealTag = "LPosZReal";
        public static string RightCondyleXRealTag = "RPosXReal";
        public static string RightCondyleYRealTag = "RPosYReal";
        public static string RightCondyleZRealTag = "RPosZReal";
    }
}
