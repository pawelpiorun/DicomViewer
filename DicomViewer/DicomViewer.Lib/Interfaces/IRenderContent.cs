using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Interfaces
{
    public interface IRenderContent : IWithCleaningUp
    {
        bool IsLoaded { get; }

        void RequestRender();
        IntPtr GetPointer();

        event EventHandler RenderRequested;
        event EventHandler ContentReady;
    }
}
