using System;
using System.Text;

namespace DicomViewer.Lib.Extensions
{
    public static class DateTimeExtensions
    {
        public static string ToFileFormat(this DateTime dateTime)
        {
            var sb = new StringBuilder();

            sb.Append(dateTime.Year);
            sb.Append(string.Format("{0:00}", dateTime.Month));
            sb.Append(string.Format("{0:00}", dateTime.Day));

            sb.Append(" ");

            sb.Append(string.Format("{0:00}", dateTime.Hour));
            sb.Append(string.Format("{0:00}", dateTime.Minute));
            sb.Append(string.Format("{0:00}", dateTime.Second));
            sb.Append(string.Format("{0:00}", dateTime.Millisecond));

            return sb.ToString();
        }
    }
}
