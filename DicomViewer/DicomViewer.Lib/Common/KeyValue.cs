using System.Collections.Generic;

namespace DicomViewer.Lib.Common
{
    public class KeyValue<TKey, TValue>
        : NotifyPropertyChanged
    {
        public KeyValue(TKey key, TValue value)
        {
            this.Key = key;
            this.Value = value;
        }

        public TKey Key
        {
            get { return key; }
            set
            {
                if (EqualityComparer<TKey>.Default.Equals(key, value)) return;
                key = value;
                OnPropertyChanged();
            }
        }
        TKey key;

        public TValue Value
        {
            get { return val; }
            set
            {
                if (EqualityComparer<TValue>.Default.Equals(val, value)) return;
                val = value;
                OnPropertyChanged();
            }
        }
        TValue val;
    }
}
