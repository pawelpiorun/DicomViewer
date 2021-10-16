using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.ViewModels.Properties
{
    public class StringPropertyViewModel : PropertyBaseViewModel
    {
        public StringPropertyViewModel(string title, string value) : base(title)
        {
            this.Value = value;
        }

        public string Value
        {
            get { return value; }
            set
            {
                if (string.Equals(this.value, value)) return;
                this.value = value;
                OnPropertyChanged();
            }
        }
        string value;
    }
}
