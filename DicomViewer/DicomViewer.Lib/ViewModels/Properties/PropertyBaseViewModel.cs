using DicomViewer.Lib.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.ViewModels.Properties
{
    public class PropertyBaseViewModel : NotifyPropertyChanged
    {
        public PropertyBaseViewModel(string title)
        {
            this.Title = title;
        }

        public string Title
        {
            get { return title; }
            set
            {
                if (string.Equals(title, value)) return;
                title = value;
                OnPropertyChanged();
            }
        }
        string title;
    }
}
