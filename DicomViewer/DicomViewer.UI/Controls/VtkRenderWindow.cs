using DicomViewer.Lib.Extensions;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.Threading;
using DicomViewer.UI.Helpers;
using Serilog;
using System;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media.Media3D;

namespace DicomViewer.UI.Controls
{
    public class VtkRenderWindow : ControlHost
    {
        #region Extern

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr VtkRender_CreateInstance();

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetRenderHost(IntPtr instance, IntPtr handle);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_RenderTestVolume(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]

        public static extern void VtkRender_LoadDicomVolume(IntPtr instance, IntPtr volume);
        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_LoadAdditionalDicomVolume(IntPtr instance, IntPtr volume);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetWindowSize(IntPtr instance, int width, int height);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_Dispose(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_StartRender(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_Render(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_UpdateData(IntPtr instance, IntPtr dicomSeries);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetPreset(IntPtr instance, IntPtr preset);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetPresetsSource(IntPtr instance, IntPtr presetsSingleton);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetRenderOption(IntPtr instance, int option);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetSliceIndex(IntPtr instance, int index);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int VtkRender_GetSliceIndex(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern int VtkRender_GetMaxSliceIndex(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_ToggleResliceCursors(IntPtr instance, bool show);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetRenderSync(IntPtr instance, IntPtr sync);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SetAdditionalVolumeTransform(IntPtr instance, double[] transform);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_ToggleShowSecondVolume(IntPtr instance, bool show);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_SelectPoint(IntPtr instance, double x, double y, double z, bool onFirst);

        [DllImport("BimaxProcessing.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void VtkRender_ClearSelectedPoints(IntPtr instance, bool onFirst);
        #endregion

        public VtkRenderWindow() : base(0, 0)
        {
            this.Loaded += OnLoaded;
        }
        IntPtr instance = IntPtr.Zero;

        public IntPtr GetInstance()
        {
            return instance;
        }

        void Initialize()
        {
            instance = VtkRender_CreateInstance();
            VtkRender_SetRenderHost(instance, this.Handle);
            VtkRender_SetPresetsSource(instance, Presets.Instance.GetPresetsInstance());
            VtkRender_SetRenderOption(instance, (int)RenderType);
        }

        double scaleX = 1.0d;
        double scaleY = 1.0d;
        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnLoaded;
            Initialize();
            RegisterEvents();
            if (RenderContent != null && RenderContent.IsLoaded)
                Render(RenderContent.GetPointer());
        }

        private void OnRenderContentReady(object sender, EventArgs e)
        {
            if (sender as DicomSeries is null) return;
            Render((sender as DicomSeries).GetPointer());
        }

        private void OnContentCleaningUp(object sender, EventArgs e)
        {
            if (!(sender is DicomSeries dicomSeries)) return;

            dicomSeries.ContentReady -= OnRenderContentReady;
            dicomSeries.RenderRequested -= OnContentRenderRequested;
            dicomSeries.CleaningUp -= OnContentCleaningUp;
            VtkRender_Dispose(instance);
            if (RenderSync != null)
                RenderSync.Cleanup();
            instance = IntPtr.Zero;
        }

        private void OnContentRenderRequested(object sender, EventArgs e)
        {
            if (sender is not DicomSeries dicomSeries) return;
            try
            {
                ReRender(dicomSeries.GetPointer());
            }
            catch (Exception x)
            {
                Log.Fatal(x, "Failed to re-render content.");
            }
        }

        bool needsReRendering = false;
        void ReRender(IntPtr volume)
        {
            VtkRender_UpdateData(instance, volume);

            if (RenderSync is null)
            {
                needsReRendering = true;
                return;
            }

            Threads.UI(() =>
            {
                RenderSync.Enter();
                VtkRender_Render(instance);
                RenderSync.Exit();
            });
        }

        void RegisterEvents()
        {
            if (isRegistered) return;
            isRegistered = true;

            if (Parent is not FrameworkElement parent) return;
            WeakEventManager<FrameworkElement, SizeChangedEventArgs>.AddHandler(parent, nameof(SizeChanged), OnParentSizeChanged);
        }
        bool isRegistered = false;

        void OnParentSizeChanged(object sender, SizeChangedEventArgs e)
        {
            UpdateControlHostSize();
        }

        protected void UpdateControlHostSize()
        {
            if (Parent is not FrameworkElement parent) return;

            UpdateDpiScaling();

            var width = parent.ActualWidth;
            var height = parent.ActualHeight;

            SetSize(width, height);
        }

        private void UpdateDpiScaling()
        {
            // take into consideration custom dpi scaling
            var source = PresentationSource.FromVisual(this);
            scaleX = source.CompositionTarget.TransformToDevice.M11;
            scaleY = source.CompositionTarget.TransformToDevice.M11;
        }

        public override void SetSize(double width, double height)
        {
            Width = width;
            Height = height;

            if (instance == IntPtr.Zero) return;
            VtkRender_SetWindowSize(instance, (int)(scaleX * width), (int)(scaleY * height));

            base.SetSize(width, height);
        }

        protected void Render(IntPtr volume)
        {
            if (instance == IntPtr.Zero)
                Initialize();

            // TODO: check what is the type of content, render respectively
            // for now expecting dicom volume only
            VtkRender_LoadDicomVolume(instance, volume);

            if (RenderType != RenderType.Volumetric && RenderSync != null
                && !RenderSync.IsRegistered)
            {
                RenderSync.RegisterReslicer(this);
                VtkRender_SetRenderSync(instance, RenderSync.GetInstance());
            }

            if (RenderType != RenderType.Volumetric)
            {
                MaxSliceIndex = VtkRender_GetMaxSliceIndex(instance);
                SliceIndex = VtkRender_GetSliceIndex(instance);
            }

            VtkRender_Render(instance);
            UpdateControlHostSize();
            SetVtkPreset(Preset);
        }

        public void SetVtkPreset(Preset preset)
        {
            if (instance == IntPtr.Zero) return;

            VtkRender_SetPreset(instance, preset.UnmanagedInstance);
        }

        public Preset Preset
        {
            get { return (Preset)GetValue(PresetProperty); }
            set { SetValue(PresetProperty, value); }
        }

        public static readonly DependencyProperty PresetProperty =
            DependencyProperty.Register(
                nameof(Preset),
                typeof(Preset),
                typeof(VtkRenderWindow),
                new PropertyMetadata(
                    Presets.Instance.GetDefaultPresets().FirstOrDefault(),
                    OnPresetChanged));

        private static void OnPresetChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow renderWindow
                || e.NewValue is not Preset newPreset) return;
            renderWindow.SetVtkPreset(newPreset);
        }

        public IRenderContent RenderContent
        {
            get { return (IRenderContent)GetValue(RenderContentProperty); }
            set { SetValue(RenderContentProperty, value); }
        }

        public static readonly DependencyProperty RenderContentProperty =
            DependencyProperty.Register(nameof(RenderContent), typeof(IRenderContent), typeof(VtkRenderWindow), new PropertyMetadata(null, OnRenderContentChanged));

        private static void OnRenderContentChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow renderWindow) return;

            if (e.OldValue is IRenderContent oldContent)
            {
                oldContent.ContentReady -= renderWindow.OnRenderContentReady;
                oldContent.RenderRequested -= renderWindow.OnContentRenderRequested;
                oldContent.CleaningUp -= renderWindow.OnContentCleaningUp;
            }

            if (e.NewValue is IRenderContent newContent)
            {
                newContent.ContentReady += renderWindow.OnRenderContentReady;
                newContent.RenderRequested += renderWindow.OnContentRenderRequested;
                newContent.CleaningUp += renderWindow.OnContentCleaningUp;
            }
        }

        public IRenderContent AdditionalContent
        {
            get { return (IRenderContent)GetValue(AdditionalContentProperty); }
            set { SetValue(AdditionalContentProperty, value); }
        }

        public static readonly DependencyProperty AdditionalContentProperty =
            DependencyProperty.Register(nameof(AdditionalContent), typeof(IRenderContent), typeof(VtkRenderWindow), new PropertyMetadata(null, OnAdditionalRenderContentChanged));

        private static void OnAdditionalRenderContentChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow renderWindow) return;

            if (e.OldValue is IRenderContent oldContent)
            {
                oldContent.ContentReady -= renderWindow.OnAdditionalRenderContentReady;
                //oldContent.RenderRequested -= renderWindow.OnContentRenderRequested;
                //oldContent.CleaningUp -= renderWindow.OnContentCleaningUp;
            }

            if (e.NewValue is IRenderContent newContent)
            {
                newContent.ContentReady += renderWindow.OnAdditionalRenderContentReady;
                //newContent.RenderRequested += renderWindow.OnContentRenderRequested;
                //newContent.CleaningUp += renderWindow.OnContentCleaningUp;
            }
        }

        private void OnAdditionalRenderContentReady(object sender, EventArgs e)
        {
            if (sender as DicomSeries is null) return;
            RenderAdditional((sender as DicomSeries).GetPointer());
        }

        protected void RenderAdditional(IntPtr volume)
        {
            if (instance == IntPtr.Zero)
                Initialize();
            VtkRender_LoadAdditionalDicomVolume(instance, volume);
            UpdateAdditionalVolumeTransform(AdditionalVolumeTransform);
            UpdateSelectedPoints(nameof(SelectedPoints));
            UpdateSelectedPoints(nameof(AdditionalSelectedPoints));
            VtkRender_Render(instance);
        }

        public BindingList<Point3D> SelectedPoints
        {
            get { return (BindingList<Point3D>)GetValue(SelectedPointsProperty); }
            set { SetValue(SelectedPointsProperty, value); }
        }

        public static readonly DependencyProperty SelectedPointsProperty =
            DependencyProperty.Register(nameof(SelectedPoints), typeof(BindingList<Point3D>), typeof(VtkRenderWindow), new PropertyMetadata(null, OnSelectedPointsPropertyChanged));

        public BindingList<Point3D> AdditionalSelectedPoints
        {
            get { return (BindingList<Point3D>)GetValue(AdditionalSelectedPointsProperty); }
            set { SetValue(AdditionalSelectedPointsProperty, value); }
        }

        public static readonly DependencyProperty AdditionalSelectedPointsProperty =
            DependencyProperty.Register(nameof(AdditionalSelectedPoints), typeof(BindingList<Point3D>), typeof(VtkRenderWindow), new PropertyMetadata(null, OnSelectedPointsPropertyChanged));

        private static void OnSelectedPointsPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow window) return;
            var oldList = e.OldValue as BindingList<Point3D>;
            var newList = e.NewValue as BindingList<Point3D>;
            if (oldList != null)
            {
                oldList.ListChanged -= window.OnSelectedPointsChanged;
                var onFirst = e.Property.Name == nameof(SelectedPoints);
                if (window.instance != IntPtr.Zero && !onFirst)
                    VtkRender_ClearSelectedPoints(window.instance, onFirst);
            }
            if (newList != null)
            {
                newList.ListChanged += window.OnSelectedPointsChanged;
                window.UpdateSelectedPoints(e.Property.Name);
            }
        }

        private void OnSelectedPointsChanged(object sender, ListChangedEventArgs e)
        {
            UpdateSelectedPoints(e.PropertyDescriptor.Name);
        }

        void UpdateSelectedPoints(string listName)
        {
            bool onFirst = listName == nameof(SelectedPoints);
            if (instance == IntPtr.Zero) return;
            var list = onFirst ? SelectedPoints : AdditionalSelectedPoints;
            foreach (var pt in list)
            {
                VtkRender_SelectPoint(instance, pt.X, pt.Y, pt.Z, onFirst);
            }
        }

        public MatrixTransform3D AdditionalVolumeTransform
        {
            get { return (MatrixTransform3D)GetValue(AdditionalVolumeTranslateProperty); }
            set { SetValue(AdditionalVolumeTranslateProperty, value); }
        }

        public static readonly DependencyProperty AdditionalVolumeTranslateProperty =
            DependencyProperty.Register(
                nameof(AdditionalVolumeTransform),
                typeof(MatrixTransform3D),
                typeof(VtkRenderWindow),
                new PropertyMetadata(null, OnTransformPropertyChanged));

        private static void OnTransformPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var window = d as VtkRenderWindow;
            if (window is null || window.AdditionalContent is null) return;

            window.UpdateAdditionalVolumeTransform(e.NewValue as MatrixTransform3D);
        }

        private void UpdateAdditionalVolumeTransform(MatrixTransform3D transform)
        {
            if (transform is null || instance == IntPtr.Zero) return;

            var mtxArr = transform.ToArray();
            VtkRender_SetAdditionalVolumeTransform(instance, mtxArr);
        }

        public bool ShowSecondVolume
        {
            get { return (bool)GetValue(ShowSecondVolumeProperty); }
            set { SetValue(ShowSecondVolumeProperty, value); }
        }

        public static readonly DependencyProperty ShowSecondVolumeProperty =
            DependencyProperty.Register(nameof(ShowSecondVolume), typeof(bool), typeof(VtkRenderWindow), new PropertyMetadata(true, OnShowSecondVolumeChanged));

        private static void OnShowSecondVolumeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow window) return;
            VtkRender_ToggleShowSecondVolume(window.instance, (bool)e.NewValue);
        }

        public RenderType RenderType
        {
            get { return (RenderType)GetValue(RenderTypeProperty); }
            set { SetValue(RenderTypeProperty, value); }
        }

        public static readonly DependencyProperty RenderTypeProperty =
            DependencyProperty.Register(nameof(RenderType), typeof(RenderType), typeof(VtkRenderWindow), new PropertyMetadata(RenderType.Volumetric));

        public int SliceIndex
        {
            get { return (int)GetValue(SliceIndexProperty); }
            set { SetValue(SliceIndexProperty, value); }
        }

        public static readonly DependencyProperty SliceIndexProperty =
            DependencyProperty.Register(nameof(SliceIndex), typeof(int), typeof(VtkRenderWindow), new PropertyMetadata(0, OnSliceIndexChanged));

        private static void OnSliceIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow renderWindow) return;

            if (renderWindow.RenderType == RenderType.Volumetric) return;
            renderWindow.SetNativeSliceIndex((int)e.NewValue);
        }

        void SetNativeSliceIndex(int newValue)
        {
            if (instance != IntPtr.Zero)
                VtkRender_SetSliceIndex(instance, newValue);
        }

        public int MaxSliceIndex
        {
            get { return (int)GetValue(MaxSliceIndexProperty); }
            set { SetValue(MaxSliceIndexProperty, value); }
        }

        public static readonly DependencyProperty MaxSliceIndexProperty =
            DependencyProperty.Register(nameof(MaxSliceIndex), typeof(int), typeof(VtkRenderWindow), new PropertyMetadata(0));

        public VtkRenderSync RenderSync
        {
            get { return (VtkRenderSync)GetValue(RenderSyncProperty); }
            set { SetValue(RenderSyncProperty, value); }
        }

        public static readonly DependencyProperty RenderSyncProperty =
            DependencyProperty.Register(nameof(RenderSync), typeof(VtkRenderSync), typeof(VtkRenderWindow), new PropertyMetadata(null, OnRenderSyncChanged));

        private static void OnRenderSyncChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow renderWindow) return;
            renderWindow.OnRenderSyncChanged();
        }

        private void OnRenderSyncChanged()
        {
            if (needsReRendering && RenderSync != null)
                ReRender(RenderContent.GetPointer());
        }

        public bool ShowResliceCursors
        {
            get { return (bool)GetValue(ShowResliceCursorsProperty); }
            set { SetValue(ShowResliceCursorsProperty, value); }
        }

        public static readonly DependencyProperty ShowResliceCursorsProperty =
            DependencyProperty.Register("ShowResliceCursors", typeof(bool), typeof(VtkRenderWindow), new PropertyMetadata(true, OnShowResliceCursorsChanged));

        private static void OnShowResliceCursorsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not VtkRenderWindow renderWindow) return;
            renderWindow.UpdateResliceCursors((bool)e.NewValue);
        }

        void UpdateResliceCursors(bool show)
        {
            VtkRender_ToggleResliceCursors(instance, show);
            VtkRender_Render(instance);
        }
    }

    public enum RenderType
    {
        Volumetric = 0,
        FlatXY = 1,
        FlatXZ = 2,
        FlatYZ = 3
    }
}
