using Serilog;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace DicomViewer.Lib.Threading
{
    public class Threads
    {
        public static Threads Instance => instance.Value;
        private static readonly Lazy<Threads> instance = new Lazy<Threads>(() => new Threads());

        public static Dispatcher UIDispatcher
        {
            get => Instance.Dispatcher;
            set => Instance.Dispatcher = value;
        }
        Dispatcher Dispatcher { get; set; }

        private Dispatcher GetDispatcher()
        {
            var dispatcher = default(Dispatcher);
            if (Application.Current != null)
                dispatcher = Application.Current.Dispatcher;
            else if (Dispatcher != null)
                dispatcher = Dispatcher;
            else if (Dispatcher.CurrentDispatcher != null)
                dispatcher = Dispatcher.CurrentDispatcher;

            return dispatcher;
        }

        public static void UI(Action a) => Instance.UIPriv(a);
        public static void UI(DispatcherPriority priority, Action a) => Instance.UIPriv(priority, a);
        void UIPriv(Action a)
        {
            UIPriv(DispatcherPriority.Normal, a);
        }
        void UIPriv(DispatcherPriority priority, Action a)
        {
            var dispatcher = GetDispatcher();
            if (dispatcher == null)
            {
                a();
                return;
            }

            if (dispatcher.CheckAccess())
                a();
            else
                dispatcher.BeginInvoke(priority, a);
        }

        public static void UISync(Action a) => Instance.UISyncPriv(a);
        public static void UISync(DispatcherPriority priority, Action a, Dispatcher dispatcher = null)
            => Instance.UISyncPriv(priority, a, dispatcher);
        void UISyncPriv(Action a)
        {
            Instance.UISyncPriv(DispatcherPriority.Normal, a);
        }

        void UISyncPriv(DispatcherPriority priority, Action a, Dispatcher dispatcher = null)
        {
            if (dispatcher == null)
                dispatcher = GetDispatcher();
            if (dispatcher == null)
            {
                a();
                return;
            }

            if (dispatcher.CheckAccess())
                a();
            else
            {
                try
                {
                    dispatcher.Invoke(a, priority);
                }
                catch (TaskCanceledException)
                {

                }
                catch (Exception x)
                {
                    Log.Fatal(x, "Failed to run action on dispatcher.");
                }
            }
        }

        public static void UIDelayed(int ms, Action a) => Instance.UIDelayedPriv(ms, a);
        public static void UIDelayed(Action a) => Instance.UIDelayedPriv(a);
        public static void UIDelayed(IEnumerable<Action> actions) => Instance.UIDelayedPriv(actions);
        
        void UIDelayedPriv(int ms, Action a)
        {
            Background(() =>
            {
                Thread.Sleep(ms);
                UI(a);
            });
        }
        void UIDelayedPriv(Action a)
        {
            Background(() =>
            {
                Thread.Sleep(1);
                UI(a);
            });
        }

        void UIDelayedPriv(IEnumerable<Action> actions)
        {
            Background(() =>
            {
                foreach (var a in actions.ToList())
                {
                    Thread.Sleep(1);
                    UI(a);
                }
            });
        }


        public static void Background(Action a) => Instance.BackgroundPriv(a);
        public static void BackgroundDelayed(Action a, int delayMs) => Instance.BackgroundDelayedPriv(a, delayMs);
        public static void BackgroundWorker(Action a) => Instance.BackgroundWorkerPriv(a);

        void BackgroundPriv(Action a)
        {
            ThreadPool.QueueUserWorkItem(x => a());
        }

        void BackgroundDelayedPriv(Action a, int delayMs)
        {
            BackgroundPriv(() =>
            {
                Thread.Sleep(delayMs);

                a();
            });
        }

        void BackgroundWorkerPriv(Action a)
        {
            var worker = new BackgroundWorker();
            worker.DoWork += (object sender, DoWorkEventArgs e) =>
            {
                a();
                worker.Dispose();
            };
            worker.RunWorkerAsync();

        }
    }
}
