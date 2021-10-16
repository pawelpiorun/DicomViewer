using System;

namespace DicomViewer.Lib.Extensions
{
    public static class TimeSpanExtensions
    {
        public static string Format(this TimeSpan timeSpan)
        {
            if (timeSpan.TotalDays > 1)
                return string.Format("{0}d {1}h {2}min", timeSpan.Days, timeSpan.Hours, timeSpan.Minutes);

            if (timeSpan.TotalHours > 1)
                return string.Format("{0}h {1}min {2}s", timeSpan.Hours, timeSpan.Minutes, timeSpan.Seconds);

            if (timeSpan.TotalMinutes > 15)
                return string.Format("{0}min {1}s", timeSpan.Minutes, timeSpan.Seconds);

            if (timeSpan.TotalMinutes > 1)
                return string.Format("{0}min {1}s {2}ms", timeSpan.Minutes, timeSpan.Seconds, timeSpan.Milliseconds);

            if (timeSpan.TotalSeconds > 1)
                return string.Format("{0}s {1}ms", timeSpan.Seconds, timeSpan.Milliseconds);

            return string.Format("{0}ms", timeSpan.TotalMilliseconds);
        }
    }
}
