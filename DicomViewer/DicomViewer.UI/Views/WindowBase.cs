using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;

namespace DicomViewer.UI.Views
{
    public class WindowBase : Window
    {
        public string SubTitle
        {
            get { return (string)GetValue(SubTitleProperty); }
            set { SetValue(SubTitleProperty, value); }
        }

        public static readonly DependencyProperty SubTitleProperty =
            DependencyProperty.Register(nameof(SubTitle), typeof(string), typeof(WindowBase), new PropertyMetadata(null));

        [DllImport("user32")]
        public static extern int FlashWindow(IntPtr hwnd, bool bInvert);

        public WindowBase()
        {
            this.CommandBindings.Add(new CommandBinding(SystemCommands.CloseWindowCommand, Close));
            this.CommandBindings.Add(new CommandBinding(SystemCommands.RestoreWindowCommand, Restore, CanResize));
            this.CommandBindings.Add(new CommandBinding(SystemCommands.MinimizeWindowCommand, Minimize, CanMinimize));
            this.CommandBindings.Add(new CommandBinding(SystemCommands.MaximizeWindowCommand, Maximize, CanResize));
        }

        private void CanResize(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = this.ResizeMode == ResizeMode.CanResize || this.ResizeMode == ResizeMode.CanResizeWithGrip;
        }

        private void CanMinimize(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = this.ResizeMode != ResizeMode.NoResize;
        }

        private void Close(object target, ExecutedRoutedEventArgs e)
        {
            SystemCommands.CloseWindow(this);
        }

        private void Maximize(object target, ExecutedRoutedEventArgs e)
        {
            SystemCommands.MaximizeWindow(this);
        }

        private void Minimize(object target, ExecutedRoutedEventArgs e)
        {
            SystemCommands.MinimizeWindow(this);
        }

        private void Restore(object target, ExecutedRoutedEventArgs e)
        {
            SystemCommands.RestoreWindow(this);
        }

        public void Flash()
        {
            WindowInteropHelper wih = new WindowInteropHelper(this);
            FlashWindow(wih.Handle, true);
        }
    }
}
