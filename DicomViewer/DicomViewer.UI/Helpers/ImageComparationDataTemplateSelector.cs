using DicomViewer.Lib.ViewModels;
using System.Windows;
using System.Windows.Controls;

namespace DicomViewer.UI.Helpers
{
    public class ImageComparationDataTemplateSelector : DataTemplateSelector
    {
        public DataTemplate DicomFittingComparationDataTemplate { get; set; }
        public DataTemplate DefaultTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (item is DicomFittingComparationViewModel)
                return DicomFittingComparationDataTemplate;

            return DefaultTemplate;
        }
    }
}
