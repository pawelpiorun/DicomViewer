using System;
using System.Windows.Input;

namespace DicomViewer.Lib.Common
{
    public class ActionCommand : ICommand
    {
        public ActionCommand(Action executeMethod, Func<bool> canExecuteMethod = null)
        {
            this.executeMethod = executeMethod;
            this.canExecuteMethod = canExecuteMethod;
        }

        readonly Action executeMethod;
        readonly Func<bool> canExecuteMethod;

#pragma warning disable 0067
        public event EventHandler CanExecuteChanged;
#pragma warning restore 0067

        bool CanExecute()
        {
            var m = canExecuteMethod;
            if (m != null)
                return m();

            return true;
        }

        public void Execute()
        {
            executeMethod?.Invoke();
        }

        bool ICommand.CanExecute(object parameter)
        {
            return CanExecute();
        }

        void ICommand.Execute(object parameter)
        {
            Execute();
        }
    }

    public class ActionCommand<T> : ICommand
    {
        public ActionCommand(Action<T> executeMethod, Func<T, bool> canExecuteMethod = null)
        {
            this.executeMethod = executeMethod;
            this.canExecuteMethod = canExecuteMethod;
        }
        readonly Action<T> executeMethod;
        readonly Func<T, bool> canExecuteMethod;

#pragma warning disable 0067
        public event EventHandler CanExecuteChanged;
#pragma warning restore 0067

        bool ICommand.CanExecute(object parameter)
        {
            return CanExecute((T)parameter);
        }

        bool CanExecute(T parameter)
        {
            if (canExecuteMethod != null)
                return canExecuteMethod.Invoke(parameter);

            return true;
        }

        void ICommand.Execute(object parameter)
        {
            Execute((T)parameter);
        }

        void Execute(T parameter)
        {
            executeMethod?.Invoke(parameter);
        }
    }
}
