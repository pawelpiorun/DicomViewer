using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Helpers
{
    public static class AppState
    {
        public static bool IsAlpha
        {
            get
            {
#if DEBUG
                return true;
#else
                return isAlpha;
#endif
            }
            set
            {
                isAlpha = value;
                AppStateChanged?.Invoke(null, EventArgs.Empty);
            }
        }
        static bool isAlpha;

        public static event EventHandler AppStateChanged;
    }
}
