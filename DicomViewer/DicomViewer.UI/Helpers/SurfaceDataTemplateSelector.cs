using DicomViewer.Lib.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace DicomViewer.UI.Helpers
{
    public class SurfaceDataTemplateSelector : DataTemplateSelector
    {
        public DataTemplate DicomImageDataTemplate { get; set; }
        public DataTemplate DicomSeriesDataTemplate { get; set; }
        public DataTemplate ImageDataTemplate { get; set; }
        public DataTemplate DefaultDataTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (item is ImageSurfaceViewModel)
                return ImageDataTemplate;
            else if (item is DicomImageSurfaceViewModel)
                return DicomImageDataTemplate;
            else if (item is DicomSeriesSurfaceViewModel)
                return DicomSeriesDataTemplate;

            return DefaultDataTemplate;
        }
    }
}
