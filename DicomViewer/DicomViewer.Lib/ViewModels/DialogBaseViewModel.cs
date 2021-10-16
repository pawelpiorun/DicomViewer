using DicomViewer.Lib.Common;
using DicomViewer.Lib.Interfaces;
using DicomViewer.Lib.Threading;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.ViewModels
{
    public abstract class DialogBaseViewModel : NotifyPropertyChanged
    {
        public virtual string Title
        {
            get { return title; }
            set
            {
                if (title == value) return;
                title = value;
                OnPropertyChanged();
            }
        }
        string title;

        public virtual string SubTitle
        {
            get { return subTitle; }
            set
            {
                if (subTitle == value) return;
                subTitle = value;
                OnPropertyChanged();
            }
        }
        string subTitle;

        public virtual string Message
        {
            get { return message; }
            set
            {
                if (message == value) return;
                message = value;
                OnPropertyChanged();
            }
        }
        string message;

        public virtual void Show()
        {
            var dialogService = ServiceRegister.Resolve<IDialogService>();
            if (dialogService is null)
                return;

            Threads.Background(() => Threads.UISync(() => dialogService.OpenDialog(this)));
        }

        public virtual void Close()
        {
            Threads.UISync(() => CloseRequested?.Invoke(this, EventArgs.Empty));
        }

        public EventHandler CloseRequested;
    }

    public abstract class DialogBaseViewModel<TResult> : DialogBaseViewModel
    {
        public TResult Result { get; private set; }

        public override void Show()
        {
            var dialogService = ServiceRegister.Resolve<IDialogService>();
            if (dialogService is null)
                return;

            Result = dialogService.OpenDialog<TResult>(this);
        }
    }
}
