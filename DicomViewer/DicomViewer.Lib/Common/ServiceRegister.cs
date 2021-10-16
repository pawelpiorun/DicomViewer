using System;
using System.Collections.Concurrent;

namespace DicomViewer.Lib.Common
{
    public static class ServiceRegister
    {
        public static void Register(Type serviceType, object instance)
        {
            registered.TryAdd(serviceType, instance);
        }
        readonly static ConcurrentDictionary<Type, object> registered = new ConcurrentDictionary<Type, object>();

        public static T Resolve<T>()
        {
            if (registered.TryGetValue(typeof(T), out object value))
                return (T)value;

            return default;
        }
    }
}
