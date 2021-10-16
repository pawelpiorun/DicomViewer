using DicomViewer.Lib.ViewModels.Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace DicomViewer.UI.Helpers
{
    public class PropertiesTemplateSelector : DataTemplateSelector
    {
        public DataTemplate StringDataTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (item is StringPropertyViewModel)
                return StringDataTemplate;

            return null;
        }
    }
}
