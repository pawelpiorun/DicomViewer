using DicomViewer.Lib.ViewModels;
using System;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Data;

namespace DicomViewer.UI.Converters
{
    public class ProcessingModeToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var current = (ProcessingMode)value;
            string stringParam = parameter as string;
            var desiredList = stringParam.Split('|');

            if (desiredList.Any(x => (ProcessingMode)Enum.Parse(typeof(ProcessingMode), x) == current))
                return Visibility.Visible;

            return Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
