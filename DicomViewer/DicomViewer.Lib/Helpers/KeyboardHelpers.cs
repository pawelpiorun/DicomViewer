using System;
using System.Windows;
using System.Windows.Input;

namespace DicomViewer.Lib.Helpers
{
    public class KeyboardHelpers
    {
        private KeyboardHelpers()
        {
            EventManager.RegisterClassHandler(
                typeof(UIElement),
                Keyboard.PreviewKeyDownEvent,
                new RoutedEventHandler(OnGlobalKeyDown));
        }

        void OnGlobalKeyDown(object sender, RoutedEventArgs e)
        {
            var keyEventArgs = e as KeyEventArgs;
            if (keyEventArgs == null
                || !(sender == keyEventArgs.InputSource?.RootVisual)) return;

            KeyDown?.Invoke(sender, keyEventArgs);
        }

        public event EventHandler<KeyEventArgs> KeyDown;

        public static KeyboardHelpers Instance
        {
            get
            {
                if (keyboardHelpers == null)
                    keyboardHelpers = new KeyboardHelpers();
                return keyboardHelpers;
            }
        }
        static KeyboardHelpers keyboardHelpers;
    }
}
