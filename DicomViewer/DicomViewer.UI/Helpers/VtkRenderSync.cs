using DicomViewer.Lib.Threading;
using DicomViewer.UI.Controls;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace DicomViewer.UI.Helpers
{
    public class VtkRenderSync : IDisposable
    {
        #region Extern
        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr VtkRenderSync_CreateInstance();

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRenderSync_Dispose(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRenderSync_RegisterReslicer(IntPtr instance, IntPtr render);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRenderSync_Cleanup(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRenderSync_SetResliceCenter(IntPtr instance, double x, double y, double z);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRenderSync_SetResliceNormal(IntPtr instance, double x, double y, double z);
       
        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRenderSync_ResetReslicers(IntPtr instance);

        #endregion

        public VtkRenderSync()
        {
            instance = VtkRenderSync_CreateInstance();
        }
        IntPtr instance = IntPtr.Zero;

        public IntPtr GetInstance() => instance;

        public void RegisterReslicer(VtkRenderWindow window)
        {
            var win = window.GetInstance();
            lock (locker)
            {
                VtkRenderSync_RegisterReslicer(instance, win);
                registered[win] = true;
            }
        }
        readonly object locker = new object();
        public bool IsRegistered => registered.Count >= 3;
        private readonly Dictionary<IntPtr, bool> registered = new Dictionary<IntPtr, bool>();

        public void Enter()
        {
            Monitor.Enter(renderLocker);
        }
        readonly object renderLocker = new object();

        public void Exit()
        {
            Monitor.Exit(renderLocker);
        }

        public void Cleanup()
        {
            registered.Clear();
            VtkRenderSync_Cleanup(instance);
        }

        public void Dispose()
        {
            registered.Clear();
            VtkRenderSync_Dispose(instance);
            instance = IntPtr.Zero;
        }
        public void SetResliceCenter(int[] center)
        {
            SetResliceCenter(center[0], center[1], center[2]);
        }
        public void SetResliceCenter(double[] center)
        {
            SetResliceCenter(center[0], center[1], center[2]);
        }
        public void SetResliceCenter(double x, double y, double z)
        {
            VtkRenderSync_SetResliceCenter(instance, x, y, z);
        }

        public void SetResliceNormal(double[] normal)
        {
            SetResliceNormal(normal[0], normal[1], normal[2]);
        }
        public void SetResliceNormal(double x, double y, double z)
        {
            VtkRenderSync_SetResliceNormal(instance, x, y, z);
        }
        public void ResetReslicers()
        {
            VtkRenderSync_ResetReslicers(instance);
        }
    }
}
